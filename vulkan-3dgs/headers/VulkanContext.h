#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

  struct {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
  } _vcxMainDevice;

  VkQueue _vcxGraphicsQueue;
  VkQueue _vcxPresentationQueue;
  VkSurfaceKHR _vcxSurface;

  // create Functions
  void CreateInstance();

  // support
  bool
  CheckInstanceExtensionSupport(std::vector<const char *> *checkExtensions);
};