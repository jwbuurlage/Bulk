#pragma once

#include <iostream>

extern int total, success;

#define BULK_CHECK_ONCE(body, error)                           \
    if (world.processor_id() == 0) {                           \
        ++total;                                               \
        if (!(body)) {                                         \
            std::cout << "  FAILED: " << error << "\n";        \
        } else {                                               \
            ++success;                                         \
            std::cout << "  SUCCESS: *not* " << error << "\n"; \
        }                                                      \
    }

#define BULK_SECTION(name)                        \
    if (world.processor_id() == 0) {              \
        std::cout << "SECTION: " << name << "\n"; \
    }

#define BULK_REQUIRE(body)           \
    if (world.processor_id() == 0) { \
        assert(body);                \
    }

#define BULK_FINALIZE_TESTS(env)                                     \
    env.spawn(env.available_processors(), [](auto world, int, int) { \
        if (world.processor_id() == 0) {                             \
            std::cout << "-------------\n";                          \
            std::cout << total - success << " tests of " << total    \
                      << " failed.\n";                               \
        }                                                            \
    })

#define BULK_MESSAGE(body)           \
    if (world.processor_id() == 0) { \
        std::cout << body << "\n";   \
    }

using provider = bulk::mpi::provider;
