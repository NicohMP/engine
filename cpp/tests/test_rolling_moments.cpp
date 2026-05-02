#include "data/features/rolling_moments.hpp"

#include <cassert>
#include <cmath>
#include <limits>

int main() {
    {
        RollingMoments1D rm(3);
        assert(!rm.ready());
        assert(rm.size() == 0);
        assert(rm.window() == 3);

        assert(rm.push(1.0));
        assert(rm.push(2.0));
        assert(!rm.ready());
        assert(rm.z_last() == std::nullopt);

        assert(rm.push(3.0));
        assert(rm.ready());
        assert(rm.size() == 3);
        assert(std::abs(rm.mean() - 2.0) < 1e-12);
        assert(std::abs(rm.var_pop() - (2.0 / 3.0)) < 1e-12);

        const auto z = rm.z_last();
        assert(z.has_value());
        assert(std::abs(*z - ((3.0 - 2.0) / std::sqrt(2.0 / 3.0))) < 1e-12);

        assert(rm.push(4.0)); // evicts 1.0
        assert(std::abs(rm.mean() - 3.0) < 1e-12);
        assert(std::abs(rm.var_pop() - (2.0 / 3.0)) < 1e-12);

        const size_t before = rm.size();
        assert(!rm.push(std::numeric_limits<double>::quiet_NaN()));
        assert(rm.size() == before);
    }

    {
        RollingMoments2D rm(3);
        assert(!rm.ready());
        assert(rm.push(1.0, 2.0));
        assert(rm.push(2.0, 4.0));
        assert(rm.push(3.0, 6.0));
        assert(rm.ready());

        assert(std::abs(rm.cov_pop() - (4.0 / 3.0)) < 1e-12);

        const auto corr = rm.corr();
        assert(corr.has_value());
        assert(std::abs(*corr - 1.0) < 1e-12);

        const auto beta = rm.beta_y_on_x();
        assert(beta.has_value());
        assert(std::abs(*beta - 2.0) < 1e-12);

        assert(rm.push(4.0, 8.0)); // evicts (1,2)
        assert(std::abs(rm.cov_pop() - (4.0 / 3.0)) < 1e-12);
    }

    {
        RollingMoments2D rm(2);
        assert(rm.push(1.0, 2.0));
        assert(rm.push(1.0, 3.0)); // var(x)==0
        assert(rm.corr() == std::nullopt);
        assert(rm.beta_y_on_x() == std::nullopt);
    }

    return 0;
}
