#include "bindings.hpp"

#include "risk/base.hpp"
#include "risk/simple.hpp"

#include <pybind11/stl.h>

class PyRiskManager : public RiskManagerBase, public py::trampoline_self_life_support {
public:
    using RiskManagerBase::RiskManagerBase;

    void reset() override {
        PYBIND11_OVERRIDE_PURE(
            void,
            RiskManagerBase,
            reset
        );
    }

    std::optional<std::vector<OrderEvent>> accept_order_batch(
        const std::vector<OrderEvent>& orders,
        const Portfolio& portfolio
    ) const override {
        PYBIND11_OVERRIDE_PURE(
            std::optional<std::vector<OrderEvent>>,
            RiskManagerBase,
            accept_order_batch,
            orders,
            portfolio
        );
    }
};

void bind_risk(py::module_& m) {
    py::class_<RiskManagerBase, PyRiskManager, py::smart_holder>(m, "RiskManagerBase", "Base class for Python or C++ risk managers.")
        .def(py::init<>())
        .def(
            "accept_order_batch",
            &RiskManagerBase::accept_order_batch,
            py::arg("orders"),
            py::arg("portfolio")
        );

    py::class_<RiskLimits>(m, "RiskLimits", "Limits used by SimpleRiskManager.")
        .def(
            py::init([](
                double max_position_weight,
                double max_gross_exposure,
                double max_net_exposure,
                double max_nb_positions
            ) {
                RiskLimits limits;
                limits.max_position_weight = max_position_weight;
                limits.max_gross_exposure = max_gross_exposure;
                limits.max_net_exposure = max_net_exposure;
                limits.max_nb_positions = max_nb_positions;
                return limits;
            }),
            py::arg("max_position_weight") = 1.0,
            py::arg("max_gross_exposure") = 1.0,
            py::arg("max_net_exposure") = 50.0,
            py::arg("max_nb_positions") = 1000.0
        )
        .def_readwrite("max_position_weight", &RiskLimits::max_position_weight)
        .def_readwrite("max_gross_exposure", &RiskLimits::max_gross_exposure)
        .def_readwrite("max_net_exposure", &RiskLimits::max_net_exposure)
        .def_readwrite("max_nb_positions", &RiskLimits::max_nb_positions);

    py::class_<SimpleRiskManager, RiskManagerBase, py::smart_holder>(m, "SimpleRiskManager", "Risk manager that checks portfolio exposure after a proposed order batch.")
        .def(py::init<RiskLimits>(), py::arg("limits") = RiskLimits{})
        .def(
            "accept_order_batch",
            &SimpleRiskManager::accept_order_batch,
            py::arg("orders"),
            py::arg("portfolio")
        )
        .def("acceptable", &SimpleRiskManager::acceptable, py::arg("portfolio"));
}
