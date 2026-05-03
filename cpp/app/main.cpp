#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "backtester.hpp"
#include "data/store/market_data_store.hpp"
#include "data/extract/parse_csv.hpp"
#include "portfolio.hpp"
#include "random.hpp"

int main() {
    const std::vector<std::string> tickers = {"XOM", "AAPL", "BNP.PA", "BRK-B"};
    // const std::vector<std::string> tickers = {"AAPL"};
    const int n_runs = 1000;
    const std::vector<Tick> ticks = parse_csv("ml/data/ohlcv_daily_2y.csv");

    BuildResult res = MarketDataStore::from_ticks(ticks);
    BuildStats stats = res.stats;
    std::cout << "=== MarketDataStore Build Stats ===\n";
    std::cout << "Input rows: " << stats.input_rows << "\n";
    std::cout << "Kept rows: " << stats.kept_rows << "\n";
    std::cout << "Dropped invalid: " << stats.dropped_invalid << "\n";
    std::cout << "Dropped duplicates: " << stats.dropped_duplicates << "\n";
    std::cout << "Num symbols: " << stats.num_symbols << "\n";
    std::cout << "Num slices: " << stats.num_slices << "\n";

    MarketDataStore data = std::move(res.store);
    std::vector<SymbolId> ids;
    ids.reserve(tickers.size());
    for (const std::string& ticker : tickers) {
        ids.push_back(data.get_id(ticker));
    }

    FeatureConfig feature_config{FeatureSchema(std::vector<std::string>{"ret_1d"}), 20, true};

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < n_runs; ++i) {
        auto strat = std::make_shared<RandomStrategy>(
            ids, 200.0, std::nullopt, static_cast<unsigned int>(1000 + i)
        );
        Portfolio portfolio({}, 2000.0);
        Backtester backtester(data, strat, portfolio, feature_config);
        backtester.run();
    }
    auto end = std::chrono::steady_clock::now();

    const double seconds = std::chrono::duration<double>(end - start).count();
    std::cout << "Runs: " << n_runs << "\n";
    std::cout << "Total seconds: " << seconds << "\n";
    std::cout << "Seconds per run: " << (seconds / n_runs) << "\n";
    std::cout << "Runs per second: " << (n_runs / seconds) << "\n";
}
