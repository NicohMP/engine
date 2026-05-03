#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "backtest/backtester.hpp"
#include "core/event_types.hpp"
#include "data/extract/tick.hpp"
#include "data/store/market_data_store.hpp"
#include "execution/execution_engine.hpp"
#include "portfolio/portfolio.hpp"
#include "portfolio/position.hpp"
#include "risk/base.hpp"
#include "risk/simple.hpp"
#include "strategy/base.hpp"
#include "strategy/random.hpp"
#include "data/extract/parse_csv.hpp"
#include "data/features/feature_config.hpp"
#include "data/features/feature_id.hpp"
#include "data/features/feature_schema.hpp"

#include <utility>

namespace py = pybind11;

int add(int a, int b) { return a + b; }

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

PYBIND11_MODULE(engine_cpp, m) {
    m.doc() = "C++ event-driven backtesting engine bindings.";
    m.attr("__version__") = "0.1.0";

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

    py::class_<Tick>(m, "Tick", "Raw OHLCV input row.")
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
        .def_readwrite("timestamp", &Tick::timestamp)
        .def_readwrite("ticker", &Tick::ticker)
        .def_readwrite("open", &Tick::open)
        .def_readwrite("high", &Tick::high)
        .def_readwrite("low", &Tick::low)
        .def_readwrite("volume", &Tick::volume)
        .def_readwrite("close", &Tick::close);

    py::class_<TimeSlice>(m, "TimeSlice", "Contiguous range of bars with same timestamp.")
        .def_readonly("timestamp", &TimeSlice::timestamp)
        .def_readonly("begin_idx", &TimeSlice::begin_idx)
        .def_readonly("end_idx", &TimeSlice::end_idx);

    py::class_<BuildStats>(m, "BuildStats", "Stats from MarketDataStore build.")
        .def(py::init<>())
        .def_readwrite("input_rows", &BuildStats::input_rows)
        .def_readwrite("kept_rows", &BuildStats::kept_rows)
        .def_readwrite("dropped_invalid", &BuildStats::dropped_invalid)
        .def_readwrite("dropped_duplicates", &BuildStats::dropped_duplicates)
        .def_readwrite("num_symbols", &BuildStats::num_symbols)
        .def_readwrite("num_slices", &BuildStats::num_slices);

    py::class_<MarketDataStore>(m, "MarketDataStore", "Read-only clean market data store.")
        .def_static("from_ticks", &MarketDataStore::from_ticks, py::arg("rows"))
        .def("get_or_add_id", &MarketDataStore::get_or_add_id, py::arg("ticker"))
        .def("get_symbol", &MarketDataStore::get_symbol, py::arg("symbol_id"))
        .def("get_id", &MarketDataStore::get_id, py::arg("ticker"))
        .def("symbol_ids", &MarketDataStore::symbol_ids)
        .def(
            "bars",
            &MarketDataStore::bars,
            py::return_value_policy::reference_internal
        )
        .def(
            "slices",
            &MarketDataStore::slices,
            py::return_value_policy::reference_internal
        );

    py::class_<BuildResult>(m, "BuildResult", "Build output with store and stats.")
        .def_readwrite("store", &BuildResult::store)
        .def_readwrite("stats", &BuildResult::stats);

    py::enum_<FeatureId>(m, "FeatureId", "Supported online feature id.")
        .value("Ret1D", FeatureId::Ret1D)
        .value("LogRet1D", FeatureId::LogRet1D)
        .value("RetVar", FeatureId::RetVar)
        .value("RetVol", FeatureId::RetVol)
        .value("PriceZ", FeatureId::PriceZ)
        .value("VolumeZ", FeatureId::VolumeZ)
        .value("HighLowRange", FeatureId::HighLowRange)
        .value("CloseToSma", FeatureId::CloseToSma)
        .export_values();

    py::class_<FeatureSchema>(m, "FeatureSchema", "Ordered online feature schema.")
        .def(py::init<std::vector<FeatureId>>(), py::arg("features"))
        .def(py::init<const std::vector<std::string>&>(), py::arg("names"))
        .def("size", &FeatureSchema::size)
        .def("empty", &FeatureSchema::empty)
        .def("names", &FeatureSchema::names, py::return_value_policy::reference_internal)
        .def("features", &FeatureSchema::features, py::return_value_policy::reference_internal)
        .def("name", &FeatureSchema::name, py::arg("idx"), py::return_value_policy::reference_internal)
        .def("feature", &FeatureSchema::feature, py::arg("idx"))
        .def("contains", &FeatureSchema::contains, py::arg("id"))
        .def("index_of", &FeatureSchema::index_of, py::arg("id"));

    py::class_<FeatureConfig>(m, "FeatureConfig", "Online feature engine config.")
        .def(
            py::init([](FeatureSchema schema, std::size_t window, bool enable_cross_section) {
                return FeatureConfig{std::move(schema), window, enable_cross_section};
            }),
            py::arg("schema"),
            py::arg("window"),
            py::arg("enable_cross_section") = false
        )
        .def_readwrite("schema", &FeatureConfig::schema)
        .def_readwrite("window", &FeatureConfig::window)
        .def_readwrite("enable_cross_section", &FeatureConfig::enable_cross_section);

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

    py::class_<ExecutionEngine>(m, "ExecutionEngine", "Order queue and fill model.")
        .def(py::init<double>(), py::arg("fee_rate") = 0.0010)
        .def("pass_order", &ExecutionEngine::pass_order, py::arg("order"))
        .def("fill_orders", &ExecutionEngine::fill_orders, py::arg("market_event"));

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

    m.def("parse_csv", &parse_csv, py::arg("path"));

}
