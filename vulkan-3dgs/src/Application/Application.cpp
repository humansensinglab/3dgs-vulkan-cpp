#include "GaussianRenderer.h"
#include "PLYLoader.h"
#include "VulkanContext.h"
#include "Window.h"
#include <chrono>
#include <filesystem>
#include <iostream>

int main() {

  // Load PLY
  int shDegree = 3;
  std::unique_ptr<GaussianBase> gaussianData =
      PLYLoader::LoadPLY("../point_cloud2.ply", shDegree);

  // window
  WindowManager windowManager("Vulkan 3DGS API");
  windowManager.InitWindow();
  int width, height;
  glfwGetFramebufferSize(windowManager.getWindow(), &width, &height);

  // Vulkan context
  VulkanContext vkContext(windowManager.getWindow());
  vkContext.InitContext();

  // init renderPipeline
  GaussianRenderer renderPipeline(vkContext, shDegree);
  renderPipeline.InitializeCamera(static_cast<float>(width),
                                  static_cast<float>(height));
  renderPipeline.LoadGaussianData(std::move(gaussianData));
  renderPipeline.CreateBuffers();
  renderPipeline.InitComputePipeline();

  auto lastTime = std::chrono::high_resolution_clock::now();

  while (windowManager.IsActive()) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    double deltaTime =
        std::chrono::duration<double>(currentTime - lastTime).count();
    double fps = 1.0 / deltaTime;
    std::cout << "FPS: " << fps << std::endl;
    lastTime = currentTime;

    glfwPollEvents();
    renderPipeline.processInput(static_cast<float>(deltaTime));
    renderPipeline.UpdateCameraUniforms();
    renderPipeline.Render();
  }

  glfwTerminate();
  return 0;
}