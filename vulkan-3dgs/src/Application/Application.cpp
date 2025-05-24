#include "GaussianRenderer.h"
#include "PLYLoader.h"
#include "VulkanContext.h"
#include "Window.h"
#include <filesystem>

int main() {

  std::unique_ptr<GaussianBase> gaussianData =
      PLYLoader::LoadPLY("../point_cloud0.ply", 1);

  WindowManager windowManager("Vulkan 3DGS API");
  windowManager.InitWindow();
  VulkanContext vkContext(windowManager.getWindow());
  vkContext.InitContext();
  // Check if EXIT_FAILURE
  GaussianRenderer renderPipeline(vkContext);
  renderPipeline.LoadGaussianData(std::move(gaussianData));

  while (windowManager.IsActive()) {
    glfwPollEvents();
    /* renderer.draw();*/
  }
  // renderer.cleanUp();
  glfwTerminate();
  return 0;
}