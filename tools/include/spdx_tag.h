/* spdx_tag.h - общие функции для работы с SPDX-тегами (C89) */
#ifndef SPDX_TAG_H
#define SPDX_TAG_H

/* Проверяет, содержит ли файл строку "SPDX-License-Identifier:" */
int file_has_spdx_tag(const char *filename);

/* Извлекает значение лицензии из тега SPDX-License-Identifier.
   Возвращает указатель на строку, выделенную malloc, или NULL, если тег не найден.
   Вызывающий код должен освободить память с помощью free(). */
char *file_get_spdx_license(const char *filename);

/* Извлекает все значения SPDX-FileCopyrightText, разделённые символом '\n'.
   Возвращает строку, выделенную malloc, или NULL, если теги не найдены.
   Вызывающий код должен освободить память с помощью free(). */
char *file_get_spdx_copyright(const char *filename);

#endif
