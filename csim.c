#include "cachelab.h"
#include <stdio.h>

long extractBits(long number, int bitPos, int bitNum)
{
    unsigned long mask = ~0;
    mask = mask >> (64 - bitNum) << bitPos;
    return number & mask;
}

long extractTagBits(long address, int s, int b)
{
    return extractBits(address, b+s, 64-b-s);
}

long extractRowIndex(long address, int s, int b)
{
    return extractBits(adress, b, s);
}

long LeaderOfCacheSlot(long tagBits, int rowIndex, int b)
{
    rowIndex <<= b;
    return tagBits & rowIndex;
}




int main()
{
    printf("%lu\n", extractBits(172, 4, 4));
}
