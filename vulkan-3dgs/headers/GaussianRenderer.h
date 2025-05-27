#pragma once

#include "BufferManager.h"
#include "Camera.h"
#include "ComputePipeline.h"
#include "GaussianBase.h"
class GaussianRenderer {

public:
  GaussianRenderer(VulkanContext &vulkanContext, int shDegree)
      : _vulkanContext(vulkanContext), _bufferManager(),
        _computePipeline(vulkanContext) {

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

  void CreateGaussianBuffers();
  void CreatePipelineStorageBuffers();
  template <typename T>
  void CreateAndUploadBuffer(VkBuffer &buffer, const void *data,
                             std::string type, int offset = 1);

  template <typename T>
  void CreateWriteBuffers(VkBuffer &buffer, std::string type, int offset = 1,
                          int dst = false);
  void CreateUniformBuffer();
  void CreateCopyStagingBuffer();

  GaussianBuffers _buffers;
  std::unique_ptr<Camera> _camera;
  int _shDegree;
  int _nGauss;

  void *_cameraUniformMapped = nullptr;
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

template <typename T>
inline void GaussianRenderer::CreateWriteBuffers(VkBuffer &buffer,
                                                 std::string type, int offset,
                                                 int dst) {
  VkDeviceSize bufferSize = sizeof(T) * _nGauss * offset;
  std::cout << " Creating " << type << " buffer : " << bufferSize << " bytes "
            << std::endl;
  VkPhysicalDevice physicalDevice = _vulkanContext.GetPhysicalDevice();
  VkDevice device = _vulkanContext.GetLogicalDevice();
  VkBufferUsageFlags usage = (dst == true)
                                 ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                                 : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  buffer =
      _bufferManager.CreateBuffer(device, physicalDevice, bufferSize, usage,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}