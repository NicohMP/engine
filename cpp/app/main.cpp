#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <stdexcept>
#include "parse_csv.hpp"
#include "analytics.hpp"
#include "portfolio.hpp"
#include "base.hpp"
#include "sma_strategy.hpp"
#include "backtester.hpp"

int main() {
    std::string ticker = "0GFDUSD";
    std::vector<Tick> ticks =  parse_kline_csv("ml/data/0GFDUSD-1m-2025-10.csv", ticker);
    std::vector<MarketEvent> mevents = ticks_to_mevents(ticks);
    auto strat = std::make_unique<SMAStrategy>(20,100,-50.0,50.0,ticker);
    Position pos(ticker, 0.0);
    std::vector<Position> init_positions;
    init_positions.push_back(pos);
    Portfolio portfolio(init_positions, 2000.0);
    Backtester backtester(ticker,mevents, std::move(strat), portfolio);
    backtester.run();
    BacktestReport report = compute_backtest_report(
        backtester.get_equity(),
        backtester.get_orders(),
        backtester.get_fills()
    );
    display_backtest_report(report);
}
