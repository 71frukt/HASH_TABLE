#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "hash_table.h"
#include "hash_funcs.h"
#include "hash_table_debug.h"
#include "list.h"

extern "C" void LoadBuckets(char *source_data, char *end_of_source_data, HashTable *hash_table);

HashTableFuncRes HashTableCtor(HashTable *hash_table, size_t buckets_count, size_t load_factor)
{
    hash_table->buckets_count = buckets_count;
    hash_table->load_factor = load_factor;

    hash_table->buckets = (list_t *) calloc(buckets_count, sizeof(list_t));

    if (hash_table->buckets == NULL)
    {
        fprintf(stderr, "error in calloc hash_table->buckets\n");
        return HASH_FUNC_FAIL;
    }

    for (size_t i = 0; i < buckets_count; i++)
    {
        ListCtor(&hash_table->buckets[i], load_factor, sizeof(BucketItem));
    }
    
    HASH_TABLE_VERIFY(hash_table);
    return HASH_FUNC_OK;
}

HashTableFuncRes HashTableDtor(HashTable *hash_table)
{
    for (size_t i = 0; i < hash_table->buckets_count; i++)
    {
        ListDtor(&hash_table->buckets[i]);
    }

    free(hash_table->buckets);

    hash_table->buckets_count = 0;
    hash_table->load_factor = 0;

    return HASH_FUNC_OK;
}

HashTableFuncRes LoadHashTable(HashTable *hash_table, FILE *source)
{
    HASH_TABLE_VERIFY(hash_table);

    if (source == NULL)
    {
        fprintf(stderr, "error: source file = NULL\n");
        return HASH_FUNC_FAIL;
    }
    
    while (true)
    {
        SkipSpaces(source);
        
        if (feof(source))
        break;
        
        char cur_word[DEFAULT_WORD_LEN] = {};
        fscanf(source, "%" STR(DEFAULT_WORD_LEN) "[a-zA-Z]", cur_word);
        __m256i cur_word_m256 = _mm256_loadu_si256((__m256i *)cur_word);
        
        BucketItem *item = LoadItem(hash_table, &cur_word_m256);
        lassert(item, "LoadItem failed");
        
        // fprintf(stderr, "item = '%s'\n", item->word);
    }

    HASH_TABLE_DUMP(hash_table);

    HASH_TABLE_VERIFY(hash_table);

    return HASH_FUNC_OK;
}



HashTableFuncRes LoadHashTable2(HashTable *hash_table, const char *const source_file_name)
{
    HASH_TABLE_VERIFY(hash_table);

    int source_fd = open(source_file_name, O_RDONLY);

    if (source_fd == -1) 
    {
        fprintf(stderr, "open file error\n");
        return HASH_FUNC_FAIL;
    }

    struct stat st;
    fstat(source_fd, &st);
    size_t source_size = st.st_size;

    char* source_data = (char *) mmap(NULL, source_size, PROT_READ, MAP_PRIVATE, source_fd, 0);
    char* end_of_data = source_data + source_size;

    if (source_data == MAP_FAILED)
    {
        fprintf(stderr, "mmap failed\n");
        close(source_fd);
        return HASH_FUNC_FAIL;
    }

    // while (source_data < end_of_data)
    // {
    //     if (!isalpha(*source_data++)) continue;
        
    //     int i = 0;
    //     char cur_word[DEFAULT_WORD_LEN] = {};

    //     while (isalpha(*source_data))
    //     {
    //         cur_word[i++] = *source_data;
    //         source_data++;
    //     }
        
    //     __m256i cur_word_m256 = _mm256_loadu_si256((__m256i *)cur_word);
    //     BucketItem *item = LoadItem(hash_table, &cur_word_m256);
        
    //     lassert(item, "LoadItem failed");
    // }

    LoadBuckets(source_data, end_of_data, hash_table);
    
    munmap(source_data, source_size);
    close(source_fd);

    HASH_TABLE_VERIFY(hash_table);

    return HASH_FUNC_OK;
}



BucketItem *LoadItem(HashTable *hash_table, const __m256i *const word_m256_ptr)
{
    lassert(hash_table, "hash_table = NULL");
    lassert(hash_table->buckets_count, "attempt to load in empty hash table");
    lassert(word_m256_ptr, "word_m256_ptr = NULL");

    __m256i word_m256 = _mm256_loadu_si256(word_m256_ptr);

    size_t word_hash  = Crc32(word_m256);
    size_t bucket_num = word_hash % hash_table->buckets_count;

    list_t *bucket = hash_table->buckets + bucket_num;

    int item_index = bucket->head;

    if (bucket->head != 0)
    {
        while (true)
        {
            lassert(item_index != 0, "item_index points on manager");

            BucketItem *item = (BucketItem *) ListGetItem(bucket, item_index);

            __m256i cmp_256   = _mm256_cmpeq_epi8(word_m256, item->word);
            int cmp_mask_bits = _mm256_movemask_epi8(cmp_256);

            if (cmp_mask_bits == -1)    // -1 = 0xFFF..F  (bytes are equal => bit in mask)
            {
                item->val++;

                // if (item->val >= ((BucketItem *) ListGetItem(bucket, bucket->prev[item_index]))->val && bucket->prev[item_index] != 0)
                // {
                //     int new_prev_index = bucket->prev[bucket->prev[item_index]];
                //     int new_next_index = bucket->prev[item_index];

                //     ListBind(bucket, new_next_index, bucket->next[item_index]);
                //     ListBind(bucket, new_prev_index, item_index);
                //     ListBind(bucket, item_index,     new_next_index);
                // }

                return item;
            }

            if (item_index == bucket->tail)
                break;

            item_index = bucket->next[item_index];
        }
    }

    // if didn't find
    BucketItem new_item = {};
    new_item.word = word_m256;

    new_item.val = 1;

    ListPasteTail(bucket, &new_item);
    return (BucketItem *) ListGetItem(bucket, bucket->tail);
}

BucketItem *FindItem(HashTable *hash_table, const __m256i *const word_m256_ptr)
{
    __m256i word_m256 = _mm256_loadu_si256(word_m256_ptr);

    size_t word_hash  = Crc32(word_m256);
    size_t bucket_num = word_hash % hash_table->buckets_count;

    list_t *bucket = hash_table->buckets + bucket_num;

    int item_index = bucket->head;

    if (bucket->head != 0)
    {
        while (true)
        {
            lassert(item_index != 0, "item_index points on manager");

            BucketItem *item = (BucketItem *) ListGetItem(bucket, item_index);

            __m256i cmp_256   = _mm256_cmpeq_epi8(word_m256, item->word);
            int cmp_mask_bits = _mm256_movemask_epi8(cmp_256);

            if (cmp_mask_bits == -1)    // -1 = 0xFFF..F  (bytes are equal => bit in mask)
                return item;

            if (item_index == bucket->tail)
                break;

            item_index = bucket->next[item_index];
        }
    }

    return NULL;
}

char SkipSpaces(FILE *file)     // returns first read alpha (or EOF) letter
{
    int c = fgetc(file);
    while (!feof(file) && !isalpha(c))
    {
        c = fgetc(file);
    }
    
    ungetc(c, file); 

    return c;
}

