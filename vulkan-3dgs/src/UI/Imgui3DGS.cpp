#include "Imgui3DGS.h"

void ImguiUI::Init() {

  CreateRenderPass();
  CreateFrameBuffers();
  CreateDescriporPool();
  // CreateCommandBuffers();

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
  init_info.QueueFamily = _vkContext.GetGraphicsFamily();
  init_info.Queue = _vkContext.GetGraphicsQueue();
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = _descriptorPool;
  init_info.Subpass = 0;
  init_info.MinImageCount = uint32_t(_vkContext.GetSwapchainImages().size());
  init_info.ImageCount = uint32_t(_vkContext.GetSwapchainImages().size());
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = nullptr;
  init_info.RenderPass = _renderPass;

  ImGui_ImplVulkan_Init(&init_info);

  ImGui_ImplVulkan_CreateFontsTexture();

  int windowResize = 1;
#ifdef __APPLE__
  windowResize = 2;
#endif
  _w = _vkContext.GetSwapchainExtent().width / windowResize;
  _h = _vkContext.GetSwapchainExtent().height / windowResize;
}

void ImguiUI::CreateRenderPass() {

  VkDevice device = _vkContext.GetLogicalDevice();
  std::cout << "Device handle: " << device << std::endl;

  if (device == VK_NULL_HANDLE) {
    throw std::runtime_error("Device is VK_NULL_HANDLE!");
  }

  // Debug: Check if we can query device properties (device is alive)
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(_vkContext.GetPhysicalDevice(), &properties);
  std::cout << "Device name: " << properties.deviceName << std::endl;

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = _vkContext.GetSwapchainFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
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

  if (vkCreateRenderPass(_vkContext.GetLogicalDevice(), &renderPassInfo,
                         nullptr, &_renderPass) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create ImGui render pass!");
  }
  std::cout << "IMGui Render Pass created" << std::endl;
}

void ImguiUI::CreateFrameBuffers() {
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
      throw std::runtime_error("Failed to create ImGui framebuffer!");
    }
  }
  std::cout << "IMGui Frame Buffers created" << std::endl;
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
  poolInfo.maxSets = 1000 * uint32_t(poolSizes.size());
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();

  if (vkCreateDescriptorPool(_vkContext.GetLogicalDevice(), &poolInfo, nullptr,
                             &_descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create ImGui descriptor pool!");
  }

  std::cout << "IMGui Descriptor Pool Created" << std::endl;
}

void ImguiUI::CreateCommandBuffers() {

  _commandBuffers.resize(_vkContext.GetSwapchainImages().size());

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = _vkContext.GetCommandPool();
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

  vkAllocateCommandBuffers(_vkContext.GetLogicalDevice(), &allocInfo,
                           _commandBuffers.data());
}

void ImguiUI::RecordImGuiRenderPass(VkCommandBuffer commandBuffer,
                                    uint32_t imageIndex) {
  // Begin render pass
  VkRenderPassBeginInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = _renderPass;
  renderPassInfo.framebuffer = _frameBuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = _vkContext.GetSwapchainExtent();

  renderPassInfo.clearValueCount = 0;
  renderPassInfo.pClearValues = nullptr;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

  vkCmdEndRenderPass(commandBuffer);
}

