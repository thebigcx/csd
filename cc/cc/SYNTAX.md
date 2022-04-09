# Syntax
Similar to C-syntax in many ways (curly braces to denote code blocks, semicolons to end lines), but with many notable differences.

Functions are variables, therefore the ``let`` keyword is used to define them.

Functions are defined using the following syntax: ``let <name>: fn(<params>) -> <returntype> {};``

Void return type is implicit, will be assumed if no other type is given.

Very lenient compiler (in the name of power), will accept code that C compilers generally wouldn't (such as implicit pointer casts), and will not make assumptions for optimisation - no ``volatile`` keyword necessary.
