#pragma once

#define BULK_IN_ORDER(body)                                                    \
    for (int t = 0; t < bsp_nprocs(); ++t) {                                   \
        if (bsp_pid() == t) {                                                  \
            body                                                               \
        }                                                                      \
        bsp_sync();                                                            \
    }

#define BULK_ONCE(body)                                                        \
    if (bsp_pid() == 0) {                                                      \
        body                                                                   \
    }

#define BULK_LOG_VAR(var)                                                      \
    BULK_IN_ORDER(std::cout << __FILE__ << ":" << __LINE__ << "(" << __func__  \
                            << "): " #var " = " << var << std::endl;)

#define BULK_LOG_VAR_ONCE(var)                                                 \
    BULK_ONCE(std::cout << __FILE__ << ":" << __LINE__ << "(" << __func__      \
                        << "): " #var " = " << var << std::endl;)
