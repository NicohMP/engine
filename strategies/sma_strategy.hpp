#pragma once
#include "../strategy.hpp"
#include <deque>
#include <optional>


class  SMAStrategy : public Strategy {
    private:
        // Parameters : asset ticker, windows sizes, target shares
        std::string ticker;
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
        SMAStrategy(size_t fast_window, size_t slow_window,double long_target,double short_target, std::string ticker);
        virtual std::optional<OrderEvent> on_market_event(const MarketEvent& mevent, const Portfolio& Portfolio) override;
 
};