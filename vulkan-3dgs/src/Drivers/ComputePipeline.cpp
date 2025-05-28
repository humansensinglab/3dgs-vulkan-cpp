// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "ComputePipeline.h"

void ComputePipeline::Initialize(GaussianBuffers gaussianBuffer) {
  std::cout << "\n === Compute Pipeline Initalization === \n" << std::endl;

  _gaussianBuffers = gaussianBuffer;
  bool resultInBufferB = (_numSteps % 2) == 1;
  _resultBufferPrefix = resultInBufferB
                            ? _gaussianBuffers.tilesTouched
                            : _gaussianBuffers.tilesTouchedPrefixSum;
  CreateCommandBuffers();
  CreateDescriptorPool();

  CreateDescriptorSetLayout(PipelineType::PREPROCESS);
  CreateComputePipeline("src/Shaders/preprocess.spv", PipelineType::PREPROCESS,
                        1);
  SetupDescriptorSet(PipelineType::PREPROCESS);
  UpdateAllDescriptorSets(PipelineType::PREPROCESS);

  CreateDescriptorSetLayout(PipelineType::PREFIXSUM);
  CreateComputePipeline("src/Shaders/sum.spv", PipelineType::PREFIXSUM, 3);
  SetupDescriptorSet(PipelineType::PREFIXSUM);
  UpdateAllDescriptorSets(PipelineType::PREFIXSUM);

  CreateDescriptorSetLayout(PipelineType::NEAREST);
  CreateComputePipeline("src/Shaders/nearest.spv", PipelineType::NEAREST);
  SetupDescriptorSet(PipelineType::NEAREST);
  UpdateAllDescriptorSets(PipelineType::NEAREST);

  CreateDescriptorSetLayout(PipelineType::ASSIGN_TILE_IDS);
  CreateComputePipeline("src/Shaders/idkeys.spv", PipelineType::ASSIGN_TILE_IDS,
                        2);
  SetupDescriptorSet(PipelineType::ASSIGN_TILE_IDS);
  UpdateAllDescriptorSets(PipelineType::ASSIGN_TILE_IDS);

  CreateDescriptorSetLayout(PipelineType::RADIX_HISTOGRAM_0);
  CreateComputePipeline("src/Shaders/histogram.spv",
                        PipelineType::RADIX_HISTOGRAM_0, 4);
  CreateDescriptorSetLayout(PipelineType::RADIX_SCATTER_0);
  CreateComputePipeline("src/Shaders/sort.spv", PipelineType::RADIX_SCATTER_0,
                        4);

  SetupDescriptorSet(PipelineType::RADIX_HISTOGRAM_0);
  UpdateAllDescriptorSets(PipelineType::RADIX_HISTOGRAM_0);

  SetupDescriptorSet(PipelineType::RADIX_HISTOGRAM_1);
  UpdateAllDescriptorSets(PipelineType::RADIX_HISTOGRAM_1);

  SetupDescriptorSet(PipelineType::RADIX_SCATTER_0);
  UpdateAllDescriptorSets(PipelineType::RADIX_SCATTER_0);

  SetupDescriptorSet(PipelineType::RADIX_SCATTER_1);
  UpdateAllDescriptorSets(PipelineType::RADIX_SCATTER_1);

  CreateSynchronization();

  // RecordAllCommandBuffers();
  std::cout << "\n=== Compute Pipeline Initialization Complete ===\n"
            << std::endl;
}

