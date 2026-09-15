/* spdx_tag.h - разбор SPDX-тегов и сниппетов из файлов (C89) */
#ifndef SPDX_TAG_H
#define SPDX_TAG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Сниппет, извлечённый из одного файла. */
typedef struct {
    int  line_start;      /* номер строки с SPDX-SnippetBegin */
    int  line_end;        /* номер строки с SPDX-SnippetEnd */
    char *license;        /* SPDX-License-Identifier внутри сниппета; NULL, если не найден */
    char *copyright;      /* SPDX-SnippetCopyrightText (строки склеены через '\n'); NULL, если нет */
} TagSnippet;

typedef struct {
    TagSnippet *items;
    int count;
    int capacity;
} TagSnippetList;

void tag_snippets_init(TagSnippetList *list);
void tag_snippets_free(TagSnippetList *list);

/* Возвращает 1, если в файле есть тег SPDX-License-Identifier вне блоков
 * REUSE-IgnoreStart / REUSE-IgnoreEnd. */
int file_has_spdx_tag(const char *filename);

/* Возвращает значение тега SPDX-License-Identifier (malloc-строка,
 * caller free) или NULL. */
char *file_get_spdx_license(const char *filename);

/* Возвращает склеенные через '\n' значения SPDX-FileCopyrightText /
 * Copyright / © (malloc-строка, caller free) или NULL. */
char *file_get_spdx_copyright(const char *filename);

/* Извлекает все сниппеты из файла.
 * Возвращает 0 при успехе, -1 при ошибке (сообщение напечатано в stderr).
 * Список *out инициализируется внутри; caller освобождает через
 * tag_snippets_free. */
int file_get_snippets(const char *filename, TagSnippetList *out);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_TAG_H */
