#pragma once

namespace bulk {

template <typename T, class World>
class array {
  public:
    array(World& world, int size) : world_(world) {
        data_ = new T[size];
        world_.register_location_(data_, sizeof(T) * size);
    }

    ~array() {
        if (data_ != nullptr) {
            world_.unregister_location_(data_);
            delete[] data_;
        }
    }

    T* data() { return data_; }
    T& operator[](int i) { return data_[i]; }

    World& world() { return world_; }

  private:
    World& world_;
    T* data_;
};

template<typename T, typename World>
typename World::template array_type<T> create_array(World& world, int size) {
      return array<T, World>(world, size);
}

} // namespace bulk
