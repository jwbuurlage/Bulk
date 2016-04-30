#include <bulk.hpp>

int main() {
    bulk::spawn(bulk::available_processors(), [](int s, int p) {
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    });

    bulk::spawn(n, []() {});

    return 0;
}
