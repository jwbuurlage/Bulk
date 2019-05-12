#pragma once

#include <cstring>
#include <memory>
#include <type_traits>

namespace bulk::detail {

struct scale {
    std::size_t size = 0;

    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    void operator|(const T&) {
        size += sizeof(T);
    }

    void operator|(const std::string& str) {
        size += (str.size() + 1) * sizeof(char);
    }

    template <typename T>
    void operator|(const std::vector<T>& xs) {
        size += sizeof(int) + xs.size() * sizeof(T);
    }
};

// non-owned buffer
struct memory_buffer_base {
    memory_buffer_base(void* const buffer_)
    : buffer((char*)buffer_), index(0) {}
    ~memory_buffer_base() {}

    char* const buffer;
    std::size_t index;

    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    void operator<<(const T& value) {
        memcpy(buffer + index, &value, sizeof(T));
        index += sizeof(T);
    }

    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    void operator>>(T& value) {
        memcpy(&value, buffer + index, sizeof(T));
        index += sizeof(T);
    }

    void operator<<(const std::string& str) {
        memcpy(&buffer[index], str.data(), str.length());
        index += str.length();
        buffer[index++] = '\0';
    }

    void operator>>(std::string& str) {
        str = std::string(buffer + index);
        index += (str.size() + 1) * sizeof(char);
    }

    template <typename T>
    void operator<<(const std::vector<T>& xs) {
        (*this) << (int)xs.size();
        memcpy(&buffer[index], &xs[0], xs.size() * sizeof(T));
        index += xs.size() * sizeof(T);
    }

    template <typename T>
    void operator>>(std::vector<T>& xs) {
        int size = 0;
        (*this) >> size;
        xs.resize(size);
        memcpy(xs.data(), buffer + index, sizeof(T) * size);
        index += size * sizeof(T);
    }
};

// memory owned by memory_buffer
struct memory_buffer : public memory_buffer_base {
    memory_buffer(std::size_t size) : memory_buffer_base(new char[size]) {}
    ~memory_buffer() { delete[] buffer; }

    memory_buffer(std::size_t size, char* data)
    : memory_buffer_base(new char[size]) {
        memcpy(buffer, data, size);
    }
};

struct omembuf {
    omembuf(memory_buffer_base& membuf_) : membuf(membuf_) {}

    template <typename T>
    void operator|(T& rhs) {
        membuf >> rhs;
    }

    memory_buffer_base& membuf;
};

struct imembuf {
    imembuf(memory_buffer_base& membuf_) : membuf(membuf_) {}

    template <typename T>
    void operator|(T& rhs) {
        membuf << rhs;
    }

    memory_buffer_base& membuf;
};

template <typename Buffer, typename U, typename... Us>
void fill(Buffer& buf, U& elem, Us&&... other) {
    buf | elem;
    fill(buf, std::forward<Us>(other)...);
}

template <typename Buffer>
void fill(Buffer&) {}

template <typename Buffer, typename Tuple, typename Is = std::make_index_sequence<std::tuple_size<Tuple>::value>>
void fill(Buffer& buf, Tuple& xs) {
    fill_tuple_(buf, xs, Is{});
}

template <typename Buffer, typename Tuple, size_t... Is>
void fill_tuple_(Buffer& buf, Tuple& xs, std::index_sequence<Is...>) {
    fill(buf, std::get<Is>(xs)...);
}

} // namespace bulk::detail
