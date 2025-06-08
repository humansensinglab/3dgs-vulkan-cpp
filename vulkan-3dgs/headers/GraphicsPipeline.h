#pragma once

#include "BufferManager.h"
#include "Camera.h"
#include "RenderSettings.h"
#include "VulkanContext.h"
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <vulkan/vulkan.h>
struct AxisVertex {
  glm::vec3 position;
  glm::vec3 color;
};

struct AxisUBO {
  glm::mat4 mvp;
};

class GraphicsPipeline {
public:
  GraphicsPipeline(VulkanContext &vkContext);
  ~GraphicsPipeline();

  void Init();
  void CreateRenderPass();
  void CreateFrameBuffers();
  void CreateDescriptorPool();
  void CreateDescriptorSetLayout();
  void CreateDescriptorSets();
  void CreateGraphicsPipeline();
  void CreateVertexBuffer();
  void CreateUniformBuffers();
  void setBufferManager(BufferManager *buffer) { _bufferManager = buffer; };
  void RecordAxisRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                            Camera &cam);
  void UpdateUniformBuffer(uint32_t imageIndex, const glm::mat4 &mvpMatrix);

  void CleanUp();

private:
  VulkanContext &_vkContext;
  BufferManager *_bufferManager;
  VkRenderPass _renderPass = VK_NULL_HANDLE;
  std::vector<VkFramebuffer> _frameBuffers;

  // Descriptor pool and sets
  VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> _descriptorSets;

  // Graphics pipeline
  VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
  VkPipeline _graphicsPipeline = VK_NULL_HANDLE;

  // Vertex data
  VkBuffer _vertexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;

  // Uniform buffers (one per frame in flight)
  std::vector<VkBuffer> _uniformBuffers;

  // Helper functions
  VkShaderModule CreateShaderModule(const std::vector<char> &code);

  // Axis geometry data
  const std::array<AxisVertex, 6> _axisVertices = {
      {// X axis (red)
       {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
       {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},

       // Y axis (green)
       {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
       {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},

       // Z axis (blue)
       {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
       {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}}};
};