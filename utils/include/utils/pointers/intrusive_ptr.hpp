#pragma once

#include <utils/macro.hpp>

#include <cstddef>
#include <type_traits>

namespace nlang {


class IntrusivePtrRefCounter {
public:
    void ref() const noexcept {
        ++ref_count;
    }

    void unref() const noexcept {
        if (!--ref_count) {
            delete this;
        }
    }

    long use_count() const noexcept {
        return ref_count;
    }

    virtual ~IntrusivePtrRefCounter() = 0;

protected:
    IntrusivePtrRefCounter()
        : ref_count(0)
    {}

private:
    mutable long ref_count;
};


inline IntrusivePtrRefCounter::~IntrusivePtrRefCounter() = default;


template<typename T>
class IntrusivePtr final {
public:
    using element_type = T;

    constexpr IntrusivePtr() noexcept
        : ptr(nullptr)
    {}

    constexpr IntrusivePtr(std::nullptr_t) noexcept
        : ptr(nullptr)
    {}

    template<typename U>
    explicit IntrusivePtr(U* ptr_) noexcept
        : ptr(ptr_)
    {
        static_assert(std::is_base_of_v<IntrusivePtrRefCounter, U>);
        if (ptr) {
            ptr->ref();
        }
    }

    IntrusivePtr(const IntrusivePtr& other) noexcept
        : ptr(other.get())
    {
        if (ptr) {
            ptr->ref();
        }
    }

    template<typename U>
    IntrusivePtr(const IntrusivePtr<U>& other) noexcept
        : ptr(other.get())
    {
        static_assert(std::is_base_of_v<IntrusivePtrRefCounter, U>);
        if (ptr) {
            ptr->ref();
        }
    }

    IntrusivePtr(IntrusivePtr&& other) noexcept
        : ptr(other.get())
    {
        other.ptr = nullptr;
    }

    template<typename U>
    IntrusivePtr(IntrusivePtr<U>&& other) noexcept
        : ptr(other.get())
    {
        static_assert(std::is_base_of_v<IntrusivePtrRefCounter, U>);
        other.ptr = nullptr;
    }

    IntrusivePtr& operator=(const IntrusivePtr& other) noexcept {
        IntrusivePtr(other).swap(*this);
        return *this;
    }

    template<typename U>
    IntrusivePtr& operator=(const IntrusivePtr<U>& other) noexcept {
        static_assert(std::is_base_of_v<IntrusivePtrRefCounter, U>);
        IntrusivePtr<T>(other).swap(*this);
        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr&& other) noexcept {
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    template<typename U>
    IntrusivePtr& operator=(IntrusivePtr<U>&& other) noexcept {
        static_assert(std::is_base_of_v<IntrusivePtrRefCounter, U>);
        ptr = other.get();
        other.ptr = nullptr;
        return *this;
    }

    void reset() noexcept {
        IntrusivePtr().swap(*this);
    }

    template<typename U>
    void reset(U* ptr_) noexcept {
        IntrusivePtr<T>(ptr_).swap(*this);
    }

    void swap(IntrusivePtr& other) noexcept {
        T* tmp = other.ptr;
        other.ptr = ptr;
        ptr = tmp;
    }

    element_type *get() const noexcept {
        return ptr;
    }

    T& operator*() const noexcept {
        return *get();
    }

    T* operator->() const noexcept {
        return get();
    }

    long use_count() const noexcept {
        return ptr ? ptr->use_count() : 0;
    }

    explicit operator bool() const noexcept {
        return get() != nullptr;
    }

    ~IntrusivePtr() noexcept {
        if (ptr) {
            ptr->unref();
        }
    }

private:
    T* ptr;
};


template<typename T, typename U>
bool operator==(const IntrusivePtr<T>& lhs, const IntrusivePtr<U>& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template<typename T, typename U>
bool operator!=(const IntrusivePtr<T>& lhs, const IntrusivePtr<U>& rhs) noexcept {
    return lhs.get() != rhs.get();
}

template<typename T, typename U>
bool operator<(const IntrusivePtr<T>& lhs, const IntrusivePtr<U>& rhs) noexcept {
    return lhs.get() < rhs.get();
}

template<typename T, typename U>
bool operator>(const IntrusivePtr<T>& lhs, const IntrusivePtr<U>& rhs) noexcept {
    return lhs.get() > rhs.get();
}

template<typename T, typename U>
bool operator<=(const IntrusivePtr<T>& lhs, const IntrusivePtr<U>& rhs) noexcept {
    return lhs.get() <= rhs.get();
}

template<typename T, typename U>
bool operator>=(const IntrusivePtr<T>& lhs, const IntrusivePtr<U>& rhs) noexcept {
    return lhs.get() >= rhs.get();
}


template<typename T>
bool operator==(const IntrusivePtr<T>& lhs, std::nullptr_t) noexcept {
    return !lhs;
}

template<typename T>
bool operator==(std::nullptr_t , const IntrusivePtr<T>& rhs) noexcept {
    return !rhs;
}

template<typename T>
bool operator!=(const IntrusivePtr<T>& lhs, std::nullptr_t) noexcept {
    return !!lhs;
}

template<typename T>
bool operator!=(std::nullptr_t , const IntrusivePtr<T>& rhs) noexcept {
    return !!rhs;
}

template<typename T>
bool operator<(const IntrusivePtr<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() < nullptr;
}

template<typename T>
bool operator<(std::nullptr_t, const IntrusivePtr<T>& rhs) noexcept {
    return nullptr < rhs.get();
}

template<typename T>
bool operator>(const IntrusivePtr<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() > nullptr;
}

template<typename T>
bool operator>(std::nullptr_t, const IntrusivePtr<T>& rhs) noexcept {
    return nullptr > rhs.get();
}

template<typename T>
bool operator<=(const IntrusivePtr<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() <= nullptr;
}

template<typename T>
bool operator<=(std::nullptr_t, const IntrusivePtr<T>& rhs) noexcept {
    return nullptr <= rhs.get();
}

template<typename T>
bool operator>=(const IntrusivePtr<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() >= nullptr;
}

template<typename T>
bool operator>=(std::nullptr_t, const IntrusivePtr<T>& rhs) noexcept {
    return nullptr >= rhs.get();
}


template<typename T, typename U>
NLANG_FORCE_INLINE IntrusivePtr<T> StaticPointerCast(const IntrusivePtr<U>& p) noexcept {
    return IntrusivePtr<T>(static_cast<T*>(p.get()));
}

template<typename T, typename U>
NLANG_FORCE_INLINE IntrusivePtr<T> DynamicPointerCast(const IntrusivePtr<U>& p) noexcept {
    return IntrusivePtr<T>(dynamic_cast<T*>(p.get()));
}

template<typename T, typename U>
NLANG_FORCE_INLINE IntrusivePtr<T> ConstPointerCast(const IntrusivePtr<U>& p) noexcept {
    return IntrusivePtr<T>(const_cast<T*>(p.get()));
}


}