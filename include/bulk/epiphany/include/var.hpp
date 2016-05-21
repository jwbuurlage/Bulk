#pragma once
#include "epiphany_internals.hpp"

namespace bulk {
namespace epiphany {

template <typename T>
class var {
  public:
    var();
    ~var();

    // Disallow copy constructor and copy assignment
    var(const var<T>& other) = delete;
    void operator=(const var<T>& other) = delete;

    // Explicitly get the local copy of the variable
    T& value() { return value_; }

    // Cast to type T to get the local copy of the variable
    operator T&() { return value_; }
    operator const T&() const { return value_; }

    // Write to the local copy using the assignment operator
    var<T>& operator=(const T& rhs) {
        value_ = rhs;
        return *this;
    }

    // Get a remote copy of the variable
    // If pid is the local pid, this will not be a `const` function
    // However we want the compiler to optimize so we denote it as `const`
    // so that `for (int i ...) { a(pid)[i]; }` might not call the ()
    // operator every iteration
    T& operator()(pid_t pid) const;

  protected:
    T value_;
    var_id_t var_id_;
};

} // namespace epiphany
} // namespace bulk
