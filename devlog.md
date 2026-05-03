
## 2026-05-03 11:30 AM Project Objective

The goal of this project is to build a command-line program that creates and manages a B-tree stored inside a binary index file. The program supports six operations: create, insert, search, load, print, and extract. This forms a simple persistent key/value store. The index file is then divided into 512-byte blocks, with each block holding either the file header or one B-tree node. The main constraint is that the program must never hold more than 3 nodes in memory at once, which simulates how a real database pages data in and out of the disk rather than loading the whole tree into RAM.
## 2026-05-03 12:03 PM — header guards and isbigendian

Created the header file with include guards and a single declaration:
isbigendian(). This is the main structure because every read and write of an
on disk integer eventually depends on knowing the host byte order.

## 2026-5-3 12:20 PM Added ReverseBytes & file I/O wrappers
Then added the wrappers themselves: read_uint64_be and write_uint64_be. These will be the only entry points the rest of the codebase needs. Nothing outside this module should ever call isbigendian or reversebytes directly. That keeps the endian decision in exactly one area. If I find a bug later, I fix it wuth 2 functions, not 50 or more call sites.

byteorder.h is done. Implementation next.
