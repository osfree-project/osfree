/* spdx_utils.h - общие утилиты для SPDX-утилит (C89) */
#ifndef SPDX_UTILS_H
#define SPDX_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Динамический список строк */
typedef struct {
    char **items;
    int count;
    int capacity;
} SpdxStrList;

void spdx_strlist_init(SpdxStrList *list);
void spdx_strlist_add(SpdxStrList *list, const char *str);
void spdx_strlist_add_unique(SpdxStrList *list, const char *str);
int  spdx_strlist_contains(const SpdxStrList *list, const char *str);
void spdx_strlist_free(SpdxStrList *list);

/* Чтение файла целиком в malloc-буфер.
 * Возвращает NULL при ошибке. Если out_size != NULL, туда пишется размер. */
char *spdx_read_file_all(const char *path, long *out_size);

/* Указатель на базовое имя файла в пути. Никогда не NULL. */
const char *spdx_get_file_name(const char *path);

/* Разбор SPDX-выражения на отдельные идентификаторы (без AND/OR/WITH
 * и скобок). Добавляет в out, устраняя дубликаты. */
void spdx_expression_collect_ids(const char *expr, SpdxStrList *out);

/* Нормализация текста лицензии для сравнения:
 *   - удалить BOM в начале;
 *   - CRLF и одиночные CR -> LF;
 *   - удалить пробелы и табы в конце каждой строки;
 *   - удалить пустые строки в конце.
 * Возвращает malloc-строку (caller free) или NULL при OOM. */
char *spdx_normalize_text(const char *src);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_UTILS_H */
