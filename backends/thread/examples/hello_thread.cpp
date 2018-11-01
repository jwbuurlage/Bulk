#include <bulk/backends/thread/thread.hpp>
#include <bulk/bulk.hpp>

int main() {
    bulk::thread::environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.rank();
        int p = world.active_processors();

        // hello world
        for (int t = 0; t < p; ++t) {
            if (s == t)
                std::cout << "Hello, world " << s << "/" << p << "\n";
            world.sync();
        }

        // writing to variables
        bulk::var<int> a(world);
        bulk::var<int> b(world);
        a(world.next_rank()) = s;
        b = 5 + s;
        world.sync();

        b(world.next_rank()) = 5;
        world.sync();

        world.log("%d <- %d\n", s, a.value());
        world.sync();

        // coarrays
        bulk::coarray<int> xs(world, 5);
        xs(world.next_rank())[0] = s;
        xs(world.next_rank())[2] = s;
        xs[1] = s;
        world.sync();

        // Test if direct logging can produce different output orders
        // world.log_direct("%d <= %d %d %d\n", s, xs[0], xs[1], xs[2]);

#if 0
        // queues
        bulk::queue<int, int> q(world);
        q(world.next_processor()).send(1, 1);

        world.sync();

        // read queue
        for (auto& msg : q)
            world.log("%d got sent %d, %d\n", s, msg.tag, msg.content);

        world.sync();

        q(world.next_processor()).send(2, 3);
        q(world.next_processor()).send(123, 1337);

        bulk::queue<int, float> q2(world);
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
#endif
    });

    return 0;
}
