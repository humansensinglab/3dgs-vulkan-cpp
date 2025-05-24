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
  std::cout << "\n--- Creating Gaussian GPU Buffers and uploading to GPU "
               "thorugh staging buffers ---"
            << std::endl;

  CreateAndUploadBuffer<glm::vec3>(_xyzBuffer,
                                   _gaussianData->GetPositionsData(), "_xyz");
  CreateAndUploadBuffer<glm::vec3>(_scaleBuffer, _gaussianData->GetScalesData(),
                                   "_scale");
  CreateAndUploadBuffer<glm::vec4>(_rotationBuffer,
                                   _gaussianData->GetRotationsData(), "_rot");
  CreateAndUploadBuffer<float>(_opacityBuffer,
                               _gaussianData->GetOpacitiesData(), "_opacity");
  CreateAndUploadBuffer<float>(
      _shBuffer, _gaussianData->GetSHData(), "_SH",
      3 * (_gaussianData->GetSHCoefficientsPerChannel()));
}
