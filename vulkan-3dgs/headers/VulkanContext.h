// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#pragma once

#include "utils.h"
#include <iostream>

#ifdef DEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class VulkanContext {
public:
  VulkanContext(GLFWwindow *newWindow) : _vcxWindow(newWindow){};

  int InitContext();

  void CleanUp();

  VkDevice GetLogicalDevice() const { return _vcxMainDevice.logicalDevice; }
  VkPhysicalDevice GetPhysicalDevice() const {
    return _vcxMainDevice.physicalDevice;
  }

  VkQueue GetGraphicsQueue() const { return _vcxGraphicsQueue; }
  VkQueue GetPresentationQueue() const { return _vcxPresentationQueue; }
  uint32_t GetGraphicsFamily() {
    return GetQueueFamilies(_vcxMainDevice.physicalDevice).graphicsFamily;
  }

  VkInstance GetInstance() const { return _vcxInstance; }
  VkSwapchainKHR GetSwapchain() const { return _vcxSwapchain; }
  VkFormat GetSwapchainFormat() const { return _vcxSwapChainFormat; }
  const VkExtent2D &GetSwapchainExtent() const { return _vcxSwapChainExtent2D; }
  std::vector<SwapChainImage> &GetSwapchainImages() { return _vcxImages; }
  int32_t SwapchainSize() const { return (int32_t)_vcxImages.size(); }

  VkCommandPool GetCommandPool() const { return _vcxCommandPool; }

  ~VulkanContext();

  GLFWwindow *getWindow() const { return _vcxWindow; }

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

  VkCommandPool _vcxCommandPool;

  // get Functions
  void GetPhysicalDeviceInternal();
  QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device);
  SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device);

  // create Functions
  void CreateInstance();
  void CreateSurface();
  void CreateLogicalDevice();
  void CreateSwapChain();
  void CreateCommandPool();

  // support
  bool
  CheckInstanceExtensionSupport(std::vector<const char *> *checkExtensions);
  bool CheckDeviceSuitable(VkPhysicalDevice device);
  bool CheckDeviceExtensionSupport(
      VkPhysicalDevice device); // swapchain compatibility is checked on
                                // physical device level
  bool CheckValidationLayerSupport();

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

  // Debugging::

  VkDebugUtilsMessengerEXT _debugMessenger;
  void SetupDebugMessenger();
  void PopulateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {

    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE; // Return false to let Vulkan continue
  }

  // Function to create the debug messenger
  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkDebugUtilsMessengerEXT *pDebugMessenger);

  // Function to destroy the debug messenger
  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debugMessenger,
                                     const VkAllocationCallbacks *pAllocator);
};
