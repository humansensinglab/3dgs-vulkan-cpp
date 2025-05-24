#pragma once
#include "glm/glm.hpp"
#include <iostream>
#include <vector>

class GaussianBase {
public:
  GaussianBase(){};

  // GPU upload interface
  const void *GetPositionsData() const { return _xyz.data(); }
  const void *GetScalesData() const { return _scales.data(); }
  const void *GetRotationsData() const { return _rotations.data(); }
  const void *GetOpacitiesData() const { return _opacities.data(); }
  const void *GetSHData() const { return _shCoefficients.data(); }

  size_t GetCount() const { return _numGaussians; }
  int GetSHDegree() const { return _shDegree; }
  int GetSHCoefficientsPerChannel() const {
    return (_shDegree + 1) * (_shDegree + 1);
  }

  ~GaussianBase(){};

  // Data vectors
  std::vector<glm::vec3> _xyz;
  std::vector<glm::vec3> _normals;
  std::vector<float> _shCoefficients;
  std::vector<float> _opacities;
  std::vector<glm::vec3> _scales;
  std::vector<glm::vec4> _rotations;

  size_t _numGaussians = 0;
  int _maxSHDegree = 3;
  int _shDegree = 0;
};