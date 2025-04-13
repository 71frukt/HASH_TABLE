#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "hash_table.h"

int main()
{
    fprintf(stderr, "START\n\n");

    HashTable hash_table = {};
    ERROR_HANDLER(HashTableCtor(&hash_table, BUCKETS_COUNT, 10));

    // const char *const word = "SOSAL!";
    // 
    // BucketItem *item = GetOrCreateItem(&hash_table, word);
// 
    // fprintf(stderr, "AAA = '%s'\n", item->word);

    FILE *source = fopen("hash_table/build/source.txt", "r");
    ERROR_HANDLER(LoadHashTable(&hash_table, source));

    ERROR_HANDLER(HashTableDtor(&hash_table));

    fprintf(stderr, "END\n\n");

    // ListPasteAfter(&list, 10, 0);
    // ListPasteHead(&list, 20);
    // ListPasteHead(&list, 30);
    // ListPasteHead(&list, 100);
    // ListPasteHead(&list, 200);

    // // ListDelElem(&list, 5);
    // // ListDelElem(&list, 4);
    // // ListDelElem(&list, 3);
    // // ListDelElem(&list, 2);
    // // ListDelElem(&list, 1);

    // ListPasteAfter(&list, 150, 1);
    // ListPasteAfter(&list, 350, 3);
    // ListPasteAfter(&list, 250, 2);

    // ListPasteTail(&list, 444);
    // ListPasteTail(&list, 555);
    // ListPasteTail(&list, 666);

    // ListDtor(&list);
}