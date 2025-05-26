#include "Camera.h"

Camera::Camera(int w, int h, float fov, float aspectRatio, float nearPlane,
               float farPlane)
    : _pos(glm::vec3(-0.53f, -0.25f, 33.74f)),
      _front(glm::vec3(0.0f, 0.0f, -1.0f)),
      _worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), _yaw(-90.0f), _w(w),
      _h(h) // Start looking forward (negative Z)
      ,
      _pitch(0.0f), _fov(fov), _aspectRatio(aspectRatio), _nearPlane(nearPlane),
      _farPlane(farPlane), _moveSpeed(5.0f), _mouseSensitivity(0.1f) {

  UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
  return glm::lookAt(_pos, _pos + _front, _up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
  return glm::perspective(glm::radians(_fov), _aspectRatio, _nearPlane,
                          _farPlane);
}

void Camera::ProcessMouseMovement(float deltaX, float deltaY,
                                  bool constrainPitch) {
  deltaX *= _mouseSensitivity;
  deltaY *= _mouseSensitivity;

  _yaw += deltaX;
  _pitch += deltaY;

  // Constrain pitch to prevent screen flip
  if (constrainPitch) {
    if (_pitch > 89.0f)
      _pitch = 89.0f;
    if (_pitch < -89.0f)
      _pitch = -89.0f;
  }

  UpdateCameraVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
  float velocity = _moveSpeed * deltaTime;

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
}

void Camera::UpdateAspectRatio(float newAspectRatio) {
  _aspectRatio = newAspectRatio;
}

CameraUniforms Camera::getUniforms() {
  // a lot of this can be just stored in first frame... for now whatever....
  float fov_radians = glm::radians(_fov);

  float fov_y = 2.0f * atan(tan(fov_radians / 2.0f) / _aspectRatio);

  _uniforms.focal_x = _w / (2.0f * tan(fov_radians / 2.0f));
  _uniforms.focal_y = _h / (2.0f * tan(fov_y / 2.0f));

  _uniforms.tan_fovx = tan(fov_radians / 2.0f);
  _uniforms.tan_fovy = tan(fov_y / 2.0f);

  _uniforms.imageWidth = _w;
  _uniforms.imageHeight = _h;

  _uniforms.camPos = _pos;
  _uniforms.viewMatrix = GetViewMatrix();
  _uniforms.projMatrix = GetProjectionMatrix() * _uniforms.viewMatrix;

  return _uniforms;
}

void Camera::UpdateCameraVectors() {

  glm::vec3 newFront;
  newFront.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
  newFront.y = sin(glm::radians(_pitch));
  newFront.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));

  _front = glm::normalize(newFront);

  // Calculate right and up vectors
  _right = glm::normalize(glm::cross(_front, _worldUp));
  _up = glm::normalize(glm::cross(_right, _front));
}