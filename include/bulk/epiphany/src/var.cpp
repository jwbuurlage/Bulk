#pragma once
#include "var.hpp"
#include "world_provider.hpp"

namespace bulk {
namespace epiphany {

var::var() {
    var_id_ = ::world.provider().register_var_((void*)&value_);
    ::world.sync();
};

~var::var() { ::world.provider().unregister_var_(var_id_); }

// Get a remote copy of the variable
// If pid is the local pid, this will not be a `const` function
// However we want the compiler to optimize so we denote it as `const`
// so that `for (int i ...) { a(pid)[i]; }` might not call the ()
// operator every iteration
T& var::operator()(pid_t pid) const {
    return *((T*)::world.provider().get_direct_address_(pid, var_id_));
}

} // namespace epiphany
}
