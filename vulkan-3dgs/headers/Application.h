#pragma once
#include "GaussianRenderer.h"
#include "Imgui3DGS.h"
#include "PLYLoader.h"
#include "VulkanContext.h"
#include "Window.h"
#include <chrono>
#include <filesystem>
#include <iostream>

class Application {
public:
  Application(InputArgs args)
      : _pointCloudFile(args.ply), _degree(args.degree),
        _windowManager("3DGS Vulkan"), _frameTimer() {}
  void Start();
  void Render();
  bool IsRunning() const { return _windowManager.IsActive(); }

private:
  const std::string _pointCloudFile;
  const int _degree;
  FrameTimer _frameTimer;
  WindowManager _windowManager;
  std::unique_ptr<GaussianBase> _gaussianData;
  std::optional<VulkanContext> _vkContext;
  std::optional<GaussianRenderer> _renderPipeline;
};