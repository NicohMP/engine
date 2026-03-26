#pragma once
#include "../strategy.hpp"
#include <deque>


class  SMAStrategy : public Strategy {
    private:
        // Parameters : windows sizes and target shares
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
        SMAStrategy(size_t fast_window, size_t slow_window,double long_target,double short_target);

        virtual Action on_tick(
                const Tick& tick,
                const Position& position,
                const AccountState& account_state) override;
 
};