#pragma once

#include <vulkan/vulkan.h>
//
#include "VulkanContext.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

class ImguiUI {
public:
  ImguiUI(VulkanContext vkContext) : _vkContext(vkContext){};
  void Init();

private:
  VulkanContext _vkContext;

  void CreateRenderPass();
  void createFrameBuffers();
  void CreateDescriporPool();
  void cleanUp();
};