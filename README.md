# Vulkan 3D Gaussian Splatting

Cross-platform 3D Gaussian Splatting implementation using **pure Vulkan compute shaders** — no CUDA dependencies.

![Current State](media/preprocess.gif)
*Real-time preprocessing of 1.5M Gaussians with interactive camera*

---

Example ply : https://huggingface.co/datasets/dylanebert/3dgs/tree/main/bonsai/point_cloud/iteration_30000

## 🚀 Why Vulkan?

Most 3DGS implementations rely on CUDA, locking them to NVIDIA GPUs. This project uses **standard Vulkan 1.3**, ensuring:

- ✅ **Cross-platform**: Runs on Windows, Linux, and macOS
- ✅ **Any GPU vendor**: NVIDIA, AMD, Intel, Apple Silicon
- ✅ **No vendor lock-in**: Fully open, portable GPU compute
- ✅ **Cloud-ready**: Compatible with any Vulkan-capable infrastructure

---

## 🛠️ Features

- Load large 3D Gaussian point clouds (`.ply`)
- Real-time camera control (WASD + mouse)
- GPU-accelerated preprocessing with compute shaders
- View-dependent color (Spherical Harmonics, degrees 0–3)
- Full Vulkan context with memory & descriptor management
- Efficient buffer handling with persistent mappings
- Cross-vendor rendering pipeline

---

## ✅ Completed

- **PLY Loading** — Gaussians: position, scale, rotation, opacity, SH coeffs
- **Vulkan Context** — Logical/physical device, compute queues
- **Buffer Management** — Persistent mapped staging & storage buffers
- **Compute Pipelines** — Descriptor sets, shaders, command recording
- **Camera System** — First-person control, matrix updates
- **Preprocessing** — Frustum culling, 3D→2D covariance, NDC projection
- **Spherical Harmonics** — Degrees 0–3, view-dependent lighting
- **Screen Projection** — Pixel transform + radius estimation
- **Basic Rasterization** — First-hit splat to screen
- **Prefix Sum** — Tile count → offset mapping
- **View-dependent Depth Sort** — Radix sort with dynamic buffers (ongoing)

---

## 🔄 In Progress

- **Tile-Based Rasterization** — Efficient per-tile Gaussian rendering
- **Alpha Blending** — Correct order-independent transparency

---

## ⏳ Planned

- **Per-tile Binning Optimization** — Reduce overdraw, improve occupancy
- **Memory Pooling** — Avoid buffer reallocation on resize
- **Performance Optimization** — Final tuning to achieve stable 60+ FPS
- **Optional UI** — Camera debug, toggles for culling/blending/etc.

---

## 🧰 Tech Stack

- **API**: Vulkan 1.3
- **Language**: C++17
- **Shaders**: GLSL (compiled to SPIR-V)
- **Math**: [GLM](https://github.com/g-truc/glm)
- **Platform**: Windows / Linux / macOS
- **Build System**: CMake

---

## 📂 Current Capabilities

- Load multi-million point Gaussian clouds
- Real-time camera and transform updates
- GPU-based preprocessing: culling, sorting, projection, SH lighting
- Depth-aware splat sorting using radix sort
- Dynamic buffer resizing and memory-safe descriptor updates
- Pure Vulkan rendering to screen (no graphics pipeline yet)

---

## 🧭 Project Goal

> **Democratize 3D Gaussian Splatting beyond NVIDIA hardware.**

Make high-performance splatting available on **any GPU**, on **any platform**, fully in the **open**, without CUDA lock-in.

---

