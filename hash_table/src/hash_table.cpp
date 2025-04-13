#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hash_table.h"
#include "hash_table_debug.h"

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
        ListCtor(hash_table->buckets + i, load_factor, sizeof(BucketItem));
    }
    
    HASH_TABLE_VERIFY(hash_table);
    return HASH_FUNC_OK;
}

HashTableFuncRes HashTableDtor(HashTable *hash_table)
{
    for (size_t i = 0; i < hash_table->buckets_count; i++)
    {
        ListDtor(hash_table->buckets + i);
    }

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

    char cur_word[DEFAULT_WORD_LEN] = {};

    while (true)
    {
        SkipSpaces(source);

        if (feof(source))
            break;

        fscanf(source, "%" STR(DEFAULT_WORD_LEN) "[a-zA-Z]", cur_word);

        BucketItem *item = GetOrCreateItem(hash_table, cur_word);
        fprintf(stderr, "item = '%s'\n", item->word);
    }

    HASH_TABLE_VERIFY(hash_table);

    return HASH_FUNC_OK;
}

BucketItem *GetOrCreateItem(HashTable *hash_table, const char *const word)
{
    size_t word_hash  = HashFunc(word);
    size_t bucket_num = word_hash % hash_table->buckets_count;

    list_t *bucket = hash_table->buckets + bucket_num;

    int item_index = bucket->head;
    do
    {
        BucketItem *item = (BucketItem *) ListGetItem(bucket, item_index);  //  bucket->data[item_index];

        if (strcmp(word, item->word) == 0)
        {
            item->val++;
            return item;
        }

        item_index = bucket->next[item_index];
    } 
    while (item_index != bucket->tail);

    // if didn't find
    BucketItem new_item = {};
    strncpy(new_item.word, word, DEFAULT_WORD_LEN - 1);
    new_item.val = 1;

    ListPasteTail(bucket, &new_item);
    
    return (BucketItem *) ListGetItem(bucket, item_index);
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