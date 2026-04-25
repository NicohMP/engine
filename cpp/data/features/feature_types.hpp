#pragma once

#include "core/event_types.hpp"

#include <vector>

struct FeatureRow {
    SymbolId symbol_id;
    std::vector<double> values; // size must match feature schema
};

