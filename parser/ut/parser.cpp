#include <catch2/catch.hpp>

#include <iostream>
#include <parser/char_stream.hpp>

TEST_CASE("hello world test for parser") {
    nlang::CharStream::Create<nlang::StringCharStream>("kek");
}