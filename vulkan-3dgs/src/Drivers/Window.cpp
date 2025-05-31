// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "Window.h"

void WindowManager::InitWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  _window = glfwCreateWindow(_w, _h, _wName.c_str(), nullptr, nullptr);
  glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  g_renderSettings.width = _w;
  g_renderSettings.height = _h;
}

void WindowManager::DestroyWindow() { glfwDestroyWindow(_window); }

bool WindowManager::IsActive() const {
  return _window && !glfwWindowShouldClose(_window);
}
