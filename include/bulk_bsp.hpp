#pragma once

#include <functional>
#include <iostream>

extern "C" {
#define MCBSP_COMPATIBILITY_MODE
#include <mcbsp.h>
}

#include <bulk_util.hpp>

namespace bulk {

// Initialization and information
// ------------------------------

std::function<void(int, int)> g_spmd;

void spawn(int processors, std::function<void(int, int)> spmd) {
    g_spmd = spmd;

    auto spmd_no_args = []() {
        bsp_begin(bsp_nprocs());
        g_spmd(bsp_pid(), bsp_nprocs());
        bsp_end();
    };

    bsp_init(spmd_no_args, 0, nullptr);
    spmd_no_args();
}

int available_processors() { return bsp_nprocs(); }
int active_processors() { return bsp_nprocs(); }
int processor_id() { return bsp_pid(); }

int next_processor() { return (bsp_pid() + 1) % bsp_nprocs(); }
int prev_processor() { return (bsp_pid() + bsp_nprocs() - 1) % bsp_nprocs(); }


// Communication and synchronization
// ---------------------------------

template <typename T>
var<T>::var() { 
    bsp_push_reg(&value(), sizeof(T));
    bsp_sync();
}

template <typename T>
var<T>::~var() {
    bsp_pop_reg(&value());
}

template <typename T>
T& var<T>::value() { return value_; }

template <typename T>
void put(int processor, T value, var<T>& variable) {
    bsp_put(processor, &value, &variable.value(), 0, sizeof(T));
}

template <typename T>
future<T>::future(T* buffer) { 
    buffer_ = buffer;
}

template <typename T>
future<T>::~future() {
    if (buffer_ != nullptr)
        delete buffer_;
}

template <typename T>
future<T>::future(future<T>&& other) {
    auto tmp_buffer = buffer_;
    buffer_ = other.buffer_;
    other.buffer_ = tmp_buffer;
}

template <typename T>
future<T>&& get(int processor, var<T>& variable) {
    future<T> result(new T);
    bsp_get(processor, &variable.value(), 0, result.buffer_, sizeof(T));
    return std::move(result);
}

template <typename T>
T future<T>::value() { return *buffer_; }

inline void sync() { bsp_sync(); }

// Messages
// --------

template <typename TTag, typename TContent>
void send(int processor, TTag tag, TContent content) { }

template <typename TTag, typename TContent>
messages<TTag, TContent>::messages() { }

template <typename TTag, typename TContent>
message_iterator<TTag, TContent> messages<TTag, TContent>::begin() { }

template <typename TTag, typename TContent>
message_iterator<TTag, TContent> messages<TTag, TContent>::end() { }

// Message iterator
// --------
 
} // namespace bulk
