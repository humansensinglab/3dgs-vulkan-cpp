#include "ComputePipeline.h"

void ComputePipeline::Initialize(GaussianBuffers gaussianBuffer) {
  std::cout << "\n === Compute Pipeline Initalization === \n" << std::endl;

  _gaussianBuffers = gaussianBuffer;
  CreateCommandBuffers();
  CreateDescriptorSetLayout(PipelineType::DEBUG_RED_FILL);
  CreateDescriptorPool();
  CreateComputePipeline("src/Shaders/comp.spv", PipelineType::DEBUG_RED_FILL);
  SetupDescriptorSet(PipelineType::DEBUG_RED_FILL);
  UpdateAllDescriptorSets(PipelineType::DEBUG_RED_FILL);
  CreateSynchronization();

  RecordAllCommandBuffers();
  std::cout << "\n=== Compute Pipeline Initialization Complete ===\n"
            << std::endl;
}

void ComputePipeline::CleanUp() {
  if (_vkContext.GetLogicalDevice() == VK_NULL_HANDLE) {
    return;
  }

  std::cout << "Cleaning up ComputePipeline..." << std::endl;

  for (auto &fence : _fences) {
    if (fence != VK_NULL_HANDLE) {
      vkDestroyFence(_vkContext.GetLogicalDevice(), fence, nullptr);
    }
  }
  _fences.clear();

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

  _semaphores.resize(frames_in_flight); // When swapchain image is ready
  _fences.resize(frames_in_flight);     // When compute work is done

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < frames_in_flight; i++) {

    if (vkCreateSemaphore(_vkContext.GetLogicalDevice(), &semaphoreInfo,
                          nullptr, &_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(_vkContext.GetLogicalDevice(), &fenceInfo, nullptr,
                      &_fences[i]) != VK_SUCCESS) {
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
                                            const PipelineType pType) {
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
  pipelineLayoutInfo.pSetLayouts =
      &_descriptorSetLayouts[pType];             // Use our descriptor layout
  pipelineLayoutInfo.pushConstantRangeCount = 0; // No push constants for now
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

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

  // Create layout array (same layout for all sets)
  std::vector<VkDescriptorSetLayout> layouts(swapchainImageCount,
                                             _descriptorSetLayouts[pType]);

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

void ComputePipeline::RecordCommandBufferForImage(uint32_t imageIndex) {
  VkCommandBuffer commandBuffer = _commandBuffers[imageIndex];

  // Begin recording
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Not one-time submit

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer!");
  }

  // Transition image to GENERAL
  TransitionImage(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_GENERAL,
                  _vkContext.GetSwapchainImages()[imageIndex].image, 0,
                  VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  // Bind pipeline
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    _computePipelines[PipelineType::DEBUG_RED_FILL]);

  // Bind descriptor set
  vkCmdBindDescriptorSets(
      commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
      _pipelineLayouts[PipelineType::DEBUG_RED_FILL], 0, 1,
      &_descriptorSets[PipelineType::DEBUG_RED_FILL][imageIndex], 0, nullptr);

  // Dispatch
  VkExtent2D extent = _vkContext.GetSwapchainExtent();
  uint32_t groupCountX = (extent.width + 15) / 16;
  uint32_t groupCountY = (extent.height + 15) / 16;
  vkCmdDispatch(commandBuffer, groupCountX, groupCountY, 1);

  // Transition back to PRESENT
  TransitionImage(commandBuffer, VK_IMAGE_LAYOUT_GENERAL,
                  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                  _vkContext.GetSwapchainImages()[imageIndex].image,
                  VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

  // End recording
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffer!");
  }

  std::cout << " Command buffer recorded for swapchain image " << imageIndex
            << std::endl;
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

  vkWaitForFences(_vkContext.GetLogicalDevice(), 1, &_fences[_currentFrame],
                  VK_TRUE, UINT64_MAX);
  vkResetFences(_vkContext.GetLogicalDevice(), 1, &_fences[_currentFrame]);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      _vkContext.GetLogicalDevice(), _vkContext.GetSwapchain(), UINT64_MAX,
      _semaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to acquire swapchain image!");
  }

  // 5. Submit

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; // Type of submit

  VkSemaphore waitSemaphores[] = {
      _semaphores[_currentFrame]}; // Wait for image to be available
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT}; // Wait before compute stage
  submitInfo.waitSemaphoreCount = 1;         // Number of semaphores to wait for
  submitInfo.pWaitSemaphores =
      waitSemaphores; // Array of semaphores to wait for
  submitInfo.pWaitDstStageMask =
      waitStages; // Which pipeline stages wait for semaphores
  submitInfo.commandBufferCount = 1; // Number of command buffers to submit
  submitInfo.pCommandBuffers =
      &_commandBuffers[imageIndex]; // Array of command buffers to submit

  if (vkQueueSubmit(_vkContext.GetGraphicsQueue(), // Queue to submit to
                    1,                             // Number of submits
                    &submitInfo,                   // Submit info
                    _fences[_currentFrame]) !=
      VK_SUCCESS) { // Fence to signal when done
    throw std::runtime_error("Failed to submit compute command buffer!");
  }

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
  _currentFrame = (_currentFrame + 1) % _fences.size();
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
  if (bufferName == "viewProjection")
    return _gaussianBuffers.viewProjection;

  throw std::runtime_error("Unknown buffer name: " + bufferName);
}

void ComputePipeline::RecordAllCommandBuffers() {
  std::cout << "  - Pre-recording command buffers..." << std::endl;

  uint32_t swapchainImageCount =
      static_cast<uint32_t>(_vkContext.GetSwapchainImages().size());

  for (uint32_t i = 0; i < swapchainImageCount; i++) {
    RecordCommandBufferForImage(i);
  }

  std::cout << " All " << swapchainImageCount
            << " command buffers pre-recorded!" << std::endl;
}

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
                         ? 2 * sizeof(glm::mat4)
                         : VK_WHOLE_SIZE;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = _descriptorSets[pType][i]; // Specific descriptor set
  descriptorWrite.dstBinding = bindingIndex;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = descriptorType;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &bufferInfo;

  vkUpdateDescriptorSets(_vkContext.GetLogicalDevice(), 1, &descriptorWrite, 0,
                         nullptr);

  std::cout << "Descriptor set " << i << "  Buffer " << i << "type"
            << static_cast<int>(descriptorType) << std::endl;
}
