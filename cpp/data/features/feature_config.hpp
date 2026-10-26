#pragma once

#include "feature_schema.hpp"

#include <cstddef>

struct FeatureConfig {
    FeatureSchema schema;
    std::size_t window;
    bool enable_cross_section = false;
};
