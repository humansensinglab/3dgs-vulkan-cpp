#include "Sequence.h"

void Sequence::Play(float deltaTime) {

  if (g_renderSettings.playing && _sequence->size() >= 2) {
    // Update time
    _currTime += deltaTime;

    int keyA = -1, keyB = -1;
    for (int i = 0; i < _sequence->size() - 1; i++) {
      if (_currTime >= _sequence->at(i).time &&
          _currTime <= _sequence->at(i + 1).time) {
        keyA = i;
        keyB = i + 1;
        break;
      }
    }
    if (keyA >= 0 && keyB >= 0) {
      float t1 = _sequence->at(keyA).time;
      float t2 = _sequence->at(keyB).time;
      float alpha = (_currTime - t1) / (t2 - t1);

      g_renderSettings.pos = glm::mix(_sequence->at(keyA).position,
                                      _sequence->at(keyB).position, alpha);
      g_renderSettings.fov =
          glm::mix(_sequence->at(keyA).fov, _sequence->at(keyB).fov, alpha);
      g_renderSettings.nearPlane = glm::mix(
          _sequence->at(keyA).nearPlane, _sequence->at(keyB).nearPlane, alpha);
      g_renderSettings.farPlane = glm::mix(_sequence->at(keyA).farPlane,
                                           _sequence->at(keyB).farPlane, alpha);

      g_renderSettings.showWireframe = _sequence->at(keyB).wireframe;

      g_renderSettings.yaw =
          LerpAngle(_sequence->at(keyA).yaw, _sequence->at(keyB).yaw, alpha);
      g_renderSettings.pitch = LerpAngle(_sequence->at(keyA).pitch,
                                         _sequence->at(keyB).pitch, alpha);
      glm::vec3 worldUpInterp = glm::mix(_sequence->at(keyA).worldUp,
                                         _sequence->at(keyB).worldUp, alpha);
      g_renderSettings.worldUp = glm::normalize(worldUpInterp);
      glm::vec3 interpFront =
          glm::mix(_sequence->at(keyA).front, _sequence->at(keyB).front, alpha);
      g_renderSettings.front = glm::normalize(interpFront);

      g_renderSettings.baseReference = _sequence->at(keyB).baseReference;
      g_renderSettings.currentReference = _sequence->at(keyB).currentReference;

    } else {
      g_renderSettings.playing = false;
    }
  } else {
    g_renderSettings.playing = false;
  }
}

inline float Sequence::LerpAngle(float a, float b, float t) {
  float diff = b - a;
  while (diff > 180.0f)
    diff -= 360.0f;
  while (diff < -180.0f)
    diff += 360.0f;
  return a + diff * t;
}
