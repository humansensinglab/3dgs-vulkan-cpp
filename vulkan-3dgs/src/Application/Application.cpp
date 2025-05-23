#include "VulkanContext.h"
#include "Window.h"

int main() {
  // Load PCloud
  WindowManager windowManager("Vulkan 3DGS API");
  windowManager.InitWindow();
  VulkanContext vkContext(windowManager.getWindow());
  vkContext.InitContext();
  // Check if EXIT_FAILURE

  while (windowManager.IsActive()) {
    glfwPollEvents();
    /* renderer.draw();*/
  }
  // renderer.cleanUp();
  glfwTerminate();
  return 0;
}