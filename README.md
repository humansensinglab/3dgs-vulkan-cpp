# Vulkan 3D Gaussian Splatting

Cross-platform 3D Gaussian Splatting implementation using **pure Vulkan compute shaders** — no CUDA dependencies.

![Current State](media/gaussian.gif)
*Real-time rendering of 2.6M Gaussians at 1800x1600 resolution (30-60 FPS) on Nvidia 3070*

---

## 🚀 Why Vulkan?

Most 3DGS implementations rely on CUDA, locking them to NVIDIA GPUs. This project uses **standard Vulkan 1.3**, ensuring:

- ✅ **Cross-platform**: Runs on Windows, Linux, and macOS
- ✅ **Any GPU vendor**: NVIDIA, AMD, Intel, Apple Silicon  
- ✅ **No vendor lock-in**: Fully open, portable GPU compute
- ✅ **Cloud-ready**: Compatible with any Vulkan-capable infrastructure

---

## 🛠️ Features

- **High Performance**: 30-60 FPS on 2.6M+ Gaussians at 1800x1600 resolution
- **Pure Vulkan**: Cross-vendor compute pipeline with no CUDA dependencies
- **Real-time Interaction**: WASD + mouse camera control with immediate feedback
- **Advanced Rendering**: View-dependent Spherical Harmonics (degrees 0–3)
- **Smart Memory Management**: Persistent buffer mappings and dynamic resizing
- **Flexible Architecture**: Compile-time shared memory optimization options

---

## ✅ Core Implementation Complete

**Full 3D Gaussian Splatting Pipeline**:
- **PLY Loading** — Multi-million Gaussian support: position, scale, rotation, opacity, SH coefficients
- **Vulkan Foundation** — Complete device/queue setup with cross-platform compatibility
- **Buffer Architecture** — Persistent mapped staging & storage buffers with dynamic resizing
- **Compute Infrastructure** — Descriptor management, pipeline creation, command recording
- **Camera System** — Smooth first-person controls with real-time matrix updates
- **GPU Preprocessing** — Frustum culling, 3D→2D covariance, NDC projection
- **Spherical Harmonics** — Full SH evaluation (degrees 0–3) for view-dependent lighting
- **Screen Projection** — Accurate pixel transforms with radius estimation
- **Prefix Sum** — Efficient tile count to offset mapping
- **Depth Sorting** — GPU radix sort for proper depth ordering
- **Tile-Based Rasterization** — Complete per-tile Gaussian binning and rendering
- **Alpha Blending** — Order-independent transparency with correct compositing


**The core 3DGS algorithm is fully functional and production-ready.**

---

## 🔄 Optimization & Polish

- **Shared Memory Refinement** — The `SHARED_MEM_RENDER` compile-time option uses workgroup shared memory for significantly faster rendering, but has small tile boundary artifacts due to GLSL limitations. Working to eliminate these visual issues while preserving the performance gains.
- **Performance Tuning** — Achieving consistent 30+ FPS across all hardware configurations

---

## ⏳ Roadmap

### Platform Expansion  
- **Animation System** — Gaussian sequence loading and playback
- **Cross-Platform Build** — Universal CMake configuration for all OS
- **Mobile Support** — Native Android app + iOS via MoltenVK

### User Experience
- **ImGui Integration** — Real-time controls and debugging interface
- **Drag & Drop** — Direct PLY file loading from desktop
- **Export Features** — Render to video/image sequences

---

## 🧰 Tech Stack

- **Graphics API**: Vulkan 1.3 (compute + graphics)
- **Language**: C++17 with modern practices
- **Shaders**: GLSL → SPIR-V compilation
- **Math Library**: [GLM](https://github.com/g-truc/glm) for transforms
- **Build System**: CMake (cross-platform)
- **Build System**: [GLFW](https://github.com/glfw/glfw)) for cross-platform windowing
- **Radix Sort:** [VkRadixSort](https://github.com/MircoWerner/VkRadixSort) by MicroWerner for GPU depth sorting
- **Platforms**: Windows, Linux, macOS (mobile planned)

---

## 🎯 Performance

**Current Metrics** (2.6M Gaussians at 1800x1600):
- **Framerate**: 30-60 FPS with full 3DGS pipeline active
- **Memory**: Efficient GPU buffer management with minimal CPU overhead
- **Scalability**: Handles multi-million point clouds with dynamic LOD

**Optimization Modes**:
- `SHARED_MEM_RENDER`: Faster rendering with workgroup shared memory (compile-time option)
- Standard mode: Stable rendering without shared memory artifacts

---

## 🧭 Project Vision

> **Democratize 3D Gaussian Splatting beyond NVIDIA hardware.**

Make high-performance neural radiance field rendering accessible on **any GPU**, **any platform**, fully **open source**, without proprietary dependencies.


**Requirements**:
- Vulkan 1.3 compatible GPU and drivers
- C++17 compiler
- CMake 3.16+
- Vulkan SDK

---

## 🤝 Contributing

This project aims to advance open 3DGS rendering. Contributions welcome for:
- Performance optimizations
- Platform-specific improvements  
- New rendering features
- Mobile platform support

---

## 📄 License

MIT 
