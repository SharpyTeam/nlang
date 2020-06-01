#pragma once

#include <utils/macro.hpp>

#include <iterator>

namespace nlang {

/**
 * Structure that organized objects of classes that are inherited from it into forward linked list.
 * Uses no additional memory.
 * @tparam T Type of list nodes
 */
template<typename T>
class IntrusiveForwardList {
public:
    struct Hook {
        T* next;
        Hook(T* next = nullptr) : next(next) {}
    };

    using value_type = T;
    using allocator_type = std::allocator<std::nullptr_t>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    class iterator {
        friend class IntrusiveForwardList;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

    public:
        NLANG_FORCE_INLINE iterator(Hook* node) : node(node) {}
        NLANG_FORCE_INLINE iterator(const iterator& other) : node(other.node) {}
        NLANG_FORCE_INLINE ~iterator() = default;

        NLANG_FORCE_INLINE iterator& operator=(const iterator&) = default;

        NLANG_FORCE_INLINE bool operator==(const iterator& other) const {
            return node == other.node;
        }

        NLANG_FORCE_INLINE bool operator!=(const iterator& other) const {
            return !this->operator==(other);
        }

        NLANG_FORCE_INLINE iterator& operator++() {
            // NLANG_ASSERT(node != nullptr)
            node = node->next;
            return *this;
        }

        NLANG_FORCE_INLINE iterator operator++(int) {
            iterator temp(*this);
            this->operator++();
            return temp;
        }

        NLANG_FORCE_INLINE reference operator*() const {
            return *static_cast<T*>(node);
        }

        NLANG_FORCE_INLINE pointer operator->() const {
            return static_cast<T*>(node);
        }

    private:
        Hook* node;
    };

    class const_iterator {
        friend class IntrusiveForwardList;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

    public:
        NLANG_FORCE_INLINE const_iterator(const Hook* node) : node(node) {}
        NLANG_FORCE_INLINE const_iterator(const const_iterator& other) : node(other.node) {}
        NLANG_FORCE_INLINE const_iterator(const iterator& other) : node(other.node) {}
        NLANG_FORCE_INLINE ~const_iterator() = default;

        NLANG_FORCE_INLINE const_iterator& operator=(const const_iterator&) = default;
        NLANG_FORCE_INLINE const_iterator& operator=(const iterator& other) {
            node = other.node;
            return *this;
        };

        NLANG_FORCE_INLINE bool operator==(const const_iterator& other) const {
            return node == other.node;
        }

        NLANG_FORCE_INLINE bool operator!=(const const_iterator& other) const {
            return !this->operator==(other);
        }

        NLANG_FORCE_INLINE const_iterator& operator++() {
            NLANG_ASSERT(node != nullptr);
            node = node->next;
            return *this;
        }

        NLANG_FORCE_INLINE const_iterator operator++(int) {
            const_iterator temp(*this);
            this->operator++();
            return temp;
        }

        NLANG_FORCE_INLINE const_reference operator*() const {
            return *static_cast<const_pointer>(node);
        }

        NLANG_FORCE_INLINE const_pointer operator->() const {
            return static_cast<const_pointer>(node);
        }

    private:
        const Hook* node;
    };

public:
    IntrusiveForwardList() {
        static_assert(std::is_base_of_v<Hook, T>);
    };
    IntrusiveForwardList(const IntrusiveForwardList& other) : head(other.head) {
        static_assert(std::is_base_of_v<Hook, T>);
    }

    IntrusiveForwardList(reference chain) : head(&chain) {
        static_assert(std::is_base_of_v<Hook, T>);
    }

    ~IntrusiveForwardList() = default;
    IntrusiveForwardList& operator=(const IntrusiveForwardList&) = default;

    reference front() {
        return *head.next;
    }

    const_reference front() const {
        return *head.next;
    }

    iterator before_begin() const {
        return iterator(const_cast<Hook*>(&head));
    }

    const_iterator cbefore_begin() const {
        return const_iterator(&head);
    }

    iterator begin() const {
        return iterator(head.next);
    }

    const_iterator cbegin() const {
        return const_iterator(head.next);
    }

    iterator end() const {
        return iterator(nullptr);
    }

    const_iterator cend() const {
        return const_iterator(nullptr);
    }

    bool empty() const {
        return head.next == nullptr;
    }

    size_t size() const {
        size_t nodes_count = 0;
        Hook* current = head.next;
        while (current) {
            ++nodes_count;
            current = current->next;
        }
        return nodes_count;
    }

