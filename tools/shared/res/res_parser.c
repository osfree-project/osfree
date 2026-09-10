/* res_parser.c - извлечение списка исходных файлов из .res (C89)
 *
 * Формат .res (Microsoft Resource Compiler, используется OpenWatcom и
 * Borland):
 *
 *   RESOURCEHEADER:
 *     DWORD DataSize
 *     DWORD HeaderSize
 *     ResourceType
 *     ResourceName
 *     [padding до 4 байт]
 *
 *   ResourceType / ResourceName:
 *     - Если первый WORD == 0xFFFF: ordinal (следующий WORD - значение)
 *     - Иначе: length (WORD) и строка. Чаще всего UTF-16, но бывает и ASCII.
 *
 *   После заголовка идёт DataSize байт данных ресурса.
 *   После данных - padding до границы 4 байт.
 *
 * Нас интересует ресурс:
 *   тип  = 0x79 (DEP_LIST_TYPE)
 *   имя  = "EBWF_XFMMTUPPE" (DEP_LIST_NAME)
 *
 * Данные этого ресурса - последовательность записей DepInfo:
 *   DWORD time (Unix timestamp)
 *   WORD  len  (длина имени, включая завершающий '\0')
 *   char  name[len]
 *
 * Завершается записью time=0, len=0.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "res_parser.h"

#define DEP_LIST_TYPE 0x0079
static const char DEP_LIST_NAME[] = "EBWF_XFMMTUPPE";

/* ---------- Чтение little-endian примитивов ---------- */

static unsigned int read_u16le(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static unsigned long read_u32le(const unsigned char *p) {
    return (unsigned long)p[0] |
           ((unsigned long)p[1] << 8) |
           ((unsigned long)p[2] << 16) |
           ((unsigned long)p[3] << 24);
}

/* ---------- Динамический список строк ---------- */

static int add_unique(char ***list, int *count, const char *str, size_t len) {
    int i;
    char *copy;
    char **new_list;

    for (i = 0; i < *count; i++) {
        if (strlen((*list)[i]) == len && memcmp((*list)[i], str, len) == 0)
            return 0;
    }
    copy = (char*)malloc(len + 1);
    if (!copy) return -1;
    memcpy(copy, str, len);
    copy[len] = '\0';
    new_list = (char**)realloc(*list, (*count + 1) * sizeof(char*));
    if (!new_list) {
        free(copy);
        return -1;
    }
    *list = new_list;
    (*list)[*count] = copy;
    (*count)++;
    return 0;
}

/* ---------- Разбор заголовка ресурса ---------- */

static size_t read_res_header_field(const unsigned char *buf, size_t buf_size,
                                    size_t pos,
                                    int *is_ordinal,
                                    unsigned int *ordinal_value,
                                    const char *expect_name,
                                    int *matched) {
    unsigned int first;
    unsigned int wlen;
    size_t i;
    char name_buf[256];
    int is_utf16;

    if (matched) *matched = 0;
    if (is_ordinal) *is_ordinal = 0;
    if (ordinal_value) *ordinal_value = 0;

    if (pos + 2 > buf_size) return 0;
    first = read_u16le(buf + pos);

    if (first == 0xFFFF) {
        if (pos + 4 > buf_size) return 0;
        if (is_ordinal) *is_ordinal = 1;
        if (ordinal_value) *ordinal_value = read_u16le(buf + pos + 2);
        return pos + 4;
    }

    wlen = first;
    pos += 2;
    if (wlen == 0) return pos;

    if (pos + (size_t)wlen * 2 > buf_size) return 0;

    is_utf16 = 1;
    for (i = 0; i < wlen; i++) {
        if (buf[pos + i * 2 + 1] != 0) { is_utf16 = 0; break; }
    }

    if (wlen < sizeof(name_buf)) {
        if (is_utf16) {
            for (i = 0; i < wlen; i++)
                name_buf[i] = (char)buf[pos + i * 2];
            name_buf[wlen] = '\0';
            if (expect_name && matched) {
                if (strcmp(name_buf, expect_name) == 0) *matched = 1;
            }
        } else {
            if (pos + wlen > buf_size) return 0;
            memcpy(name_buf, buf + pos, wlen);
            name_buf[wlen] = '\0';
            if (expect_name && matched) {
                if (strcmp(name_buf, expect_name) == 0) *matched = 1;
            }
        }
    }

    return pos + (size_t)wlen * 2;
}

/* ---------- Разбор данных ресурса DepInfo ---------- */

static int parse_depinfo(const unsigned char *data, size_t data_size,
                         char ***sources, int *count) {
    size_t pos = 0;

    while (pos + 6 <= data_size) {
        unsigned long time = read_u32le(data + pos);
        unsigned int  len  = read_u16le(data + pos + 4);
        pos += 6;

        if (time == 0 && len == 0) {
            return 0;
        }

        if (len == 0) break;
        if (pos + len > data_size) break;

        if (len > 1) {
            add_unique(sources, count, (const char*)(data + pos), len - 1);
        }
        pos += len;
    }
    return 0;
}

/* ---------- Основная функция ---------- */

int res_extract_sources(const char *res_path, char ***sources, int *count) {
    FILE *fp;
    unsigned char *buf;
    long file_size;
    size_t read_size;
    size_t pos = 0;
    int found = 0;

    *sources = NULL;
    *count = 0;

    fp = fopen(res_path, "rb");
    if (!fp) {
        fprintf(stderr, "res: cannot open %s\n", res_path);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (file_size <= 0) {
        fclose(fp);
        return -1;
    }

    buf = (unsigned char*)malloc((size_t)file_size);
    if (!buf) {
        fclose(fp);
        return -1;
    }
    read_size = fread(buf, 1, (size_t)file_size, fp);
    fclose(fp);
    if (read_size != (size_t)file_size) {
        free(buf);
        return -1;
    }

    while (pos + 8 <= (size_t)file_size) {
        unsigned long data_size;
        unsigned long header_size;
        size_t header_start;
        size_t header_end;
        int is_ordinal;
        unsigned int ordinal_value;
        int matched;
        size_t type_end;
        size_t name_end;
        size_t data_start;
        size_t next_pos;

        data_size   = read_u32le(buf + pos);
        header_size = read_u32le(buf + pos + 4);
        header_start = pos + 8;
        header_end = header_start + (size_t)header_size;

        if (header_end > (size_t)file_size) break;

        /* Тип */
        type_end = read_res_header_field(buf, (size_t)file_size, header_start,
                                         &is_ordinal, &ordinal_value,
                                         NULL, NULL);
        if (type_end == 0) break;
        while ((type_end % 4) != 0) type_end++;

        /* Имя */
        name_end = read_res_header_field(buf, (size_t)file_size, type_end,
                                         &is_ordinal, NULL,
                                         DEP_LIST_NAME, &matched);
        if (name_end == 0) break;
        while ((name_end % 4) != 0) name_end++;

        data_start = header_end;
        while ((data_start % 4) != 0) data_start++;

        if (data_start + data_size > (size_t)file_size) break;

        if (ordinal_value == DEP_LIST_TYPE && matched) {
            parse_depinfo(buf + data_start, (size_t)data_size, sources, count);
            found = 1;
        }

        next_pos = data_start + (size_t)data_size;
        while ((next_pos % 4) != 0) next_pos++;
        if (next_pos <= pos) break;
        pos = next_pos;
    }

    free(buf);

    if (!found) return -1;
    return 0;
}
