/* reuse.c - парсер REUSE.toml с поддержкой иерархии (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef __LINUX__
#include <unistd.h>
#else
#include <direct.h>
#include <io.h>
#endif

#include "reuse.h"
#include "toml.h"
#include "dep5.h"

/* ------------------------------------------------------------------ */
/* Утилиты                                                             */
/* ------------------------------------------------------------------ */

static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

/* Сравнение символов пути: регистрозависимо на Linux, регистронезависимо
 * на Windows. */
static int path_char_eq(int a, int b) {
#ifdef _WIN32
    return tolower((unsigned char)a) == tolower((unsigned char)b);
#else
    return (unsigned char)a == (unsigned char)b;
#endif
}

static int path_prefix_eq(const char *s, const char *prefix, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (!s[i]) return 0;
        if (!path_char_eq(s[i], prefix[i])) return 0;
    }
    return 1;
}

static void add_path(Annotation *ann, const char *value) {
    char **na = (char**)realloc(ann->paths,
        (size_t)(ann->path_count + 1) * sizeof(char*));
    if (!na) { fprintf(stderr, "ERROR: out of memory\n"); exit(EXIT_FAILURE); }
    ann->paths = na;
    ann->paths[ann->path_count++] = dup_str(value);
}

/* ------------------------------------------------------------------ */
/* Сопоставление паттернов (* и **)                                    */
/* ------------------------------------------------------------------ */

static int match_component(const char *pat, const char *str) {
    if (*pat == '\0') return *str == '\0';
    if (*pat == '*') {
        while (1) {
            if (match_component(pat + 1, str)) return 1;
            if (*str == '\0' || *str == '/') return 0;
            str++;
        }
    }
    if (*str == '\0' || *str == '/') return 0;
    if (*pat != *str) return 0;
    return match_component(pat + 1, str + 1);
}

static int match_path(const char *pat, const char *path) {
    const char *pat_slash;
    const char *path_slash;

    if (pat[0] == '*' && pat[1] == '*') {
        if (pat[2] == '\0') return 1;
        if (pat[2] == '/') {
            const char *rest = pat + 3;
            if (match_path(rest, path)) return 1;
            while ((path_slash = strchr(path, '/')) != NULL) {
                path = path_slash + 1;
                if (match_path(rest, path)) return 1;
            }
            return 0;
        }
    }

    pat_slash  = strchr(pat, '/');
    path_slash = strchr(path, '/');

    if (!pat_slash) {
        if (path_slash) return 0;
        return match_component(pat, path);
    }
    if (!path_slash) return 0;

    {
        size_t plen = (size_t)(pat_slash - pat);
        size_t flen = (size_t)(path_slash - path);
        char pat_seg[256];
        char path_seg[256];

        if (plen >= sizeof(pat_seg) || flen >= sizeof(path_seg)) return 0;
        memcpy(pat_seg, pat, plen);
        pat_seg[plen] = '\0';
        memcpy(path_seg, path, flen);
        path_seg[flen] = '\0';

        if (!match_component(pat_seg, path_seg)) return 0;
        return match_path(pat_slash + 1, path_slash + 1);
    }
}

int matches_pattern(const char *pattern, const char *filename) {
    return match_path(pattern, filename);
}

/* Возвращает путь filename относительно cfg->source_dir.
 * Проверяет, что совпадение префикса — на границе сегмента. */
static const char *rel_to_config(const ReuseConfig *cfg, const char *filename) {
    const char *base = cfg->source_dir;
    size_t blen;
    if (!base || !base[0]) return filename;
    blen = strlen(base);
    if (path_prefix_eq(filename, base, blen)) {
        const char *rest = filename + blen;
        if (*rest == '\0') return rest;
        if (*rest != '/' && *rest != '\\') return filename;
        while (*rest == '/' || *rest == '\\') rest++;
        return rest;
    }
    return filename;
}

/* ------------------------------------------------------------------ */
/* Разбор REUSE.toml (через библиотеку TOML v1.0.0)                    */
/*                                                                     */
/* Весь лексический и синтаксический разбор TOML выполняется в         */
/* toml.c. Здесь — только извлечение ключей REUSE-семантики из         */
/* дерева, построенного TomlOpen:                                      */
/*   - version = 1                    (REUSE 3.3 §4.1.1, обязателен)   */
/*   - [[annotations]]                (массив таблиц)                  */
/*   - path = <str> | [<str>, ...]    (обязателен в каждой аннотации)  */
/*   - SPDX-License-Identifier        (строка или массив)              */
/*   - SPDX-FileCopyrightText         (строка или массив)              */
/*   - SPDX-FileContributor           (строка или массив)              */
/*   - SPDX-PackageName               (строка)                         */
/*   - SPDX-PackageSupplier           (строка)                         */
/*   - SPDX-PackageDownloadLocation   (строка)                         */
/*   - SPDX-PackageComment            (строка)                         */
/*   - precedence = "closest"|"aggregate"|"override"                   */
/* ------------------------------------------------------------------ */

/* Возвращает malloc-строку со значением строкового узла или NULL
 * при ошибке. */
