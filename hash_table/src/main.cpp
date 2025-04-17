#include <stdio.h>
#include <stdlib.h>


#include "hash_table.h"
#include "hash_table_debug.h"
#include "list.h"
#include "logger.h"

int main()
{
    fprintf(stderr, "START\n\n");

    logctor();

    Benchmark();

    // HashTable hash_table = {};
    // ERROR_HANDLER(HashTableCtor(&hash_table, BUCKETS_COUNT, LOAD_FACTOR));
    
    // FILE *source = fopen("hash_table/build/source.txt", "r");

    // HASH_TABLE_DUMP(&hash_table);

    // ERROR_HANDLER(LoadHashTable(&hash_table, source));    

    // fprintf(stderr, "after load\n");

    // LogBucketsLoadFactor(&hash_table);

    // HASH_TABLE_DUMP(&hash_table);

    // ERROR_HANDLER(HashTableDtor(&hash_table));
    // fclose(source);

    fprintf(stderr, "END\n\n");
}