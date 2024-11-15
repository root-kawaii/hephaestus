﻿
#include "vk_textures.h"
#include <fstream>
#include <iostream>
#include <player_camera.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

#include "dia.h"

#include <btBulletDynamicsCommon.h>

#include "physics.h"

#include "vk_engine.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>
#include <vk_char_control.h>

#include "VkBootstrap.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

constexpr bool bUseValidationLayers = true;

// we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
using namespace std;
namespace fs = std::filesystem;

vector<string> prev_commands;
vector<string> files;
bool ischanged;
int ID = 0;

#define VK_CHECK(x)                                               \
  do                                                              \
  {                                                               \
    VkResult err = x;                                             \
    if (err)                                                      \
    {                                                             \
      std::cout << "Detected Vulkan error: " << err << std::endl; \
      abort();                                                    \
    }                                                             \
  } while (0)

void VulkanEngine::init()
{
  // We initialize SDL and create a window with it.
  _renderables.reserve(100); // needs to be fixed
  SDL_Init(SDL_INIT_VIDEO);

  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

  _window = SDL_CreateWindow(
      "Hephaestus",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      _windowExtent.width,
      _windowExtent.height,
      window_flags | SDL_WINDOW_RESIZABLE);

  init_vulkan();

  init_swapchain();

  init_default_renderpass();

  init_framebuffers();

  init_commands();

  init_sync_structures();

  init_descriptors();

  init_pipelines();

  // load_images();

  // load_meshes();

  // init_scene();

  init_imgui();

  // everything went fine
  _isInitialized = true;

  Phys::physics_setup();
}

void VulkanEngine::cleanup()
{
  if (_isInitialized)
  {

    // make sure the gpu has stopped doing its things
    vkDeviceWaitIdle(_device);

    cleanupSwapChain();

    _mainDeletionQueue.flush();

    vkDestroySurfaceKHR(_instance, _surface, nullptr);

    vkDestroyDevice(_device, nullptr);
    vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
    vkDestroyInstance(_instance, nullptr);

    SDL_DestroyWindow(_window);
  }
}

void VulkanEngine::cleanupSwapChain()
{
  _swapChainDeletionQueue.flush();
}
// might not work since we cleanup already

void VulkanEngine::recreateSwapChain()
{
  vkDeviceWaitIdle(_device);

  cleanupSwapChain();

  init_swapchain();

  init_default_renderpass();
  // // createImageViews();
  init_framebuffers();

  // init_descriptors();

  init_pipelines();
}

void VulkanEngine::draw()
{

  ImGui::Render();
  // check if window is minimized and skip drawing
  if (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED)
    return;

  // wait until the gpu has finished rendering the last frame. Timeout of 1 second
  VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
  VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

  // now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
  VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

  // request image from the swapchain
  uint32_t swapchainImageIndex;
  VkResult result = vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._presentSemaphore, nullptr, &swapchainImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    // recreateSwapChain();
    return;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  // naming it cmd for shorter writing
  VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

  // begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
  VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  // make a clear-color from frame number. This will flash with a 120 frame period.
  VkClearValue clearValue;
  float flash = abs(sin(_frameNumber / 100000.f));
  clearValue.color = {{0.2f, 0.2f, 0.2f, 1.0f}};

  // clear depth at 1
  VkClearValue depthClear;
  depthClear.depthStencil.depth = 1.f;

  // start the main renderpass.
  // We will use the clear color from above, and the framebuffer of the index the swapchain gave us
  VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_renderPass, _windowExtent, _framebuffers[swapchainImageIndex]);

  // connect clear values
  rpInfo.clearValueCount = 2;

  VkClearValue clearValues[] = {clearValue, depthClear};

  rpInfo.pClearValues = &clearValues[0];

  vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

  if (_sceneLoaded)
  {
    draw_objects(cmd, _renderables.data(), _renderables.size());
  }
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
  // finalize the render pass
  vkCmdEndRenderPass(cmd);
  // finalize the command buffer (we can no longer add commands, but it can now be executed)
  VK_CHECK(vkEndCommandBuffer(cmd));

  // prepare the submission to the queue.
  // we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
  // we will signal the _renderSemaphore, to signal that rendering has finished

  VkSubmitInfo submit = vkinit::submit_info(&cmd);
  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  submit.pWaitDstStageMask = &waitStage;

  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

  // prepare present
  //  this will put the image we just rendered to into the visible window.
  //  we want to wait on the _renderSemaphore for that,
  //  as its necessary that drawing commands have finished before the image is displayed to the user
  VkPresentInfoKHR presentInfo = vkinit::present_info();

  presentInfo.pSwapchains = &_swapchain;
  presentInfo.swapchainCount = 1;

  presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &swapchainImageIndex;

  result = vkQueuePresentKHR(_graphicsQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
  {
    // recreateSwapChain();
  }
  else if (result != VK_SUCCESS)
  {
    throw std::runtime_error("failed to present swap chain image!");
  }

  // increase the number of frames drawn
  _frameNumber++;
}

