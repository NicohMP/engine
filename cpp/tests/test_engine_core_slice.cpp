#include "core/engine_core.hpp"
#include "data/features/config/feature_kind.hpp"
#include "data/features/feature_schema.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <vector>

namespace {
MarketEvent mevent(int64_t ts, SymbolId id, double close) {
    return MarketEvent{ts, id, close, close, close, 1000.0, close};
}

class InspectCrossSectionStrategy : public Strategy {
public:
    int cross_section_events = 0;

    void reset() override {
        cross_section_events = 0;
    }

    std::optional<std::vector<OrderEvent>> on_market_event(
        const MarketEvent& mevent,
        const Portfolio& portfolio,
        const FeatureView& features
    ) override {
        (void)portfolio;
        if (mevent.timestamp == 2) {
            assert(features.has_row());
            assert(features.has_cross_section());
            assert(features.cross_section->timestamp == 2);
            assert((features.cross_section->symbols == std::vector<SymbolId>{1, 2}));
            assert(features.cross_section->raw.size() == 2);
            ++cross_section_events;
        }
        return std::nullopt;
    }
};
}

int main() {
    auto strategy = std::make_shared<InspectCrossSectionStrategy>();
    FeatureConfig feature_config{
        FeatureSchema(std::vector<FeatureKind>{FeatureKind::Return}),
        1,
        true
    };
    EngineCore engine({2, 1}, strategy, Portfolio({}, 1000.0), feature_config);

    const std::vector<MarketEvent> warmup{
        mevent(1, 1, 100.0),
        mevent(1, 2, 100.0),
    };
    engine.step_slice(warmup);
    assert(strategy->cross_section_events == 0);

    const std::vector<MarketEvent> ready{
        mevent(2, 1, 110.0),
        mevent(2, 2, 90.0),
    };
    engine.step_slice(ready);
    assert(strategy->cross_section_events == 2);

    return 0;
}
