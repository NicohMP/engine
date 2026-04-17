#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <stdexcept>
#include <chrono>
#include "parse_csv.hpp"
#include "analytics.hpp"
#include "portfolio.hpp"
#include "base.hpp"
#include "sma.hpp"
#include "risk/risk_manager.hpp"
#include "buy_and_hold.hpp"
#include "random.hpp"
#include "backtester.hpp"

int main() {
    std::vector<std::string> tickers = {"XOM","AAPL","BNP.PA","BRK-B"};
    const int n_runs = 1000;

    std::vector<Tick> ticks = parse_csv("ml/data/ohlcv_daily_2y.csv");
    std::vector<MarketEvent> mevents = ticks_to_mevents(ticks);

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < n_runs; ++i) {
        auto strat = std::make_shared<RandomStrategy>(
            tickers,
            200.0,
            std::nullopt,
            static_cast<unsigned int>(1000 + i)
        );

        std::vector<Position> init_positions;
        Portfolio portfolio(init_positions, 2000.0);

        RiskLimits limits;
        limits.max_net_exposure = 1.0;    // if you want random orders to pass more often
        limits.max_gross_exposure = 2.0;
        RiskManager risk_manager(limits);

        Backtester backtester(mevents, strat, portfolio);
        backtester.run();
    }

    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration<double>(end - start).count();

    std::cout << "Runs: " << n_runs << "\n";
    std::cout << "Total seconds: " << seconds << "\n";
    std::cout << "Seconds per run: " << seconds / n_runs << "\n";
    std::cout << "Runs per second: " << n_runs / seconds << "\n";

    // for (const FillEvent& fill : backtester.get_fills()) {
    // std::cout
    //     << fill.timestamp << " "
    //     << fill.ticker << " "
    //     << fill.quantity << " @ "
    //     << fill.price << " fees="
    //     << fill.fees << "\n";
    // }
}
