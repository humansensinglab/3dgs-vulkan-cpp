// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "entrypoint_python.h"
#include "Application.h"

void renderGaussians(const std::string path, std::optional<uint32_t> width,
                     std::optional<uint32_t> height) {

  std::vector<std::string> args;
  args.push_back("vulkan-3dgs");
  args.push_back(path);

  if (width.has_value()) {
    args.push_back(std::to_string(width.value()));
  }

  if (height.has_value()) {
    args.push_back(std::to_string(height.value()));
  }

  std::vector<char *> argv;
  for (auto &arg : args) {
    argv.push_back(const_cast<char *>(arg.c_str()));
  }
  argv.push_back(nullptr);

  int argc = static_cast<int>(argv.size() - 1);

  auto args_ = checkArgs(argc, argv.data());
  if (!args_.has_value()) {
    return;
  }
  Application application(*args_);
  application.Start();

  while (application.IsRunning()) {

    application.Render();
  }

  glfwTerminate();
  return;
}
