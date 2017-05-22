#include <cassert>
#include <vector>

#include <experimental/optional>

namespace std {
template <typename T>
using optional = experimental::optional<T>;
}

namespace bulk {
namespace util {

template <typename Iterable>
auto average(Iterable& iter) {
    auto first = iter.begin();
    auto last = iter.end();
    auto k = 0u;
    auto a = *first;
    a = 0;
    for (; first != last; ++first) {
        a += *first;
        ++k;
    }
    return a / k;
}

/** Compute the 'zip' of two vectors. */
template <typename T, typename U>
std::vector<std::pair<T, U>> zip(std::vector<T> xs, std::vector<U> ys) {
    std::vector<std::pair<T, U>> result;
    for (size_t i = 0; i < (xs.size() < ys.size() ? xs.size() : ys.size());
         ++i) {
        result.push_back(std::make_pair(xs[i], ys[i]));
    }
    return result;
}

std::optional<std::pair<double, double>> fit(const std::vector<size_t>& xs,
                                             const std::vector<double>& ys) {
    if (xs.size() < 2 || ys.size() != xs.size()) {
        return std::optional<std::pair<double, double>>();
    }

    auto avg_x = std::accumulate(xs.begin(), xs.end(), 0u) / (double)xs.size();
    auto avg_y = std::accumulate(ys.begin(), ys.end(), 0.0) / (double)ys.size();
    auto points = zip(xs, ys);

    auto num = std::accumulate(
        points.begin(), points.end(), 0.0, [=](double a, auto p) {
            return a + (p.first - avg_x) * (p.second - avg_y);
        });

    auto denum =
        std::accumulate(xs.begin(), xs.end(), 0.0, [=](double a, double x) {
            return a + (x - avg_x) * (x - avg_x);
        });

    auto g = num / denum;
    auto l = avg_y - g * avg_x;

    return std::optional<std::pair<double, double>>({l, g});
}

} // namespace util
} // namespace bulk
