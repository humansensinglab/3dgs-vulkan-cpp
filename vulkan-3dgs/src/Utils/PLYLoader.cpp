// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "PLYLoader.h"

std::unique_ptr<GaussianBase> PLYLoader::LoadPLY(const std::string &path,
                                                 int &sh_degree) {
  auto data = std::make_unique<GaussianBase>();

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open PLY file: " << path << std::endl;
    return nullptr;
  }

  // Parse PLY header
  if (!ParseHeader(file, *data)) {
    std::cerr << "Error: Invalid PLY header" << std::endl;
    return nullptr;
  }

  sh_degree = data->_shDegree;
  // Read vertex data
  if (!ReadVertexData(file, *data)) {
    std::cerr << "Error: Failed to read vertex data" << std::endl;
    return nullptr;
  }

  std::cout << "Loaded " << data->_numGaussians << " Gaussians from " << path
            << std::endl;
  return data;
}

bool PLYLoader::ParseHeader(std::ifstream &file, GaussianBase &data) {
  std::string line;
  std::vector<std::string> properties;

  // Read header
  while (std::getline(file, line)) {
    if (line.find("element vertex") != std::string::npos) {
      std::istringstream iss(line);
      std::string element, vertex;
      size_t count;
      iss >> element >> vertex >> count;
      data._numGaussians = count;
    } else if (line.find("property") != std::string::npos) {
      properties.push_back(line);
    } else if (line == "end_header") {
      break;
    }
  }
  uint32_t numberProperties = properties.size();
  switch (numberProperties) {
  case 17:
    data._shDegree = 0;
    break;
  case 26:
    data._shDegree = 1;
    break;
  case 41:
    data._shDegree = 2;
    break;
  case 62:
    data._shDegree = 3;
    break;
  }

  return data._numGaussians > 0;
}

bool PLYLoader::ReadVertexData(std::ifstream &file, GaussianBase &data) {
  const size_t num_gaussians = data._numGaussians;
  const int shDegree = data._shDegree;
  const int sh_coeffs_per_channel = (shDegree + 1) * (shDegree + 1);
  const int total_sh_coeffs = 3 * sh_coeffs_per_channel;   // 3 channels (RGB)
  const int rest_coeffs = 3 * (sh_coeffs_per_channel - 1); // Exclude DC

  // Resize vectors
  data._xyz.resize(num_gaussians);
  data._normals.resize(num_gaussians);
  data._shCoefficients.resize(num_gaussians * total_sh_coeffs);
  data._opacities.resize(num_gaussians);
  data._scales.resize(num_gaussians);
  data._rotations.resize(num_gaussians);

  for (size_t i = 0; i < num_gaussians; ++i) {
    // Position
    file.read(reinterpret_cast<char *>(&data._xyz[i]), sizeof(glm::vec3));
    data._xyz[i].w = 1.0f;
    // Normals
    file.read(reinterpret_cast<char *>(&data._normals[i]), sizeof(glm::vec3));

    // DC SH coefficients (f_dc_0, f_dc_1, f_dc_2)
    glm::vec3 dc_coeffs;
    file.read(reinterpret_cast<char *>(&dc_coeffs), sizeof(glm::vec3));

    // Rest SH coefficients - read only what exists in PLY
    std::vector<float> rest_coeffs_vector(rest_coeffs);
    file.read(reinterpret_cast<char *>(rest_coeffs_vector.data()),
              rest_coeffs * sizeof(float));

    // Store in combined SH array
    size_t sh_offset = i * total_sh_coeffs;

    // Store DC coefficients
    data._shCoefficients[sh_offset + 0] = dc_coeffs.x; // f_dc_0
    data._shCoefficients[sh_offset + 1] = dc_coeffs.y; // f_dc_1
    data._shCoefficients[sh_offset + 2] = dc_coeffs.z; // f_dc_2

    // Store rest coefficients
    for (int j = 0; j < rest_coeffs / 3; ++j) {
      int out_idx = (j + 1) * 3; // +1 because DC is at index 0
      data._shCoefficients[sh_offset + out_idx + 0] =
          rest_coeffs_vector[j]; // R
      data._shCoefficients[sh_offset + out_idx + 1] =
          rest_coeffs_vector[j + (rest_coeffs / 3)]; // G
      data._shCoefficients[sh_offset + out_idx + 2] =
          rest_coeffs_vector[j + 2 * (rest_coeffs / 3)]; // B
    }

    // Read raw values, apply activations, and store directly
    float rawOpacity;
    glm::vec3 rawScales;
    glm::vec4 rawRotation;

    file.read(reinterpret_cast<char *>(&rawOpacity), sizeof(float));
    file.read(reinterpret_cast<char *>(&rawScales), sizeof(glm::vec3));
    file.read(reinterpret_cast<char *>(&rawRotation), sizeof(glm::vec4));

    // Apply activations and store with original names
    data._opacities[i] = 1.0f / (1.0f + std::exp(-rawOpacity)); // Sigmoid
    data._scales[i] = glm::vec4(glm::exp(rawScales), 0.0f);
    data._rotations[i] = glm::normalize(rawRotation);
  }

  return file.good() || file.eof();
}
