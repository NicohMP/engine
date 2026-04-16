#include "backtester.hpp"
#include "execution_engine.hpp"
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <variant>


Backtester::Backtester(
    std::string ticker,
    const std::vector<MarketEvent>& mevents,
    std::unique_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    double fee_rate
)
    : ticker_(std::move(ticker)),
      portfolio(std::move(init_portfolio)),
      strategy_(std::move(strategy)),
      execution_engine_(fee_rate),
      fee_rate_(fee_rate) {
    if (fee_rate_ < 0.0) {
        throw std::runtime_error("fee_rate must be non-negative");
    }

    for (const MarketEvent& mevent : mevents) { // filter only markets relevant to specified ticker
        if (mevent.ticker == ticker_) {
            mevents_.push_back(mevent);
        }
    }
    if (mevents_.empty()){ // check filtered market events isn't empty
        throw std::runtime_error("no data for ticker " + ticker_);
    }
    std::sort(mevents_.begin(), mevents_.end(), [](const MarketEvent& a, const MarketEvent& b){return a.timestamp < b.timestamp;});
    equity_.reserve(mevents_.size());
    
}


void Backtester::run() {
    for (const MarketEvent& mevent : mevents_) {
        event_queue_.clear();
        event_queue_.push_back(Event{mevent});

        while (!event_queue_.empty()) { // while there still are events left (new orders and fill might be created in the loop and will be handled in the next iterations)
            Event event = std::move(event_queue_.front()); // get next event 
            event_queue_.pop_front(); // pop it from the queue

            // For a MarketEvent (new market information)
            if (std::holds_alternative<MarketEvent>(event)) { 
                const MarketEvent& market_event = std::get<MarketEvent>(event);
                portfolio.update_prices(market_event); // update portfolio info with new market info

                std::optional<std::vector<OrderEvent>> maybe_orders = strategy_->on_market_event(market_event, portfolio); // apply strategy to possibly place new orders
                if (maybe_orders) {
                    for (OrderEvent order : *maybe_orders) {
                        if (order.id == 0) {
                            order.id = generate_order_id();
                        }
                        if (!order.quantity || *order.quantity <= 0.0) {
                            throw std::runtime_error("Invalid order quantity");
                        }
                        event_queue_.push_back(Event{order}); // if an order is passed, add to the queue
                    }
                }

                // Let queued orders get a chance to fill even when strategy emits no new order.
                const std::vector<FillEvent> fills = execution_engine_.fill_orders(market_event);
                for (const FillEvent& fill : fills) {
                    event_queue_.push_back(Event{fill});
                }
            } else if (std::holds_alternative<OrderEvent>(event)) {  // For an OrderEvent, pass it to the execution engine
                const OrderEvent& order = std::get<OrderEvent>(event);
                orders_.push_back(order);
                execution_engine_.pass_order(order);

                const std::vector<FillEvent> fills = execution_engine_.fill_orders(mevent);
                for (const FillEvent& fill : fills) {
                    event_queue_.push_back(Event{fill}); // if new order triggers a FillEvent, add it to queue
                }
            } else if (std::holds_alternative<FillEvent>(event)) { // For FillEvent, log it a update portfolio accordingly
                const FillEvent& fill = std::get<FillEvent>(event);
                fills_.push_back(fill);
                portfolio.update_position(fill);
            }
        }

        equity_.push_back({mevent.timestamp, portfolio.total_equity()});
    }
}
