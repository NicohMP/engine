#pragma once
#include "base.hpp"
#include <deque>
#include <optional>
#include <vector>
#include "core/event_types.hpp"

class  SMAStrategy : public Strategy {
    private:
        // Parameters : asset id, windows sizes, target shares
        SymbolId symbol_id;
        size_t slow_window;
        size_t fast_window;
        double long_target;
        double short_target;

        // Rolling state
        std::deque<double> fast_buffer_;
        std::deque<double> slow_buffer_;
        double fast_sum_ = 0.0;
        double slow_sum_ = 0.0;

        // Signal state for crossover detection
        bool has_previous_relation_ = false;
        bool previous_fast_above_slow_ = false;

    public: 
        SMAStrategy(size_t fast_window, size_t slow_window,double long_target,double short_target, SymbolId symbol_id);
        virtual std::optional<std::vector<OrderEvent>> on_market_event(const MarketEvent& mevent, const Portfolio& Portfolio) override;
        virtual void reset() override {};
};
