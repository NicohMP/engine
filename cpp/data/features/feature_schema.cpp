#include "feature_schema.hpp"
#include <unordered_set>
#include <stdexcept>

// Construct from vector of feature names (string)
FeatureSchema::FeatureSchema(const std::vector<std::string>& names){
    // Check non-empty names
    if (names.empty()){
        throw std::runtime_error("Input names can't be empty");
    }
    // Throw if duplicate names
    std::unordered_set<std::string> seen;
    for (const std::string& name: names){
        if (!seen.insert(name).second) {
            throw std::runtime_error("Duplicate feature name: " + name);
        }
        names_.push_back(name);
    }
    // Fill index map
    for (size_t i =0; i<names_.size(); i++){
        features_.push_back(feature_id_from_name(names_[i])); // feature_id_from_name throws if name is unsupported
        index_.emplace(features_[i],i);

    }
}

// Construct from vector of features (FeatureId)
FeatureSchema::FeatureSchema(std::vector<FeatureId> features){
    // Check non-empty features
    if (features.empty()){
        throw std::runtime_error("Input features can't be empty");
    }
    // Throw if duplicate names, fill features_ and names_
    std::unordered_set<FeatureId, FeatureIdHash> seen;
    for (const FeatureId& feat: features){
        if (!seen.insert(feat).second) {
            throw std::runtime_error("Duplicate feature id: " + feature_name(feat));
        }
        features_.push_back(feat);
        names_.push_back(feature_name(feat));
    }
    // Fill feat/index map
    for (size_t i =0; i<features_.size(); i++){
        index_.emplace(features_[i],i);
    }
}



bool FeatureSchema::empty() const{
    return features_.empty();
}

std::size_t FeatureSchema::size() const{
    return features_.size();
}

const std::vector<std::string>& FeatureSchema::names() const {
    if (empty()){
        throw std::runtime_error("No names in schema");
    }
    return names_;
}

const std::string& FeatureSchema::name(std::size_t idx) const{
    if (idx >= names_.size()){
        throw std::runtime_error("Index out of range");
    }
    return names_[idx];
}

const std::vector<FeatureId>& FeatureSchema::features() const{
    if (empty()){
        throw std::runtime_error("No features in schema");
    }
    return features_;
}

FeatureId FeatureSchema::feature(std::size_t idx) const{
    if (idx >= features_.size()){ throw std::runtime_error("Index out of range");}
    return features_[idx];
}


bool FeatureSchema::contains(FeatureId id) const{
    return index_.find(id) != index_.end();
}

std::size_t FeatureSchema::index_of(FeatureId id) const{
    auto it = index_.find(id);
    if (it == index_.end()) {
        throw std::runtime_error("Feature id not supported: " + feature_name(id));
    }
    return it->second;
}