void ComputePipeline::CleanUp() {
  if (_vkContext.GetLogicalDevice() == VK_NULL_HANDLE) {
    return;
  }

  std::cout << "Cleaning up ComputePipeline..." << std::endl;

  for (auto &fence : _renderFences) {
    if (fence != VK_NULL_HANDLE) {
      vkDestroyFence(_vkContext.GetLogicalDevice(), fence, nullptr);
    }
  }
  _renderFences.clear();

  for (auto &fence : _preprocessFences) {
    if (fence != VK_NULL_HANDLE) {
      vkDestroyFence(_vkContext.GetLogicalDevice(), fence, nullptr);
    }
  }
  _preprocessFences.clear();

  for (auto &semaphore : _semaphores) {
    if (semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(_vkContext.GetLogicalDevice(), semaphore, nullptr);
    }
  }
  _semaphores.clear();

  _commandBuffers.clear();

  if (_computePipelines[PipelineType::DEBUG_RED_FILL] != VK_NULL_HANDLE) {
    vkDestroyPipeline(_vkContext.GetLogicalDevice(),
                      _computePipelines[PipelineType::DEBUG_RED_FILL], nullptr);
    _computePipelines[PipelineType::DEBUG_RED_FILL] = VK_NULL_HANDLE;
  }

  if (_pipelineLayouts[PipelineType::DEBUG_RED_FILL] != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(_vkContext.GetLogicalDevice(),
                            _pipelineLayouts[PipelineType::DEBUG_RED_FILL],
                            nullptr);
    _pipelineLayouts[PipelineType::DEBUG_RED_FILL] = VK_NULL_HANDLE;
  }

  // 6. Descriptor pool (this automatically frees all descriptor sets)
  if (_descriptorPool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(_vkContext.GetLogicalDevice(), _descriptorPool,
                            nullptr);
    _descriptorPool = VK_NULL_HANDLE;
  }
  _descriptorSets.clear();

  if (_descriptorSetLayouts[PipelineType::DEBUG_RED_FILL] != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(
        _vkContext.GetLogicalDevice(),
        _descriptorSetLayouts[PipelineType::DEBUG_RED_FILL], nullptr);
    _descriptorSetLayouts[PipelineType::DEBUG_RED_FILL] = VK_NULL_HANDLE;
  }

  std::cout << "ComputePipeline cleanup complete" << std::endl;
}

void ComputePipeline::CreateCommandBuffers() {

  int size_sw = _vkContext.GetSwapchainImages().size();
  _commandBuffers.resize(size_sw);
  VkCommandBufferAllocateInfo cbAllocInfo = {};
  cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cbAllocInfo.commandPool = _vkContext.GetCommandPool();
  cbAllocInfo.level =
      VK_COMMAND_BUFFER_LEVEL_PRIMARY; // executed by queue--secondary-> by
                                       // other buffers

  cbAllocInfo.commandBufferCount = static_cast<uint32_t>(size_sw);

  if (vkAllocateCommandBuffers(_vkContext.GetLogicalDevice(), &cbAllocInfo,
                               _commandBuffers.data()) != VK_SUCCESS)
    throw std::runtime_error("Fail to allocate buffers");

  std::cout << " Command buffer allocated: " << size_sw << std::endl;
}

