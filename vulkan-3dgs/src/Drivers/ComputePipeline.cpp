#include "ComputePipeline.h"

void ComputePipeline::Initialize() {
  std::cout << "\n === Compute Pipeline Initalization === \n" << std::endl;
  CreateCommandBuffers();
  CreateSynchronization();
  CreateDescriptorSetLayout();
  CreateDescriptorPool();
}

void ComputePipeline::CreateCommandBuffers() {

  int size_sw = 1;
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

  _semaphores.resize(1); // When swapchain image is ready
  _fences.resize(1);     // When compute work is done

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < 1; i++) {

    if (vkCreateSemaphore(_vkContext.GetLogicalDevice(), &semaphoreInfo,
                          nullptr, &_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(_vkContext.GetLogicalDevice(), &fenceInfo, nullptr,
                      &_fences[0]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create synchronization objects!");
    }
  }
  std::cout << " Created Sempahores and Fences " << std::endl;
}

void ComputePipeline::CreateDescriptorSetLayout() {

  std::vector<VkDescriptorSetLayoutBinding> vulkanBindings;
  vulkanBindings.reserve(PIPELINE_BINDINGS.size());

  for (const auto &binding : PIPELINE_BINDINGS) {
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
                                  nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }

  std::cout << " Descriptor set layout created (" << vulkanBindings.size()
            << " bindings)" << std::endl;
}

void ComputePipeline::CreateDescriptorPool() {

  std::map<VkDescriptorType, uint32_t> typeCounts;

  for (const auto &binding : PIPELINE_BINDINGS) {
    typeCounts[binding.type] += binding.count;
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
  poolInfo.maxSets = 1;

  if (vkCreateDescriptorPool(_vkContext.GetLogicalDevice(), &poolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool!");
  }

  std::cout << " Descriptor Pool created with " << poolSizes.size()
            << " different Pool Size(s)" << std::endl;
}