#pragma once

#include <common/token.hpp>

#include <parser/token_stream.hpp>
#include <parser/stream_cache.hpp>

#include <utils/pointers/unique_ptr.hpp>

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
        void Apply();
        void ApplyOnDestroy();
        ~BookMark();

    private:
        explicit BookMark(Scanner* scanner);

        Scanner* scanner;
        int32_t pos;
        bool apply_on_destroy;
    };

    Scanner(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner& operator=(Scanner&&) = delete;


    BookMark Mark() const;

    bool IsEOF() const;
    bool IsEOL() const;

    TokenInstance& NextToken();
    TokenInstance& NextTokenAssert(Token token);
    TokenInstance& NextTokenLookahead() const;


    static UniquePtr<Scanner> New(UniquePtr<TokenStream>&& token_stream) {
        return UniquePtr<Scanner>(new Scanner(std::move(token_stream)));
    }

private:
    Scanner(UniquePtr<TokenStream>&& token_stream);

private:
    StreamCache<TokenStream> cache;
    int32_t pos;
};

}