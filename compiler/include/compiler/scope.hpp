#pragma once

#include <compiler/registers_shape.hpp>
#include <compiler/bytecode.hpp>

#include <common/ast.hpp>

#include <utils/pointers.hpp>

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace nlang {


class Scope : public ast::INode::IMeta {
public:
    enum class StorageType {
        Register,
        Context,
    };

    struct Location {
        StorageType storage_type;
        union {
            bytecode::Register reg;
            bytecode::ContextDescriptor context_descriptor;
        };
    };

    Scope(Scope* parent, bool weak)
        : parent(parent)
        , weak(weak)
        , registers_shape(weak ? parent->registers_shape : IntrusivePtr<RegistersShape>(new RegistersShape))
        , bytecode_generator(weak ? parent->bytecode_generator : IntrusivePtr<bytecode::BytecodeGenerator>(new bytecode::BytecodeGenerator))
    {}

    Scope(const Scope&) = delete;
    Scope(Scope&&) = delete;
    Scope& operator=(const Scope&) = delete;
    Scope& operator=(Scope&&) = delete;
    virtual ~Scope() = default;

    // building

    void Touch(const UString& name, bool move_to_context = false) {
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

    void DeclareArgument(const UString& name, int32_t index) {
        if (weak) {
            throw; // weak scope can't have args
        }
        auto p = values.try_emplace(name, StorageType::Register);
        if (!p.second) {
            throw; // redeclaration
        }
        registers_shape->StoreArgument(name, index);
    }

    void DeclareLocal(const UString& name) {
        auto p = values.try_emplace(name, StorageType::Register);
        if (!p.second) {
            throw; // redeclaration
        }
        registers_shape->StoreLocal(name);
    }

    // using

    Location GetLocation(const UString& name) const {
        auto get_real_context_index = [&](const std::unordered_map<UString, StorageType>& m, const std::unordered_map<UString, StorageType>::const_iterator& iter) {
            int32_t index = 0;
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
        int32_t depth = 0;
        const Scope* current = this;
        while (current) {
            if (auto it = current->values.find(name); it != current->values.end()) {
                location.storage_type = it->second;
                if (it->second == StorageType::Register) {
                    if (current->registers_shape != registers_shape) {
                        throw;
                    }
                    location.reg = registers_shape->GetIndex(name);
                } else {
                    location.context_descriptor = { get_real_context_index(current->values, it), depth };
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

    int32_t GetCount(StorageType storage_type) const {
        int32_t count = 0;
        for (auto& [k, v] : values) {
            count += v == storage_type;
        }
        return count;
    }

    IntrusivePtr<RegistersShape> GetRegistersShape() const {
        return registers_shape;
    }

    IntrusivePtr<bytecode::BytecodeGenerator> GetBytecodeGenerator() const {
        return bytecode_generator;
    }

private:
    Scope* parent;
    bool weak;
    IntrusivePtr<RegistersShape> registers_shape;
    IntrusivePtr<bytecode::BytecodeGenerator> bytecode_generator;
    std::unordered_map<UString, StorageType> values;
};


}