void VulkanEngine::run()
{
  SDL_Event e;
  bool bQuit = false;

  ImGuiStyle &style = ImGui::GetStyle();
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0.90);

  // Using time point and system_clock
  std::chrono::time_point<std::chrono::system_clock>
      start,
      end, old;

  start = std::chrono::system_clock::now();
  end = std::chrono::system_clock::now();

  // main loop
  while (!bQuit)
  {
    old = end;
    end = std::chrono::system_clock::now();
    std::chrono::duration<float> frame_time = end - old;
    std::chrono::duration<float> elapsed_seconds = end - start;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame(_window);

    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Create"))
        {
          init_scene();
        }
        // if (ImGui::MenuItem("Open", "Ctrl+O"))
        // {
        // }
        // if (ImGui::MenuItem("Save", "Ctrl+S"))
        // {
        // }
        // if (ImGui::MenuItem("Save as.."))
        // {
        // }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }
    // Handle events on queue
    while (SDL_PollEvent(&e) != 0)

    {
      ImGui_ImplSDL2_ProcessEvent(&e);

      if (!_sceneLoaded)
      {
      }

      // close the window when user alt-f4s or clicks the X button
      if (_mode == 1)
      {
        _currentScene.world_camera.process_input_event(&e);
      }
      if (_mode == 2)
      {
        //_currentScene.world_camera._obj = _renderables[_renderables.size() - 1];

        // _mover.move(_mainChar, &e, &_currentScene.world_camera);
      }
      if (e.type == SDL_QUIT)
      {
        bQuit = true;
      }

      else if (e.type == SDL_WINDOWEVENT)
      {
        if (e.window.event == SDL_WINDOWEVENT_EXPOSED)
        {
          SDL_GetWindowSize(_window, (int *)&_windowExtent.width, (int *)&_windowExtent.height);
          printf("Resizing window to: %d x %d\n", (int *)&_windowExtent.width, (int *)&_windowExtent.height);
          SDL_SetWindowSize(_window, (int)_windowExtent.width, (int)_windowExtent.height);
          SDL_Surface *_surface = SDL_GetWindowSurface(_window);
          SDL_BlitSurface(_surface, NULL, _surface, NULL);
          SDL_UpdateWindowSurface(_window);
          recreateSwapChain();
        }
        break;
      }
      else if (e.type == SDL_KEYDOWN)
      {

        if (e.key.keysym.sym == SDLK_1)
        {
          _mode = 0; // konsole
        }
        if (e.key.keysym.sym == SDLK_2)
        {
          _mode = 1; // freecamera
        }
        if (e.key.keysym.sym == SDLK_3)
        {
          _mode = 2; // playmode
        }
        if (e.key.keysym.sym == SDLK_4)
        {
          _renderables[0].material = get_material("defaultmesh");
        }
        if (e.key.keysym.sym == SDLK_5)
        {
          _renderables[0].material = get_material("gridmesh");
        }
        if (e.key.keysym.sym == SDLK_6)
        {
          SDL_GetMouseState(&x_mouse, &y_mouse);
          line_startPoint = _currentScene.world_camera.position;
          line_endPoint = rayCast((double)x_mouse, (double)y_mouse, _currentScene.world_camera.get_projection_matrix(), _currentScene.world_camera.get_view_matrix());
          lineMesh._vertices[0].position = line_startPoint;
          lineMesh._vertices[1].position = line_endPoint;
          lineMesh._vertices[2].position = line_endPoint + glm::vec3(1, 1, 1);
          upload_mesh(lineMesh);
          glm::vec3 intersectLine = line_endPoint - line_startPoint;
        }
        if (e.key.keysym.sym == SDLK_RETURN)
        {
          prev_commands.push_back(_path);
          console_parser();
        }
      }
      _currentScene.world_camera.update_camera(elapsed_seconds.count() * 1000.f);
    }

    if (_sceneLoaded)
    {
      if (_mode == 0)
      {
        // ImGui::InputTextWithHint("", "", _consoleBuffer.data(), 1024);
      }
      for (auto it = std::begin(_currentScene.obj_world); it != std::end(_currentScene.obj_world); ++it)
      {
        std::string a = std::to_string(it->position.x);
        std::string b = std::to_string(it->position.y);
        std::string c = std::to_string(it->position.z);
        a = it->objectName + " " + a + " " + b + " " + c;
        ImGui::Text(a.data());
        // ImGui::Text(std::to_string(_currentScene.obj_world.size()).data());

        std::string newPlusX = std::string("+x");
        if (ImGui::Button(newPlusX.data()))
        {
          it->setPosition(it->position + glm::vec3(1, 0, 0));
        }
        ImGui::SameLine();
        std::string newMinusX = std::string("-x");
        if (ImGui::Button(newMinusX.data()))
        {
          it->setPosition(it->position - glm::vec3(1, 0, 0));
        }
        ImGui::SameLine();
        std::string newPlusY = std::string("+y");
        if (ImGui::Button(newPlusY.data()))
        {
          it->setPosition(it->position + glm::vec3(0, 1, 0));
        }
        ImGui::SameLine();
        std::string newMinusY = std::string("-y");
        if (ImGui::Button(newMinusY.data()))
        {
          it->setPosition(it->position - glm::vec3(0, 1, 0));
        }
        ImGui::SameLine();
        std::string newPlusZ = std::string("+z");
        if (ImGui::Button(newPlusZ.data()))
        {
          it->setPosition(it->position + glm::vec3(0, 0, 1));
        }
        ImGui::SameLine();
        std::string newMinusZ = std::string("-z");
        if (ImGui::Button(newMinusZ.data()))
        {
          it->setPosition(it->position - glm::vec3(0, 0, 1));
        }
      }
      // ImGui::Text("%f,%f", line_endPoint, line_startPoint);
      ImGui::Text("%f", (_mode == 1 || _mode == 2) ? 1 / frame_time.count() : 0);
      ImGui::Text("%f,%f,%f ", _currentScene.world_camera.position.y, _currentScene.world_camera.position.z, _currentScene.world_camera.position.x);
      // ImGui::Text("%f,%f,%f ", _mainChar->position.x, _mainChar->position.y, _mainChar->position.z);

      if (ImGui::Button("Load"))
      {

        pathos(&_path);
        load_meshes2();
        update_scene();
      }
    }

    draw();
  }
}

FrameData &VulkanEngine::get_current_frame()
{
  return _frames[_frameNumber % FRAME_OVERLAP];
}

FrameData &VulkanEngine::get_last_frame()
{
  return _frames[(_frameNumber - 1) % 2];
}

void VulkanEngine::init_vulkan()
{
  vkb::InstanceBuilder builder;

  // make the vulkan instance, with basic debug features
  auto inst_ret = builder.set_app_name("Vulkan Application")
                      .request_validation_layers(bUseValidationLayers)
                      .use_default_debug_messenger()
                      .require_api_version(1, 1, 0)
                      .build();

  vkb::Instance vkb_inst = inst_ret.value();

  // grab the instance
  _instance = vkb_inst.instance;
  _debug_messenger = vkb_inst.debug_messenger;

  SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

  // use vkbootstrap to select a gpu.
  // We want a gpu that can write to the SDL surface and supports vulkan 1.2
  vkb::PhysicalDeviceSelector selector{vkb_inst};
  vkb::PhysicalDevice physicalDevice = selector
                                           .set_minimum_version(1, 1)
                                           .set_surface(_surface)
                                           .select()
                                           .value();

  // create the final vulkan device

  vkb::DeviceBuilder deviceBuilder{physicalDevice};

  vkb::Device vkbDevice = deviceBuilder.build().value();

  // Get the VkDevice handle used in the rest of a vulkan application
  _device = vkbDevice.device;
  _chosenGPU = physicalDevice.physical_device;

  // use vkbootstrap to get a Graphics queue
  _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

  _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

  // initialize the memory allocator
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = _chosenGPU;
  allocatorInfo.device = _device;
  allocatorInfo.instance = _instance;
  vmaCreateAllocator(&allocatorInfo, &_allocator);

  _mainDeletionQueue.push_function([&]()
                                   { vmaDestroyAllocator(_allocator); });

  vkGetPhysicalDeviceProperties(_chosenGPU, &_gpuProperties);

  std::cout << "The gpu has a minimum buffer alignement of " << _gpuProperties.limits.minUniformBufferOffsetAlignment << std::endl;
}

