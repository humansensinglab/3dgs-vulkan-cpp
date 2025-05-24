#pragma once

#include "BufferManager.h"
#include "GaussianBase.h"
#include "VulkanContext.h"

class GaussianRenderer {

public:
  GaussianRenderer(VulkanContext &vulkanContext)
      : _vulkanContext(vulkanContext), _bufferManager() {
    std::cout << "GaussianRenderer created" << std::endl;
  };

  ~GaussianRenderer();

  void LoadGaussianData(std::unique_ptr<GaussianBase> gaussianData);
  // void RenderFrame();

  bool IsInitialized() const { return _gaussianData != nullptr; }
  size_t GetGaussianCount() const {
    return _gaussianData ? _gaussianData->GetCount() : 0;
  }

private:
  VulkanContext &_vulkanContext;
  BufferManager _bufferManager;
  std::unique_ptr<GaussianBase> _gaussianData;

  VkBuffer _xyzBuffer = VK_NULL_HANDLE;
  VkBuffer _scaleBuffer = VK_NULL_HANDLE;
  VkBuffer _rotationBuffer = VK_NULL_HANDLE;
  VkBuffer _opacityBuffer = VK_NULL_HANDLE;
  VkBuffer _shBuffer = VK_NULL_HANDLE;

  void CreateGaussianBuffers();
  template <typename T>
  void CreateAndUploadBuffer(VkBuffer &buffer, const void *data,
                             std::string type, int offset = 1);
};

template <typename T>
inline void
GaussianRenderer::CreateAndUploadBuffer(VkBuffer &buffer, const void *data,
                                        std::string type, int offset) {

  VkDevice device = _vulkanContext.GetLogicalDevice();
  VkPhysicalDevice physicalDevice = _vulkanContext.GetPhysicalDevice();

  VkDeviceSize bufferSize = _gaussianData->GetCount() * sizeof(T) * offset;
  std::cout << " Creating " << type << " buffer : " << bufferSize << " bytes "
            << std::endl;

  VkBuffer stagingBuffer = _bufferManager.CreateBuffer(
      device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void *mappedMem;
  VkDeviceMemory deviceMem = _bufferManager.GetBufferMemory(stagingBuffer);
  vkMapMemory(device, deviceMem, 0, bufferSize, 0, &mappedMem);
  memcpy(mappedMem, data, static_cast<size_t>(bufferSize));
  vkUnmapMemory(device, deviceMem);

  buffer =
      _bufferManager.CreateStorageBuffer(device, physicalDevice, bufferSize);

  _bufferManager.copyBuffer(device, bufferSize, stagingBuffer, buffer,
                            _vulkanContext.GetCommandPool(),
                            _vulkanContext.GetGraphicsQueue());

  _bufferManager.DestroyBuffer(device, stagingBuffer);

  std::cout << " buffer created!" << std::endl;
}
