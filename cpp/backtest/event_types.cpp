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
