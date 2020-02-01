//
// Created by Ilya on 06.09.2019.
//

#ifndef IP_INTRUSIVE_PTR_HPP
#define IP_INTRUSIVE_PTR_HPP

#include "ref_counter.hpp"

#include <stdexcept>

// Check if RTTI is enabled
#if defined(__clang__)
    #if __has_feature(cxx_rtti)
        #define RTTI_ENABLED
    #endif
#elif defined(__GNUG__)
    #if defined(__GXX_RTTI)
        #define RTTI_ENABLED
    #endif
#elif defined(_MSC_VER)
    #if defined(_CPPRTTI)
        #define RTTI_ENABLED
    #endif
#endif

namespace ip {

template<typename T>
class intrusive_ptr {
    //static_assert(std::is_base_of_v<ref_counter, T>, "T is not derived from ref_counter");
public:
    intrusive_ptr() : ptr(nullptr) {}

    intrusive_ptr(T *ptr) : ptr(ptr) {
        if (ptr) {
            ptr->add_ref();
        }
    }

    intrusive_ptr(const intrusive_ptr<T> &other) : ptr(other.ptr) {
        if (ptr) {
            ptr->add_ref();
        }
    }

    intrusive_ptr(intrusive_ptr<T> &&other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    T *get() const {
        return ptr;
    }

    [[nodiscard]]
    size_t get_ref_count() const {
        return ptr ? ptr->get_ref_count() : 0;
    }

    ~intrusive_ptr() {
        if (ptr) {
            ptr->remove_ref();
        }
    }

    void swap(intrusive_ptr<T> &other) {
        std::swap(other.ptr, ptr);
    }

    // Operators

    T &operator*() const {
        return *ptr;
    }

    T *operator->() const {
        return ptr;
    };

    bool operator>(const intrusive_ptr<T> &other) const {
        return ptr > other.ptr;
    }

    bool operator<(const intrusive_ptr<T> &other) const {
        return ptr < other.ptr;
    }

    intrusive_ptr<T> &operator=(const intrusive_ptr<T> &other) {
        intrusive_ptr<T>(other).swap(*this);
        return *this;
    }

    intrusive_ptr<T> &operator=(intrusive_ptr<T> &&other) noexcept {
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    intrusive_ptr<T> &operator=(T *ptr) {
        intrusive_ptr<T>(ptr).swap(*this);
        return *this;
    }

    bool operator==(const intrusive_ptr<T> &other) const {
        return ptr == other.ptr;
    }

    bool operator!=(const intrusive_ptr<T> &other) const {
        return ptr != other.ptr;
    }

    bool operator!=(const T *ptr) const {
        return ptr != this->ptr;
    }

    bool operator!() const {
        return ptr == nullptr;
    }

    operator bool() const {
        return ptr != nullptr;
    }

    template<typename U>
    intrusive_ptr(const intrusive_ptr<U> &other) {
        static_assert(std::is_base_of_v<U, T> || std::is_base_of_v<T, U>, "U and T not convertible");

        if (!other.get()) {
            ptr = nullptr;
            return;
        }

#if defined(RTTI_ENABLED)
        ptr = dynamic_cast<T *>(other.get());
        if (!ptr) {
            throw std::runtime_error("Cannot convert U to T");
        }
#else
        ptr = static_cast<T *>(other.get());
#endif

        ptr->add_ref();
    }

    template<typename U>
    intrusive_ptr(U *ptr) {
        static_assert(std::is_base_of_v<U, T> || std::is_base_of_v<T, U>, "U and T not convertible");

        if (!ptr) {
            this->ptr = nullptr;
            return;
        }

#if defined(RTTI_ENABLED)
        this->ptr = dynamic_cast<T *>(ptr);
        if (!this->ptr) {
            throw std::runtime_error("Cannot convert U to T");
        }
#else
        this->ptr = static_cast<T *>(ptr);
#endif
        this->ptr->add_ref();
    }

private:
    T *ptr;
};

}

#undef RTTI_ENABLED

#endif //IP_INTRUSIVE_PTR_HPP