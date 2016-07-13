#pragma once

#include <functional>

extern "C" {
#define MCBSP_COMPATIBILITY_MODE
#include <mcbsp.h>
}

#include <bulk/world.hpp>
#include "backend.hpp"

namespace bulk {
namespace bsp {

class provider {
  public:
    using world_provider_type = world_provider;

    void spawn(int processors,
               std::function<void(bulk::world<backend>, int, int)> spmd) {
        struct spmd_parameters {
            std::function<void(bulk::world<backend>, int, int)>* f;
            int* processors;
        };

        auto spmd_no_args = [](void* parameters_ptr) {
            spmd_parameters* parameters = (spmd_parameters*)parameters_ptr;

            bsp_begin(*(parameters->processors));
            (*(parameters->f))(bulk::world<backend>(), bsp_pid(), bsp_nprocs());
            bsp_end();
        };

        spmd_parameters parameters = {&spmd, &processors};
        bsp_init_with_user_data(spmd_no_args, 0, nullptr, &parameters);
        spmd_no_args(&parameters);
    }

    int available_processors() const { return bsp_nprocs(); }
};

} // namespace bsp
} // namespace bulk
