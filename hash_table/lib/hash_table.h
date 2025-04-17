#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <immintrin.h>

#include "list.h"
#include "text_color.h"
#include "logger.h"
#include "hash_funcs.h"

#define STR_HELPER(x) #x  
#define STR(x) STR_HELPER(x)

#define DEFAULT_WORD_LEN  32


const size_t BUCKETS_COUNT = 1117;
const size_t LOAD_FACTOR   = 20;

struct BucketItem
{
    // char  word[DEFAULT_WORD_LEN];
    __m256i word;     // < 32 letters
    size_t  val;     // how many times did the word occur
};

struct HashTable
{
    size_t  buckets_count;
    size_t  load_factor;
    list_t *buckets;
};

enum HashTableFuncRes
{
    HASH_FUNC_OK,
    HASH_FUNC_FAIL,
};


HashTableFuncRes HashTableCtor (HashTable *hash_table, size_t buckets_count, size_t load_factor);
HashTableFuncRes HashTableDtor (HashTable *hash_table);

HashTableFuncRes LoadHashTable (HashTable *hash_table, FILE *source);
BucketItem *     LoadItem      (HashTable *hash_table, const __m256i *const word_m256);
BucketItem *     FindItem      (HashTable *hash_table, const __m256i *const word_m256);

const char *     GetHashTableItemVal(void *item);

char SkipSpaces(FILE *file);     // returns first read alpha (or EOF) letter

#define ERROR_HANDLER(hash_table_func) do                                                                                   \
{                                                                                                                            \
    HashTableFuncRes func_res = hash_table_func;                                                                              \
    if (func_res != HASH_FUNC_OK)                                                                                              \
    {                                                                                                                           \
        log(ERROR, "Failed " #hash_table_func);                                                                                  \
    }                                                                                                                             \
} while (0)

#endif