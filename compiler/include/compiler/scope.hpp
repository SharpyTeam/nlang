#pragma once

#include <common/handles/handle.hpp>
#include <common/objects/string.hpp>
#include <common/ast/ast.hpp>
#include <common/bytecode.hpp>

#include <utils/shared_ptr.hpp>

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace nlang {


class RegistersShape {
public:
    struct RegistersRange {
        int32_t index;
        uint32_t count;
    };

    RegistersShape() = default;
    RegistersShape(const RegistersShape&) = delete;
    RegistersShape(RegistersShape&&) = delete;
    RegistersShape& operator=(const RegistersShape&) = delete;
    RegistersShape& operator=(RegistersShape&&) = delete;

    // building

    void StoreLocal(Handle<String> name) {
        auto p = registers.try_emplace(name, locals_count);
        if (!p.second) {
            throw; // redeclaration
        }
        ++locals_count;
    }

    void StoreArgument(Handle<String> name, int32_t index) {
        auto p = registers.try_emplace(name, -index - 1);
        if (!p.second) {
            throw; // redeclaration
        }
        arguments_count = arguments_count > index ? arguments_count : index + 1;
    }

    void RemoveName(Handle<String> name) {
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

    RegistersRange LockRegisters(uint32_t count) {
        RegistersRange current_range { (int32_t)locals_count, 0 };
        for (size_t i = 0; i < anonymous_registers.size(); ++i) {
            if (anonymous_registers[i]) {
                current_range = { (int32_t)(locals_count + i + 1), 0 };
            } else {
                ++current_range.count;
            }
            if (current_range.count == count) {
                break;
            }
        }

        if (current_range.count < count) {
            anonymous_registers.resize(current_range.index - locals_count + count);
            current_range.count = count;
        }

        for (size_t i = 0; i < count; ++i) {
            anonymous_registers[current_range.index - locals_count + i] = true;
        }

        return current_range;
    }

    void ReleaseRegisters(RegistersRange range) {
        for (size_t i = 0; i < range.count; ++i) {
            anonymous_registers[range.index - locals_count + i] = false;
        }
    }

    size_t GetRegistersCount() const {
        return locals_count + anonymous_registers.size();
    }

    size_t GetArgumentsCount() const {
        return arguments_count;
    }

    int32_t GetIndex(Handle<String> name) const {
        return registers.at(name);
    }

    void Declare(Handle<String> name) {
        declared_registers.emplace(name);
    }

    bool IsDeclared(Handle<String> name) const {
        return declared_registers.count(name);
    }

private:
    std::unordered_map<Handle<String>, int32_t> registers;
    std::unordered_set<Handle<String>> declared_registers;
    size_t arguments_count = 0;
    size_t locals_count = 0;
    std::vector<bool> anonymous_registers;
};


class Scope : public ast::INode::IMeta {
public:
    enum class StorageType {
        Register,
        Context,
    };

    struct Location {
        struct ContextDescriptor {
            uint32_t index;
            uint32_t depth;
        };

        StorageType storage_type;
        union {
            int32_t register_index;
            ContextDescriptor context_descriptor;
        };
    };

    Scope(Scope* parent, bool weak)
        : parent(parent)
        , weak(weak)
        , registers_shape(weak ? parent->registers_shape : MakeShared<RegistersShape>())
        , bytecode_generator(weak ? parent->bytecode_generator : MakeShared<bytecode::BytecodeGenerator>())
    {}

    Scope(const Scope&) = delete;
    Scope(Scope&&) = delete;
    Scope& operator=(const Scope&) = delete;
    Scope& operator=(Scope&&) = delete;
    virtual ~Scope() = default;

    // building

    void Touch(Handle<String> name, bool move_to_context = false) {
        if (auto it = values.find(name); it != values.end()) {
            if (move_to_context && it->second == StorageType::Register) {
                registers_shape->RemoveName(name);
                it->second = StorageType::Context;
            }
            return;
        }
        if (!parent) {
            throw; // no such name
        }
        parent->Touch(name, move_to_context || !weak);
    }

    void DeclareArgument(Handle<String> name, int32_t index) {
        if (weak) {
            throw; // weak scope can't have args
        }
        auto p = values.try_emplace(name, StorageType::Register);
        if (!p.second) {
            throw; // redeclaration
        }
        registers_shape->StoreArgument(name, index);
    }

    void DeclareLocal(Handle<String> name) {
        auto p = values.try_emplace(name, StorageType::Register);
        if (!p.second) {
            throw; // redeclaration
        }
        registers_shape->StoreLocal(name);
    }

    // using

    Location GetLocation(Handle<String> name) const {
        auto get_real_context_index = [&](const std::unordered_map<Handle<String>, StorageType>& m, const std::unordered_map<Handle<String>, StorageType>::const_iterator& iter) {
            size_t index = 0;
            for (auto it = m.begin(); it != m.end(); ++it) {
                if (it == iter) {
                    return index;
                }
                if (it->second == StorageType::Context) {
                    ++index;
                }
            }
            throw;
        };

        Location location;
        uint32_t depth = 0;
        const Scope* current = this;
        while (current) {
            if (auto it = current->values.find(name); it != current->values.end()) {
                location.storage_type = it->second;
                if (it->second == StorageType::Register) {
                    if (current->registers_shape != registers_shape) {
                        throw;
                    }
                    location.register_index = registers_shape->GetIndex(name);
                } else {
                    location.context_descriptor = { (uint32_t)get_real_context_index(current->values, it), depth };
                }
                return location;
            }
            if (current->GetCount(StorageType::Context)) {
                ++depth;
            }
            current = current->parent;
        }

        throw; // not found
    }

    size_t GetCount(StorageType storage_type) const {
        size_t count = 0;
        for (auto& [k, v] : values) {
            count += v == storage_type;
        }
        return count;
    }

    SharedPtr<RegistersShape> GetRegistersShape() const {
        return registers_shape;
    }

    SharedPtr<bytecode::BytecodeGenerator> GetBytecodeGenerator() const {
        return bytecode_generator;
    }

private:
    Scope* parent;
    bool weak;
    SharedPtr<RegistersShape> registers_shape;
    SharedPtr<bytecode::BytecodeGenerator> bytecode_generator;
    std::unordered_map<Handle<String>, StorageType> values;
};


}