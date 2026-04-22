#include "backtest/backtester.hpp"


Backtester::Backtester(
    const MarketDataStore& data,
    std::shared_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    std::shared_ptr<RiskManagerBase> risk_manager,
    double fee_rate
    ): 
    data_(data),
    engine(std::move(strategy),std::move(init_portfolio),std::move(risk_manager),fee_rate){}

Backtester::Backtester(
    const MarketDataStore& data,
    std::shared_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    double fee_rate
    ): 
    data_(data),
    engine(std::move(strategy),std::move(init_portfolio),fee_rate) {}


void Backtester::run() {
    engine.reset_all();
    for (const TimeSlice& slice : data_.slices()) {
        for (size_t i= slice.begin_idx; i<slice.end_idx; i++){
            const MarketEvent& mevent = data_.bars()[i];
            engine.step(mevent);
        }
    }
}
