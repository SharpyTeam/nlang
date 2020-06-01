# Nlang Developer Guide

## Code Reference

[**Interpreter Source Code Reference**](https://sharpyteam.github.io/nlang-reference/)

## Project Structure

The interpreter is divided into subrpojects (and each of them is a CMake subproject on its own).

[grammar/](../grammar/) - contains nlang grammar in [ANTLR](https://www.antlr.org/) notation.

[common/](../common/) - contains common files to share among different modules (e.g. AST definition).

[deps/](../deps/) - contains external dependencies.

[docs/](../docs/) - contains documentation and this file.

[ide/](../ide/) - contains IntelliJ plugin sources and JNI module that is used by the plugin.

[interpreter/](../interpreter/) - contains virtual machine sources and objects, used only in runtime.

[parser/](../parser/) - contains token scanner and parser.

[samples/](../samples/) - contains sample nlang code.

[utils/](../utils/) - contains lots of common utility files, used in all places of the project.

[Root Directory](../) - contains all the directories, mentioned above, CI/CD, Git and Doxygen settings, main CMake file and license file.

## How it works

The source code of the program being run is split by the scanner into a sequence of tokens, which the parser then uses to build an abstract syntax tree (AST). The AST is then bypassed by the compiler, which then generates a sequence of instructions (bytecode) that is passed to the VM that executes it.

## Grammar

The nlang grammar definition is written in ANTLRv4. The code does not use this grammar file, but it's purpose is to make sure that the parser and scanner works correctly (by comparing its output to ANLR tool output).

## Scanner

The scanner uses regular expressions to split the source code into tokens.
For example, `var a = 1` string will be splitter into `let` (`TOKEN::LET`), ` ` (`TOKEN::SPACE`), `a` (`TOKEN::IDENTIFIER`), ` ` (`TOKEN::SPACE`), `=` (`TOKEN::ASSIGN`), ` ` (`TOKEN::SPACE`), `1` (`TOKEN::NUMBER`).

## Parser

The parser is a recursive-descent parser with lookahead, which creates an AST from the sequence of tokens.

## Compiler

The compiler converts the AST to a sequence of instructions, recursively descending from the top of the tree. The nlang interpreter uses visitor pattern to traverse all AST nodes, which makes it easy to add new syntactic constructs with minimal changes to existing code. For an AST vertex, a function is called that generates the necessary instructions, while if there are references to other nodes (for example, for the if construct, this is its body), and in the desired order, calls functions that process nodes that the current node has a reference to. When a construction meets a context, such as when a block in curly brackets or a function meets, the handler function calls the corresponding functions to generate instructions that control the context.

## Virtual Machine

The nlang VM is register-based, meaning that registers are manipulated during bytecode execution. Code execution is reduced to iterating through an array of instructions and performing the necessary actions depending on the current instruction. The VM has a stack, which is a sequence of so-called frames that are used to track the current context. It also stores references to local variables for the current context, references to arguments passed when calling the current function, and so on. Attempting to return from the only remaining context is equivalent to shutting down the program.

## Common stuff

### Handles
The Handle class is an analog of `std::shared_ptr` from STL C++. it is used for creating links for shared ownership of internal interpreter objects.

### Strings
`UString` is an extension of ICU's `UnicodeStrinhg`. The interpreter uses it as low-level string storage.
`String` is an extension of `UString`

### Heap
The interpreter uses it's own heap, which is similar to the V8's heap. The heap is a sequence of typed pages that in turn represent a sequence of slots that store objects or pointers to them, with the ability to mark slots (for the garbage collector). Memory is allocated by system pages and is used for storing all objects during parsing, compilation, and code interpretation.

### Garbage collector (GC)
The garbage collector uses the standard mark-sweep-compact algorithm, the first run triggers the mark phase, and the second - the sweep phase. Marks are stored in the heap slots.

### Nan-boxed primitives
The interpreter use nan-boxing - the 8-bytes can store a double, 4-byte int, boolean, null, or pointer to a value that is stored in the heap. They also store a type as a bit mask.

### Forward list view
The `IntrusiveForwardList` class allows you to make a connected list of objects that are inherited from it without creating additional instances of any data structures.