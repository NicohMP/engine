#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <stdexcept>
#include "parse_csv.hpp"
#include "analytics.hpp"
#include "strategy.hpp"
#include "strategies/sma_strategy.hpp"
#include "backtester.hpp"

int main() {
    std::string ticker = "0GFDUSD";
    std::vector<Tick> ticks =  parse_kline_csv("0GFDUSD-1m-2025-10.csv", ticker);
    auto strat = std::make_unique<SMAStrategy>(20,100,-200.0,200.0);
    AccountState initial_account_state{2000.0, 0.0};
    Backtester backtester(ticker,ticks, std::move(strat), initial_account_state);
    backtester.run();
    BacktestReport report = compute_backtest_report(backtester.get_equity(), backtester.get_trades());
    display_backtest_report(report);
}
