# Vulkan 3D Gaussian Splatting

Cross-platform 3D Gaussian Splatting implementation using **pure Vulkan compute shaders** — no CUDA dependencies.

![Current State](media/gaussian_record.gif)
*Real-time rendering with keyframe animation system and configurable parameters on NVIDIA RTX 3070*

---

Example ply : https://huggingface.co/datasets/dylanebert/3dgs/tree/main/bonsai/point_cloud/iteration_30000

## 🚀 Why Vulkan?

Most 3DGS implementations rely on CUDA, locking them to NVIDIA GPUs. This project uses **standard Vulkan 1.3**, ensuring:

- ✅ **Cross-platform**: Runs on Windows, Linux, and macOS
- ✅ **Any GPU vendor**: NVIDIA, AMD, Intel, Apple Silicon (still not working on Mac, need to activate certain extensions)
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

### 🎬 NEW: Keyframe Animation System

**Professional Camera Animation**:
- 📹 **Keyframe Recording**: Capture camera positions, rotations, and rendering parameters at specific time points
- 🎯 **Smooth Interpolation**: Automatic interpolation between keyframes for cinematic camera movements
- ⏯️ **Playback Control**: Play, stop, and loop through your animation sequences
- 📊 **Parameter Animation**: Animate FOV, near/far planes, wireframe mode, and more
- 🎞️ **Video Export Ready**: Fixed timestep rendering for consistent video output

### 🎛️ Configurable Parameters

**Real-time Controls via ImGui**:
- **Camera Settings**:
  - FOV adjustment (30° - 120°)
  - Mouse sensitivity (0.01 - 0.4)
  - Movement speed (1.0 - 10.0)
  - Free-fly or look-at modes
  
- **Rendering Options**:
  - Distance culling with adjustable near/far planes
  - Wireframe mode with Gaussian scale control
  - Real-time performance metrics (FPS, GPU memory)
  - Tile size configuration
  
- **Animation Keyframes Include**:
  - Camera position (X, Y, Z)
  - Camera rotation (Yaw, Pitch)
  - Field of View
  - Near/Far culling planes
  - Wireframe mode toggle
  - Timing control

---

## ✅ Core Implementation Complete

**Full 3D Gaussian Splatting Pipeline**:
- **PLY Loading** — Multi-million Gaussian support: position, scale, rotation, opacity, SH coefficients
- **Vulkan Foundation** — Complete device/queue setup with cross-platform compatibility
- **Buffer Architecture** — Persistent mapped staging & storage buffers with dynamic resizing
- **Compute Infrastructure** — Descriptor management, pipeline creation, command recording
- **Camera System** — Smooth first-person controls with real-time matrix updates + keyframe animation
- **GPU Preprocessing** — Frustum culling, 3D→2D covariance, NDC projection
- **Spherical Harmonics** — Full SH evaluation (degrees 0–3) for view-dependent lighting
- **Screen Projection** — Accurate pixel transforms with radius estimation
- **Prefix Sum** — Efficient tile count to offset mapping
- **Depth Sorting** — GPU radix sort for proper depth ordering
- **Tile-Based Rasterization** — Complete per-tile Gaussian binning and rendering
- **Alpha Blending** — Order-independent transparency with correct compositing
- **ImGui Integration** — Professional UI for real-time parameter control and animation

**The core 3DGS algorithm is fully functional and production-ready.**

## Building from Source

### Prerequisites
- Vulkan SDK 1.3+ installed
- Visual Studio 2022 (Windows) or GCC/Clang (Linux/macOS)
- CMake 3.10+
- Compatible GPU with Vulkan support

### Step 1: Clone the Repository
```bash
git clone https://github.com/AlejandroAmat/3dgs-vulkan-cpp.git
cd 3dgs-vulkan-cpp
```

### Step 2: Create Build Directory
```bash
mkdir build
cd build
```

### Step 3: Configure with CMake
```bash
cmake ..
```

For Visual Studio users, you can specify the generator:
```bash
cmake -G "Visual Studio 17 2022" ..
```

