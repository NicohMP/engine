#include "core/event_types.hpp"
#include "features/feature_types.hpp"
#include "portfolio/portfolio.hpp"
#include <vector>

// Strcture of Signal Model output : one asset, signal value and confidence
// The decision model should outout a vector of Signal``
struct Signal {
    SymbolId symbol_id;
    double value; //actual value of the signal
    double confidence; // optional, defaults to abs(value)
};


class SignalModel {
    public :
        virtual ~SignalModel() = default;

        virtual std::vector<Signal> update(
            const std::vector<MarketEvent>& events,
            const Portfolio& portfolio,
            const std::vector<FeatureView>& features
        ) = 0 ;

        virtual void reset() {}
};