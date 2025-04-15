#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>

#include "list.h"
#include "text_color.h"
#include "logger.h"

#define STR_HELPER(x) #x  
#define STR(x) STR_HELPER(x)

#define DEFAULT_WORD_LEN  32


const size_t BUCKETS_COUNT = 5;

struct BucketItem
{
    char   word[DEFAULT_WORD_LEN];
    size_t val;     // how many times did the word occur
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
BucketItem *     LoadItem      (HashTable *hash_table, const char *const word);
BucketItem *     FindItem      (HashTable *hash_table, const char *const word);

const char *     GetHashTableItemVal(void *item);

char SkipSpaces(FILE *file);     // returns first read alpha (or EOF) letter

size_t HashFunc(const char *const str);


#define ERROR_HANDLER(hash_table_func) do                                                                                   \
{                                                                                                                            \
    HashTableFuncRes func_res = hash_table_func;                                                                              \
    if (func_res != HASH_FUNC_OK)                                                                                              \
    {                                                                                                                           \
        log(ERROR, "Failed " #hash_table_func);                                                                          \
    }                                                                                                                             \
} while (0)

#endif