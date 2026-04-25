#include "data/features/online_feature_state.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
MarketEvent make_event(int64_t ts, SymbolId id, double close) {
    return MarketEvent{
        ts,
        id,
        close,
        close,
        close,
        1000.0,
        close,
    };
}
}

int main() {
    {
        const SymbolId id = 7;
        OnlineFeatureState state(id, 3);

        // First bar: initializes previous close only.
        assert(!state.push(make_event(1, id, 100.0)));
        assert(!state.ready());
        assert(state.price_z() == std::nullopt);

        bool ret_threw = false;
        try {
            (void)state.ret_1d();
        } catch (const std::runtime_error&) {
            ret_threw = true;
        }
        assert(ret_threw);

        // Second bar: first return is available.
        assert(state.push(make_event(2, id, 110.0)));
        assert(!state.ready());
        assert(std::abs(state.ret_1d() - 0.1) < 1e-12);

        bool vol_threw = false;
        try {
            (void)state.vol_pop();
        } catch (const std::runtime_error&) {
            vol_threw = true;
        }
        assert(vol_threw);

        // Third bar: price rolling window ready, return rolling window still warming up.
        assert(state.push(make_event(3, id, 121.0)));
        assert(!state.ready());
        assert(std::abs(state.ret_1d() - 0.1) < 1e-12);
        const auto z = state.price_z();
        assert(z.has_value());

        // Fourth bar: return rolling window ready, returns are constant => zero variance.
        assert(state.push(make_event(4, id, 133.1)));
        assert(state.ready());
        assert(std::abs(state.ret_1d() - 0.1) < 1e-12);
        assert(std::abs(state.vol_pop()) < 1e-12);
    }

    {
        const SymbolId id = 9;
        OnlineFeatureState state(id, 2);

        bool id_threw = false;
        try {
            (void)state.push(make_event(1, id + 1, 100.0));
        } catch (const std::runtime_error&) {
            id_threw = true;
        }
        assert(id_threw);

        bool close_threw = false;
        try {
            (void)state.push(make_event(1, id, std::numeric_limits<double>::quiet_NaN()));
        } catch (const std::runtime_error&) {
            close_threw = true;
        }
        assert(close_threw);
    }

    return 0;
}