void ComputePipeline::CreateSynchronization() {

  _semaphores.resize(frames_in_flight);       // When swapchain image is ready
  _preprocessFences.resize(frames_in_flight); // When compute work is done
  _renderFences.resize(frames_in_flight);     // When compute work is done

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < frames_in_flight; i++) {

    if (vkCreateSemaphore(_vkContext.GetLogicalDevice(), &semaphoreInfo,
                          nullptr, &_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(_vkContext.GetLogicalDevice(), &fenceInfo, nullptr,
                      &_renderFences[i]) != VK_SUCCESS ||
        vkCreateFence(_vkContext.GetLogicalDevice(), &fenceInfo, nullptr,
                      &_preprocessFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create synchronization objects!");
    }
  }
  std::cout << " Created Sempahores and Fences " << std::endl;
}

void ComputePipeline::CreateDescriptorSetLayout(const PipelineType pType) {

  std::vector<VkDescriptorSetLayoutBinding> vulkanBindings;
  vulkanBindings.reserve(SHADER_LAYOUTS[pType].size());

  for (const auto &binding : SHADER_LAYOUTS[pType]) {

    VkDescriptorSetLayoutBinding vulkanBinding = {};
    vulkanBinding.binding = binding.binding;
    vulkanBinding.descriptorType = binding.type;
    vulkanBinding.descriptorCount = binding.count;
    vulkanBinding.stageFlags = binding.stageFlags;
    vulkanBinding.pImmutableSamplers = nullptr;

    vulkanBindings.push_back(vulkanBinding);
  }

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(vulkanBindings.size());
  layoutInfo.pBindings = vulkanBindings.data();

  if (vkCreateDescriptorSetLayout(_vkContext.GetLogicalDevice(), &layoutInfo,
                                  nullptr, &_descriptorSetLayouts[pType]) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }

  std::cout << " Descriptor set layout created (" << vulkanBindings.size()
            << " bindings)" << std::endl;
}

void ComputePipeline::CreateComputePipeline(std::string shaderName,
                                            const PipelineType pType,
                                            int numPushConstants) {
  std::cout << "  - Loading and creating compute pipeline..." << std::endl;
  auto computeShaderCode = ReadFile(shaderName);
  VkShaderModule computeShader = CreateShaderModule(computeShaderCode);

  VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
  computeShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  computeShaderStageInfo.module = computeShader;
  computeShaderStageInfo.pName = "main";

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayouts[pType];

  VkPushConstantRange pushConstantRange = {};
  if (numPushConstants != 0) {

    pushConstantRange.stageFlags =
        VK_SHADER_STAGE_COMPUTE_BIT; // Should be 0x00000020
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(uint32_t) * numPushConstants;

    std::cout << "Push constant range - stage: " << pushConstantRange.stageFlags
              << ", offset: " << pushConstantRange.offset
              << ", size: " << pushConstantRange.size << std::endl;

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  } else {
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
  }

  if (vkCreatePipelineLayout(_vkContext.GetLogicalDevice(), &pipelineLayoutInfo,
                             nullptr, &_pipelineLayouts[pType]) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout!");
  }

  VkComputePipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = _pipelineLayouts[pType];
  pipelineInfo.stage = computeShaderStageInfo;

  if (vkCreateComputePipelines(_vkContext.GetLogicalDevice(), VK_NULL_HANDLE, 1,
                               &pipelineInfo, nullptr,
                               &_computePipelines[pType]) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create compute pipeline!");
  }
  vkDestroyShaderModule(_vkContext.GetLogicalDevice(), computeShader, nullptr);
  std::cout << "Compute pipeline created for pipeline type " << (int)pType
            << " using shader: " << shaderName << std::endl;
}

void ComputePipeline::SetupDescriptorSet(const PipelineType pType) {
  std::cout << "  - Setting up descriptor sets for pipeline type " << (int)pType
            << "..." << std::endl;

  // Get number of swapchain images
  uint32_t swapchainImageCount =
      static_cast<uint32_t>(_vkContext.GetSwapchainImages().size());

  // Resize vector to hold one descriptor set per swapchain image
  _descriptorSets[pType].resize(swapchainImageCount);

  std::vector<VkDescriptorSetLayout> layouts;

  if (pType == PipelineType::RADIX_HISTOGRAM_1) {
    layouts = std::vector<VkDescriptorSetLayout>(
        swapchainImageCount,
        _descriptorSetLayouts[PipelineType::RADIX_HISTOGRAM_0]);
  } else if (pType == PipelineType::RADIX_SCATTER_1) {
    layouts = std::vector<VkDescriptorSetLayout>(
        swapchainImageCount,
        _descriptorSetLayouts[PipelineType::RADIX_SCATTER_0]);
  } else {
    layouts = std::vector<VkDescriptorSetLayout>(swapchainImageCount,
                                                 _descriptorSetLayouts[pType]);
  }

  // Now 'layouts' is accessible here
  // Allocate all descriptor sets at once
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount = swapchainImageCount;
  allocInfo.pSetLayouts = layouts.data();

  if (vkAllocateDescriptorSets(_vkContext.GetLogicalDevice(), &allocInfo,
                               _descriptorSets[pType].data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets!");
  }

  std::cout << " Allocated " << swapchainImageCount
            << " descriptor sets for pipeline type " << (int)pType << std::endl;
}

void ComputePipeline::RecordCommandPreprocess(uint32_t imageIndex) {
  VkCommandBuffer commandBuffer = _commandBuffers[imageIndex];

  // Begin recording
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Not one-time submit

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer!");
  }
  /////////////////////////////////////////////////////////////////////////////////////
  // Transition image to GENERAL
  TransitionImage(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_GENERAL,
                  _vkContext.GetSwapchainImages()[imageIndex].image, 0,
                  VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  /////////////////////////////////////////////////////////////////////////////////////
  // Bind pipeline 1

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    _computePipelines[PipelineType::PREPROCESS]);

  vkCmdPushConstants(commandBuffer, _pipelineLayouts[PipelineType::PREPROCESS],
                     VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(int),
                     &_numGaussians);
  // Bind descriptor set
  vkCmdBindDescriptorSets(
      commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
      _pipelineLayouts[PipelineType::PREPROCESS], 0, 1,
      &_descriptorSets[PipelineType::PREPROCESS][imageIndex], 0, nullptr);

  uint32_t groupX = (_numGaussians + 255) / 256;
  vkCmdDispatch(commandBuffer, groupX, 1, 1);

  /////////////////////////////////////////////////////////////////////////////////////
  // Barrier1
  VkMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0,
                       nullptr, 0, nullptr);

  ///////////////////////////////////////////////////////////////////////////////////////
  //// Prefix Sum

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    _computePipelines[PipelineType::PREFIXSUM]);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          _pipelineLayouts[PipelineType::PREFIXSUM], 0, 1,
                          &_descriptorSets[PipelineType::PREFIXSUM][imageIndex],
                          0, nullptr);

  uint32_t prefixSumGroups = (_numGaussians + 255) / 256;

  for (uint32_t step = 0; step <= _numSteps; step++) {
    struct PushConstants {
      uint32_t step;
      uint32_t numElements;
      uint32_t readFromA; // ADD THIS
    } pushConstants = {step, _numGaussians,
                       (step % 2) == 0 ? 1 : 0}; // MODIFY THIS

    vkCmdPushConstants(commandBuffer, _pipelineLayouts[PipelineType::PREFIXSUM],
                       VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants),
                       &pushConstants);
    vkCmdDispatch(commandBuffer, prefixSumGroups, 1, 1);

    if (step < _numSteps - 1) {
      VkMemoryBarrier stepBarrier = {};
      stepBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
      stepBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      stepBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
                           &stepBarrier, 0, nullptr, 0, nullptr);
    }
  }

  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = (_numGaussians - 1) * sizeof(uint32_t);
  copyRegion.dstOffset = 0;
  copyRegion.size = sizeof(uint32_t);

  vkCmdCopyBuffer(commandBuffer, _resultBufferPrefix,
                  _gaussianBuffers.numRendered.staging, 1, &copyRegion);

  ///////////////////// END PREFIX SUM /////////////////////

  /////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////
  // End recording
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffer!");
  }

  /*std::cout << " Command buffer recorded for swapchain image " << imageIndex
            << std::endl;*/
}

