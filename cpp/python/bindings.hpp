#pragma once
#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_core(py::module_& m);
void bind_data(py::module_& m);
