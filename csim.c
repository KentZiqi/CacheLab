#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Slot {
  short validBits;
  long tagBits;
} Slot;

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
    return extractBits(address, b, s);
}

long LeaderOfCacheSlot(long tagBits, int rowIndex, int b)
{
    rowIndex <<= b;
    return tagBits & rowIndex;
}

Slot ** initializeCache(int S, int E)
{
    Slot **a = malloc(sizeof *a * S);
    if (a){
        for (int i = 0; i < S; i++)
            {
              a[i] = malloc(sizeof *a[i] * E);
            }
    }
    return a;
}


int main()
{
    Slot ** cache = initializeCache(100,2);
    Slot s = cache[99][1];
    s.tagBits = 99;
    printf("%lu\n", s.tagBits);
    printf("%lu\n", extractBits(172, 4, 4));
}
