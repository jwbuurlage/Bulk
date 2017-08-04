#pragma once

#include <type_traits>
#include <cstring>
#include <memory>

namespace bulk::detail {

struct scale {
    std::size_t size = 0;

    template <typename T>
    void operator|(T&) {
        size += sizeof(T);
    }

    void operator|(std::string& str) {
        size += (str.size() + 1) * sizeof(char);
    }

    template <typename T>
    void operator|(std::vector<T>& xs) {
        size += sizeof(int) + xs.size() * sizeof(T);
    }
};

struct memory_buffer {
    memory_buffer(std::size_t size) { buffer = std::make_unique<char[]>(size); }

    memory_buffer(std::size_t size, char* data) {
        buffer = std::make_unique<char[]>(size);
        // FIXME double buffer is kind of a waste
        memcpy(buffer.get(), data, size);
    }

    std::unique_ptr<char[]> buffer;
    std::size_t index = 0;

    template <typename T, typename = std::enable_if_t<std::is_pod<T>::value>>
    void operator<<(const T& value) {
        memcpy(buffer.get() + index, &value, sizeof(T));
        index += sizeof(T);
    }

    template <typename T>
    void operator>>(T& value) {
        memcpy(&value, buffer.get() + index, sizeof(T));
        index += sizeof(T);
    }

    void operator<<(const std::string& str) {
        for (auto c : str)
            (*this) << c;
        (*this) << '\0';
    }

    void operator>>(std::string& str) {
        str = std::string(buffer.get() + index);
        index += (str.size() + 1) * sizeof(char);
    }

    template <typename T>
    void operator<<(std::vector<T>& xs) {
        (*this) << (int)xs.size();
        for (auto x : xs) {
            (*this) << x;
        }
    }

    template <typename T>
    void operator>>(std::vector<T>& xs) {
        int size = 0;
        (*this) >> size;
        xs.resize(size);
        memcpy(xs.data(), buffer.get() + index, sizeof(T) * size);
        index += size * sizeof(T);
    }
};

struct omembuf {
    omembuf(memory_buffer& membuf_) : membuf(membuf_) {}

    template <typename T>
    void operator|(T& rhs) {
        membuf >> rhs;
    }

    memory_buffer& membuf;
};

struct imembuf {
    imembuf(memory_buffer& membuf_) : membuf(membuf_) {}

    template <typename T>
    void operator|(T& rhs) {
        membuf << rhs;
    }

    memory_buffer& membuf;
};

template <typename Buffer, typename U, typename... Us>
void fill(Buffer& buf, U& elem, Us&&... other) {
    buf | elem;
    fill(buf, std::forward<Us>(other)...);
}

template <typename Buffer>
void fill(Buffer&) {}

template <typename Buffer, typename Tuple,
          typename Is =
              std::make_index_sequence<std::tuple_size<Tuple>::value>>
void fill(Buffer& buf, Tuple& xs) {
    fill_tuple_(buf, xs, Is{});
}

template <typename Buffer, typename Tuple, size_t... Is>
void fill_tuple_(Buffer& buf, Tuple& xs, std::index_sequence<Is...>) {
    fill(buf, std::get<Is>(xs)...);
}

} // namespace bulk::detail
