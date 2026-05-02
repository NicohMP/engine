#pragma once

#include "core/event_types.hpp"
#include "feature_schema.hpp"
#include <vector>

struct CrossSectionResult;

struct FeatureRow {
    SymbolId symbol_id;
    std::vector<double> values; // size must match feature schema
};

struct FeatureView {
    int64_t timestamp = 0;
    const FeatureSchema* schema = nullptr;
    const FeatureRow* row = nullptr;
    const CrossSectionResult* cross_section = nullptr;

    bool has_row() const { return row != nullptr; }
    bool has_cross_section() const { return cross_section != nullptr; }
};
