#include "simulation/simulator.hpp"
#include "bindings.hpp"
#include <pybind11/stl.h>

void bind_simulator(py::module_& m) {

    py::class_<StepResult>(m, "StepResult", py::dynamic_attr(), "Result of engine core bar step.")
        .def(
            py::init([](
                std::optional<int64_t> timestamp,
                std::vector<OrderEvent> new_orders,
                std::vector<FillEvent> new_fills,
                double equity_after,
                size_t total_orders,
                size_t total_fills
            ) {
                return StepResult{
                    timestamp,
                    std::move(new_orders),
                    std::move(new_fills),
                    equity_after,
                    total_orders,
                    total_fills
                };
            }),
            py::arg("timestamp"),
            py::arg("new_orders"),
            py::arg("new_fills"),
            py::arg("equity_after"),
            py::arg("total_orders"),
            py::arg("total_fills")
        )
        .def_readwrite("timestamp", &StepResult::timestamp)
        .def_readwrite("new_orders", &StepResult::new_orders)
        .def_readwrite("new_fills", &StepResult::new_fills)
        .def_readwrite("equity_after", &StepResult::equity_after)
        .def_readwrite("total_orders", &StepResult::total_orders)
        .def_readwrite("total_fills", &StepResult::total_fills);
        // .def("__repr__", [](const StepResult& St) {
        //     return "<StepResult timestamp= " + std::to_string(*St.timestamp) +
        //         " new_orders=" + std::to_string(St.new_orders) +
        //         " new_fills=" + std::to_string(St.new_fills) +
        //         " equity_after=" + std::to_string(St.equity_after) +
        // });

    py::class_<Simulator>(m, "Simulator", "Event-driven simulator.")
        .def(
            py::init<std::vector<SymbolId>, std::shared_ptr<Strategy>, Portfolio, FeatureConfig, double>(),
            py::arg("universe"),
            py::arg("strategy"),
            py::arg("portfolio"),
            py::arg("feature_config"),
            py::arg("fee_rate") = 0.0010,
            py::keep_alive<1, 2>()
        )
        .def(
            py::init<
                std::vector<SymbolId>,
                std::shared_ptr<Strategy>,
                Portfolio,
                FeatureConfig,
                std::shared_ptr<RiskManagerBase>,
                double
            >(),
            py::arg("universe"),
            py::arg("strategy"),
            py::arg("portfolio"),
            py::arg("feature_config"),
            py::arg("risk_manager"),
            py::arg("fee_rate") = 0.0010,
            py::keep_alive<1, 2>()
        )
        .def("step_bar", &Simulator::step_bar)
        .def("step_timestamp", &Simulator::step_timestamp)
        .def("reset_runtime", &Simulator::reset_runtime)
        .def("portfolio", &Simulator::portfolio, py::return_value_policy::reference_internal)
        .def("get_equity", &Simulator::get_equity)
        .def("get_orders", &Simulator::get_orders)
        .def("get_fills", &Simulator::get_fills)
        .def("get_fee_rate", &Simulator::get_fee_rate);

}
