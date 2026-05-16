#include "bindings.hpp"

#include "execution/execution_engine.hpp"

void bind_execution(py::module_& m) {
    py::class_<ExecutionEngine>(m, "ExecutionEngine", "Order queue and fill model.")
        .def(py::init<double>(), py::arg("fee_rate") = 0.0010)
        .def("pass_order", &ExecutionEngine::pass_order, py::arg("order"))
        .def("fill_orders", &ExecutionEngine::fill_orders, py::arg("market_event"));
}
