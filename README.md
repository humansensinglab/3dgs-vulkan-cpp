# Vulkan 3D Gaussian Splatting Implementation

A high-performance Vulkan implementation of 3D Gaussian Splatting for real-time neural rendering.

## Development Status

**Currently in active development.** Core components implemented:

- ✅ PLY file loading 
- ✅ Vulkan context and buffer management  
- ✅ GPU memory allocation and staging buffers
- ✅ Compute pipeline and descriptor sets (in progress)
- ⏳ 3DGS compute shaders (projection, sorting, rasterization)
- ⏳ Real-time rendering loop

## Goals

- Pure compute shader implementation (no graphics pipeline)
- Real-time rendering of large point clouds
- Direct swapchain output from compute shaders
- Cross-platform Vulkan support

## Tech Stack

- **Graphics API**: Vulkan 1.3
- **Language**: C++17
- **Dependencies**: GLM, Vulkan SDK
- **Target**: Windows/Linux

## Current Capabilities

Successfully loads and uploads Gaussian point cloud data to GPU memory with optimized staging buffer pipeline.

---

*More documentation and usage instructions coming as development progresses.*
