#include "data/features/feature_id.hpp"
#include "data/features/feature_schema.hpp"
#include "data/features/online_feature_state.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
MarketEvent make_event(int64_t ts, SymbolId id, double close, double volume = 1000.0) {
    return MarketEvent{
        ts,
        id,
        close,
        close,
        close,
        volume,
        close,
    };
}
}

int main() {
    {
        const SymbolId id = 7;
        OnlineFeatureState state(id, 3);
        FeatureSchema schema({
            FeatureId::Ret1D,
            FeatureId::LogRet1D,
            FeatureId::RetVar,
            FeatureId::RetVol,
            FeatureId::PriceZ,
            FeatureId::VolumeZ,
            FeatureId::HighLowRange,
            FeatureId::CloseToSma,
        });

        // First bar: initializes previous close only.
        bool pushed = state.push(make_event(1, id, 100.0, 1000.0));
        assert(!pushed);
        assert(!state.ready());
        FeatureRow row = state.to_feature_row(schema);
        assert(row.symbol_id == id);
        assert(row.values.size() == schema.size());
        assert(std::isnan(row.values[schema.index_of(FeatureId::Ret1D)]));
        assert(std::isnan(row.values[schema.index_of(FeatureId::LogRet1D)]));
        assert(std::isnan(row.values[schema.index_of(FeatureId::RetVar)]));
        assert(std::isnan(row.values[schema.index_of(FeatureId::PriceZ)]));
        assert(std::abs(row.values[schema.index_of(FeatureId::HighLowRange)]) < 1e-12);

        // Second bar: first return is available.
        pushed = state.push(make_event(2, id, 110.0, 1100.0));
        assert(pushed);
        assert(!state.ready());
        row = state.to_feature_row(schema);
        assert(std::abs(row.values[schema.index_of(FeatureId::Ret1D)] - 0.1) < 1e-12);
        assert(std::abs(row.values[schema.index_of(FeatureId::LogRet1D)] - std::log(1.1)) < 1e-12);
        assert(std::isnan(row.values[schema.index_of(FeatureId::RetVar)]));

        // Third bar: price rolling window ready, return rolling window still warming up.
        pushed = state.push(make_event(3, id, 121.0, 1200.0));
        assert(pushed);
        assert(!state.ready());
        row = state.to_feature_row(schema);
        assert(std::abs(row.values[schema.index_of(FeatureId::Ret1D)] - 0.1) < 1e-12);
        assert(std::isfinite(row.values[schema.index_of(FeatureId::PriceZ)]));
        assert(std::isfinite(row.values[schema.index_of(FeatureId::VolumeZ)]));
        assert(std::isfinite(row.values[schema.index_of(FeatureId::CloseToSma)]));

        // Fourth bar: return rolling window ready, returns are constant => zero variance.
        pushed = state.push(make_event(4, id, 133.1, 1300.0));
        assert(pushed);
        assert(state.ready());
        row = state.to_feature_row(schema);
        assert(std::abs(row.values[schema.index_of(FeatureId::Ret1D)] - 0.1) < 1e-12);
        assert(std::abs(row.values[schema.index_of(FeatureId::RetVar)]) < 1e-12);
        assert(std::abs(row.values[schema.index_of(FeatureId::RetVol)]) < 1e-12);
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
