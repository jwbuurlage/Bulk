#pragma once

#include <memory>

#include "future.hpp"
#include "util/meta_helpers.hpp"
#include "util/serialize.hpp"
#include "world.hpp"

/**
 * \file variable.hpp
 *
 * This header defines a distributed variable, which has a value on each
 * processor.
 */

namespace bulk {

template <typename T>
class future;

class var_base {
  public:
    virtual void deserialize_put(size_t size, char* data) = 0;
    virtual size_t serialized_size() = 0;
    virtual void serialize(void* buffer) = 0;
    virtual std::pair<void*, size_t> location_and_size() = 0;
};

/**
 * Represents a distributed object with an image for each processor, that is
 * readable and writable from remote processors.
 */
template <typename T>
class var {
  public:
    using value_type = typename bulk::meta::representation<T>::type;

    class image {
      public:
        /**
         * Assign a value to a remote image
         *
         * \param value the new value of the image
         */
        var<T>& operator=(const value_type& value) {
            var_.impl_->put(t_, value);
            return var_;
        }

        /**
         * Get a future to the remote image value.
         */
        future<T> get() const { return var_.impl_->get(t_); }

      private:
        friend var;

        image(var<T>& v, int t) : var_(v), t_(t) {}

        var<T>& var_;
        int t_;
    };

    /**
     * Initialize and registers the variable with the world
     */
    var(bulk::world& world) {
        // TODO: Here we should ask world to create the appropriate
        // subclass of var_impl
        // var_impl constructor can include a barrier in certain backends
        impl_ = std::make_unique<var_impl>(world);
    }

    /**
     * Initialize and registers the variable with the world, and sets its value
     * to `value`.
     */
    var(bulk::world& world, value_type value) : var(world) { *this = value; }

    /**
     * Deconstructs and deregisters the variable with the world
     */
    ~var() {
        // It could be that some core is already unregistering while
        // another core is still reading from the variable. Therefore
        // use a barrier before var_impl unregisters

        // FIXME we really do not want this on distributed, for obvious
        // performance reasons
        // FIXME: what if the variable has moved, do we delay moving until next
        // superstep?

        if (impl_)
            impl_->world_.barrier();
    }

    // A variable can not be copied
    var(var<T>& other) = delete;
    void operator=(var<T>& other) = delete;

    /**
     * Move from one var to another
     */
    var(var<T>&& other) { impl_ = std::move(other.impl_); }

    /**
     * Move from one var to another
     */
    void operator=(var<T>&& other) { impl_ = std::move(other.impl_); }

    /**
     * Get an image object to a remote image, added for syntactic sugar
     *
     * \returns a `var::image` object to the image with index `t`.
     */
    image operator()(int t) { return image(*this, t); };

    /**
     * Broadcast a value to all images.
     */
    void broadcast(value_type x) {
        for (int t = 0; t < world().active_processors(); ++t) {
            impl_->put(t, x);
        }
    }

    /**
     * Implicitly get the value held by the local image of the var
     *
     * \note This is for code like `myint = myvar + 5;`.
     */
    operator value_type&() { return impl_->value_; }
    operator const value_type&() const { return impl_->value_; }

    /**
     * Write to the local image
     *
     * \note This is for code like `myvar = 5;`.
     */
    var<T>& operator=(const value_type& rhs) {
        impl_->value_ = rhs;
        return *this;
    }

    /**
     * Returns the value held by the local image of the var
     *
     * \returns a reference to the value held by the local image
     */
    value_type& value() { return impl_->value_; }

    /**
     * Retrieve the world to which this var is registed.
     *
     * \returns a reference to the world of the var
     */
    bulk::world& world() { return impl_->world_; }

  private:
    // Default implementation is a value, world and id.
    // Backends can subclass bulk::var<T>::var_impl to add more.
    // Backends can overload var_impl::put and var_impl::get.
    class var_impl : var_base {
      public:
        var_impl(bulk::world& world) : world_(world), value_{} {
            // register_location_ can include a barrier in certain backends
            id_ = world.register_variable_(this);
        }

        virtual ~var_impl() { world_.unregister_variable_(id_); }

        // No copies or moves
        var_impl(var_impl& other) = delete;
        var_impl(var_impl&& other) = delete;
        void operator=(var_impl& other) = delete;
        void operator=(var_impl&& other) = delete;

        virtual void put(int t, const value_type& source) {
            bulk::detail::scale ruler;
            bulk::detail::fill(ruler, source);
            auto size = ruler.size;
            auto target_buffer = world_.put_buffer_(t, id_, size);
            // use non-owning memory buffer
            auto tbuf = bulk::detail::memory_buffer_base(target_buffer);
            auto ibuf = bulk::detail::imembuf(tbuf);
            bulk::detail::fill(ibuf, source);
        }

        virtual future<T> get(int processor) const {
            future<T> result(world_);
            world_.get_buffer_(processor, id_, result);
            return result;
        }

        void deserialize_put(size_t, char* data) override {
            // use non-owning memory buffer
            auto membuf = bulk::detail::memory_buffer_base(data);
            auto obuf = bulk::detail::omembuf(membuf);
            bulk::detail::fill(obuf, value_);
        }

        size_t serialized_size() override final {
            bulk::detail::scale ruler;
            bulk::detail::fill(ruler, value_);
            return ruler.size;
        }

        void serialize(void* buffer) override final {
            auto membuf = bulk::detail::memory_buffer_base(buffer);
            auto ibuf = bulk::detail::imembuf(membuf);
            bulk::detail::fill(ibuf, value_);
        }

        std::pair<void*, size_t> location_and_size() override final {
            return {&value_, sizeof(value_type)};
        }

        bulk::world& world_;
        value_type value_;
        int id_;
    };
    std::unique_ptr<var_impl> impl_;

    friend class image;
};

} // namespace bulk
