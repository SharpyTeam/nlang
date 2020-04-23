#include <catch2/catch.hpp>

#include <parser/token_stream.hpp>
#include <parser/scanner.hpp>

#include <string>

TEST_CASE("scanner test") {
    using namespace nlang;

    const std::string s = "_345kek lol; for + -= \n     /*block comment*/    $$$ 0.5123   ololo()\n  kek  // line comment ";

    Heap heap;

    auto scanner = Scanner::New(TokenStream::New(&heap, String::New(&heap, s)));

    REQUIRE(!scanner->IsEOF());
    REQUIRE(!scanner->IsEOL());
    REQUIRE(scanner->NextTokenLookahead().token == Token::IDENTIFIER);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "_345kek"));
    REQUIRE(scanner->NextTokenLookahead().token == Token::IDENTIFIER);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "lol"));
    REQUIRE(!scanner->IsEOL());
    REQUIRE(scanner->NextToken().token == Token::SEMICOLON);
    REQUIRE(scanner->NextTokenLookahead().token == Token::FOR);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "for"));
    REQUIRE(scanner->NextTokenLookahead().token == Token::ADD);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "+"));
    REQUIRE(scanner->NextTokenLookahead().token == Token::ASSIGN_SUB);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "-="));
    REQUIRE(!scanner->IsEOF());
    REQUIRE(scanner->IsEOL());
    REQUIRE(scanner->NextTokenLookahead().token == Token::INVALID);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "$$$"));
    REQUIRE(scanner->NextTokenLookahead().token == Token::NUMBER);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "0.5123"));
    REQUIRE(scanner->NextTokenLookahead().token == Token::IDENTIFIER);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "ololo"));
    REQUIRE(scanner->NextToken().token == Token::LEFT_PAR);
    REQUIRE(scanner->NextToken().token == Token::RIGHT_PAR);
    REQUIRE(scanner->IsEOL());
    REQUIRE(scanner->NextTokenLookahead().token == Token::IDENTIFIER);
    REQUIRE(scanner->NextToken().text == String::New(&heap, "kek"));
    REQUIRE(scanner->IsEOL());
    REQUIRE(scanner->IsEOF());
}