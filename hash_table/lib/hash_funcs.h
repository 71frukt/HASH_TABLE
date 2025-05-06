#ifndef HASH_FUNCS_H
#define HASH_FUNCS_H

#include <stdio.h>
#include <stdint.h>

size_t   DJB2          (const __m256i *const str_256);
size_t   Crc32         (const __m256i str_256);
uint32_t Murmurhash_32 (const __m256i *const str_256);
uint32_t XXH32         (const char *const input_str);
size_t   YMM_HashFunc  (const __m256i data);

#endif