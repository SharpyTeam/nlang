//
// Created by ilya on 08.11.2019.
//

#include <scanner.hpp>
#include <version.hpp>

#include <iostream>
#include <string>

void printTokens(const std::string &input) {
    nlang::Scanner sc(input);
    for (auto &token : sc.GetTokens()) {
        std::cout << "'" << token.value << "'" << " [" << static_cast<int>(token.token)  << "]:"
            << token.row << ":" << token.column << std::endl;
    }
}

int main(int argc, char *argv[]) {
    std::cout << "nlang " NLANG_VERSION " (" NLANG_BUILD_GIT_REVISION ", " NLANG_BUILD_PROCESSOR ",  " __DATE__ " " __TIME__  ")" << std::endl;
    std::cout << "[" << ((NLANG_BUILD_COMPILER_ID == "GNU") ? "GCC" : NLANG_BUILD_COMPILER_ID) << " " NLANG_BUILD_COMPILER_VERSION << "]" << std::endl;
    if (argc > 1) {
        if (std::string(argv[1]) == std::string("--extract-tokens")) {
            if (argc < 3) {
                std::cout << "No input was provided.";
                exit(-1);
            }

            std::string input(argv[2]);
            printTokens(input);
        } else {
            std::cout << "Unknown argument '" << std::string(argv[1]) << "'." << std::endl;
        }
    } else {
        std::cout << "Interactive mode. Type 'exit' to exit.\nThis mode only prints tokens at the moment." << std::endl;
        std::string input;
        std::cout << "> " << std::flush;

        while (std::getline(std::cin, input) && input != "exit") {
            printTokens(input);
            std::cout << "> " << std::flush;
        }
    }
}