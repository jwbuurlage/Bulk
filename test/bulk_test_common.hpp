#pragma once

#define BULK_CHECK_ONCE(body)                                                  \
    if (center.processor_id() == 0) {                                          \
        CHECK(body);                                                           \
    }
