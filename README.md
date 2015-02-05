Brainfuck LLVM JIT
=====================
Brainfuck compiler with JIT support written in C++11 using the mighty LLVM!

Developed in Linux, ported to Windows.

Usage
---------
You can either
1. Pass the brainfuck code as a string
2. Read it from a file then pass the ifstream to the compiler.

Dependencies
--------------
LLVM 3.5.0 or higher (it might also work with previous versions, but I haven't tested)


How to Compile
------------------
For Windows, you can either use CMake, or you can open .vxproj in visual studio.
For Linux, use CMake to build it.

License
------------
MIT Licence


TODO
--------
1. Integrate different JIT optimizations
2. Implement malloc function
