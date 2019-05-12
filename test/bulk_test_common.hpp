#pragma once

extern int total, success;

#define BULK_CHECK(body, error)                                                  \
    {                                                                            \
        auto result = (bool)(body);                                              \
        auto succes = bulk::var<bool>(world, result);                            \
        int all_good =                                                           \
        bulk::foldl(succes, [](auto& lhs, auto rhs) { lhs += rhs ? 1 : 0; }, 0); \
        if (world.rank() == 0) {                                                 \
            ++total;                                                             \
            if (all_good != world.active_processors()) {                         \
                world.log("FAILED %s (%i / %i)", error, all_good,                \
                          world.active_processors());                            \
            } else {                                                             \
                ++success;                                                       \
            }                                                                    \
        }                                                                        \
    }

#define BULK_CHECK_ONCE(body, error)                    \
    if (world.rank() == 0) {                            \
        ++total;                                        \
        if (!(body)) {                                  \
            world.log("  FAILED: *did not* %s", error); \
        } else {                                        \
            ++success;                                  \
        }                                               \
    }

#define BULK_SECTION(name)              \
    if (world.rank() == 0) {            \
        world.log("SECTION: %s", name); \
    }                                   \
    world.sync();

#define BULK_SKIP_SECTION_IF(name, body)                              \
    if ((body)) {                                                     \
        if (world.rank() == 0) {                                      \
            world.log("SECTION: %s SKIPPED BECAUSE %s", name, #body); \
        }                                                             \
        return;                                                       \
    }

#define BULK_REQUIRE(body)                              \
    if (world.rank() == 0) {                            \
        if (!(body)) {                                  \
            world.log("  ASSERTION FAILED: %s", #body); \
            world.abort();                              \
        }                                               \
    }

#define BULK_FINALIZE_TESTS(env)                                           \
    env.spawn(env.available_processors(), [](auto& world) {                \
        if (world.rank() == 0) {                                           \
            world.log("-------------");                                    \
            world.log("%d test(s) of %d failed.", total - success, total); \
        }                                                                  \
    })
