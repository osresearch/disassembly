     ____                              
    |  _ \ _____   _____ _ __ ___  ___ 
    | |_) / _ \ \ / / _ \ '__/ __|/ _ \
    |  _ <  __/\ V /  __/ |  \__ \  __/
    |_| \_\___| \_/ \___|_|  |___/\___|

                 _____             _                      _             
                | ____|_ __   __ _(_)_ __   ___  ___ _ __(_)_ __   __ _ 
                |  _| | '_ \ / _` | | '_ \ / _ \/ _ \ '__| | '_ \ / _` |
                | |___| | | | (_| | | | | |  __/  __/ |  | | | | | (_| |
                |_____|_| |_|\__, |_|_| |_|\___|\___|_|  |_|_| |_|\__, |
                             |___/                                |___/ 

                                       Trammell Hudson <hudson@trmm.net>

Hopper:
	http://hopperapp.com/download.html

Sample program:
	https://github.com/osresearch/disassembly/blob/master/example

This document:
	http://bit.ly/reverseengineering











Disassembly
===
The really old way to disassemble a program is to hex dump it and walk
through the processor datasheet to decode the instructions:

    0001540: 55 48 89 e5 89 7d fc 89 75 f8 8b 75 fc 03 75 f8  UH...}..u..u..u.

    55 == 0x50 == push, 0x05 == %rbp => push %rbp
    48 89 E5
    01001000 10001001 11100101
    REX.W    MOV          %BP

This is far too much work!  Instead we have tools to help us, like
`obdjump -d` on Linux and `otool` on OS X:

    % otool -tV example | head
    0000000100001540	pushq	%rbp
    0000000100001541	movq	%rsp, %rbp

But even this is not as useful as it could be -- if the binary is
stripped there are no function names to help with the decoding
and not even antyhing to mark where functions begin.  Thus the
desire for an interactive tool to assist us in understanding
these programs.


Reverse engineering examples
===

These are simplified examples to demonstrate common programming techniques
so that you can recognize them while reverse engineering larger programs.

The ease of reading the assemnbly varies with the optimization level.
`gcc -O3` will produce very complex vectorized code sometimes for
functions that you might not expect.

For clarify the base pointer can be omitted since it doesn't need to
be present. `-momit-leaf-frame-pointer` can be specified.


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
| x86-64 | 64-bit | rax, rbx, rcx, rdx, rsi, rdi, r8-r15 | rip, rbp, rsp,... |
| ARM | 32-bit | r0-r15 | r15 is the pc, r14 is the lr, r13 is the sp |

Some registers are special, such as the Program Counter (PC, arm)
or Instruction Pointer (IP, x86).  This points to the current
instruction being executed and is incremented after each instruction,
or can be modified by a branch instruction.  On the ARM and x86-64
it can be treated as a normal register, but other architectures
can only modify it via special instructions.

For the most part we will ignore the legacy cruft of i386 --
there are segment registers, descriptor tables, call gates and
an enormous number of dusty corners that just don't matter anymore.


Argument passing
---

Compilers implement an "ABI", the application binary interface,
that defines for that OS and architecture how publicly
visible external functions are called.  This is just a polite
fiction -- for the most part the CPU doesn't care about how
control is transfered since it is just processing numbers.
Additionally, compilers are free to do whatever they want inside of
functions or for ones that aren't visible to others.

| Arg | i386     | x86-64   | x86-64   | ARM      |
|     | Fastcall | Linux    | Windows  |          |
|-----|----------|----------|----------|----------|
| Ret | %eax     | %rax     | %rax     | %r0      |
| 0   | %ecx     | %rdi     | %rcx     | %r1      |
| 1   | %edx     | %rsi     | %rdx     | %r2      |
| 2   | sp[0x00] | %rdx     | %r8      | %r3      |
| 3   | sp[0x04] | %rcx     | %r9      | sp[0x00] |
| 4   | sp[0x08] | %r8      | sp[0x00] | sp[0x04] |
| 5   | sp[0x0C] | %r9      | sp[0x08] | sp[0x08] |
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
in a world that has goto and those gotos are used trillions of times
by billions of CPUs every second of everyday.  Those developers
have the luxury of not knowing what we assembly programmers know:
goto statements, while grotesque and incomprehensible to them, are
necessary for computers to function.  They can't handle the GOTO!

Branch instructions come in a few varieties:

* absolute, relative or indirect: is the destination address an
actual address, relative to the current address or read from a register?

* unconditional or conditional: should the branch always be taken,
or does it depend on a predicate?

* normal or linked: on x86, `CALL` will push the return address onto
the stack.  On ARM, `BL` will move the return address into the link
register `%r14`.


Arrays
---

Arrays are just pointers plus offsets.  Most of the time you will
see them used with a offset addressing mode -- the function will
take the base pointer of the array as an argument and then do
operations that walk along an array like

		MOV %rax, 0
		MOV %rdi, 0
	LOOP:
		ADD %rax, [%rdx + %rdi]
		ADD %rdi, 4
		DEC %rcx
		JNE LOOP


Interactive dissassembly
===

Most useful features:
* Finding function boundaries
* `p`: Marking something as a function
* `n`: Naming functions and addresses
* `x`: Showing cross references -- what calls this?
* `d`: Change data type: 8, 16, 32, or 64 bytes
* '-': Signed/unsigned: turn large constants into their negative values
* 'a': Mark something as an ASCII string
* 'A': Mark something as a Unicode string
* 'u': Mark something as unexplored
* `Space`: Show control flow graph
* `Alt-Enter`: "Decompile"

*Caution* -- i386 instructions can start on arbitrary boundaries,
so the "Find next function" is not guaranteed to work and might find
another instruction.

Limitations
---
The "decompilers" in Hopper and IDA Pro are pretty good, but they
are not perfect.  They are doing a difficult job of trying to
figure out what the compiler was thinking and take advantage of
the fact that most compilers use a similar set of transforms and
adhere to the ABI, but inlining, loop unrolling, vectorization,
whole-program-optimization, dead code elimination, constant folding
and other techniques can be hard or impossible to undo.

Any code that works with special registers or instructions will
probably not decompile well. Atomics, for instance, or Intel
legacy cruft like `LGDT` will be ignored.

There is also signifcant amounts of line noise around `SIGNEXTEND`
and `LOWBYTE` calls since the decompiler doesn't know which are
important to the code, but wants to be sure that you know when
only partial registers are being considered.

Other tools
===

`strings` is incredibly useful for initial exploration.  The `-x` option
will print the hex offset of the strings so that you can find it in
your disassembler and hopefully find the cross references that use it.
With the GNU version you can also search for Unicode strings:

	strings -t x -e l file.raw

`xxd` is an all purpose hex dumper.  Again, very useful for initial
exploration to get a feel for what is in the file.  By default is
uses 16-bit words, but most often single bytes is better.

	xxd -g 1 file.raw | less

`dd` can be used to transform large files into smaller ones.  Frequently
ROM dumps will have portions that are copied to different memory locations
or that you want to analyze individually.  To read the 64K at offset 0x40000
from `file.raw` into `file.exe` you can use a combination of the block size,
skip and count options:

	dd if=file.raw bs=1 skip=$[0x40000] count=$[0x1000] of=file.part

`nm` for dumping symbols.

`file` for attempting to analyze what something is.  If it says `data`, then
you'll need to do more digging.

`objdump` / `otool` for doing quick disassemblies.

`objcopy` to convert a raw firmware dump into an ELF for easier analysis.


Common patterns
===

Since most compilers generate fairly straightforward code it is
possible to recognize common patterns and use them to make sense of the
code.

printf
---
Debug prints statements are possibly the most useful -- even in a
totally stripped binary there are occasionally printf calls lefts
by the programmers.  These calls are the signposts that light our
way.


Function pointers and classes
---
These can be especially hard to debug since they do not have common
call sites.  With normal functions you can see "what else calls here",
but with function pointers it can be hard to track them down.
Hopefully there is a constant pointer table that you can reference, but
sometimes the pointers are copied into the object (although this
does open up other exploits later).


Constants
---
Most hash functions like crc32, sha256, etc all have well-defined
constants that are giveaways when you find them in the binaries.
The length can also be useful to figure what sort of checksum is
in use, since MD5 and SHA1 share some values.

| Bits    | Algorithm  | Constants             |
|---------|------------|-----------------------|
| 8 or 16 | Simple sum | maybe 1's complement  |
| 32      | crc32      | 0x77073096 0xEE0E612C |
| 64      | MD5        | 0xd76aa478 0xe8c7b756 |
| 160     | SHA1       | 0x5A827999 0x6ED9EBA1 |
| 256     | SHA256     | 0x428a2f98 0x71374491 |


Linked lists
---
Traversing linked lists shows up fairly frequently and is often inlined,
so it is worth looking at this pattern.


Optimizations
===
A compiler that does no optimizations can be hard to follow in the
disassembly since it will make lots of random copies of things that
serve no purpose.  A compiler with too much optimization can be hard
to folow since it will use tricks like SSE and loop unrolling.

These examples were built with `-O1`, which keeps them fairly
simple.  For instance, this simple memory copy routine:

    void my_memcpy(uint8_t * d, const uint8_t * s, size_t n)
    {
        for (size_t i = 0 ; i < n ; i++)
                d[i] = s[i];
    }

With `gcc -O1` it is what you might expect:

    testq	%rdx, %rdx
    je	0x1f
    nopw	%cs:_my_memcpy(%rax,%rax)
    movb	_my_memcpy(%rsi), %al
    movb	%al, _my_memcpy(%rdi)
    incq	%rsi
    incq	%rdi
    decq	%rdx
    jne	0x10
    retq

But with `gcc -O3` it becomes much more complex:

	testq	%rdx, %rdx
	je	0x6f
	xorl	%ecx, %ecx
	movq	%rdx, %rax
	andq	$-0x20, %rax
	je	0x4e
	leaq	-0x1(%rdx), %r8
	leaq	_my_memcpy(%rsi,%r8), %r9
	xorl	%ecx, %ecx
	cmpq	%rdi, %r9
	jb	0x27
	addq	%rdi, %r8
	cmpq	%rsi, %r8
	jae	0x4e
	xorl	%ecx, %ecx
	nopl	_my_memcpy(%rax)
	movups	_my_memcpy(%rsi,%rcx), %xmm0
	movups	0x10(%rsi,%rcx), %xmm1
	movups	%xmm0, _my_memcpy(%rdi,%rcx)
	movups	%xmm1, 0x10(%rdi,%rcx)
	addq	$0x20, %rcx
	cmpq	%rcx, %rax
	jne	0x30
	movq	%rax, %rcx
	cmpq	%rdx, %rcx
	je	0x6f
	addq	%rcx, %rsi
	addq	%rcx, %rdi
	subq	%rcx, %rdx
	nopl	_my_memcpy(%rax)
	movb	_my_memcpy(%rsi), %al
	movb	%al, _my_memcpy(%rdi)
	incq	%rsi
	incq	%rdi
	decq	%rdx
	jne	0x60
	retq


Challenge
===
https://github.com/osresearch/disassembly/blob/master/secret

As a small challenge, can you figure out how to get this program to
accept your input and determine what secret message it prints?







