#pragma once

#define BULK_IN_ORDER(body)                  \
    for (int t = 0; t < bsp_nprocs(); ++t) { \
        if (bsp_pid() == t) {                \
            body                             \
        }                                    \
        bsp_sync();                          \
    }

#define BULK_ONCE(body)   \
    if (bsp_pid() == 0) { \
        body              \
    }

#define BULK_LOG_VAR_BODY(var) \
    std::cout << "$" << world.processor_id() << ": " << #var " = " << var << std::endl;

#define BULK_LOG_VAR(var) BULK_IN_ORDER(BULK_LOG_VAR_BODY(var))

#define BULK_LOG_VAR_ONCE(var) BULK_ONCE(BULK_LOG_VAR_BODY(var))
