#include "feature_engine.hpp"
#include <unordered_set>
#include <optional>

FeatureEngine::FeatureEngine(std::vector<SymbolId> universe,FeatureSchema schema,std::size_t window):
    schema_(schema){
        // Dedupe universe
        std::unordered_set<SymbolId> seen;
        for (const SymbolId& id: universe){
            if (seen.insert(id).second){
                universe_.push_back(id);
            }
        }
        // Build feature states from clean universe
        for (const SymbolId& id:universe_){
            states_.emplace(id,OnlineFeatureState(id,window));
        }
}

bool FeatureEngine::push(const MarketEvent& event){
    // Check event id supported
    auto it = states_.find(event.symbol_id);
    if (it == states_.end()) {
        throw std::runtime_error("Id " + std::to_string(event.symbol_id) + " not supported.");
    }

    bool accepted = it->second.push(event);
    if (!accepted || !it->second.ready()) {
        return false;
    }

    latest_rows_[event.symbol_id] = it->second.to_feature_row(schema_);
    return true;
}

bool FeatureEngine::ready(SymbolId id) const{
    if (states_.find(id) == states_.end()){
        throw std::runtime_error("Id "+std::to_string(id)+" not supported.");
    }
    return states_.at(id).ready();
}

bool FeatureEngine::ready_all() const{
    bool ready = true;
    for (const auto& [id,state]: states_){
        if (!state.ready()){
            ready = false;
        }
    }
    return ready;
}

const FeatureSchema& FeatureEngine::schema() const{
    return schema_;
}

const std::vector<SymbolId>& FeatureEngine::universe() const{
    return universe_;
}

const FeatureRow* FeatureEngine::latest_row(SymbolId id) const{
    auto it = latest_rows_.find(id);
    if (it == latest_rows_.end()) return nullptr;
    return &it->second;
}

std::vector<FeatureRow> FeatureEngine::latest_rows() const{
    // Build rows vector and allocate memory
    std::vector<FeatureRow> rows;
    rows.reserve(latest_rows_.size());
    
    // Fill with rows
    for (SymbolId id : universe_) {
        auto it = latest_rows_.find(id);
        if (it != latest_rows_.end()) {
            rows.push_back(it->second);
        }
    }
    return rows;
}

std::optional<CrossSectionResult> FeatureEngine::compute_cross_section(int64_t timestamp) const{
    // Check that all states are ready
    if (!ready_all()) {
        return std::nullopt;
    }
    // Build snapshot class and try to push
    CrossSectionSnapshot css(universe_,schema_.names());
    bool ok = css.push_snapshot(timestamp, latest_rows());
    // If push fails, return None
    if (!ok) {
        return std::nullopt;
    }
    // If push succeed, compute result
    return css.compute();
}

void FeatureEngine::reset(){
    latest_rows_.clear();
    for (auto& [id, state] : states_) {
        state.reset();
    }
}
