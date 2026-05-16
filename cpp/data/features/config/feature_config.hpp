#pragma once

#include "feature_spec.hpp"

#include <vector>


struct FeatureConfig {
    std::vector<FeatureSpec> features;
    bool enable_cross_section = false;
};
