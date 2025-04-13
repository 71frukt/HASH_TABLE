#include <stdio.h>

#include "list.h"

extern FILE *LogFile;

int main()
{
    fprintf(stderr, "START\n\n");

    list_t list = {};
        
    ListCtor(&list, BASE_LIST_CAPA);
    LIST_DUMP(&list);

    // ListPasteHead(&list, 10);
    ListPasteAfter(&list, 10, 0);
    ListPasteHead(&list, 20);
    ListPasteHead(&list, 30);
    ListPasteHead(&list, 100);
    ListPasteHead(&list, 200);

    // ListDelElem(&list, 5);
    // ListDelElem(&list, 4);
    // ListDelElem(&list, 3);
    // ListDelElem(&list, 2);
    // ListDelElem(&list, 1);

    ListPasteAfter(&list, 150, 1);
    ListPasteAfter(&list, 350, 3);
    ListPasteAfter(&list, 250, 2);

    ListPasteTail(&list, 444);
    ListPasteTail(&list, 555);
    ListPasteTail(&list, 666);

    fprintf(stderr, "num in data = %d\n", GetNumInData(&list, 7));

    LIST_DUMP(&list);
    LIST_DUMP(&list);
    ListDtor(&list);

    fprintf(stderr, "END!\n");
    return 0;
}