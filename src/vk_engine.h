﻿// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "vk_char_control.h"
#include <vk_types.h>
#include <vector>
#include <functional>
#include <deque>
#include <vk_mesh.h>
#include <player_camera.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <unordered_map>
#include <string>

// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.
struct Movement;
struct PlayerCamera;

class PipelineBuilder
{
public:
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
    VkViewport _viewport;
    VkRect2D _scissor;
    VkPipelineRasterizationStateCreateInfo _rasterizer;
    VkPipelineColorBlendAttachmentState _colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo _multisampling;
    VkPipelineLayout _pipelineLayout;
    VkPipelineDepthStencilStateCreateInfo _depthStencil;
    VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};

struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()> &&function)
    {
        deletors.push_back(function);
    }

    void flush()
    {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
        {
            (*it)(); // call functors
        }

        deletors.clear();
    }
};

struct MeshPushConstants
{
    glm::vec4 data;
    glm::mat4 render_matrix;
};

struct Material
{
    VkDescriptorSet textureSet{VK_NULL_HANDLE};
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

struct Texture
{
    AllocatedImage image;
    VkImageView imageView;
};

struct RenderObject
{
    Mesh *mesh;

    Material *material;

    glm::mat4 transformMatrix;

    glm::vec3 position;
};

struct FrameData
{
    VkSemaphore _presentSemaphore, _renderSemaphore;
    VkFence _renderFence;

    DeletionQueue _frameDeletionQueue;

    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;

    AllocatedBuffer cameraBuffer;
    VkDescriptorSet globalDescriptor;

    AllocatedBuffer objectBuffer;
    VkDescriptorSet objectDescriptor;
};

struct UploadContext
{
    VkFence _uploadFence;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
};
struct GPUCameraData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewproj;
};

struct GPUSceneData
{
    glm::vec4 fogColor;     // w is for exponent
    glm::vec4 fogDistances; // x for min, y for max, zw unused.
    glm::vec4 ambientColor;
    glm::vec4 sunlightDirection; // w for sun power
    glm::vec4 sunlightColor;
};

struct GPUObjectData
{
    glm::mat4 modelMatrix;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class WorldObject
{
public:
    glm::vec3 position;

    std::string objectName;

    RenderObject *reference;

    void setPosition(glm::vec3 newpos); // index the right object using ID, basically get the render object I added as ID-th

    int ID;

    // rotation
};

struct Scene
{
    std::vector<WorldObject> obj_world;
    PlayerCamera world_camera;
    WorldObject player;
};

class State
{
public:
    float time;
    Scene *scene;

private:
    float id;
};

class VulkanEngine
{

public:
    RenderObject *_mainChar;
    bool _sceneLoaded{false};
    std::string _path{""};
    std::string _consoleBuffer{""};
    int _mode{0}; // 0 is conole, 1 is freecam, 2 is playmode
    bool _isInitialized{false};
    int _frameNumber{0};
    int _selectedShader{0};
    Scene _currentScene;
    int x_mouse;
    int y_mouse;
    Mesh lineMesh;
    glm::vec3 line_startPoint;
    glm::vec3 line_endPoint;

    VkExtent2D _windowExtent{1000, 400};

    std::vector<VkFence> inFlightFences;

    bool framebufferResized = false;

    struct SDL_Window *_window{nullptr};

    // PlayerCamera _camera;
    Movement _mover;

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;
    VkPhysicalDevice _chosenGPU;
    VkDevice _device;

    VkPhysicalDeviceProperties _gpuProperties;

    FrameData _frames[FRAME_OVERLAP];

    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;

    VkRenderPass _renderPass;

    VkSurfaceKHR _surface;
    VkSwapchainKHR _swapchain;
    VkFormat _swachainImageFormat;

    std::vector<VkFramebuffer> _framebuffers;
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;

    DeletionQueue _mainDeletionQueue;
    DeletionQueue _swapChainDeletionQueue;

    VmaAllocator _allocator; // vma lib allocator

    // depth resources
    VkImageView _depthImageView;
    AllocatedImage _depthImage;

    // the format for the depth image
    VkFormat _depthFormat;

    VkDescriptorPool _descriptorPool;

    VkDescriptorSetLayout _globalSetLayout;
    VkDescriptorSetLayout _objectSetLayout;
    VkDescriptorSetLayout _singleTextureSetLayout;

    GPUSceneData _sceneParameters;
    AllocatedBuffer _sceneParameterBuffer;

    bool _checko{false};

    UploadContext _uploadContext;
    // initializes everything in the engine
    void init();

    // shuts down the engine
    void cleanup();

    // draw loop
    void draw();

    // run main loop
    void run();

    FrameData &get_current_frame();
    FrameData &get_last_frame();

    // default array of renderable objects
    std::vector<RenderObject> _renderables;

    std::unordered_map<std::string, Material> _materials;
    std::unordered_map<std::string, Mesh> _meshes;
    std::unordered_map<std::string, Texture> _loadedTextures;
    // functions

    // create material and add it to the map
    Material *create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name);

    // returns nullptr if it cant be found
    Material *get_material(const std::string &name);

    // returns nullptr if it cant be found
    Mesh *get_mesh(const std::string &name);

    VkSampler _blockySampler;

    void console_parser();

    void init_imgui();

    void update_scene();

    void load_meshes2();

    void recreateSwapChain();

    void cleanupSwapChain();

    // our draw function
    void draw_objects(VkCommandBuffer cmd, RenderObject *first, int count);

    AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

    size_t pad_uniform_buffer_size(size_t originalSize);

    void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

private:
    void init_vulkan();

    void init_swapchain();

    void init_default_renderpass();

    void init_framebuffers();

    void init_commands();

    void init_sync_structures();

    void init_pipelines();

    void init_scene();

    void init_descriptors();

    // loads a shader module from a spir-v file. Returns false if it errors
    bool load_shader_module(const char *filePath, VkShaderModule *outShaderModule);

    void load_meshes();

    void load_images();

    void upload_mesh(Mesh &mesh);

    glm::vec3 rayCast(double xpos, double ypos, glm::mat4 projection, glm::mat4 view);
};
