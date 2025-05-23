#pragma once
#include <fstream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct QueueFamilyIndices {
  int graphicsFamily = -1; // location
  int presentationFamily = -1;
  bool isValid() { return graphicsFamily >= 0 && presentationFamily >= 0; }
};