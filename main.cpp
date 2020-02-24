#include "version.hpp"

#include <parser/scanner.hpp>
#include <parser/parser.hpp>
#include <parser/char_stream.hpp>
#include <utils/page.hpp>
#include <interpreter/heap.hpp>
#include <interpreter/ast_interpreter.hpp>
#include <interpreter/value.hpp>
#include <interpreter/object.hpp>
#include <interpreter/string.hpp>

#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <interpreter/interpreter.hpp>

void print(const std::string &input) {
    auto sc = nlang::Scanner::Create(nlang::CharStream::Create<nlang::StringCharStream>(input));
    auto mark = sc->Mark();
    for (auto token = sc->NextToken(); token.token != nlang::Token::THE_EOF; token = sc->NextToken()) {
        std::cout << "'" << token.source << "'" << " [" << std::string(nlang::TokenUtils::TokenToString(token.token)) << ", " << static_cast<int>(token.token)  << "]:"
            << token.row << ":" << token.column << std::endl;
    }
    mark.Apply();
    auto parser = nlang::Parser::Create(sc);
    nlang::ASTStringifier stringifier;
    parser->ParseFile()->Accept(stringifier);
    std::cout << stringifier.ToString() << std::endl;
}



int main(int argc, char *argv[]) {
    using namespace nlang;

    Heap heap;

    auto object_handle = heap.Store(new Object);

    std::cout << object_handle.Is<StackValue>() << std::endl;
    std::cout << object_handle.Is<HeapValue>() << std::endl;
    std::cout << object_handle.Is<Object>() << std::endl;

    Handle<Value> handle = Number::New(12351);
    std::cout << handle.Is<Number>() << std::endl;
    std::cout << handle.Is<Int32>() << std::endl;
    std::cout << handle.As<Number>()->Value() << std::endl;
    handle = Int32::New(555);
    std::cout << handle.Is<Number>() << std::endl;
    std::cout << handle.Is<Int32>() << std::endl;
    Handle<Int32> int32_handle = handle.As<Int32>();
    std::cout << int32_handle->Value() << std::endl;
    std::cout << int32_handle.Is<Object>() << std::endl;

    Handle<String> result = String::New(heap, u"TEST🌲, ");//, std::string("kek, "), std::u32string(U"чебурек"));
    typedef deletable_facet<std::codecvt<char32_t, char, std::mbstate_t>> facet_u32;
    std::wstring_convert<facet_u32, char32_t> conv;
    std::cout << conv.to_bytes(result->GetRawString()) << std::endl;

    std::string a("w");
    std::u32string b(U"й");
    std::u16string c(u"m");
    Handle<String> v = String::New(heap, a, b, c);
    std::cout << "Length: " << v->GetLength() << std::endl;


    if (argc == 2) {
        auto scanner = nlang::Scanner::Create(nlang::CharStream::Create<nlang::FileCharStream>(argv[1]));
        auto parser = nlang::Parser::Create(scanner);
        nlang::ast_interpreter::Interpreter interpreter(parser->ParseFile());
        interpreter.Run();
        return 0;
    }

    if (argc > 1) {
        if (std::string(argv[1]) == "print-ast") {
            if (argc < 3) {
                std::cout << "No input was provided.";
                exit(-1);
            }

            auto scanner = nlang::Scanner::Create(nlang::CharStream::Create<nlang::FileCharStream>(argv[2]));
            auto parser = nlang::Parser::Create(scanner);
            nlang::ASTStringifier stringifier;
            parser->ParseFile()->Accept(stringifier);
            std::cout << stringifier.ToString() << std::endl;
        } else if (std::string(argv[1]) == std::string("extract-tokens")) {
            if (argc < 3) {
                std::cout << "No input was provided.";
                exit(-1);
            }

            std::string input(argv[2]);
            print(input);
        } else {
            std::cout << "Unknown argument '" << std::string(argv[1]) << "'." << std::endl;
        }
    } else {
        std::cout << "nlang " NLANG_VERSION " (" NLANG_BUILD_GIT_REVISION ", " NLANG_BUILD_PROCESSOR ",  " __DATE__ " " __TIME__  ")" << std::endl;
        std::cout << "[" << ((std::strcmp(NLANG_BUILD_COMPILER_ID, "GNU") == 0) ? "GCC" : NLANG_BUILD_COMPILER_ID) << " " NLANG_BUILD_COMPILER_VERSION << "]" << std::endl;

        std::cout << "Interactive mode. Type 'exit' to exit.\nThis mode only prints tokens at the moment." << std::endl;
        std::string input;
        std::cout << "> " << std::flush;

        while (std::getline(std::cin, input) && input != "exit") {
            print(input);
            std::cout << "> " << std::flush;
        }
    }
}