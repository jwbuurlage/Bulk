#include <numeric>
#include <vector>

#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.rank();
        int p = world.active_processors();

        // block distribution
        int size = 1000;
        int local_size = size / p;
        std::vector<int> xs(local_size);
        std::vector<int> ys(local_size);
        std::iota(xs.begin(), xs.end(), s * local_size);
        std::iota(ys.begin(), ys.end(), s * local_size);

        // compute local dot product
        bulk::var<int> result(world);
        for (int i = 0; i < local_size; ++i) {
            result.value() += xs[i] * ys[i];
        }

        world.sync();

        // reduce to find global dot product
        auto alpha = bulk::foldl(result, [](int& lhs, int rhs) { lhs += rhs; });

        world.log("%d/%d: alpha = %d", s, p, alpha);
    });

    return 0;
}
