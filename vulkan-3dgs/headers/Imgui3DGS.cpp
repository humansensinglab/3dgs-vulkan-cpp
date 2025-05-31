#include "Imgui3DGS.h"

void ImguiUI::Init() {

  CreateRenderPass();
  CreateFrameBuffers();
  CreateDescriporPool();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForVulkan(_vkContext.getWindow(), true);

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = _vkContext.GetInstance();
  init_info.PhysicalDevice = _vkContext.GetPhysicalDevice();
  init_info.Device = _vkContext.GetLogicalDevice();
  init_info.QueueFamily = _vkContext.GetGraphicsQueueFamily(); // You need this!
  init_info.Queue = _vkContext.GetGraphicsQueue();
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = _descriptorPool; // You need this too!
  init_info.Subpass = 0;
  init_info.MinImageCount = _vkContext.GetSwapchainImages().size();
  init_info.ImageCount = _vkContext.GetSwapchainImages().size();
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = nullptr;

  ImGui_ImplVulkan_Init(&init_info, _renderPass); // Uncomment this!

  // Simplified font upload - newer API handles command buffers internally
  ImGui_ImplVulkan_CreateFontsTexture();
}

void ImguiUI::CreateRenderPass() {
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = _vkContext.swapchainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp =
      VK_ATTACHMENT_LOAD_OP_LOAD; // Load existing content from compute
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

  if (vkCreateRenderPass(_vkContext.device, &renderPassInfo, nullptr,
                         &_renderPass) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create ImGui render pass!");
  }
}

void ImguiUI::CreateFrameBuffers() {
  _framebuffers.resize(_vkContext.swapchainImageCount);

  for (size_t i = 0; i < _vkContext.swapchainImageCount; i++) {
    VkImageView attachments[] = {
        _vkContext.swapchainImageViews[i]}; // Use your existing image views

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = _vkContext.swapchainExtent.width;
    framebufferInfo.height = _vkContext.swapchainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(_vkContext.device, &framebufferInfo, nullptr,
                            &_framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create ImGui framebuffer!");
    }
  }
}

void ImguiUI::CreateDescriporPool() {
  std::array<VkDescriptorPoolSize, 11> poolSizes = {
      {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
       {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
       {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
       {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
       {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
       {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}}};

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets = 1000 * poolSizes.size();
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();

  if (vkCreateDescriptorPool(_vkContext.device, &poolInfo, nullptr,
                             &_descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create ImGui descriptor pool!");
  }
}

void ImguiUI::CleanUp() {}
