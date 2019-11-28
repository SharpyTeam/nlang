//
// Created by ilya on 09.11.2019.
//

#ifndef NLANG_VM_HPP
#define NLANG_VM_HPP

#include <bytecode.hpp>

namespace nlang {

class VM {
    enum class RunResult {
        OK,
        COMPILATION_ERROR,
        RUNTIME_ERROR
    };



    RunResult Run(const Bytecode &bytecode);

};

}


#endif //NLANG_VM_HPP
