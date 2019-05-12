#pragma once

#include <algorithm>
#include <functional>
#include <limits>
#include <numeric>
#include <sstream>

#include <bulk/bulk.hpp>

#include "vector.hpp"

namespace psc {

template <typename T>
bulk::coarray<T> sort(vector<T>& x) {
    auto& world = x.world();

    // helper function to find an equal sampling of a list of sorted numbers
    auto sample = [](const auto& vec, auto samples) {
        auto result = std::vector<T>(samples);
        auto step = vec.size() / samples;
        for (int i = 0; i < samples; ++i) {
            result[i] = vec[i * step];
        }
        return result;
    };

    // helper function to perform a merge sort with already sorted blocks
    auto blocked_merge = [&](auto& vec, auto block_sizes) {
        while (block_sizes.size() > 1) {
            int offset = 0;
            for (auto i = 0u; i < (block_sizes.size() / 2); ++i) {
                // we merge the blocks (2 * i) and (2 * i + 1)
                std::inplace_merge(vec.begin() + offset,
                                   vec.begin() + offset + block_sizes[2 * i],
                                   vec.begin() + offset + block_sizes[2 * i] +
                                   block_sizes[2 * i + 1]);
                offset += block_sizes[2 * i] + block_sizes[2 * i + 1];
                block_sizes[i] = block_sizes[2 * i] + block_sizes[2 * i + 1];
            }
            if (block_sizes.size() % 2 == 1) {
                block_sizes[((block_sizes.size() + 1) / 2) - 1] =
                block_sizes[block_sizes.size() - 1];
            }
            block_sizes.resize((block_sizes.size() + 1) / 2);
        }
    };

    auto s = world.rank();
    auto p = world.active_processors();

    // (1) sort local vector
    std::sort(x.begin(), x.end());

    // find the p samples
    auto local_samples = sample(x, p);

    // (2) communicate p samples
    auto samples = bulk::coarray<T>(world, p * p);

    for (int t = 0; t < p; ++t) {
        samples(t)[{s * p, (s + 1) * p}] = local_samples;
    }
    world.sync();

    blocked_merge(samples, std::vector<int>(p, p));

    // (3) finding the global samples
    auto splitters = sample(samples, p);
    splitters.push_back(std::numeric_limits<T>::max());

    // (4) identify local blocks
    int idx = 0;
    int previous = 0;
    std::vector<int> block_sizes(p);
    for (int t = 0; t < p; ++t) {
        while (idx < (int)x.size() && x[idx] < splitters[t + 1]) {
            ++idx;
        }
        block_sizes[t] = idx - previous;
        previous = idx;
    }

    // (5) communicate the blocks
    auto block_starts = std::vector<int>(p);
    std::partial_sum(block_sizes.begin(), block_sizes.end() - 1, block_starts.begin() + 1);

    auto q = bulk::queue<T[]>(world);
    for (int t = 0; t < p; ++t) {
        q(t).send(std::vector<T>(x.begin() + block_starts[t],
                                 x.begin() + block_starts[t] + block_sizes[t]));
    }
    world.sync();

    // (6) merge sort the received blocks
    auto receive_sizes = std::vector<int>(p);
    std::transform(q.begin(), q.end(), receive_sizes.begin(),
                   [](const auto& chunk) { return chunk.size(); });
    auto receive_starts = std::vector<int>(p);
    std::partial_sum(receive_sizes.begin(), receive_sizes.end() - 1,
                     receive_starts.begin() + 1);
    auto total = std::accumulate(receive_sizes.begin(), receive_sizes.end(), 0);

    auto ys = bulk::coarray<T>(world, total);

    auto t = 0u;
    for (const auto& chunk : q) {
        std::copy(chunk.begin(), chunk.end(), ys.begin() + receive_starts[t++]);
    }

    blocked_merge(ys, receive_sizes);

    return ys;
}

} // namespace psc
