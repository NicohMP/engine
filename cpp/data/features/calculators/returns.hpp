#pragma once

#include "online_feature.hpp"
#include <deque>
#include <string>
#include <optional>


class Returns : public OnlineFeature {
        std::string name_;
        std::deque<double> closes_;
        std::optional<double> last_value_;
        size_t window_;

    public:
        Returns(size_t window);
        virtual const std::string& name() const override;
        virtual std::optional<double> push(const MarketEvent& event) override;
        virtual std::optional<double> value() const override;
        virtual bool ready() const override;
        virtual void reset() override;
        
};
