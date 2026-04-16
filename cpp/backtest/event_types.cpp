#include "event_types.hpp"

#include <atomic>

namespace {
std::atomic<std::uint64_t> g_next_order_id{1};
}

std::uint64_t generate_order_id() {
    return g_next_order_id.fetch_add(1, std::memory_order_relaxed);
}

void reset_order_id_counter(std::uint64_t start_id) {
    g_next_order_id.store(start_id, std::memory_order_relaxed);
}


std::vector<MarketEvent> ticks_to_mevents(const std::vector<Tick>& ticks) {
    std::vector<MarketEvent> mevents;
    mevents.reserve(ticks.size());
    for (const Tick& tick : ticks) {
        mevents.push_back(MarketEvent{
            tick.timestamp,
            tick.ticker,
            tick.open,
            tick.high,
            tick.low,
            tick.volume,
            tick.close
        });
    }
    return mevents;
}

MarketEvent tick_to_mevent(const Tick& tick) {
    return MarketEvent{
        tick.timestamp,
        tick.ticker,
        tick.open,
        tick.high,
        tick.low,
        tick.volume,
        tick.close
    };
}
