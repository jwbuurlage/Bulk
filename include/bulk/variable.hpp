#pragma once

namespace bulk {

template <typename T, class Hub>
class future;

template <typename T, class Hub>
class var {
  public:
    var(Hub& hub) : hub_(hub) { hub_.register_location_(&value_, sizeof(T)); }
    ~var() { hub_.unregister_location_(&value_); }

    var(var<T, Hub>& other) = delete;
    void operator=(var<T, Hub>& other) = delete;

    var(var<T, Hub>&& other) : hub_(other.hub_) {
        *this = std::move(other);
    }
    void operator=(var<T, Hub>&& other) {
        hub_.register_location_(&value_, sizeof(T));
        value_ = other.value();
    }

    T& value() { return value_; }

    Hub& hub() { return hub_; }

  private:
    T value_;
    Hub& hub_;
};

template<typename T, typename Hub>
var<T, Hub> create_var(Hub& hub) {
      return var<T, Hub>(hub);
}

} // namespace bulk
