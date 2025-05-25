#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <fstream>
#include <set>
#include <vector>

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
