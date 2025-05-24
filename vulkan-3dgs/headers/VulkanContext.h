#pragma once

#include "utils.h"
#include <iostream>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class VulkanContext {
public:
  VulkanContext(GLFWwindow *newWindow) : _vcxWindow(newWindow){};

  int InitContext();

  void CleanUp();

  ~VulkanContext();

private:
  GLFWwindow *_vcxWindow;
  VkInstance _vcxInstance;

  VkSurfaceKHR _vcxSurface;

  struct {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
  } _vcxMainDevice;

  VkQueue _vcxGraphicsQueue;
  VkQueue _vcxPresentationQueue;

  VkSwapchainKHR _vcxSwapchain;
  std::vector<SwapChainImage> _vcxImages;

  VkFormat _vcxSwapChainFormat;
  VkExtent2D _vcxSwapChainExtent2D;

  // get Functions
  void GetPhysicalDevice();
  QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device);
  SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device);

  // create Functions
  void CreateInstance();
  void CreateSurface();
  void CreateLogicalDevice();
  void CreateSwapChain();

  // support
  bool
  CheckInstanceExtensionSupport(std::vector<const char *> *checkExtensions);
  bool CheckDeviceSuitable(VkPhysicalDevice device);

  // choose for Swapchain
  VkSurfaceFormatKHR
  ChooseBestFormatSurface(const std::vector<VkSurfaceFormatKHR> &formats);
  VkPresentModeKHR
  ChooseBestPresentMode(const std::vector<VkPresentModeKHR> &presentModes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  // helper cerate
  VkImageView CreateImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags);
  VkShaderModule CreateShaderModule(const std::vector<char> &code);
};