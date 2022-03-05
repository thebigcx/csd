# Compiler

Custom programming language, assembly language, and binary format.

## Language Features
- C-like syntax
- Rust-like variable declaration. 'let' keyword for declaring variables, type follows variable name or omitted if automatic. E.g: ```let x: i32```
- Variable shadowing, lets you 'create' a new variable with the same name, replaces previous declaration
- ```pub``` for public declarations, ```global``` for external variables, private is default
- All code is compiled, even outside functions, no 'main' function
- Namespaces, C++ like (```x::member```)
- Extremely bare bones object-orientedness (no polymorphism, vtables, or other similar garbage)

## Assembly Features
- Intel-like syntax

## Binary Format Features