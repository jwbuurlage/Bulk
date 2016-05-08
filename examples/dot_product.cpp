#include <iostream>
#include <numeric>
#include <vector>

#include <bulk/bulk.hpp>
#include <bulk/variable.hpp>
#include <bulk/algorithm.hpp>
#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int) {
        // block distribution
        int size = 10;
        std::vector<int> xs(size);
        std::vector<int> ys(size);
        std::iota(xs.begin(), xs.end(), s * size);
        std::iota(ys.begin(), ys.end(), s * size);

        // compute local dot product
        auto result = bulk::create_var<int>(hub);
        for (int i = 0; i < size; ++i) {
            result.value() += xs[i] * ys[i];
        }

        hub.sync();

        // reduce to find global dot product
        auto alpha = bulk::reduce(result, [](int& lhs, int rhs) { lhs += rhs; });

        BULK_LOG_VAR(alpha);
    });

    return 0;
}
