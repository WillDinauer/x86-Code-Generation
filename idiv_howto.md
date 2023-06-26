# The x86-64 *idiv* Command
--pete

The x86-64 integer division instruction is *idiv*. It does a 128-bit/64-bit division, where
- *rax* holds the lower 64 bits
- *rdx* holds the upper 64 bits.
- The divisor goes into a register of your choice (e.g., *rbx*) - - The resultant quotient is found in *rax*
- The remainder is in *rdx*

*Do we have to figure out how to load *rdx*?*
No, you can use the instruction *cqto*, which will automatically sign-extend *rax* properly into *rdx*. You may have to save *rdx* if it is already being used (*push/pop* or equivalent.)