void ComputePipeline::RecordCommandRender(uint32_t imageIndex,
                                          int numRendered) {
  VkCommandBuffer commandBuffer = _commandBuffers[imageIndex];

  // Begin recording
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Not one-time submit

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer!");
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    _computePipelines[PipelineType::ASSIGN_TILE_IDS]);

  VkExtent2D extent = _vkContext.GetSwapchainExtent();
  uint32_t tileX = (extent.width + 15) / 16;
  struct {
    uint32_t tile;
    int nGauss;
  } pushCt = {tileX, _numGaussians};
  vkCmdPushConstants(commandBuffer,
                     _pipelineLayouts[PipelineType::ASSIGN_TILE_IDS],
                     VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushCt), &pushCt);

  vkCmdBindDescriptorSets(
      commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
      _pipelineLayouts[PipelineType::ASSIGN_TILE_IDS], 0, 1,
      &_descriptorSets[PipelineType::ASSIGN_TILE_IDS][imageIndex], 0, nullptr);

  vkCmdDispatch(commandBuffer, (_numGaussians + 255) / 256, 1, 1);

  VkMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; // For presentation

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 1, &barrier, 0,
                       nullptr, 0, nullptr);

  ////////////////////////////////////////////////////////////////////////////////////////
  //
  //
  //
  //
  VkMemoryBarrier barrier_t = {};
  barrier_t.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  barrier_t.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier_t.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; // For presentation

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 1, &barrier_t,
                       0, nullptr, 0, nullptr);
  uint32_t numElementsToSort =
      numRendered; // This should be the total gaussian-tile pairs

  // Calculate workgroups for radix sort
  uint32_t elementsPerWorkgroup =
      WORKGROUP_SIZE * blocks_per_workgroup; // 256 * 32 = 8192
  uint32_t numWorkgroups =
      (numElementsToSort + elementsPerWorkgroup - 1) / elementsPerWorkgroup;

  // Prepare push constants
  struct RadixPushConstants {
    uint32_t g_num_elements;
    uint32_t g_shift;
    uint32_t g_num_workgroups;
    uint32_t g_num_blocks_per_workgroup;
  } radixPC;

  radixPC.g_num_elements = numElementsToSort;
  radixPC.g_num_workgroups = numWorkgroups;
  radixPC.g_num_blocks_per_workgroup = blocks_per_workgroup;

  // Perform 6 passes of radix sort (tiles_ID always can be represented with
  // 2 bits)
  for (uint32_t pass = 0; pass < 6; pass++) {
    radixPC.g_shift = pass * 8;

    bool isEven = (pass % 2 == 0);

    // HISTOGRAM PASS
    PipelineType histType = isEven ? PipelineType::RADIX_HISTOGRAM_0
                                   : PipelineType::RADIX_HISTOGRAM_1;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                      _computePipelines[PipelineType::RADIX_HISTOGRAM_0]);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            _pipelineLayouts[PipelineType::RADIX_HISTOGRAM_0],
                            0, 1, &_descriptorSets[histType][imageIndex], 0,
                            nullptr);
    vkCmdPushConstants(
        commandBuffer, _pipelineLayouts[PipelineType::RADIX_HISTOGRAM_0],
        VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RadixPushConstants), &radixPC);
    vkCmdDispatch(commandBuffer, numWorkgroups, 1, 1);

    VkMemoryBarrier histBarrier = {};
    histBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    histBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    histBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
                         &histBarrier, 0, nullptr, 0, nullptr);

    // SCATTER PASS
    PipelineType scatterType =
        isEven ? PipelineType::RADIX_SCATTER_0 : PipelineType::RADIX_SCATTER_1;

    vkCmdBindPipeline(
        commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        _computePipelines[PipelineType::RADIX_SCATTER_0]); // Use same pipeline
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            _pipelineLayouts[PipelineType::RADIX_SCATTER_0], 0,
                            1, &_descriptorSets[scatterType][imageIndex], 0,
                            nullptr);
    vkCmdPushConstants(
        commandBuffer, _pipelineLayouts[PipelineType::RADIX_SCATTER_0],
        VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RadixPushConstants), &radixPC);
    vkCmdDispatch(commandBuffer, numWorkgroups, 1, 1);

    if (pass < 5) {
      VkMemoryBarrier scatterBarrier = {};
      scatterBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
      scatterBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      scatterBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
                           &scatterBarrier, 0, nullptr, 0, nullptr);
    }
  }

  VkMemoryBarrier finalBarrier = {};
  finalBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  finalBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  finalBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
                       &finalBarrier, 0, nullptr, 0, nullptr);

  // VkBufferCopy copyRegion = {};
  // copyRegion.srcOffset = 0;
  // copyRegion.dstOffset = 0;
  // copyRegion.size = 1000 * sizeof(uint64_t);

  // vkCmdCopyBuffer(commandBuffer, _gaussianBuffers.keys,
  //                 _gaussianBuffers.numRendered.staging, 1, &copyRegion);

  // VkBufferCopy copyRegionx = {};
  // copyRegionx.srcOffset = (numRendered - 1001) * sizeof(uint64_t);
  // copyRegionx.dstOffset = 1000 * sizeof(uint64_t);
  // copyRegionx.size = 1000 * sizeof(uint64_t);

  // vkCmdCopyBuffer(commandBuffer, _gaussianBuffers.keys,
  //                 _gaussianBuffers.numRendered.staging, 1, &copyRegionx);

  /////////////////////////////////////////////////////////////////////////////////////////

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    _computePipelines[PipelineType::NEAREST]);

  // Bind descriptor set
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          _pipelineLayouts[PipelineType::NEAREST], 0, 1,
                          &_descriptorSets[PipelineType::NEAREST][imageIndex],
                          0, nullptr);

  vkCmdDispatch(commandBuffer, (extent.width + 15) / 16,
                (extent.height + 15) / 16, 1);

  VkMemoryBarrier barrier_x = {};
  barrier_x.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  barrier_x.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier_x.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; // For presentation

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 1, &barrier_x,
                       0, nullptr, 0, nullptr);

  // Transition back to PRESENT
  TransitionImage(commandBuffer, VK_IMAGE_LAYOUT_GENERAL,
                  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                  _vkContext.GetSwapchainImages()[imageIndex].image,
                  VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffer!");
  }

  /* std::cout << " Command buffer recorded for swapchain image " << imageIndex
             << std::endl;*/
}

