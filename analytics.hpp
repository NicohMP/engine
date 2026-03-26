// analytics.hpp
#pragma once
#include "tick.hpp"
#include "trading_types.hpp"
#include <cstddef>
#include <string>
#include <vector>
#include <utility>

class SymbolSeries {
    private :
        std::vector<Tick> ticks;
        std::string ticker;
    public :
        SymbolSeries(const std::vector<Tick>& ticks, const std::string& ticker);
        double min_price(double Tick::* field) const;
        double max_price(double Tick::* field) const;
        double avg_price(double Tick::* field) const;
        std::vector<double> mva(double Tick::* field, size_t window) const;
}; 


struct BacktestReport {
    double final_equity = 0.0;
    double total_return = 0.0;
    double volatility = 0.0;
    double max_drawdown = 0.0;
    size_t num_trades = 0;
};

BacktestReport compute_backtest_report(const std::vector<std::pair<int64_t, double>>& equity,const std::vector<Trade>& trades);
void display_backtest_report(const BacktestReport& report);