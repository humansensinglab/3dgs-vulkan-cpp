// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "Application.h"
#include <iostream>

int main(int argc, char *argv[]) {

  InputArgs args = checkArgs(argc, argv);
  Application application(args);
  application.Start();

  while (application.IsRunning()) {

    application.Render();
  }

  glfwTerminate();
  return 0;
}
