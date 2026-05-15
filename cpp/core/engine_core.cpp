#include "core/engine_core.hpp"
#include <stdexcept>

EngineCore::EngineCore(
    std::vector<SymbolId> universe,
    std::shared_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    FeatureConfig feature_config,
    std::shared_ptr<RiskManagerBase> risk_manager,
    double fee_rate
    ): 
    universe_(std::move(universe)),
    initial_portfolio(std::move(init_portfolio)),
    portfolio(initial_portfolio),
    risk_manager_(std::move(risk_manager)),
    strategy_(std::move(strategy)),
    feature_config_(std::move(feature_config)),
    feat_engine_(universe_, feature_config_.schema, feature_config_.window),
    execution_engine_(fee_rate),
    fee_rate_(fee_rate) 
    {
    if (fee_rate_ < 0.0) {
        throw std::runtime_error("fee_rate must be non-negative");
    }
    if (universe_.empty()) {
        throw std::runtime_error("universe must not be empty");
    }
}

EngineCore::EngineCore(
    std::vector<SymbolId> universe,
    std::shared_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    FeatureConfig feature_config,
    double fee_rate
) : EngineCore(
        std::move(universe),
        std::move(strategy),
        std::move(init_portfolio),
        std::move(feature_config),
        nullptr, // now that we use a shared_ptr, the None equivalent is nullptr
        fee_rate
    ) {}


void EngineCore::reset_runtime_logs(){
    orders_.clear();
    fills_.clear();
    equity_.clear();
}

void EngineCore::reset_step_state(){
    event_queue_.clear();
    execution_engine_.reset_pending_orders();
}

void EngineCore::reset_all(){
    reset_runtime_logs();
    reset_step_state();
    portfolio = initial_portfolio;
    strategy_->reset();
    if (risk_manager_){
        risk_manager_->reset();
    }
    feat_engine_.reset();
}


void EngineCore::process_market_decision(const MarketEvent& market_event, const FeatureView& view){
    auto maybe_orders = strategy_->on_market_event(market_event, portfolio,view); // apply strategy to possibly place new orders
    
    if (maybe_orders) {
        std::optional<std::vector<OrderEvent>> accepted_orders = maybe_orders;
        if (risk_manager_){
            accepted_orders = risk_manager_->accept_order_batch(*maybe_orders, portfolio);
        }
        if (accepted_orders){
            for (OrderEvent order : *accepted_orders) {
                if (order.id == 0) {
                    order.id = generate_order_id();
                }
                if (!order.quantity || *order.quantity <= 0.0) {
                    throw std::runtime_error("Invalid order quantity");
                }
                event_queue_.push_back(Event{order}); // if an order is passed, add to the queue
            }
        }
    }

    // Let queued orders get a chance to fill even when strategy emits no new order.
    const std::vector<FillEvent> fills = execution_engine_.fill_orders(market_event);
    for (const FillEvent& fill : fills) {
        event_queue_.push_back(Event{fill});
    }
}

void EngineCore::drain_event_queue(const MarketEvent& market_event){
    while (!event_queue_.empty()) {
        Event event = std::move(event_queue_.front());
        event_queue_.pop_front();

        if (std::holds_alternative<OrderEvent>(event)) {
            const OrderEvent& order = std::get<OrderEvent>(event);
            orders_.push_back(order);
            execution_engine_.pass_order(order);

            const std::vector<FillEvent> fills = execution_engine_.fill_orders(market_event);
            for (const FillEvent& fill : fills) {
                event_queue_.push_back(Event{fill});
            }
        } else if (std::holds_alternative<FillEvent>(event)) {
            const FillEvent& fill = std::get<FillEvent>(event);
            fills_.push_back(fill);
            portfolio.update_position(fill);
        } else if (std::holds_alternative<MarketEvent>(event)) {
            throw std::runtime_error("Unexpected MarketEvent while draining event queue");
        }
    }
}