void VulkanEngine::init_swapchain()
{
  vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};

  vkb::Swapchain vkbSwapchain = swapchainBuilder
                                    .use_default_format_selection()
                                    // use vsync present mode
                                    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                    .set_desired_extent(_windowExtent.width, _windowExtent.height)
                                    .build()
                                    .value();

  // store swapchain and its related images
  _swapchain = vkbSwapchain.swapchain;
  _swapchainImages = vkbSwapchain.get_images().value();
  _swapchainImageViews = vkbSwapchain.get_image_views().value();

  _swachainImageFormat = vkbSwapchain.image_format;

  _swapChainDeletionQueue.push_function([=]()
                                        { vkDestroySwapchainKHR(_device, _swapchain, nullptr); });

  // depth image size will match the window
  VkExtent3D depthImageExtent = {
      _windowExtent.width,
      _windowExtent.height,
      1};

  // hardcoding the depth format to 32 bit float
  _depthFormat = VK_FORMAT_D32_SFLOAT;

  // the depth image will be a image with the format we selected and Depth Attachment usage flag
  VkImageCreateInfo dimg_info = vkinit::image_create_info(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

  // for the depth image, we want to allocate it from gpu local memory
  VmaAllocationCreateInfo dimg_allocinfo = {};
  dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // allocate and create the image
  vmaCreateImage(_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);

  // build a image-view for the depth image to use for rendering
  VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
  ;

  VK_CHECK(vkCreateImageView(_device, &dview_info, nullptr, &_depthImageView));

  // add to deletion queues
  _swapChainDeletionQueue.push_function([=]()
                                        {
		vkDestroyImageView(_device, _depthImageView, nullptr);
		vmaDestroyImage(_allocator, _depthImage._image, _depthImage._allocation); });
}

void VulkanEngine::init_default_renderpass()
{
  // we define an attachment description for our main color image
  // the attachment is loaded as "clear" when renderpass start
  // the attachment is stored when renderpass ends
  // the attachment layout starts as "undefined", and transitions to "Present" so its possible to display it
  // we dont care about stencil, and dont use multisampling

  VkAttachmentDescription color_attachment = {};
  color_attachment.format = _swachainImageFormat;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref = {};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depth_attachment = {};
  // Depth attachment
  depth_attachment.flags = 0;
  depth_attachment.format = _depthFormat;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_ref = {};
  depth_attachment_ref.attachment = 1;
  depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // we are going to create 1 subpass, which is the minimum you can do
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;
  // hook the depth attachment into the subpass
  subpass.pDepthStencilAttachment = &depth_attachment_ref;

  // 1 dependency, which is from "outside" into the subpass. And we can read or write color
  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  // dependency from outside to the subpass, making this subpass dependent on the previous renderpasses
  VkSubpassDependency depth_dependency = {};
  depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  depth_dependency.dstSubpass = 0;
  depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  depth_dependency.srcAccessMask = 0;
  depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  // array of 2 dependencies, one for color, two for depth
  VkSubpassDependency dependencies[2] = {dependency, depth_dependency};

  // array of 2 attachments, one for the color, and other for depth
  VkAttachmentDescription attachments[2] = {color_attachment, depth_attachment};

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  // 2 attachments from attachment array
  render_pass_info.attachmentCount = 2;
  render_pass_info.pAttachments = &attachments[0];
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  // 2 dependencies from dependency array
  render_pass_info.dependencyCount = 2;
  render_pass_info.pDependencies = &dependencies[0];

  VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass));

  _swapChainDeletionQueue.push_function([=]()
                                        { vkDestroyRenderPass(_device, _renderPass, nullptr); });
}

void VulkanEngine::init_framebuffers()
{
  // create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
  VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(_renderPass, _windowExtent);

  const uint32_t swapchain_imagecount = _swapchainImages.size();
  _framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

  for (int i = 0; i < swapchain_imagecount; i++)
  {

    VkImageView attachments[2];
    attachments[0] = _swapchainImageViews[i];
    attachments[1] = _depthImageView;

    fb_info.pAttachments = attachments;
    fb_info.attachmentCount = 2;
    VK_CHECK(vkCreateFramebuffer(_device, &fb_info, nullptr, &_framebuffers[i]));

    _swapChainDeletionQueue.push_function([=]()
                                          {
                                       vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
                                       vkDestroyImageView(_device, _swapchainImageViews[i], nullptr); });
  }
}

void VulkanEngine::init_commands()
{
  // create a command pool for commands submitted to the graphics queue.
  // we also want the pool to allow for resetting of individual command buffers
  VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  for (int i = 0; i < FRAME_OVERLAP; i++)
  {

    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

    // allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));

    _mainDeletionQueue.push_function([=]()
                                     { vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr); });
  }

  VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily);
  // create pool for upload context
  VK_CHECK(vkCreateCommandPool(_device, &uploadCommandPoolInfo, nullptr, &_uploadContext._commandPool));

  _mainDeletionQueue.push_function([=]()
                                   { vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr); });

  // allocate the default command buffer that we will use for rendering
  VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_uploadContext._commandPool, 1);

  VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_uploadContext._commandBuffer));
}

void VulkanEngine::init_sync_structures()
{
  // create syncronization structures
  // one fence to control when the gpu has finished rendering the frame,
  // and 2 semaphores to syncronize rendering with swapchain
  // we want the fence to start signalled so we can wait on it on the first frame
  VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

  VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

  for (int i = 0; i < FRAME_OVERLAP; i++)
  {

    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

    // enqueue the destruction of the fence
    _mainDeletionQueue.push_function([=]()
                                     { vkDestroyFence(_device, _frames[i]._renderFence, nullptr); });

    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));

    // enqueue the destruction of semaphores
    _mainDeletionQueue.push_function([=]()
                                     {
		vkDestroySemaphore(_device, _frames[i]._presentSemaphore, nullptr);
		vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr); });
  }

  VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();

  VK_CHECK(vkCreateFence(_device, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence));
  _mainDeletionQueue.push_function([=]()
                                   { vkDestroyFence(_device, _uploadContext._uploadFence, nullptr); });
}

