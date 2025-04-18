# Хеш-таблица
Шелонин Арсений, 1 курс ФРКТ МФТИ

## Описание проекта
Проект представляет собой базовую версию хеш-таблицы, элементами которой являются слова из текста. Программа загружает в хеш-таблицу данные из файла, а затем производит поиск определенных слов. Цель проекта - исследовать программу на быстродействие горячих функций и локальность с помощью профилировщиков и улучшить горячие функции тремя способами: ассемблерными вставками, интринсиками и отдельными ассемблерными функциями.

## Устройство хеш-таблицы
Наша хеш-таблица представляет из себя массив двусвязных списков (бакетов), каждый из которых содержит некоторое количество нод (далее будем называть это количество load-фактором). Нода представляет собой пару key (слово) - value (количество встреч).

<image src="readme_images/h_t_explanation.png" alt="Пример работы middle-end части" width=50%>

Итак, на вход хеш-таблице подаётся некоторое слово, затем некоторая хеш-функция сопоставляет этому слову хеш-код, по которому мы определяем соответствующий слову бакет. Таким образом, теперь нам надо искать слово не среди всех слов текста, а только среди слов данного бакета. В реальности load-фактор стараются делать около двух - в таком случае и список не нужен - но мы в учебных целях сделаем его около 10. 

