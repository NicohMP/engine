#pragma once
#include "strategy.hpp"
#include "portfolio.hpp"
#include "event_types.hpp"
#include "execution_engine.hpp"
#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <utility>
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

        std::vector<MarketEvent> mevents_;
        std::string ticker_;
        Portfolio portfolio;
        std::unique_ptr<Strategy> strategy_;
        ExecutionEngine execution_engine_;
        std::deque<Event> event_queue_;
        std::vector<OrderEvent> orders_;
        std::vector<FillEvent> fills_;
        std::vector<std::pair<int64_t, double>> equity_;
        double fee_rate_;
    
    public:
        Backtester(
            std::string ticker,
            const std::vector<MarketEvent>& mevents,
            std::unique_ptr<Strategy> strategy,
            Portfolio portfolio,
            double fee_rate = kFeeRateStandard
        );
        void run();
        const std::vector<std::pair<int64_t, double>>& get_equity() const { return equity_; }
        const std::vector<OrderEvent>& get_orders() const { return orders_; }
        const std::vector<FillEvent>& get_fills() const { return fills_; }
        double get_fee_rate() const { return fee_rate_; }
    
};
