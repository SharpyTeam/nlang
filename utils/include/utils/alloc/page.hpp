#pragma once

#include <utils/macro.hpp>

#include <cstddef>
#include <iterator>

namespace nlang {

/**
 * The page.
 * Allocates respecting the borders of system pages to ensure higher access speed.
 */
class Page {
public:
    class PageIterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Page;
        using difference_type = std::ptrdiff_t;
        using pointer = Page*;
        using reference = Page&;

    public:
        NLANG_FORCE_INLINE PageIterator(void* data = nullptr) : data(data) {}
        NLANG_FORCE_INLINE PageIterator(const PageIterator&) = default;
        NLANG_FORCE_INLINE PageIterator& operator=(const PageIterator&) = default;

        NLANG_FORCE_INLINE ~PageIterator() = default;

        NLANG_FORCE_INLINE difference_type operator-(const PageIterator& other) const {
            return std::distance(static_cast<uint8_t*>(data), static_cast<uint8_t*>(other.data)) / Page::size();
        }

        NLANG_FORCE_INLINE PageIterator operator+(difference_type count) const {
            return PageIterator((char*)data + (difference_type)Page::size() * count);
        }

        NLANG_FORCE_INLINE PageIterator operator-(difference_type count) const {
            return PageIterator((char*)data - (difference_type)Page::size() * count);
        }

        NLANG_FORCE_INLINE PageIterator& operator+=(difference_type count) {
            data = (char*)data + (difference_type)Page::size() * count;
            return *this;
        }

        NLANG_FORCE_INLINE PageIterator& operator-=(difference_type count) {
            data = (char*)data - (difference_type)Page::size() * count;
            return *this;
        }

        NLANG_FORCE_INLINE PageIterator& operator++() {
            *this += 1;
            return *this;
        }

        NLANG_FORCE_INLINE PageIterator& operator--() {
            *this -= 1;
            return *this;
        }

        NLANG_FORCE_INLINE PageIterator operator++(int) {
            PageIterator p(data);
            *this += 1;
            return p;
        }

        NLANG_FORCE_INLINE PageIterator operator--(int) {
            PageIterator p(data);
            *this -= 1;
            return p;
        }

        NLANG_FORCE_INLINE bool operator==(const PageIterator& other) const {
            return data == other.data;
        }

        NLANG_FORCE_INLINE bool operator!=(const PageIterator& other) const {
            return data != other.data;
        }

        NLANG_FORCE_INLINE bool operator<(const PageIterator& other) const {
            return data < other.data;
        }

        NLANG_FORCE_INLINE bool operator>(const PageIterator& other) const {
            return data > other.data;
        }

        NLANG_FORCE_INLINE bool operator<=(const PageIterator& other) const {
            return data <= other.data;
        }

        NLANG_FORCE_INLINE bool operator>=(const PageIterator& other) const {
            return data >= other.data;
        }

        NLANG_FORCE_INLINE const Page& operator*() const {
            return *static_cast<const Page*>(data);
        }

        NLANG_FORCE_INLINE Page& operator*() {
            return *static_cast<Page*>(data);
        }

        NLANG_FORCE_INLINE Page* operator->() {
            return static_cast<Page*>(data);
        }

        NLANG_FORCE_INLINE const Page* operator->() const {
            return static_cast<const Page*>(data);
        }

    protected:
        void* data;
    };

    static std::pair<PageIterator, PageIterator> AllocateRange(size_t count = 1);
    static void FreeRange(const std::pair<PageIterator, PageIterator>& range);

    static Page* Allocate();

    static void Free(Page* page);

    Page(const Page&) = delete;
    Page(Page&&) = delete;
    Page& operator=(const Page&) = delete;
    Page& operator=(Page&&) = delete;

    NLANG_FORCE_INLINE void* data() const {
        return const_cast<Page*>(this);
    }

    NLANG_FORCE_INLINE static size_t size() {
        return size_;
    }

protected:
    Page() = default;
    ~Page() = default;

    static size_t SizeImpl();

    inline static size_t size_ = SizeImpl();
};


}
