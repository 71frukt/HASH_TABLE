#ifndef HASH_FUNCS_H
#define HASH_FUNCS_H

#include <stdio.h>
#include <stdint.h>

size_t SimpleHash(const __m256i *const str_256);
uint32_t Murmurhash_32(const char *const key);

uint32_t XXH32(const char *const input_str);

#endif