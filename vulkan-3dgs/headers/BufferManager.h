// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

class BufferManager {
public:
  BufferManager(){};
  ~BufferManager(){};

  VkBuffer CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                        VkDeviceSize size, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties);

  VkBuffer CreateStorageBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                               VkDeviceSize size);

  VkBuffer CreateUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                               VkDeviceSize size);
  void CleanupAllBuffers(VkDevice device);
  void DestroyBuffer(VkDevice device, VkBuffer buffer);
  void copyBuffer(VkDevice device, VkDeviceSize deviceSize, VkBuffer srcBuffer,
                  VkBuffer dstBuffer, VkCommandPool transferPool,
                  VkQueue transferQueue);

  VkDeviceMemory GetBufferMemory(VkBuffer buffer);

private:
  void CreateBufferInternal(VkDevice device, VkPhysicalDevice physicalDevice,
                            VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkBuffer &buffer,
                            VkDeviceMemory &bufferMemory);

  struct BufferInfo {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;
  };

  std::vector<BufferInfo> _buffers;

  uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};
