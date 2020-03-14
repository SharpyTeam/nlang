#pragma once

#include "module.hpp"

#include <utils/holder.hpp>
#include <parser/scanner.hpp>
#include <parser/parser.hpp>
#include <parser/char_stream.hpp>
#include <parser/token_stream.hpp>

namespace nlang {

class IModuleLoader {
public:
    virtual Holder<Module> LoadModule(const std::string& name);
    virtual Holder<Module> LoadMainModule();

    virtual ~IModuleLoader() = default;
};

class SimpleModuleLoader : public IModuleLoader {
public:
    SimpleModuleLoader(const std::string& text)
        : text(text)
    {}

    Holder<Module> LoadMainModule() override {
        auto parser = Parser::New(Scanner::New(TokenStream::New(StringCharStream::New(text))));
        return Module::New(parser->ParseModule());
    }

    Holder<Module> LoadModule(const std::string &name) override {
        throw std::runtime_error("Not implemented");
    }

private:
    const std::string text;
};

}