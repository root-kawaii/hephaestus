# Hephaestus - Vulkan 3D Rendering Engine

A custom 3D rendering engine built from scratch using Vulkan API, following the vulkan-guide tutorial with additional custom features. This project demonstrates low-level graphics programming and real-time rendering techniques.

## Project Overview

Hephaestus is a real-time 3D rendering engine that showcases modern graphics programming concepts including GPU-accelerated rendering, physics simulation, and interactive camera controls. Built with C++ and Vulkan, this project emphasizes performance-oriented architecture and memory management.

## Technical Skills Demonstrated

### Graphics Programming
- **Vulkan API**: Direct GPU programming with explicit control over graphics pipeline
- **Shader Programming**: GLSL vertex and fragment shaders for various rendering techniques
- **Pipeline Management**: Multiple rendering pipelines for different material types
- **Memory Management**: VMA (Vulkan Memory Allocator) integration for efficient GPU memory handling
- **Texture Mapping**: Image loading and texture sampling with custom samplers

### Rendering Features
- **3D Mesh Rendering**: Support for loading and rendering 3D models (OBJ format via tinyobjloader)
- **Lighting System**:
  - Directional lighting (sunlight)
  - Ambient lighting
  - Custom lit and textured materials
- **Camera System**:
  - Free-flying camera mode
  - Player-controlled camera with character movement
  - Raycasting for mouse-based world interaction
- **Grid Rendering**: Custom grid shader for visual debugging and scene reference
- **ImGui Integration**: Real-time debugging UI and scene controls

### Advanced Techniques
- **Descriptor Sets**: Efficient GPU resource binding using Vulkan descriptors
- **Push Constants**: Fast per-draw call data updates
- **SSBO (Shader Storage Buffer Objects)**: Dynamic mesh rendering
- **Double Buffering**: Frame overlap system to prevent GPU stalls
- **Swapchain Management**: Window resizing and swapchain recreation
- **Deletion Queue Pattern**: RAII-style resource cleanup for Vulkan objects

### Physics Integration
- **Bullet Physics**: Integrated Bullet3 physics engine for rigid body dynamics
- Character controller implementation with physics-based movement

### Build System & Dependencies
- **CMake**: Cross-platform build configuration
- **Third-party Integration**:
  - SDL2 for window management and input
  - GLM for mathematics
  - Volk for Vulkan function loading
  - stb_image for texture loading
  - Dear ImGui for UI

## Project Structure

```
hephaestus/
├── src/                          # Core engine source code
│   ├── vk_engine.cpp/h          # Main engine class and rendering loop
│   ├── vk_initializers.cpp/h    # Vulkan structure initialization helpers
│   ├── vk_mesh.cpp/h            # Mesh data structures and vertex descriptions
│   ├── vk_textures.cpp/h        # Texture loading and management
│   ├── vk_types.h               # Common Vulkan type definitions
│   ├── player_camera.cpp/h      # Camera controller implementation
│   ├── vk_char_control.cpp/h    # Character movement system
│   ├── physics.cpp/h            # Bullet physics integration
│   └── main.cpp                 # Application entry point
│
├── shaders/                      # GLSL shader source files
│   ├── triangle.vert/frag       # Basic triangle shaders
│   ├── colored_triangle.*       # Colored geometry shaders
│   ├── tri_mesh.vert            # Mesh rendering (vertex only)
│   ├── tri_mesh_ssbo.vert       # SSBO-based mesh rendering
│   ├── textured_lit.frag        # Textured material with lighting
│   ├── default_lit.frag         # Default lit material
│   ├── grid.vert/frag           # Debug grid visualization
│   └── *.spv                    # Compiled SPIR-V binaries
│
├── assets/                       # 3D models and textures
├── asset_core/                   # Asset loading utilities
├── third_party/                  # External dependencies
│   ├── imgui/                   # Dear ImGui UI library
│   ├── volk/                    # Vulkan meta-loader
│   ├── tinyobjloader/           # OBJ model loader
│   ├── bullet3/                 # Physics engine
│   └── ...
│
└── CMakeLists.txt               # Build configuration
```

## Key Components

### VulkanEngine
The core engine class that manages:
- Vulkan instance, device, and swapchain initialization
- Render pass and framebuffer setup
- Pipeline creation and management
- Per-frame rendering with command buffer recording
- Scene management and object rendering
- Resource cleanup with deletion queues

### Rendering Pipeline
1. **Initialization**: Create Vulkan instance, select physical device, create logical device
2. **Swapchain Setup**: Configure presentation surface and swapchain images
3. **Pipeline Creation**: Build graphics pipelines with custom shader stages
4. **Descriptor Management**: Set up uniform buffers and texture bindings
5. **Frame Rendering**:
   - Acquire swapchain image
   - Record command buffers with render passes
   - Submit to graphics queue
   - Present rendered image

### Material System
Flexible material system supporting:
- Textured and untextured materials
- Custom pipeline per material type
- Descriptor set binding for textures and uniforms
- Push constants for per-object data (transform matrices)

### Scene Graph
- WorldObject system for scene hierarchy
- RenderObject references for efficient rendering
- Transform matrices for positioning and orientation
- Camera-relative rendering

## Learning Outcomes

Through building this engine, I gained expertise in:

1. **Low-level Graphics API**: Deep understanding of Vulkan's explicit rendering model
2. **GPU Architecture**: Knowledge of how modern GPUs process graphics commands
3. **Memory Management**: Manual control over GPU memory allocation and synchronization
4. **Shader Programming**: Writing and debugging GLSL shaders
5. **Linear Algebra**: Practical application of matrices, vectors, and transformations
6. **Design Patterns**: Implementing builder, factory, and RAII patterns in C++
7. **Performance Optimization**: Frame timing, memory pools, and efficient rendering
8. **Debugging Graphics**: Using validation layers and visual debugging tools

## Build Instructions

### Prerequisites
- CMake 3.8+
- Vulkan SDK
- C++17 compatible compiler
- SDL2
- GLM

### Building
```bash
mkdir build && cd build
cmake ..
make
```

### Running
```bash
./bin/hephaestus
```

## Controls
- **WASD**: Camera movement
- **Mouse**: Look around
- **ESC**: Exit application
- **Console Mode**: Runtime scene manipulation

## Future Enhancements
- Shadow mapping for dynamic shadows
- Deferred rendering pipeline
- Particle system
- Post-processing effects (bloom, SSAO)
- Model animation system
- Advanced physics constraints

## Technical Highlights for Employers

This project demonstrates:
- ✅ Strong C++ fundamentals and modern C++17 features
- ✅ Experience with industry-standard graphics APIs
- ✅ Understanding of computer graphics theory and practice
- ✅ Ability to integrate complex third-party libraries
- ✅ Clean code architecture with separation of concerns
- ✅ Version control with Git
- ✅ CMake build system proficiency
- ✅ Problem-solving in performance-critical applications
- ✅ Self-directed learning and documentation following

## References
- [Vulkan Guide](https://vkguide.dev/) - Primary learning resource
- [Vulkan Specification](https://www.khronos.org/vulkan/)
- [Learn OpenGL](https://learnopengl.com/) - Graphics fundamentals

