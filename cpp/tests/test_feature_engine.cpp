#include "data/features/feature_engine.hpp"
#include "data/features/feature_id.hpp"
#include "data/features/feature_schema.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <vector>

namespace {
MarketEvent mevent(int64_t ts, SymbolId id, double close, double volume = 1000.0) {
    return MarketEvent{ts, id, close, close, close, volume, close};
}
}

int main() {
    const SymbolId id1 = 1;
    const SymbolId id2 = 2;
    FeatureSchema schema({
        FeatureId::Ret1D,
        FeatureId::LogRet1D,
        FeatureId::RetVar,
        FeatureId::RetVol,
        FeatureId::PriceZ,
        FeatureId::VolumeZ,
        FeatureId::CloseToSma,
    });
    FeatureEngine engine({id2, id1}, schema, 3);

    bool produced = engine.push(mevent(1, id1, 100.0));
    assert(!produced);
    produced = engine.push(mevent(1, id2, 100.0));
    assert(!produced);
    assert(!engine.ready_all());

    produced = engine.push(mevent(2, id1, 110.0));
    assert(!produced);
    produced = engine.push(mevent(2, id2, 90.0));
    assert(!produced);

    produced = engine.push(mevent(3, id1, 121.0));
    assert(!produced);
    produced = engine.push(mevent(3, id2, 81.0));
    assert(!produced);

    produced = engine.push(mevent(4, id1, 133.1));
    assert(produced);
    produced = engine.push(mevent(4, id2, 72.9));
    assert(produced);
    assert(engine.ready(id1));
    assert(engine.ready(id2));
    assert(engine.ready_all());

    std::optional<FeatureRow> maybe_row = engine.latest_row(id1);
    assert(maybe_row.has_value());
    const FeatureRow& row = *maybe_row;
    assert(row.symbol_id == id1);
    assert(row.values.size() == schema.size());
    assert(std::abs(row.values[schema.index_of(FeatureId::Ret1D)] - 0.1) < 1e-12);
    assert(std::abs(row.values[schema.index_of(FeatureId::RetVar)]) < 1e-12);
    assert(std::isfinite(row.values[schema.index_of(FeatureId::PriceZ)]));
    assert(std::isfinite(row.values[schema.index_of(FeatureId::CloseToSma)]));

    const std::vector<FeatureRow> rows = engine.latest_rows();
    assert(rows.size() == 2);
    assert(rows[0].symbol_id == id1);
    assert(rows[1].symbol_id == id2);

    std::optional<CrossSectionResult> xs = engine.compute_cross_section(4);
    assert(xs.has_value());
    assert(xs->timestamp == 4);
    assert((xs->symbols == std::vector<SymbolId>{id1, id2}));
    assert(xs->raw.size() == 2);
    assert(xs->raw[0].size() == schema.size());
    assert(xs->rank_pct[0][schema.index_of(FeatureId::Ret1D)].has_value());
    assert(xs->rank_pct[1][schema.index_of(FeatureId::Ret1D)].has_value());

    engine.reset();
    assert(!engine.ready_all());
    assert(!engine.latest_row(id1).has_value());

    return 0;
}
