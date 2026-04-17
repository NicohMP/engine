// This header gives us the core pybind11 API:
// - PYBIND11_MODULE(...)
// - m.def(...)
// - pybind11 types and helpers
// It must be found through the CMake pybind11 target, so this file should be
// built with `cmake --build build --target engine_cpp`, not compiled directly.
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "backtest/event_types.hpp"
#include "portfolio/position.hpp"
#include "portfolio/portfolio.hpp"
#include "strategy/random.hpp"


// Short alias so we can write `py::...` instead of `pybind11::...`.
// This is a common pybind11 convention.
namespace py = pybind11;

// This is a normal C++ function.
// By itself, Python cannot see or call it.
// It only becomes available from Python after we register it below with m.def.
int add(int a, int b) {
    return a + b;
}

// This macro creates the Python extension module.
//
// The first argument, `engine_cpp`, must match:
// - the CMake module target name: pybind11_add_module(engine_cpp ...)
// - the Python import name: import engine_cpp
//
// The second argument, `m`, is the module object.
// We use it to add functions/classes to the Python module.
PYBIND11_MODULE(engine_cpp, m) {
    // Expose the C++ function `add` to Python.
    //
    // The first argument, "add", is the Python name:
    //     engine_cpp.add(...)
    //
    // The second argument, &add, is a pointer to the C++ function above.
    m.def("add", &add, py::arg("a"), py::arg("b"));

    py::enum_<TradeSide>(m, "TradeSide")
    .value("Buy", TradeSide::Buy)
    .value("Sell", TradeSide::Sell)
    .export_values();

    py::enum_<OrderType>(m, "OrderType")
    .value("Market", OrderType::Market)
    .value("Limit", OrderType::Limit)
    .value("Stop", OrderType::Stop)
    .export_values();


    py::class_<MarketEvent>(m,"MarketEvent",py::dynamic_attr())
        .def(
            py::init<int64_t, std::string, double, double, double, double, double>(),
            py::arg("timestamp"),
            py::arg("ticker"),
            py::arg("open"),
            py::arg("high"),
            py::arg("low"),
            py::arg("volume"),
            py::arg("close")
        )
        .def_readwrite("timestamp", &MarketEvent::timestamp)
        .def_readwrite("ticker", &MarketEvent::ticker)
        .def_readwrite("open", &MarketEvent::open)
        .def_readwrite("high", &MarketEvent::high)
        .def_readwrite("low", &MarketEvent::low)
        .def_readwrite("volume", &MarketEvent::volume)
        .def_readwrite("close", &MarketEvent::close);

    
    py::class_<OrderEvent>(m,"OrderEvent",py::dynamic_attr())
        .def(
            py::init<std::uint64_t, std::string, TradeSide, std::optional<double>, OrderType, std::optional<double>>(),
            py::arg("id"),
            py::arg("ticker"),
            py::arg("side"),
            py::arg("quantity"),
            py::arg("order_type") = OrderType::Market,
            py::arg("price") = std::nullopt
        )
        .def_readwrite("id", &OrderEvent::id)
        .def_readwrite("ticker", &OrderEvent::ticker)
        .def_readwrite("side", &OrderEvent::side)
        .def_readwrite("quantity", &OrderEvent::quantity)
        .def_readwrite("order_type", &OrderEvent::order_type)
        .def_readwrite("price", &OrderEvent::price);

    py::class_<FillEvent>(m,"FillEvent",py::dynamic_attr())
        .def(
            py::init<std::uint64_t, int64_t, std::string, TradeSide, double, double, double>(),
            py::arg("order_id"),
            py::arg("timestamp"),
            py::arg("ticker"),
            py::arg("side"),
            py::arg("quantity"),
            py::arg("price"),
            py::arg("fees")
        )
        .def_readwrite("order_id", &FillEvent::order_id)
        .def_readwrite("timestamp", &FillEvent::timestamp)
        .def_readwrite("ticker", &FillEvent::ticker)
        .def_readwrite("side", &FillEvent::side)
        .def_readwrite("quantity", &FillEvent::quantity)
        .def_readwrite("price", &FillEvent::price)
        .def_readwrite("fees", &FillEvent::fees);

    py::class_<Position>(m,"Position")
        .def(
            py::init<const std::string&, double>(),
            py::arg("ticker"),
            py::arg("initial_quantity")
        )
        .def("get_quantity",&Position::get_quantity)
        .def("get_ticker",&Position::get_ticker)
        .def("update", &Position::update, py::arg("fill_event"));

    py::class_<Portfolio,py::smart_holder>(m,"Portfolio")
        .def(
            py::init<std::vector<Position>, double>(),
            py::arg("init_positions"),
            py::arg("init_cash")
        )
        .def("get_cash", &Portfolio::get_cash)
        .def("has_position", &Portfolio::has_position, py::arg("ticker"))
        .def("has_price", &Portfolio::has_price, py::arg("ticker"))
        .def("position_qty", &Portfolio::position_qty, py::arg("ticker"))
        .def("total_equity", &Portfolio::total_equity);

    
    py::class_<Strategy, py::smart_holder>(m, "Strategy");


    py::class_<RandomStrategy,Strategy, py::smart_holder>(m,"RandomStrategy")
        .def(
            py::init<std::vector<std::string>,
            double,
            std::optional<std::vector<double>>,
            unsigned int>(),
            py::arg("universe"),
            py::arg("target_notional"),
            py::arg("probabilities") = std::nullopt,
            py::arg("seed") = 42)
        .def(
            "on_market_event",
            &RandomStrategy::on_market_event,
            py::arg("mevent"),
            py::arg("portfolio")
        );
    

    
}
