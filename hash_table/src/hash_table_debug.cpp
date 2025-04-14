#include <stdio.h>
#include <string.h>

#include "hash_table.h"
#include "hash_table_debug.h"
#include "logger.h"

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

char *GetHashTableErrors(int error, FILE *dest)
{
    static char error_str[ERROR_STR_MAX_SIZE];
    size_t error_str_cursor = 0;

    #define PRINT_ERROR_PART(full_err, cmp_err)                                         \
        if (full_err & cmp_err)                                                          \
        {                                                                                 \
            strncpy(error_str, #cmp_err, ERROR_STR_MAX_SIZE - error_str_cursor - 1);       \
            error_str_cursor += strlen(#cmp_err);                                           \
                                                                                             \
            if (error_str_cursor == ERROR_STR_MAX_SIZE)                                       \
                log(WARNING, "error_str overflow");                                   \
        }
        
    PRINT_ERROR_PART(error, HASH_TABLE_PTR_ERR);
    PRINT_ERROR_PART(error, HASH_TABLE_BUCKETS_PTR_ERR);
    PRINT_ERROR_PART(error, HASH_TABLE_ACCORDANCE_ERR);

    fprintf(stderr, "res_error = %s\n", error_str);

    return error_str;

    #undef PRINT_ERROR_PART
}


void HashTableDump(HashTable *hash_table)
{
    log(DUMP, "HASH TABLE DUMP");

    log(INFO, "Hash table [%p]\n", hash_table);
    log(INFO, "{\n");

    log(INFO, "\tbuckets_count = %ld\n",   hash_table->buckets_count);    
    log(INFO, "\tload_factor   = %ld\n\n", hash_table->load_factor  );    
    
    log(INFO, "\tBuckets:\n");    

    for (size_t i = 0; i < hash_table->buckets_count; i++)
    {
        log(INFO, "\tBucket %ld:\n", i);
        BucketDump(hash_table->buckets + i);
    }
    
    log(INFO, "}\n");
}

void BucketDump(list_t *bucket)
{
    lassert(bucket, "bucket == NULL");

    // log(INFO, "bucket[%p]:   ", bucket);

    int num = bucket->head;

    while (num != 0)
    {
        void *item = (char *) bucket->data + num * bucket->item_size;
        const char *item_val = GetHashTableItemVal(item);

        log(INFO, "%s ", item_val);
        num = bucket->next[num];
    } 

    // table
    log(INFO, "\n<table border width = \"85%%\"style=\"margin-left: 3%%\">\n");

    log(INFO, "<tr>\n");
    log(INFO, "<td>index</td>");

    for (int i = 0; i < bucket->capacity; i++)
    {
        log(INFO, "<td>%d</td>", i);
    }

    log(INFO, "</tr>\n");

    // data
    log(INFO, "<tr>\n");

    log(INFO, "<td>data [%p]:</td>", bucket->data);

    log(INFO, "\t");

    for (int i = 0; i < bucket->capacity; i++)
    {
        log(INFO, "<td>");

        void *item = (char *) bucket->data + num * bucket->item_size;
        const char *item_val = GetHashTableItemVal(item);

        log(INFO, "'%s'", item_val);

        log(INFO, "</td>\n");   
    }

    log(INFO, "</tr>\n");

    // next
    log(INFO, "<tr>\n");

    log(INFO, "<td>next [%p]:</td>", bucket->next);

    log(INFO, "\t");

    for (int i = 0; i < bucket->capacity; i++)
    {
        log(INFO, "<td>");

        if (bucket->next[i] == NEXT_POISON)
            log(INFO, "NX# ");

        if (bucket->next[i] == END_OF_FREE)
            log(INFO, END_OF_FREE_MARK);

        else
            log(INFO, "%3d ", bucket->next[i]);

        log(INFO, "</td>\n");
    }

    log(INFO, "</tr>\n");

    // prev
    log(INFO, "<tr>\n");

    log(INFO, "<td>prev [%p]:</td>", bucket->prev);

    log(INFO, "\t");

    for (int i = 0; i < bucket->capacity; i++)
    {
        log(INFO, "<td>");

        if (bucket->prev[i] == PREV_POISON)
            log(INFO, "PR# ");

        else
            log(INFO, "%3d ", bucket->prev[i]);

        log(INFO, "</td>\n");
    }
}


const char *GetHashTableItemVal(void *item)
{
    BucketItem *bucket_item = (BucketItem *) item;
    return bucket_item->word;
}