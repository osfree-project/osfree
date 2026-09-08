/* omf_parser.c - точный разбор OMF для извлечения списка исходных файлов (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "omf_parser.h"

/* Чтение 2-байтового little-endian беззнакового числа */
static unsigned int read_u16(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

/* Добавление строки в динамический массив с проверкой на дубликаты */
static int add_unique_string(char ***list, int *count, const char *str, size_t len) {
    int i;
    char *copy;
    char **new_list;

    for (i = 0; i < *count; i++) {
        if (strlen((*list)[i]) == len && memcmp((*list)[i], str, len) == 0) {
            return 0; /* уже есть */
        }
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

/* Обработка записи THEADR (0x80) — содержит имя модуля (обычно исходный файл) */
static void process_theadr(const unsigned char *data, size_t len,
                           char ***sources, int *count) {
    size_t pos = 0;
    size_t name_len;

    if (len < 1) return;
    name_len = data[pos];
    pos++;
    if (pos + name_len > len) return;
    if (name_len > 0) {
        add_unique_string(sources, count, (const char*)&data[pos], name_len);
    }
}

/* Обработка записи COMENT (0x88) с классами, хранящими имена файлов */
static void process_coment(const unsigned char *data, size_t len,
                           char ***sources, int *count) {
    unsigned char comment_type;
    unsigned char comment_class;
    size_t pos;
    size_t payload_len;

    if (len < 2) return;

    /* Первый байт — тип комментария, второй — класс */
    comment_type = data[0];
    comment_class = data[1];
    pos = 2;
    payload_len = len - 2;

    switch (comment_class) {
        case 0xE9:    /* Borland auto-dependency: timestamp (4) + [len][string]... */
        case 0xFB: {  /* OpenWatcom dependency: timestamp (4) + [len][string]... */
            if (payload_len < 4) return;
            pos += 4;   /* пропускаем timestamp */

            while (pos + 1 <= len) {
                size_t str_len = data[pos];
                pos++;
                if (str_len == 0) continue;
                if (pos + str_len > len) break;
                add_unique_string(sources, count, (const char*)&data[pos], str_len);
                pos += str_len;
            }
            break;
        }
        case 0x88: {  /* Dependency File: timestamp (4) + [len][name] */
            if (payload_len < 5) return;
            pos += 4; /* timestamp */
            if (pos < len) {
                size_t name_len = data[pos];
                pos++;
                if (pos + name_len <= len && name_len > 0) {
                    add_unique_string(sources, count, (const char*)&data[pos], name_len);
                }
            }
            break;
        }
        case 0xE8: {  /* Borland source file: [len][name] */
            if (payload_len < 1) return;
            if (pos < len) {
                size_t name_len = data[pos];
                pos++;
                if (pos + name_len <= len && name_len > 0) {
                    add_unique_string(sources, count, (const char*)&data[pos], name_len);
                }
            }
            break;
        }
        default:
            break;
    }
}

/* Основная функция: извлечение исходных файлов из OMF-объекта */
int omf_extract_sources(const char *obj_path, char ***sources, int *count) {
    FILE *fp;
    unsigned char *buf;
    long file_size;
    size_t read_size;
    size_t pos = 0;

    *sources = NULL;
    *count = 0;

    fp = fopen(obj_path, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (file_size <= 0) {
        fclose(fp);
        return -1;
    }

    buf = (unsigned char*)malloc(file_size);
    if (!buf) {
        fclose(fp);
        return -1;
    }
    read_size = fread(buf, 1, file_size, fp);
    fclose(fp);
    if (read_size != (size_t)file_size) {
        free(buf);
        return -1;
    }

    /* Последовательный разбор OMF-записей без принудительного выравнивания */
    while (pos + 3 <= (size_t)file_size) {
        unsigned char record_type = buf[pos];
        size_t record_len;
        size_t data_start;
        size_t data_len;

        pos++;
        record_len = read_u16(&buf[pos]);
        pos += 2;

        if (pos + record_len > (size_t)file_size) {
            break;
        }
        data_start = pos;
        data_len = record_len;
        pos += record_len;

        switch (record_type) {
            case 0x80: /* THEADR */
                process_theadr(&buf[data_start], data_len, sources, count);
                break;
            case 0x88: /* COMENT */
                process_coment(&buf[data_start], data_len, sources, count);
                break;
            default:
                break;
        }
    }

    free(buf);

    if (*count > 0) return 0;
    return -1;
}
