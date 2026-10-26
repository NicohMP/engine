#include "returns.hpp"
#include <stdexcept>
#include <string>

Returns::Returns(size_t window) : window_(window), name_("ret_" + std::to_string(window)) {
    if (window == 0) {
        throw std::runtime_error("Returns window must be > 0");
    }
}

std::optional<double> Returns::push(const MarketEvent& event) {
    if (ready()){
        double oldest = closes_.front();
        closes_.pop_front();
        closes_.push_back(event.close);
        last_value_ = event.close / oldest  -1.0;
        return last_value_;
    }
    closes_.push_back(event.close);
    return std::nullopt;
}

std::optional<double> Returns::value() const {
    return last_value_;
}

const std::string& Returns::name() const {
    return name_;
}

bool Returns::ready() const {
    return (closes_.size() == window_);
}
void Returns::reset() {
    closes_.clear();
    last_value_.reset();
}
        
