#include "cachelab.h"
#include <stdio.h>

long extractBits(long number, int bitPos, int bitNum)
{
    unsigned long mask = ~0;
    mask = mask >> (64 - bitNum) << bitPos;
    return number & mask;
}


int main()
{
    printf("%lu\n", extractBits(172, 4, 4));
}
