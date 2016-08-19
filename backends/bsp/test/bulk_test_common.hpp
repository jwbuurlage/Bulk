#pragma once

#define BULK_CHECK_ONCE(body)                                                  \
    if (world.processor_id() == 0) {                                             \
        CHECK(body);                                                           \
    }
