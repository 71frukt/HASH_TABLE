#include <stdio.h>
#include <string.h>

#include "hash_table.h"
#include "hash_funcs.h"
#include "logger.h"

size_t SimpleHash(const __m256i *const str_256)
{
    // fprintf(stderr, "easy");

    // return BUCKETS_COUNT * strlen(str) / 16;

    const char *const str = (const char *const) str_256;

    size_t hash = 5381;

    for (size_t i = 0; str[i] != '\0'; i++)
    {
        hash = hash * 33 + str[i];
    }

    return hash;
}

uint32_t Murmurhash_32(const char *const key)
{
    // fprintf(stderr, "mur");
    uint32_t str_len = strlen(key);

    uint32_t hash = 0;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t r1 = 15;
    const uint32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;

    const uint8_t *data = (const uint8_t *)key;
    const int nblocks = str_len / 4;

    // Обработка блоков по 4 байта
    for (int i = 0; i < nblocks; i++) {
        uint32_t k;
        memcpy(&k, data + i * 4, sizeof(k));

        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        hash ^= k;
        hash = (hash << r2) | (hash >> (32 - r2));
        hash = hash * m + n;
    }

    // Обработка оставшихся байтов
    const uint8_t *tail = data + nblocks * 4;
    uint32_t k1 = 0;

    switch (str_len & 3)
    {
        case 3: k1 ^= tail[2] << 16; [[fallthrough]];
        case 2: k1 ^= tail[1] << 8;  [[fallthrough]];
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2;
                hash ^= k1;
    }

    // Финальное перемешивание
    hash ^= str_len;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

    return hash;
}



static const unsigned int XXH_PRIME32_1 = 0x9E3779B1U;
static const unsigned int XXH_PRIME32_2 = 0x85EBCA77U;
static const unsigned int XXH_PRIME32_3 = 0xC2B2AE3DU;
static const unsigned int XXH_PRIME32_4 = 0x27D4EB2FU;
static const unsigned int XXH_PRIME32_5 = 0x165667B1U;

static uint32_t XXH_read32(const void* ptr);

static uint32_t XXH_read32(const void* ptr) 
{
    uint32_t val;
    memcpy(&val, ptr, sizeof(val));
    return val;
}

// Основная функция xxHash32
uint32_t XXH32(const char *const input_str) 
{
    size_t   len = strlen(input_str);
    uint32_t seed = 0;

    const uint8_t* data = (const uint8_t*)input_str;
    const uint8_t* end = data + len;
    uint32_t h32;

    if (len >= 16) {
        const uint8_t* limit = end - 16;
        uint32_t v1 = seed + XXH_PRIME32_1 + XXH_PRIME32_2;
        uint32_t v2 = seed + XXH_PRIME32_2;
        uint32_t v3 = seed + 0;
        uint32_t v4 = seed - XXH_PRIME32_1;

        // Обработка блоков по 16 байт
        do {
            v1 += XXH_read32(data) * XXH_PRIME32_2;
            v1 = (v1 << 13) | (v1 >> 19);
            v1 *= XXH_PRIME32_1;
            data += 4;

            v2 += XXH_read32(data) * XXH_PRIME32_2;
            v2 = (v2 << 13) | (v2 >> 19);
            v2 *= XXH_PRIME32_1;
            data += 4;

            v3 += XXH_read32(data) * XXH_PRIME32_2;
            v3 = (v3 << 13) | (v3 >> 19);
            v3 *= XXH_PRIME32_1;
            data += 4;

            v4 += XXH_read32(data) * XXH_PRIME32_2;
            v4 = (v4 << 13) | (v4 >> 19);
            v4 *= XXH_PRIME32_1;
            data += 4;
        } while (data <= limit);

        h32 = (v1 << 1) | (v1 >> 31);
        h32 += (v2 << 7) | (v2 >> 25);
        h32 += (v3 << 12) | (v3 >> 20);
        h32 += (v4 << 18) | (v4 >> 14);
    } 

    else 
    {
        h32 = seed + XXH_PRIME32_5;
    }

    // Добавляем оставшиеся байты
    h32 += (uint32_t)len;

    // Финальное перемешивание
    while (data + 4 <= end)
    {
        h32 += XXH_read32(data) * XXH_PRIME32_3;
        h32 = ((h32 << 17) | (h32 >> 15)) * XXH_PRIME32_4;
        data += 4;
    }

    while (data < end)
    {
        h32 += (*data) * XXH_PRIME32_5;
        h32 = ((h32 << 11) | (h32 >> 21)) * XXH_PRIME32_1;
        data++;
    }

    // Avalanche effect (улучшение распределения)
    h32 ^= h32 >> 15;
    h32 *= XXH_PRIME32_2;
    h32 ^= h32 >> 13;
    h32 *= XXH_PRIME32_3;
    h32 ^= h32 >> 16;

    return h32;
}
