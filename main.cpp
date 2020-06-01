/**
 * Main interpreter file
 */


#include <version/version.hpp>

#include <compiler/bytecode.hpp>

#include <interpreter/thread.hpp>
#include <interpreter/native_function.hpp>

#include <parser/scanner.hpp>
#include <parser/parser.hpp>

#include <interpreter/objects/string.hpp>

#include <compiler/semantic_analyser.hpp>
#include <compiler/compiler.hpp>

#include <iostream>
#include <string>
#include <cstring>

/**
 * Splits up the source code string into tokens and prints them
 * @param input The input string with source code
 */
void print(const nlang::UString &input) {
    using namespace nlang;
    Heap heap;
    auto scanner = Scanner::New(TokenStream::New(input));

    for (auto& token = scanner->NextToken(); token.token != nlang::Token::THE_EOF; token = scanner->NextToken()) {
        std::cout << "'" << token.text << "'" << " [" << nlang::TokenUtils::GetTokenName(token.token) << ", " << static_cast<int>(token.token) << "]:"
            << token.row << ":" << token.column << std::endl;
    }
}

/**
 * The main entry function
 * @param argc Number of command-line arguments
 * @param argv Command-line arguments
 * @return Nothing
 */
int main(int argc, char *argv[]) {
    using namespace nlang;

    Heap heap;
    auto parser = Parser::New(Scanner::New(TokenStream::New(UString(
R"(
fn fibonacci(n) {
    if (n == 1) {
        return 0
    } else if (n == 2) {
        return 1
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}
fibonacci(10)
)"))));

    auto ast = parser->ParseModule();

    SemanticAnalyser semantic_analyzer;
    semantic_analyzer.Process(*ast);

    Compiler compiler;
    auto module = compiler.Compile(&heap, *ast);

    ast::ASTStringifier a;
    std::cout << a.Stringify(*ast) << std::endl;

    //std::cout << bytecode::BytecodeDisassembler::Disassemble(module.As<BytecodeFunction>()->bytecode_chunk) << std::endl << std::endl;
    //std::cout << bytecode::BytecodeDisassembler::Disassemble(module.As<BytecodeFunction>()->bytecode_chunk.constant_pool[0].As<BytecodeFunction>()->bytecode_chunk) << std::endl << std::endl;
    //std::cout << bytecode::BytecodeDisassembler::Disassemble(module.As<BytecodeFunction>()->bytecode_chunk.constant_pool[0].As<BytecodeFunction>()->bytecode_chunk.constant_pool[0].As<BytecodeFunction>()->bytecode_chunk) << std::endl << std::endl;

    Thread thread(&heap, Closure::New(&heap, Handle<Context>(), module), 0, nullptr);
    std::cout << thread.Join().As<Number>()->Value() << std::endl;
    return 0;
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