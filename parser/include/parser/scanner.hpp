#pragma once

#include "token_stream.hpp"

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

    private:
        explicit BookMark(Scanner* scanner)
            : scanner(scanner)
            , pos_it(scanner->marked_positions.insert(scanner->pos))
            , apply_on_destroy(false)
        {

        }

        ~BookMark() {
            if (apply_on_destroy) {
                Apply();
            }
            scanner->marked_positions.erase(pos_it);
        }

        Scanner* scanner;
        std::multiset<size_t>::const_iterator pos_it;
        bool apply_on_destroy;
    };

    Scanner(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner& operator=(Scanner&&) = delete;


    BookMark Mark() const {
        return BookMark(const_cast<Scanner*>(this));
    }

    bool IsEOF() const {
        return NextTokenLookahead().token == Token::THE_EOF;
    }

    bool IsEOL() const {
        size_t p = pos;
        auto mark = Mark();
        mark.ApplyOnDestroy();
        auto& token_instance = const_cast<Scanner*>(this)->NextToken();
        if (token_instance.token == Token::THE_EOF || token_instance.token == Token::SEMICOLON) {
            return true;
        }
        for (size_t i = p + 1; i < pos; ++i) {
            if (cache[i].token == Token::NEWLINE) {
                return true;
            }
        }
        return false;
    }

    const TokenInstance& NextToken() {
        static std::unordered_set<Token> tokens_to_skip { Token::NEWLINE, Token::COMMENT, Token::SPACE };
        auto it = StreamCache<TokenStream>::StreamCacheIterator(&cache, pos);
        while (it != cache.end() && tokens_to_skip.find(it->token) != tokens_to_skip.end()) {
            ++it;
        }
        pos = it.GetPosition() + 1;
        // TODO maybe return by value and uncomment this?
        // cache.Cut(marked_positions.empty() ? pos : *marked_positions.begin());
        return *it;
    }

    const TokenInstance& NextTokenAssert(Token token) {
        auto mark = Mark();
        auto& token_instance = NextToken();
        if (token_instance.token != token) {
            mark.Apply();
            // TODO error
        }
        return token_instance;
    }

    const TokenInstance& NextTokenLookahead() const {
        auto mark = Mark();
        mark.ApplyOnDestroy();
        return const_cast<Scanner*>(this)->NextToken();
    }


    static std::unique_ptr<Scanner> New(std::unique_ptr<TokenStream>&& token_stream) {
        return std::unique_ptr<Scanner>(new Scanner(std::move(token_stream)));
    }

private:
    Scanner(std::unique_ptr<TokenStream>&& token_stream)
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