#include "sma_strategy.hpp"
#include <stdexcept>

SMAStrategy::SMAStrategy(size_t fw, size_t sw, double lt, double st, std::string t) : fast_window(fw), slow_window(sw), long_target(lt), short_target(st), ticker(t) {
    if (fast_window == 0){
        throw std::runtime_error("Window size can't be null");
    } else if (slow_window < fast_window) {
        throw std::runtime_error("Slow window can't be smaller than fast window");
    }
}

std::optional<OrderEvent> SMAStrategy::on_market_event(const MarketEvent& mevent, const Portfolio& Portfolio) {
    if (mevent.tick.ticker != ticker){return std::nullopt;}
    const Position& pos = Portfolio.get_position(ticker);
    const Tick& tick = mevent.tick;
    if (slow_buffer_.size() < slow_window) { // while not enough values, fill queues, updates sums and hold
        slow_sum_ += tick.close;
        slow_buffer_.push_back(tick.close);
        fast_sum_ += tick.close;
        fast_buffer_.push_back(tick.close);
        if (fast_buffer_.size() > fast_window) {
            fast_sum_ -= fast_buffer_.front();
            fast_buffer_.pop_front();
        }
        return std::nullopt;
    }
    
    // when there's enough values, we can safely update queues and sums
    slow_sum_ += tick.close - slow_buffer_.front();
    slow_buffer_.pop_front();
    slow_buffer_.push_back(tick.close);
    double slow_sma = slow_sum_ / slow_window; 

    fast_sum_ += tick.close - fast_buffer_.front();
    fast_buffer_.pop_front();
    fast_buffer_.push_back(tick.close);
    double fast_sma = fast_sum_ / fast_window;
    
    if (!has_previous_relation_) {
        previous_fast_above_slow_  = (fast_sma > slow_sma);
        has_previous_relation_ = true ;
    }


    const double current = pos.get_quantity();
    if ((!previous_fast_above_slow_) && (fast_sma > slow_sma)) {
        previous_fast_above_slow_ = true;
        const double desired = long_target;
        const double delta = desired - current;
        if (delta > 0) {
            std::uint64_t id = generate_order_id();
            OrderEvent order{id, ticker, TradeSide::Buy, delta, OrderType::Market};
            return order;
        }if (delta < 0) {
            std::uint64_t id = generate_order_id();
            OrderEvent order{id, ticker, TradeSide::Sell, -delta, OrderType::Market};
            return order;
        }
        return std::nullopt;
    } else if (previous_fast_above_slow_ && (fast_sma < slow_sma)) {
        previous_fast_above_slow_ = false;
        const double desired = short_target;
        const double delta = desired - current;
        if (delta > 0) {
            std::uint64_t id = generate_order_id();
            OrderEvent order{id, ticker, TradeSide::Buy, delta, OrderType::Market};
            return order;
        }
        if (delta < 0) {
            std::uint64_t id = generate_order_id();
            OrderEvent order{id, ticker, TradeSide::Sell, -delta, OrderType::Market};
            return order;
        }
        return std::nullopt;
    } else {
        return std::nullopt;
    }
}
