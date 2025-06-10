// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "Camera.h"

Camera::Camera(int w, int h, float fov, float aspectRatio, float nearPlane,
               float farPlane)
    : _pos(glm::vec3(0, 2, -2.5)), _front(glm::vec3(0.0f, 0.0f, 1.0f)),
      _worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), _w(w), _h(h), _yaw(-280.0f),
      _pitch(-10.0f), _fov(fov), _aspectRatio(aspectRatio),
      _nearPlane(nearPlane), _farPlane(farPlane), _moveSpeed(5.0f),
      _mouseSensitivity(0.1f), _baseReference(glm::mat4(1.0f)),
      _currentReference(glm::mat4(1.0f)) {
  g_renderSettings.yaw = _yaw;
  g_renderSettings.pitch = _pitch;
  g_renderSettings.pos = _pos;
  g_renderSettings.nearPlane = nearPlane;
  g_renderSettings.farPlane = farPlane;
  g_renderSettings.baseReference = _baseReference;
  g_renderSettings.currentReference = _currentReference;

  g_renderSettings.worldUp = _worldUp;
  UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
  glm::mat4 view = glm::lookAtLH(_pos, _pos - _front, _worldUp);
  return view;
}

glm::mat4 Camera::GetProjectionMatrix() const {
  glm::mat4 proj =
      glm::perspective(glm::radians(g_renderSettings.fov), _aspectRatio,
                       g_renderSettings.nearPlane, g_renderSettings.farPlane);
  return proj;
}

void Camera::ProcessMouseMovement(float deltaX, float deltaY,
                                  bool constrainPitch) {
  deltaX *= g_renderSettings.mouseSensitivity;
  deltaY *= g_renderSettings.mouseSensitivity;

  if (g_renderSettings.playing) {
    return;
  }

  if (_baseReference != glm::mat4(1.0f)) {

    glm::mat3 refRotation = glm::mat3(_baseReference);

    glm::vec3 transformedUp = refRotation * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 yawRot =
        glm::rotate(glm::mat4(1.0f), glm::radians(-deltaX), transformedUp);

    glm::mat4 pitchRot =
        glm::rotate(glm::mat4(1.0f), glm::radians(-deltaY), _right);

    _front = glm::vec3(yawRot * pitchRot * glm::vec4(_front, 0.0f));
    _front = glm::normalize(_front);
    g_renderSettings.front = _front;

    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));

    UpdateAnglesFromVectors();
  } else {

    _yaw += deltaX;
    _pitch -= deltaY;

    if (_yaw >= 360.0f)
      _yaw -= 360.0f;
    if (_yaw < 0.0f)
      _yaw += 360.0f;

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
    _pos += _up * velocity;
    break;
  case CameraMovement::DOWN:
    _pos -= _up * velocity;
    break;
  case CameraMovement::ROLL_LEFT: {
    glm::mat4 rollRot =
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f * deltaTime), _front);
    _up = glm::vec3(rollRot * glm::vec4(_up, 0.0f));
    _right = glm::vec3(rollRot * glm::vec4(_right, 0.0f));
    _worldUp = _up;
    g_renderSettings.worldUp = _up;
    UpdateAnglesFromVectors();
  } break;
  case CameraMovement::ROLL_RIGHT: {
    glm::mat4 rollRot =
        glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f * deltaTime), _front);
    _up = glm::vec3(rollRot * glm::vec4(_up, 0.0f));
    _right = glm::vec3(rollRot * glm::vec4(_right, 0.0f));
    _worldUp = _up;
    g_renderSettings.worldUp = _up;
    UpdateAnglesFromVectors();
  } break;
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

  _uniforms.camPos = glm::vec4(g_renderSettings.pos, 0.0f);
  if (g_renderSettings.playing) {
    _pos = g_renderSettings.pos;
    _baseReference = g_renderSettings.baseReference;

    _front = g_renderSettings.front;
    _worldUp = g_renderSettings.worldUp;
    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));

    UpdateAnglesFromVectors();
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
  g_renderSettings.front = _front;
  _right = glm::normalize(glm::cross(_front, _worldUp));
  _up = glm::normalize(glm::cross(_right, _front));
}

void Camera::SetNewReference() {

  _baseReference = glm::mat4(glm::mat3(_right, _up, -_front));

  _worldUp = _up;

  _yaw = 0.0f;
  _pitch = 0.0f;
  g_renderSettings.yaw = _yaw;
  g_renderSettings.pitch = _pitch;

  _currentReference = _baseReference;
  g_renderSettings.baseReference = _baseReference;
  g_renderSettings.currentReference = _currentReference;
  UpdateAnglesFromVectors();
  g_renderSettings.front = _front;
  g_renderSettings.worldUp = _worldUp;
}

glm::mat4 Camera::GetCurrentRotationForGizmo() const {

  glm::mat4 currentOrientation = glm::mat4(glm::mat3(_right, _up, -_front));

  if (_baseReference != glm::mat4(1.0f)) {

    return glm::transpose(_baseReference) * currentOrientation;
  }

  return currentOrientation;
}

void Camera::UpdateAnglesFromVectors() {
  if (_baseReference != glm::mat4(1.0f)) {

    glm::vec3 localFront =
        glm::vec3(glm::transpose(_baseReference) * glm::vec4(_front, 0.0f));
    localFront = glm::normalize(localFront);

    _pitch = glm::degrees(asin(localFront.y));

    float cosPitch = cos(glm::radians(_pitch));
    if (abs(cosPitch) > 0.001f) {
      _yaw = glm::degrees(atan2(localFront.z, localFront.x));
    }

    if (_yaw < 0.0f)
      _yaw += 360.0f;
    if (_yaw >= 360.0f)
      _yaw -= 360.0f;

    g_renderSettings.yaw = _yaw;
    g_renderSettings.pitch = _pitch;
  }
}