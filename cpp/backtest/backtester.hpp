#pragma once
#include "core/engine_core.hpp"
#include "data/store/market_data_store.hpp"

#include <memory>
#include <utility>
#include <vector>


// Single-symbol backtester.
// Execution model (Stage 2): now takes events. places orders and fills them on the same tick at tick.close.
// Outputs:
// - passed orders log
// - equity time series (timestamp, equity)
class Backtester {
    private:
        static constexpr double kFeeRateStandard = 0.0010;  // 0.10%
        static constexpr double kFeeRateDiscount = 0.00075; // 0.075%

        const MarketDataStore& data_;
        EngineCore engine;
    
    public:
        Backtester(
            const MarketDataStore& data,
            std::shared_ptr<Strategy> strategy,
            Portfolio portfolio,
            std::shared_ptr<RiskManagerBase> risk_manager = nullptr,
            double fee_rate = kFeeRateStandard
        );
        Backtester(
            const MarketDataStore& data,
            std::shared_ptr<Strategy> strategy,
            Portfolio portfolio,
            double fee_rate
        );
        void run();
        const std::vector<std::pair<int64_t, double>>& get_equity() const { return engine.get_equity(); }
        const std::vector<OrderEvent>& get_orders() const { return engine.get_orders(); }
        const std::vector<FillEvent>& get_fills() const { return engine.get_fills(); }
        double get_fee_rate() const { return engine.get_fee_rate(); }
    
};
