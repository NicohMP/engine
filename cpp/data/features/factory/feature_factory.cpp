#include "feature_factory.hpp"

#include "data/features/calculators/returns.hpp"

#include <stdexcept>

std::unique_ptr<OnlineFeature> make_feature(const FeatureSpec& spec) {
    switch (spec.kind) {
        case FeatureKind::Return:
            return std::make_unique<Returns>(spec.window);
        default:
            throw std::runtime_error("Unsupported feature kind in factory");
    }
}

std::vector<std::unique_ptr<OnlineFeature>> make_features(const std::vector<FeatureSpec>& specs) {
    std::vector<std::unique_ptr<OnlineFeature>> features;
    features.reserve(specs.size());
    for (const FeatureSpec& spec : specs) {
        features.push_back(make_feature(spec));
    }
    return features;
}
