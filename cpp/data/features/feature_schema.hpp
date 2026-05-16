#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include "config/feature_kind.hpp"

class FeatureSchema {
public:
    explicit FeatureSchema(std::vector<FeatureKind> features);
    explicit FeatureSchema(const std::vector<std::string>& names);

    std::size_t size() const;
    bool empty() const;

    const std::vector<FeatureKind>& features() const;
    const std::vector<std::string>& names() const;

    FeatureKind feature(std::size_t idx) const;
    const std::string& name(std::size_t idx) const;

    bool contains(FeatureKind id) const;
    std::size_t index_of(FeatureKind id) const;

private:
    std::vector<FeatureKind> features_;
    std::vector<std::string> names_;
    std::unordered_map<FeatureKind, std::size_t, FeatureKindHash> index_;
};
