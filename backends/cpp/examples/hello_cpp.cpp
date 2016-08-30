#include <iostream>

#include <bulk/backends/cpp/cpp.hpp>
#include <bulk/bulk.hpp>

int main() {
    auto env = bulk::environment<bulk::cpp::provider>();

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
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

        // coarrays
        auto xs = bulk::create_coarray<int>(world, 5);
        xs(world.next_processor())[0] = s;
        xs(world.next_processor())[2] = s;
        xs[1] = s;
        world.sync();

        for (int t = 0; t < p; ++t) {
            if (s == t)
                std::cout << s << " <= " << xs[0] << " " << xs[1] << " "
                          << xs[2] << "\n";
            world.sync();
        }
    });

    return 0;
}
