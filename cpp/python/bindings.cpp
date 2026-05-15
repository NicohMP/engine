#include "bindings.hpp"

PYBIND11_MODULE(engine_cpp, m) {
    m.doc() = "C++ event-driven backtesting engine bindings.";
    m.attr("__version__") = "0.1.0";

    bind_core(m);
    bind_data(m);
    bind_features(m);
    bind_portfolio(m);
    bind_execution(m);
    bind_strategy(m);
    bind_risk(m);
    bind_backtest(m);
    bind_simulator(m);
}