static char *node_to_str(HTOMLNODE hNode) {
    ULONG ulSize = 0;
    char *buf;
    APIRET rc;

    rc = TomlNodeGetString(hNode, NULL, 0, &ulSize);
    if (rc != TOML_NO_ERROR || ulSize == 0) return NULL;
    buf = (char*)malloc(ulSize);
    if (!buf) return NULL;
    rc = TomlNodeGetString(hNode, buf, ulSize, NULL);
    if (rc != TOML_NO_ERROR) { free(buf); return NULL; }
    return buf;
}

/* Читает скалярный строковый ключ из таблицы. Возвращает NULL, если
 * ключ отсутствует или имеет неверный тип. */
static char *read_scalar_string(HTOMLNODE hTable, PCSZ pszKey) {
    HTOMLNODE hChild = NULLHANDLE;
    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != TOML_NO_ERROR)
        return NULL;
    return node_to_str(hChild);
}

/* Читает ключ, значение которого — строка или массив строк. Если это
 * массив, элементы соединяются через pszSep. Возвращает malloc-строку
 * или NULL. */
static char *read_string_or_join(HTOMLNODE hTable, PCSZ pszKey,
                                 PCSZ pszSep) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, i;
    char *result = NULL;
    size_t sep_len = strlen(pszSep);

    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != TOML_NO_ERROR)
        return NULL;
    if (TomlNodeGetType(hChild, &ulType) != TOML_NO_ERROR)
        return NULL;

    if (ulType == TOML_TYPE_STRING) {
        return node_to_str(hChild);
    }
    if (ulType != TOML_TYPE_ARRAY) return NULL;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (i = 0; i < ulCount; i++) {
        HTOMLNODE hElem = NULLHANDLE;
        char *part;
        size_t rlen, plen;
        char *nresult;

        if (TomlNodeGetArrayElement(hChild, i, &hElem) != TOML_NO_ERROR) {
            free(result);
            return NULL;
        }
        part = node_to_str(hElem);
        if (!part) { free(result); return NULL; }

        rlen = result ? strlen(result) : 0;
        plen = strlen(part);
        nresult = (char*)realloc(result,
            rlen + (rlen ? sep_len : 0) + plen + 1);
        if (!nresult) { free(part); free(result); return NULL; }
        result = nresult;
        if (rlen) {
            memcpy(result + rlen, pszSep, sep_len);
            rlen += sep_len;
        }
        memcpy(result + rlen, part, plen);
        result[rlen + plen] = '\0';
        free(part);
    }
    return result;
}

/* Читает ключ "path": строка или массив строк. Каждый элемент
 * добавляется в ann->paths. */
static int read_paths_into(HTOMLNODE hTable, Annotation *ann) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, i;

    if (TomlNodeGetTableEntryByKey(hTable, "path", &hChild) != TOML_NO_ERROR)
        return 0;   /* нет path — аннотация не будет создана */
    if (TomlNodeGetType(hChild, &ulType) != TOML_NO_ERROR) return -1;

    if (ulType == TOML_TYPE_STRING) {
        char *s = node_to_str(hChild);
        if (!s) return -1;
        add_path(ann, s);
        free(s);
        return 0;
    }
    if (ulType != TOML_TYPE_ARRAY) return -1;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (i = 0; i < ulCount; i++) {
        HTOMLNODE hElem = NULLHANDLE;
        char *s;
        if (TomlNodeGetArrayElement(hChild, i, &hElem) != TOML_NO_ERROR)
            return -1;
        s = node_to_str(hElem);
        if (!s) return -1;
        add_path(ann, s);
        free(s);
    }
    return 0;
}

/* Читает ключ "SPDX-FileContributor" как список отдельных
 * участников. */
static int read_contributors_into(HTOMLNODE hTable, Annotation *ann) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, i;

    if (TomlNodeGetTableEntryByKey(hTable, "SPDX-FileContributor",
                                   &hChild) != TOML_NO_ERROR)
        return 0;
    if (TomlNodeGetType(hChild, &ulType) != TOML_NO_ERROR) return -1;

    if (ulType == TOML_TYPE_STRING) {
        char *s = node_to_str(hChild);
        char **na;
        if (!s) return -1;
        na = (char**)realloc(ann->contributors,
                             (size_t)(ann->contributor_count + 1) *
                             sizeof(char*));
        if (!na) { free(s); return -1; }
        ann->contributors = na;
        ann->contributors[ann->contributor_count++] = s;
        return 0;
    }
    if (ulType != TOML_TYPE_ARRAY) return -1;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (i = 0; i < ulCount; i++) {
        HTOMLNODE hElem = NULLHANDLE;
        char *s;
        char **na;
        if (TomlNodeGetArrayElement(hChild, i, &hElem) != TOML_NO_ERROR)
            return -1;
        s = node_to_str(hElem);
        if (!s) return -1;
        na = (char**)realloc(ann->contributors,
                             (size_t)(ann->contributor_count + 1) *
                             sizeof(char*));
        if (!na) { free(s); return -1; }
        ann->contributors = na;
        ann->contributors[ann->contributor_count++] = s;
    }
    return 0;
}

