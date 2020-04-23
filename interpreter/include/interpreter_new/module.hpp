#pragma once

#include <utils/holder.hpp>
#include <common/ast/ast.hpp>

namespace nlang {

class Module {
public:

    static Holder<Module> New(Holder<ast::Module>&& ast) {
        return Holder<Module>(new Module(std::move(ast)));
    }

private:
    Module(Holder<ast::Module>&& ast)
        : ast(std::move(ast))
    {}

    Holder<ast::Module> ast;
};

}