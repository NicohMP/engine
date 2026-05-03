#include "cross_section.hpp"
#include <unordered_set>
#include <limits>
#include <cmath>
#include <algorithm>

CrossSectionSnapshot::CrossSectionSnapshot(
    std::vector<SymbolId> universe_ids,std::vector<std::string> feature_names)
    {
     // Remove duplicates from universe
        std::unordered_set<SymbolId> seen;
        for (const SymbolId& id : universe_ids) {
            if (seen.insert(id).second) {
                universe_.push_back(id);
            }
        }
        // Check if universe is empty
        if (universe_.empty()) {
            throw std::runtime_error("Universe cannot be empty");
        }
        // Sort id values 
        std::sort(universe_.begin(),universe_.end(),[](const SymbolId& a, const SymbolId& b) {return a < b;});
        // Create canonical order for feature rows based on the ids order. 
        for (size_t i = 0; i<universe_.size(); i++){
            id_to_row_.emplace(universe_[i],i);
        }
        // Deduplicate features
        std::unordered_set<std::string> seen_f;
        for (const std::string& feat : feature_names) {
            if (seen_f.insert(feat).second) {
                feature_names_.push_back(feat);
            }
        }
        // values_ is a matrix of fixed shape nb_assets * nb_features, initially filled with NaNs
        const double nan = std::numeric_limits<double>::quiet_NaN();
        values_.assign(universe_.size(),std::vector<double>(feature_names_.size(), nan));

        
}

bool CrossSectionSnapshot::push_snapshot(int64_t ts, const std::vector<FeatureRow>& rows){
    // Check no timstamps mix up, assing new ts if timestamp_ is None. 
    if (timestamp_.has_value()){
        if (*timestamp_ != ts) {
            throw std::runtime_error("Combining different timestamps in the same snapshot");
        }
    } else {timestamp_ = ts;}

    // Refill values_ w/ NaNs, no shape changes.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    for (auto& row : values_) {std::fill(row.begin(), row.end(), nan);}
    
    // Fill with values from rows
    bool all_valid = true;
    for (const FeatureRow& row: rows){
        if (std::find(universe_.begin(), universe_.end(),row.symbol_id) == universe_.end()) { 
            throw std::runtime_error("Unsupported asset with id" + std::to_string(row.symbol_id));
        }
        if (row.values.size() != feature_names_.size()){
            throw std::runtime_error("Missing values in feature row for id" + std::to_string(row.symbol_id));
        }
        for (const double& v: row.values){
            if (!std::isfinite(v)) {all_valid = false;}
        }
        size_t r = id_to_row_.at(row.symbol_id);
        values_[r] = row.values;
    }
    timestamp_ = ts;
    return all_valid;
}

bool CrossSectionSnapshot::push_snapshot(int64_t ts, const std::vector<const FeatureRow*>& rows){
    // Check no timstamps mix up, assing new ts if timestamp_ is None. 
    if (timestamp_.has_value()){
        if (*timestamp_ != ts) {
            throw std::runtime_error("Combining different timestamps in the same snapshot");
        }
    } else {timestamp_ = ts;}

    // Refill values_ w/ NaNs, no shape changes.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    for (auto& row : values_) {std::fill(row.begin(), row.end(), nan);}
    
    // Fill with values from rows without copying FeatureRow containers.
    bool all_valid = rows.size() == universe_.size();
    for (const FeatureRow* row: rows){
        if (row == nullptr) {
            all_valid = false;
            continue;
        }
        auto it = id_to_row_.find(row->symbol_id);
        if (it == id_to_row_.end()) { 
            throw std::runtime_error("Unsupported asset with id" + std::to_string(row->symbol_id));
        }
        if (row->values.size() != feature_names_.size()){
            throw std::runtime_error("Missing values in feature row for id" + std::to_string(row->symbol_id));
        }
        for (const double& v: row->values){
            if (!std::isfinite(v)) {all_valid = false;}
        }
        std::vector<double>& target = values_[it->second];
        std::copy(row->values.begin(), row->values.end(), target.begin());
    }
    timestamp_ = ts;
    return all_valid;
}