/* Читает ключ "precedence". Неизвестные значения игнорируются. */
static int read_precedence(HTOMLNODE hTable, Annotation *ann) {
    char *s = read_scalar_string(hTable, "precedence");
    if (!s) return 0;
    if (strcmp(s, "closest") == 0)
        ann->precedence = REUSE_PRECEDENCE_CLOSEST;
    else if (strcmp(s, "aggregate") == 0)
        ann->precedence = REUSE_PRECEDENCE_AGGREGATE;
    else if (strcmp(s, "override") == 0)
        ann->precedence = REUSE_PRECEDENCE_OVERRIDE;
    free(s);
    return 0;
}

/* Разбирает одну аннотацию из таблицы [[annotations]]. */
static int parse_one_annotation(HTOMLNODE hItem, Annotation *ann,
                                PCSZ pszFilename) {
    memset(ann, 0, sizeof(*ann));
    ann->precedence = REUSE_PRECEDENCE_CLOSEST;

    if (read_paths_into(hItem, ann) != 0) {
        fprintf(stderr, "ERROR: %s: invalid 'path' in [[annotations]].\n",
                pszFilename);
        return -1;
    }
    if (ann->path_count == 0) {
        fprintf(stderr,
                "ERROR: %s: [[annotations]] without 'path' key.\n"
                "       REUSE 3.3 §4.1.2 requires 'path'.\n",
                pszFilename);
        return -1;
    }

    ann->license = read_string_or_join(hItem, "SPDX-License-Identifier",
                                        " AND ");
    ann->copyright = read_string_or_join(hItem, "SPDX-FileCopyrightText",
                                          "\n");
    if (read_contributors_into(hItem, ann) != 0) return -1;
    read_precedence(hItem, ann);

    ann->package_name = read_scalar_string(hItem, "SPDX-PackageName");
    ann->package_supplier = read_scalar_string(hItem, "SPDX-PackageSupplier");
    ann->package_download_location =
        read_scalar_string(hItem, "SPDX-PackageDownloadLocation");
    ann->package_comment = read_scalar_string(hItem, "SPDX-PackageComment");

    return 0;
}

/* Разбирает REUSE.toml, используя библиотеку TOML v1.0.0.
 * Возвращает ReuseConfig* или NULL при ошибке. Все ERROR-сообщения
 * печатаются в stderr. */
