// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "Camera.h"

Camera::Camera(int w, int h, float fov, float aspectRatio, float nearPlane,
               float farPlane)
    : _pos(glm::vec3(-0.53f, -0.25f, 33.74f)),
      _front(glm::vec3(0.0f, 0.0f, 1.0f)),
      _worldUp(glm::vec3(0.0f, -1.0f, 0.0f)), _w(w),
      _h(h) // Start looking forward (negative Z)
      ,
      _fov(fov), _aspectRatio(aspectRatio), _nearPlane(nearPlane),
      _farPlane(farPlane), _moveSpeed(5.0f), _mouseSensitivity(0.1f) {
  g_renderSettings.pos = _pos;
  g_renderSettings.nearPlane = nearPlane;
  g_renderSettings.farPlane = farPlane;
  UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {

  return glm::lookAtLH(_pos, _pos - _front, _up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
  return glm::perspective(glm::radians(g_renderSettings.fov), _aspectRatio,
                          g_renderSettings.nearPlane,
                          g_renderSettings.farPlane);
}

void Camera::ProcessMouseMovement(float deltaX, float deltaY,
                                  bool constrainPitch) {
  deltaX *= g_renderSettings.mouseSensitivity;
  deltaY *= g_renderSettings.mouseSensitivity;

  glm::quat yawRotation = glm::angleAxis(glm::radians(-deltaX), GetUpVector());

  // Pitch (up/down) around camera's current RIGHT vector
  glm::quat pitchRotation =
      glm::angleAxis(glm::radians(-deltaY), GetRightVector());

  // Apply rotations to current orientation (LOCAL SPACE)
  _orientation = yawRotation * _orientation * pitchRotation;
  _orientation = glm::normalize(_orientation);

  // Optional: Constrain pitch if needed
  if (constrainPitch) {
    // Convert back to Euler to check pitch constraint
    glm::vec3 euler = glm::eulerAngles(_orientation);
    float pitch = glm::degrees(euler.x);

    if (pitch > 89.0f || pitch < -89.0f) {
      // Revert the pitch rotation if it goes too far
      _orientation = yawRotation * _orientation; // Only apply yaw
    }
  }

  UpdateCameraVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
  float velocity = g_renderSettings.speed * deltaTime;

  switch (direction) {
  case CameraMovement::FORWARD:
    _pos += _front * velocity;
    break;
  case CameraMovement::BACKWARD:
    _pos -= _front * velocity;
    break;
  case CameraMovement::LEFT:
    _pos -= _right * velocity;
    break;
  case CameraMovement::RIGHT:
    _pos += _right * velocity;
    break;
  case CameraMovement::UP:
    _pos += _worldUp * velocity;
    break;
  case CameraMovement::DOWN:
    _pos -= _worldUp * velocity;
    break;
  }
  g_renderSettings.pos = _pos;
}

void Camera::UpdateAspectRatio(float newAspectRatio) {
  _aspectRatio = newAspectRatio;
}

CameraUniforms Camera::getUniforms() {
  // a lot of this can be just stored in first frame... for now whatever....
  float fov_radians = glm::radians(g_renderSettings.fov);

  float fov_y = 2.0f * atan(tan(fov_radians / 2.0f) / _aspectRatio);

  _uniforms.focal_x = _w / (2.0f * tan(fov_radians / 2.0f));
  _uniforms.focal_y = _h / (2.0f * tan(fov_y / 2.0f));

  _uniforms.tan_fovx = tan(fov_radians / 2.0f);
  _uniforms.tan_fovy = tan(fov_y / 2.0f);

  _uniforms.imageWidth = _w;
  _uniforms.imageHeight = _h;

  _uniforms.camPos = _pos;
  _uniforms.viewMatrix = GetViewMatrix();
  _uniforms.projMatrix = GetProjectionMatrix();

  return _uniforms;
}

void Camera::UpdateCameraVectors() {

  _front = GetForwardVector();
  _right = GetRightVector();
  _up = GetUpVector();

  _front = glm::normalize(_front);
  _right = glm::normalize(_right);
  _up = glm::normalize(_up);
}
