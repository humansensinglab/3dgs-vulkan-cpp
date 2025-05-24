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
  void CreateBuffer(VkBuffer &buffer, std::string type, int offset = 1);
};

template <typename T>
inline void GaussianRenderer::CreateBuffer(VkBuffer &buffer, std::string type,
                                           int offset) {

  VkDeviceSize bufferSize = _gaussianData->GetCount() * sizeof(T) * offset;
  std::cout << " Creating " << type << " buffer : " << bufferSize << " bytes "
            << std::endl;

  buffer = _bufferManager.CreateStorageBuffer(
      _vulkanContext.GetLogicalDevice(), _vulkanContext.GetPhysicalDevice(),
      bufferSize);

  std::cout << " buffer created!" << std::endl;
}
