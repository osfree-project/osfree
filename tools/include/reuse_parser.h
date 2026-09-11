/* reuse_parser.h - парсер REUSE.toml с поддержкой иерархии (C89) */
#ifndef REUSE_PARSER_H
#define REUSE_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Режимы приоритета аннотации */
#define REUSE_PRECEDENCE_CLOSEST   1
#define REUSE_PRECEDENCE_AGGREGATE 2
#define REUSE_PRECEDENCE_OVERRIDE  3

typedef struct {
    char **paths;
    int path_count;
    char *license;
    char *copyright;
    int precedence;    /* REUSE_PRECEDENCE_* */
    int order_in_file; /* порядковый номер [[annotations]] в файле */
} Annotation;

typedef struct {
    Annotation *annotations;
    int annotation_count;
    char *source_dir;  /* каталог, где лежит этот REUSE.toml */
    int depth;         /* 0 - корень, больше - глубже */
    int version;       /* должен быть 1 */
} ReuseConfig;

/* --- Работа с одним REUSE.toml (совместимость со старым API) --- */

ReuseConfig* parse_reuse_toml(const char *filename);
void free_reuse_config(ReuseConfig *config);
char* find_reuse_toml_upwards(const char *start_dir);

/* --- Работа с иерархией REUSE.toml --- */

/* Находит все REUSE.toml от repo_root до target_dir включительно.
 * Порядок: сначала корневой, потом вложенные.
 * Если repo_root == NULL, читается только <target_dir>/REUSE.toml.
 * Возвращает 0 при успехе (даже если файлов нет). */
int reuse_find_all_tomls(const char *repo_root,
                         const char *target_dir,
                         SpdxStrList *out_paths);

/* Парсит все REUSE.toml из списка paths. Глубина проставляется по порядку.
 * Возвращает массив конфигов (malloc), *out_count — количество.
 * Отсутствующие файлы просто пропускаются. */
ReuseConfig **reuse_parse_all(const SpdxStrList *paths, int *out_count);

void reuse_free_all(ReuseConfig **configs, int count);

/* Разрешает license/copyright для файла с учётом иерархии и precedence.
 *
 * in_license / in_copyright — информация из sidecar или тегов файла (может быть NULL).
 *
 * Возвращает:
 *   *out_license, *out_copyright — malloc-строки или NULL (caller free).
 *   *out_precedence — REUSE_PRECEDENCE_* выигравшей аннотации (0, если ни одна не совпала).
 *   *out_has_reuse — 1, если хоть одна аннотация совпала.
 *
 * Приоритет: override > aggregate > closest.
 * При равном precedence выигрывает более глубокая аннотация.
 * При равной глубине — последняя в файле. */
int reuse_resolve_for_file(ReuseConfig **configs, int count,
                           const char *filename,
                           const char *in_license,
                           const char *in_copyright,
                           char **out_license,
                           char **out_copyright,
                           int *out_precedence,
                           int *out_has_reuse);

/* --- Совместимость со старым API --- */

const char* find_license_for_file(ReuseConfig *config, const char *filename);
const char* find_copyright_for_file(ReuseConfig *config, const char *filename);

/* Сопоставление с поддержкой * и ** по спецификации REUSE. */
int matches_pattern(const char *pattern, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_PARSER_H */
