#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
    int valid;
    unsigned long tag;
    unsigned int timestamp;
} CacheLine;

typedef struct
{
    CacheLine *lines;
} CacheSet;

typedef struct
{
    CacheSet *sets;
    int s;                          // how many sets
    int E;                          // how many lines per set
    int b;                          // how many bytes in the block
    unsigned int timestamp_counter; // last time the line has been accessed
} cache;

cache myCache;
int verbose = 0;
int hit = 0;
int miss = 0;
int eviction = 0;

void initializeCache(int s, int E, int b)
{
    myCache.s = s;
    myCache.E = E;
    myCache.b = b;
    myCache.timestamp_counter = 0;

    int num_cache_sets = 1 << s; // this equals to 2^s cache sets
    myCache.sets = (CacheSet *)malloc(sizeof(CacheSet) * num_cache_sets);

    for (int i = 0; i < num_cache_sets; i++)
    {
        myCache.sets[i].lines = (CacheLine *)malloc(E * sizeof(CacheLine));
        for (int j = 0; j < E; j++)
        {
            myCache.sets[i].lines[j].valid = 0;
            myCache.sets[i].lines[j].tag = 0;
            myCache.sets[i].lines[j].timestamp = 0;
        }
    }
};

void accessCache(unsigned long address, char operation)
{
    unsigned long tag = address >> (myCache.s + myCache.b);
    unsigned int set_index = (address >> myCache.b) & ((1 << myCache.s) - 1);

    // locate the cache set
    CacheSet *set = &myCache.sets[set_index];
    int emptyLine = -1;
    int evictionLine = -1;
    unsigned int min_timestamp = UINT32_MAX;

    // loop through cache lines
    for (int i = 0; i < myCache.E; i++)
    {
        // empty line
        if (!set->lines[i].valid)
        {
            if (emptyLine == -1)
                emptyLine = i;
        }
        //  this is a hit
        else if (set->lines[i].tag == tag)
        {
            hit++;
            set->lines[i].timestamp = myCache.timestamp_counter;
            myCache.timestamp_counter++;
            if (verbose)
                printf("hit\n");
            return;
        }
        else
        {
            // track which line to lru
            if (set->lines[i].timestamp < min_timestamp)
            {
                evictionLine = i;
                min_timestamp = set->lines[i].timestamp;
            }
        }
    }

    // handle miss
    miss++;
    if (verbose)
        printf("miss\n");

    if (emptyLine != -1)
    {
        set->lines[emptyLine].tag = tag;
        set->lines[emptyLine].valid = 1;
        set->lines[emptyLine].timestamp = myCache.timestamp_counter;
    }
    else
    {
        eviction++;
        set->lines[evictionLine].tag = tag;
        set->lines[evictionLine].valid = 1;
        set->lines[evictionLine].timestamp = myCache.timestamp_counter;
        if (verbose)
            printf("eviction\n");
    }
    myCache.timestamp_counter++;
}

void freeCache()
{
    int num_cache_sets = 1 << myCache.s;
    for (int i = 0; i < num_cache_sets; i++)
    {
        free(myCache.sets[i].lines);
    }
    free(myCache.sets);
}

int main(int argc, char *argv[])
{
    int s = 0;
    int E = 0;
    int b = 0;
    int opt;
    char *traceFile = NULL;

    while ((opt = getopt(argc, argv, "vhs:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 'v':
            verbose = 1;
            break;
        case 'h':
            printf("Usage: %s [-v] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
            return 0;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            traceFile = optarg;
            break;
        default:
            printf("Unknown option: %c\n", opt);
            return 1;
        }
    }
    initializeCache(s, E, b);

    FILE *fp = fopen(traceFile, "r");
    if (fp == NULL)
    {
        printf("Error: Cannot open trace file %s\n", traceFile);
        return 1;
    }

    char operation;
    unsigned long address;
    int size;

    while (fscanf(fp, " %c %lx,%d", &operation, &address, &size) == 3)
    {
        if (operation == 'I')
            continue;

        if (verbose)
            printf("%c %lx,%d ", operation, address, size);

        accessCache(address, operation);
        if (operation == 'M')
            accessCache(address, operation);
    }
    fclose(fp);
    freeCache();
    printSummary(hit, miss, eviction);
    return 0;
}