#include "backtest/backtester.hpp"


Backtester::Backtester(
    const MarketDataStore& data,
    std::shared_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    FeatureConfig feature_config,
    std::shared_ptr<RiskManagerBase> risk_manager,
    double fee_rate
    ): 
    data_(data),
    engine(data_.symbol_ids(), std::move(strategy), std::move(init_portfolio), feature_config, std::move(risk_manager), fee_rate),
    feature_config_(std::move(feature_config)) {}

Backtester::Backtester(
    const MarketDataStore& data,
    std::shared_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    FeatureConfig feature_config,
    double fee_rate
    ): 
    data_(data),
    engine(data_.symbol_ids(), std::move(strategy), std::move(init_portfolio), feature_config, fee_rate),
    feature_config_(std::move(feature_config)) {}


void Backtester::run() {
    engine.reset_all();
    const std::vector<MarketEvent>& bars = data_.bars();
    for (const TimeSlice& slice : data_.slices()) {
        engine.step_slice(&bars[slice.begin_idx], &bars[slice.end_idx]);
    }
}
