#pragma once

#include "BufferManager.h"
#include "Camera.h"
#include "VulkanContext.h"
#include "glm/glm.hpp"
#include "utils.h"
#include <iostream>
#include <map>

constexpr int frames_in_flight = 2;

struct DescriptorBinding {
  uint32_t binding;
  VkDescriptorType type;
  VkShaderStageFlags stageFlags;
  uint32_t count = 1;
  std::string name = "";
};

enum class PipelineType {
  DEBUG_RED_FILL,
  CULLING,
  SORTING,
  SPLATTING,
  PREPROCESS,
  PREFIXSUM,
  ASSIGN_TILE_IDS,
  NEAREST
};

class ComputePipeline {
public:
  ComputePipeline(VulkanContext &vkContext) : _vkContext(vkContext){};
  ~ComputePipeline() { CleanUp(); }

  void Initialize(GaussianBuffers gaussianBuffer);
  void RenderFrame();
  void CleanUp();
  void setNumGaussians(int gauss) {
    _numGaussians = gauss;
    _sizeBufferMax = gauss * AVG_GAUSS_TILE;
  }
  void setBufferManager(BufferManager *bufferManager) {
    _buffManager = bufferManager;
  };

private:
  VulkanContext &_vkContext;

  std::vector<VkCommandBuffer> _commandBuffers;
  std::vector<VkFence> _preprocessFences;
  std::vector<VkFence> _renderFences;
  std::vector<VkSemaphore> _semaphores;

  std::map<PipelineType, VkDescriptorSetLayout> _descriptorSetLayouts;
  VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
  std::map<PipelineType, std::vector<VkDescriptorSet>> _descriptorSets;

  std::map<PipelineType, VkPipelineLayout> _pipelineLayouts;
  std::map<PipelineType, VkPipeline> _computePipelines;
  uint32_t _currentFrame = 0;

  void CreateCommandBuffers();
  void CreateSynchronization();
  void CreateDescriptorSetLayout(const PipelineType pType);
  void CreateDescriptorPool();
  void CreateComputePipeline(std::string shaderName, const PipelineType pType,
                             int numPushConstants = 0);
  void SetupDescriptorSet(const PipelineType pType);
  void RecordCommandPreprocess(uint32_t imageIndex);
  void RecordCommandRender(uint32_t imageIndex);
  VkShaderModule CreateShaderModule(const std::vector<char> &code);

  void TransitionImage(VkCommandBuffer commandBuffer, VkImageLayout in,
                       VkImageLayout out, VkImage image, VkAccessFlags src,
                       VkAccessFlags dst, VkPipelineStageFlagBits srcStage,
                       VkPipelineStageFlagBits dstStage);
  void UpdateAllDescriptorSets(const PipelineType pType);
  void RecordAllCommandBuffers();
  void BindImageToDescriptor(const PipelineType pType, uint32_t i,
                             VkImageView view);

  void BindBufferToDescriptor(const PipelineType pType, uint32_t bindingIndex,
                              uint32_t i, VkBuffer buffer,
                              VkDescriptorType descriptorType);
  VkBuffer GetBufferByName(const std::string &bufferName);

  void submitCommandBuffer(uint32_t imageIndex, bool waitSem = true);

  void resizeBuffers(uint32_t size);
  // repeated layouts:: we can share them. TODO
  std::map<PipelineType, std::vector<DescriptorBinding>> SHADER_LAYOUTS = {
      {PipelineType::PREPROCESS,
       {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "outputImage"},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "xyz"},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "scales"},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "rotations"},
        {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "opacity"},
        {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "sh"},
        {6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "camUniform"},
        {7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "radii"},
        {8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "depths"},
        {9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "rgb"},
        {10, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "conicOpacity"},
        {11, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "pointsXY"},
        {12, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "tilesTouched"},
        {13, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "boundingBox"}}},

      {PipelineType::NEAREST,
       {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "outputImage"},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "radii"},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "depths"},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "rgb"},
        {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "pointsXY"},
        {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "xyz"},
        {6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "camUniform"}}},

      {PipelineType::PREFIXSUM,
       {
           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            1, "tilesTouched"},
           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            1, "tilesTouchedPrefixSum"},

       }},

      {PipelineType::ASSIGN_TILE_IDS,
       {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "tilesTouched"},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "depths"},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "radii"},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "boundingBox"},
        {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "keysUnsorted"},
        {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1,
         "valuesUnsorted"}}}};

  int _sizeBufferMax;
  GaussianBuffers _gaussianBuffers;
  BufferManager *_buffManager;
  int _numGaussians;

  inline uint32_t ReadFinalPrefixSum() {

    return *static_cast<uint32_t *>(_gaussianBuffers.numRendered.mem);
  }
};