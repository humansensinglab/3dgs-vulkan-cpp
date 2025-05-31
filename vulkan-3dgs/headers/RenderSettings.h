#pragma once
#include "glm/glm.hpp"
struct RenderSettings {

  // Display
  int numGaussians;
  int numRendered;
  int width;
  int height;
  glm::vec3 pos;

  float speed = 1.0f;
  float mouseSensitivity = 1.0f;
  float fov = 60.0f;
  float nearPlane = 0.1f;
  float farPlane = 1000.0f;
  bool enableCulling = true;
  int tileSize = 16;

  float gaussianScale = 1.0f;
  bool showWireframe = false;

  float exposure = 1.0f;
  float gamma = 2.2f;
  float saturation = 1.0f;

  bool camRotationActive = false;
};

// Global instance
extern RenderSettings g_renderSettings;