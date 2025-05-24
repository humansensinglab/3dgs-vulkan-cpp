#include "PLYLoader.h"

std::unique_ptr<GaussianBase> PLYLoader::LoadPLY(const std::string &path,
                                                 int sh_degree) {
  auto data = std::make_unique<GaussianBase>();
  data->_shDegree = sh_degree;

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
    for (int j = 0; j < rest_coeffs; ++j) {
      data._shCoefficients[sh_offset + 3 + j] = rest_coeffs_vector[j];
    }

    // Opacity
    file.read(reinterpret_cast<char *>(&data._opacities[i]), sizeof(float));

    // Scales
    file.read(reinterpret_cast<char *>(&data._scales[i]), sizeof(glm::vec3));

    // Rotations
    file.read(reinterpret_cast<char *>(&data._rotations[i]), sizeof(glm::vec4));
  }

  return file.good() || file.eof();
}