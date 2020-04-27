#pragma once

#include <utils/macro.hpp>

#include <memory>

namespace nlang {

template<typename T, typename D = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, D>;

template<typename T, typename ...Args>
NLANG_FORCE_INLINE UniquePtr<T> MakeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

}