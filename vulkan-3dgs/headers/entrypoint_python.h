#pragma once
#include "string"
#include <optional>
#include "RenderSettings.h"

void renderGaussians(const std::string path,
                     const std::string& shaderDir,
                     std::optional<uint32_t> width = std::nullopt,
                     std::optional<uint32_t> height = std::nullopt);