VkShaderModule
ComputePipeline::CreateShaderModule(const std::vector<char> &code) {
  VkShaderModuleCreateInfo shaderCreateInfo = {};
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.codeSize = code.size();
  shaderCreateInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(_vkContext.GetLogicalDevice(), &shaderCreateInfo,
                           nullptr, &shaderModule) != VK_SUCCESS)
    throw std::runtime_error("Fail Creating Shader Module");

  return shaderModule;
}

void ComputePipeline::TransitionImage(VkCommandBuffer commandBuffer,
                                      VkImageLayout in, VkImageLayout out,
                                      VkImage image, VkAccessFlags src,
                                      VkAccessFlags dst,
                                      VkPipelineStageFlagBits srcStage,
                                      VkPipelineStageFlagBits dstStage) {

  // Transition swapchain image to GENERAL layout for compute write
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // Type of barrier
  barrier.oldLayout = in;  // Don't care about previous contents
  barrier.newLayout = out; // Layout for compute shader access
  barrier.srcQueueFamilyIndex =
      VK_QUEUE_FAMILY_IGNORED; // Not transferring between queue families
  barrier.dstQueueFamilyIndex =
      VK_QUEUE_FAMILY_IGNORED; // Not transferring between queue families
  barrier.image = image;       // The specific swapchain image
  barrier.subresourceRange.aspectMask =
      VK_IMAGE_ASPECT_COLOR_BIT;               // Color data (not depth/stencil)
  barrier.subresourceRange.baseMipLevel = 0;   // Mipmap level 0
  barrier.subresourceRange.levelCount = 1;     // Only 1 mipmap level
  barrier.subresourceRange.baseArrayLayer = 0; // Array layer 0
  barrier.subresourceRange.layerCount = 1;     // Only 1 array layer
  barrier.srcAccessMask = src;                 // No previous access to wait for
  barrier.dstAccessMask = dst;                 // Compute shader will write

  vkCmdPipelineBarrier(
      commandBuffer, // Command buffer to record into
      srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1,
      &barrier); // 1 image barrier, no memory/buffer barriers //
}

