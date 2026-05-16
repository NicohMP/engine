#include "bindings.hpp"

#include "core/event_types.hpp"

#include <pybind11/stl.h>

int add(int a, int b) { return a + b; }

void bind_core(py::module_& m) {
    m.def("add", &add, py::arg("a"), py::arg("b"), "Return a + b.");
    m.def("generate_order_id", &generate_order_id, "Return the next process-wide order id.");
    m.def(
        "reset_order_id_counter",
        &reset_order_id_counter,
        py::arg("start_id") = 1,
        "Reset the process-wide order id counter."
    );

    py::enum_<TradeSide>(m, "TradeSide", "Order side.")
        .value("Buy", TradeSide::Buy)
        .value("Sell", TradeSide::Sell)
        .export_values();

    py::enum_<OrderType>(m, "OrderType", "Supported order type.")
        .value("Market", OrderType::Market)
        .value("Limit", OrderType::Limit)
        .value("Stop", OrderType::Stop)
        .export_values();

    py::class_<MarketEvent>(m, "MarketEvent", py::dynamic_attr(), "One OHLCV market bar.")
        .def(
            py::init<int64_t, SymbolId, double, double, double, double, double>(),
            py::arg("timestamp"),
            py::arg("symbol_id"),
            py::arg("open"),
            py::arg("high"),
            py::arg("low"),
            py::arg("volume"),
            py::arg("close")
        )
        .def_readwrite("timestamp", &MarketEvent::timestamp)
        .def_readwrite("symbol_id", &MarketEvent::symbol_id)
        .def_readwrite("open", &MarketEvent::open)
        .def_readwrite("high", &MarketEvent::high)
        .def_readwrite("low", &MarketEvent::low)
        .def_readwrite("volume", &MarketEvent::volume)
        .def_readwrite("close", &MarketEvent::close)
        .def("__repr__", [](const MarketEvent& e) {
            return "<MarketEvent timestamp=" + std::to_string(e.timestamp) +
                " symbol_id=" + std::to_string(e.symbol_id) +
                " close=" + std::to_string(e.close) + ">";
        });

    py::class_<OrderEvent>(m, "OrderEvent", py::dynamic_attr(), "Order request emitted by a strategy.")
        .def(
            py::init<std::uint64_t, SymbolId, TradeSide, std::optional<double>, OrderType, std::optional<double>>(),
            py::arg("id"),
            py::arg("symbol_id"),
            py::arg("side"),
            py::arg("quantity"),
            py::arg("order_type") = OrderType::Market,
            py::arg("price") = std::nullopt
        )
        .def(
            py::init([](
                SymbolId symbol_id,
                TradeSide side,
                std::optional<double> quantity,
                OrderType order_type,
                std::optional<double> price,
                std::uint64_t id
            ) {
                return OrderEvent{id, symbol_id, side, quantity, order_type, price};
            }),
            py::arg("symbol_id"),
            py::arg("side"),
            py::arg("quantity"),
            py::arg("order_type") = OrderType::Market,
            py::arg("price") = std::nullopt,
            py::arg("id") = 0
        )
        .def_readwrite("id", &OrderEvent::id)
        .def_readwrite("symbol_id", &OrderEvent::symbol_id)
        .def_readwrite("side", &OrderEvent::side)
        .def_readwrite("quantity", &OrderEvent::quantity)
        .def_readwrite("order_type", &OrderEvent::order_type)
        .def_readwrite("price", &OrderEvent::price)
        .def("__repr__", [](const OrderEvent& o) {
            return "<OrderEvent id=" + std::to_string(o.id) +
                " symbol_id=" + std::to_string(o.symbol_id) +
                " quantity=" + std::to_string(o.quantity.value_or(0.0)) + ">";
        });

    py::class_<FillEvent>(m, "FillEvent", py::dynamic_attr(), "Execution fill generated from an order.")
        .def(
            py::init<std::uint64_t, int64_t, SymbolId, TradeSide, double, double, double>(),
            py::arg("order_id"),
            py::arg("timestamp"),
            py::arg("symbol_id"),
            py::arg("side"),
            py::arg("quantity"),
            py::arg("price"),
            py::arg("fees")
        )
        .def_readwrite("order_id", &FillEvent::order_id)
        .def_readwrite("timestamp", &FillEvent::timestamp)
        .def_readwrite("symbol_id", &FillEvent::symbol_id)
        .def_readwrite("side", &FillEvent::side)
        .def_readwrite("quantity", &FillEvent::quantity)
        .def_readwrite("price", &FillEvent::price)
        .def_readwrite("fees", &FillEvent::fees)
        .def("__repr__", [](const FillEvent& f) {
            return "<FillEvent order_id=" + std::to_string(f.order_id) +
                " symbol_id=" + std::to_string(f.symbol_id) +
                " quantity=" + std::to_string(f.quantity) +
                " price=" + std::to_string(f.price) +
                " fees=" + std::to_string(f.fees) + ">";
        });
}
