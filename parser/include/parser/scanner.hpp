#pragma once

#include "char_stream.hpp"

#include <common/token.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <stack>
#include <memory>
#include <unordered_set>

namespace nlang {

class Scanner {
private:
    class ScannerImpl {
    public:
        class CachingCharStreamIterator {
            friend class ScannerImpl;
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = const char;
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            explicit CachingCharStreamIterator(ScannerImpl* scanner_impl = nullptr, size_t pos = 0)
                : scanner_impl(scanner_impl)
                , pos(pos)
            {
                if (scanner_impl && pos != size_t(-1) && !scanner_impl->AdvanceCharCache(pos)) {
                    this->pos = -1;
                }
            }

            CachingCharStreamIterator(const CachingCharStreamIterator&) = default;

            CachingCharStreamIterator& operator++() {
                if (!scanner_impl->AdvanceCharCache(++pos)) {
                    pos = -1;
                }
                return *this;
            }

            CachingCharStreamIterator operator++(int) {
                auto iter = CachingCharStreamIterator(scanner_impl, pos);
                this->operator++();
                return iter;
            }

            CachingCharStreamIterator& operator--() {
                if (pos == size_t(-1)) {
                    pos = scanner_impl->GetRealCharCacheEndIndex();
                }
                --pos;
                return *this;
            }

            CachingCharStreamIterator operator--(int) {
                auto iter = CachingCharStreamIterator(scanner_impl, pos);
                this->operator--();
                return iter;
            }

            bool operator==(const CachingCharStreamIterator& other) const {
                return pos == other.pos;
            }

            bool operator!=(const CachingCharStreamIterator& other) const {
                return pos != other.pos;
            }

            value_type operator*() const {
                return scanner_impl->GetCharFromCache(pos);
            }

        private:
            ScannerImpl* scanner_impl;
            size_t pos;
        };

        explicit ScannerImpl(std::shared_ptr<ICharStream> char_stream);

        TokenInstance NextToken();

    private:
        inline bool AdvanceCharCache(size_t index) {
            while (index >= char_stream_cache.size() + char_stream_cache_offset) {
                if (!char_stream->HasNext()) {
                    return false;
                }
                char_stream_cache.emplace_back(char_stream->Next());
            }
            return true;
        }

        inline char GetCharFromCache(size_t index) const {
            return char_stream_cache.at(index - char_stream_cache_offset);
        }

        inline void CutCharCache(size_t index_to) {
            char_stream_cache.erase(char_stream_cache.begin(), char_stream_cache.begin() + index_to - char_stream_cache_offset);
            char_stream_cache_offset = index_to;
        }

        inline size_t GetRealCharCacheEndIndex() {
            while (char_stream->HasNext()) {
                char_stream_cache.emplace_back(char_stream->Next());
            }
            return char_stream_cache_offset + char_stream_cache.size();
        }

        size_t row;
        size_t column;
        size_t char_stream_cache_offset;
        std::vector<char> char_stream_cache;
        std::unique_ptr<ICharStream> char_stream;

        CachingCharStreamIterator current_iterator;
        CachingCharStreamIterator end_iterator;
    };

public:
    class BookMark {
        friend class Scanner;
    public:
        void Apply() {
            scanner->pos = pos;
        }

    private:
        explicit BookMark(Scanner* scanner)
            : scanner(scanner)
            , pos(scanner->pos)
        {

        }

    private:
        Scanner* scanner;
        size_t pos;
    };

    Scanner(const Scanner&) = delete;
    Scanner(Scanner&& other) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner& operator=(Scanner&& other) = delete;

    enum class AdvanceBehaviour {
        DEFAULT,
        NO_IGNORE
    };

    bool TrySkipToken(Token token, AdvanceBehaviour advance_behaviour = AdvanceBehaviour::DEFAULT);
    const TokenInstance& NextTokenAssert(Token token, AdvanceBehaviour advance_behaviour = AdvanceBehaviour::DEFAULT);
    const TokenInstance& NextTokenLookahead(AdvanceBehaviour advance_behaviour = AdvanceBehaviour::DEFAULT);
    const TokenInstance& NextToken(AdvanceBehaviour advance_behaviour = AdvanceBehaviour::DEFAULT);

    BookMark Mark() const;

    void ResetIgnore();
    bool IsIgnoring(Token token) const;
    void SetIgnore(Token token, bool ignore = true);

    static std::shared_ptr<Scanner> Create(const std::shared_ptr<ICharStream>& char_stream);

private:
    explicit Scanner(std::unique_ptr<ICharStream> char_stream);

    const TokenInstance& NextTokenWithoutIgnore();

    ScannerImpl impl;
    size_t pos;
    std::vector<TokenInstance> tokens;

    std::unordered_set<Token> tokens_to_ignore;
};

}
