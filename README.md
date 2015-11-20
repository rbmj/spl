===============
SPL Compiler
===============

Compiling:
-----------

Dependencies for this project include
 - Readline
 - flex/bison
 - nasm

Compilation is a simple:

    $ make

Usage:
-----------

To build a file (e.g. examples/factorStr.spl):

    $ ./spl examples/factorStr.spl
    $ nasm examples/factorStr.asm -felf
    $ ld examples/factorStr.o libspl.o -x -m elf_i386 -o examples/factorStr

Currently, the compiler only produces assembly code - it does not contain
logic to assemble this file, or link in the support code needed for IO in
libspl.o.
