#include <version/version.hpp>

#include <parser/scanner.hpp>
#include <parser/char_stream.hpp>

#include <interpreter/interpreter.hpp>
#include <interpreter/native_function.hpp>
#include <interpreter/bytecode_function.hpp>
#include <interpreter/function.hpp>

#include <iostream>
#include <string>
#include <cstring>
#include <common/bytecode.hpp>

void print(const std::string &input) {
    using namespace nlang;
    auto scanner = Scanner::New(TokenStream::New(StringCharStream::New(input)));

    for (auto& token = scanner->NextToken(); token.token != nlang::Token::THE_EOF; token = scanner->NextToken()) {
        std::cout << "'" << token.text << "'" << " [" << std::string(nlang::TokenUtils::GetTokenName(token.token)) << ", " << static_cast<int>(token.token) << "]:"
            << token.row << ":" << token.column << std::endl;
    }
}



int main(int argc, char *argv[]) {
    using namespace nlang;

    Heap heap;
    Handle<Closure> print_f = Closure::New(&heap, NativeFunction::New(&heap, [&](Thread* thread, Handle<Context>, size_t args_count, const Handle<Value>* args) {
        std::cout << args[0].As<Number>()->Value() << std::endl;
        return Null::New();
    }));

    Instruction call { Opcode::Call, { 0 } };
    call.operand.registers_range = { 0, 1 };

    Handle<Closure> bc = Closure::New(&heap, BytecodeFunction::New(&heap, 1, 3, {
        { Opcode::RegToAcc, { -2 } },
        { Opcode::Mul, { -1 } },
        { Opcode::AccToReg, { 0 } },
        { Opcode::RegToAcc, { -3 } },
        call,
        { Opcode::RegToAcc, { -2 } },
        { Opcode::Div, { -1 } },
        { Opcode::Ret, {} }
    }));

    std::vector<Handle<Value>> args { print_f, Number::New(3), Number::New(4) };
    Thread t(&heap, bc, args.size(), args.data());
    std::cout << t.Join().As<Number>()->Value() << std::endl;

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