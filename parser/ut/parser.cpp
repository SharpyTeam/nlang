#include <catch2/catch.hpp>

#include <iostream>
#include <parser/char_stream.hpp>
#include <parser/scanner.hpp>
#include <parser/parser_new.hpp>

TEST_CASE("parser test") {
    using namespace nlang;

    const std::string source =
R"(fn print_my_name_and_predict_age(first_name: string, last_name: string = 'Smith') : number {
    print(first_name); print(second_name)
    let static_age = 15
    let guessed_age: number
    if (random() > 0.5) {
        guessed_age = random() * 50
    } else if (random() <= 0.2) {
        guessed_age = 5 +
            8 * random()
    } else {
        guessed_age = static_age
    }
    while (true) {
        break;
    }
    return static_age
})";

    auto parser = ParserNew::New(Scanner::New(TokenStream::New(StringCharStream::New(source))));
    auto ast = parser->ParseFunctionDefinitionExpression();

    ASTPrinter printer;
    printer.Visit(*ast);

    REQUIRE(printer.text ==
R"(fn print_my_name_and_predict_age(first_name: string, last_name: string = 'Smith'): number {
    print(first_name)
    print(second_name)
    let static_age = 15.000000
    let guessed_age: number
    if (random() > 0.500000) {
        guessed_age = random() * 50.000000
    } else if (random() <= 0.200000) {
        guessed_age = 5.000000 + 8.000000 * random()
    } else {
        guessed_age = static_age
    }
    while (true) {
        break
    }
    return static_age
}
)"); // NEWLINE here is important
}