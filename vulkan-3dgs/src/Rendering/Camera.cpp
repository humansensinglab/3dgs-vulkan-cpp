// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "Camera.h"

Camera::Camera(int w, int h, float fov, float aspectRatio, float nearPlane,
               float farPlane)
    : _pos(glm::vec3(0, 2, -2.5)), _front(glm::vec3(0.0f, 0.0f, -1.0f)),
      _worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), _w(w),
      _h(h) // Start looking forward (negative Z)
      ,
      _yaw(-280.0f), _pitch(-10.0f), _fov(fov), _aspectRatio(aspectRatio),
      _nearPlane(nearPlane), _farPlane(farPlane), _moveSpeed(5.0f),
      _mouseSensitivity(0.1f) {
  g_renderSettings.yaw = _yaw;
  g_renderSettings.pitch = _pitch;
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

  if (g_renderSettings.playing) {
    return;
  }
  // Standard FPS camera - rotate around world axes
  _yaw += deltaX;
  _pitch += deltaY;

  // Constrain pitch to prevent camera flipping
  if (constrainPitch) {
    if (_pitch > 89.0f)
      _pitch = 89.0f;
    if (_pitch < -89.0f)
      _pitch = -89.0f;
  }

  g_renderSettings.yaw = _yaw;
  g_renderSettings.pitch = _pitch;
  UpdateCameraVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
  float velocity = g_renderSettings.speed * deltaTime;
  if (g_renderSettings.playing) {
    _pos = g_renderSettings.pos;
    return;
  }
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
  int mult = 1;

  float fov_y = 2.0f * atan(tan(fov_radians / 2.0f) / _aspectRatio);

  _uniforms.focal_x = _w / (2.0f * mult * tan(fov_radians / 2.0f));
  _uniforms.focal_y = _h / (2.0f * mult * tan(fov_y / 2.0f));

  _uniforms.tan_fovx = tan(fov_radians / 2.0f);
  _uniforms.tan_fovy = tan(fov_y / 2.0f);

  _uniforms.imageWidth = _w;
  _uniforms.imageHeight = _h;

  _uniforms.camPos = g_renderSettings.pos;
  if (g_renderSettings.playing) {
    _yaw = g_renderSettings.yaw;
    _pitch = g_renderSettings.pitch;
    UpdateCameraVectors();
  }
  _uniforms.viewMatrix = GetViewMatrix();
  _uniforms.projMatrix = GetProjectionMatrix();

  return _uniforms;
}

void Camera::UpdateCameraVectors() {

  glm::vec3 front;
  front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
  front.y = sin(glm::radians(_pitch));
  front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));

  _front = glm::normalize(front);

  _right = glm::normalize(
      glm::cross(_front, glm::vec3(0.0f, 1.0f, 0.0f))); // World up
  _up = glm::normalize(glm::cross(_right, _front));
}