void EngineCore::step(const MarketEvent& mevent){
    event_queue_.push_back(Event{mevent});
    while (!event_queue_.empty()) { // while there still are events left (new orders and fill might be created in the loop and will be handled in the next iterations)
        Event event = std::move(event_queue_.front()); // get next event 
        event_queue_.pop_front(); // pop it from the queue

        // For a MarketEvent (new market information)
        if (std::holds_alternative<MarketEvent>(event)) { 
            const MarketEvent& market_event = std::get<MarketEvent>(event);
            portfolio.update_prices(market_event); // update portfolio info with new market info
            
            // Update feature engine, retrieve latest feature rows and cross-asset features
            feat_engine_.push(market_event);
            const FeatureRow* row = feat_engine_.latest_row(market_event.symbol_id);
            const CrossSectionResult* xs = nullptr;
            if (feature_config_.enable_cross_section) {
                xs = feat_engine_.compute_cross_section_view(market_event.timestamp);
            }

            FeatureView view{
                market_event.timestamp,
                &feature_config_.schema,
                row,
                xs
            };

            process_market_decision(market_event, view);
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

void EngineCore::step_slice(const MarketEvent* begin, const MarketEvent* end){
    // If its empty, do nothing
    if (begin == end) {
        return;
    }
    
    // Declare the ts we're working with
    const int64_t timestamp = begin->timestamp;
    
    // Make sure it's the only timestamp in that slice we're given 
    for (const MarketEvent* it = begin; it != end; ++it) {
    if (it->timestamp != timestamp) {
        throw std::runtime_error("EngineCore::step_slice requires one timestamp");
        }
    }

    // Update all prices and features so portfolio adn feature engine are up to date
    for (const MarketEvent* it = begin; it != end; ++it) {
        if (it->timestamp != timestamp) {
            throw std::runtime_error("EngineCore::step_slice requires one timestamp");
        }
        portfolio.update_prices(*it);
        feat_engine_.push(*it);
    }

    // If cross-section mode enabled, compute cross section (that's the heavy code so we're making it optional)
    const CrossSectionResult* xs = nullptr;
    if (!feature_config_.enable_cross_section) {
        xs = feat_engine_.compute_cross_section_view(timestamp);
    }

    // Build vectors for events and feature views
    // Declare vectors
    std::vector<MarketEvent> events;
    std::vector<FeatureView> views;
    // Reserve memory since we know their sizes
    events.reserve(static_cast<size_t>(end - begin));
    views.reserve(static_cast<size_t>(end - begin));
    // For each mevent, add it to he events vector and add the corresponding feature row from crosssection to the features vector
    for (const MarketEvent* it = begin; it != end; ++it) {
        const MarketEvent& market_event = *it;
        const FeatureRow* row = feat_engine_.latest_row(market_event.symbol_id);

        events.push_back(market_event);
        views.push_back(FeatureView{
            market_event.timestamp,
            &feature_config_.schema,
            row,
            xs
        });
    }

    // strategy decisions based on time slice
    auto maybe_orders = strategy_->on_time_slice(events, portfolio, views);

    // Risk check produced orders batch
    if (maybe_orders) {
        std::optional<std::vector<OrderEvent>> accepted_orders = maybe_orders;

        if (risk_manager_) {
            accepted_orders = risk_manager_->accept_order_batch(*maybe_orders, portfolio);
        }

        if (accepted_orders) {
            for (OrderEvent order : *accepted_orders) {
                if (order.id == 0) {
                    order.id = generate_order_id();
                }
                if (!order.quantity || *order.quantity <= 0.0) {
                    throw std::runtime_error("Invalid order quantity");
                }
                event_queue_.push_back(Event{order});
            }
        }
    }

    // Since drain_queue is event-wise for now we drain event per event. to be updated later
    for (const MarketEvent* it = begin; it != end; ++it) {
        drain_event_queue(*it);
    }

    // Record equity after having processed the whole batch
    equity_.push_back({timestamp, portfolio.total_equity()});
}

void EngineCore::step_slice(const std::vector<MarketEvent>& events){
    if (events.empty()) {
        return;
    }
    step_slice(events.data(), events.data() + events.size());
}