На вход хеш-таблице подадим файл, состоящий из текста [Властелина Колец](https://www.cole13.com/wp-content/uploads/2023/08/Tolkien-J.-R.-R.-The-Fellowship-of-the-Ring-Tolkien-J.-R.-R.-Tolkien-J.-R.-R.-Z-Library.pdf), четырех томов [Войны и мира](https://www.gutenberg.org/files/2600/2600-h/2600-h.htm), [Сборника работ Шекспира](https://www.gutenberg.org/files/100/100-h/100-h.htm) и [Гарри Поттера](https://gist.githubusercontent.com/cmaspi/41e1d8e552a30a6d5ef0be7e574da513/raw/0a9a8247da3468a7a40edc2c62479df208c421d9/Harry_Potter_all_books_preprocessed.txt), повторенных несколько раз. В сумме получился файл на ~18 млн. слов.

### Про количество бакетов и load-фактор
Подберем количество бакетов так, чтобы load-фактор был около 10. 
Поставим некоторое количество бакетов `BUCKET_NUM`, загрузим хеш-таблицу и рассчитаем средний load-factor `load_factor_average` и среднеквадратическое отклонение $\sigma$

| BUCKET_NUM | load_factor_average | $\sigma$ |
|------------|---------------------|----------|
| 1009       |  41.6               | 6.5      |
| 2011       |  20.9               | 4.4      |
| 3001       |  14.1               | 3.7      |
| 4507       |  9.3                | 3.1      |

### Про хеш-функцию

Итак, мы получили, что количество бакетов, равное 4507, оптимально для нашей задачи. Однако мы можем заметить, что среднеквадратическое отклонение довольно велико. Получившееся $\sigma = 3.1$ в последнем эксперименте говорит о том, что:

| отклонение | промежуток    | вероятность попадания в промежуток |
|------------|---------------|------------------------------------|
| $\sigma/2$ | $[7.8, 11.0]$ | $38\%$                             |
| $\sigma$   | $[6.3, 12.5]$ | $62\%$                             |
| $2\sigma$  | $[3.2, 15.6]$ | $95\%$                             |
| $3\sigma$  | $[0.1, 18.7]$ | $99.7\%$                           |

<image src="readme_images/3sigma.png" alt="Пример работы middle-end части" width=50%>

Это далеко от идеала, ведь, даже если мы сделаем начальный load-фактор равным 19, реаллоцировать придётся 0.3% бакетов - это около 14 реаллоков, загружена будет где-то половина выделенной памяти, а разброс значений довольно велик.

В этом эксперименте я использовал самую обычную полиномиальную функцию `SimpleHash`:
<details>
<summary>SimpleHash</summary>

```C
SimpleHash(const char *const str)
{
    size_t hash = 5381;

    for (size_t i = 0; str[i] != '\0'; i++)
    {
        hash = hash * 33 + str[i];
    }

    return hash;
}
```

</details>

<br>

Она довольно проста и в целом имеет хорошее распределение, но попробуем найти функцию получше.

Используем известные `Murmurhash` и `XXH32`:

<details>
<summary>Murmurhash_32</summary>

```C
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
```
</details>

<details>
<summary>XXH32</summary>

```C

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

```

</details>

<br>

Для них получаем такой результат:

|      хеш-функция  |  $\sigma$        |
|-------------------|------------------|
| Murmurhash        |  3.077           |
| XXH32             |  3.045           |
| SimpleHash        |  3.092           |

Интересно, что значения $\sigma$ почти не отличаются. Тогда смысл переходить на какую-то более сложную другую функцию отпадает и мы будем использовать `SimpleHash`. Начальный load-фактор зададим как load_factor_average + $\sigma  \approx 19$. Тогда в теории на $4507$ бакетов должно вызваться около $0.3\% * 4507 = 14$ реаллоков. Проверим теорию практикой - создадим статическую переменную в функции реаллокации списка и посчитаем количество её вызовов. Запустив программу получим $17$ вызовов - очень близко с теорией. 


## Параметры системы

### Эталонная машина
* Процессор - Intel Core Ultra 5 125H
* Частота процессора при измерениях - 2000MHz
* OC - Manjaro Linux

#### Про запуск программы
Для корректных и точных расчётов количества тактов, затраченных на работу программы, выполним два действия:
* Установим стационарную частоту процессора по ядрам:
```bash
$ sudo cpupower frequency-set -u 2000MHz    # верхняя граница
$ sudo cpupower frequency-set -d 2000MHz    #  нижняя граница
```

* Установим наивысший приоритет нашей программе, используя утилиту `nice` с флагами `-n -20`. Запускать программу будем так:


Программа запускалась и тестировалась следующими программами:

| Программа       |  Версия         |
|-----------------|-----------------|
| gcc             | 14.2.1          |
| valgrind        | 3.24.0          |
| perf            | 6.14-1          |
| cpupower        | 6.14-1          |
| nice            | 9.6             |


## Ход работы

Для начала клонируйте репозиторий:
```bash
$ git clone https://github.com/71frukt/HASH_TABLE.git
```

Соберите проект:
```bash
$ make
```

* `make run` - запуск программы  
* `make rebuild` - пересобрать  
* `make clean` - очистить
* `make gdb` - запуск под gdb
* `make callgrind` - запуск под valgrind

Аргументы make:

### Метод анализа производительности программы в тактах процессора

Итак, запустим начальную версию программы. Для анализа числа тактов я написал скрипт на python, который делает 10 запусков программы под perf и считает среднее значение и погрешность.  

Запуск скрипта:
```bash
$ python benchmark/perf_profile_all_cores.py
```

Результат:

| Запуск   | Общее количество тактов | Тактов E-cores         | Тактов P-cores        |
|----------|------------------------:|-----------------------:|----------------------:|
| 1        |  11 842 742 271         |   4 190 855 759        |   7 651 886 512       |
| 2        |  13 892 908 985         |   6 584 327 374        |   7 308 581 611       |
| 3        |  14 061 100 715         |   6 701 511 238        |   7 359 589 477       |
| 4        |  14 273 419 125         |   6 852 480 163        |   7 420 938 962       |
| 5        |   7 348 962 277         |   0                    |   7 348 962 277       |
| 6        |  14 585 163 554         |   7 068 434 723        |   7 516 728 831       |
| 7        |  15 387 739 682         |   7 580 490 269        |   7 807 249 413       |
| 8        |  14 261 759 687         |   6 898 544 150        |   7 363 215 537       |
| 9        |  10 884 443 832         |   3 630 746 417        |   7 253 697 415       |
| 10       |  14 499 025 945         |   7 061 277 480        |   7 437 748 465       |

* Среднее значение - 13 103 726 607
* Вариация - 0,18

Это никуда не годится, тем более трудно сказать, как процессор работает с энергоэффективными ядрами и как рассчитывать общее количество тактов. Можно заметить, что значение на энергоэффективных ядрах постоянно скачет, в то время как значение производительных остаётся почти неизменным. Тогда запустим программу исключительно на производительных ядрах. Для этого при запуске используем утилиту `taskset -c 0-7` - на моем процессоре это производительные ядра. 

Запуск второго скрипта (работа программы на производительных ядрах):
```bash
$ python benchmark/perf_profile_p_cores
```

| Запуск | Тактов P-cores  |
|--------|----------------:|
| 1      | 7 631 723 841   |
| 2      | 7 253 495 372   |
| 3      | 7 444 132 432   |
| 4      | 7 550 385 169   |
| 5      | 7 237 035 646   |
| 6      | 7 447 371 351   |
| 7      | 7 384 212 382   |
| 8      | 7 177 848 148   |
| 9      | 7 142 104 809   |
| 10     | 7 406 809 034   |

* Среднее значение - 7,367,511,818
* Вариация - 0,02

Теперь результаты имеют высокое качество.

