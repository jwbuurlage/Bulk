#pragma once

#include <functional>
#include <iostream>

extern "C" {
#define MCBSP_COMPATIBILITY_MODE
#include <mcbsp.h>
}

#include <bulk/util/log.hpp>
#include <bulk/bulk.hpp>

namespace bulk {

template <typename T>
class bsp_array {
  public:
    bsp_array(int size) {
        data_ = new T[size];
        bsp_push_reg(data_, sizeof(T) * size);
        bsp_sync();
    };

    ~bsp_array() {
        if (data_ != nullptr) {
            bsp_pop_reg(data_);
            delete[] data_;
        }
    }

    T* data() { return data_; }

    T& operator[](int i) { return data_[i]; }

  private:
    T* data_;
};

template <typename T>
class bsp_var {
  public:
    bsp_var() {
        bsp_push_reg(&value(), sizeof(T));
        bsp_sync();
    };

    ~bsp_var() { bsp_pop_reg(&value()); }

    T& value() { return value_; }

  private:
    T value_;
};

template <typename T>
class bsp_future {
  public:
    bsp_future() { buffer_ = new T; }

    ~bsp_future() {
        if (buffer_ != nullptr)
            delete buffer_;
    };

    bsp_future(bsp_future<T>& other) = delete;
    void operator=(bsp_future<T>& other) = delete;

    bsp_future(bsp_future<T>&& other) { *this = std::move(other); }

    void operator=(bsp_future<T>&& other) {
        auto tmp_buffer = buffer_;
        buffer_ = other.buffer_;
        other.buffer_ = tmp_buffer;
    }

    T value() { return *buffer_; }

    T* buffer_;
};

template <typename TTag, typename TContent>
class bsp_message_iterator
    : std::iterator<std::forward_iterator_tag, message<TTag, TContent>> {
  public:
    bsp_message_iterator(int i, bool get_message) : i_(i) {
        if (get_message) {
            get_message_();
        }
    }

    bsp_message_iterator(const bsp_message_iterator& other)
        : i_(other.i_), current_message_(other.current_message_) {}

    bsp_message_iterator& operator++(int) {
        auto current = *this;
        ++(*this);
        return current;
    }

    bool operator==(const bsp_message_iterator& other) const {
        return i_ == other.i_;
    }

    bool operator!=(const bsp_message_iterator& other) const {
        return !(*this == other);
    }

    message<TTag, TContent> operator*() { return current_message_; }

    bsp_message_iterator& operator++() {
        ++i_;
        get_message_();
        return *this;
    }

  private:
    void get_message_() {
        int dummy = 0;
        bsp_get_tag(&dummy, &current_message_.tag);
        bsp_move(&current_message_.content, sizeof(TContent));
    }

    int i_;
    message<TTag, TContent> current_message_;
};

template <typename TTag, typename TContent>
class bsp_message_container {
  public:
    bsp_message_container() {
        int packets = 0;
        int accum_bytes = 0;
        bsp_qsize(&packets, &accum_bytes);

        queue_size_ = packets;
    }

    bsp_message_iterator<TTag, TContent> begin() {
        return bsp_message_iterator<TTag, TContent>(0, true);
    }

    bsp_message_iterator<TTag, TContent> end() {
        return bsp_message_iterator<TTag, TContent>(queue_size_, false);
    }

  private:
    int queue_size_;
};

class bsp_hub
    : public base_hub<bsp_var, bsp_array, bsp_future, bsp_message_container> {
  public:
    bsp_hub() { tag_size_ = 0; }

    void spawn(int processors, std::function<void(int, int)> spmd) override {
        struct spmd_parameters {
            std::function<void(int, int)>* f;
            int* processors;
        };

        auto spmd_no_args = [](void* parameters_ptr) {
            spmd_parameters* parameters = (spmd_parameters*)parameters_ptr;

            bsp_begin(*(parameters->processors));
            (*(parameters->f))(bsp_pid(), bsp_nprocs());
            bsp_end();
        };

        spmd_parameters parameters = {&spmd, &processors};
        bsp_init_with_user_data(spmd_no_args, 0, nullptr, &parameters);
        spmd_no_args(&parameters);
    }

    int available_processors() override { return bsp_nprocs(); }
    int active_processors() override { return bsp_nprocs(); }
    int processor_id() override { return bsp_pid(); }

    int next_processor() override { return (bsp_pid() + 1) % bsp_nprocs(); }
    int prev_processor() override {
        return (bsp_pid() + bsp_nprocs() - 1) % bsp_nprocs();
    }

    void sync() override { bsp_sync(); }

  private:
    size_t tag_size_ = 0;

    void internal_put_(int processor, void* value, void* variable, size_t size,
                       int offset, int count) override {
        bsp_put(processor, value, variable, offset * size, count * size);
    }

    void internal_get_(int processor, void* variable, void* target, size_t size,
                       int offset, int count) override {
        bsp_get(processor, variable, offset * size, target, count * size);
    }

    virtual void internal_send_(int processor, void* tag, void* content,
                                size_t tag_size, size_t content_size) {
        if (tag_size_ != tag_size) {
            int tag_size_copy = tag_size;
            bsp_set_tagsize(&tag_size_copy);
            sync();

            if (processor_id() == 0)
                tag_size_ = tag_size;
        }

        bsp_send(processor, tag, content, content_size);
    }
};

} // namespace bulk
