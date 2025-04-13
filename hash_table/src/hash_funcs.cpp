#include <stdio.h>

size_t HashFunc(const char *const str)
{
    size_t hash = 5381;

    for (size_t i = 0; str[i] != '\0'; i++)
    {
        hash = hash * 33 + str[i];
    }

    return hash;
}