### Step 4: Build the Project
```bash
# For Release build (recommended)
cmake --build . --config Release

# For Debug build
cmake --build . --config Debug
```

Alternatively, if you generated Visual Studio files:
- Open `build/vulkan-3dgs.sln`
- Set `vulkan-3dgs` as the startup project
- Press F5 to build and run

### Step 5: Locate the Executable
The executable will be created in:
- **Windows**: `build/vulkan-3dgs/Release/vulkan-3dgs.exe` (or `Debug/`)
- **Linux/macOS**: `build/vulkan-3dgs/vulkan-3dgs`

### Step 6: Run the Application
```bash
# Navigate to the executable directory
cd vulkan-3dgs/Release

# Run with a PLY file
./vulkan-3dgs.exe path/to/your/pointcloud.ply
```

### Directory Structure After Build
```
build/
└── vulkan-3dgs/
    └── Release/
        ├── vulkan-3dgs.exe
        ├── Shaders/
        │   ├── preprocess.spv
        │   ├── render.spv
        │   └── ... (other shader files)
        └── (place your .ply file here or specify full path)
```

### Troubleshooting Build Issues

**CMake can't find Vulkan:**
- Ensure Vulkan SDK is installed
- Check that `VULKAN_SDK` environment variable is set
- On Windows: `echo %VULKAN_SDK%`
- On Linux/macOS: `echo $VULKAN_SDK`

**Shader files not found:**
- The build process automatically copies `.spv` files from `src/Shaders/`
- Ensure shader files exist in the source directory

**Link errors with GLFW:**
- The project uses precompiled GLFW libraries in `third-party/GLFW/lib-vc2022`
- For other compilers, you may need to rebuild GLFW

---

## 🎮 Usage & Controls

### Camera Controls
- **WASD** - Move forward/backward/left/right
- **Q/E** - Move up/down
- **Right Mouse + Drag** - Look around
- **Mouse Scroll** - Adjust FOV

### Animation Workflow
1. **Enter Sequence Mode** - Click "Enter Sequence" in the Camera Sequence window
2. **Position Camera** - Navigate to desired viewpoint
3. **Add Keyframe** - Captures current camera state and parameters
4. **Repeat** - Add multiple keyframes at different times
5. **Play** - Watch smooth interpolated animation
6. **Export** - Record to video with consistent framerate

### ImGui Interface
- **Performance Panel** - Real-time FPS, frame time, GPU memory usage
- **Camera Panel** - Position, rotation, FOV, speed controls
- **Rendering Panel** - Culling, wireframe, tile size options
- **Sequence Panel** - Keyframe management and playback

---

## 🔄 Optimization & Polish

- **Shared Memory Refinement** — The `SHARED_MEM_RENDER` compile-time option uses workgroup shared memory for significantly faster rendering, but has small tile boundary artifacts due to GLSL limitations. Working to eliminate these visual issues while preserving the performance gains.
- **Performance Tuning** — Achieving consistent 30+ FPS across all hardware configurations

---

## ⏳ Roadmap

### Platform Expansion  
- **Video Export** — Direct MP4/AVI export from animation sequences
- **Cross-Platform Build** — Universal CMake configuration for all OS
- **Mobile Support** — Native Android app + iOS via MoltenVK

### User Experience
- **Advanced Animation** — Bezier curves, easing functions, path preview
- **Drag & Drop** — Direct PLY file loading from desktop
- **Multi-PLY Support** — Load and switch between multiple models

---

## 🧰 Tech Stack

- **Graphics API**: Vulkan 1.3 (compute + graphics)
- **Language**: C++17 with modern practices
- **Shaders**: GLSL → SPIR-V compilation
- **Math Library**: [GLM](https://github.com/g-truc/glm) for transforms
- **UI Framework**: [Dear ImGui](https://github.com/ocornut/imgui) for controls
- **Build System**: CMake (cross-platform)
- **Windowing**: [GLFW](https://github.com/glfw/glfw) for cross-platform support
- **Radix Sort**: [VkRadixSort](https://github.com/MircoWerner/VkRadixSort) by MircoWerner for GPU depth sorting
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