void VulkanEngine::init_pipelines()
{
  VkShaderModule colorMeshShader;
  if (!load_shader_module("shaders/default_lit.frag.spv", &colorMeshShader))
  {
    std::cout << "Error when building the colored mesh shader" << std::endl;
  }

  VkShaderModule texturedMeshShader;
  if (!load_shader_module("shaders/textured_lit.frag.spv", &texturedMeshShader))
  {
    std::cout << "Error when building the colored mesh shader" << std::endl;
  }

  VkShaderModule meshVertShader;
  if (!load_shader_module("shaders/tri_mesh_ssbo.vert.spv", &meshVertShader))
  {
    std::cout << "Error when building the mesh vertex shader module" << std::endl;
  }

  VkShaderModule gridMeshShader;
  if (!load_shader_module("shaders/grid.frag.spv", &gridMeshShader))
  {
    std::cout << "Error when building the mesh vertex shader module" << std::endl;
  }

  VkShaderModule gridVertShader;
  if (!load_shader_module("shaders/grid.vert.spv", &gridVertShader))
  {
    std::cout << "Error when building the mesh vertex shader module" << std::endl;
  }

  // build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
  PipelineBuilder pipelineBuilder;
  PipelineBuilder gridPipelineBuilder;

  pipelineBuilder._shaderStages.push_back(
      vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

  gridPipelineBuilder._shaderStages.push_back(
      vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, gridVertShader));

  pipelineBuilder._shaderStages.push_back(
      vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, colorMeshShader));

  gridPipelineBuilder._shaderStages.push_back(
      vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, gridMeshShader));

  // we start from just the default empty pipeline layout info
  VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::pipeline_layout_create_info();

  // setup push constants
  VkPushConstantRange push_constant;
  // offset 0
  push_constant.offset = 0;
  // size of a MeshPushConstant struct
  push_constant.size = sizeof(MeshPushConstants);
  // for the vertex shader
  push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
  mesh_pipeline_layout_info.pushConstantRangeCount = 1;

  VkDescriptorSetLayout setLayouts[] = {_globalSetLayout, _objectSetLayout};

  mesh_pipeline_layout_info.setLayoutCount = 2;
  mesh_pipeline_layout_info.pSetLayouts = setLayouts;

  VkPipelineLayout meshPipLayout;
  VK_CHECK(vkCreatePipelineLayout(_device, &mesh_pipeline_layout_info, nullptr, &meshPipLayout));

  VkPipelineLayoutCreateInfo grid_mesh_pipeline_layout_info = mesh_pipeline_layout_info;

  VkPipelineLayout gridMeshPipLayout;
  VK_CHECK(vkCreatePipelineLayout(_device, &grid_mesh_pipeline_layout_info, nullptr, &gridMeshPipLayout));

  // we start from  the normal mesh layout
  VkPipelineLayoutCreateInfo textured_pipeline_layout_info = mesh_pipeline_layout_info;

  VkDescriptorSetLayout texturedSetLayouts[] = {_globalSetLayout, _objectSetLayout, _singleTextureSetLayout};

  textured_pipeline_layout_info.setLayoutCount = 3;
  textured_pipeline_layout_info.pSetLayouts = texturedSetLayouts;

  VkPipelineLayout texturedPipeLayout;
  VK_CHECK(vkCreatePipelineLayout(_device, &textured_pipeline_layout_info, nullptr, &texturedPipeLayout));

  // hook the push constants layout
  pipelineBuilder._pipelineLayout = meshPipLayout;

  gridPipelineBuilder._pipelineLayout = gridMeshPipLayout;

  // vertex input controls how to read vertices from vertex buffers. We arent using it yet
  pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();
  gridPipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();

  // input assembly is the configuration for drawing triangle lists, strips, or individual points.
  // we are just going to draw triangle list
  pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  gridPipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

  // build viewport and scissor from the swapchain extents
  pipelineBuilder._viewport.x = 0.0f;
  pipelineBuilder._viewport.y = 0.0f;
  pipelineBuilder._viewport.width = (float)_windowExtent.width;
  pipelineBuilder._viewport.height = (float)_windowExtent.height;
  pipelineBuilder._viewport.minDepth = 0.0f;
  pipelineBuilder._viewport.maxDepth = 1.0f;

  gridPipelineBuilder._viewport.x = 0.0f;
  gridPipelineBuilder._viewport.y = 0.0f;
  gridPipelineBuilder._viewport.width = (float)_windowExtent.width;
  gridPipelineBuilder._viewport.height = (float)_windowExtent.height;
  gridPipelineBuilder._viewport.minDepth = 0.0f;
  gridPipelineBuilder._viewport.maxDepth = 1.0f;

  pipelineBuilder._scissor.offset = {0, 0};
  pipelineBuilder._scissor.extent = _windowExtent;

  gridPipelineBuilder._scissor.offset = {0, 0};
  gridPipelineBuilder._scissor.extent = _windowExtent;

  // configure the rasterizer to draw filled triangles
  pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

  gridPipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

  // we dont use multisampling, so just run the default one
  pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();

  gridPipelineBuilder._multisampling = vkinit::multisampling_state_create_info();

  // a single blend attachment with no blending and writing to RGBA
  pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

  gridPipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

  // default depthtesting
  pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

  gridPipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

  // build the mesh pipeline

  VertexInputDescription vertexDescription = Vertex::get_vertex_description();

  // connect the pipeline builder vertex input info to the one we get from Vertex
  pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
  pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

  pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
  pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

  gridPipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
  gridPipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

  gridPipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
  gridPipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

  // build the mesh triangle pipeline
  VkPipeline meshPipeline = pipelineBuilder.build_pipeline(_device, _renderPass);

  VkPipeline gridMeshPipeline = gridPipelineBuilder.build_pipeline(_device, _renderPass);

  create_material(meshPipeline, meshPipLayout, "defaultmesh");

  create_material(gridMeshPipeline, gridMeshPipLayout, "gridmesh");

  pipelineBuilder._shaderStages.clear();
  gridPipelineBuilder._shaderStages.clear();

  pipelineBuilder._shaderStages.push_back(
      vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

  gridPipelineBuilder._shaderStages.push_back(
      vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, gridVertShader));

  pipelineBuilder._shaderStages.push_back(
      vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, texturedMeshShader));

  pipelineBuilder._pipelineLayout = texturedPipeLayout;
  VkPipeline texPipeline = pipelineBuilder.build_pipeline(_device, _renderPass);
  create_material(texPipeline, texturedPipeLayout, "texturedmesh");

  _swapChainDeletionQueue.push_function([=]()
                                        {
                                      vkDestroyShaderModule(_device, meshVertShader, nullptr);
                                      vkDestroyShaderModule(_device, colorMeshShader, nullptr);
                                      vkDestroyShaderModule(_device, gridMeshShader, nullptr);
                                      vkDestroyShaderModule(_device, texturedMeshShader, nullptr);
                                      vkDestroyPipeline(_device, meshPipeline, nullptr);
                                      vkDestroyPipeline(_device, texPipeline, nullptr);

                                      vkDestroyPipelineLayout(_device, meshPipLayout, nullptr);
                                      vkDestroyPipelineLayout(_device, texturedPipeLayout, nullptr); });
}

