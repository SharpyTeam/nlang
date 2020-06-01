# Nlang Language Overview

Nlang is very easy to learn if you know at least one popular programming language. That's why you won't find an explanation from scratch here. It is somewhat similar to Kotlin, Python, C/C++, and Rust.

## Creating and executing nlang scripts

First, create a nlang script. Let's make a agreement to use `.n` extension for nlang scripts.
For example, let's call it `source.n`. Now, write some code in this file. 

To execute the script, you should call the interpreter like that:
```bash
./nlang source.n
```
The interpreter will execute your script and print the result, obtained by last expression in the file.

## Sample script

```
fn fibonacci(n) {
    if (n == 1) {
        return 0
    } else if (n == 2) {
        return 1
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}
fibonacci(10)
```

## Functions

Function, calculating the sum of two parameters:
```
fn sum(a, b) {
    return a + b
}
```
Or, with type hinting:
```
fn sum(a: number, b: number): number {
    return a + b
}
```

## Variables

```
let a
let b = 1
let c: number = 1.2
a = 32
```

## Comments

```
// This is a comment

let a = 123.456 // this is also a comment
```

## Conditional expressions

```
if (n == 1) {
    return 0
} else if (n == 2) {
    return 1
}
```

```
if (n) {
    return 0
} else  {
    return 1
}
```

## Literals

```
"This is a string literal"
'This is also a string literal'
1234.567 // this is a number literal
null // this is a null literal
true // this is a boolean (true) literal
```

## Loops

```
let n = 100
while (n > 1) {
    n = n / 2
}
```