set(NLANG_UTILS_SOURCES
        src/backtrace.cpp
        src/page.cpp
        src/thread_pool.cpp
)

set(NLANG_UTILS_HEADERS
        include/utils/macro.hpp
        include/utils/traits.hpp

        include/utils/alloc.hpp
        include/utils/alloc/page.hpp
        include/utils/alloc/aligned_alloc.hpp

        include/utils/containers.hpp
        include/utils/containers/forward_list_view.hpp
        include/utils/containers/nan_boxed_primitive.hpp
        include/utils/containers/slot_storage.hpp

        include/utils/debug.hpp
        include/utils/debug/backtrace.hpp

        include/utils/pointers.hpp
        include/utils/pointers/intrusive_ptr.hpp
        include/utils/pointers/shared_ptr.hpp
        include/utils/pointers/unique_ptr.hpp

        include/utils/strings.hpp
        include/utils/strings/ustring.hpp

        include/utils/threading.hpp
        include/utils/threading/thread_pool.hpp
)

if (NOT DISABLE_UT)
    add_subdirectory(ut)
endif ()

add_library(nlang_utils STATIC ${NLANG_UTILS_HEADERS} ${NLANG_UTILS_SOURCES})
target_include_directories(nlang_utils PUBLIC include)
target_link_libraries(nlang_utils PUBLIC ICU::i18n ICU::uc)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(nlang_utils PRIVATE Backward::Backward)
endif ()
