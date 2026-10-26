#include "signal/base.hpp"

class SMASignal : public SignalModel {
    private:
        // Parameters : asset id, windows sizes, target shares
        std::vector<SymbolId> universe;
        size_t slow_window;
        size_t fast_window;
        double long_target;
        double short_target;
};