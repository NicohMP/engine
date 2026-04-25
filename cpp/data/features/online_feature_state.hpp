#pragma once
#include "feature_types.hpp"
#include "rolling_moments.hpp"
#include "core/event_types.hpp"

class OnlineFeatureState {
    private:
        double last_close_ = 0.0;
        bool has_last_close_ = false;
        double last_ret_ = 0.0;
        bool has_last_ret_ = false;
        RollingMoments1D ret_moments_;
        RollingMoments1D price_moments_;
        SymbolId id_;

    public:
        OnlineFeatureState(const SymbolId& id, const size_t& window);
        bool push(const MarketEvent& mevent);
        bool ready() const;
        double ret_1d() const;
        double vol_pop() const;
        std::optional<double> price_z() const;
        FeatureRow to_feature_row_ret_1d() const;
};
