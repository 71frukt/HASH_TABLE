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

//     alignas(4) char str[9] = "12345678";

//     int *ptr = (int *)(str + 1);

//     int i = *ptr;
//     fprintf(stderr, "[ptr] = %p, ptr % 4 = %ld, i = %s\n", ptr, (size_t)ptr % 4, &i);

    Benchmark();

    // HashTable hash_table = {};
    // ERROR_HANDLER(HashTableCtor(&hash_table, BUCKETS_COUNT, LOAD_FACTOR));
    
    // // FILE *source = fopen("hash_table/build/source.txt", "r");

    // HASH_TABLE_DUMP(&hash_table);

    // // ERROR_HANDLER(LoadHashTable(&ha/sh_table, source));    
    // ERROR_HANDLER(LoadHashTable2(&hash_table, "hash_table/build/source.txt"));    

    // LogBucketsLoadFactor(&hash_table);

    // HASH_TABLE_DUMP(&hash_table);

    // ERROR_HANDLER(HashTableDtor(&hash_table));
    // // fclose(source);

    fprintf(stderr, "END\n\n");
}