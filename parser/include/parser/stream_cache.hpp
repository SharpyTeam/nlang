#pragma once

#include <iterator>
#include <memory>
#include <vector>


namespace nlang {

template<typename S>
class StreamCache {
public:
    using T = decltype(std::declval<S>().Next());

    friend class StreamCacheIterator;

    class StreamCacheIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const T;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        explicit StreamCacheIterator(StreamCache* stream_cache, std::uintptr_t pos = -1)
            : stream_cache(stream_cache)
            , pos((pos != -1 && !stream_cache->Advance(pos)) ? -1 : pos)
        {

        }

        StreamCacheIterator(const StreamCacheIterator&) = default;
        StreamCacheIterator& operator=(const StreamCacheIterator&) = default;

        StreamCacheIterator& operator++() {
            if (!stream_cache->Advance(++pos)) {
                pos = -1;
            }
            return *this;
        }

        StreamCacheIterator operator++(int) {
            auto iter = CharStreamCacheIterator(stream_cache, pos);
            this->operator++();
            return iter;
        }

        StreamCacheIterator& operator--() {
            if (pos == -1) {
                pos = stream_cache->AdvanceToEnd();
            }
            --pos;
            return *this;
        }

        StreamCacheIterator operator--(int) {
            auto iter = CharStreamCacheIterator(stream_cache, pos);
            this->operator--();
            return iter;
        }

        bool operator==(const StreamCacheIterator& other) const {
            return pos == other.pos;
        }

        bool operator!=(const StreamCacheIterator& other) const {
            return pos != other.pos;
        }

        reference operator*() const {
            return (*stream_cache)[pos];
        }

        void CutCacheToThis() {
            stream_cache->Cut(pos);
        }

        std::uintptr_t GetPosition() const {
            return pos;
        }

    private:
        StreamCache* stream_cache;
        std::uintptr_t pos;
    };

    explicit StreamCache(std::unique_ptr<S>&& stream)
        : stream(std::move(stream))
        , offset(0)
        , begin_(this, 0)
        , end_(this)
    {

    }

    const T& operator[](std::uintptr_t index) const {
        NLANG_ASSERT(index >= offset);
        Advance(index);
        return buffer[index - offset];
    }

    StreamCacheIterator begin() const {
        return begin_;
    }

    StreamCacheIterator end() const {
        return end_;
    }

    bool empty() const {
        return begin_ == end_;
    }

    void Cut(std::intptr_t index_to) {
        NLANG_ASSERT(index_to >= offset);
        buffer.erase(buffer.begin(), buffer.begin() + index_to - offset);
        offset = index_to;
        begin_ = CharStreamCacheIterator(this, offset);
    }

private:
    bool Advance(std::uintptr_t index_to) {
        while (index_to >= (std::uintptr_t)buffer.size() + offset) {
            if (!stream->HasNext()) {
                return false;
            }
            buffer.push_back(stream->Next());
        }
        return true;
    }

    std::intptr_t AdvanceToEnd() {
        while (stream->HasNext()) {
            buffer.push_back(stream->Next());
        }
        return offset + (std::uintptr_t)buffer.size();
    }

private:
    std::unique_ptr<S> stream;
    std::intptr_t offset;
    std::vector<T> buffer;
    StreamCacheIterator begin_;
    StreamCacheIterator end_;
};

}