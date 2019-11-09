//
// Created by ilya on 08.11.2019.
//

#include <compiler.hpp>
#include <version.hpp>

#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << "nlang " NLANG_VERSION " (" NLANG_BUILD_GIT_REVISION ", built on " __DATE__ " " __TIME__  ")" << std::endl;
    std::cout << "[" << ((NLANG_BUILD_COMPILER_ID == "GNU") ? "GCC" : NLANG_BUILD_COMPILER_ID) << " " NLANG_BUILD_COMPILER_VERSION << "]";
}