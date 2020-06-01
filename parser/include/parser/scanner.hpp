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

/**
 * Scanner
 * Splits the source code into tokens, generation the token sequence, understandable by parser
 */
class Scanner {
public:
    /**
     * Reference to specific position in source code.
     * Can be used to return to this position if needed.
     */
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


    /**
     * Creates a mark to specific place in source code
     * @return
     */
    BookMark Mark() const;

    /**
     * Checks if was EOF reached.
     * @return True if EOF was reached, otherwise false
     */
    bool IsEOF() const;
    /**
     * Checks if was EOL reached.
     * @return True if EOF was reached, otherwise false
     */
    bool IsEOL() const;

    /**
     * Consumes/scans next token, skipping unwanted (comment/spaces) tokens
     * @return Scanned token instance
     */
    TokenInstance& NextToken();
    /**
     * Consumes/scans next token, skipping unwanted (comment/spaces) tokens, looking for expected token
     * Throws a runtime error if the token was not expected
     * @param token Expected token
     * @return Scanned token instance
     */
    TokenInstance& NextTokenAssert(Token token);
    /**
     * Scans next token, skipping unwanted (comment/spaces) tokens, without actually consuming it
     * @return Scanned token instance
     */
    TokenInstance& NextTokenLookahead() const;

    /**
     * Creates a scanner instance from token stream
     * @param token_stream Token stream
     * @return Unique pointer to created scanner instance
     */
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