// analytics.hpp
#pragma once
#include "tick.hpp"
#include "core/event_types.hpp"
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
    double initial_equity = 0.0;
    double final_equity = 0.0;
    double gross_pnl = 0.0; // before fees
    double net_pnl = 0.0;   // after fees
    double total_return = 0.0;
    double volatility = 0.0;
    double max_drawdown = 0.0;
    size_t num_orders = 0;
    double pnl_per_order = 0.0;
    double total_traded_notional = 0.0;
    double total_fees_paid = 0.0;
    double implied_fee_rate = 0.0;
    double fees_if_10bps = 0.0;
    double fees_if_7_5bps = 0.0;
    double net_pnl_if_10bps = 0.0;
    double net_pnl_if_7_5bps = 0.0;
};

BacktestReport compute_backtest_report(
    const std::vector<std::pair<int64_t, double>>& equity,
    const std::vector<OrderEvent>& orders,
    const std::vector<FillEvent>& fills
);
void display_backtest_report(const BacktestReport& report);
