#pragma once

#define GLFW_INCLUDE_VULKAN
#include "VulkanContext.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <vector>

struct DescriptorBinding {
  uint32_t binding;
  VkDescriptorType type;
  VkShaderStageFlags stageFlags;
  uint32_t count = 1;
};

class ComputePipeline {
public:
  ComputePipeline(VulkanContext &vkContext) : _vkContext(vkContext){};
  ~ComputePipeline(){};

  void Initialize();
  /* void RenderFrame();
   void CleanUp();*/

private:
  VulkanContext &_vkContext;

  std::vector<VkCommandBuffer> _commandBuffers;
  std::vector<VkFence> _fences;
  std::vector<VkSemaphore> _semaphores;

  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> descriptorSets;

  uint32_t currentFrame = 0;

  void CreateCommandBuffers();
  void CreateSynchronization();
  void CreateDescriptorSetLayout();
  void CreateDescriptorPool();

  const std::vector<DescriptorBinding> PIPELINE_BINDINGS = {
      {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
       VK_SHADER_STAGE_COMPUTE_BIT}, // Output image
                                     //{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      // VK_SHADER_STAGE_COMPUTE_BIT}, // Camera data
      //{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      // VK_SHADER_STAGE_COMPUTE_BIT}, // Gaussian positions
      //{3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      // VK_SHADER_STAGE_COMPUTE_BIT}, // Gaussian scales
      //{4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      // VK_SHADER_STAGE_COMPUTE_BIT}, // Gaussian rotations
      //{5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      // VK_SHADER_STAGE_COMPUTE_BIT}, // Gaussian opacity
      //{6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      // VK_SHADER_STAGE_COMPUTE_BIT}, // Gaussian SH data
  };
};