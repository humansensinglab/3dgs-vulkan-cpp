#!/bin/bash

echo "Compiling shaders for linux..."

# Compile standard shaders (shaders in src/Shaders/)
glslangValidator -V --target-env spirv1.3 ../Shaders/debug.comp -o ../Shaders/debug.spv
glslangValidator -V --target-env spirv1.3 ../Shaders/preprocess.comp -o ../Shaders/preprocess.spv
glslangValidator -V --target-env spirv1.3 ../Shaders/debugGaussians.comp -o ../Shaders/nearest.spv
glslangValidator -V --target-env spirv1.3 ../Shaders/prefixsum.comp -o ../Shaders/sum.spv
glslangValidator -V --target-env spirv1.5 ../Shaders/idkeys.comp -o ../Shaders/idkeys.spv
glslangValidator -V --target-env spirv1.3 ../Shaders/tile_boundaries.comp -o ../Shaders/boundaries.spv
glslangValidator -V --target-env spirv1.5 ../Shaders/render_shared_mem.comp -o ../Shaders/render_shared.spv
glslangValidator -V --target-env spirv1.5 ../Shaders/upsample.comp -o ../Shaders/upsample.spv
glslangValidator -V --target-env spirv1.5 ../Shaders/vertex_axis.vert -o ../Shaders/axis_vert.spv
glslangValidator -V --target-env spirv1.5 ../Shaders/frag_axis.frag -o ../Shaders/axis_frag.spv


glslangValidator -V --target-env spirv1.5  ../Shaders/radix_sort/radixsort.comp -o ../Shaders/sort.spv
glslangValidator -V --target-env spirv1.5  ../Shaders/radix_sort/histogram.comp -o ../Shaders/histogram.spv
glslangValidator -V --target-env spirv1.5  ../Shaders/render.comp -o ../Shaders/render.spv

echo "macOS shader compilation complete!"

