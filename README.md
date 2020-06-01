# Nlang Programming Language

[![Build Status](https://travis-ci.org/SharpyTeam/nlang.svg?branch=master)](https://travis-ci.org/SharpyTeam/nlang)
[![Build status](https://ci.appveyor.com/api/projects/status/93jtkv03wbb6ao8i?svg=true)](https://ci.appveyor.com/project/GoltikRee/nlang)
[![CodeFactor](https://www.codefactor.io/repository/github/sharpyteam/nlang/badge)](https://www.codefactor.io/repository/github/sharpyteam/nlang)

# Documentation

[Tutorial](docs/tutorial.md)

[Developer Guide](docs/developer-guide.md)

For instructions on how to build the nlang interpreter, see the sections below.

# Build environment requirements

In order to build nlang interpreter you need to install the following packages/libraries:
- [CMake](https://cmake.org/)
- [ICU](http://site.icu-project.org/home) (with `l18n` and `uc` libraries)
- [protobuf](https://developers.google.com/protocol-buffers)
- [Catch2](https://github.com/catchorg/Catch2)

Optional requirements:
- [Git](https://git-scm.com/) - for including Git revision in build
- [JDK 11](https://www.oracle.com/java/technologies/javase-jdk11-downloads.html) - for building syntax highlighting plugin for IntelliJ
- [backwarp-cpp](https://github.com/bombela/backward-cpp)  with `libdw`, `libdwarf` and `libbfd` libraries - for printing stack traces after crashes

You also need to install any version of one of these compilers with C++17 and threads support:
- GCC
- Clang
- MSVC

**NOTE**: We highly recommend installing the latest version of Visual Studio if you choose to use MSVC compiler.


If you want to use sanitizers (e.g. `asan` or `ubsan`), you need to make sure that you have installed the corresponding library.

If you are building on Windows, you can install [vcpkg](https://github.com/microsoft/vcpkg) to install the dependencies. 

# Building
**ONLY FOR WINDOWS**:

```bat
  ; Activate MS VS build environment
  "C:\path\to\the\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

Create a directory for the build:
```bash
mkdir build
cd build
```

Run CMake to generate build files:
```bash
cmake .. -DCMAKE_BUILD_TYPE=$CONFIGURATION
```
where `$CONFIGURATION` is `Debug` or `Release`.

To build the IntelliJ plugin you need to pass `-DENABLE_JNI_TOOLS=TRUE` argument in the command above.

To enable sanitizers you need to pass `-DENABLE_SANITIZERS=TRUE` argument in the command above.
 
Run the build:
```bash
cmake --build . --config $CONFIGURATION
```
where `$CONFIGURATION` is that you used when running CMake before that.

Run tests:
```bash
ctest --output-on-failure
```

You can find built nlang binary in the build directory.

# Building IntelliJ plugin

**NOTE**: You must build the project with `-DENABLE_JNI_TOOLS=TRUE` passed to CMake before building the plugin.

Go to plugin directory:
```bash
cd ide/intellij
```

Execute `buildPlugin` Gradle task to build the plugin:
```
./gradlew buildPlugin
```

You will find the plugin zip file in `build/distributions` directory.

To install the plugin, open any IntelliJ IDE (IntelliJ IDEA, CLion, PyCharm, ...), and follow the [instructions](https://www.jetbrains.com/help/idea/managing-plugins.html#install_plugin_from_disk).

# License
This project is licensed under the terms of the MIT license.

# Contributing

All contributions are welcome! If you want to make a contribution, please open a PR on a corresponding page.