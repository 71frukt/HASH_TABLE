#include <stdio.h>
#include "hash_table.h"
#include "hash_table_debug.h"

HashTableVerifyCode HashTableVerify(HashTable *hash_table)
{
fprintf(stderr, "IN VERIFY\n");

    if (hash_table == NULL)
        return HASH_TABLE_PTR_ERR;

    if (hash_table->buckets == NULL)
        return HASH_TABLE_BUCKETS_PTR_ERR;

    for (size_t bucket_num = 0; bucket_num < hash_table->buckets_count; bucket_num++)
    {
        list_t *cur_bucket = hash_table->buckets + bucket_num;
        int item_index = cur_bucket->head;

        if (item_index == 0)    // free bucket
            continue;

        do
        {
            BucketItem *cur_item   = (BucketItem*) ListGetItem(cur_bucket, item_index);
            char *cur_word         = cur_item->word;
            BucketItem *found_item = (BucketItem*)  GetOrCreateItem(hash_table, cur_word);
            
            fprintf(stderr, "ver word = '%s', item_index = %d, bucket_num = %ld, buckets_count = %ld\n", cur_word, item_index, bucket_num, hash_table->buckets_count);
            
            if (cur_item != found_item)
                return HASH_TABLE_ACCORDANCE_ERR;

            item_index = cur_bucket->next[item_index];
        }
        while (item_index != cur_bucket->tail);
    }

    return HASH_TABLE_OK;
}

void PrintHashTableErrors(int error, FILE *dest)
{
    #define PRINT_ERROR_PART(full_err, cmp_err)         \
        if (full_err & cmp_err) fprintf(dest, #cmp_err);

    PRINT_ERROR_PART(error, HASH_TABLE_PTR_ERR);
    PRINT_ERROR_PART(error, HASH_TABLE_BUCKETS_PTR_ERR);
    PRINT_ERROR_PART(error, HASH_TABLE_ACCORDANCE_ERR);

    fprintf(dest, "\n\n");

    #undef PRINT_ERROR_PART
}



void HashTableDump(HashTable *hash_table)
{

}