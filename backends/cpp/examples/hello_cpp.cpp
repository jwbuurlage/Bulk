#include <iostream>

#include <bulk/backends/cpp/cpp.hpp>
#include <bulk/bulk.hpp>

int main() {
    auto env = bulk::environment<bulk::cpp::provider>();

    env.spawn(env.available_processors(), [](auto world, int s, int p) {

            if (world.processor_id() != s) {
            std::cout << "ERROR ERROR ERROR!\n";
            }

        // hello world
        for (int t = 0; t < p; ++t) {
            if (s == t)
                std::cout << "Hello, world " << s << "/" << p << "\n";
            world.sync();
        }

        // writing to variables
        auto a = bulk::create_var<int>(world);
        auto b = bulk::create_var<int>(world);
        a(world.next_processor()) = s;
        b = 5 + s;
        world.sync();

        b(world.next_processor()) = 5;
        world.sync();

        std::cout << s << " <- " << a.value() << "\n";
        world.sync();
    });

    return 0;
}
