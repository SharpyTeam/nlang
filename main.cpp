//
// Created by ilya on 08.11.2019.
//

#include <scanner.hpp>
#include <version.hpp>
#include <parser.hpp>
#include <char_stream.hpp>

#include <iostream>
#include <string>
#include <ast_interpreter.hpp>
#include <interpreter/heap.hpp>
#include <library/thread/thread_pool.hpp>

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

void print_heap_state(nlang::Heap& heap) {
    std::cout << "----------" << std::endl;
    std::cout << "Pages: " << heap.GetPagesCount() << std::endl;
    std::cout << "Occupied slots: " << heap.GetOccupiedSlotsCount() << std::endl;
    std::cout << "Free slots: " << heap.GetFreeSlotsCount() << std::endl;
    std::cout << "----------" << std::endl;
}

int main(int argc, char *argv[]) {
    nlang::Heap heap;

    std::vector<nlang::ObjectSlot*> slots;
    for (int i = 1; i < 151346; ++i) {
        slots.emplace_back(heap.Store(nlang::Address(i)));
    }

    print_heap_state(heap);

    heap.PreGC();
    heap.MarkOrUpdateAchievableSlot(slots[0]);
    heap.MarkOrUpdateAchievableSlot(slots[slots.size() - 1]);
    heap.PostGC();

    print_heap_state(heap);
    std::cout << int(slots[0]->Get()) << " " << int(slots[slots.size() - 1]->Get()) << " " << slots[0] << " " << slots[slots.size() - 1] << std::endl;

    heap.PreGC();
    heap.MarkOrUpdateAchievableSlot(slots[0]);
    heap.MarkOrUpdateAchievableSlot(slots[slots.size() - 1]);
    heap.PostGC();

    print_heap_state(heap);
    std::cout << int(slots[0]->Get()) << " " << int(slots[slots.size() - 1]->Get()) << " " << slots[0] << " " << slots[slots.size() - 1] << std::endl;

    heap.PreGC();
    heap.PostGC();

    print_heap_state(heap);

    heap.PreGC();
    heap.PostGC();

    print_heap_state(heap);


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
        std::cout << "[" << ((NLANG_BUILD_COMPILER_ID == "GNU") ? "GCC" : NLANG_BUILD_COMPILER_ID) << " " NLANG_BUILD_COMPILER_VERSION << "]" << std::endl;

        std::cout << "Interactive mode. Type 'exit' to exit.\nThis mode only prints tokens at the moment." << std::endl;
        std::string input;
        std::cout << "> " << std::flush;

        while (std::getline(std::cin, input) && input != "exit") {
            print(input);
            std::cout << "> " << std::flush;
        }
    }
}