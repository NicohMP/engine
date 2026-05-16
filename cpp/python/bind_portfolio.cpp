#include "bindings.hpp"

#include "portfolio/portfolio.hpp"
#include "portfolio/position.hpp"

#include <pybind11/stl.h>

void bind_portfolio(py::module_& m) {
    py::class_<Position>(m, "Position", "Single symbol-id position.")
        .def(
            py::init<const SymbolId&, double>(),
            py::arg("symbol_id"),
            py::arg("initial_quantity") = 0.0
        )
        .def("get_quantity", &Position::get_quantity)
        .def("get_id", &Position::get_id)
        .def("update", &Position::update, py::arg("fill_event"))
        .def("__repr__", [](const Position& p) {
            return "<Position symbol_id=" + std::to_string(p.get_id()) +
                " quantity=" + std::to_string(p.get_quantity()) + ">";
        });

    py::class_<Portfolio, py::smart_holder>(m, "Portfolio", "Portfolio state with cash, positions, prices, and exposure helpers.")
        .def(
            py::init<std::vector<Position>, double>(),
            py::arg("init_positions"),
            py::arg("init_cash")
        )
        .def(
            py::init([](double init_cash) { return Portfolio({}, init_cash); }),
            py::arg("init_cash")
        )
        .def("get_cash", &Portfolio::get_cash)
        .def("has_position", &Portfolio::has_position, py::arg("symbol_id"))
        .def("has_price", &Portfolio::has_price, py::arg("symbol_id"))
        .def("position_qty", &Portfolio::position_qty, py::arg("symbol_id"))
        .def("total_equity", &Portfolio::total_equity)
        .def("update_position", &Portfolio::update_position, py::arg("fill_event"))
        .def("update_prices", &Portfolio::update_prices, py::arg("market_event"))
        .def("price", &Portfolio::price, py::arg("symbol_id"))
        .def("weights", &Portfolio::weights)
        .def("gross_exposure", &Portfolio::gross_exposure)
        .def("net_exposure", &Portfolio::net_exposure);
}
