#pragma once

#include <functional>
#include <iostream>

extern "C" {
#define MCBSP_COMPATIBILITY_MODE
#include <mcbsp.h>
}

#include <bulk/util/log.hpp>

namespace bulk_bsp {

template <typename T>
class var {
  public:
    var() {
        bsp_push_reg(&value(), sizeof(T));
        bsp_sync();
    };

    ~var() { bsp_pop_reg(&value()); }

    T& value() { return value_; }

  private:
    T value_;
};

template <typename T>
class future {
  public:
    future() {
        buffer_ = new T;
    }

    ~future() {
        if (buffer_ != nullptr)
            delete buffer_;
    };

    future(future<T>& other) = delete;
    future(future<T>&& other) {
        auto tmp_buffer = buffer_;
        buffer_ = other.buffer_;
        other.buffer_ = tmp_buffer;
    }

    T value() { return *buffer_; }

    T* buffer_;
};

class center {
  public:
    template <typename TFunc>
    void spawn(int processors, TFunc spmd) {
        auto spmd_no_args = [](void* f) {
            bsp_begin(bsp_nprocs());
            (*(TFunc*)f)(bsp_pid(), bsp_nprocs());
            bsp_end();
        };

        bsp_init_with_user_data(spmd_no_args, 0, nullptr, &spmd);
        spmd_no_args(&spmd);
    }

    int available_processors() { return bsp_nprocs(); }
    int active_processors() { return bsp_nprocs(); }
    int processor_id() { return bsp_pid(); }

    int next_processor() { return (bsp_pid() + 1) % bsp_nprocs(); }
    int prev_processor() {
        return (bsp_pid() + bsp_nprocs() - 1) % bsp_nprocs();
    }

    // Communication and synchronization
    // ---------------------------------

    template <typename T>
    void put(int processor, T value, var<T>& variable) {
        bsp_put(processor, &value, &variable.value(), 0, sizeof(T));
    }

    template <typename T>
    future<T> get(int processor, var<T>& variable) {
        future<T> result;
        bsp_get(processor, &variable.value(), 0, result.buffer_, sizeof(T));
        return result;
    }

    inline void sync() { bsp_sync(); }

    // Messages
    // --------

    //    template <typename TTag, typename TContent>
    //    void send(int processor, TTag tag, TContent content) {}
    //
    //    template <typename TTag, typename TContent>
    //    messages<TTag, TContent>::messages() {}
    //
    //    template <typename TTag, typename TContent>
    //    message_iterator<TTag, TContent> messages<TTag, TContent>::begin() {}
    //
    //    template <typename TTag, typename TContent>
    //    message_iterator<TTag, TContent> messages<TTag, TContent>::end() {}

    // Message iterator
    // --------
};

} // namespace bulk
