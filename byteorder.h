#ifndef BYTEORDER_H
#define BYTEORDER_H

#include <cstdint>
#include <cstdio>

// Returns a number othan than 0 if the host CPU is big-endian, otherwise it returns 0.
int isbigendian();

// Reverses the byte order of a 64-bit unsigned integer.
uint64_t reversebytes(uint64_t x);

//writes one 8-byte unsigned integer in big-endian order to f. returns true on success, false on I/O error.
bool writeuint64be(FILE* f, uint64_t v);

//reads one 8 byte unsigned integer in big-endian order from f. Into v, Returns true if success, false if I/O error happens.
bool readuint64be(FILE* f, uint64_t& v);

#endif