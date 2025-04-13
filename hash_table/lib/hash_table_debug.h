#ifndef HASH_TABLE_DEBUG_H
#define HASH_TABLE_DEBUG_H

#include "hash_table.h"

enum HashTableVerifyCode
{
    HASH_TABLE_OK               = 1 << 0,

    HASH_TABLE_PTR_ERR          = 1 << 1,
    HASH_TABLE_BUCKETS_PTR_ERR  = 1 << 2,
    HASH_TABLE_ACCORDANCE_ERR   = 1 << 3
};

HashTableVerifyCode HashTableVerify(HashTable *hash_table);

void PrintHashTableErrors(int error, FILE *dest);


#ifdef HASH_TABLE_DEBUG

#define HASH_TABLE_VERIFY(hash_table) do                                                                    \
{                                                                                                            \
    HashTableVerifyCode verify_code = HashTableVerify(hash_table);                                            \
                                                                                                               \
    if (verify_code != HASH_TABLE_OK)                                                                           \
    {                                                                                                            \
        fprintf(stderr, COLORED("VERIFICATION FAILED", RED) " in " COLORED("%s:%d (%s) \n", CYAN) "errors:\n",    \
                __FILE__, __LINE__, __func__);                                                                     \
                                                                                                                    \
        PrintHashTableErrors(verify_code, stderr);                                                                   \
        return HASH_FUNC_FAIL;                                                                                        \
    }                                                                                                                  \
                                                                                                                        \
} while(0)

#else
#define HASH_TABLE_VERIFY(hash_table)
#endif

#endif