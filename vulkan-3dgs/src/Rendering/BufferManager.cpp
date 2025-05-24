#include "BufferManager.h"

VkBuffer BufferManager::CreateBuffer(VkDevice device,
                                     VkPhysicalDevice physicalDevice,
                                     VkDeviceSize size,
                                     VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags properties) {
  VkBuffer buffer;
  VkDeviceMemory memory;

  CreateBufferInternal(device, physicalDevice, size, usage, properties, buffer,
                       memory);

  _buffers.push_back({buffer, memory, size});

  return buffer;
}

VkBuffer BufferManager::CreateStorageBuffer(VkDevice device,
                                            VkPhysicalDevice physicalDevice,
                                            VkDeviceSize size) {
  return CreateBuffer(device, physicalDevice, size,
                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

VkBuffer BufferManager::CreateUniformBuffer(VkDevice device,
                                            VkPhysicalDevice physicalDevice,
                                            VkDeviceSize size) {
  return CreateBuffer(device, physicalDevice, size,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void BufferManager::CreateBufferInternal(
    VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
    VkBuffer &buffer, VkDeviceMemory &bufferMemory) {

  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    throw std::runtime_error("Fail creating Buffer");

  VkMemoryRequirements memReq = {};
  vkGetBufferMemoryRequirements(device, buffer, &memReq);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.allocationSize = memReq.size;
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.memoryTypeIndex =
      FindMemoryType(physicalDevice, memReq.memoryTypeBits, properties);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS)
    throw std::runtime_error("Failed to allocate VB memory");

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t BufferManager::FindMemoryType(VkPhysicalDevice physicalDevice,
                                       uint32_t typeFilter,
                                       VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type!");
}

void BufferManager::copyBuffer(VkDevice device, VkDeviceSize deviceSize,
                               VkBuffer srcBuffer, VkBuffer dstBuffer,
                               VkCommandPool commandPool,
                               VkQueue commandQueue) {

  VkCommandBuffer commandBuffer;
  VkCommandBufferAllocateInfo allocateInfo = {};
  allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocateInfo.commandPool = commandPool;
  allocateInfo.commandBufferCount = 1;

  vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VkBufferCopy copy = {};
  copy.dstOffset = 0;
  copy.srcOffset = 0;
  copy.size = deviceSize;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copy);
  vkEndCommandBuffer(commandBuffer);

  // queue submission

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(commandQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(commandQueue);

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkDeviceMemory BufferManager::GetBufferMemory(VkBuffer buffer) {
  for (const auto &bufferInfo : _buffers) {
    if (bufferInfo.buffer == buffer) {
      return bufferInfo.memory;
    }
  }
  throw std::runtime_error("Buffer memory not found!");
}

void BufferManager::DestroyBuffer(VkDevice device, VkBuffer buffer) {

  for (auto it = _buffers.begin(); it != _buffers.end(); ++it) {
    if (it->buffer == buffer) {
      vkDestroyBuffer(device, it->buffer, nullptr);
      vkFreeMemory(device, it->memory, nullptr);
      _buffers.erase(it);
      return;
    }
  }
}

void BufferManager::CleanupAllBuffers(VkDevice device) {
  for (const auto &bufferInfo : _buffers) {
    vkDestroyBuffer(device, bufferInfo.buffer, nullptr);
    vkFreeMemory(device, bufferInfo.memory, nullptr);
  }

  _buffers.clear();
  std::cout << " All buffers cleaned up" << std::endl;
}