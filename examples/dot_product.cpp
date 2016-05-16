#include <iostream>
#include <numeric>
#include <vector>

#include <bulk/bulk.hpp>
#include <bulk/bsp/provider.hpp>
#include <bulk/util/log.hpp>


int main() {
    auto env = bulk::environment<bulk::bsp::provider>();

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        // block distribution
        int size = 10;
        std::vector<int> xs(size);
        std::vector<int> ys(size);
        std::iota(xs.begin(), xs.end(), s * size);
        std::iota(ys.begin(), ys.end(), s * size);

        // compute local dot product
        auto result = bulk::create_var<int>(world);
        for (int i = 0; i < size; ++i) {
            result.value() += xs[i] * ys[i];
        }

        world.sync();

        // reduce to find global dot product
        auto alpha = bulk::reduce(result, [](int& lhs, int rhs) { lhs += rhs; });

        BULK_LOG_VAR(alpha);
    });

    return 0;
}
