#include "Window.h"

int main() {
  // Load PCloud
  WindowManager windowManager("Vulkan 3DGS API");
  windowManager.InitWindow();

  /*if (renderer.init(windowManager.getWindow()) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }*/

  while (windowManager.IsActive()) {
    glfwPollEvents();
    /* renderer.draw();*/
  }
  // renderer.cleanUp();
  glfwTerminate();
  return 0;
}