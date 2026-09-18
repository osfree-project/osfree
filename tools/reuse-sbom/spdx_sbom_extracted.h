/* spdx_sbom_extracted.h - работа с LicenseRef-* (C89) */
#ifndef SPDX_SBOM_EXTRACTED_H
#define SPDX_SBOM_EXTRACTED_H

#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void extracted_init(ExtractedLicenseList *list);
void extracted_free(ExtractedLicenseList *list);

/* Добавляет запись с указанным id. Возвращает указатель или NULL.
 * Если запись с таким id уже есть - возвращает её. */
ExtractedLicenseInfo *extracted_add(ExtractedLicenseList *list, const char *id);

/* Поиск по id. NULL, если не найдено. */
ExtractedLicenseInfo *extracted_lookup(ExtractedLicenseList *list,
                                       const char *id);

/* Собирает все LicenseRef-*, встречающиеся в лицензиях файлов.
 * Для каждой записи:
 *   1. Если id есть в extra - читает текст из указанного файла.
 *   2. Иначе ищет <project_dir>/LICENSES/<id>.txt или <project_dir>/LICENSES/<id>
 *   3. Иначе - ошибка.
 * Возвращает 0 при успехе, -1 при ошибке (сообщение уже напечатано). */
int extracted_collect_from_files(ExtractedLicenseList *list,
                                 const FileList *files,
                                 const char *project_dir,
                                 const ExtractedLicenseSource *extra,
                                 int extra_count);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_EXTRACTED_H */
