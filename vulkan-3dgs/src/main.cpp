// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "Application.h"

int main(int argc, char *argv[]) {

  auto args = checkArgs(argc, argv);
  if (!args.has_value()) {
    return -1;
  }
  Application application(*args);
  application.Start();

  while (application.IsRunning()) {

    application.Render();
  }

  glfwTerminate();
  return 0;
}
