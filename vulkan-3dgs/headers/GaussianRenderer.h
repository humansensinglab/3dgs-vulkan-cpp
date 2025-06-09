// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#pragma once

#include <memory>

#include "BufferManager.h"
#include "Camera.h"
#include "ComputePipeline.h"
#include "GaussianBase.h"
#include "GraphicsPipeline.h"
#include "Imgui3DGS.h"
#include "RenderSettings.h"
#include "Sequence.h"

class GaussianRenderer {
 public:
  GaussianRenderer(VulkanContext &vulkanContext, int shDegree,
                   Sequence &seqRecorder)
      : _vulkanContext(vulkanContext),
        _bufferManager(),
        _imguiHandler(vulkanContext, seqRecorder),
        _graphcsPipeline(_vulkanContext),
        _computePipeline(vulkanContext, _imguiHandler, _graphcsPipeline),
        _shDegree(shDegree) {
    std::cout << "GaussianRenderer created" << std::endl;
  };

  ~GaussianRenderer();

  void LoadGaussianData(std::unique_ptr<GaussianBase> gaussianData);
  // void RenderFrame();

  bool IsInitialized() const { return _gaussianData != nullptr; }
  size_t GetGaussianCount() const {
    return _gaussianData ? _gaussianData->GetCount() : 0;
  }

  void InitComputePipeline();
  void CreateBuffers();
  void Render();

  void InitializeCamera(float windowWidth, float windowHeight);

  static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
  void processInput(float deltaTime);
  void UpdateCameraUniforms();

 private:
  VulkanContext &_vulkanContext;
  BufferManager _bufferManager;
  std::unique_ptr<GaussianBase> _gaussianData;
  ComputePipeline _computePipeline;
  ImguiUI _imguiHandler;
  GraphicsPipeline _graphcsPipeline;

  void CreateGaussianBuffers();
  void CreatePipelineStorageBuffers();
  template <typename T>
  void CreateAndUploadBuffer(VkBuffer &buffer, const void *data,
                             std::string type, int offset = 1);

  template <typename T>
  void CreateWriteBuffers(VkBuffer &buffer, std::string type, int offset = 1,
                          bool dst = false);
  void CreateUniformBuffer();
  void CreateCopyStagingBuffer();
  void CreateRangesBuffer();
  GaussianBuffers _buffers;
  std::shared_ptr<Camera> _camera;
  int _shDegree;
  uint32_t _nGauss;
  void *_cameraUniformMapped = nullptr;
};

template <typename T>
inline void GaussianRenderer::CreateAndUploadBuffer(VkBuffer &buffer,
                                                    const void *data,
                                                    std::string type,
                                                    int offset) {
  VkDevice device = _vulkanContext.GetLogicalDevice();
  VkPhysicalDevice physicalDevice = _vulkanContext.GetPhysicalDevice();

  VkDeviceSize bufferSize = _gaussianData->GetCount() * sizeof(T) * offset;
  std::cout << " Creating " << type << " buffer : " << bufferSize << " bytes "
            << std::endl;
  buffer =
      _bufferManager.CreateStorageBuffer(device, physicalDevice, bufferSize);

  VkBuffer stagingBuffer = _bufferManager.CreateBuffer(
      device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void *mappedMem;
  VkDeviceMemory deviceMem = _bufferManager.GetBufferMemory(stagingBuffer);
  vkMapMemory(device, deviceMem, 0, bufferSize, 0, &mappedMem);
  memcpy(mappedMem, data, static_cast<size_t>(bufferSize));
  vkUnmapMemory(device, deviceMem);

  _bufferManager.copyBuffer(device, bufferSize, stagingBuffer, buffer,
                            _vulkanContext.GetCommandPool(),
                            _vulkanContext.GetGraphicsQueue());

  _bufferManager.DestroyBuffer(device, stagingBuffer);

  std::cout << " buffer created!" << std::endl;
}

template <typename T>
inline void GaussianRenderer::CreateWriteBuffers(VkBuffer &buffer,
                                                 std::string type, int offset,
                                                 bool dst) {
  VkDeviceSize bufferSize = sizeof(T) * _nGauss * offset;
  std::cout << " Creating " << type << " buffer : " << bufferSize << " bytes "
            << std::endl;
  VkPhysicalDevice physicalDevice = _vulkanContext.GetPhysicalDevice();
  VkDevice device = _vulkanContext.GetLogicalDevice();
  VkBufferUsageFlags usage = (dst) ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                                   : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  buffer =
      _bufferManager.CreateBuffer(device, physicalDevice, bufferSize, usage,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}
