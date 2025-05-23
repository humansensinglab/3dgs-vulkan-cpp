#pragma once

#include "utils.h"
#include <iostream>
#include <vector>

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

  // get Functions
  void GetPhysicalDevice();
  QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device);

  // create Functions
  void CreateInstance();
  void CreateSurface();

  // support
  bool
  CheckInstanceExtensionSupport(std::vector<const char *> *checkExtensions);
  bool CheckDeviceSuitable(VkPhysicalDevice device);
};