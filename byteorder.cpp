#include "byteorder.h"
#include <cstring>
int isbigendian() 
{
    int x = 1;
    return ((unsigned char*)&x)[0] != 1;
}
uint64_t reversebytes(uint64_t x) 
{
    uint8_t dest[sizeof(uint64_t)];
    uint8_t* source = (uint8_t*)&x;
    for (size_t c = 0; c < sizeof(uint64_t); c++) 
    {

        dest[c] = source[sizeof(uint64_t) - c - 1];

    }
    uint64_t out;
    std::memcpy(&out, dest, sizeof(uint64_t));
    return out;
}
bool writeuint64be(FILE* f, uint64_t v) 
{
    uint64_t to_write = isbigendian() ? v : reversebytes(v);
    return std::fwrite(&to_write, sizeof(uint64_t), 1, f) == 1;
}
bool readuint64be(FILE* f, uint64_t& v) 
{
    uint64_t raw;
    if (std::fread(&raw, sizeof(uint64_t), 1, f) != 1) 
        return false;
    v = isbigendian() ? raw : reversebytes(raw);

    return true;
}