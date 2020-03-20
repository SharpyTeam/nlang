#include <version/version.hpp>

#include <parser/scanner.hpp>
#include <parser/char_stream.hpp>

#include <iostream>
#include <string>
#include <cstring>
#include <interpreter/stack_values.hpp>
#include <interpreter/string.hpp>

void print(const std::string &input) {
    using namespace nlang;
    auto scanner = Scanner::New(TokenStream::New(StringCharStream::New(input)));

    for (auto token = scanner->NextToken(); token->token != nlang::Token::THE_EOF; token = scanner->NextToken()) {
        std::cout << "'" << token->text << "'" << " [" << std::string(nlang::TokenUtils::GetTokenName(token->token)) << ", " << static_cast<int>(token->token) << "]:"
            << token->row << ":" << token->column << std::endl;
    }
}



int main(int argc, char *argv[]) {
    using namespace nlang;

    Heap heap;

    auto h = Int32::New(123);
    auto h2 = Null::New();
    auto h3 = String::New(heap, "123");
    std::cout << std::to_string(h->IsBool()) << std::endl;
    std::cout << std::to_string(h->IsInt32()) << std::endl;
    std::cout << std::to_string(h->IsPointer()) << std::endl;
    std::cout << std::to_string((uint8_t) h.GetType()) << std::endl;

    std::cout << std::to_string(h2->IsBool()) << std::endl;
    std::cout << std::to_string(h2->IsNull()) << std::endl << std::endl;

    std::cout << h3.Is<Int32>() << std::endl;
    std::cout << h3.Is<StackValue>() << std::endl;
    std::cout << h3.Is<HeapValue>() << std::endl;
    std::cout << h3.Is<String>() << std::endl;
    std::cout << std::to_string((uint8_t)h3.GetType()) << std::endl;


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