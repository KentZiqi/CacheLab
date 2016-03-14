#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

void insertStorage(Slot ** cache, int rowIndex, int tagBits, int E)
{
  for(int i=E-1; i>0; i--){
    cache[rowIndex][i] = cache[rowIndex][i-1];
  }
  cache[rowIndex][0].tagBits = tagBits;
  cache[rowIndex][0].validBits = 1;
}

bool checkCache(long address, Slot ** cache, int s, int b, int E, int* hitCount, int* missCount)
{
  long rowIndex = extractRowIndex(address, s, b);
  long tagBits = extractTagBits(address,s, b);
  for(int i=0; i<E; i++){
    Slot currentSlot = cache[rowIndex][i];
    if(currentSlot.validBits==1 || tagBits==currentSlot.tagBits){
      hitCount++;
      return true;
    }
  }
  insertStorage(cache, rowIndex, tagBits, E);
  missCount++;
  return false;
}


int main()
{
    Slot ** cache = initializeCache(1,5);
    cache[0][0].tagBits = 1;cache[0][0].tagBits = 2;
    insertStorage(cache, 0, 1, 5);
    printf("%lu\n", cache[0][2].tagBits);
}
