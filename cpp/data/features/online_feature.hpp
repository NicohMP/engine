#pragma once

#include "core/event_types.hpp"
#include <string>
#include <optional>

class OnlineFeature {
public:
    virtual ~OnlineFeature() = default;

    virtual const std::string& name() const = 0;
    virtual std::optional<double> push(const MarketEvent& event) = 0;
    virtual std::optional<double> value() const = 0;
    virtual bool ready() const = 0;
    virtual void reset() = 0;
};
