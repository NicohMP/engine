#pragma once

#include "data/features/config/feature_spec.hpp"
#include "data/features/online_feature.hpp"

#include <memory>
#include <vector>

std::unique_ptr<OnlineFeature> make_feature(const FeatureSpec& spec);
std::vector<std::unique_ptr<OnlineFeature>> make_features(const std::vector<FeatureSpec>& specs);
