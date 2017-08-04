#pragma once

extern int total, success, cur_failed;

// The variable cur_failed is always set to zero at the end of every BULK_CHECK
// The barrier is at the end of BULK_CHECK to interfere as little as possible
// with the test case itself

#define BULK_CHECK(body, error)                                                \
    if (world.rank() == 0) {                                           \
        ++total;                                                               \
    }                                                                          \
    if (!(body)) {                                                             \
        cur_failed++;                                                          \
    }                                                                          \
    world.barrier();                                                           \
    if (world.rank() == 0) {                                           \
        if (cur_failed) {                                                      \
            world.log("  FAILED %d/%d processors *did not* %s", cur_failed,    \
                      world.active_processors(), error);                       \
        } else {                                                               \
            ++success;                                                         \
        }                                                                      \
        cur_failed = 0;                                                        \
    }                                                                          \
    world.barrier();

#define BULK_CHECK_ONCE(body, error)                                           \
    if (world.rank() == 0) {                                           \
        ++total;                                                               \
        if (!(body)) {                                                         \
            world.log("  FAILED: *did not* %s", error);                        \
        } else {                                                               \
            ++success;                                                         \
        }                                                                      \
    }

#define BULK_SECTION(name)                                                     \
    if (world.rank() == 0) {                                           \
        world.log("SECTION: %s", name);                                        \
    }                                                                          \
    world.sync();

#define BULK_SKIP_SECTION_IF(name, body)                                       \
    if ((body)) {                                                              \
        if (world.rank() == 0) {                                       \
            world.log("SECTION: %s SKIPPED BECAUSE %s", name, #body);          \
        }                                                                      \
        return;                                                                \
    }

#define BULK_REQUIRE(body)                                                     \
    if (world.rank() == 0) {                                           \
        if (!(body)) {                                                         \
            world.log("  ASSERTION FAILED: %s", #body);                        \
            world.abort();                                                     \
        }                                                                      \
    }

#define BULK_FINALIZE_TESTS(env)                                               \
    env.spawn(env.available_processors(), [](auto& world) {                    \
        if (world.rank() == 0) {                                       \
            world.log("-------------");                                        \
            world.log("%d test(s) of %d failed.", total - success, total);     \
        }                                                                      \
    })
