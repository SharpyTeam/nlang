#include <version/version.hpp>

#include <common/bytecode.hpp>

#include <interpreter/thread.hpp>

#include <parser/scanner.hpp>
#include <parser/parser.hpp>

#include <common/objects/string.hpp>

#include <compiler/semantic_analyser.hpp>
#include <compiler/compiler.hpp>

#include <iostream>
#include <string>
#include <cstring>

void print(const std::string &input) {
    using namespace nlang;
    Heap heap;
    auto scanner = Scanner::New(TokenStream::New(&heap, String::New(&heap, input)));

    for (auto& token = scanner->NextToken(); token.token != nlang::Token::THE_EOF; token = scanner->NextToken()) {
        std::string s;
        token.text->GetRawString().toUTF8String(s);
        std::cout << "'" << s << "'" << " [" << std::string(nlang::TokenUtils::GetTokenName(token.token)) << ", " << static_cast<int>(token.token) << "]:"
            << token.row << ":" << token.column << std::endl;
    }
}



int main(int argc, char *argv[]) {
    using namespace nlang;

    Heap heap;
    auto parser = Parser::New(Scanner::New(TokenStream::New(&heap, String::New(&heap,
R"(
fn foo(d) {
    let a = d * 2
    fn bar(d) {
        return b + d
    }
    let b = a - 1
    return bar
}
foo(13)(5)
)"))));

    auto ast = parser->ParseModule();

    SemanticAnalyser semantic_analyzer;
    semantic_analyzer.Process(*ast);

    Compiler compiler;
    auto module = compiler.Compile(&heap, *ast);

    std::cout << bytecode::BytecodeDisassembler::Disassemble(module.As<BytecodeFunction>()->bytecode_chunk) << std::endl << std::endl;
    std::cout << bytecode::BytecodeDisassembler::Disassemble(module.As<BytecodeFunction>()->bytecode_chunk.constant_pool[0].As<BytecodeFunction>()->bytecode_chunk) << std::endl << std::endl;
    std::cout << bytecode::BytecodeDisassembler::Disassemble(module.As<BytecodeFunction>()->bytecode_chunk.constant_pool[0].As<BytecodeFunction>()->bytecode_chunk.constant_pool[0].As<BytecodeFunction>()->bytecode_chunk) << std::endl << std::endl;

    Thread thread(&heap, Closure::New(&heap, Handle<Context>(), module), 0, nullptr);
    std::cout << thread.Join().As<Number>()->Value() << std::endl;

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