#pragma once

#include "feature_kind.hpp"

#include <cstddef>
#include <string>

struct FeatureSpec {
    FeatureKind kind;
    std::string name;
    std::size_t window;
    // Other feature-specific params can be added here.
};
