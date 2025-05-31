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

      g_renderSettings.showWireframe = (alpha < 0.5f)
                                           ? _sequence->at(keyA).wireframe
                                           : _sequence->at(keyB).wireframe;
    } else {
      g_renderSettings.playing = false;
    }
  }
}
