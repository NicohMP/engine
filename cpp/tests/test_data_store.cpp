#include "data/store/market_data_store.hpp"

#include <cassert>
#include <vector>

int main() {
    assert(normalize_ticker("  aapl  ") == "AAPL");
    assert(normalize_ticker("\tMsFt\n") == "MSFT");
    assert(normalize_ticker("   ") == "");

    const std::vector<Tick> fake_ticks = {
        {1, "  aapl", 100.0, 101.0, 99.0, 1000.0, 100.5},
        {1, "AAPL", 100.0, 101.0, 99.0, 900.0, 100.7},   // duplicate key
        {1, "msft", 200.0, 201.0, 199.0, 800.0, 200.5},
        {2, "MSFT", 201.0, 202.0, 200.0, 700.0, 201.5},
        {3, "BTC", 300.0, 299.0, 298.0, 600.0, 299.5},   // invalid (high < open)
    };

    const BuildResult built = MarketDataStore::from_ticks(fake_ticks);
    assert(built.stats.input_rows == 5);
    assert(built.stats.dropped_invalid == 1);
    assert(built.stats.dropped_duplicates == 1);
    assert(built.stats.kept_rows == 3);
    assert(built.stats.num_symbols == 2);
    assert(built.stats.num_slices == 2);

    return 0;
}
