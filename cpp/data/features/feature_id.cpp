#include "feature_id.hpp"
#include <stdexcept>

std::string feature_name(FeatureId id) {
    switch (id) {
        case FeatureId::Ret1D: return "ret_1d";
        case FeatureId::LogRet1D: return "logret_1d";
        case FeatureId::RetVar: return "ret_var";
        case FeatureId::RetVol: return "ret_vol";
        case FeatureId::PriceZ: return "price_z";
        case FeatureId::VolumeZ: return "volume_z";
        case FeatureId::HighLowRange: return "high_low_range";
        case FeatureId::CloseToSma: return "close_to_sma";
    }

    throw std::runtime_error("Unsupported FeatureId");
}

FeatureId feature_id_from_name(const std::string& name) {
    if (name == "ret_1d") return FeatureId::Ret1D;
    if (name == "logret_1d") return FeatureId::LogRet1D;
    if (name == "ret_var") return FeatureId::RetVar;
    if (name == "ret_vol") return FeatureId::RetVol;
    if (name == "price_z") return FeatureId::PriceZ;
    if (name == "volume_z") return FeatureId::VolumeZ;
    if (name == "high_low_range") return FeatureId::HighLowRange;
    if (name == "close_to_sma") return FeatureId::CloseToSma;

    throw std::runtime_error("Unsupported feature name: " + name);
}