    size_t max_size() const {
        return -1;
    }

    void clear() {
        resize(0);
    }

    iterator insert_after(const_iterator pos, reference value) {
        NLANG_ASSERT(value.next == nullptr);
        T* next = pos.node->next;
        const_cast<Hook*>(pos.node)->next = &value;
        value.next = next;
        return iterator(next);
    }

    iterator erase_after(const_iterator pos) {
        Hook* node = pos->next;
        const_cast<Hook*>(pos.node)->next = pos.node->next->next;
        node->next = nullptr;
        return iterator(pos.node->next);
    }

    void push_front(reference value) {
        insert_after(cbefore_begin(), value);
    }

    void pop_front() {
        erase_after(cbefore_begin());
    }

    void resize(size_t count) {
        auto it = before_begin();
        while (it != end() && count--) {
            it = std::next(it);
        }
        while (std::next(it) != end()) {
            erase_after(it);
        }
    }

    void swap(IntrusiveForwardList& other) {
        Hook temp = head;
        head = other.head;
        other.head = temp;
    }

    void merge(IntrusiveForwardList& other) {
        merge(other, [](const T& a, const T& b) {
            return a < b;
        });
    }

    template<typename Compare>
    void merge(IntrusiveForwardList& other, Compare&& comp) {
        T* b = other.head.next;
        other.head.next = nullptr;
        T* a = head.next;
        Hook* tail = &head;

        while (a) {
            while (b && comp(*b, *a)) {
                tail->next = b;
                tail = tail->next;
                b = b->next;
            }
            tail->next = a;
            tail = tail->next;
            a = a->next;
        }

        tail->next = b;
    }

    void sort() {
        sort([](const T& a, const T& b) {
            return a < b;
        });
    }

    template<typename Compare>
    void sort(Compare&& comp) {
        if (!head.next || !head.next->next) {
            return;
        }

        size_t half_size = size() / 2;
        auto it = before_begin();
        for (size_t i = 0; i < half_size; ++i) {
            it = std::next(it);
        }

        auto next_it = std::next(it);
        const_cast<Hook*>(it.node)->next = nullptr;

        IntrusiveForwardList<T> second(*static_cast<T*>(const_cast<Hook*>(next_it.node)));
        sort(std::forward<Compare>(comp));
        second.sort(std::forward<Compare>(comp));

        merge(second, std::forward<Compare>(comp));
    }

    size_type remove(const_reference value) {
        return remove_if([&value](const_reference v) {
            return v == value;
        });
    }

    void reverse() noexcept {
        T* prev = nullptr;
        T* current = head.next;
        while (current) {
            T* next = current->next;
            current->next = prev;
            prev = current;
            current = next;
        }
        head.next = prev;
    }

    template<typename UnaryPredicate>
    size_type remove_if(UnaryPredicate&& p) {
        size_type count = 0;
        for (const_iterator it = cbegin(), prev = cbefore_begin(); it != cend(); ) {
            if (p(*it)) {
                it = erase_after(prev);
                ++count;
            } else {
                ++it;
                ++prev;
            }
        }
        return count;
    }

    size_type unique() {
        return unique([](const_reference a, const_reference b) {
            return a == b;
        });
    }

    template<class BinaryPredicate>
    size_type unique(BinaryPredicate&& p) {
        auto prev = cbegin();
        if (prev == cend()) {
            return 0;
        }
        auto current = std::next(prev);
        size_type count = 0;
        while (current != end()) {
            if (p(*prev, *current)) {
                current = erase_after(prev);
                ++count;
            } else {
                prev = current;
                ++current;
            }
        }
        return count;
    }

    void splice_after(const_iterator pos, IntrusiveForwardList& other) {
        splice_after(pos, other, other.cbefore_begin(), other.cend());
    }

    void splice_after(const_iterator pos, IntrusiveForwardList& other, const_iterator it) {
        splice_after(pos, other, it, other.cend());
    }

    void splice_after(const_iterator pos, IntrusiveForwardList& other, const_iterator first, const_iterator last) {
        T* next = pos.node->next;
        Hook* current = const_cast<Hook*>(pos.node);

        auto it = first;
        while (true) {
            current->next = it.node->next;
            if (++it == last) {
                current->next = next;
                break;
            }
            current = current->next;
        }

        const_cast<Hook*>(first.node)->next = static_cast<T*>(const_cast<Hook*>(last.node));
    }

private:
    Hook head;
};

}