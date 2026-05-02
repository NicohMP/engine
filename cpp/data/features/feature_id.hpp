#pragma once
#include <cstddef>
#include <string>

// Authority supported feature list
enum class FeatureId {
    Ret1D,
    LogRet1D,
    RetVar,
    RetVol,
    PriceZ,
    VolumeZ,
    HighLowRange,
    CloseToSma
};

struct FeatureIdHash {
    std::size_t operator()(FeatureId id) const {
        return static_cast<std::size_t>(id);
    }
};

std::string feature_name(FeatureId id);
FeatureId feature_id_from_name(const std::string& name);
