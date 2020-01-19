//
// Created by selya on 12.01.2020.
//

#ifndef NLANG_HEAP_HPP
#define NLANG_HEAP_HPP

#include "object.hpp"

#include <utils/defs.hpp>

#include <vector>
#include <cstdint>
#include <memory>
#include <map>
#include <unordered_map>
#include <cstdlib>
#include <new>
#include <optional>
#include <atomic>
#include <iostream>

#if defined(NLANG_PLATFORM_LINUX)
#include <unistd.h>
#include <sys/mman.h>
#elif defined(NLANG_PLATFORM_WINDOWS)
// Sets minimal API level requirement to Windows 7
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <Windows.h>
#endif


namespace nlang {

class Heap {
public:
    template<typename T>
    static T* GetHeapObjectPointer(void* cell_ptr) {
        return *reinterpret_cast<T**>(cell_ptr);
    }

    template<typename T>
    static void* StoreHeapObjectPointer(T* obj_ptr) {
        std::cout << "heap" << std::endl;
        if (Storage().size() == Storage().capacity()) {
            throw std::bad_alloc();
        }
        Storage().emplace_back(obj_ptr);
        return reinterpret_cast<void*>(&Storage()[Storage().size() - 1]);
    }

private:
    static std::vector<void*>& Storage() {
        static std::vector<void*> storage;
        static bool init = false;
        if (!init) {
            init = true;
            storage.reserve(1024);
        }
        return storage;
    }
};

}

#endif //NLANG_HEAP_HPP
