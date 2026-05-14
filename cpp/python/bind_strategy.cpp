#include "bindings.hpp"

#include "strategy/base.hpp"
#include "strategy/random.hpp"

#include <pybind11/stl.h>

class PyStrategy : public Strategy, public py::trampoline_self_life_support {
public:
    using Strategy::Strategy;

    void reset() override {
        PYBIND11_OVERRIDE_PURE(
            void,
            Strategy,
            reset
        );
    }

    std::optional<std::vector<OrderEvent>> on_market_event(
        const MarketEvent& mevent,
        const Portfolio& portfolio,
        const FeatureView& view
    ) override {
        PYBIND11_OVERRIDE_PURE(
            std::optional<std::vector<OrderEvent>>,
            Strategy,
            on_market_event,
            mevent,
            portfolio,
            view
        );
    }
};

void bind_strategy(py::module_& m) {
    py::class_<Strategy, PyStrategy, py::smart_holder>(m, "Strategy", "Base class for Python or C++ strategies.")
        .def(py::init<>())
        .def(
            "on_market_event",
            &Strategy::on_market_event,
            py::arg("mevent"),
            py::arg("portfolio"),
            py::arg("features")
        );

    py::class_<RandomStrategy, Strategy, py::smart_holder>(m, "RandomStrategy", "Random target-notional strategy.")
        .def(
            py::init<std::vector<SymbolId>, double, std::optional<std::vector<double>>, unsigned int>(),
            py::arg("universe_ids"),
            py::arg("target_notional"),
            py::arg("probabilities") = std::nullopt,
            py::arg("seed") = 42
        )
        .def(
            "on_market_event",
            &RandomStrategy::on_market_event,
            py::arg("mevent"),
            py::arg("portfolio"),
            py::arg("features")
        );
}
