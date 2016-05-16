#pragma once

#include <functional>
#include <iostream>

extern "C" {
#define MCBSP_COMPATIBILITY_MODE
#include <mcbsp.h>
}

#include <bulk/util/log.hpp>
#include <bulk/world.hpp>
#include <bulk/variable.hpp>
#include <bulk/future.hpp>
#include <bulk/coarray.hpp>
#include <bulk/array.hpp>


namespace bulk {
namespace bsp {

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

    message<TTag, TContent> operator*() { return current_message_; }

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

    int i_;
    message<TTag, TContent> current_message_;
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

class world_provider {
  public:
    template <typename TTag, typename TContent>
    using message_container_type = message_container<TTag, TContent>;

    template <typename T>
    using var_type = bulk::var<T, bulk::world<world_provider>>;

    template <typename T>
    using future_type = bulk::future<T, bulk::world<world_provider>>;

    template <typename T>
    using coarray_type = bulk::coarray<T, bulk::world<world_provider>>;

    template <typename T>
    using array_type = bulk::array<T, bulk::world<world_provider>>;

    world_provider() { tag_size_ = 0; }

    int active_processors() const { return bsp_nprocs(); }
    int processor_id() const { return bsp_pid(); }

    void sync() const { bsp_sync(); }

    void internal_put_(int processor, void* value, void* variable, size_t size,
                       int offset, int count) {
        bsp_put(processor, value, variable, offset * size, count * size);
    }

    void register_location_(void* location, size_t size) {
        bsp_push_reg(location, size);
        sync();
    }

    void unregister_location_(void* location) {
        bsp_pop_reg(location);
    }

    void internal_get_(int processor, void* variable, void* target, size_t size,
                       int offset, int count) {
        bsp_get(processor, variable, offset * size, target, count * size);
    }

    virtual void internal_send_(int processor, void* tag, void* content,
                                size_t tag_size, size_t content_size) {
        if (tag_size_ != tag_size) {
            int tag_size_copy = tag_size;
            bsp_set_tagsize(&tag_size_copy);
            sync();

            tag_size_ = tag_size;
        }

        bsp_send(processor, tag, content, content_size);
    }

  private:
    size_t tag_size_ = 0;
};

class provider {
  public:
    using world_provider_type = world_provider;

    void spawn(int processors,
               std::function<void(bulk::world<world_provider>, int, int)> spmd) {
        struct spmd_parameters {
            std::function<void(bulk::world<world_provider>, int, int)>* f;
            int* processors;
        };

        auto spmd_no_args = [](void* parameters_ptr) {
            spmd_parameters* parameters = (spmd_parameters*)parameters_ptr;

            bsp_begin(*(parameters->processors));
            (*(parameters->f))(bulk::world<world_provider>(), bsp_pid(), bsp_nprocs());
            bsp_end();
        };

        spmd_parameters parameters = {&spmd, &processors};
        bsp_init_with_user_data(spmd_no_args, 0, nullptr, &parameters);
        spmd_no_args(&parameters);
    }

    int available_processors() const { return bsp_nprocs(); }
};

} // namespace bsp
} // namespace bulk