bool VulkanEngine::load_shader_module(const char *filePath, VkShaderModule *outShaderModule)
{
  // open the file. With cursor at the end
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    return false;
  }

  // find what the size of the file is by looking up the location of the cursor
  // because the cursor is at the end, it gives the size directly in bytes
  size_t fileSize = (size_t)file.tellg();

  // spirv expects the buffer to be on uint32, so make sure to reserve a int vector big enough for the entire file
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

  // put file cursor at beggining
  file.seekg(0);

  // load the entire file into the buffer
  file.read((char *)buffer.data(), fileSize);

  // now that the file is loaded into the buffer, we can close it
  file.close();

  // create a new shader module, using the buffer we loaded
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.pNext = nullptr;

  // codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
  createInfo.codeSize = buffer.size() * sizeof(uint32_t);
  createInfo.pCode = buffer.data();

  // check that the creation goes well.
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
  {
    return false;
  }
  *outShaderModule = shaderModule;
  return true;
}

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
{
  // make viewport state from our stored viewport and scissor.
  // at the moment we wont support multiple viewports or scissors
  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.pNext = nullptr;

  viewportState.viewportCount = 1;
  viewportState.pViewports = &_viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &_scissor;

  // setup dummy color blending. We arent using transparent objects yet
  // the blending is just "no blend", but we do write to the color attachment
  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.pNext = nullptr;

  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &_colorBlendAttachment;

  // build the actual pipeline
  // we now use all of the info structs we have been writing into into this one to create the pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.pNext = nullptr;

  pipelineInfo.stageCount = _shaderStages.size();
  pipelineInfo.pStages = _shaderStages.data();
  pipelineInfo.pVertexInputState = &_vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &_inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &_rasterizer;
  pipelineInfo.pMultisampleState = &_multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDepthStencilState = &_depthStencil;
  pipelineInfo.layout = _pipelineLayout;
  pipelineInfo.renderPass = pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  // its easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
  VkPipeline newPipeline;
  if (vkCreateGraphicsPipelines(
          device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
  {
    std::cout << "failed to create pipline\n";
    return VK_NULL_HANDLE; // failed to create graphics pipeline
  }
  else
  {
    return newPipeline;
  }
}

void VulkanEngine::load_meshes()
{
  Mesh triMesh{};
  // make the array 3 vertices long
  triMesh._vertices.resize(3);

  // vertex positions
  triMesh._vertices[0].position = {1.f, 1.f, 0.0f};
  triMesh._vertices[1].position = {-1.f, 1.f, 0.0f};
  triMesh._vertices[2].position = {0.f, -1.f, 0.0f};

  // vertex colors, all green
  triMesh._vertices[0].color = {0.f, 1.f, 0.0f}; // pure green
  triMesh._vertices[1].color = {0.f, 1.f, 0.0f}; // pure green
  triMesh._vertices[2].color = {0.f, 1.f, 0.0f}; // pure green
  // we dont care about the vertex normals

  // load the monkey
  Mesh monkeyMesh{};
  monkeyMesh.load_from_obj("assets/monkey_flat.obj");
  // monkeyMesh._vertices[0].position *= _selectedShader;

  Mesh lostEmpire{};
  lostEmpire.load_from_obj("assets/lost_empire.obj");

  Mesh floor{};
  floor.load_from_obj("assets/floor.obj");

  upload_mesh(triMesh);
  upload_mesh(monkeyMesh);
  upload_mesh(lostEmpire);
  upload_mesh(floor);

  _meshes["monkey"] = monkeyMesh;
  _meshes["triangle"] = triMesh;
  _meshes["empire"] = lostEmpire;
  _meshes["floor"] = floor;
}

void VulkanEngine::load_images()
{
  Texture lostEmpire;

  vkutil::load_image_from_file(*this, "assets/lost_empire-RGBA.png", lostEmpire.image);

  VkImageViewCreateInfo imageinfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, lostEmpire.image._image, VK_IMAGE_ASPECT_COLOR_BIT);
  vkCreateImageView(_device, &imageinfo, nullptr, &lostEmpire.imageView);

  _mainDeletionQueue.push_function([=]()
                                   { vkDestroyImageView(_device, lostEmpire.imageView, nullptr); });

  _loadedTextures["empire_diffuse"] = lostEmpire;
}

void VulkanEngine::upload_mesh(Mesh &mesh)
{
  const size_t bufferSize = mesh._vertices.size() * sizeof(Vertex);
  // allocate vertex buffer
  VkBufferCreateInfo stagingBufferInfo = {};
  stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  stagingBufferInfo.pNext = nullptr;
  // this is the total size, in bytes, of the buffer we are allocating
  stagingBufferInfo.size = bufferSize;
  // this buffer is going to be used as a Vertex Buffer
  stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  // let the VMA library know that this data should be writeable by CPU, but also readable by GPU
  VmaAllocationCreateInfo vmaallocInfo = {};
  vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

  AllocatedBuffer stagingBuffer;

  // allocate the buffer
  VK_CHECK(vmaCreateBuffer(_allocator, &stagingBufferInfo, &vmaallocInfo,
                           &stagingBuffer._buffer,
                           &stagingBuffer._allocation,
                           nullptr));

  // copy vertex data
  void *data;
  vmaMapMemory(_allocator, stagingBuffer._allocation, &data);

  memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

  vmaUnmapMemory(_allocator, stagingBuffer._allocation);

  // allocate vertex buffer
  VkBufferCreateInfo vertexBufferInfo = {};
  vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  vertexBufferInfo.pNext = nullptr;
  // this is the total size, in bytes, of the buffer we are allocating
  vertexBufferInfo.size = bufferSize;
  // this buffer is going to be used as a Vertex Buffer
  vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  // let the VMA library know that this data should be gpu native
  vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  // allocate the buffer
  VK_CHECK(vmaCreateBuffer(_allocator, &vertexBufferInfo, &vmaallocInfo,
                           &mesh._vertexBuffer._buffer,
                           &mesh._vertexBuffer._allocation,
                           nullptr));
  // add the destruction of triangle mesh buffer to the deletion queue
  _mainDeletionQueue.push_function([=]()
                                   { vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation); });

  immediate_submit([=](VkCommandBuffer cmd)
                   {
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = bufferSize;
		vkCmdCopyBuffer(cmd, stagingBuffer._buffer, mesh._vertexBuffer._buffer, 1, & copy); });

  vmaDestroyBuffer(_allocator, stagingBuffer._buffer, stagingBuffer._allocation);
}

