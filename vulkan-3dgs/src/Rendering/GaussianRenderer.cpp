#include "GaussianRenderer.h"

GaussianRenderer::~GaussianRenderer() {
  _bufferManager.CleanupAllBuffers(_vulkanContext.GetLogicalDevice());
  std::cout << "GaussianRenderer destroyed" << std::endl;
}

void GaussianRenderer::LoadGaussianData(
    std::unique_ptr<GaussianBase> gaussianData) {
  if (!gaussianData) {
    throw std::runtime_error("Cannot load null gaussian data!");
  }

  std::cout << "\n=== GaussianRenderer: Loading Data ===" << std::endl;
  std::cout << "Gaussians: " << gaussianData->GetCount() << std::endl;
  std::cout << "SH degree: " << gaussianData->GetSHDegree() << std::endl;

  _gaussianData = std::move(gaussianData);

  CreateGaussianBuffers();

  std::cout << " Gaussian data loaded and GPU resources created !" << std::endl;
}

void GaussianRenderer::CreateGaussianBuffers() {
  std::cout << "\n--- Creating Gaussian GPU Buffers ---" << std::endl;

  CreateBuffer<glm::vec3>(_xyzBuffer, "_xyz");
  CreateBuffer<glm::vec3>(_scaleBuffer, "_scale");
  CreateBuffer<glm::vec4>(_rotationBuffer, "_rot");
  CreateBuffer<float>(_opacityBuffer, "_opacity");
  CreateBuffer<float>(_shBuffer, "_SH",
                      _gaussianData->GetSHCoefficientsPerChannel() + 3);
}
