#include <bulk/backends/epiphany/host.hpp>
#include <bulk/environment.hpp>
#include <catch.hpp>

TEST_CASE("dma", "[bulk-dma]") {
    SECTION("everything") {
        bulk::environment<bulk::epiphany::provider> env;

        int success_count = 0;
        env.provider().set_log_callback(
            [&success_count](int pid, const std::string& log) {
                if (log.substr(0, 7) == "SUCCESS")
                    success_count++;
                else
                    std::cout << "Core " << pid << ": " << log << std::endl;
            });

        env.spawn(
            env.available_processors(),
            [](bulk::world<bulk::epiphany::backend>& world, int s, int p) {

// Assertion for inside an spmd section
// Currently its put inside here because then bulk-compile-tool will copy it to
// the kernel file
#define BULK_ASSERT(expr)                                                      \
    if (expr) {                                                                \
        world.log("SUCCESS");                                                  \
    } else {                                                                   \
        world.log("Assert failed on line %d: %s", __LINE__, #expr);            \
        world.abort();                                                         \
    }

                unsigned int count = 1000;
                auto xs = bulk::create_coarray<int>(world, count);
                auto ys = bulk::create_coarray<int>(world, count);

                bulk::epiphany::dma_task task_a;
                bulk::epiphany::dma_task task_b;

                for (auto& x : xs)
                    x = s;
                for (auto& y : ys)
                    y = -1;

                world.sync();

                int* dst = &ys(world.next_processor())[0];
                int* src = &xs[0];
                task_a.push(dst, src, count * sizeof(int), 0);

                world.barrier();

                // Immediately after the push, the last array elements
                // should not be transferred yet
                BULK_ASSERT(ys[count - 1] == -1);

                task_a.wait();

                world.sync();

                // Now it should be transferred
                BULK_ASSERT(ys[count - 1] == world.prev_processor());
            });

        CHECK(success_count == env.available_processors() * 2);
    }
}
