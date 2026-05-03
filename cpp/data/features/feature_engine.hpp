#pragma once

#include "core/event_types.hpp"
#include "feature_schema.hpp"
#include "feature_types.hpp"
#include "online_feature_state.hpp"
#include "cross_section.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

class FeatureEngine {
public:
    FeatureEngine(
        std::vector<SymbolId> universe,
        FeatureSchema schema,
        std::size_t window
    );

    bool push(const MarketEvent& event);

    bool ready(SymbolId id) const;
    bool ready_all() const;

    const FeatureSchema& schema() const;
    const std::vector<SymbolId>& universe() const;

    const FeatureRow* latest_row(SymbolId id) const;
    std::vector<FeatureRow> latest_rows() const;

    std::optional<CrossSectionResult> compute_cross_section(int64_t timestamp) const;

    void reset();

private:
    FeatureSchema schema_; //
    std::vector<SymbolId> universe_; //
    std::unordered_map<SymbolId, OnlineFeatureState> states_;
    std::unordered_map<SymbolId, FeatureRow> latest_rows_;
};
