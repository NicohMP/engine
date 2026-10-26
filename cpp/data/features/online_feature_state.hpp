#pragma once
#include "feature_types.hpp"
#include "rolling_moments.hpp"
#include "core/event_types.hpp"
#include "feature_schema.hpp"

class OnlineFeatureState {
    private:
        SymbolId id_;
        // Warmup close price buffer
        double last_close_ = 0.0;
        bool has_last_close_ = false;
        // Warmup returns buffer
        double last_ret_ = 0.0;
        bool has_last_ret_ = false;
        // Warmup log return 
        double last_logret_ = 0.0;
        bool has_last_logret_ = false;
        //Warmup high/low prices
        double last_high_low_range_ = 0.0;
        bool has_last_high_low_range_ = false;
        // Buffers
        RollingMoments1D ret_moments_;
        RollingMoments1D price_moments_;
        RollingMoments1D volume_moments_;

        // Feature compute methods
        double compute_feature(FeatureId id) const;
        double feature_logret_1d() const;
        double feature_ret_1d() const;
        double feature_ret_var() const;
        double feature_ret_vol() const;
        double feature_price_z() const;
        double feature_volume_z() const;
        double feature_high_low_range() const;
        double feature_close_to_sma() const;

    public:
        OnlineFeatureState(const SymbolId& id, const size_t& window);
        
        bool push(const MarketEvent& mevent);
        bool ready() const;
        void reset();
        
        SymbolId id() const;
        
        FeatureRow to_feature_row(const FeatureSchema& schema) const;
};

