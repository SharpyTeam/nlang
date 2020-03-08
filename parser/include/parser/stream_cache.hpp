#pragma once

#include <utils/holder.hpp>

#include <iterator>
#include <memory>
#include <vector>


namespace nlang {

template<typename S>
class StreamCache {
public:
    using T = std::decay_t<decltype(std::declval<S>().Next())>;

    friend class StreamCacheIterator;

    class StreamCacheIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        explicit StreamCacheIterator(StreamCache* stream_cache = nullptr, size_t pos = -1)
            : stream_cache(stream_cache)
            , pos((pos != (size_t)-1 && !stream_cache->Advance(pos)) ? (size_t)-1 : pos)
        {

        }

        StreamCacheIterator(const StreamCacheIterator&) = default;
        StreamCacheIterator& operator=(const StreamCacheIterator&) = default;

        StreamCacheIterator& operator++() {
            NLANG_ASSERT(pos != (size_t)-1);
            if (!stream_cache->Advance(++pos)) {
                pos = (size_t)-1;
            }
            return *this;
        }

        StreamCacheIterator operator++(int) {
            auto iter = CharStreamCacheIterator(stream_cache, pos);
            this->operator++();
            return iter;
        }

        StreamCacheIterator& operator--() {
            NLANG_ASSERT(pos != 0);
            if (pos == (size_t)-1) {
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

        pointer operator->() const {
            return &(*stream_cache)[pos];
        }

        void CutCacheToThis() {
            stream_cache->Cut(pos);
        }

        size_t GetPosition() const {
            return pos;
        }

    private:
        StreamCache* stream_cache;
        size_t pos;
    };

    explicit StreamCache(Holder<S>&& stream)
        : stream(std::move(stream))
        , offset(0)
        , begin_(this, 0)
        , end_(this)
    {

    }

    const T& operator[](size_t index) const {
        NLANG_ASSERT(index >= offset);
        const_cast<StreamCache*>(this)->Advance(index);
        return buffer[index - offset];
    }

    StreamCacheIterator begin() const {
        return begin_;
    }

    StreamCacheIterator end() const {
        return end_;
    }

    void Cut(size_t index_to) {
        NLANG_ASSERT(index_to >= offset);
        if (index_to == (size_t)-1) {
            index_to = buffer.size() + offset;
        }
        buffer.erase(buffer.begin(), buffer.begin() + index_to - offset);
        offset = index_to;
        begin_ = StreamCacheIterator(this, offset);
    }

private:
    bool Advance(size_t index_to) {
        while (index_to >= (size_t)buffer.size() + offset) {
            if (!stream->HasNext()) {
                return false;
            }
            buffer.push_back(stream->Next());
        }
        return true;
    }

    size_t AdvanceToEnd() {
        while (stream->HasNext()) {
            buffer.push_back(stream->Next());
        }
        return offset + (size_t)buffer.size();
    }

private:
    Holder<S> stream;
    std::vector<T> buffer;
    size_t offset;
    StreamCacheIterator begin_;
    StreamCacheIterator end_;
};

}