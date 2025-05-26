# Vulkan 3D Gaussian Splatting

Cross-platform 3D Gaussian Splatting implementation using pure Vulkan compute shaders. No CUDA dependencies.

## Progress Status

### ✅ Completed
- **PLY loading** - Parse Gaussian point clouds (positions, scales, rotations, opacity, SH coefficients)
- **Vulkan context** - Device setup, compute queues, memory allocation
- **Buffer management** - Staging buffers, storage buffers, uniform buffers with persistent mapping
- **Compute pipelines** - Shader compilation, descriptor sets, command buffer recording
- **Camera system** - Real-time FPS controls with uniform buffer updates
- **Preprocessing pipeline** - Frustum culling, 3D→2D covariance, NDC projection (900+ FPS)
- **Spherical harmonics** - View-dependent color computation (degrees 0-3)
- **Screen projection** - Transform Gaussians to pixel coordinates with radius calculation
- **Basic rasterization** - First-hit rendering with proper color output

### 🔄 In Progress  
- **Prefix sum** - Convert tile counts to offsets
- **Radix sort** - Depth sorting for proper blending
- **Tile-based rasterization** - Efficient final rendering

### ⏳ Planned
- **Alpha blending** - Proper transparency 
- **Performance optimization** - Target 60+ FPS full pipeline

![Current State](Media/preprocess.gif)

*Real-time preprocessing of 1.5M Gaussians with camera movement*

## Why Vulkan?

**Problem**: Most 3DGS implementations require CUDA (NVIDIA-only)  
**Solution**: Pure Vulkan implementation runs on any GPU

- ✅ **Cross-platform**: Windows, Linux, macOS
- ✅ **Any GPU vendor**: NVIDIA, AMD, Intel, Apple
- ✅ **No vendor lock-in**: Standard Vulkan API
- ✅ **Cloud deployment**: Works anywhere with Vulkan drivers

## Performance

Current preprocessing: **900+ FPS** on 1.5M Gaussians  
Target full pipeline: **60+ FPS** 

## Tech Stack

- **API**: Vulkan 1.3
- **Language**: C++17
- **Math**: GLM
- **Platform**: Windows/Linux/macOS

## Current Capabilities

- Load PLY point clouds with millions of Gaussians
- Real-time camera navigation (WASD + mouse)
- GPU preprocessing: frustum culling, covariance math, SH evaluation
- Direct compute shader rendering to screen
- Cross-platform compatibility

---

**Goal**: Democratize 3D Gaussian Splatting beyond NVIDIA hardware
