#pragma once
#include <cstddef>
#include <string>

// Authority supported feature list
enum class FeatureKind {
    Return,
    LogReturn,
    ReturnVariance,
    ReturnVolatility,
    PriceZ,
    VolumeZ,
    HighLowRange,
    CloseToSma
};

struct FeatureKindHash {
    std::size_t operator()(FeatureKind id) const {
        return static_cast<std::size_t>(id);
    }
};

std::string feature_name(FeatureKind id);
FeatureKind feature_kind_from_name(const std::string& name);
