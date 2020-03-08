set(NLANG_UTILS_SOURCES
        src/backtrace.cpp)

set(NLANG_UTILS_HEADERS
        include/utils/macro.hpp
        include/utils/nan_boxed_primitive.hpp
        include/utils/traits.hpp
        include/utils/backtrace.hpp
        include/utils/thread_pool.hpp
        include/utils/page.hpp
        include/utils/forward_list_view.hpp
        include/utils/slot_storage.hpp
        include/utils/holder.hpp)

add_subdirectory(ut)

add_library(nlang_utils ${NLANG_UTILS_HEADERS} ${NLANG_UTILS_SOURCES})
target_include_directories(nlang_utils PUBLIC include)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(nlang_utils Backward::Backward)
endif ()
