#include <bulk/bulk.hpp>
#include "bulk_test_common.hpp"
#include "set_backend.hpp"

extern bulk::environment<provider> env;

void test_initialization() {
    env.spawn(env.available_processors(), [](auto world, int s, int p) {
        BULK_SECTION("Init") {
            BULK_CHECK_ONCE(s == world.processor_id(),
                            "correct processor id");
            BULK_CHECK_ONCE(p == world.active_processors(),
                            "correct number of processors");

            // we can create a variable
            auto x = bulk::create_var<int>(world);
            x = 5;
            BULK_CHECK_ONCE(x.value() == 5, "write to variable");

            // we can create multiple variables in one step
            auto y = bulk::create_var<int>(world);

            // we can reassign variables
            x = bulk::create_var<int>(world);
            BULK_CHECK_ONCE(x.value() == 0, "reassign variable");
        }
    });
}
