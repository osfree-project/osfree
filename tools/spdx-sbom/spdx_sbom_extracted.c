/* spdx_sbom_extracted.c - работа с LicenseRef-* (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_sbom_extracted.h"
#include "spdx_utils.h"

static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

void extracted_init(ExtractedLicenseList *list) {
    list->count = 0;
    list->capacity = 8;
    list->items = (ExtractedLicenseInfo*)calloc((size_t)list->capacity,
                                                sizeof(ExtractedLicenseInfo));
    if (!list->items) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

void extracted_free(ExtractedLicenseList *list) {
    int i;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].license_id);
        free(list->items[i].extracted_text);
        free(list->items[i].name);
        free(list->items[i].comment);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

ExtractedLicenseInfo *extracted_lookup(ExtractedLicenseList *list,
                                       const char *id) {
    int i;
    for (i = 0; i < list->count; i++)
        if (strcmp(list->items[i].license_id, id) == 0)
            return &list->items[i];
    return NULL;
}

ExtractedLicenseInfo *extracted_add(ExtractedLicenseList *list,
                                    const char *id) {
    ExtractedLicenseInfo *existing = extracted_lookup(list, id);
    ExtractedLicenseInfo *e;
    if (existing) return existing;

    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (ExtractedLicenseInfo*)realloc(list->items,
            (size_t)list->capacity * sizeof(ExtractedLicenseInfo));
        if (!list->items) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        memset(&list->items[list->count], 0,
               ((size_t)list->capacity - (size_t)list->count) *
               sizeof(ExtractedLicenseInfo));
    }
    e = &list->items[list->count];
    memset(e, 0, sizeof(*e));
    e->license_id = dup_str(id);
    list->count++;
    return e;
}

/* »щет текст в <project_dir>/LICENSES/<id>.txt или без расширени€. */
static char *find_text_in_licenses_dir(const char *project_dir,
                                       const char *id) {
    char path[2048];
    char *text;

    snprintf(path, sizeof(path), "%s/LICENSES/%s.txt", project_dir, id);
    text = spdx_read_file_all(path, NULL);
    if (text) return text;

    snprintf(path, sizeof(path), "%s\\LICENSES\\%s.txt", project_dir, id);
    text = spdx_read_file_all(path, NULL);
    if (text) return text;

    snprintf(path, sizeof(path), "%s/LICENSES/%s", project_dir, id);
    text = spdx_read_file_all(path, NULL);
    if (text) return text;

    snprintf(path, sizeof(path), "%s\\LICENSES\\%s", project_dir, id);
    text = spdx_read_file_all(path, NULL);
    if (text) return text;

    return NULL;
}

/* »щет текст в extra (CLI). */
static char *find_text_in_extra(const ExtractedLicenseSource *extra,
                                int extra_count, const char *id) {
    int i;
    for (i = 0; i < extra_count; i++)
        if (strcmp(extra[i].id, id) == 0)
            return spdx_read_file_all(extra[i].path, NULL);
    return NULL;
}

/* –азбор SPDX-выражени€ на токены: LicenseRef-* добавл€ютс€ в list. */
static void scan_licenseref_tokens(const char *expr,
                                   ExtractedLicenseList *list) {
    const char *p = expr;
    char tok[256];

    if (!expr) return;
    while (*p) {
        const char *start;
        size_t len;

        while (*p && (isspace((unsigned char)*p) || *p == '(' || *p == ')'))
            p++;
        if (!*p) break;

        start = p;
        while (*p && !isspace((unsigned char)*p) && *p != '(' && *p != ')')
            p++;
        len = (size_t)(p - start);
        if (len == 0 || len >= sizeof(tok)) continue;
        memcpy(tok, start, len);
        tok[len] = '\0';

        if (strncmp(tok, "LicenseRef-", 11) == 0) {
            extracted_add(list, tok);
        }
    }
}

int extracted_collect_from_files(ExtractedLicenseList *list,
                                 const FileList *files,
                                 const char *project_dir,
                                 const ExtractedLicenseSource *extra,
                                 int extra_count) {
    int i;

    for (i = 0; i < files->count; i++)
        scan_licenseref_tokens(files->items[i].license, list);

    for (i = 0; i < list->count; i++) {
        ExtractedLicenseInfo *e = &list->items[i];
        char *text;

        if (e->extracted_text) continue;

        text = find_text_in_extra(extra, extra_count, e->license_id);
        if (!text)
            text = find_text_in_licenses_dir(project_dir, e->license_id);

        if (!text) {
            fprintf(stderr,
                    "Error: no text found for %s\n"
                    "       expected at LICENSES/%s.txt or via "
                    "--extracted-license=%s:<path>\n",
                    e->license_id, e->license_id, e->license_id);
            return -1;
        }
        e->extracted_text = text;
    }
    return 0;
}
