// #define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>

GLFWwindow *window;
// VulkanRender renderer;

void initWindow(std::string wName = "Test", const int w = 800,
                const int h = 600) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(w, h, wName.c_str(), nullptr, nullptr);
}

int main() {
  initWindow("3dgs Vulkan API");

  /*if (renderer.init(window) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }*/

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    /* renderer.draw();*/
  }
  // renderer.cleanUp();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}