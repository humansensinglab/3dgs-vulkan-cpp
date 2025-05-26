#pragma once

#include "BufferManager.h"
#include "Camera.h"
#include "ComputePipeline.h"
#include "GaussianBase.h"
class GaussianRenderer {

public:
  GaussianRenderer(VulkanContext &vulkanContext)
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

  void render();

  void InitializeCamera(float windowWidth, float windowHeight);

  static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
  void processInput(float deltaTime);

  void printViewMat() {
    glm::mat4 m = _camera->GetViewMatrix();
    std::cout << "View Matrix:" << std::endl;
    for (int i = 0; i < 4; ++i) {
      std::cout << "[ ";
      for (int j = 0; j < 4; ++j) {
        std::cout << m[j][i];
        if (j < 3)
          std::cout << ", ";
      }
      std::cout << " ]" << std::endl;
    }
    std::cout << std::endl;
  }

private:
  VulkanContext &_vulkanContext;
  BufferManager _bufferManager;
  std::unique_ptr<GaussianBase> _gaussianData;
  ComputePipeline _computePipeline;

  VkBuffer _xyzBuffer = VK_NULL_HANDLE;
  VkBuffer _scaleBuffer = VK_NULL_HANDLE;
  VkBuffer _rotationBuffer = VK_NULL_HANDLE;
  VkBuffer _opacityBuffer = VK_NULL_HANDLE;
  VkBuffer _shBuffer = VK_NULL_HANDLE;
  VkBuffer _viewProjectionBuffer = VK_NULL_HANDLE;

  void CreateGaussianBuffers();
  template <typename T>
  void CreateAndUploadBuffer(VkBuffer &buffer, const void *data,
                             std::string type, int offset = 1);

  void createUniformBuffer();

  std::unique_ptr<Camera> _camera;
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
