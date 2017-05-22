#pragma once

extern int total, success;

#define BULK_CHECK_ONCE(body, error)                           \
    if (world.processor_id() == 0) {                           \
        ++total;                                               \
        if (!(body)) {                                         \
            world.log("  FAILED: *did not* %s", error);        \
        } else {                                               \
            ++success;                                         \
            world.log("  SUCCESS: %s", error);                 \
        }                                                      \
    }

#define BULK_SECTION(name)                        \
    if (world.processor_id() == 0) {              \
        world.log("SECTION: %s", name);           \
    }                                             \
    world.sync();

#define BULK_REQUIRE(body)           \
    if (world.processor_id() == 0) { \
        if (!(body)) {                                  \
            world.log("  ASSERTION FAILED: %s", #body); \
            world.abort();                              \
        }                                               \
    }

#define BULK_FINALIZE_TESTS(env)                                     \
    env.spawn(env.available_processors(), [](auto& world) { \
        if (world.processor_id() == 0) {                             \
            world.log("-------------");                              \
            world.log("%d test(s) of %d failed.", total - success, total);    \
        }                                                            \
    })

