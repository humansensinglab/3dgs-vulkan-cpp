#include "GraphicsPipeline.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

GraphicsPipeline::GraphicsPipeline(VulkanContext &vkContext)
    : _vkContext(vkContext) {}

GraphicsPipeline::~GraphicsPipeline() { CleanUp(); }

void GraphicsPipeline::Init() {
  _axisVertices = createAxisCubeVertices();
  CreateRenderPass();
  CreateFrameBuffers();
  CreateDescriptorPool();
  CreateDescriptorSetLayout();
  CreateUniformBuffers();
  CreateDescriptorSets();
  CreateVertexBuffer();
  CreateGraphicsPipeline();
}

void GraphicsPipeline::CreateRenderPass() {
  VkDevice device = _vkContext.GetLogicalDevice();

  // Same pattern as ImGui - load existing content, store result
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = _vkContext.GetSwapchainFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp =
      VK_ATTACHMENT_LOAD_OP_LOAD; // Load existing 3DGS content
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &_renderPass) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create axis render pass!");
  }
  std::cout << "Axis Render Pass created" << std::endl;
}

void GraphicsPipeline::CreateFrameBuffers() {
  _frameBuffers.resize(_vkContext.GetSwapchainImages().size());

  for (size_t i = 0; i < _vkContext.GetSwapchainImages().size(); i++) {
    VkImageView attachments[] = {_vkContext.GetSwapchainImages()[i].imageView};

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = _vkContext.GetSwapchainExtent().width;
    framebufferInfo.height = _vkContext.GetSwapchainExtent().height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(_vkContext.GetLogicalDevice(), &framebufferInfo,
                            nullptr, &_frameBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create axis framebuffer!");
    }
  }
  std::cout << "Axis Frame Buffers created" << std::endl;
}

