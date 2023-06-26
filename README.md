# Project 3 README
--pete

Here's the code from Ben's talk, minus some of the components.

You need to build the routines to be able to process the IR into assembly code:
- add
- sub
- mul
- ICmp

To do this you will add new "handle" methods to **x86.cpp**, and their function signatures with the other handle methods in **x86.hpp** (around line 195.)

There are ll files in the directory *tests* for you to try out your code. Ben's shell scripts for building the code should work just fine on babylon.

Optionally you can also create an Sdiv instruction; I've put a write-up of how the integer divider works on the x86-64 architecture in **idiv_howto.md**. I will award 10 extra points if you get this working.
