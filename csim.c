/**
 * kshikama, Kent Shikama; zxiong, Ziqi Xiong
 */

#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>


// declare global variables
int verbosity = 0;
int set_bits = 0;
int associativity = 0;
int block_bits = 0;
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;
char* trace_file;

// define struct Slot that simulate a slot in the real cache
typedef struct Slot {
    short validBits;
    long tagBits;
} Slot;

/**
 * Helper function for extracting certain bits from a long integer
 * bitPos: the first bit to extract;
 * bitNum: the number of bits after the first bit to extract
 */
long extractBits(long number, int bitPos, int bitNum) {
    unsigned long mask = ~0;
    mask = mask >> (64 - bitNum) << bitPos;
    return number & mask;
}

/**
 * Extracts tag bits give an address
 */
long extractTagBits(long address, int s, int b) {
    return extractBits(address, b + s, 64 - b - s);
}

/**
 * Extracts row index give an address
 */
long extractRowIndex(long address, int s, int b) {
    return extractBits(address, b, s) >> b;
}

/**
 * Initializes the cache
 */
Slot** initializeCache(int S, int E) {
    //first level
    Slot **a = malloc(sizeof *a * S);
    if (a) {
        //second level
        for (int i = 0; i < S; i++) {
            a[i] = malloc(sizeof *a[i] * E);
        }
    }
    return a;
}

/**
 * This function adds an address to the row of the cache;
 * if the address is not in cache, then just inserts it to the front
 * else, remove the address from its old slot and inserts it to the front
 */
void insertStorageAndShiftUntilCurrentSlotIndex(Slot **cache, int rowIndex, long tagBits, int E, int currentSlotIndex) {
    //shift the other addresses
    for (int i = currentSlotIndex; i > 0; i--) {
        Slot previousSlot = cache[rowIndex][i - 1];
        cache[rowIndex][i] = previousSlot;
    }
    //add the address to the front of the row
    cache[rowIndex][0].tagBits = tagBits;
    cache[rowIndex][0].validBits = 1;
}

/**
 * Helper function that tells if the cache is full
 */
bool isFull(Slot **cache, int rowIndex, int E) {
    return cache[rowIndex][E - 1].validBits == 1;
}

/**
 * Function that takes an address and simulates its interaction with cache
 */

void applyAddressToCache(long address, Slot **cache, int s, int b, int E) {
    //get the set bits and tag bits of the address
    long rowIndex = extractRowIndex(address, s, b);
    long tagBits = extractTagBits(address, s, b);
    for (int i = 0; i < E; i++) {
        Slot currentSlot = cache[rowIndex][i];
        //if the address is already in cache, hit
        if (currentSlot.validBits == 1 && tagBits == currentSlot.tagBits) {
            hit_count++;
            insertStorageAndShiftUntilCurrentSlotIndex(cache, rowIndex, tagBits, E, i);
            return;
        }
    }
    //the address is not in cache, miss
    miss_count++;
    //if the row of cache is already filled, evict the least recently used one
    if (isFull(cache, rowIndex, E)) {
        eviction_count++;
    }
    //add the address to the row
    insertStorageAndShiftUntilCurrentSlotIndex(cache, rowIndex, tagBits, E, E - 1);
    return;
}

/**
 * print the usage of this program
 */
void printUsage(char *argv[]) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf(" -h Print this help message.\n");
    printf(" -v Optional verbose flag.\n");
    printf(" -s <num> Number of set index bits.\n");
    printf(" -E <num> Number of lines per set.\n");
    printf(" -b <num> Number of block offset bits.\n");
    printf(" -t <file> Trace file.\n");
    printf("\nExamples:\n");
    printf(" linux> %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf(" linux> %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
}

/**
 * process the options of this program
 */
void process_options(int argc, char *argv[]) {
    char flagchar;
    while ((flagchar = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (flagchar) {
            case 's':
                set_bits = atoi(optarg);
                break;
            case 'E':
                associativity = atoi(optarg);
                break;
            case 'b':
                block_bits = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbosity = 1;
                break;
            case 'h':
                printUsage(argv);
                exit(0);
            default:
                printUsage(argv);
                exit(1);
        }
    }
    if (set_bits == 0 ||
        associativity == 0 ||
        block_bits == 0 ||
        trace_file == NULL) {
        printf("Missing required argument.\n");
        printUsage(argv);
        exit(1);
    }
}

/**
 * the main function
 */
int main(int argc, char *argv[]) {
    process_options(argc, argv);
    char inputline[100];
    FILE *trace_file_pointer = fopen(trace_file, "r");
    if (!trace_file_pointer) {
        fprintf(stderr, "%s: %s\n", trace_file, strerror(errno));
        exit(1);
    }
    Slot **cache = initializeCache(1 << set_bits, associativity); // 1 << set_bits == 2^set_bits
    // parse each line from the teace file
    while (fgets(inputline, 100, trace_file_pointer) != NULL) {
        char operation[20];
        char str[20];
        sscanf(inputline, "%s %s", operation, str);
        char hexPrefix[20];
        strcpy (hexPrefix, "0x");
        char* hexString = strcat(hexPrefix, str);
        unsigned long address;
        sscanf(hexString, "%lx", &address);
        // skip this address if it is "I" access
        if(!strcmp(operation, "I")){
          continue;
        }
        applyAddressToCache(address, cache, set_bits, block_bits, associativity);
        // apply address to cache a second time if it is "M" access
        if (!strcmp(operation, "M")){
          applyAddressToCache(address, cache, set_bits, block_bits, associativity);
        }
    }
    fclose(trace_file_pointer);
    printSummary(hit_count, miss_count, eviction_count);
}