ReuseConfig* parse_reuse_toml(const char *filename) {
    HTOMLDOC hDoc = NULLHANDLE;
    HTOMLNODE hVer = NULLHANDLE;
    HTOMLNODE hAnn = NULLHANDLE;
    ReuseConfig *config = NULL;
    APIRET rc;
    ULONG ulType = 0, ulCount = 0, i;
    LONGLONG llVersion = 0;

    rc = TomlOpen(filename, &hDoc);
    if (rc != TOML_NO_ERROR) {
        fprintf(stderr,
                "ERROR: %s: cannot parse TOML.\n"
                "       Fix the syntax according to TOML v1.0.0:\n"
                "         https://toml.io/en/v1.0.0\n",
                filename);
        return NULL;
    }

    /* version = 1 (REUSE 3.3 §4.1.1, MUST). */
    rc = TomlQueryNode(hDoc, "version", &hVer);
    if (rc != TOML_NO_ERROR) {
        fprintf(stderr,
                "ERROR: %s: missing 'version = 1' at the top.\n"
                "       REUSE 3.3 §4.1.1 requires this line in every "
                "REUSE.toml.\n"
                "       Fix: add 'version = 1' as the first line.\n",
                filename);
        TomlClose(hDoc);
        return NULL;
    }
    if (TomlNodeGetType(hVer, &ulType) != TOML_NO_ERROR ||
        ulType != TOML_TYPE_INTEGER) {
        fprintf(stderr,
                "ERROR: %s: 'version' must be an integer, not a string.\n"
                "       TOML v1.0.0 §Integer; REUSE 3.3 §4.1.1 requires "
                "'version = 1'.\n"
                "       Fix: remove the quotes.\n",
                filename);
        TomlClose(hDoc);
        return NULL;
    }
    TomlNodeGetInteger(hVer, &llVersion);
    if (llVersion != 1) {
        fprintf(stderr,
                "ERROR: %s: unsupported version %ld.\n"
                "       REUSE 3.3 §4.1.1 defines version 1 only.\n",
                filename, (long)llVersion);
        TomlClose(hDoc);
        return NULL;
    }

    config = (ReuseConfig*)calloc(1, sizeof(ReuseConfig));
    if (!config) { TomlClose(hDoc); return NULL; }

    /* source_dir = filename без последнего компонента пути. */
    config->source_dir = dup_str(filename);
    if (config->source_dir) {
        char *slash = strrchr(config->source_dir, '/');
        char *backslash = strrchr(config->source_dir, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
        if (slash) *slash = '\0';
    }
    config->version = 1;

    /* Разбор [[annotations]]. */
    rc = TomlQueryNode(hDoc, "annotations", &hAnn);
    if (rc == TOML_NO_ERROR) {
        TomlNodeGetArrayCount(hAnn, &ulCount);
        if (ulCount > 0) {
            config->annotations =
                (Annotation*)calloc(ulCount, sizeof(Annotation));
            if (!config->annotations) {
                free_reuse_config(config);
                TomlClose(hDoc);
                return NULL;
            }
            for (i = 0; i < ulCount; i++) {
                HTOMLNODE hItem = NULLHANDLE;
                Annotation *ann;
                rc = TomlNodeGetArrayElement(hAnn, i, &hItem);
                if (rc != TOML_NO_ERROR) {
                    free_reuse_config(config);
                    TomlClose(hDoc);
                    return NULL;
                }
                ann = &config->annotations[config->annotation_count];
                if (parse_one_annotation(hItem, ann, filename) != 0) {
                    free_reuse_config(config);
                    TomlClose(hDoc);
                    return NULL;
                }
                ann->order_in_file = (int)config->annotation_count;
                config->annotation_count++;
            }
        }
    }

    TomlClose(hDoc);
    return config;
}

void free_reuse_config(ReuseConfig *config) {
    int i, j;
    if (!config) return;
    free(config->source_dir);
    for (i = 0; i < config->annotation_count; i++) {
        Annotation *a = &config->annotations[i];
        for (j = 0; j < a->path_count; j++)
            free(a->paths[j]);
        free(a->paths);
        free(a->license);
        free(a->copyright);
        for (j = 0; j < a->contributor_count; j++)
            free(a->contributors[j]);
        free(a->contributors);
        free(a->package_name);
        free(a->package_supplier);
        free(a->package_download_location);
        free(a->package_comment);
    }
    free(config->annotations);
    free(config);
}

/* ------------------------------------------------------------------ */
/* Разбор .reuse/dep5 через библиотеку DEP5 v1.0.0                     */
/*                                                                     */
/* DEP5-файл разбирается dep5.c. Здесь только конвертация Files-       */
/* параграфов в Annotation:                                            */
/*   - paths[]     — patterns из поля "Files" (со снятыми escape);     */
/*   - license     — synopsis поля "License" (short name, как есть);   */
/*   - copyright   — поле "Copyright" (многострочное, '\n');           */
/*   - precedence  — REUSE_PRECEDENCE_CLOSEST;                         */
/*   - order_in_file — порядковый номер параграфа.                     */
/*                                                                     */
/* Stand-alone License-параграфы игнорируются: DEP5 §5.3 определяет    */
/* их как опциональные и предназначенные для хранения полного текста   */
/* лицензии. В REUSE-контексте тексты хранятся в LICENSES/ (REUSE 3.3  */
/* §4.2.2), поэтому их разбор не требуется.                            */
/*                                                                     */
/* DEP5 short names передаются в Annotation.license без конвертации    */
/* в SPDX: библиотека универсальна, приведение к SPDX — задача         */
/* потребителя.                                                        */
/* ------------------------------------------------------------------ */

ReuseConfig *reuse_load_dep5(const char *repo_root) {
    char path[1024];
    HDEP5DOC hDoc = NULLHANDLE;
    HDEP5FIND hFind = NULLHANDLE;
    ReuseConfig *config = NULL;
    APIRET rc;
    ULONG ulFilesCount = 0;
    ULONG i;

    if (!repo_root || !repo_root[0]) return NULL;

#ifdef __LINUX__
    snprintf(path, sizeof(path), "%s/.reuse/dep5", repo_root);
#else
    snprintf(path, sizeof(path), "%s\\.reuse\\dep5", repo_root);
#endif

    rc = Dep5Open(path, &hDoc);
    if (rc != DEP5_NO_ERROR) return NULL;

    rc = Dep5FilesFindFirst(hDoc, &hFind, &ulFilesCount);
    if (rc != DEP5_NO_ERROR) {
        Dep5Close(hDoc);
        return NULL;
    }

    config = (ReuseConfig*)calloc(1, sizeof(ReuseConfig));
    if (!config) goto fail;

    config->source_dir = dup_str(repo_root);
    if (!config->source_dir) goto fail;
    config->depth = -1;
    config->version = 1;

    config->annotations = (Annotation*)calloc(ulFilesCount,
                                              sizeof(Annotation));
    if (!config->annotations) goto fail;

    for (i = 0; i < ulFilesCount; i++) {
        Annotation *ann = &config->annotations[i];
        ULONG ulPatCount = 0;
        ULONG j;
        ULONG ulSize;
        char *buf;

        memset(ann, 0, sizeof(*ann));
        ann->precedence = REUSE_PRECEDENCE_CLOSEST;
        ann->order_in_file = (int)i;

        /* Patterns */
        if (Dep5FilesGetPatternCount(hFind, &ulPatCount) == DEP5_NO_ERROR) {
            for (j = 0; j < ulPatCount; j++) {
                if (Dep5FilesGetPattern(hFind, j, NULL, 0, &ulSize)
                    != DEP5_NO_ERROR)
                    continue;
                buf = (char*)malloc(ulSize);
                if (!buf) goto fail;
                if (Dep5FilesGetPattern(hFind, j, buf, ulSize, NULL)
                    == DEP5_NO_ERROR)
                    add_path(ann, buf);
                free(buf);
            }
        }

        /* License (synopsis only) */
        if (Dep5FilesGetField(hFind, "License", NULL, 0, &ulSize)
            == DEP5_NO_ERROR) {
            buf = (char*)malloc(ulSize);
            if (!buf) goto fail;
            if (Dep5FilesGetField(hFind, "License", buf, ulSize, NULL)
                == DEP5_NO_ERROR)
                ann->license = dup_str(buf);
            free(buf);
        }

        /* Copyright */
        if (Dep5FilesGetField(hFind, "Copyright", NULL, 0, &ulSize)
            == DEP5_NO_ERROR) {
            buf = (char*)malloc(ulSize);
            if (!buf) goto fail;
            if (Dep5FilesGetField(hFind, "Copyright", buf, ulSize, NULL)
                == DEP5_NO_ERROR)
                ann->copyright = dup_str(buf);
            free(buf);
        }

        if (i + 1 < ulFilesCount) {
            if (Dep5FilesFindNext(hFind) != DEP5_NO_ERROR) goto fail;
        }
    }

    config->annotation_count = (int)ulFilesCount;

    Dep5FilesFindClose(hFind);
    Dep5Close(hDoc);
    return config;

fail:
    if (hFind) Dep5FilesFindClose(hFind);
    if (hDoc) Dep5Close(hDoc);
    free_reuse_config(config);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Поиск одного REUSE.toml вверх                                       */
/* ------------------------------------------------------------------ */

static void join_path_str(char *dest, size_t dest_size,
                          const char *dir, const char *name) {
    size_t len = strlen(dir);
#ifdef __LINUX__
    if (len == 0) snprintf(dest, dest_size, "%s", name);
    else snprintf(dest, dest_size, "%s/%s", dir, name);
#else
    if (len == 0) snprintf(dest, dest_size, "%s", name);
    else snprintf(dest, dest_size, "%s\\%s", dir, name);
#endif
}

char* find_reuse_toml_upwards(const char *start_dir) {
    static char path[1024];
    char *d = dup_str(start_dir);
    char *p;
    FILE *f;
    size_t len;

    if (!d) return NULL;
    len = strlen(d);
    while (len > 1 && (d[len-1] == '/' || d[len-1] == '\\')) d[--len] = '\0';

    while (1) {
        join_path_str(path, sizeof(path), d, "REUSE.toml");
        f = fopen(path, "r");
        if (f) {
            fclose(f);
            free(d);
            return path;
        }
        p = strrchr(d, '/');
#ifdef _WIN32
        {
            char *backslash = strrchr(d, '\\');
            if (backslash && (!p || backslash > p)) p = backslash;
        }
#endif
        if (!p) break;
        if (p == d) break;
#ifdef _WIN32
        if (p == d + 2 && d[1] == ':') { p[1] = '\\'; p[2] = '\0'; break; }
#endif
        *p = '\0';
    }

    free(d);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Сбор всех REUSE.toml от корня до target_dir                         */
/* ------------------------------------------------------------------ */

static char *rel_from(const char *from, const char *to) {
    size_t flen = strlen(from);
    size_t tlen = strlen(to);
    if (flen > tlen) return NULL;
    if (!path_prefix_eq(to, from, flen)) return NULL;
    if (flen == tlen) return dup_str("");
    if (to[flen] != '/' && to[flen] != '\\') return NULL;
    return dup_str(to + flen + 1);
}

int reuse_find_all_tomls(const char *repo_root,
                         const char *target_dir,
                         SpdxStrList *out_paths) {
    char path[2048];
    FILE *f;
    char *rel;
    char current[2048];

    spdx_strlist_init(out_paths);

    if (!repo_root) {
        join_path_str(path, sizeof(path), target_dir, "REUSE.toml");
        f = fopen(path, "r");
        if (f) { fclose(f); spdx_strlist_add(out_paths, path); }
        return 0;
    }

    rel = rel_from(repo_root, target_dir);
    if (!rel) {
        join_path_str(path, sizeof(path), target_dir, "REUSE.toml");
        f = fopen(path, "r");
        if (f) { fclose(f); spdx_strlist_add(out_paths, path); }
        return 0;
    }

    /* Корневой REUSE.toml */
    join_path_str(path, sizeof(path), repo_root, "REUSE.toml");
    f = fopen(path, "r");
    if (f) { fclose(f); spdx_strlist_add(out_paths, path); }

    if (rel[0] == '\0') {
        free(rel);
        return 0;
    }

    strncpy(current, repo_root, sizeof(current) - 1);
    current[sizeof(current) - 1] = '\0';

    {
        size_t ri = 0, rlen = strlen(rel);
        while (ri < rlen) {
            size_t start = ri;
            size_t seglen;
            while (ri < rlen && rel[ri] != '/' && rel[ri] != '\\') ri++;
            seglen = ri - start;
            if (seglen > 0) {
                size_t clen = strlen(current);
                if (clen + 1 + seglen + 1 > sizeof(current)) {
                    fprintf(stderr,
                            "WARNING: path too long, skipping REUSE.toml "
                            "below %s\n"
                            "         Increase the buffer size or shorten "
                            "the path.\n", current);
                    break;
                }
                current[clen] = '/';
                memcpy(current + clen + 1, rel + start, seglen);
                current[clen + 1 + seglen] = '\0';

                join_path_str(path, sizeof(path), current, "REUSE.toml");
                f = fopen(path, "r");
                if (f) { fclose(f); spdx_strlist_add(out_paths, path); }
            }
            if (ri < rlen) ri++;
        }
    }

    free(rel);
    return 0;
}

ReuseConfig **reuse_parse_all(const SpdxStrList *paths,
                              int *out_count,
                              int *out_error_count) {
    ReuseConfig **arr;
    int i, n = 0;

    *out_count = 0;
    if (out_error_count) *out_error_count = 0;
    if (!paths || paths->count == 0) return NULL;

    arr = (ReuseConfig**)malloc((size_t)paths->count * sizeof(ReuseConfig*));
    if (!arr) return NULL;

    for (i = 0; i < paths->count; i++) {
        ReuseConfig *c = parse_reuse_toml(paths->items[i]);
        if (!c) {
            if (out_error_count) (*out_error_count)++;
            continue;
        }
        c->depth = i;
        arr[n++] = c;
    }
    *out_count = n;
    return arr;
}

void reuse_free_all(ReuseConfig **configs, int count) {
    int i;
    if (!configs) return;
    for (i = 0; i < count; i++) free_reuse_config(configs[i]);
    free(configs);
}

/* ------------------------------------------------------------------ */
/* Резолвер с иерархией                                                */
/*                                                                     */
/* Политика разрешения полей:                                          */
/*                                                                     */
/* 1. Licensing Information (copyright, license).                      */
/*    Спецификация REUSE 3.3 (строки 21-23) определяет это как         */
/*    "the information that lists the copyright holders of a file or   */
/*    work, and describes under which licenses the file or work is     */
/*    made available". К этой категории применяется precedence:        */
/*    override > aggregate > closest > depth > order_in_file.          */
/*                                                                     */
/* 2. Contributors (SPDX-FileContributor).                             */
/*    Спецификация REUSE 3.3 (строки 177-182) прямо говорит:           */
/*    "You MAY include other keys and tables to convey additional      */
/*    information. Their semantics are not defined by this             */
/*    specification." SPDX-FileContributor упомянут как пример такого  */
/*    ключа. Поэтому precedence к нему формально НЕ применяется.       */
/*                                                                     */
/*    Проектное решение: contributors собираются с агрегацией         */
/*    (closest + aggregate) с дедупликацией строк. Причина: это       */
/*    список людей, и не терять информацию важнее, чем соблюдать      */
/*    иерархию. При наличии override — берётся только override.        */
/*                                                                     */
/* 3. Package* (SPDX-PackageName, SPDX-PackageSupplier,                */
/*    SPDX-PackageDownloadLocation, SPDX-PackageComment).              */
/*    Спецификация REUSE 3.3 (строки 177-182) относит их к "other      */
/*    keys", семантика не определена. precedence формально НЕ          */
/*    применяется.                                                     */
/*                                                                     */
/*    Проектное решение: closest > aggregate. Причина: package_name   */
/*    и прочие атрибуты — это атрибуты конкретного (под)пакета;       */
/*    более специфичная (ближе к файлу) аннотация должна выигрывать.  */
/*                                                                     */
/*    override по-прежнему выигрывает у всего.                         */
/*                                                                     */
/* Ссылки:                                                             */
/*   REUSE 3.3: https://reuse.software/spec-3.3/                       */
/*     - строки 21-23 (Licensing Information)                          */
/*     - строки 155-171 (precedence)                                   */
/*     - строки 177-182 (other keys)                                   */
/* ------------------------------------------------------------------ */

static int ann_better(const Annotation *a, int a_depth,
                      const Annotation *b, int b_depth) {
    if (a->precedence != b->precedence)
        return a->precedence > b->precedence;
    if (a_depth != b_depth)
        return a_depth > b_depth;
    return a->order_in_file > b->order_in_file;
}

static const Annotation *ann_find_match(ReuseConfig *cfg,
                                        const char *filename) {
    const char *rel = rel_to_config(cfg, filename);
    const Annotation *best = NULL;
    int i, j;
    for (i = 0; i < cfg->annotation_count; i++) {
        for (j = 0; j < cfg->annotations[i].path_count; j++) {
            if (matches_pattern(cfg->annotations[i].paths[j], rel)) {
                if (!best || ann_better(&cfg->annotations[i], cfg->depth,
                                        best, cfg->depth))
                    best = &cfg->annotations[i];
                break;
            }
        }
    }
    return best;
}

/* Склеивает уникальные идентификаторы из двух выражений через AND. */
static char *aggregate_licenses(const char *a, const char *b) {
    char *r;
    size_t alen, blen;
    if (!a) return b ? dup_str(b) : NULL;
    if (!b) return dup_str(a);
    alen = strlen(a);
    blen = strlen(b);
    r = (char*)malloc(alen + blen + 8);
    if (!r) return NULL;
    snprintf(r, alen + blen + 8, "(%s) AND (%s)", a, b);
    return r;
}

static char *aggregate_copyrights(const char *a, const char *b) {
    char *r;
    size_t alen, blen;
    if (!a) return b ? dup_str(b) : NULL;
    if (!b) return dup_str(a);
    alen = strlen(a);
    blen = strlen(b);
    r = (char*)malloc(alen + blen + 2);
    if (!r) return NULL;
    sprintf(r, "%s\n%s", a, b);
    return r;
}

/* Соединяет массив contributors в одну строку через '\n'. */
static char *join_contributors(char **items, int count) {
    size_t total = 0;
    int i;
    size_t pos = 0;
    char *out;

    if (count == 0) return NULL;
    for (i = 0; i < count; i++) {
        total += strlen(items[i]);
        if (i > 0) total += 1;
    }
    out = (char*)malloc(total + 1);
    if (!out) return NULL;
    for (i = 0; i < count; i++) {
        if (i > 0) out[pos++] = '\n';
        {
            size_t l = strlen(items[i]);
            memcpy(out + pos, items[i], l);
            pos += l;
        }
    }
    out[pos] = '\0';
    return out;
}

/* Проверяет, содержит ли haystack (строки через '\n') строку needle. */
static int contains_line(const char *haystack, const char *needle, size_t nlen) {
    const char *p = haystack;
    while (*p) {
        const char *line_end = strchr(p, '\n');
        size_t len = line_end ? (size_t)(line_end - p) : strlen(p);
        if (len == nlen && memcmp(p, needle, nlen) == 0) return 1;
        if (!line_end) break;
        p = line_end + 1;
    }
    return 0;
}

/* Соединяет две строки-списка ('\n'-separated) с дедупликацией строк. */
static char *join_lines_dedup(const char *a, const char *b) {
    size_t alen, blen, out_len;
    char *out;
    const char *p;

    if (!a && !b) return NULL;
    if (!a) return dup_str(b);
    if (!b) return dup_str(a);

    alen = strlen(a);
    blen = strlen(b);
    out = (char*)malloc(alen + blen + 2);
    if (!out) return NULL;
    memcpy(out, a, alen);
    out[alen] = '\0';
    out_len = alen;

    p = b;
    while (*p) {
        const char *line_end = strchr(p, '\n');
        size_t line_len = line_end ? (size_t)(line_end - p) : strlen(p);
        if (!contains_line(out, p, line_len)) {
            if (out_len + 1 + line_len + 1 > alen + blen + 2) {
                size_t ncap = out_len + 1 + line_len + 2;
                char *no = (char*)realloc(out, ncap);
                if (!no) { free(out); return NULL; }
                out = no;
            }
            out[out_len++] = '\n';
            memcpy(out + out_len, p, line_len);
            out_len += line_len;
            out[out_len] = '\0';
        }
        if (!line_end) break;
        p = line_end + 1;
    }
    return out;
}

void reuse_resolved_free(ReuseResolved *r) {
    if (!r) return;
    free(r->license);                    r->license = NULL;
    free(r->copyright);                  r->copyright = NULL;
    free(r->contributors);               r->contributors = NULL;
    free(r->package_name);               r->package_name = NULL;
    free(r->package_supplier);           r->package_supplier = NULL;
    free(r->package_download_location);  r->package_download_location = NULL;
    free(r->package_comment);            r->package_comment = NULL;
    r->precedence = 0;
    r->has_reuse = 0;
}

int reuse_resolve_for_file(ReuseConfig **configs, int count,
                           const char *filename,
                           const char *in_license,
                           const char *in_copyright,
                           ReuseResolved *out) {
    const Annotation *best_override = NULL;
    const Annotation *best_aggregate = NULL;
    const Annotation *best_closest = NULL;
    int best_override_depth = -1;
    int best_aggregate_depth = -1;
    int best_closest_depth = -1;
    int i;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));

    /* Поиск лучших аннотаций в каждой из трёх групп независимо. */
    for (i = 0; i < count; i++) {
        const Annotation *a = ann_find_match(configs[i], filename);
        if (!a) continue;
        if (a->precedence == REUSE_PRECEDENCE_OVERRIDE) {
            if (!best_override ||
                ann_better(a, configs[i]->depth,
                           best_override, best_override_depth)) {
                best_override = a;
                best_override_depth = configs[i]->depth;
            }
        } else if (a->precedence == REUSE_PRECEDENCE_AGGREGATE) {
            if (!best_aggregate ||
                ann_better(a, configs[i]->depth,
                           best_aggregate, best_aggregate_depth)) {
                best_aggregate = a;
                best_aggregate_depth = configs[i]->depth;
            }
        } else {
            if (!best_closest ||
                ann_better(a, configs[i]->depth,
                           best_closest, best_closest_depth)) {
                best_closest = a;
                best_closest_depth = configs[i]->depth;
            }
        }
    }

    /* === 1. Override — выигрывает у всего. === */
    if (best_override) {
        out->has_reuse = 1;
        out->precedence = REUSE_PRECEDENCE_OVERRIDE;
        if (best_override->license)
            out->license = dup_str(best_override->license);
        if (best_override->copyright)
            out->copyright = dup_str(best_override->copyright);
        if (best_override->contributor_count > 0)
            out->contributors = join_contributors(
                best_override->contributors,
                best_override->contributor_count);
        if (best_override->package_name)
            out->package_name = dup_str(best_override->package_name);
        if (best_override->package_supplier)
            out->package_supplier = dup_str(best_override->package_supplier);
        if (best_override->package_download_location)
            out->package_download_location =
                dup_str(best_override->package_download_location);
        if (best_override->package_comment)
            out->package_comment = dup_str(best_override->package_comment);
        return 0;
    }

    /* === 2. База: closest + in-file. === */
    {
        const char *base_lic = NULL;
        const char *base_cop = NULL;
        int base_has_reuse = 0;

        if (best_closest) {
            base_has_reuse = 1;
            if (best_closest->license) base_lic = best_closest->license;
            if (best_closest->copyright) base_cop = best_closest->copyright;
        }
        if (in_license) base_lic = in_license;
        if (in_copyright) base_cop = in_copyright;

        /* === 2a. Без aggregate. === */
        if (!best_aggregate) {
            out->has_reuse = base_has_reuse;
            out->precedence = REUSE_PRECEDENCE_CLOSEST;
            if (base_lic) out->license = dup_str(base_lic);
            if (base_cop) out->copyright = dup_str(base_cop);
            if (best_closest) {
                if (best_closest->contributor_count > 0)
                    out->contributors = join_contributors(
                        best_closest->contributors,
                        best_closest->contributor_count);
                if (best_closest->package_name)
                    out->package_name = dup_str(best_closest->package_name);
                if (best_closest->package_supplier)
                    out->package_supplier =
                        dup_str(best_closest->package_supplier);
                if (best_closest->package_download_location)
                    out->package_download_location =
                        dup_str(best_closest->package_download_location);
                if (best_closest->package_comment)
                    out->package_comment =
                        dup_str(best_closest->package_comment);
            }
            return 0;
        }

        /* === 2b. Aggregate поверх base. === */
        out->has_reuse = 1;
        out->precedence = REUSE_PRECEDENCE_AGGREGATE;

        if (base_lic && best_aggregate->license)
            out->license = aggregate_licenses(base_lic,
                                              best_aggregate->license);
        else if (base_lic)
            out->license = dup_str(base_lic);
        else if (best_aggregate->license)
            out->license = dup_str(best_aggregate->license);

        if (base_cop && best_aggregate->copyright)
            out->copyright = aggregate_copyrights(base_cop,
                                                  best_aggregate->copyright);
        else if (base_cop)
            out->copyright = dup_str(base_cop);
        else if (best_aggregate->copyright)
            out->copyright = dup_str(best_aggregate->copyright);

        {
            char *base_contrib = NULL;
            char *agg_contrib = NULL;
            if (best_closest && best_closest->contributor_count > 0)
                base_contrib = join_contributors(
                    best_closest->contributors,
                    best_closest->contributor_count);
            if (best_aggregate->contributor_count > 0)
                agg_contrib = join_contributors(
                    best_aggregate->contributors,
                    best_aggregate->contributor_count);
            if (base_contrib && agg_contrib) {
                out->contributors = join_lines_dedup(base_contrib,
                                                     agg_contrib);
                free(base_contrib);
                free(agg_contrib);
            } else if (base_contrib) {
                out->contributors = base_contrib;
            } else if (agg_contrib) {
                out->contributors = agg_contrib;
            }
        }

        if (best_closest && best_closest->package_name)
            out->package_name = dup_str(best_closest->package_name);
        else if (best_aggregate->package_name)
            out->package_name = dup_str(best_aggregate->package_name);

        if (best_closest && best_closest->package_supplier)
            out->package_supplier = dup_str(best_closest->package_supplier);
        else if (best_aggregate->package_supplier)
            out->package_supplier = dup_str(best_aggregate->package_supplier);

        if (best_closest && best_closest->package_download_location)
            out->package_download_location =
                dup_str(best_closest->package_download_location);
        else if (best_aggregate->package_download_location)
            out->package_download_location =
                dup_str(best_aggregate->package_download_location);

        if (best_closest && best_closest->package_comment)
            out->package_comment = dup_str(best_closest->package_comment);
        else if (best_aggregate->package_comment)
            out->package_comment = dup_str(best_aggregate->package_comment);
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* Совместимость со старым API                                         */
/* ------------------------------------------------------------------ */

const char* find_license_for_file(ReuseConfig *config, const char *filename) {
    const Annotation *a;
    if (!config) return NULL;
    a = ann_find_match(config, filename);
    return a ? a->license : NULL;
}
