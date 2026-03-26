#pragma once
#include "strategy.hpp"
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <cstdint>

// Single-symbol backtester.
// Execution model (Stage 1): signal and fill on the same tick at tick.close.
// Outputs:
// - executed trades log
// - equity time series (timestamp, equity)
class Backtester {
    private:
        std::vector<Tick> ticks_;
        std::string ticker_;
        AccountState account_state_;
        Position position_;
        std::unique_ptr<Strategy> strategy_;
        std::vector<Trade> trades_;
        std::vector<std::pair<int64_t, double>> equity_;
    
    public:
        Backtester(std::string ticker,const std::vector<Tick>& ticks,std::unique_ptr<Strategy> strategy, AccountState init_account_state);
        void run();
        std::vector<std::pair<int64_t, double>> get_equity() {return equity_;};
        std::vector<Trade> get_trades() {return trades_;};
    
};