void CrossSectionSnapshot::reset(){
    // Refill values_ w/ NaNs, no shape changes.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    for (auto& row : values_) {std::fill(row.begin(), row.end(), nan);}
    timestamp_ = std::nullopt;
}

bool CrossSectionSnapshot::ready() const{
    return timestamp_.has_value();
}
int64_t CrossSectionSnapshot::timestamp() const{
    if (!ready()){
        throw std::runtime_error("No timestamp set yet");
    }
    return *timestamp_;
}
std::size_t CrossSectionSnapshot::num_symbols() const{return universe_.size();}

std::size_t CrossSectionSnapshot::num_features() const{return feature_names_.size();}

const std::vector<SymbolId>& CrossSectionSnapshot::symbols() const{return universe_;}

const std::vector<std::string>& CrossSectionSnapshot::feature_names() const{return feature_names_;}

CrossSectionResult CrossSectionSnapshot::compute(double eps) const{
    CrossSectionResult res;
    std::vector<std::pair<double, std::size_t>> valid;
    compute_into(res, valid, eps);
    return res;
}

void CrossSectionSnapshot::compute_into(
    CrossSectionResult& res,
    std::vector<std::pair<double, std::size_t>>& valid,
    double eps
) const{
    if (!ready()) {
        throw std::runtime_error("Cannot compute cross-section: snapshot not ready");
    }

    res.timestamp = *timestamp_;

    const std::size_t n_symbols = universe_.size();
    const std::size_t n_features = feature_names_.size();

    res.symbols = universe_;
    res.raw.resize(n_symbols);
    res.zscore.resize(n_symbols);
    res.rank_pct.resize(n_symbols);
    for (std::size_t i = 0; i < n_symbols; ++i) {
        res.raw[i].resize(n_features);
        std::copy(values_[i].begin(), values_[i].end(), res.raw[i].begin());
        res.zscore[i].assign(n_features, std::nullopt);
        res.rank_pct[i].assign(n_features, std::nullopt);
    }

    // Compute cross-sectional statistics per feature (column-wise across symbols).
    for (std::size_t j = 0; j < n_features; ++j) {
        valid.clear(); // (value, row index)
        valid.reserve(n_symbols);

        for (std::size_t i = 0; i < n_symbols; ++i) {
            const double v = values_[i][j];
            if (std::isfinite(v)) {
                valid.emplace_back(v, i);
            }
        }

        if (valid.empty()) {
            continue;
        }

        // z-score with population std (ddof=0).
        double mu = 0.0;
        for (const auto& p : valid) {
            mu += p.first;
        }
        mu /= static_cast<double>(valid.size());

        double var = 0.0;
        for (const auto& p : valid) {
            const double d = p.first - mu;
            var += d * d;
        }
        var /= static_cast<double>(valid.size());
        const double sigma = std::sqrt(std::max(0.0, var));

        if (sigma > eps) {
            for (const auto& p : valid) {
                res.zscore[p.second][j] = (p.first - mu) / sigma;
            }
        }

        // rank_pct with average tie policy (ascending): avg_rank / n_valid in (0, 1].
        std::sort(
            valid.begin(),
            valid.end(),
            [](const auto& a, const auto& b) {
                if (a.first != b.first) return a.first < b.first;
                return a.second < b.second;
            }
        );

        const double n_valid = static_cast<double>(valid.size());
        std::size_t s = 0;
        while (s < valid.size()) {
            std::size_t e = s + 1;
            while (e < valid.size() && valid[e].first == valid[s].first) {
                ++e;
            }

            // 1-based average rank for ties.
            const double min_rank = static_cast<double>(s + 1);
            const double max_rank = static_cast<double>(e);
            const double avg_rank = 0.5 * (min_rank + max_rank);
            const double pct = avg_rank / n_valid;

            for (std::size_t k = s; k < e; ++k) {
                res.rank_pct[valid[k].second][j] = pct;
            }
            s = e;
        }
    }
}
