#pragma once
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };
struct CameraUniforms {
  alignas(16) glm::mat4 viewMatrix;
  alignas(16) glm::mat4 projMatrix;
};

class Camera {

public:
  Camera(float fov = 45.0f, float aspectRatio = 16.0f / 9.0f,
         float nearPlane = 0.1f, float farPlane = 1000.0f);

  glm::mat4 GetViewMatrix() const;
  glm::mat4 GetProjectionMatrix() const;
  glm::vec3 GetPosition() const { return _pos; }
  glm::vec3 GetFront() const { return _front; }

  void SetMovementSpeed(float speed) { _moveSpeed = speed; }
  void SetMouseSensitivity(float sensitivity) {
    _mouseSensitivity = sensitivity;
  }
  void SetFOV(float newFov) { _fov = newFov; }

  void ProcessMouseMovement(float deltaX, float deltaY,
                            bool constrainPitch = true);
  void ProcessKeyboard(CameraMovement direction, float deltaTime);
  void UpdateAspectRatio(float aspectRatio);
  CameraUniforms getViewProjection() const {
    return {GetViewMatrix(), GetProjectionMatrix()};
  }

private:
  glm::vec3 _pos, _front, _up, _worldUp, _right;

  float _yaw, _pitch;

  float _fov, _aspectRatio, _nearPlane, _farPlane;

  float _moveSpeed, _mouseSensitivity;

  void UpdateCameraVectors();
};