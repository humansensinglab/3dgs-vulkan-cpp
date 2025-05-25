#include "GaussianRenderer.h"
#include "PLYLoader.h"
#include "VulkanContext.h"
#include "Window.h"
#include <chrono>
#include <filesystem>
#include <iostream>

int main() {

  std::unique_ptr<GaussianBase> gaussianData =
      PLYLoader::LoadPLY("../point_cloud0.ply", 1);

  WindowManager windowManager("Vulkan 3DGS API");
  windowManager.InitWindow();

  glfwSwapInterval(0);

  VulkanContext vkContext(windowManager.getWindow());
  vkContext.InitContext();

  GaussianRenderer renderPipeline(vkContext);
  renderPipeline.LoadGaussianData(std::move(gaussianData));

  auto lastTime = std::chrono::high_resolution_clock::now();

  while (windowManager.IsActive()) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    double deltaTime =
        std::chrono::duration<double>(currentTime - lastTime).count();
    double fps = 1.0 / deltaTime;

    std::cout << "FPS: " << fps << std::endl;

    lastTime = currentTime;

    glfwPollEvents();
    renderPipeline.render();
  }

  glfwTerminate();
  return 0;
}