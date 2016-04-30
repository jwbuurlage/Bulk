#include <bulk.hpp>

int main() {
    bulk::spawn(bulk::available_processors(), [](int s, int p) {
        for (int t = 0; t < p; ++t) {
            bulk::send<int, int>(t, s, s);
        }

        bulk::sync();

        if (s == 0) {
            for (auto message : bulk::messages<int, int>()) {
                std::cout << message.tag << ", " << message.content << std::endl;
            }
        }
    });

    return 0;
}
