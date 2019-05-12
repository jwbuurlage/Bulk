#pragma once

#include <cassert>
#include <optional>
#include <vector>

namespace bulk::util {

/** Find the average of an iterable container. */
template <typename Iterable>
double average(Iterable& iter) {
    auto first = iter.begin();
    auto last = iter.end();
    auto k = 0u;
    auto a = *first;
    a = 0;
    for (; first != last; ++first) {
        a += *first;
        ++k;
    }
    return a / (double)k;
}

/** Compute the 'zip' of two vectors. */
template <typename T, typename U>
std::vector<std::pair<T, U>> zip(std::vector<T> xs, std::vector<U> ys) {
    std::vector<std::pair<T, U>> result;
    for (size_t i = 0; i < (xs.size() < ys.size() ? xs.size() : ys.size()); ++i) {
        result.push_back(std::make_pair(xs[i], ys[i]));
    }
    return result;
}

/**
 * Find the estimator for the slope `g` given `xs` and `ys`, given a fixed
 * offset `l`.
 *
 * g = (sum x * (y - l)) / (sum x * x)
 */
inline std::optional<double>
fit_slope(const std::vector<size_t>& xs, const std::vector<double>& ys, float offset) {
    if (xs.size() < 2 || ys.size() != xs.size()) {
        return std::optional<double>();
    }

    auto points = zip(xs, ys);

    auto num = std::accumulate(points.begin(), points.end(), 0.0, [=](double a, auto p) {
        return a + p.first * (p.second - offset);
    });

    auto denum = std::accumulate(xs.begin(), xs.end(), 0.0,
                                 [=](double a, double x) { return a + x * x; });

    auto g = num / denum;

    return {g};
}

/**
 * Find the estimator for the slope `g` and offset `l` given `xs` and `ys`.
 *
 * g = (sum (x - x_avg) * (y - y_avg)) / (sum (x - x_avg) * (x - x_avg))
 * l = avg_y - g * avg_x
 *
 * The result is an optional pair given as (offset, slope).
 */
inline std::optional<std::pair<double, double>>
fit(const std::vector<size_t>& xs, const std::vector<double>& ys) {
    if (xs.size() < 2 || ys.size() != xs.size()) {
        return std::optional<std::pair<double, double>>();
    }

    auto avg_x = std::accumulate(xs.begin(), xs.end(), 0u) / (double)xs.size();
    auto avg_y = std::accumulate(ys.begin(), ys.end(), 0.0) / (double)ys.size();
    auto points = zip(xs, ys);

    auto num = std::accumulate(points.begin(), points.end(), 0.0, [=](double a, auto p) {
        return a + (p.first - avg_x) * (p.second - avg_y);
    });

    auto denum = std::accumulate(xs.begin(), xs.end(), 0.0, [=](double a, double x) {
        return a + (x - avg_x) * (x - avg_x);
    });

    auto g = num / denum;
    auto l = avg_y - g * avg_x;

    return std::optional<std::pair<double, double>>({l, g});
}

} // namespace bulk::util
