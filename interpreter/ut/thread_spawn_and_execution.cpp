#include <catch2/catch.hpp>

#include <interpreter/interpreter.hpp>
#include <interpreter/function.hpp>

#include <iostream>

TEST_CASE("thread spawn, native and interpreted function execution") {
    // TODO fix this test

    /*nlang::Environment environment;

    double ctr = 0;

    auto print = nlang::NativeFunction::New(&environment, [&](nlang::NativeFunction::NativeFunctionInfo& info) {
        for (size_t i = 0; i < info.GetArgumentsCount(); ++i) {
            ctr = info.GetArgument(i).As<nlang::Number>()->Value();
        }
    }, 1);

    auto f = nlang::InterpretedFunction::New(&environment, {
        4, 0, 0, 0, 0, 0, 0, 0, 0,
        5, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        2,
    }, 2, 1);

    nlang::Handle<nlang::Value> args[] { nlang::Number::New(123), nlang::Handle<nlang::Value>(print) };

    nlang::Thread thread(&environment, f, args, args + 2);
    thread.Join();

    REQUIRE(ctr == 123);*/
}