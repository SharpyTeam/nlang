//
// Created by ilya on 19.11.2019.
//

#ifndef NLANG_OPCODES_HPP
#define NLANG_OPCODES_HPP

#include <string>
#include <unordered_map>

namespace nlang {

#define OPCODES_LIST                    \
T(NOP, 1, "Nop")                        \
T(INVALID, 2, "")                       \
T(HALT, 3, "Hlt")                       \
T(RETURN, 4, "Ret")                     \
T(BIN_ADD, 5, "Add")                    \
T(BIN_SUB, 6, "Sub")                    \
T(BIN_MUL, 7, "Mul")                    \
T(BIN_DIV, 8, "Div")                    \
T(BIN_FLOOR_DIV, 9, "FloorDiv")         \
T(BIN_MOD, 10, "Mod")                   \
T(BIT_LSH, 11, "Lsh")                   \
T(BIT_RSH, 12, "Rsh")                   \
T(BIT_OR, 13, "BitOr")                  \
T(BIT_AND, 14, "BitAnd")                \
T(BIT_XOR, 15, "BitXor")                \
T(STORE_STR_LITERAL, 16, "StStrL")      \
T(STORE_INT_LITERAL, 17, "StIntL")      \
T(STORE_DBL_LITERAL, 18, "StDblL")      \
T(CALL, 19, "Call")                     \
T(MOV, 20, "Mov")                       \
T(ALOAD, 21, "Aload")                   \
T(ASTORE, 22, "Astore")                 \


enum class OpCodes {
#define T(opcode, byte_repr, text_repr) opcode = byte_repr,
    OPCODES_LIST
#undef T
};

inline static std::unordered_map<OpCodes, std::string> opcodes_to_strings = {
#define T(opcode, byte_repr, text_repr) {OpCodes::opcode, std::string(text_repr)},
        OPCODES_LIST
#undef T
};

inline static std::unordered_map<OpCodes, std::string> opcodes_to_bytes = {
#define T(opcode, byte_repr, text_repr) {OpCodes::opcode, std::string(text_repr)},
        OPCODES_LIST
#undef T
};

inline static std::unordered_map<OpCodes, std::string> bytes_to_opcodes = {
#define T(opcode, byte_repr, text_repr) {OpCodes::opcode, std::string(text_repr)},
        OPCODES_LIST
#undef T
};


}

#endif //NLANG_OPCODES_HPP
