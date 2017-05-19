#include <numeric>
#include <vector>

#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world, int s, int p) {
        // block distribution
        int size = 10;
        std::vector<int> xs(size);
        std::vector<int> ys(size);
        std::iota(xs.begin(), xs.end(), s * size);
        std::iota(ys.begin(), ys.end(), s * size);

        // compute local dot product
        bulk::var<int> result(world);
        for (int i = 0; i < size; ++i) {
            result.value() += xs[i] * ys[i];
        }

        world.sync();

        // reduce to find global dot product
        auto alpha = bulk::foldl(result, [](int& lhs, int rhs) { lhs += rhs; });

        world.log("%d/%d: alpha = %d", s, p, alpha);
    });

    return 0;
}
