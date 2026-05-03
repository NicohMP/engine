#pragma once
#include "feature_types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct CrossSectionResult {
    int64_t timestamp = 0;
    std::vector<SymbolId> symbols; // deterministic order (sorted by symbol_id)

    // shape: [symbols.size()][num_features]
    std::vector<std::vector<double>> raw;
    std::vector<std::vector<std::optional<double>>> zscore;
    std::vector<std::vector<std::optional<double>>> rank_pct; // [0,1], average tie policy
};

class CrossSectionSnapshot {
private:
    std::vector<SymbolId> universe_;//
    std::unordered_map<SymbolId, std::size_t> id_to_row_;//
    std::vector<std::string> feature_names_;//
    std::optional<int64_t> timestamp_;//

    // shape fixed by universe x features; NaN means missing/invalid
    std::vector<std::vector<double>> values_;

public:
    CrossSectionSnapshot(
        std::vector<SymbolId> universe_ids,
        std::vector<std::string> feature_names
    );

    // Returns false only if any row has non-finite feature values.
    // Throws on schema mismatch / duplicate symbol / timestamp mismatch.
    bool push_snapshot(int64_t ts, const std::vector<FeatureRow>& rows);
    bool push_snapshot(int64_t ts, const std::vector<const FeatureRow*>& rows);

    bool ready() const;
    int64_t timestamp() const;
    std::size_t num_symbols() const;
    std::size_t num_features() const;
    const std::vector<SymbolId>& symbols() const;
    const std::vector<std::string>& feature_names() const;

    CrossSectionResult compute(double eps = 1e-12) const;
    void compute_into(
        CrossSectionResult& res,
        std::vector<std::pair<double, std::size_t>>& valid,
        double eps = 1e-12
    ) const;
    void reset();
};
