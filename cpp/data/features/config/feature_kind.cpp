#include "feature_kind.hpp"
#include <stdexcept>

std::string feature_name(FeatureKind id) {
    switch (id) {
        case FeatureKind::Return: return "ret_1d";
        case FeatureKind::LogReturn: return "logret_1d";
        case FeatureKind::ReturnVariance: return "ret_var";
        case FeatureKind::ReturnVolatility: return "ret_vol";
        case FeatureKind::PriceZ: return "price_z";
        case FeatureKind::VolumeZ: return "volume_z";
        case FeatureKind::HighLowRange: return "high_low_range";
        case FeatureKind::CloseToSma: return "close_to_sma";
    }

    throw std::runtime_error("Unsupported FeatureKind");
}

FeatureKind feature_kind_from_name(const std::string& name) {
    if (name == "ret_1d") return FeatureKind::Return;
    if (name == "logret_1d") return FeatureKind::LogReturn;
    if (name == "ret_var") return FeatureKind::ReturnVariance;
    if (name == "ret_vol") return FeatureKind::ReturnVolatility;
    if (name == "price_z") return FeatureKind::PriceZ;
    if (name == "volume_z") return FeatureKind::VolumeZ;
    if (name == "high_low_range") return FeatureKind::HighLowRange;
    if (name == "close_to_sma") return FeatureKind::CloseToSma;

    throw std::runtime_error("Unsupported feature name: " + name);
}
