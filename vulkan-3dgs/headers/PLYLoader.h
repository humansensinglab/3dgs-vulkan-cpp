// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "GaussianBase.h"
class PLYLoader {
 public:
  static std::unique_ptr<GaussianBase> LoadPLY(const std::string &path,
                                               int &max_sh_degree);

 private:
  static bool ParseHeader(std::ifstream &file, GaussianBase &data);
  static bool ReadVertexData(std::ifstream &file, GaussianBase &data);
};
