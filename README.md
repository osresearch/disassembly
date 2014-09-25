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
Registers
---

Argument passing
---

Stack
---

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


