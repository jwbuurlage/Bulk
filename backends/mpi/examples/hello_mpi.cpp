#include <iostream>

#include <bulk/backends/mpi/mpi.hpp>
#include <bulk/bulk.hpp>

int main() {
    auto env = bulk::environment<bulk::mpi::provider>();

    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        // hello world
        for (int t = 0; t < p; ++t) {
            if (s == t)
                std::cout << "Hello, world " << s << "/" << p << " "
                          << world.provider().name() << "\n";
            world.sync();
        }

        // writing to variables
        auto a = bulk::create_var<int>(world);
        auto b = bulk::create_var<int>(world);
        bulk::put(world.next_processor(), s, a);
        b = 5 + s;
        world.sync();

        b(world.next_processor()) = 5;
        auto d = b(world.next_processor()).get();
        world.sync();

        std::cout << s << " <- " << a.value() << "\n";
        world.sync();

        // coarrays
        auto xs = bulk::create_coarray<int>(world, 5);
        xs(world.next_processor())[0] = s;
        xs(world.next_processor())[2] = s;

        world.sync();

        std::cout << s << " <= " << xs[0] << " " << xs[1] << " " << xs[2]
                  << "\n";

        // 'getting' values
        auto c = bulk::get(world.next_processor(), b);
        world.sync();

        std::cout << s << " <+ " << c.value() << "\n";

        // queues
        auto q = bulk::create_queue<int, int>(world);
        q(world.next_processor()).push(1, 1);

        world.sync();

        // read queue
        for (auto& msg : q) {
            std::cout << s << " got sent " << msg.tag << ", " << msg.content << "\n";
        }

        world.sync();

        q(world.next_processor()).push(2, 3);
        q(world.next_processor()).push(123, 1337);

        auto q2 = bulk::create_queue<int, float>(world);
        q2(world.next_processor()).push(5, 2.1f);
        q2(world.next_processor()).push(3, 4.0f);

        world.sync();

        // read queue
        for (auto& msg : q) {
            std::cout << s << " got sent in q " << msg.tag << ", " << msg.content << "\n";
        }

        for (auto& msg : q2) {
            std::cout << s << " got sent in q2 " << msg.tag << ", " << msg.content << "\n";
        }

    });

    return 0;
}
