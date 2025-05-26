# Vulkan 3D Gaussian Splatting Implementation

A high-performance Vulkan implementation of 3D Gaussian Splatting for real-time neural rendering using pure compute shaders.

## Development Status

**Currently in active development.** Core infrastructure completed and ready for compute shader implementation.

### ✅ Completed Components

#### Data Loading & Management
- **PLY File Loading**: Custom PLY parser that extracts Gaussian parameters (positions, scales, rotations, opacity, spherical harmonics)
- **Gaussian Data Structure**: Organized data containers for efficient GPU upload
- **Memory Layout Optimization**: Structured data layout for optimal GPU access patterns

#### Vulkan Infrastructure  
- **Context Management**: Complete Vulkan device setup with compute queue family selection
- **Memory Allocator**: Custom buffer manager with staging buffer pipeline for efficient GPU uploads
- **Descriptor Management**: Multi-binding descriptor sets for Gaussian data and camera uniforms

#### GPU Buffer System
- **Storage Buffers**: Separate buffers for positions (vec3), scales (vec3), rotations (vec4), opacity (float), and SH coefficients (float arrays)
- **Uniform Buffers**: Camera view/projection matrices with host-visible memory for real-time updates
- **Staging Pipeline**: Optimized data transfer using staging buffers for large static data

#### Camera System
- **FPS Controls**: WASD movement, mouse look, space/shift for up/down movement
- **Real-time Updates**: Per-frame camera uniform buffer updates via mapped memory
- **Input Processing**: GLFW-based input handling with delta-time movement calculations

#### Compute Pipeline Foundation
- **Pipeline Layout**: Configured for 6+ descriptor bindings (image output + Gaussian data + camera)
- **Descriptor Sets**: Bound storage buffers for all Gaussian parameters and camera uniforms
- **Command Buffer Management**: Ready for compute shader dispatch calls

### ⏳ In Progress

#### 3D Gaussian Splatting Shaders
- **Projection Stage**: Transform 3D Gaussians to screen space
- **Culling & Sorting**: Depth-based sorting for proper alpha blending
- **Rasterization**: Splatting Gaussians to output image

#### Rendering Loop
- **Compute Dispatch**: Orchestrating multi-stage compute pipeline
- **Image Output**: Direct write to swapchain images from compute shaders

## Technical Architecture

### Core Design Principles
- **Pure Compute Approach**: No graphics pipeline - everything handled in compute shaders
- **Direct Output**: Compute shaders write directly to swapchain images
- **Memory Efficiency**: Minimize CPU-GPU transfers with persistent mapped buffers

### Data Flow
```
PLY File → CPU Parsing → Staging Buffers → GPU Storage Buffers
Camera Input → Uniform Buffer → GPU Access
Compute Shaders → Direct Swapchain Write → Display
```

## Tech Stack

- **Graphics API**: Vulkan 1.3
- **Language**: C++17
- **Math Library**: GLM (OpenGL Mathematics)
- **Window/Input**: GLFW
- **Build System**: CMake
- **Target Platforms**: Windows, Linux, macOS (via MoltenVK)

## Dependencies

```bash
# Required
- Vulkan SDK 1.3+
- GLM 
- GLFW3

# Build Tools  
- CMake 3.16+
- C++17 compatible compiler (GCC, Clang, MSVC)
```

## Current Capabilities

### Functional Systems
- Complete Vulkan compute pipeline setup ready for shader dispatch
- Real-time camera system with smooth FPS-style controls
- Efficient GPU memory management with staging buffer uploads
- Per-frame camera uniform updates via host-visible memory mapping
- Descriptor set bindings configured for all Gaussian data streams

### Performance Features
- Zero-copy camera updates using persistent memory mapping
- Optimized buffer layouts for GPU cache efficiency
- Staging buffers used only for large static data (Gaussian parameters)
- Direct uniform buffer updates for small dynamic data (camera matrices)

### Code Architecture
- Modular design with separate classes for Camera, Buffer Management, Vulkan Context
- Template-based buffer creation for type safety
- RAII resource management for automatic cleanup
- Static callback system for GLFW mouse input integration

## Next Steps

1. **Implement compute shaders** for Gaussian projection and rasterization
2. **Add depth sorting** for proper alpha blending order  
3. **Optimize workgroup dispatching** for different screen resolutions
4. **Performance profiling** and GPU timing measurements

**Status**: Core infrastructure complete. Ready for compute shader implementation and rendering loop optimization.

*Documentation will be expanded as compute shaders and rendering features are implemented.*
