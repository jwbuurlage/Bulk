#pragma once
#include <cstdint>
#include <functional>
#include <bulk/world.hpp>
#include <bulk/future.hpp>
#include <bulk/coarray.hpp>
#include <bulk/array.hpp>
#include <bulk/messages.hpp>

#include "epiphany_internals.hpp"

namespace bulk {
namespace epiphany {

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
        // int dummy = 0;
        // bsp_get_tag(&dummy, &current_message_.tag);
        // bsp_move(&current_message_.content, sizeof(TContent));
    }

    int i_;
    message<TTag, TContent> current_message_;
};

template <typename TTag, typename TContent>
class message_container {
  public:
    message_container() {
        int packets = 0;
        // int accum_bytes = 0;
        // bsp_qsize(&packets, &accum_bytes);

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



} // namespace epiphany
} // namespace bulk