Material *VulkanEngine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name)
{
  Material mat;
  mat.pipeline = pipeline;
  mat.pipelineLayout = layout;
  _materials[name] = mat;
  return &_materials[name];
}

Material *VulkanEngine::get_material(const std::string &name)
{
  // search for the object, and return nullpointer if not found
  auto it = _materials.find(name);
  if (it == _materials.end())
  {
    return nullptr;
  }
  else
  {
    return &(*it).second;
  }
}

Mesh *VulkanEngine::get_mesh(const std::string &name)
{
  auto it = _meshes.find(name);
  if (it == _meshes.end())
  {
    return nullptr;
  }
  else
  {
    return &(*it).second;
  }
}

void VulkanEngine::draw_objects(VkCommandBuffer cmd, RenderObject *first, int count)
{
  // make a model view matrix for rendering the object
  // camera view
  glm::vec3 camPos = _currentScene.world_camera.position;
  glm::mat4 view;
  if (_mode == 2)
  {
    // view =_currentScene.world_camera.get_view_matrix_obj(_mainChar);
  }
  else
  {
    view = _currentScene.world_camera.get_view_matrix();
  }

  // camera projection
  glm::mat4 projection = _currentScene.world_camera.get_projection_matrix(false);
  // projection[1][1] *= -1;

  GPUCameraData camData;
  camData.proj = projection;
  camData.view = view;
  camData.viewproj = projection * view;

  void *data;
  vmaMapMemory(_allocator, get_current_frame().cameraBuffer._allocation, &data);

  memcpy(data, &camData, sizeof(GPUCameraData));

  vmaUnmapMemory(_allocator, get_current_frame().cameraBuffer._allocation);

  float framed = (_frameNumber / 120.f);

  _sceneParameters.ambientColor = {sin(framed), 0, cos(framed), 1};

  char *sceneData;
  vmaMapMemory(_allocator, _sceneParameterBuffer._allocation, (void **)&sceneData);

  int frameIndex = _frameNumber % FRAME_OVERLAP;

  sceneData += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;

  memcpy(sceneData, &_sceneParameters, sizeof(GPUSceneData));

  vmaUnmapMemory(_allocator, _sceneParameterBuffer._allocation);

  void *objectData;
  vmaMapMemory(_allocator, get_current_frame().objectBuffer._allocation, &objectData);

  GPUObjectData *objectSSBO = (GPUObjectData *)objectData;

  for (int i = 0; i < count; i++)
  {
    RenderObject &object = first[i];
    objectSSBO[i].modelMatrix = object.transformMatrix;
  }

  vmaUnmapMemory(_allocator, get_current_frame().objectBuffer._allocation);

  Mesh *lastMesh = nullptr;
  Material *lastMaterial = nullptr;

  for (int i = 0; i < count; i++)
  {

    RenderObject &object = first[i];

    // only bind the pipeline if it doesnt match with the already bound one
    if (object.material != lastMaterial)
    {

      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
      lastMaterial = object.material;

      uint32_t uniform_offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0, 1, &get_current_frame().globalDescriptor, 1, &uniform_offset);

      // object data descriptor
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1, &get_current_frame().objectDescriptor, 0, nullptr);

      if (object.material->textureSet != VK_NULL_HANDLE)
      {
        // texture descriptor
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 2, 1, &object.material->textureSet, 0, nullptr);
      }
    }

    glm::mat4 model = object.transformMatrix;
    // final render matrix, that we are calculating on the cpu
    glm::mat4 mesh_matrix = model;

    MeshPushConstants constants;
    constants.render_matrix = mesh_matrix;

    // upload the mesh to the gpu via pushconstants
    vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

    // only bind the mesh if its a different one from last bind
    if (object.mesh != lastMesh)
    {
      // bind the mesh vertex buffer with offset 0
      VkDeviceSize offset = 0;
      vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->_vertexBuffer._buffer, &offset);
      lastMesh = object.mesh;
    }
    // we can now draw
    vkCmdDraw(cmd, object.mesh->_vertices.size(), 1, 0, i);
  }
}

void VulkanEngine::init_scene()
{
  // _currentScene.obj_world.push_back();
  PlayerCamera newCam;
  newCam.inputAxis.x = 0.f;
  newCam.inputAxis.y = 0.f;
  newCam.inputAxis.z = 0.f;
  newCam.position = {0.f, 5.f, -5.f};
  _currentScene.world_camera = newCam;

  _sceneLoaded = true;
  // _currentScene.player = ;

  // RenderObject floor;
  // floor.mesh = get_mesh("floor");
  // floor.material = get_material("defaultmesh");
  // floor.transformMatrix = glm::mat4{1.0f};
  // floor.position = {0, 0, 0};

  // _renderables.push_back(floor);

  // RenderObject monkey;
  // monkey.mesh = get_mesh("monkey");
  // monkey.material = get_material("defaultmesh");
  // monkey.transformMatrix = glm::mat4{1.0f};
  // monkey.position = {1, 0, 0};

  // _renderables.push_back(monkey);

  // Material *texturedMat = get_material("texturedmesh");

  // VkDescriptorSetAllocateInfo allocInfo = {};
  // allocInfo.pNext = nullptr;
  // allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  // allocInfo.descriptorPool = _descriptorPool;
  // allocInfo.descriptorSetCount = 1;
  // allocInfo.pSetLayouts = &_singleTextureSetLayout;

  // vkAllocateDescriptorSets(_device, &allocInfo, &texturedMat->textureSet);

  // VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

  // vkCreateSampler(_device, &samplerInfo, nullptr, &_blockySampler);

  // _mainDeletionQueue.push_function([=]()
  //                                  { vkDestroySampler(_device, _blockySampler, nullptr); });

  // VkDescriptorImageInfo imageBufferInfo;
  // imageBufferInfo.sampler = blockySampler;
  // imageBufferInfo.imageView = _loadedTextures["empire_diffuse"].imageView;
  // imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  // VkWriteDescriptorSet texture1 = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat->textureSet, &imageBufferInfo, 0);

  // vkUpdateDescriptorSets(_device, 1, &texture1, 0, nullptr);
}

