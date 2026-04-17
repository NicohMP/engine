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
#include "strategy/base.hpp"
#include "backtest/backtester.hpp"
#include "execution/execution_engine.hpp"
#include "risk/base.hpp"
#include "risk/simple.hpp"

// Short alias so we can write `py::...` instead of `pybind11::...`.
// This is a common pybind11 convention.
namespace py = pybind11;

// This is a normal C++ function.
// By itself, Python cannot see or call it.
// It only becomes available from Python after we register it below with m.def.
int add(int a, int b) {
    return a + b;
}


class PyStrategy : public Strategy, public py::trampoline_self_life_support {
    public:
        using Strategy::Strategy;
        std::optional<std::vector<OrderEvent>> on_market_event(const MarketEvent& mevent, const Portfolio& portfolio) override {
            PYBIND11_OVERRIDE_PURE(
                std::optional<std::vector<OrderEvent>>,
                Strategy,
                on_market_event,
                mevent,
                portfolio);
            }
        };

class PyRiskManager : public RiskManagerBase, public py::trampoline_self_life_support{
    public:
        using RiskManagerBase::RiskManagerBase;
        std::optional<std::vector<OrderEvent>> accept_order_batch(
        const std::vector<OrderEvent>& orders,
        const Portfolio& portfolio
            )const override{
        PYBIND11_OVERRIDE_PURE(
            std::optional<std::vector<OrderEvent>>,
            RiskManagerBase,
            accept_order_batch,
            orders,
            portfolio);
    }
};



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
        .def_readwrite("price", &OrderEvent::price)
        .def("__repr__", [](const OrderEvent& o) {
            return "<OrderEvent id=" + std::to_string(o.id) +
                " ticker=" + o.ticker +
                " quantity=" + std::to_string(o.quantity.value_or(0.0)) +
                ">";
        });


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
        .def_readwrite("fees", &FillEvent::fees)
        .def("__repr__", [](const FillEvent& f) {
            return "<FillEvent order_id=" + std::to_string(f.order_id) +
                " ticker=" + f.ticker +
                " quantity=" + std::to_string(f.quantity) +
                " price=" + std::to_string(f.price) +
                " fees=" + std::to_string(f.fees) +
                ">";
        });


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
        .def("total_equity", &Portfolio::total_equity)
        .def("update_position", &Portfolio::update_position, py::arg("fill_event"))
        .def("update_prices", &Portfolio::update_prices, py::arg("market_event"))
        .def("price", &Portfolio::price, py::arg("ticker"))
        .def("weights", &Portfolio::weights)
        .def("gross_exposure", &Portfolio::gross_exposure)
        .def("net_exposure", &Portfolio::net_exposure);


    py::class_<ExecutionEngine>(m, "ExecutionEngine")
    .def(py::init<double>(), py::arg("fee_rate"))
    .def("pass_order", &ExecutionEngine::pass_order, py::arg("order"))
    .def("fill_orders", &ExecutionEngine::fill_orders, py::arg("market_event"));

    
    py::class_<Strategy, PyStrategy, py::smart_holder>(m, "Strategy")
        .def(py::init<>())
        .def(
            "on_market_event",
            &Strategy::on_market_event,
            py::arg("mevent"),
            py::arg("portfolio")
        );
    
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

    py::class_<RiskManagerBase, PyRiskManager, py::smart_holder>(m,"RiskManagerBase")
        .def(py::init<>())
        .def(
            "accept_order_batch",
            &RiskManagerBase::accept_order_batch,
            py::arg("orders"),
            py::arg("portfolio")
        );

    py::class_<RiskLimits>(m, "RiskLimits")
    .def(py::init<>())
    .def_readwrite("max_position_weight", &RiskLimits::max_position_weight)
    .def_readwrite("max_gross_exposure", &RiskLimits::max_gross_exposure)
    .def_readwrite("max_net_exposure", &RiskLimits::max_net_exposure)
    .def_readwrite("max_nb_positions", &RiskLimits::max_nb_positions);
    
    py::class_<SimpleRiskManager, RiskManagerBase, py::smart_holder>(m, "SimpleRiskManager")
    .def(
        py::init<RiskLimits>(),
        py::arg("limits")
    )
    .def(
        "accept_order_batch",
        &SimpleRiskManager::accept_order_batch,
        py::arg("orders"),
        py::arg("portfolio")
    )
    .def(
        "acceptable",
        &SimpleRiskManager::acceptable,
        py::arg("portfolio")
    );


    
    py::class_<Backtester>(m, "Backtester")
    .def(
        py::init<
            const std::vector<MarketEvent>&,
            std::shared_ptr<Strategy>,
            Portfolio,
            double
        >(),
        py::arg("market_events"),
        py::arg("strategy"),
        py::arg("portfolio"),
        py::arg("fee_rate") = 0.0010
    )
    .def(
    py::init<
        const std::vector<MarketEvent>&,
        std::shared_ptr<Strategy>,
        Portfolio,
        std::shared_ptr<RiskManagerBase>,
        double
    >(),
    py::arg("market_events"),
    py::arg("strategy"),
    py::arg("portfolio"),
    py::arg("risk_manager"),
    py::arg("fee_rate") = 0.0010
)


    .def("run", &Backtester::run)
    .def("get_equity", &Backtester::get_equity)
    .def("get_orders", &Backtester::get_orders)
    .def("get_fills", &Backtester::get_fills)
    .def("get_fee_rate", &Backtester::get_fee_rate);

    

    
}
