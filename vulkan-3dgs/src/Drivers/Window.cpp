// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "Window.h"

void WindowManager::InitWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  _window = glfwCreateWindow(_w, _h, _wName.c_str(), nullptr, nullptr);
}

void WindowManager::DestroyWindow() { glfwDestroyWindow(_window); }

bool WindowManager::IsActive() const {
  return _window && !glfwWindowShouldClose(_window);
}
