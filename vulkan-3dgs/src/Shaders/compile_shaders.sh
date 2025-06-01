#!/bin/bash



echo "Compiling shaders for macOS (with Apple/MoltenVK compatibility)..."

# Standard shaders (no special defines needed)
glslangValidator -V --target-env spirv1.3 debug.comp
glslangValidator -V --target-env spirv1.3 preprocess.comp -o preprocess.spv
glslangValidator -V --target-env spirv1.3 debugGaussians.comp -o nearest.spv
glslangValidator -V --target-env spirv1.3 prefixsum.comp -o sum.spv
glslangValidator -V --target-env spirv1.5 idkeys.comp -o idkeys.spv
glslangValidator -V --target-env spirv1.3 tile_boundaries.comp -o boundaries.spv
glslangValidator -V --target-env spirv1.5 render_shared_mem.comp -o render_shared.spv
glslangValidator -V --target-env spirv1.5 render.comp -o render.spv

# Radix sort shaders (need APPLE define for MoltenVK compatibility)
glslangValidator -V --target-env spirv1.5 -DAPPLE radix_sort/radixsort.comp -o sort.spv
glslangValidator -V --target-env spirv1.5 -DAPPLE radix_sort/histogram.comp -o histogram.spv

echo "macOS shader compilation complete!"
