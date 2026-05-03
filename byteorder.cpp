#include "byteorder.h"

int isbigendian() 
{
    int x = 1;
    return ((unsigned char*)&x)[0] != 1;
}