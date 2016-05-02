#pragma once

#define BULK_CHECK_ONCE(body)                                                  \
    if (hub.processor_id() == 0) {                                          \
        CHECK(body);                                                           \
    }
