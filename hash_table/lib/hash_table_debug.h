#ifndef HASH_TABLE_DEBUG_H
#define HASH_TABLE_DEBUG_H

#include "hash_table.h"
#include "logger.h"

enum HashTableVerifyCode
{
    HASH_TABLE_OK               = 1 << 0,

    HASH_TABLE_PTR_ERR          = 1 << 1,
    HASH_TABLE_BUCKETS_PTR_ERR  = 1 << 2,
    HASH_TABLE_ACCORDANCE_ERR   = 1 << 3
};

HashTableVerifyCode HashTableVerify(HashTable *hash_table);

void HashTableDump (HashTable *hash_table);
void BucketDump    (list_t *bucket);


const size_t ERROR_STR_MAX_SIZE = 300;
char *GetHashTableErrors(int error, FILE *dest);

#ifdef HASH_TABLE_DEBUG

#define HASH_TABLE_VERIFY(hash_table) do                                                                                                        \
{                                                                                                                                                \
    HashTableVerifyCode verify_code = HashTableVerify(hash_table);                                                                                \
                                                                                                                                                   \
    if (verify_code != HASH_TABLE_OK)                                                                                                               \
    {                                                                                                                                                \
        const char *const errors = GetHashTableErrors(verify_code, stderr);                                                                           \
        log(ERROR, "HASH_TABLE_VERIFY failed! Errors:\n %s\n", errors);                                                                         \
        return HASH_FUNC_FAIL;                                                                                                                          \
    }                                                                                                                                                    \
                                                                                                                                                          \
} while(0)

#else
#define HASH_TABLE_VERIFY(hash_table)
#endif

#endif