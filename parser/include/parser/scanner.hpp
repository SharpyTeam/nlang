#pragma once

#include "token_stream.hpp"

#include <utils/holder.hpp>
#include <common/token.hpp>

#include <memory>
#include <set>
#include <unordered_set>
#include <cstddef>
#include <iostream>

namespace nlang {


class Scanner {
public:
    class BookMark {
        friend class Scanner;

    public:
        void Apply() {
            scanner->pos = *pos_it;
        }

        void ApplyOnDestroy() {
            apply_on_destroy = true;
        }

        ~BookMark() {
            if (apply_on_destroy) {
                Apply();
            }
            scanner->marked_positions.erase(pos_it);
        }

    private:
        explicit BookMark(Scanner* scanner)
            : scanner(scanner)
            , pos_it(scanner->marked_positions.insert(scanner->pos))
            , apply_on_destroy(false)
        {

        }

        Scanner* scanner;
        std::multiset<size_t>::const_iterator pos_it;
        bool apply_on_destroy;
    };

    class TokenInstanceHandle {
        friend class Scanner;

    public:
        TokenInstance& operator*() const {
            return scanner->cache[*pos_it];
        }

        TokenInstance* operator->() const {
            return &scanner->cache[*pos_it];
        }

        ~TokenInstanceHandle() {
            scanner->marked_positions.erase(pos_it);
        }

    private:
        explicit TokenInstanceHandle(Scanner* scanner, size_t pos)
            : scanner(scanner)
            , pos_it(scanner->marked_positions.insert(pos))
        {

        }

    private:
        Scanner* scanner;
        std::multiset<size_t>::const_iterator pos_it;
    };

    Scanner(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner& operator=(Scanner&&) = delete;


    BookMark Mark() const {
        return BookMark(const_cast<Scanner*>(this));
    }

    bool IsEOF() const {
        return NextTokenLookahead()->token == Token::THE_EOF;
    }

    bool IsEOL() const {
        size_t p = pos;
        auto mark = Mark();
        mark.ApplyOnDestroy();
        auto tok = const_cast<Scanner*>(this)->NextToken();
        if (tok->token == Token::THE_EOF) {
            return true;
        }
        for (size_t i = p; i < pos; ++i) {
            if (cache[i].token == Token::NEWLINE) {
                return true;
            }
        }
        return false;
    }

    TokenInstanceHandle NextToken() {
        // TODO skip and report invalid tokens
        static std::unordered_set<Token> tokens_to_skip { Token::NEWLINE, Token::COMMENT, Token::SPACE };
        auto it = StreamCache<TokenStream>::StreamCacheIterator(&cache, pos);
        while (it != cache.end() && tokens_to_skip.find(it->token) != tokens_to_skip.end()) {
            ++it;
        }
        TokenInstanceHandle handle(this, it.GetPosition());
        pos = it.GetPosition() + 1;
        cache.Cut(*marked_positions.begin());
        return handle;
    }

    TokenInstanceHandle NextTokenAssert(Token token) {
        auto mark = Mark();
        auto tok = NextToken();
        if (tok->token != token) {
            mark.Apply();
            throw std::runtime_error("Expected " + TokenUtils::GetTokenName(token) + ", got " + TokenUtils::GetTokenName(tok->token));
        }
        return tok;
    }

    TokenInstanceHandle NextTokenLookahead() const {
        auto mark = Mark();
        mark.ApplyOnDestroy();
        return const_cast<Scanner*>(this)->NextToken();
    }


    static Holder<Scanner> New(Holder<TokenStream>&& token_stream) {
        return Holder<Scanner>(new Scanner(std::move(token_stream)));
    }

private:
    Scanner(Holder<TokenStream>&& token_stream)
        : cache(std::move(token_stream))
        , pos(0)
    {

    }

private:
    StreamCache<TokenStream> cache;
    size_t pos;
    std::multiset<size_t> marked_positions;
};

}