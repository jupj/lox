# Lox interpreters

Interpreters for the Lox language, based on
[Crafting Interpreters](https://craftinginterpreters.com).

## `clox`

Lox interpreter written in C, as described in the
[third part](https://craftinginterpreters.com/a-bytecode-virtual-machine.html)
of Crafting Interpreters.

The interpreter compiles Lox source code into bytecode and executes the bytecode
in a VM.

## `jlox`

Lox interpreter written in Java, as described in the
[second part](https://craftinginterpreters.com/a-tree-walk-interpreter.html) of
Crafting Interpreters.

The interpreter parses Lox source code into an AST and executes code by walking
the AST.