void GraphicsPipeline::CreateDescriptorPool() {
  // Simpler than ImGui - just need uniform buffers
  VkDescriptorPoolSize poolSize = {};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount =
      static_cast<uint32_t>(_vkContext.GetSwapchainImages().size());

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets =
      static_cast<uint32_t>(_vkContext.GetSwapchainImages().size());

  if (vkCreateDescriptorPool(_vkContext.GetLogicalDevice(), &poolInfo, nullptr,
                             &_descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create axis descriptor pool!");
  }

  std::cout << "Axis Descriptor Pool Created" << std::endl;
}

void GraphicsPipeline::CreateDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uboLayoutBinding = {};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;

  if (vkCreateDescriptorSetLayout(_vkContext.GetLogicalDevice(), &layoutInfo,
                                  nullptr,
                                  &_descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create axis descriptor set layout!");
  }
}

void GraphicsPipeline::CreateUniformBuffers() {
  VkDeviceSize bufferSize = sizeof(AxisUBO);

  _uniformBuffers.resize(_vkContext.GetSwapchainImages().size());

  for (size_t i = 0; i < _vkContext.GetSwapchainImages().size(); i++) {
    _uniformBuffers[i] = _bufferManager->CreateUniformBuffer(
        _vkContext.GetLogicalDevice(), _vkContext.GetPhysicalDevice(),
        bufferSize);
  }
  std::cout << "Axis Uniform Buffers" << std::endl;
}

void GraphicsPipeline::CreateDescriptorSets() {
  std::vector<VkDescriptorSetLayout> layouts(
      _vkContext.GetSwapchainImages().size(), _descriptorSetLayout);

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount =
      static_cast<uint32_t>(_vkContext.GetSwapchainImages().size());
  allocInfo.pSetLayouts = layouts.data();

  _descriptorSets.resize(_vkContext.GetSwapchainImages().size());
  if (vkAllocateDescriptorSets(_vkContext.GetLogicalDevice(), &allocInfo,
                               _descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate axis descriptor sets!");
  }
  std::cout << "Axis descriptor Sets" << std::endl;

  for (size_t i = 0; i < _vkContext.GetSwapchainImages().size(); i++) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = _uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(AxisUBO);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = _descriptorSets[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(_vkContext.GetLogicalDevice(), 1, &descriptorWrite,
                           0, nullptr);
  }
}

void GraphicsPipeline::CreateVertexBuffer() {
  VkDeviceSize bufferSize = sizeof(_axisVertices[0]) * _axisVertices.size();

  _vertexBuffer = _bufferManager->CreateVertexBuffer(
      _vkContext.GetLogicalDevice(), _vkContext.GetPhysicalDevice(),
      bufferSize);

  void *data;
  vkMapMemory(_vkContext.GetLogicalDevice(),
              _bufferManager->GetBufferMemory(_vertexBuffer), 0, bufferSize, 0,
              &data);
  memcpy(data, _axisVertices.data(), (size_t)bufferSize);
  vkUnmapMemory(_vkContext.GetLogicalDevice(),
                _bufferManager->GetBufferMemory(_vertexBuffer));
}

void GraphicsPipeline::CreateGraphicsPipeline() {
  // Load shaders (you'll need to implement these)
  auto vertShaderCode =
      ReadFile(g_renderSettings.shaderPath + "Shaders/axis_vert.spv");
  auto fragShaderCode =
      ReadFile(g_renderSettings.shaderPath + "Shaders/axis_frag.spv");

  VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
  VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  // Vertex input
  VkVertexInputBindingDescription bindingDescription = {};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(AxisVertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(AxisVertex, position);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(AxisVertex, color);

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  // Input assembly - LINE topology for axes
  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Viewport and scissor
  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)_vkContext.GetSwapchainExtent().width;
  viewport.height = (float)_vkContext.GetSwapchainExtent().height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = _vkContext.GetSwapchainExtent();

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  // Rasterizer
  // VkPipelineRasterizationStateCreateInfo rasterizer = {};
  // rasterizer.sType =
  // VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  // rasterizer.depthClampEnable = VK_FALSE;
  // rasterizer.rasterizerDiscardEnable = VK_FALSE;
  // rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  // rasterizer.lineWidth = 2.0f; // Thicker lines for better visibility
  // rasterizer.cullMode = VK_CULL_MODE_NONE;
  // rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  // rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  // Color blending - enable alpha blending
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;

  // Pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

  if (vkCreatePipelineLayout(_vkContext.GetLogicalDevice(), &pipelineLayoutInfo,
                             nullptr, &_pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create axis pipeline layout!");
  }

  // Create graphics pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = _pipelineLayout;
  pipelineInfo.renderPass = _renderPass;
  pipelineInfo.subpass = 0;

  if (vkCreateGraphicsPipelines(_vkContext.GetLogicalDevice(), VK_NULL_HANDLE,
                                1, &pipelineInfo, nullptr,
                                &_graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create axis graphics pipeline!");
  }

  vkDestroyShaderModule(_vkContext.GetLogicalDevice(), fragShaderModule,
                        nullptr);
  vkDestroyShaderModule(_vkContext.GetLogicalDevice(), vertShaderModule,
                        nullptr);

  std::cout << "Axis Graphics Pipeline created" << std::endl;
}

void GraphicsPipeline::RecordAxisRenderPass(VkCommandBuffer commandBuffer,
                                            uint32_t imageIndex, Camera &cam) {
  // Update uniform buffer

  UpdateUniformBuffer(imageIndex, cam.GetCurrentRotationForGizmo());

  // Begin render pass
  VkRenderPassBeginInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = _renderPass;
  renderPassInfo.framebuffer = _frameBuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = _vkContext.GetSwapchainExtent();
  renderPassInfo.clearValueCount =
      0; // No clear since we're loading existing content
  renderPassInfo.pClearValues = nullptr;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    _graphicsPipeline);

  VkBuffer vertexBuffers[] = {_vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          _pipelineLayout, 0, 1, &_descriptorSets[imageIndex],
                          0, nullptr);

  vkCmdDraw(commandBuffer, static_cast<uint32_t>(_axisVertices.size()), 1, 0,
            0);

  vkCmdEndRenderPass(commandBuffer);
}

void GraphicsPipeline::UpdateUniformBuffer(uint32_t imageIndex,
                                           const glm::mat4 &mvpMatrix) {
  AxisUBO ubo = {};
  ubo.mvp = mvpMatrix;

  void *data;
  vkMapMemory(_vkContext.GetLogicalDevice(),
              _bufferManager->GetBufferMemory(_uniformBuffers[imageIndex]), 0,
              sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(_vkContext.GetLogicalDevice(),
                _bufferManager->GetBufferMemory(_uniformBuffers[imageIndex]));
}

VkShaderModule
GraphicsPipeline::CreateShaderModule(const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(_vkContext.GetLogicalDevice(), &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module!");
  }

  return shaderModule;
}

void GraphicsPipeline::CleanUp() {
  VkDevice device = _vkContext.GetLogicalDevice();

  // Clean up uniform buffers
  for (size_t i = 0; i < _uniformBuffers.size(); i++) {
    vkDestroyBuffer(device, _uniformBuffers[i], nullptr);
    vkFreeMemory(device, _bufferManager->GetBufferMemory(_uniformBuffers[i]),
                 nullptr);
  }

  // Clean up vertex buffer
  vkDestroyBuffer(device, _vertexBuffer, nullptr);
  vkFreeMemory(device, _bufferManager->GetBufferMemory(_vertexBuffer), nullptr);

  // Clean up pipeline
  vkDestroyPipeline(device, _graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);

  // Clean up descriptor set layout
  vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);

  // Clean up descriptor pool
  vkDestroyDescriptorPool(device, _descriptorPool, nullptr);

  // Clean up framebuffers
  for (auto framebuffer : _frameBuffers) {
    vkDestroyFramebuffer(device, framebuffer, nullptr);
  }

  // Clean up render pass
  vkDestroyRenderPass(device, _renderPass, nullptr);
}

std::vector<AxisVertex> GraphicsPipeline::createAxisCubeVertices() {
  std::vector<AxisVertex> vertices;

  const float axisLength = 1.0f;
  const float thickness = 0.04f;

  // Colors for each axis
  const glm::vec3 xColor = {1.0f, 0.0f, 0.0f};
  const glm::vec3 yColor = {0.0f, 1.0f, 0.0f};
  const glm::vec3 zColor = {0.0f, 0.0f, 1.0f};

  auto addCube = [&](glm::vec3 center, glm::vec3 size, glm::vec3 color) {
    glm::vec3 halfSize = size * 0.5f;

    std::array<glm::vec3, 8> cubeVertices = {
        {center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
         center + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
         center + glm::vec3(halfSize.x, halfSize.y, -halfSize.z),
         center + glm::vec3(-halfSize.x, halfSize.y, -halfSize.z),
         center + glm::vec3(-halfSize.x, -halfSize.y, halfSize.z),
         center + glm::vec3(halfSize.x, -halfSize.y, halfSize.z),
         center + glm::vec3(halfSize.x, halfSize.y, halfSize.z),
         center + glm::vec3(-halfSize.x, halfSize.y, halfSize.z)}};

    std::array<std::array<int, 3>, 36> triangles = {{// Front face (-Z)
                                                     {{0, 2, 1}},
                                                     {{0, 3, 2}},
                                                     // Back face (+Z)
                                                     {{4, 5, 6}},
                                                     {{4, 6, 7}},
                                                     // Left face (-X)
                                                     {{0, 7, 3}},
                                                     {{0, 4, 7}},
                                                     // Right face (+X)
                                                     {{1, 2, 6}},
                                                     {{1, 6, 5}},
                                                     // Top face (+Y)
                                                     {{3, 6, 2}},
                                                     {{3, 7, 6}},
                                                     // Bottom face (-Y)
                                                     {{0, 1, 5}},
                                                     {{0, 5, 4}}}};

    for (const auto &tri : triangles) {
      for (int i : tri) {
        vertices.push_back({cubeVertices[i], color});
      }
    }
  };

  addCube(glm::vec3(axisLength * 0.5f, 0.0f, 0.0f),
          glm::vec3(axisLength, thickness, thickness), xColor);

  addCube(glm::vec3(0.0f, axisLength * 0.5f, 0.0f),
          glm::vec3(thickness, axisLength, thickness), yColor);

  addCube(glm::vec3(0.0f, 0.0f, axisLength * 0.5f),
          glm::vec3(thickness, thickness, axisLength), zColor);

  return vertices;
}