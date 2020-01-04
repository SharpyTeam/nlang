//
// Created by selya on 05.11.2019.
//

#ifndef NLANG_SCANNER_HPP
#define NLANG_SCANNER_HPP

#include "char_stream.hpp"

#include <token.hpp>

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
        class CachingCharStreamIterator : public std::iterator<
            std::bidirectional_iterator_tag,
            char,
            ptrdiff_t,
            const char *,
            char>
        {
            friend class ScannerImpl;
        public:
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

            reference operator*() const {
                return scanner_impl->GetCharFromCache(pos);
            }

        private:
            ScannerImpl* scanner_impl;
            size_t pos;
        };

        explicit ScannerImpl(std::shared_ptr<CharStream> char_stream);

        TokenInstance NextToken();

    private:
        inline bool AdvanceCharCache(size_t index) {
            while (index >= char_stream_cache.size() + char_stream_cache_offset) {
                if (!char_stream->HasNextChar()) {
                    return false;
                }
                char_stream_cache.emplace_back(char_stream->NextChar());
            }
            return true;
        }

        [[nodiscard]]
        inline char GetCharFromCache(size_t index) const {
            return char_stream_cache.at(index - char_stream_cache_offset);
        }

        inline void CutCharCache(size_t index_to) {
            char_stream_cache.erase(char_stream_cache.begin(), char_stream_cache.begin() + index_to - char_stream_cache_offset);
            char_stream_cache_offset = index_to;
        }

        inline size_t GetRealCharCacheEndIndex() {
            while (char_stream->HasNextChar()) {
                char_stream_cache.emplace_back(char_stream->NextChar());
            }
            return char_stream_cache_offset + char_stream_cache.size();
        }

        size_t row;
        size_t column;
        size_t char_stream_cache_offset;
        std::vector<char> char_stream_cache;
        std::shared_ptr<CharStream> char_stream;

        CachingCharStreamIterator current_iterator;
        CachingCharStreamIterator end_iterator;
    };

public:
    class BookMark {
        friend class Scanner;
    public:
        Scanner* scanner;

    private:
        size_t pos;

        explicit BookMark(Scanner* scanner)
            : scanner(scanner)
            , pos(scanner->pos)
        {

        }

    public:
        void Apply() {
            scanner->pos = pos;
        }
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

    static std::shared_ptr<Scanner> Create(const std::shared_ptr<CharStream>& char_stream);

private:
    explicit Scanner(std::shared_ptr<CharStream> char_stream);

    const TokenInstance& NextTokenWithoutIgnore();

    ScannerImpl impl;
    size_t pos;
    std::vector<TokenInstance> tokens;

    std::unordered_set<Token> tokens_to_ignore;
};

}

#endif //NLANG_SCANNER_HPP