AllocatedBuffer VulkanEngine::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
  // allocate vertex buffer
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.pNext = nullptr;
  bufferInfo.size = allocSize;

  bufferInfo.usage = usage;

  // let the VMA library know that this data should be writeable by CPU, but also readable by GPU
  VmaAllocationCreateInfo vmaallocInfo = {};
  vmaallocInfo.usage = memoryUsage;

  AllocatedBuffer newBuffer;

  // allocate the buffer
  VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
                           &newBuffer._buffer,
                           &newBuffer._allocation,
                           nullptr));

  return newBuffer;
}

size_t VulkanEngine::pad_uniform_buffer_size(size_t originalSize)
{
  // Calculate required alignment based on minimum device offset alignment
  size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
  size_t alignedSize = originalSize;
  if (minUboAlignment > 0)
  {
    alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
  }
  return alignedSize;
}

void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function)
{
  VkCommandBuffer cmd = _uploadContext._commandBuffer;
  // begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
  VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  function(cmd);

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkSubmitInfo submit = vkinit::submit_info(&cmd);

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _uploadContext._uploadFence));

  vkWaitForFences(_device, 1, &_uploadContext._uploadFence, true, 9999999999);
  vkResetFences(_device, 1, &_uploadContext._uploadFence);

  vkResetCommandPool(_device, _uploadContext._commandPool, 0);
}

