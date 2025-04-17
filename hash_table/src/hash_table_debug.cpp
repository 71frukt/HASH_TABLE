#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "hash_table.h"
#include "hash_table_debug.h"
#include "logger.h"

HashTableVerifyCode HashTableVerify(HashTable *hash_table)
{
// fprintf(stderr, "IN VERIFY\n");

    if (hash_table == NULL)
        return HASH_TABLE_PTR_ERR;

    if (hash_table->buckets == NULL)
        return HASH_TABLE_BUCKETS_PTR_ERR;

    if (CheckHashTableAccordance(hash_table) == HASH_TABLE_ACCORDANCE_ERR)
        return HASH_TABLE_ACCORDANCE_ERR;

    return HASH_TABLE_OK;
}


HashTableVerifyCode CheckHashTableAccordance(HashTable *hash_table)
{
    for (size_t bucket_num = 0; bucket_num < hash_table->buckets_count; bucket_num++)
    {
        list_t *cur_bucket = hash_table->buckets + bucket_num;
        int item_index = cur_bucket->head;

        if (cur_bucket->head == 0)    // free bucket
            continue;

        while (true)
        {
            BucketItem *cur_item   = (BucketItem*) ListGetItem(cur_bucket, item_index);
            BucketItem *found_item = (BucketItem*) FindItem(hash_table, &cur_item->word);
            
            fprintf(stderr, "ver word = '%s', item_index = %d, bucket_num = %ld, buckets_count = %ld\n", &cur_item->word, item_index, bucket_num, hash_table->buckets_count);
            
            if (cur_item != found_item)
            {
                log(WARNING, "incorrectly find word: item_index = %d, bucket = %d, word = %s", item_index, bucket_num, &cur_item->word);
                return HASH_TABLE_ACCORDANCE_ERR;
            }

            if (item_index == cur_bucket->tail)
                break;
                
            item_index = cur_bucket->next[item_index];
        }
    }


    return HASH_TABLE_OK;
}

void Benchmark()
{
    HashTable hash_table = {};
    ERROR_HANDLER(HashTableCtor(&hash_table, BUCKETS_COUNT, LOAD_FACTOR));
    
    FILE *source = fopen("hash_table/build/source.txt", "r");

    ERROR_HANDLER(LoadHashTable(&hash_table, source));    
   
    volatile HashTableVerifyCode code = HASH_TABLE_OK;
    HashTableVerifyCode (*volatile wrapper)(HashTable *hash_table) = CheckHashTableAccordance;

    for (size_t i = 0; i < 1000; i++)
    {
        printf("i = %ld\n", i);
        code = wrapper(&hash_table);
    }
    
    ERROR_HANDLER(HashTableDtor(&hash_table));
    fclose(source);
}

char *GetHashTableErrors(int error)
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
            {                                                                                  \
                log(WARNING, "error_str overflow");                                             \
            }                                                                                    \
        }
        
    PRINT_ERROR_PART(error, HASH_TABLE_PTR_ERR);
    PRINT_ERROR_PART(error, HASH_TABLE_BUCKETS_PTR_ERR);
    PRINT_ERROR_PART(error, HASH_TABLE_ACCORDANCE_ERR);

    return error_str;

    #undef PRINT_ERROR_PART
}


void HashTableDump(HashTable *hash_table)
{
    log(INFO, "IN DUMP\n");

    log(LOG, "Hash table [%p]\n", hash_table);
    log(LOG, "{\n");

    log(LOG, "\tbuckets_count = %ld\n",   hash_table->buckets_count);    
    log(LOG, "\tload_factor   = %ld\n\n", hash_table->load_factor  );    
    
    for (size_t i = 0; i < hash_table->buckets_count; i++)
    {
        log(LOG, "\tBucket %ld:\n", i);
        BucketDump(hash_table->buckets + i);
    }
    
    log(LOG, "}\n\n\n");
}

void BucketDump(list_t *bucket)
{
    ON_LOGS(

    lassert(bucket, "bucket == NULL");

    // log(LOG, "bucket[%p]:   ", bucket);
    log(LOG, "\thead = %d\n", bucket->head);
    log(LOG, "\ttail = %d\n", bucket->tail);
    log(LOG, "\tfree = %d\n", bucket->free);

    // table
    log(LOG, "\n<table border width = \"85%%\"style=\"margin-left: 3%%\">\n");

    log(LOG, "<tr>\n");
    log(LOG, "<td>index</td>");

    for (int i = 0; i < bucket->capacity; i++)
    {
        log(LOG, "<td>%d</td>", i);
    }

    log(LOG, "</tr>\n");

    // data
    log(LOG, "<tr>\n");

    log(LOG, "<td>data [%p]:</td>\n", bucket->data);

    // manager
    log(LOG, "<td>");

    log(LOG, "%s", "manager");

    log(LOG, "</td>\n"); 

    for (int i = 1; i < bucket->capacity; i++)
    {
        log(LOG, "<td>");

        void *item = ListGetItem(bucket, i); // (char *) bucket->data + num * bucket->item_size;
        const char *item_val = GetHashTableItemVal(item);

        log(LOG, "%s", item_val);

        log(LOG, "</td>\n");   
    }

    log(LOG, "</tr>\n");

    // next
    log(LOG, "<tr>\n");

    log(LOG, "<td>next [%p]:</td>\n", bucket->next);

    for (int i = 0; i < bucket->capacity; i++)
    {
        log(LOG, "<td>");

        if (bucket->next[i] == NEXT_POISON)
            log(LOG, "NX# ");

        if (bucket->next[i] == END_OF_FREE)
            log(LOG, END_OF_FREE_MARK);

        else
            log(LOG, "%3d ", bucket->next[i]);

        log(LOG, "</td>\n");
    }

    log(LOG, "</tr>\n");

    // prev
    log(LOG, "<tr>\n");

    log(LOG, "<td>prev [%p]:</td>\n", bucket->prev);

    for (int i = 0; i < bucket->capacity; i++)
    {
        log(LOG, "<td>");

        if (bucket->prev[i] == PREV_POISON)
            log(LOG, "PR# ");

        else
            log(LOG, "%3d ", bucket->prev[i]);

        log(LOG, "</td>\n");
    }

    log(LOG, "</table>\n\n");

    )
}


const char *GetHashTableItemVal(void *item)
{
    static char item_val[ITEM_NAME_LEN] = {};

    BucketItem *bucket_item = (BucketItem *) item;
    
    snprintf(item_val, ITEM_NAME_LEN - 1, "'%s' (%ld)", (char *) &bucket_item->word, bucket_item->val);

    return item_val;
}

void LogBucketsLoadFactor(HashTable *hash_table)
{
    double load_factor_average = 0;

    for (size_t i = 0; i < hash_table->buckets_count; i++)
    {
        load_factor_average += hash_table->buckets[i].size;
        // fprintf(stderr, "size = %d\n", hash_table->buckets[i].size);
    }

    load_factor_average /= hash_table->buckets_count;

    double sigma = 0;

    for (size_t i = 0; i < hash_table->buckets_count; i++)
    {
        sigma += (hash_table->buckets[i].size - load_factor_average) * (hash_table->buckets[i].size - load_factor_average);
        // fprintf(stderr, "sigma = %f\n", sigma);
    }

    sigma = sqrt(sigma / hash_table->buckets_count);

    log(INFO, "load_factor_average = %f, sigma = %f\n", load_factor_average, sigma);
}