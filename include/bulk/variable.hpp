#pragma once

namespace bulk {

template <typename T, class World>
class future;

/// Represents a distributed object with an image for each processor, that is
/// readable and writable from remote processors.
template <typename T, class World>
class var {
  public:
    /// Initialize and registers the variable with the world
    var(World& world) : world_(world) {
        world_.register_location_(&value_, sizeof(T));
    }

    /// Deconstructs and deregisters the variable with the world
    ~var() { world_.unregister_location_(&value_); }

    var(var<T, World>& other) = delete;
    void operator=(var<T, World>& other) = delete;

    /// Move from one var to another
    var(var<T, World>&& other) : world_(other.world_) {
        *this = std::move(other);
    }

    /// Move from one var to another
    void operator=(var<T, World>&& other) {
        world_.register_location_(&value_, sizeof(T));
        value_ = other.value();
    }

    /// Returns the value held by the local image of the var
    T& value() { return value_; }

    /// Returns the world to which this variable belongs
    World& world() { return world_; }

  private:
    T value_;
    World& world_;
};

/// Create a variable and register it with a world
///
/// \note this function is included so that the programmer does not explicitely
/// has to pass the type of the world
template<typename T, typename World>
typename World::template var_type<T> create_var(World& world) {
      return var<T, World>(world);
}

} // namespace bulk
