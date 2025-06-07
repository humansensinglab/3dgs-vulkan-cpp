#include "entrypoint_python.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

PYBIND11_MODULE(vulkan_3dgs_py, m) {
  m.def("run", &renderGaussians, "Run the 3D Gaussian Splatting application",
        pybind11::arg("path"), pybind11::arg("width") = pybind11::none(),
        pybind11::arg("height") = pybind11::none());
}