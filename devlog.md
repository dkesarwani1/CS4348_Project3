## 2026-05-03 11:30 AM Project Objective

The goal of this project is to build a command-line program that creates and manages a B-tree stored inside a binary index file. The program supports six operations: create, insert, search, load, print, and extract. This forms a simple persistent key/value store. The index file is then divided into 512-byte blocks, with each block holding either the file header or one B-tree node. The main constraint is that the program must never hold more than 3 nodes in memory at once, which simulates how a real database pages data in and out of the disk rather than loading the whole tree into RAM.
## 2026-05-03 12:03 PM — header guards and isbigendian

Created the header file with include guards and a single declaration:
isbigendian(). This is the main structure because every read and write of an
on disk integer eventually depends on knowing the host byte order.

## 2026-5-3 12:20 PM Added ReverseBytes & file I/O wrappers
Then added the wrappers themselves: readuint64be and writeuint64be. These will be the only entry points the rest of the codebase needs. Nothing outside this module should ever call isbigendian or reversebytes directly. That keeps the endian decision in exactly one area. If I find a bug later, I fix it wuth 2 functions, not 50 or more call sites.

byteorder.h is done. Implementation next.

## 2026-05-03 6:50PM — byteorder.cpp- Added bigendian function

Implemented isbigendian() using the canonical trick: write a 1 to a multi byte int, look at byte 0. On little endian machines byte 0 is 1
and the function returns 0. On big endian machines byte 0 is 0 and it returns a number other than 0.
This compiles with just this and a tiny test main that prints the result. On an x8664 computer it prints 0.

## 2026-5-04 12:22PM — byteorder.cpp Added reversebytes()
Implemented reversebytes() and is used in uint8t consistently and copied the result back through
a byte loop instead of the spec's pointer cast deref trick. Verified by reversebytes(reversebytes(x)) == x on a few sample
values. 

## 2026-5-04 7:40PM — byteorder.cpp Added writeuint64be(), readuint64be(), and done modifying reversebytes()
Added writeuint64be(), readuint64be(), and modified reversebytes for one pass. writeuint64be and readuint64be now correctly convert values on little endian so all 64-bit integers are stored on disk in big endian order and read back in host byte order. I also replaced the manual byte-copy loop in reversebytes with std::memcpy and verified everything with a round-trip smoke test using 0x0102030405060708, confirming the file bytes were 01 02 03 04 05 06 07 08 and the value read back correctly. Implementation of byteorder.cppis done.

## 2026-5-05 2:27 PM FInsihed btree.h
Created btree.h as the main interface for the B-tree.cpp file. I added include guards first so the header can be safely included in multiple source files, then included <cstdint>, <cstdio>, and <string> because the file uses uint64t, FILE*, and std::string. I defined the core B-tree constants: a 512-byte block size, minimum degree 10, maximum 19 keys, and maximum 20 children. These constants match the project format where each node is stored as one fixed-size disk block. I also added the MAGIC string "4348PRJ3" so the program can identify the correct index files. I then defined the Header struct with the file magic, root block id, and the next free block id. It then is followed by the Node struct with its block id, parent id, key count, key array, value array, and the child pointer array. Finally, I declared the disk I/O helper functions, the leaf checking and block allocation helpers, and all command functions for create, insert, search, load, print, and extract. The header will now give both btree.cpp and main.cpp a shared structure for the index file without putting any implementation in the header.
