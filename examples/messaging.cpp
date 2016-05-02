#include <iostream>

#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>

namespace bulk = bulk_bsp;


int main() {
    auto center = bulk::center();
    center.spawn(center.available_processors(), [&center](int s, int p) {
        for (int t = 0; t < p; ++t) {
            center.send<int, int>(t, s, s);
        }

        center.sync();

        if (s == 0) {
            for (auto message : center.messages<int, int>()) {
                std::cout << message.tag << ", " << message.content << std::endl;
            }
        }
    });

    return 0;
}
