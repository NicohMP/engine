#pragma once

#include "core/event_types.hpp"
#include "feature_schema.hpp"
#include "feature_types.hpp"
#include "online_feature_state.hpp"
#include "cross_section.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
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

    const CrossSectionResult* compute_cross_section_view(int64_t timestamp);
    std::optional<CrossSectionResult> compute_cross_section(int64_t timestamp);

    void reset();

private:
    FeatureSchema schema_; //
    std::vector<SymbolId> universe_; //
    std::unordered_map<SymbolId, OnlineFeatureState> states_;
    std::unordered_map<SymbolId, FeatureRow> latest_rows_;
    std::optional<CrossSectionSnapshot> cross_section_;
    CrossSectionResult latest_cross_section_;
    std::vector<const FeatureRow*> latest_row_ptrs_;
    std::vector<std::pair<double, std::size_t>> cross_section_valid_;
};
