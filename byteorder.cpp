#include "byteorder.h"
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
    for (size_t c = 0; c < sizeof(uint64_t); c++) 
    {
        ((uint8_t*)&out)[c] = dest[c];
    }
    return out;
}