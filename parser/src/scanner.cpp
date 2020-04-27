#include <parser/scanner.hpp>

namespace nlang {

Scanner::Scanner(nlang::UniquePtr<nlang::TokenStream>&& token_stream)
    : cache(std::move(token_stream))
    , pos(0)
{}

Scanner::BookMark Scanner::Mark() const {
    return BookMark(const_cast<Scanner*>(this));
}

bool Scanner::IsEOF() const {
    return NextTokenLookahead().token == Token::THE_EOF;
}

bool Scanner::IsEOL() const {
    int32_t p = pos;
    auto mark = Mark();
    mark.ApplyOnDestroy();
    auto tok = const_cast<Scanner*>(this)->NextToken();
    if (tok.token == Token::THE_EOF) {
        return true;
    }
    for (int32_t i = p; i < pos; ++i) {
        if (cache[i].token == Token::NEWLINE) {
            return true;
        }
    }
    return false;
}

TokenInstance& Scanner::NextToken() {
    // TODO skip and report invalid tokens
    static std::unordered_set<Token> tokens_to_skip { Token::NEWLINE, Token::COMMENT, Token::SPACE };
    auto it = StreamCache<TokenStream>::StreamCacheIterator(&cache, pos);
    while (it != cache.end() && tokens_to_skip.find(it->token) != tokens_to_skip.end()) {
        ++it;
    }
    TokenInstance& to_ret = cache[it.GetPosition()];
    pos = it.GetPosition() + 1;
    // TODO cut cache by bookmarks here cache.Cut(...);
    return to_ret;
}

TokenInstance& Scanner::NextTokenAssert(Token token) {
    auto mark = Mark();
    auto& tok = NextToken();
    if (tok.token != token) {
        mark.Apply();
        throw std::runtime_error("Expected " + TokenUtils::GetTokenName(token).GetStdStr() + ", got " + TokenUtils::GetTokenName(tok.token).GetStdStr());
    }
    return tok;
}

TokenInstance& Scanner::NextTokenLookahead() const {
    auto mark = Mark();
    mark.ApplyOnDestroy();
    return const_cast<Scanner*>(this)->NextToken();
}

Scanner::BookMark::BookMark(Scanner* scanner)
    : scanner(scanner)
    , pos(scanner->pos)
    , apply_on_destroy(false)
{}

void Scanner::BookMark::Apply() {
    scanner->pos = pos;
}

void Scanner::BookMark::ApplyOnDestroy() {
    apply_on_destroy = true;
}

Scanner::BookMark::~BookMark() {
    if (apply_on_destroy) {
        Apply();
    }
}


}