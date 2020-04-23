#pragma once

#include <utils/macro.hpp>

#include <memory>

namespace nlang {

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using WeakPtr = std::weak_ptr<T>;

template<typename T, typename ...Args>
NLANG_FORCE_INLINE SharedPtr<T> MakeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename U>
NLANG_FORCE_INLINE SharedPtr<T> SharedCast(const SharedPtr<U>& p) {
    return std::static_pointer_cast<T>(p);
}

}