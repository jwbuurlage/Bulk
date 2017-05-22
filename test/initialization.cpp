#include <bulk/bulk.hpp>
#include "bulk_test_common.hpp"
#include "set_backend.hpp"

extern environment env;

void test_initialization() {
    env.spawn(env.available_processors(), [](auto& world) {
        int s = world.processor_id();
        int p = world.active_processors();


        BULK_SECTION("Init") {
            BULK_CHECK_ONCE(s == world.processor_id(),
                            "correct processor id");
            BULK_CHECK_ONCE(p == world.active_processors(),
                            "correct number of processors");

            // we can create a variable
            bulk::var<int> x(world);
            x = 5;
            BULK_CHECK_ONCE(x.value() == 5, "write to variable");

            // we can create multiple variables in one step
            bulk::var<int> y(world);

            // we can reassign variables
            x = bulk::var<int>(world);
            BULK_CHECK_ONCE(x.value() == 0, "reassign variable");
        }
    });
}
