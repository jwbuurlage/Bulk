#include <bulk/bulk.hpp>
#include "set_backend.hpp"

void test_initialization() {
    environment env;
    env.spawn(env.available_processors(), [](auto& world) {
        #include "bulk_test_common.hpp"

        int s = world.rank();
        int p = world.active_processors();

        BULK_SECTION("Init") {
            BULK_CHECK(s == world.rank(),
                            "correct processor id");
            BULK_CHECK(p == world.active_processors(),
                            "correct number of processors");

            // we can create a variable
            bulk::var<int> x(world);
            x = 5;
            BULK_CHECK(x.value() == 5, "write to variable");

            // we can create multiple variables in one step
            bulk::var<int> y(world);

            // we can reassign variables
            x = bulk::var<int>(world);
            BULK_CHECK(x.value() == 0, "reassign variable");
        }

        BULK_FINALIZE_TESTS("Initialization");
    });
}
