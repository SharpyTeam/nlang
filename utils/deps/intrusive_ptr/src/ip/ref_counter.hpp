//
// Created by Ilya on 07.09.2019.
//

#ifndef IP_REF_COUNTER_HPP
#define IP_REF_COUNTER_HPP

#include <cstddef>

namespace ip {

class ref_counter {
    size_t refs;

    template<class T>
    friend class intrusive_ptr;

    void add_ref() {
        ++refs;
    }

    void remove_ref() {
        if (--refs <= 0) {
            delete this;
        }
    }

    [[nodiscard]]
    size_t get_ref_count() const {
        return refs;
    }

protected:
    ref_counter() : refs(0) {}
    virtual ~ref_counter() = 0;
};

inline ip::ref_counter::~ref_counter() = default;

}

#endif //IP_REF_COUNTER_HPP
