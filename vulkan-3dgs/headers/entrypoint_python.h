#pragma once
#include "string"
#include <optional>
void renderGaussians(const std::string path,
                     std::optional<uint32_t> width = std::nullopt,
                     std::optional<uint32_t> height = std::nullopt);