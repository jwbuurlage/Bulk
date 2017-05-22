#include <chrono>
#include <cstdio>
#include <vector>

#include <bulk/bulk.hpp>

#include "../examples/set_backend.hpp"
#include "fit.hpp"

using namespace std::chrono;

double flop_rate(bulk::world& world) {
    using clock = high_resolution_clock;

    unsigned int r_size = 1 << 23;
    std::vector<float> xs(r_size);
    std::iota(xs.begin(), xs.end(), 0);
    std::vector<float> ys = xs;
    std::vector<float> zs = xs;

    auto begin_time = clock::now();

    auto alpha = 1.0f / 3.0f;
    auto beta = 4.0f / 9.0f;
    for (auto i = 0u; i < r_size; ++i) {
        zs[i] = zs[i] + alpha * xs[i] - beta * ys[i];
    }

    auto end_time = clock::now();

    auto total_ms = duration<double, std::milli>(end_time - begin_time).count();
    auto flops = r_size * 4;

    auto flops_per_s = bulk::gather_all(world, 1000.0 * flops / total_ms);
    return bulk::average(flops_per_s.begin(), flops_per_s.end());
}

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.processor_id();
        int p = world.active_processors();

        using clock = high_resolution_clock;

        // about 4 MB
        unsigned int size = 1'000'000u;

        std::vector<int> dummy_data(size);
        std::iota(dummy_data.begin(), dummy_data.end(), 0);

        auto r = flop_rate(world);

        bulk::coarray<int> target(world, size);

        std::vector<size_t> test_sizes = {
            1'000u,   2'000u,   4'000u,    8'000u,   16'000u,
            32'000u,  64'000u,  100'000u,  128'000u, 200'000u,
            256'000u, 512'000u, 1'000'000u};
        std::vector<double> test_results;

        for (auto test_size : test_sizes) {
            double total = 0.0f;
            int sample_size = p > 8 ? 8 : p;
            for (auto k = 0; k < sample_size; ++k) {
                auto t = (world.next_processor() + p / sample_size) % p;

                auto begin_time = clock::now();

                target.put(t, dummy_data.begin(),
                           dummy_data.begin() + test_size);
                world.sync();

                auto end_time = clock::now();

                auto total_ms =
                    duration<double, std::milli>(end_time - begin_time).count();

                total += total_ms;
            }

            test_results.push_back(total / sample_size);
        }

        auto parameters = bulk::fit(test_sizes, test_results);
        if (parameters) {
            auto g = parameters.value().first * (r / 1000.0);
            auto l = parameters.value().second * (r / 1000.0);
            auto rs = bulk::gather_all(world, r / 1e9);
            auto gs = bulk::gather_all(world, g);
            auto ls = bulk::gather_all(world, l);

            if (s == 0) {
                std::vector<double> durations;
                for (int i = 0; i < 100; ++i) {
                    auto a = clock::now();
                    auto b = clock::now();
                    durations.push_back(
                        duration<double, std::milli>(b - a).count() *
                        (r / 1000.0));
                }

                auto avg_r = bulk::average(rs.begin(), rs.end());
                world.log("> p = %i\n> r = %f GLOPS\n> l = %f FLOPs\n> g = %f "
                          "FLOPs\n> total = %f GLOPS\n> dt = %f FLOPs",
                          p, avg_r, bulk::average(gs.begin(), gs.end()),
                          bulk::average(ls.begin(), ls.end()), avg_r * p,
                          bulk::average(durations.begin(), durations.end()));
            }
        }
    });

    return 0;
}
