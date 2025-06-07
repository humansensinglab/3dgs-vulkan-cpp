// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

struct StagingRead {
  VkBuffer staging;
  void *mem;
};

struct FrameTimer {
  std::chrono::steady_clock::time_point lastTime;
  double fps;
  double deltaTime;

  FrameTimer()
      : lastTime(std::chrono::steady_clock::now()), fps(0.0), deltaTime(0.0) {}

  void UpdateTime() {
    auto currentTime = std::chrono::steady_clock::now();
    deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
    lastTime = currentTime;

    if (deltaTime > 0.0) {
      fps = 1.0 / deltaTime;
    }
  }
  void PrintStats() const {
    std::cout << "FPS: " << fps << " | Frame Time: " << deltaTime * 1000.0
              << "ms" << std::endl;
  }
};

struct InputArgs {
  std::string ply;
  int w;
  int h;
};
constexpr int AVG_GAUSS_TILE = 4;

struct GaussianBuffers {
  VkBuffer xyz;
  VkBuffer scales;
  VkBuffer rotations;
  VkBuffer opacity;
  VkBuffer sh;
  VkBuffer camUniform;
  VkBuffer radii;
  VkBuffer depth;
  VkBuffer color;
  VkBuffer conicOpacity;
  VkBuffer points2d;
  VkBuffer tilesTouched;
  VkBuffer tilesTouchedPrefixSum;
  VkBuffer boundingBox;
  StagingRead numRendered;
  VkBuffer keys;
  VkBuffer valuesRadix;
  VkBuffer keysRadix;
  VkBuffer values;
  VkBuffer ranges;
  VkBuffer histogram;
};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

struct QueueFamilyIndices {
  int graphicsFamily = -1; // location
  int presentationFamily = -1;
  bool isValid() { return graphicsFamily >= 0 && presentationFamily >= 0; }
};

struct SwapChainDetails {
  VkSurfaceCapabilitiesKHR surfaceCapabilities;    // surface properties
  std::vector<VkSurfaceFormatKHR> imageFormat;     // RGB, HSV...
  std::vector<VkPresentModeKHR> presentationsMode; // presentationMode
};

struct SwapChainImage {
  VkImage image;
  VkImageView imageView;
};

static std::vector<char> ReadFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    throw std::runtime_error("Failed opening file!");
  }
  size_t fileSize = (size_t)file.tellg();
  std::vector<char> fileBuffer(fileSize);
  file.seekg(0);

  file.read(fileBuffer.data(), fileSize);

  file.close();

  return fileBuffer;
}

static std::optional<InputArgs> checkArgs(int argc, char *argv[]) {
  if (argc != 2 && argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " <pointcloud_file> (<width> <height>)-optional-" << std::endl;
    std::cerr << "Example: " << argv[0] << " data/scene.ply 1200 800"
              << std::endl;
    return std::nullopt;
  }

  std::string pointcloudPath = argv[1];

  if (!std::filesystem::exists(pointcloudPath)) {
    std::cerr << "Error: File '" << pointcloudPath << "' does not exist!"
              << std::endl;
    return std::nullopt;
  }
  int w = 1200;
  int h = 800;

  if (argc > 2) {
    w = atoi(argv[2]);
    h = atoi(argv[3]);
    if (w < 500 && w > 3200 && h < 400 && h > 2500) {
      std::cerr << "Error: Width & Hight dimensions exceed the expected range"
                << std::endl;
      return std::nullopt;
    }
  }

  return InputArgs{pointcloudPath, w, h};
}