// Also add the NewFrame and UI creation methods for completeness
void ImguiUI::NewFrame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImguiUI::CreateUI(Camera &cam) {

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(std::min(_w / 3, 350), std::min(_h, 800)),
                           ImGuiCond_Once);
  ImGui::Begin("3D Gaussian Splatting");

  // FPS
  {
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                       "Performance"); // Yellow header
    ImGui::Separator();
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "GPU Memory");
    ImGui::Separator();
  }

  // MEM
  {

    static GPUMemoryInfo::MemoryStats memStats;
    static auto lastUpdate = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate)
            .count() >= 1) {
      memStats = GPUMemoryInfo::GetCompleteMemoryStats(
          _vkContext.GetPhysicalDevice(), _vkContext.GetLogicalDevice());
      lastUpdate = now;
    }
    if (memStats.hasDetailedInfo) {
      ImGui::Text("Used: %s",
                  GPUMemoryInfo::FormatBytes(memStats.usedMemory).c_str());
      ImGui::Text("Available: %s",
                  GPUMemoryInfo::FormatBytes(memStats.availableMemory).c_str());

      // Usage bar
      float usage = static_cast<float>(memStats.usedMemory) /
                    static_cast<float>(memStats.totalMemory);
      ImGui::ProgressBar(usage, ImVec2(-1, 0), "");
      ImGui::SameLine(0, 10);
      ImGui::Text("%.1f%%", usage * 100.0f);
    } else {
      ImGui::Text("(Detailed usage info not available)");
    }
  }
  ImGui::Spacing();

  // Display Info II
  {
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Display Info");
    ImGui::Separator();
    int resize = 1;
#ifdef __APPLE__
    resize = 2;
#endif
    ImGui::Text("Resolution: %d x %d", g_renderSettings.width * resize,
                g_renderSettings.height * resize);
    ImGui::Text("Number of Gaussians: %d", g_renderSettings.numGaussians);
    ImGui::Text("Number of Rendered Splats: %d", g_renderSettings.numRendered);

    ImGui::Spacing();
  }

  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Camera");
  ImGui::Separator();
  ImGui::Text("Camera Rotation: ");
  ImGui::SameLine();
  if (g_renderSettings.camRotationActive) {
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ACTIVE");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "(Hold Right Mouse)");
  } else {
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "INACTIVE");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "(Hold Right Mouse)");
  }
  ImGui::Separator();
  ImGui::PushItemWidth(120);
  ImGui::Text("Position: (%.1f,%.1f,%.1f)", g_renderSettings.pos.x,
              g_renderSettings.pos.y, g_renderSettings.pos.z);
  ImGui::Text("Rotation: Yaw: %.1f, Pitch: %.1f", g_renderSettings.yaw,
              g_renderSettings.pitch);
  ImGui::Separator();

  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Parameters");
  ImGui::BeginDisabled(g_renderSettings.playing);
  ImGui::SliderFloat("FOV", &g_renderSettings.fov, 30.0f, 120.0f, "%.1f");
  ImGui::SliderFloat("Sensitivity", &g_renderSettings.mouseSensitivity, 0.01f,
                     0.4f, "%.2f");
  ImGui::SliderFloat("Speed", &g_renderSettings.speed, 1.0f, 10.0f, "%.1f");

  ImGui::Separator();
  ImGui::Checkbox("Distance Culling", &g_renderSettings.enableCulling);
  ImGui::BeginDisabled(!g_renderSettings.enableCulling);
  ImGui::SliderFloat("Near Plane", &g_renderSettings.nearPlane, 0.01f, 50.0f,
                     "%.1f");
  ImGui::SliderFloat("Far Plane", &g_renderSettings.farPlane, 1.0f, 300.0f,
                     "%.1f");
  ImGui::EndDisabled();
  ImGui::PopItemWidth();
  ImGui::Separator();
  ImGui::BeginDisabled(true);
  ImGui::SliderInt("TileSize", &g_renderSettings.tileSize, 8, 256);
  ImGui::Button("Size Culling: ACTIVE");
  ImGui::EndDisabled();
  ImGui::Separator();
  ImGui::Checkbox("Wireframe Mode", &g_renderSettings.showWireframe);
  ImGui::BeginDisabled(!g_renderSettings.showWireframe);
  ImGui::SliderFloat("Gaussian Scale", &g_renderSettings.gaussianScale, 0.01f,
                     10.0f, "%.1f");
  ImGui::EndDisabled();
  ImGui::Separator();
  ImGui::Checkbox("Sequence Mode", &g_renderSettings.sequenceMode);
  ImGui::Separator();
  ImGui::EndDisabled();

  ImGui::Separator();
  static float rotX = 0.0f;
  static float rotY = 0.0f;
  static float rotZ = 0.0f;

  ImGui::Spacing();

  if (ImGui::Button("Set Current View as Reference")) {
    // Call camera's SetNewReference method
    cam.SetNewReference(); // You'll need access to camera instance
  }

  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                     "(Sets current orientation as new 'up')");

  ImGui::Spacing();

  // Controls
  {
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Controls");
    ImGui::Separator();
    ImGui::Text("Use WASD to move camera");
    ImGui::Text("C/Space to move up/down");
    ImGui::Text(
        "Use mouse to look around (Hold Right / Two fingers in trackpad)");
    ImGui::Text("Q/E to rotate(Roll)");
  }

  ImGui::End();

  ///////////////////////

  if (!g_renderSettings.sequenceMode)
    return;
  ImGui::SetNextWindowPos(ImVec2(_w - 400, _h - 300), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
  if (ImGui::Begin("Camera Sequence")) {

    ImGui::Separator();

    if (ImGui::Button("Add Keyframe")) {
      CameraKeyframe kf;
      kf.time = sequence.empty() ? 0.0f : sequence.back().time + 1.0f;
      kf.position = g_renderSettings.pos;
      kf.fov = g_renderSettings.fov;
      kf.wireframe = g_renderSettings.showWireframe;
      kf.nearPlane = g_renderSettings.nearPlane;
      kf.farPlane = g_renderSettings.farPlane;
      kf.pitch = g_renderSettings.pitch;
      kf.yaw = g_renderSettings.yaw;
      kf.worldUp = g_renderSettings.worldUp;
      kf.baseReference = g_renderSettings.baseReference;
      kf.currentReference = g_renderSettings.currentReference;
      kf.front = g_renderSettings.front;

      sequence.push_back(kf);
    }

    ImGui::SameLine();
    if (ImGui::Button("Play")) {
      g_renderSettings.playing = true;
      _seqRecorder.setSequence(&sequence);
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
      g_renderSettings.playing = false; // Fix: use g_renderSettings
    }

    // List keyframes
    ImGui::Separator();
    for (int i = 0; i < sequence.size(); i++) {
      ImGui::PushID(i);

      bool isSelected = (selectedKeyframe == i);
      if (ImGui::Selectable(("Keyframe " + std::to_string(i)).c_str(),
                            isSelected)) {
        selectedKeyframe = (selectedKeyframe == i) ? -1 : i;
      }

      if (isSelected) {
        ImGui::Text("Time: %.2f", sequence[i].time);
        ImGui::Text("Position: (%.1f, %.1f, %.1f)", sequence[i].position.x,
                    sequence[i].position.y, sequence[i].position.z);
        ImGui::SameLine();
        ImGui::Text("Pitch: %.2f, Yaw: %2f", sequence[i].pitch,
                    sequence[i].yaw);
        ImGui::Text("WorldUp: (%.1f, %.1f, %.1f)", sequence[i].worldUp.x,
                    sequence[i].worldUp.y, sequence[i].worldUp.z);
        ImGui::Text("FOV: %.1f", sequence[i].fov);
        ImGui::SameLine();
        ImGui::Text("Wireframe: %s", sequence[i].wireframe ? "Yes" : "No");
        ImGui::SameLine();
        ImGui::Text("Near: %.2f", sequence[i].nearPlane);
        ImGui::SameLine();
        ImGui::Text("Far: %.2f", sequence[i].farPlane);

        ImGui::Separator();
        ImGui::DragFloat("Adjust Time", &sequence[i].time, 0.1f, 0.0f, 100.0f);

        if (ImGui::Button("Delete")) {
          sequence.erase(sequence.begin() + i);
          selectedKeyframe = -1;
          ImGui::PopID();
          break;
        }
      }

      ImGui::PopID();
    }
  }
  ImGui::End();
}

void ImguiUI::CleanUp(){};
