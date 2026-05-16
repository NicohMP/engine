#include "data/features/cross_section.hpp"
#include "data/features/config/feature_kind.hpp"
#include "data/features/feature_schema.hpp"
#include "data/features/online_feature_state.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

namespace {
MarketEvent mevent(int64_t ts, SymbolId id, double close) {
    return MarketEvent{ts, id, close, close, close, 1000.0, close};
}
}

int main() {
    const SymbolId id1 = 1;
    const SymbolId id2 = 2;
    const SymbolId id3 = 3;

    OnlineFeatureState s1(id1, 3);
    OnlineFeatureState s2(id2, 3);
    OnlineFeatureState s3(id3, 3);
    FeatureSchema schema({FeatureKind::Return});

    // Warmup first close.
    bool pushed = s1.push(mevent(1, id1, 100.0));
    assert(!pushed);
    pushed = s2.push(mevent(1, id2, 100.0));
    assert(!pushed);
    pushed = s3.push(mevent(1, id3, 100.0));
    assert(!pushed);

    // Second close creates ret_1d.
    pushed = s1.push(mevent(2, id1, 110.0)); // +10%
    assert(pushed);
    pushed = s2.push(mevent(2, id2, 90.0));  // -10%
    assert(pushed);
    pushed = s3.push(mevent(2, id3, 120.0)); // +20%
    assert(pushed);

    CrossSectionSnapshot cs({id3, id1, id2}, {"ret_1d"});
    std::vector<FeatureRow> rows{
        s1.to_feature_row(schema),
        s2.to_feature_row(schema),
        s3.to_feature_row(schema),
    };
    bool ok = cs.push_snapshot(2, rows);
    assert(ok);
    assert(cs.ready());

    const CrossSectionResult out = cs.compute();
    assert(out.timestamp == 2);
    assert((out.symbols == std::vector<SymbolId>{id1, id2, id3}));
    assert(out.raw.size() == 3);
    assert(out.raw[0].size() == 1);

    // symbols are sorted -> [id1, id2, id3] => [0.1, -0.1, 0.2]
    assert(std::abs(out.raw[0][0] - 0.1) < 1e-12);
    assert(std::abs(out.raw[1][0] + 0.1) < 1e-12);
    assert(std::abs(out.raw[2][0] - 0.2) < 1e-12);

    // rank_pct ascending with pct = rank / n (average ties): [-0.1, 0.1, 0.2] => [1/3, 2/3, 1]
    assert(out.rank_pct[0][0].has_value());
    assert(out.rank_pct[1][0].has_value());
    assert(out.rank_pct[2][0].has_value());
    assert(std::abs(*out.rank_pct[0][0] - (2.0 / 3.0)) < 1e-12);
    assert(std::abs(*out.rank_pct[1][0] - (1.0 / 3.0)) < 1e-12);
    assert(std::abs(*out.rank_pct[2][0] - 1.0) < 1e-12);

    // z-scores should be finite for all three here.
    assert(out.zscore[0][0].has_value());
    assert(out.zscore[1][0].has_value());
    assert(out.zscore[2][0].has_value());

    // A snapshot containing NaN is accepted but flagged invalid by return value.
    cs.reset();
    const double kNaN = std::numeric_limits<double>::quiet_NaN();
    ok = cs.push_snapshot(3, {
        FeatureRow{id1, {kNaN}},
        FeatureRow{id2, {0.0}},
        FeatureRow{id3, {0.1}},
    });
    assert(!ok);
    const CrossSectionResult out_nan = cs.compute();
    assert(!out_nan.rank_pct[0][0].has_value());
    assert(!out_nan.zscore[0][0].has_value());
    assert(out_nan.rank_pct[1][0].has_value());
    assert(out_nan.rank_pct[2][0].has_value());

    return 0;
}
