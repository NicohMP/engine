#include "bindings.hpp"

#include "backtest/backtester.hpp"

#include <pybind11/stl.h>

void bind_backtest(py::module_& m) {
    py::class_<Backtester>(m, "Backtester", "Event-driven backtester.")
        .def(
            py::init<const MarketDataStore&, std::shared_ptr<Strategy>, Portfolio, FeatureConfig, double>(),
            py::arg("data"),
            py::arg("strategy"),
            py::arg("portfolio"),
            py::arg("feature_config"),
            py::arg("fee_rate") = 0.0010,
            py::keep_alive<1, 2>()
        )
        .def(
            py::init<
                const MarketDataStore&,
                std::shared_ptr<Strategy>,
                Portfolio,
                FeatureConfig,
                std::shared_ptr<RiskManagerBase>,
                double
            >(),
            py::arg("data"),
            py::arg("strategy"),
            py::arg("portfolio"),
            py::arg("feature_config"),
            py::arg("risk_manager"),
            py::arg("fee_rate") = 0.0010,
            py::keep_alive<1, 2>()
        )
        .def("run", &Backtester::run)
        .def("get_equity", &Backtester::get_equity)
        .def("get_orders", &Backtester::get_orders)
        .def("get_fills", &Backtester::get_fills)
        .def("get_fee_rate", &Backtester::get_fee_rate);
}
