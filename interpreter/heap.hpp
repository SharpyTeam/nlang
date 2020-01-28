//
// Created by selya on 12.01.2020.
//

#ifndef NLANG_HEAP_HPP
#define NLANG_HEAP_HPP

#include <utils/defs.hpp>
#include <cstdint>


namespace nlang {

class HeapValue;

class Heap {
public:
    struct HeapEntry {
        HeapValue* value;
        uintptr_t type;
    };


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
