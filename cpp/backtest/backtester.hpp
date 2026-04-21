#pragma once
#include "base.hpp"
#include "portfolio.hpp"
#include "event_types.hpp"
#include "execution_engine.hpp"
#include "risk/base.hpp"
#include "data_store/market_data_store.hpp"
#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <utility>
#include <optional>
#include <cstdint>


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
        Portfolio portfolio;
        std::shared_ptr<RiskManagerBase> risk_manager_; // shared pointer is automatically nullptr when nothig is passed so no need for std::optional<> anymore
        std::shared_ptr<Strategy> strategy_;
        ExecutionEngine execution_engine_;
        std::deque<Event> event_queue_;
        std::vector<OrderEvent> orders_;
        std::vector<FillEvent> fills_;
        std::vector<std::pair<int64_t, double>> equity_;
        double fee_rate_;
    
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
        const std::vector<std::pair<int64_t, double>>& get_equity() const { return equity_; }
        const std::vector<OrderEvent>& get_orders() const { return orders_; }
        const std::vector<FillEvent>& get_fills() const { return fills_; }
        double get_fee_rate() const { return fee_rate_; }
    
};
