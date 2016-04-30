#include <bulk.hpp>

#include <iostream>

int main() {
    bulk::spawn(bulk::available_processors(), [](int s, int p) {
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    });

    return 0;
}
