//
// Created by ilya on 25.11.2019.
//

#ifndef NLANG_BYTECODE_HPP
#define NLANG_BYTECODE_HPP

#include <vector>
#include <cstdint>

namespace nlang {

class Bytecode {
    std::vector<uint8_t> code;

    void Append(uint8_t byte);
    void AppendMany(uint8_t bytes...);

    [[nodiscard]]
    size_t GetLength() const;

};

}

#endif //NLANG_BYTECODE_HPP
