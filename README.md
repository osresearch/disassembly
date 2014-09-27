Reverse engineering examples
===

These are simplified examples to demonstrate common programming techniques
so that you can recognize them while reverse engineering larger programs.

The ease of reading the assemnbly varies with the optimization level.
`gcc -O3` will produce very complex vectorized code sometimes for
functions that you might not expect.

For clarify the base pointer can be omitted since it doesn't need to
be present. `-momit-leaf-base-pointer` can be specified.


Machine model
===
Data structures, higher order functions, even function names
are the world that has been pulled over your eyes to blind you
from the truth: computers only process numbers.  And, for the
most part, these numbers are stored in registers and memory.

Registers
---
Most CPUs have a small number of very high-speed registers.  Depending
on the architecture these might be named ("ax", "bx") or numbered
("r17", "r23") or a weird mix of both ("rax", "r9d", "r11", etc).

| Arch | Width | General Purpose Registers | Special Registers |
|------|-------|---------------------------|-------------------|
| i386 | 8, 16 and 32 |ax, bx, cx, dx, si, di | eip, sp, bp, ds, cs, etc |
| x86-64 | 64-bit | rax, rbx, rcx, rdx, rsi, rdi, r8-r15 | rip, ... |
| ARM | 32-bit | r0-r15 | r15 is the pc, r14 is the lr, r13 is the sp |

Some registers are special, such as the Program Counter (PC, arm)
or Instruction Pointer (IP, x86).  This points to the current
instruction being executed and is incremented after each instruction,
or can be modified by a branch instruction.  On the ARM and x86-64
it can be treated as a normal register, but other architectures
can only modify it via special instructions.

	MOV %rax, 0x1000
	MOV %rbx, 0x0100
	ADD %rcx, %rax, %rbx


Argument passing
---

Compilers implement an "ABI", the application binary interface,
that defines for that OS and architecture how publicly
visible external functions are called.  This is just a polite
fiction -- for the most part the CPU doesn't care about how
control is transfered since it is just processing numbers.
Additionally, compilers are free to do whatever they want inside of
functions or for ones that aren't visible to others.

| Arg | i386 (fastcall) | x86-64 Linux | x86-64 Win | ARM |
|-----|------|--------------|------------|-----|
| Return | %eax | %rax | %rax | %r0 |
| 0   | %ecx | %rdi | %rcx | %r1 |
| 1   | %edx | %rsi | %rdx | %r2 |
| 2   | sp[0x00] | %rdx | %r8 | %r3 |
| 3   | sp[0x04] | %rcx | %r9 | sp[0x00] |
| 4   | sp[0x08] | %r8 | sp[0x00] | sp[0x04] |
| 5   | sp[0x0C] | %r9 | sp[0x08 | sp[0x08] |
| 6   | sp[0x10] | sp[0x00] | sp[0x10] | sp[0x0c] |
| 7   | sp[0x14] | sp[0x08] | sp[0x18] | sp[0x10] |

Sometimes the best thing to do is to just write a test program and
compile it to see what happens.

	lots_of_args(0,1,2,3,4,5,6,7,8,9,10);

	sub        rsp, 0x20
	mov        edx, 0x3
	mov        ecx, 0x4
	xor        eax, eax
	mov        dword [ss:rsp+0x18], 0xa
	mov        dword [ss:rsp+0x10], 0x9
	mov        dword [ss:rsp+0x8], 0x8
	mov        dword [ss:rsp], 0x7
	mov        edi, 0x0
	mov        esi, 0x2
	mov        r8d, 0x5
	mov        r9d, 0x6
	call       lots_of_args
	add        rsp, 0x20


Stack
---

The stack was mentioned as a way that excess arguments are passed.
On x86-64 and many other architectures it also holds the return address
(although ARM uses the link register for this purpose).  It is also
used for temporary storage during function calls.

Registers can be either "callee saved" or "caller saved".  Callee
saved registers are ones that the called function must restore to
their original value if it changes them.  The caller saved registers
can be overwritten by the called function for its own purpose, and
if the caller wanted to preserve their value it must store them
somewhere, typically by pushing them onto the stack.


Goto
---

Higher level language programmers use phrases like "goto considered
harmful", but only because they can't handle the truth.  We live
in a world that has goto and those gotos are used billions of times
by CPUs.  Those developers have the luxury of not knowing what we
assembly programmers know: goto, while grotesque and incomprehensible
to them, is necessary for computers to function.



Arrays
---


Common patterns
===

printf
---

Constants
---
crc32, sha256, etc

Linked lists
---

Function pointers and classes
---

