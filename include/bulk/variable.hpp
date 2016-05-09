#pragma once

namespace bulk {

template <typename T, class Hub>
class future;

/// Represents a distributed object with an image for each processor, that is
/// readable and writable from remote processors.
template <typename T, class Hub>
class var {
  public:
    /// Initialize and registers the variable with the hub
    var(Hub& hub) : hub_(hub) { hub_.register_location_(&value_, sizeof(T)); }

    /// Deconstructs and deregisters the variable with the hub
    ~var() { hub_.unregister_location_(&value_); }

    var(var<T, Hub>& other) = delete;
    void operator=(var<T, Hub>& other) = delete;

    /// Move from one var to another
    var(var<T, Hub>&& other) : hub_(other.hub_) {
        *this = std::move(other);
    }

    /// Move from one var to another
    void operator=(var<T, Hub>&& other) {
        hub_.register_location_(&value_, sizeof(T));
        value_ = other.value();
    }

    /// Returns the value held by the local image of the var
    T& value() { return value_; }

    /// Returns the hub to which this variable belongs
    Hub& hub() { return hub_; }

  private:
    T value_;
    Hub& hub_;
};

/// Create a variable and register it with a hub
///
/// \note this function is included so that the programmer does not explicitely
/// has to pass the type of the hub
template<typename T, typename Hub>
var<T, Hub> create_var(Hub& hub) {
      return var<T, Hub>(hub);
}

} // namespace bulk
