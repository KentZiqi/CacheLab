#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

typedef struct Slot {
    short validBits;
    long tagBits;
} Slot;

long extractBits(long number, int bitPos, int bitNum) {
    unsigned long mask = ~0;
    mask = mask >> (64 - bitNum) << bitPos;
    return number & mask;
}

long extractTagBits(long address, int s, int b) {
    return extractBits(address, b + s, 64 - b - s);
}

long extractRowIndex(long address, int s, int b) {
    return extractBits(address, b, s) >> b;
}

Slot** initializeCache(int S, int E) {
    Slot **a = malloc(sizeof *a * S);
    if (a) {
        for (int i = 0; i < S; i++) {
            a[i] = malloc(sizeof *a[i] * E);
        }
    }
    return a;
}

void insertStorageAndShiftUntilCurrentSlotIndex(Slot **cache, int rowIndex, long tagBits, int E, int currentSlotIndex) {
    for (int i = currentSlotIndex; i > 0; i--) {
        Slot previousSlot = cache[rowIndex][i - 1];
        cache[rowIndex][i] = previousSlot;
    }
    cache[rowIndex][0].tagBits = tagBits;
    cache[rowIndex][0].validBits = 1;
}

void printCache(Slot **cache, int s, int E){
    printf("cache: \n");
    for(int i=0; i < 1<<s; i++){
      for(int j=0; j<E; j++){
        printf("validBits: %d,", cache[i][j].validBits);
        printf("tagBits: %lx", cache[i][j].tagBits);
      }
      printf("\n");
    }
}


bool isFull(Slot **cache, int rowIndex, int E) {
    return cache[rowIndex][E - 1].validBits == 1;
}


int verbosity = 0;
int set_bits = 0;
int associativity = 0;
int block_bits = 0;
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;
char* trace_file;


void applyAddressToCache(long address, Slot **cache, int s, int b, int E) {
    long rowIndex = extractRowIndex(address, s, b);
    long tagBits = extractTagBits(address, s, b);
    for (int i = 0; i < E; i++) {
        Slot currentSlot = cache[rowIndex][i];
        if (currentSlot.validBits == 1 && tagBits == currentSlot.tagBits) {
            hit_count++;
            insertStorageAndShiftUntilCurrentSlotIndex(cache, rowIndex, tagBits, E, i);
            return;
        }
    }
    miss_count++;
    if (isFull(cache, rowIndex, E)) {
        eviction_count++;
    }
    insertStorageAndShiftUntilCurrentSlotIndex(cache, rowIndex, tagBits, E, E - 1);
    return;
}

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

void process_options(int argc, char *argv[]) {
    char flagchar;
    verbosity = 0;
    set_bits = 0;
    associativity = 0;
    block_bits = 0;
    trace_file = NULL;
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

int main(int argc, char *argv[]) {
    process_options(argc, argv);
    char inputline[100];
    FILE *trace_file_pointer = fopen(trace_file, "r");
    if (!trace_file_pointer) {
        fprintf(stderr, "%s: %s\n", trace_file, strerror(errno));
        exit(1);
    }
    Slot **cache = initializeCache(1 << set_bits, associativity); // 1 << set_bits == 2^set_bits
    while (fgets(inputline, 100, trace_file_pointer) != NULL) {
        char operation[20];
        char str[20];
        sscanf(inputline, "%s %s", operation, str);
        char hexPrefix[20];
        strcpy (hexPrefix, "0x");
        char* hexString = strcat(hexPrefix, str);
        unsigned int address;
        sscanf(hexString, "%x", &address);
        applyAddressToCache(address, cache, set_bits, block_bits, associativity);
        if (!strcmp(operation, "M")){
          applyAddressToCache(address, cache, set_bits, block_bits, associativity);
        }
    }
    fclose(trace_file_pointer);
    printSummary(hit_count, miss_count, eviction_count);
}
