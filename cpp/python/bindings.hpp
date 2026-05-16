#pragma once
#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_core(py::module_& m);
void bind_data(py::module_& m);
void bind_features(py::module_& m);
void bind_portfolio(py::module_& m);
void bind_execution(py::module_& m);
void bind_strategy(py::module_& m);
void bind_risk(py::module_& m);
void bind_backtest(py::module_& m);
void bind_simulator(py::module_& m);
