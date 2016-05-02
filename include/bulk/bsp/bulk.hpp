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
    void operator=(future<T>& other) = delete;

    future(future<T>&& other) {
        *this = std::move(other);
    }

    void operator=(future<T>&& other) {
        auto tmp_buffer = buffer_;
        buffer_ = other.buffer_;
        other.buffer_ = tmp_buffer;       
    }

    T value() { return *buffer_; }

    T* buffer_;
};

class center {
  public:
    center() {
        tagsize_ = 0;
    }

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


    template <typename TTag, typename TContent>
    void send(int processor, TTag tag, TContent content) {
        if (tagsize_ != sizeof(TTag))  {
            int tagsize = sizeof(TTag);
            bsp_set_tagsize(&tagsize);
            sync();

            if (processor_id() == 0)
                tagsize_ = sizeof(TTag);
        }

        bsp_send(processor, &tag, &content, sizeof(TContent));
    }

    template <typename TTag, typename TContent>
    struct message {
        TTag tag;
        TContent content;
    };

    template <typename TTag, typename TContent>
    class message_iterator
        : std::iterator<std::forward_iterator_tag, message<TTag, TContent>> {
      public:
        message_iterator(int i, bool get_message) : i_(i) {
            if (get_message) {
                get_message_();
            }
        }

        message_iterator(const message_iterator& other)
            : i_(other.i_), current_message_(other.current_message_) {}

        message_iterator& operator++(int) {
            auto current = *this;
            ++(*this);
            return current;
        }

        bool operator==(const message_iterator& other) const {
            return i_ == other.i_;
        }

        bool operator!=(const message_iterator& other) const {
            return !(*this == other);
        }

        message<TTag, TContent> operator*() {
            return current_message_;
        }

        message_iterator& operator++() {
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

        message<TTag, TContent> current_message_;
        int i_;
    };

    template <typename TTag, typename TContent>
    class message_container {
      public:
        message_container() {
            int packets = 0;
            int accum_bytes = 0;
            bsp_qsize(&packets, &accum_bytes);

            queue_size_ = packets;
        }

        message_iterator<TTag, TContent> begin() {
            return message_iterator<TTag, TContent>(0, true);
        }

        message_iterator<TTag, TContent> end() {
            return message_iterator<TTag, TContent>(queue_size_, false);
        }

      private:
        int queue_size_;
    };

    template <typename TTag, typename TContent>
    message_container<TTag, TContent> messages() {
        return message_container<TTag, TContent>();
    }

  private:
    int tagsize_ = 0;
};

} // namespace bulk
