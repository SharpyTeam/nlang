#include <catch2/catch.hpp>

#include <parser/char_stream.hpp>
#include <parser/token_stream.hpp>
#include <parser/scanner.hpp>

#include <string>

TEST_CASE("scanner test") {
    using namespace nlang;

    const std::string s = "_345kek lol; for + -= \n     /*block comment*/    $$$ 0.5123   ololo()\n  kek  // line comment ";

    auto scanner = Scanner::New(TokenStream::New(StringCharStream::New(s)));

    REQUIRE(!scanner->IsEOF());
    REQUIRE(!scanner->IsEOL());
    REQUIRE(scanner->NextTokenLookahead()->token == Token::IDENTIFIER);
    REQUIRE(scanner->NextToken()->text == "_345kek");
    REQUIRE(scanner->NextTokenLookahead()->token == Token::IDENTIFIER);
    REQUIRE(scanner->NextToken()->text == "lol");
    REQUIRE(!scanner->IsEOL());
    REQUIRE(scanner->NextToken()->token == Token::SEMICOLON);
    REQUIRE(scanner->NextTokenLookahead()->token == Token::FOR);
    REQUIRE(scanner->NextToken()->text == "for");
    REQUIRE(scanner->NextTokenLookahead()->token == Token::ADD);
    REQUIRE(scanner->NextToken()->text == "+");
    REQUIRE(scanner->NextTokenLookahead()->token == Token::ASSIGN_SUB);
    REQUIRE(scanner->NextToken()->text == "-=");
    REQUIRE(!scanner->IsEOF());
    REQUIRE(scanner->IsEOL());
    REQUIRE(scanner->NextTokenLookahead()->token == Token::INVALID);
    REQUIRE(scanner->NextToken()->text == "$$$");
    REQUIRE(scanner->NextTokenLookahead()->token == Token::NUMBER);
    REQUIRE(scanner->NextToken()->text == "0.5123");
    REQUIRE(scanner->NextTokenLookahead()->token == Token::IDENTIFIER);
    REQUIRE(scanner->NextToken()->text == "ololo");
    REQUIRE(scanner->NextToken()->token == Token::LEFT_PAR);
    REQUIRE(scanner->NextToken()->token == Token::RIGHT_PAR);
    REQUIRE(scanner->IsEOL());
    REQUIRE(scanner->NextTokenLookahead()->token == Token::IDENTIFIER);
    REQUIRE(scanner->NextToken()->text == "kek");
    REQUIRE(scanner->IsEOL());
    REQUIRE(scanner->IsEOF());
}