void VulkanEngine::init_descriptors()
{

  // create a descriptor pool that will hold 10 uniform buffers
  std::vector<VkDescriptorPoolSize> sizes =
      {
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
          {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = 0;
  pool_info.maxSets = 10;
  pool_info.poolSizeCount = (uint32_t)sizes.size();
  pool_info.pPoolSizes = sizes.data();

  vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorPool);

  VkDescriptorSetLayoutBinding cameraBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
  VkDescriptorSetLayoutBinding sceneBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

  VkDescriptorSetLayoutBinding bindings[] = {cameraBind, sceneBind};

  VkDescriptorSetLayoutCreateInfo setinfo = {};
  setinfo.bindingCount = 2;
  setinfo.flags = 0;
  setinfo.pNext = nullptr;
  setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  setinfo.pBindings = bindings;

  vkCreateDescriptorSetLayout(_device, &setinfo, nullptr, &_globalSetLayout);

  VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

  VkDescriptorSetLayoutCreateInfo set2info = {};
  set2info.bindingCount = 1;
  set2info.flags = 0;
  set2info.pNext = nullptr;
  set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set2info.pBindings = &objectBind;

  vkCreateDescriptorSetLayout(_device, &set2info, nullptr, &_objectSetLayout);

  VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  VkDescriptorSetLayoutCreateInfo set3info = {};
  set3info.bindingCount = 1;
  set3info.flags = 0;
  set3info.pNext = nullptr;
  set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set3info.pBindings = &textureBind;

  vkCreateDescriptorSetLayout(_device, &set3info, nullptr, &_singleTextureSetLayout);

  const size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));

  _sceneParameterBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

  for (int i = 0; i < FRAME_OVERLAP; i++)
  {
    _frames[i].cameraBuffer = create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    const int MAX_OBJECTS = 10000;
    _frames[i].objectBuffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_globalSetLayout;

    vkAllocateDescriptorSets(_device, &allocInfo, &_frames[i].globalDescriptor);

    VkDescriptorSetAllocateInfo objectSetAlloc = {};
    objectSetAlloc.pNext = nullptr;
    objectSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    objectSetAlloc.descriptorPool = _descriptorPool;
    objectSetAlloc.descriptorSetCount = 1;
    objectSetAlloc.pSetLayouts = &_objectSetLayout;

    vkAllocateDescriptorSets(_device, &objectSetAlloc, &_frames[i].objectDescriptor);

    VkDescriptorBufferInfo cameraInfo;
    cameraInfo.buffer = _frames[i].cameraBuffer._buffer;
    cameraInfo.offset = 0;
    cameraInfo.range = sizeof(GPUCameraData);

    VkDescriptorBufferInfo sceneInfo;
    sceneInfo.buffer = _sceneParameterBuffer._buffer;
    sceneInfo.offset = 0;
    sceneInfo.range = sizeof(GPUSceneData);

    VkDescriptorBufferInfo objectBufferInfo;
    objectBufferInfo.buffer = _frames[i].objectBuffer._buffer;
    objectBufferInfo.offset = 0;
    objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

    VkWriteDescriptorSet cameraWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _frames[i].globalDescriptor, &cameraInfo, 0);

    VkWriteDescriptorSet sceneWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _frames[i].globalDescriptor, &sceneInfo, 1);

    VkWriteDescriptorSet objectWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _frames[i].objectDescriptor, &objectBufferInfo, 0);

    VkWriteDescriptorSet setWrites[] = {cameraWrite, sceneWrite, objectWrite};

    vkUpdateDescriptorSets(_device, 3, setWrites, 0, nullptr);
  }

  _mainDeletionQueue.push_function([&]()
                                   {

		vmaDestroyBuffer(_allocator, _sceneParameterBuffer._buffer, _sceneParameterBuffer._allocation);

		vkDestroyDescriptorSetLayout(_device, _objectSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(_device, _globalSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(_device, _singleTextureSetLayout, nullptr);

		vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

		for (int i = 0; i < FRAME_OVERLAP; i++)
		{
			vmaDestroyBuffer(_allocator, _frames[i].cameraBuffer._buffer, _frames[i].cameraBuffer._allocation);

			vmaDestroyBuffer(_allocator, _frames[i].objectBuffer._buffer, _frames[i].objectBuffer._allocation);
		} });
}

void VulkanEngine::init_imgui()
{
  // 1: create descriptor pool for IMGUI
  //  the size of the pool is very oversize, but it's copied from imgui demo itself.
  VkDescriptorPoolSize pool_sizes[] =
      {
          {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
          {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
          {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
          {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
          {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
          {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
          {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = std::size(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;

  VkDescriptorPool imguiPool;
  VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr, &imguiPool));

  // 2: initialize imgui library

  // this initializes the core structures of imgui
  ImGui::CreateContext();

  // this initializes imgui for SDL
  ImGui_ImplSDL2_InitForVulkan(_window);

  // this initializes imgui for Vulkan
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = _instance;
  init_info.PhysicalDevice = _chosenGPU;
  init_info.Device = _device;
  init_info.Queue = _graphicsQueue;
  init_info.DescriptorPool = imguiPool;
  init_info.MinImageCount = 3;
  init_info.ImageCount = 3;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  ImGui_ImplVulkan_Init(&init_info, _renderPass);

  // execute a gpu command to upload imgui font textures
  immediate_submit([&](VkCommandBuffer cmd)
                   { ImGui_ImplVulkan_CreateFontsTexture(cmd); });

  // clear font textures from cpu data
  ImGui_ImplVulkan_DestroyFontUploadObjects();

  // add the destroy the imgui created structures
  _mainDeletionQueue.push_function([=]()
                                   {

    vkDestroyDescriptorPool(_device, imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown(); });
}

void VulkanEngine::update_scene()
{

  // // make the array 3 vertices long
  // lineMesh._vertices.resize(3);

  // // vertex positions
  // lineMesh._vertices[0].position = {1.f, 1.f, 0.0f};
  // lineMesh._vertices[1].position = {-1.f, 1.f, 0.0f};
  // lineMesh._vertices[2].position = {0.f, -1.f, 0.0f};

  // // vertex colors, all green
  // lineMesh._vertices[0].color = {1.f, 1.f, 0.0f}; // pure green
  // lineMesh._vertices[1].color = {1.f, 1.f, 0.0f}; // pure green
  // lineMesh._vertices[2].color = {1.f, 1.f, 0.0f}; // pure green

  // // we don't care about the vertex normals

  // upload_mesh(lineMesh);

  // RenderObject line;
  // line.mesh = &lineMesh;
  // line.material = get_material("defaultmesh");
  // line.transformMatrix = glm::mat4{1.0f};
  // line.position = {0, 0, 0};

  // _renderables.push_back(line);

  // WorldObject newOne0;

  // newOne0.position = {0, 0, 0};
  // newOne0.objectName = "ciao";
  // newOne0.ID = ID;
  // ID += 1;
  // newOne0.reference = &_renderables.back();

  // // maybe reference directly object ??

  // _currentScene.obj_world.push_back(newOne0);

  RenderObject obj;
  obj.mesh = get_mesh(_path.data());
  obj.material = get_material("defaultmesh");
  obj.transformMatrix = glm::mat4{1.0f};
  obj.position = {0, 0, 0};

  _renderables.push_back(obj);

  WorldObject newOne;

  newOne.position = {0, 0, 0};
  newOne.objectName = _path.erase(0, 7).erase(_path.length() - 4, 4).data();
  newOne.ID = ID;
  ID += 1;
  newOne.reference = &_renderables.back();

  // maybe reference directly object ??

  _currentScene.obj_world.push_back(newOne);

  // Material *texturedMat = get_material("texturedmesh");

  // VkDescriptorImageInfo imageBufferInfo;

  // imageBufferInfo.sampler = _blockySampler;
  // imageBufferInfo.imageView = _loadedTextures["empire_diffuse"].imageView;
  // imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  // VkWriteDescriptorSet texture1 = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat->textureSet, &imageBufferInfo, 0);
  // vkUpdateDescriptorSets(_device, 1, &texture1, 0, nullptr);
}

void VulkanEngine::load_meshes2()
{

  // load the monkey
  Mesh pathMesh{};
  pathMesh.load_from_obj(_path.data());
  // monkeyMesh._vertices[0].position *= _selectedShader;

  upload_mesh(pathMesh);

  _meshes[_path.data()] = pathMesh;
}

void VulkanEngine::console_parser()
{
  if (strncmp(_consoleBuffer.data(), "load", 4) == 0)
  {
    std::string str = _consoleBuffer.data();
    str.erase(0, 5);
    std::string newone = "assets/";
    _path = newone + str;
    load_meshes2();
    update_scene();
  }
  if (strncmp(_consoleBuffer.data(), "reset", 5) == 0)
  {
    // _path.erase(0, 4);
    _currentScene.world_camera.position = {0.f, 6.f, 5.f};
  }
  if (strncmp(_consoleBuffer.data(), "ls", 2) == 0)
  {
    // _path.erase(0, 4);
    std::string str = _consoleBuffer.data();
    str.erase(0, 3);
    for (const auto &entry : fs::directory_iterator(str))
      files.push_back(entry.path());
  }
}

glm::vec3 VulkanEngine::rayCast(double xpos, double ypos, glm::mat4 projection, glm::mat4 view)
{
  // converts a position from the 2d xpos, ypos to a normalized 3d direction
  float x = (2.0f * xpos) / _windowExtent.width - 1.0f;
  float y = 1.0f - (2.0f * ypos) / _windowExtent.height;
  float z = 1.0f;
  glm::vec3 ray_nds = glm::vec3(x, y, z);
  glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
  // eye space to clip we would multiply by projection so
  // clip space to eye space is the inverse projection
  glm::vec4 ray_eye = inverse(projection) * ray_clip;
  // convert point to forwards
  ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
  // world space to eye space is usually multiply by view so
  // eye space to world space is inverse view
  glm::vec4 inv_ray_wor = (inverse(view) * ray_eye);
  glm::vec3 ray_wor = glm::vec3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
  ray_wor = normalize(ray_wor);
  return ray_wor;
}

void WorldObject::setPosition(glm::vec3 newpos)
{

  this->position = newpos;
  this->reference->position = newpos;
  glm::mat4 translation = glm::translate(glm::mat4{1.0}, newpos);
  glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));

  this->reference->transformMatrix = translation * scale;
}
