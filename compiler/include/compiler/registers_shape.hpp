#pragma once

#include <compiler/bytecode.hpp>

#include <utils/pointers.hpp>
#include <utils/strings.hpp>

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>

namespace nlang {


class RegistersShape : public IntrusivePtrRefCounter {
public:
    RegistersShape() = default;
    RegistersShape(const RegistersShape&) = delete;
    RegistersShape(RegistersShape&&) = delete;
    RegistersShape& operator=(const RegistersShape&) = delete;
    RegistersShape& operator=(RegistersShape&&) = delete;

    // building

    void StoreLocal(const UString& name) {
        auto p = registers.try_emplace(name, locals_count);
        if (!p.second) {
            throw; // redeclaration
        }
        ++locals_count;
    }

    void StoreArgument(const UString& name, int32_t index) {
        auto p = registers.try_emplace(name, -index - 1);
        if (!p.second) {
            throw; // redeclaration
        }
        arguments_count = arguments_count > index ? arguments_count : index + 1;
    }

    void RemoveName(const UString& name) {
        if (auto it = registers.find(name); it != registers.end()) {
            int32_t removed_index = it->second;
            registers.erase(it);
            if (removed_index >= 0) {
                --locals_count;
                for (auto& [name, index] : registers) {
                    if (index > removed_index) {
                        --index;
                    }
                }
            }
            return;
        }
        throw; // no such name
    }

    // using

    bytecode::RegistersRange LockRegisters(int32_t count) {
        bytecode::RegistersRange current_range { locals_count, 0 };
        for (int32_t i = 0; i < anonymous_registers.size(); ++i) {
            if (anonymous_registers[i]) {
                current_range = { locals_count + i + 1, 0 };
            } else {
                ++current_range.count;
            }
            if (current_range.count == count) {
                break;
            }
        }

        if (current_range.count < count) {
            anonymous_registers.resize(current_range.first - locals_count + count);
            current_range.count = count;
        }

        for (int32_t i = 0; i < count; ++i) {
            anonymous_registers[current_range.first - locals_count + i] = true;
        }

        return current_range;
    }

    void ReleaseRegisters(bytecode::RegistersRange range) {
        for (int32_t i = 0; i < range.count; ++i) {
            anonymous_registers[range.first - locals_count + i] = false;
        }
    }

    int32_t GetRegistersCount() const {
        return locals_count + anonymous_registers.size();
    }

    int32_t GetArgumentsCount() const {
        return arguments_count;
    }

    int32_t GetIndex(const UString& name) const {
        return registers.at(name);
    }

    void Declare(const UString& name) {
        declared_registers.emplace(name);
    }

    bool IsDeclared(const UString& name) const {
        return declared_registers.count(name);
    }

private:
    std::unordered_map<UString, int32_t> registers;
    std::unordered_set<UString> declared_registers;
    int32_t arguments_count = 0;
    int32_t locals_count = 0;
    std::vector<bool> anonymous_registers;
};


}