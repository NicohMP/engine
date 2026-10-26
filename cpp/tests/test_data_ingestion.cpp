#include "data/extract/parse_csv.hpp"
#include "data/extract/tick.hpp"

#include <vector>
#include <cassert>
#include <cstdint>
#include <string>


int main(){
    const std::int64_t t = timestamp_to_int64("2026-12-08");
    assert(t == 20261208);
    return 0;

    const Tick tick = parse_line_into_tick(
    "2026-12-08,AAPL,100,105,95,1000,102"
    );

    assert(tick.timestamp == 20261208);
    assert(tick.ticker == "AAPL");
    assert(tick.open == 100.0);
    assert(tick.high == 105.0);
    assert(tick.low == 95.0);
    assert(tick.volume == 1000.0);
    assert(tick.close == 102.0);
}