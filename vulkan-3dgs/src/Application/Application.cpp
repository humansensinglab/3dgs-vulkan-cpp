// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "Application.h"

void Application::Start() {

  _gaussianData = PLYLoader::LoadPLY(_pointCloudFile, _degree);
  _windowManager.InitWindow();
  int width, height;
  glfwGetFramebufferSize(_windowManager.getWindow(), &width, &height);

  _vkContext.emplace(_windowManager.getWindow());
  _vkContext->InitContext();

  _renderPipeline.emplace(*_vkContext, _degree, _seqRecorder);
  _renderPipeline->InitializeCamera(static_cast<float>(width),
                                    static_cast<float>(height));
  _renderPipeline->LoadGaussianData(std::move(_gaussianData));
  _renderPipeline->CreateBuffers();
  _renderPipeline->InitComputePipeline();

  glfwSetInputMode(_windowManager.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Application::Render() {

  _frameTimer.UpdateTime();
  //_frameTimer.PrintStats();
  glfwPollEvents();

  _renderPipeline->processInput(static_cast<float>(_frameTimer.deltaTime));
  _renderPipeline->UpdateCameraUniforms();
  _renderPipeline->Render();

  if (g_renderSettings.playing)
    _seqRecorder.Play(static_cast<float>(_frameTimer.deltaTime));
}
