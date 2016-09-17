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

        world.log("%d <- %d\n", s, a.value());
        world.sync();

        // coarrays
        auto xs = bulk::create_coarray<int>(world, 5);
        xs(world.next_processor())[0] = s;
        xs(world.next_processor())[2] = s;
        xs[1] = s;
        world.sync();

        // Test if direct logging can produce different output orders
        //world.log_direct("%d <= %d %d %d\n", s, xs[0], xs[1], xs[2]);

        // queues
        auto q = bulk::create_queue<int, int>(world);
        q(world.next_processor()).send(1, 1);

        world.sync();

        // read queue
        for (auto& msg : q)
            world.log("%d got sent %d, %d\n", s, msg.tag, msg.content);

        world.sync();

        q(world.next_processor()).send(2, 3);
        q(world.next_processor()).send(123, 1337);

        auto q2 = bulk::create_queue<int, float>(world);
        q2(world.next_processor()).send(5, 2.1f);
        q2(world.next_processor()).send(3, 4.0f);

        world.sync();

        // read queue
        for (auto& msg : q) {
            world.log("%d got sent in q %d, %d\n", s, msg.tag, msg.content);
        }
        for (auto& msg : q2) {
            world.log("%d got sent in q2 %d, %d\n", s, msg.tag, msg.content);
        }
    });

    return 0;
}
