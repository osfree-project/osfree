/* reuse.h - REUSE.toml parser with hierarchy support (C89) */
#ifndef REUSE_H
#define REUSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Режимы приоритета аннотации                                         */
/* ------------------------------------------------------------------ */

#define REUSE_PRECEDENCE_CLOSEST   1
#define REUSE_PRECEDENCE_AGGREGATE 2
#define REUSE_PRECEDENCE_OVERRIDE  3

/* ------------------------------------------------------------------ */
/* Структуры                                                           */
/* ------------------------------------------------------------------ */

typedef struct {
    char **paths;
    int path_count;
    char *license;
    char *copyright;

    /* SPDX-FileContributor: массив строк (каждая — отдельный contributor). */
    char **contributors;
    int contributor_count;

    /* SPDX-Package*: скаляры. */
    char *package_name;
    char *package_supplier;
    char *package_download_location;
    char *package_comment;

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

/* Результат разрешения аннотации для конкретного файла.
 * Все строковые поля — malloc, освобождаются через reuse_resolved_free. */
typedef struct {
    char *license;
    char *copyright;
    char *contributors;               /* '\n'-separated, дедуплицировано */
    char *package_name;
    char *package_supplier;
    char *package_download_location;
    char *package_comment;
    int precedence;                   /* REUSE_PRECEDENCE_* или 0 */
    int has_reuse;                    /* 1, если хоть одна аннотация совпала */
} ReuseResolved;

void reuse_resolved_free(ReuseResolved *r);

/* ------------------------------------------------------------------ */
/* Работа с одним REUSE.toml (совместимость со старым API)             */
/* ------------------------------------------------------------------ */

ReuseConfig* parse_reuse_toml(const char *filename);
void free_reuse_config(ReuseConfig *config);
char* find_reuse_toml_upwards(const char *start_dir);

/* ------------------------------------------------------------------ */
/* Работа с иерархией REUSE.toml                                       */
/* ------------------------------------------------------------------ */

/* Находит все REUSE.toml от repo_root до target_dir включительно.
 * Порядок: сначала корневой, потом вложенные.
 * Если repo_root == NULL, читается только <target_dir>/REUSE.toml.
 * Возвращает 0 при успехе (даже если файлов нет). */
int reuse_find_all_tomls(const char *repo_root,
                         const char *target_dir,
                         SpdxStrList *out_paths);

/* Парсит все REUSE.toml из списка paths. Глубина проставляется по порядку.
 * Возвращает массив конфигов (malloc), *out_count — количество.
 * *out_error_count — сколько файлов не удалось распарсить (ERROR уже
 * напечатан в stderr самим parse_reuse_toml).
 * Отсутствующие файлы просто пропускаются. */
ReuseConfig **reuse_parse_all(const SpdxStrList *paths,
                              int *out_count,
                              int *out_error_count);

void reuse_free_all(ReuseConfig **configs, int count);

/* Разрешает license/copyright/contributors/package_* для файла с учётом
 * иерархии и precedence.
 *
 * in_license / in_copyright — информация из sidecar или тегов файла
 * (может быть NULL). Для contributors и package_* in-file источников нет.
 *
 * Приоритет: override > aggregate > closest.
 * При равном precedence выигрывает более глубокая аннотация.
 * При равной глубине — последняя в файле.
 *
 * out инициализируется внутри (memset 0). Все строковые поля —
 * malloc; освобождаются через reuse_resolved_free.
 *
 * Возвращает 0 всегда (кроме out == NULL). */
int reuse_resolve_for_file(ReuseConfig **configs, int count,
                           const char *filename,
                           const char *in_license,
                           const char *in_copyright,
                           ReuseResolved *out);

/* ------------------------------------------------------------------ */
/* Совместимость со старым API                                         */
/* ------------------------------------------------------------------ */

const char* find_license_for_file(ReuseConfig *config, const char *filename);

/* Сопоставление с поддержкой * и ** по спецификации REUSE. */
int matches_pattern(const char *pattern, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_H */
