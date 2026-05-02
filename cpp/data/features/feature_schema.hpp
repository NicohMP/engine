#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include "feature_id.hpp"

class FeatureSchema {
public:
    explicit FeatureSchema(std::vector<FeatureId> features);
    explicit FeatureSchema(const std::vector<std::string>& names);

    std::size_t size() const;
    bool empty() const;

    const std::vector<FeatureId>& features() const;
    const std::vector<std::string>& names() const;

    FeatureId feature(std::size_t idx) const;
    const std::string& name(std::size_t idx) const;

    bool contains(FeatureId id) const;
    std::size_t index_of(FeatureId id) const;

private:
    std::vector<FeatureId> features_;
    std::vector<std::string> names_;
    std::unordered_map<FeatureId, std::size_t, FeatureIdHash> index_;
};