void ComputePipeline::RenderFrame() {

  vkWaitForFences(_vkContext.GetLogicalDevice(), 1,
                  &_renderFences[_currentFrame], VK_TRUE, UINT64_MAX);

  vkResetFences(_vkContext.GetLogicalDevice(), 1,
                &_preprocessFences[_currentFrame]);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      _vkContext.GetLogicalDevice(), _vkContext.GetSwapchain(), UINT64_MAX,
      _semaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to acquire swapchain image!");
  }

  vkResetCommandBuffer(_commandBuffers[imageIndex], 0);
  RecordCommandPreprocess(imageIndex);
  submitCommandBuffer(imageIndex);

  vkWaitForFences(_vkContext.GetLogicalDevice(), 1,
                  &_preprocessFences[_currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t totalRendered = ReadFinalPrefixSum();
  std::cout << totalRendered << std::endl;
  if (totalRendered > _sizeBufferMax) {
    resizeBuffers(totalRendered * 1.25);
  }
  vkResetFences(_vkContext.GetLogicalDevice(), 1,
                &_renderFences[_currentFrame]);

  // another command buffer?
  RecordCommandRender(imageIndex, totalRendered);
  submitCommandBuffer(imageIndex, false);
  // Present the image

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; // Type of present
  presentInfo.waitSemaphoreCount = 0;    // No additional semaphores to wait for
  presentInfo.pWaitSemaphores = nullptr; // No wait semaphores
  presentInfo.swapchainCount = 1;        // Number of swapchains to present to
  VkSwapchainKHR swapchains[] = {
      _vkContext.GetSwapchain()};       // Array of swapchains
  presentInfo.pSwapchains = swapchains; // Swapchains to present to
  presentInfo.pImageIndices =
      &imageIndex; // Which image in each swapchain to present

  vkQueuePresentKHR(_vkContext.GetGraphicsQueue(), &presentInfo);
  _currentFrame = (_currentFrame + 1) % _renderFences.size();
}

void ComputePipeline::CreateDescriptorPool() {
  std::map<VkDescriptorType, uint32_t> typeCounts;
  uint32_t swapchainImageCount =
      static_cast<uint32_t>(_vkContext.GetSwapchainImages().size());

  for (const auto &[pipelineType, bindings] : SHADER_LAYOUTS) {
    for (const auto &binding : bindings) {
      typeCounts[binding.type] += binding.count * swapchainImageCount;
    }
  }

  std::vector<VkDescriptorPoolSize> poolSizes;
  for (const auto &[type, count] : typeCounts) {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = type;
    poolSize.descriptorCount = count;
    poolSizes.push_back(poolSize);
  }

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();

  poolInfo.maxSets =
      static_cast<uint32_t>(SHADER_LAYOUTS.size()) * swapchainImageCount;

  if (vkCreateDescriptorPool(_vkContext.GetLogicalDevice(), &poolInfo, nullptr,
                             &_descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool!");
  }

  std::cout << " Descriptor Pool created with " << poolSizes.size()
            << " different Pool Size(s)" << std::endl;
}

void ComputePipeline::UpdateAllDescriptorSets(const PipelineType pType) {
  std::cout << "  - Updating descriptor sets with swapchain images..."
            << std::endl;

  auto &swapchainImages = _vkContext.GetSwapchainImages();

  for (const auto &descriptor : SHADER_LAYOUTS[pType]) {
    for (uint32_t i = 0; i < swapchainImages.size(); i++) {
      if (descriptor.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        BindImageToDescriptor(pType, i, swapchainImages[i].imageView);
      else {
        BindBufferToDescriptor(pType, descriptor.binding, i,
                               GetBufferByName(descriptor.name),
                               descriptor.type);
      }
    }
  }
}

VkBuffer ComputePipeline::GetBufferByName(const std::string &bufferName) {
  // Map is better XD
  if (bufferName == "xyz")
    return _gaussianBuffers.xyz;
  if (bufferName == "scales")
    return _gaussianBuffers.scales;
  if (bufferName == "rotations")
    return _gaussianBuffers.rotations;
  if (bufferName == "opacity")
    return _gaussianBuffers.opacity;
  if (bufferName == "sh")
    return _gaussianBuffers.sh;
  if (bufferName == "camUniform")
    return _gaussianBuffers.camUniform;
  if (bufferName == "radii")
    return _gaussianBuffers.radii;
  if (bufferName == "depths")
    return _gaussianBuffers.depth;
  if (bufferName == "rgb")
    return _gaussianBuffers.color;
  if (bufferName == "conicOpacity")
    return _gaussianBuffers.conicOpacity;
  if (bufferName == "pointsXY")
    return _gaussianBuffers.points2d;
  if (bufferName == "tilesTouched")
    return _gaussianBuffers.tilesTouched;
  if (bufferName == "tilesTouchedPrefixSum")
    return _gaussianBuffers.tilesTouchedPrefixSum;
  if (bufferName == "boundingBox")
    return _gaussianBuffers.boundingBox;
  if (bufferName == "keys")
    return _gaussianBuffers.keys;
  if (bufferName == "values")
    return _gaussianBuffers.values;
  if (bufferName == "keysRadix")
    return _gaussianBuffers.keysRadix;
  if (bufferName == "valuesRadix")
    return _gaussianBuffers.valuesRadix;
  if (bufferName == "ranges")
    return _gaussianBuffers.ranges;
  if (bufferName == "prefixResult")
    return _resultBufferPrefix;
  if (bufferName == "histograms")
    return _gaussianBuffers.histogram;

  throw std::runtime_error("Unknown buffer name: " + bufferName);
}

void ComputePipeline::submitCommandBuffer(uint32_t imageIndex, bool waitSem) {
  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; // Type of submit
  submitInfo.commandBufferCount = 1; // Number of command buffers to submit
  submitInfo.pCommandBuffers = &_commandBuffers[imageIndex];

  VkSemaphore waitSemaphores[] = {_semaphores[_currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};

  if (waitSem) {
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
  }
  VkFence fence = (waitSem) ? _preprocessFences[_currentFrame]
                            : _renderFences[_currentFrame];

  if (vkQueueSubmit(_vkContext.GetGraphicsQueue(), // Queue to
                                                   // submit to
                    1,                             // Number of submits
                    &submitInfo,                   // Submit info
                    fence) != VK_SUCCESS) {        // Fence to signal when done
    throw std::runtime_error("Failed to submit compute command buffer!");
  }
}

int ComputePipeline::getRadixIterations() {
  VkExtent2D extent = _vkContext.GetSwapchainExtent();
  uint32_t nTiles = ((extent.width + 15) / 16) * ((extent.height + 15) / 16);
  uint32_t tileBits = static_cast<uint32_t>(std::ceil(std::log2(nTiles)));
  uint32_t totalBits = 32 + tileBits;
  return (totalBits + 7) / 8;
}

void ComputePipeline::resizeBuffers(uint32_t size) {
  // We could implement memory Pool?
  VkPhysicalDevice physicalDevice = _vkContext.GetPhysicalDevice();
  VkDevice device = _vkContext.GetLogicalDevice();

  _buffManager->DestroyBuffer(device, _gaussianBuffers.valuesRadix);
  _buffManager->DestroyBuffer(device, _gaussianBuffers.values);
  _buffManager->DestroyBuffer(device, _gaussianBuffers.keys);
  _buffManager->DestroyBuffer(device, _gaussianBuffers.keysRadix);
  _buffManager->DestroyBuffer(device, _gaussianBuffers.histogram);

  VkDeviceSize bufferSizeKey = sizeof(int64_t) * size;
  VkDeviceSize bufferSizeValue = sizeof(int32_t) * size;

  uint32_t elementsPerWorkgroup =
      WORKGROUP_SIZE * blocks_per_workgroup; // 256 * 32 = 8192
  uint32_t numWorkgroups =
      (size + elementsPerWorkgroup - 1) / elementsPerWorkgroup;
  VkDeviceSize histogramSize =
      RADIX_SORT_BINS * numWorkgroups * sizeof(uint32_t);

  VkBufferUsageFlags usage =
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  _gaussianBuffers.keys =
      _buffManager->CreateBuffer(device, physicalDevice, bufferSizeKey, usage,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  _gaussianBuffers.keysRadix =
      _buffManager->CreateBuffer(device, physicalDevice, bufferSizeKey, usage,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  _gaussianBuffers.values =
      _buffManager->CreateBuffer(device, physicalDevice, bufferSizeValue, usage,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  _gaussianBuffers.valuesRadix =
      _buffManager->CreateBuffer(device, physicalDevice, bufferSizeValue, usage,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  _gaussianBuffers.histogram =
      _buffManager->CreateBuffer(device, physicalDevice, histogramSize, usage,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  _sizeBufferMax = size;
  UpdateAllDescriptorSets(PipelineType::ASSIGN_TILE_IDS);
  UpdateAllDescriptorSets(PipelineType::RADIX_HISTOGRAM_0);
  UpdateAllDescriptorSets(PipelineType::RADIX_HISTOGRAM_1);
  UpdateAllDescriptorSets(PipelineType::RADIX_SCATTER_0);
  UpdateAllDescriptorSets(PipelineType::RADIX_SCATTER_1);

  std::cout << "resize Buffers and update Descriptors" << std::endl;
}

void ComputePipeline::SetUpRadixBuffers() {}

void ComputePipeline::RecordAllCommandBuffers() {}

void ComputePipeline::BindImageToDescriptor(const PipelineType pType,
                                            uint32_t i, VkImageView view) {
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  imageInfo.imageView = view;
  imageInfo.sampler = VK_NULL_HANDLE;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = _descriptorSets[pType][i]; // Specific descriptor set
  descriptorWrite.dstBinding = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(_vkContext.GetLogicalDevice(), 1, &descriptorWrite, 0,
                         nullptr);

  std::cout << "Descriptor set " << i << "  Swapchain image " << i << std::endl;
}

void ComputePipeline::BindBufferToDescriptor(const PipelineType pType,
                                             uint32_t bindingIndex, uint32_t i,
                                             VkBuffer buffer,
                                             VkDescriptorType descriptorType) {
  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer = buffer;
  bufferInfo.offset = 0;
  bufferInfo.range = (descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                         ? sizeof(CameraUniforms)
                         : VK_WHOLE_SIZE;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = _descriptorSets[pType][i];
  descriptorWrite.dstBinding = bindingIndex;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = descriptorType;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &bufferInfo;

  vkUpdateDescriptorSets(_vkContext.GetLogicalDevice(), 1, &descriptorWrite, 0,
                         nullptr);
}
