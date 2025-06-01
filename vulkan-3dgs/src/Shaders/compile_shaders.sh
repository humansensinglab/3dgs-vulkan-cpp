#!/bin/bash

echo "Compiling shaders for macOS (with Apple/MoltenVK compatibility)..."

# Compile standard shaders (corrected paths for root Shaders folder)
glslangValidator -V --target-env spirv1.3 ../../Shaders/debug.comp -o ../../Shaders/debug.spv
glslangValidator -V --target-env spirv1.3 ../../Shaders/preprocess.comp -o ../../Shaders/preprocess.spv
glslangValidator -V --target-env spirv1.3 ../../Shaders/debugGaussians.comp -o ../../Shaders/nearest.spv
glslangValidator -V --target-env spirv1.3 ../../Shaders/prefixsum.comp -o ../../Shaders/sum.spv
glslangValidator -V --target-env spirv1.5 ../../Shaders/idkeys.comp -o ../../Shaders/idkeys.spv
glslangValidator -V --target-env spirv1.3 ../../Shaders/tile_boundaries.comp -o ../../Shaders/boundaries.spv
glslangValidator -V --target-env spirv1.5 ../../Shaders/render_shared_mem.comp -o ../../Shaders/render_shared.spv
glslangValidator -V --target-env spirv1.5 ../../Shaders/render.comp -o ../../Shaders/render.spv

# Radix sort shaders (need APPLE define for MoltenVK compatibility)
glslangValidator -V --target-env spirv1.5 -DAPPLE ../../Shaders/radix_sort/radixsort.comp -o ../../Shaders/sort.spv
glslangValidator -V --target-env spirv1.5 -DAPPLE ../../Shaders/radix_sort/histogram.comp -o ../../Shaders/histogram.spv

echo "macOS shader compilation complete!"