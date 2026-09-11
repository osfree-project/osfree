/* spdx_db.c - библиотека SPDX license list (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_db.h"
#include "json_parser.h"
#include "sha1_utils.h"
#include "spdx_utils.h"

#define CACHE_MAGIC   "SPDXDB06"
#define CACHE_VERSION 6

/* ------------------------------------------------------------------ */
/* Структуры                                                           */
/* ------------------------------------------------------------------ */

typedef struct {
    SpdxLicenseEntry *items;
    int count;
    int capacity;
} LicenseList;

typedef struct {
    SpdxExceptionEntry *items;
    int count;
    int capacity;
} ExceptionList;

static LicenseList   g_licenses;
static ExceptionList g_exceptions;

static FILE  *g_cache_fp = NULL;
static char  *g_details_dir = NULL;
static char  *g_exceptions_dir = NULL;

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

static void free_strlist(char **list) {
    int i;
    if (!list) return;
    for (i = 0; list[i]; i++) free(list[i]);
    free(list);
}

/* ------------------------------------------------------------------ */
/* Динамические массивы                                                */
/* ------------------------------------------------------------------ */

static void license_list_init(LicenseList *l) {
    l->count = 0;
    l->capacity = 16;
    l->items = (SpdxLicenseEntry*)calloc((size_t)l->capacity,
                                         sizeof(SpdxLicenseEntry));
    if (!l->items) { fprintf(stderr, "OOM\n"); exit(1); }
}

static SpdxLicenseEntry *license_list_add(LicenseList *l) {
    SpdxLicenseEntry *e;
    if (l->count >= l->capacity) {
        l->capacity *= 2;
        l->items = (SpdxLicenseEntry*)realloc(l->items,
            (size_t)l->capacity * sizeof(SpdxLicenseEntry));
        if (!l->items) { fprintf(stderr, "OOM\n"); exit(1); }
    }
    e = &l->items[l->count++];
    memset(e, 0, sizeof(*e));
    return e;
}

static void license_list_free(LicenseList *l) {
    int i;
    for (i = 0; i < l->count; i++) {
        SpdxLicenseEntry *e = &l->items[i];
        free(e->id);
        free(e->name);
        free_strlist(e->see_also);
        free(e->text);
        free(e->template);
        free(e->text_html);
    }
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

static void exception_list_init(ExceptionList *l) {
    l->count = 0;
    l->capacity = 16;
    l->items = (SpdxExceptionEntry*)calloc((size_t)l->capacity,
                                           sizeof(SpdxExceptionEntry));
    if (!l->items) { fprintf(stderr, "OOM\n"); exit(1); }
}

static SpdxExceptionEntry *exception_list_add(ExceptionList *l) {
    SpdxExceptionEntry *e;
    if (l->count >= l->capacity) {
        l->capacity *= 2;
        l->items = (SpdxExceptionEntry*)realloc(l->items,
            (size_t)l->capacity * sizeof(SpdxExceptionEntry));
        if (!l->items) { fprintf(stderr, "OOM\n"); exit(1); }
    }
    e = &l->items[l->count++];
    memset(e, 0, sizeof(*e));
    return e;
}

static void exception_list_free(ExceptionList *l) {
    int i;
    for (i = 0; i < l->count; i++) {
        SpdxExceptionEntry *e = &l->items[i];
        free(e->id);
        free(e->name);
        free_strlist(e->see_also);
        free(e->text);
        free(e->template);
        free(e->text_html);
    }
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

/* ------------------------------------------------------------------ */
/* Сортировка и бинарный поиск                                         */
/* ------------------------------------------------------------------ */

static int cmp_lic(const void *a, const void *b) {
    return strcmp(((const SpdxLicenseEntry*)a)->id,
                  ((const SpdxLicenseEntry*)b)->id);
}
static int cmp_exc(const void *a, const void *b) {
    return strcmp(((const SpdxExceptionEntry*)a)->id,
                  ((const SpdxExceptionEntry*)b)->id);
}

static int lic_lower_bound(const char *id) {
    int lo = 0, hi = g_licenses.count;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (strcmp(g_licenses.items[mid].id, id) < 0) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}
static int exc_lower_bound(const char *id) {
    int lo = 0, hi = g_exceptions.count;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (strcmp(g_exceptions.items[mid].id, id) < 0) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

/* ------------------------------------------------------------------ */
/* Разбор JSON-индекса                                                 */
/* ------------------------------------------------------------------ */

static char **parse_string_array(JsonNode *arr) {
    int n, i;
    char **out;
    if (!arr || arr->type != JSON_ARRAY) return NULL;
    n = arr->child_count;
    out = (char**)malloc((size_t)(n + 1) * sizeof(char*));
    if (!out) return NULL;
    for (i = 0; i < n; i++) {
        const char *s = json_get_string(arr->children[i]);
        out[i] = s ? dup_str(s) : NULL;
    }
    out[n] = NULL;
    return out;
}

static int load_licenses_index(const char *path) {
    char *text = spdx_read_file_all(path, NULL);
    JsonNode *root, *arr;
    int i;
    if (!text) return -1;
    root = json_parse(text);
    free(text);
    if (!root) return -1;
    arr = json_find_child(root, "licenses");
    if (!arr || arr->type != JSON_ARRAY) { json_free(root); return -1; }
    for (i = 0; i < arr->child_count; i++) {
        JsonNode *item = arr->children[i];
        JsonNode *id_n  = json_find_child(item, "licenseId");
        JsonNode *nm_n  = json_find_child(item, "name");
        JsonNode *osi_n = json_find_child(item, "isOsiApproved");
        JsonNode *fsf_n = json_find_child(item, "isFsfLibre");
        JsonNode *dep_n = json_find_child(item, "isDeprecatedLicenseId");
        JsonNode *see_n = json_find_child(item, "seeAlso");
        SpdxLicenseEntry *e;
        const char *id = id_n ? json_get_string(id_n) : NULL;
        if (!id) continue;
        e = license_list_add(&g_licenses);
        e->id = dup_str(id);
        if (nm_n) e->name = dup_str(json_get_string(nm_n));
        if (osi_n && osi_n->type == JSON_BOOLEAN && osi_n->bool_value)
            e->flags |= SPDX_DB_FLAG_OSI;
        if (fsf_n && fsf_n->type == JSON_BOOLEAN && fsf_n->bool_value)
            e->flags |= SPDX_DB_FLAG_FSF_LIBRE;
        if (dep_n && dep_n->type == JSON_BOOLEAN && dep_n->bool_value)
            e->flags |= SPDX_DB_FLAG_DEPRECATED;
        e->see_also = parse_string_array(see_n);
    }
    json_free(root);
    return 0;
}

static int load_exceptions_index(const char *path) {
    char *text = spdx_read_file_all(path, NULL);
    JsonNode *root, *arr;
    int i;
    if (!text) return -1;
    root = json_parse(text);
    free(text);
    if (!root) return -1;
    arr = json_find_child(root, "exceptions");
    if (!arr || arr->type != JSON_ARRAY) { json_free(root); return -1; }
    for (i = 0; i < arr->child_count; i++) {
        JsonNode *item = arr->children[i];
        JsonNode *id_n  = json_find_child(item, "licenseExceptionId");
        JsonNode *nm_n  = json_find_child(item, "name");
        JsonNode *dep_n = json_find_child(item, "isDeprecatedLicenseId");
        JsonNode *see_n = json_find_child(item, "seeAlso");
        SpdxExceptionEntry *e;
        const char *id = id_n ? json_get_string(id_n) : NULL;
        if (!id) continue;
        e = exception_list_add(&g_exceptions);
        e->id = dup_str(id);
        if (nm_n) e->name = dup_str(json_get_string(nm_n));
        if (dep_n && dep_n->type == JSON_BOOLEAN && dep_n->bool_value)
            e->flags |= SPDX_DB_FLAG_DEPRECATED;
        e->see_also = parse_string_array(see_n);
    }
    json_free(root);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Бинарные примитивы                                                  */
/* ------------------------------------------------------------------ */

static int write_u32(FILE *f, unsigned long v) {
    unsigned char b[4];
    b[0] = (unsigned char)(v & 0xFF);
    b[1] = (unsigned char)((v >> 8) & 0xFF);
    b[2] = (unsigned char)((v >> 16) & 0xFF);
    b[3] = (unsigned char)((v >> 24) & 0xFF);
    return fwrite(b, 1, 4, f) == 4 ? 0 : -1;
}
static int read_u32(FILE *f, unsigned long *out) {
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) return -1;
    *out = (unsigned long)b[0] |
           ((unsigned long)b[1] << 8) |
           ((unsigned long)b[2] << 16) |
           ((unsigned long)b[3] << 24);
    return 0;
}
static int write_u8v(FILE *f, unsigned char v) {
    return fwrite(&v, 1, 1, f) == 1 ? 0 : -1;
}
static int read_u8v(FILE *f, unsigned char *v) {
    return fread(v, 1, 1, f) == 1 ? 0 : -1;
}

static int write_str32(FILE *f, const char *s) {
    size_t n = s ? strlen(s) : 0;
    if (write_u32(f, (unsigned long)n) != 0) return -1;
    if (n > 0 && fwrite(s, 1, n, f) != n) return -1;
    return 0;
}

static int read_str32(FILE *f, char **out) {
    unsigned long n;
    char *s;
    if (read_u32(f, &n) != 0) return -1;
    s = (char*)malloc(n + 1);
    if (!s) return -1;
    if (n > 0 && fread(s, 1, n, f) != n) { free(s); return -1; }
    s[n] = '\0';
    *out = s;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Запись кеш-файла                                                    */
/* ------------------------------------------------------------------ */

static int write_detail_block(FILE *f, const char *text,
                              const char *tmpl, const char *html,
                              unsigned long *out_offset,
                              unsigned long *out_size) {
    long start;
    fflush(f);
    start = ftell(f);
    if (start < 0) return -1;
    if (write_str32(f, text ? text : "") != 0) return -1;
    if (write_str32(f, tmpl ? tmpl : "") != 0) return -1;
    if (write_str32(f, html ? html : "") != 0) return -1;
    fflush(f);
    {
        long end = ftell(f);
        if (end < 0) return -1;
        *out_offset = (unsigned long)start;
        *out_size = (unsigned long)(end - start);
    }
    return 0;
}

static int write_index_record(FILE *f, const SpdxLicenseEntry *e) {
    int i;
    if (write_str32(f, e->id) != 0) return -1;
    if (write_u8v(f, e->flags) != 0) return -1;
    if (write_str32(f, e->name ? e->name : "") != 0) return -1;
    {
        unsigned long cnt = 0;
        if (e->see_also) for (i = 0; e->see_also[i]; i++) cnt++;
        if (write_u32(f, cnt) != 0) return -1;
        for (i = 0; e->see_also && e->see_also[i]; i++)
            if (write_str32(f, e->see_also[i]) != 0) return -1;
    }
    if (write_u32(f, e->detail_offset) != 0) return -1;
    if (write_u32(f, e->detail_size) != 0) return -1;
    return 0;
}

static int write_index_record_exc(FILE *f, const SpdxExceptionEntry *e) {
    int i;
    if (write_str32(f, e->id) != 0) return -1;
    if (write_u8v(f, e->flags) != 0) return -1;
    if (write_str32(f, e->name ? e->name : "") != 0) return -1;
    {
        unsigned long cnt = 0;
        if (e->see_also) for (i = 0; e->see_also[i]; i++) cnt++;
        if (write_u32(f, cnt) != 0) return -1;
        for (i = 0; e->see_also && e->see_also[i]; i++)
            if (write_str32(f, e->see_also[i]) != 0) return -1;
    }
    if (write_u32(f, e->detail_offset) != 0) return -1;
    if (write_u32(f, e->detail_size) != 0) return -1;
    return 0;
}

static int write_license_detail_from_json(FILE *f, const char *id,
                                          unsigned long *out_offset,
                                          unsigned long *out_size) {
    char path[2048];
    char *text;
    JsonNode *root, *n;
    long sz;

    snprintf(path, sizeof(path), "%s/%s.json", g_details_dir, id);
    text = spdx_read_file_all(path, &sz);
    if (!text) return -1;
    root = json_parse(text);
    free(text);
    if (!root) return -1;
    {
        const char *s_text = NULL, *s_tmpl = NULL, *s_html = NULL;
        n = json_find_child(root, "licenseText");
        if (n) s_text = json_get_string(n);
        n = json_find_child(root, "standardLicenseTemplate");
        if (n) s_tmpl = json_get_string(n);
        n = json_find_child(root, "licenseTextHtml");
        if (n) s_html = json_get_string(n);

        if (write_detail_block(f, s_text, s_tmpl, s_html,
                               out_offset, out_size) != 0) {
            json_free(root);
            return -1;
        }
    }
    json_free(root);
    return 0;
}

static int write_exception_detail_from_json(FILE *f, const char *id,
                                            unsigned long *out_offset,
                                            unsigned long *out_size) {
    char path[2048];
    char *text;
    JsonNode *root, *n;
    long sz;

    snprintf(path, sizeof(path), "%s/%s.json", g_exceptions_dir, id);
    text = spdx_read_file_all(path, &sz);
    if (!text) return -1;
    root = json_parse(text);
    free(text);
    if (!root) return -1;
    {
        const char *s_text = NULL, *s_tmpl = NULL, *s_html = NULL;
        n = json_find_child(root, "licenseExceptionText");
        if (n) s_text = json_get_string(n);
        n = json_find_child(root, "licenseExceptionTemplate");
        if (n) s_tmpl = json_get_string(n);
        n = json_find_child(root, "exceptionTextHtml");
        if (n) s_html = json_get_string(n);

        if (write_detail_block(f, s_text, s_tmpl, s_html,
                               out_offset, out_size) != 0) {
            json_free(root);
            return -1;
        }
    }
    json_free(root);
    return 0;
}

static int compute_sha1_raw(const char *path, unsigned char out[20]) {
    char *hex;
    int i;
    memset(out, 0, 20);
    if (!path) return 0;
    hex = sha1_file(path);
    if (!hex) return -1;
    for (i = 0; i < 20; i++) {
        char b[3];
        b[0] = hex[i*2]; b[1] = hex[i*2+1]; b[2] = '\0';
        out[i] = (unsigned char)strtol(b, NULL, 16);
    }
    free(hex);
    return 0;
}

static int build_cache(const char *cache_path,
                       const unsigned char sha1_lic[20],
                       const unsigned char sha1_exc[20]) {
    FILE *f;
    unsigned long off_lic_idx = 0, off_exc_idx = 0;
    long here;
    int i;

    f = fopen(cache_path, "wb");
    if (!f) return -1;

    {
        fwrite(CACHE_MAGIC, 1, 8, f);
        write_u32(f, CACHE_VERSION);
        fwrite(sha1_lic, 1, 20, f);
        fwrite(sha1_exc, 1, 20, f);
        write_u32(f, (unsigned long)g_licenses.count);
        write_u32(f, (unsigned long)g_exceptions.count);
        write_u32(f, 0);
        write_u32(f, 0);
        write_u32(f, 0);
        write_u32(f, 0);
    }

    fflush(f);
    off_lic_idx = (unsigned long)ftell(f);
    for (i = 0; i < g_licenses.count; i++) {
        SpdxLicenseEntry *e = &g_licenses.items[i];
        if (g_details_dir) {
            unsigned long doff = 0, dsz = 0;
            if (write_license_detail_from_json(f, e->id, &doff, &dsz) == 0) {
                e->detail_offset = doff;
                e->detail_size = dsz;
            }
        }
        if (write_index_record(f, e) != 0) {
            fclose(f);
            return -1;
        }
    }

    fflush(f);
    off_exc_idx = (unsigned long)ftell(f);
    for (i = 0; i < g_exceptions.count; i++) {
        SpdxExceptionEntry *e = &g_exceptions.items[i];
        if (g_exceptions_dir) {
            unsigned long doff = 0, dsz = 0;
            if (write_exception_detail_from_json(f, e->id, &doff, &dsz) == 0) {
                e->detail_offset = doff;
                e->detail_size = dsz;
            }
        }
        if (write_index_record_exc(f, e) != 0) {
            fclose(f);
            return -1;
        }
    }

    here = ftell(f);
    fseek(f, 0, SEEK_SET);
    fwrite(CACHE_MAGIC, 1, 8, f);
    write_u32(f, CACHE_VERSION);
    fwrite(sha1_lic, 1, 20, f);
    fwrite(sha1_exc, 1, 20, f);
    write_u32(f, (unsigned long)g_licenses.count);
    write_u32(f, (unsigned long)g_exceptions.count);
    write_u32(f, off_lic_idx);
    write_u32(f, off_exc_idx);
    write_u32(f, 0);
    write_u32(f, 0);
    fseek(f, here, SEEK_SET);

    fclose(f);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Чтение кеш-файла                                                    */
/* ------------------------------------------------------------------ */

static int read_license_index_record(FILE *f, SpdxLicenseEntry *e) {
    unsigned long cnt, i;
    if (read_str32(f, &e->id) != 0) return -1;
    if (read_u8v(f, &e->flags) != 0) return -1;
    if (read_str32(f, &e->name) != 0) return -1;
    if (read_u32(f, &cnt) != 0) return -1;
    if (cnt > 0) {
        e->see_also = (char**)malloc((size_t)(cnt + 1) * sizeof(char*));
        if (!e->see_also) return -1;
        for (i = 0; i < cnt; i++) {
            if (read_str32(f, &e->see_also[i]) != 0) {
                e->see_also[i] = NULL;
                return -1;
            }
        }
        e->see_also[cnt] = NULL;
    }
    if (read_u32(f, &e->detail_offset) != 0) return -1;
    if (read_u32(f, &e->detail_size) != 0) return -1;
    e->detail_loaded = 0;
    return 0;
}

static int read_exception_index_record(FILE *f, SpdxExceptionEntry *e) {
    unsigned long cnt, i;
    if (read_str32(f, &e->id) != 0) return -1;
    if (read_u8v(f, &e->flags) != 0) return -1;
    if (read_str32(f, &e->name) != 0) return -1;
    if (read_u32(f, &cnt) != 0) return -1;
    if (cnt > 0) {
        e->see_also = (char**)malloc((size_t)(cnt + 1) * sizeof(char*));
        if (!e->see_also) return -1;
        for (i = 0; i < cnt; i++) {
            if (read_str32(f, &e->see_also[i]) != 0) {
                e->see_also[i] = NULL;
                return -1;
            }
        }
        e->see_also[cnt] = NULL;
    }
    if (read_u32(f, &e->detail_offset) != 0) return -1;
    if (read_u32(f, &e->detail_size) != 0) return -1;
    e->detail_loaded = 0;
    return 0;
}

static int load_cache(const char *path,
                      const unsigned char expect_lic[20],
                      const unsigned char expect_exc[20]) {
    FILE *f;
    char magic[9];
    unsigned long version, cnt_lic, cnt_exc, off_lic, off_exc;
    unsigned long dummy;
    unsigned char sha1_lic[20], sha1_exc[20];
    unsigned long i;

    f = fopen(path, "rb");
    if (!f) return -1;
    if (fread(magic, 1, 8, f) != 8 ||
        memcmp(magic, CACHE_MAGIC, 8) != 0) { fclose(f); return -1; }
    if (read_u32(f, &version) != 0 || version != CACHE_VERSION) {
        fclose(f); return -1;
    }
    if (fread(sha1_lic, 1, 20, f) != 20 ||
        fread(sha1_exc, 1, 20, f) != 20) { fclose(f); return -1; }
    if (memcmp(sha1_lic, expect_lic, 20) != 0 ||
        memcmp(sha1_exc, expect_exc, 20) != 0) { fclose(f); return -1; }
    if (read_u32(f, &cnt_lic) != 0 ||
        read_u32(f, &cnt_exc) != 0 ||
        read_u32(f, &off_lic) != 0 ||
        read_u32(f, &off_exc) != 0 ||
        read_u32(f, &dummy) != 0 ||
        read_u32(f, &dummy) != 0) { fclose(f); return -1; }

    if (fseek(f, (long)off_lic, SEEK_SET) != 0) { fclose(f); return -1; }
    for (i = 0; i < cnt_lic; i++) {
        SpdxLicenseEntry *e = license_list_add(&g_licenses);
        if (read_license_index_record(f, e) != 0) { fclose(f); return -1; }
    }
    if (fseek(f, (long)off_exc, SEEK_SET) != 0) { fclose(f); return -1; }
    for (i = 0; i < cnt_exc; i++) {
        SpdxExceptionEntry *e = exception_list_add(&g_exceptions);
        if (read_exception_index_record(f, e) != 0) { fclose(f); return -1; }
    }

    g_cache_fp = f;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Ленивое чтение деталей из кеша                                      */
/* ------------------------------------------------------------------ */

static int read_detail_from_cache(unsigned long offset, unsigned long size,
                                  char **out_text, char **out_tmpl,
                                  char **out_html) {
    char *buf;
    long got;
    unsigned long pos = 0;
    unsigned long len;

    if (!g_cache_fp || offset == 0 || size == 0) return -1;
    buf = (char*)malloc(size);
    if (!buf) return -1;
    if (fseek(g_cache_fp, (long)offset, SEEK_SET) != 0) { free(buf); return -1; }
    got = (long)fread(buf, 1, size, g_cache_fp);
    if (got != (long)size) { free(buf); return -1; }

    if (pos + 4 > size) { free(buf); return -1; }
    len = (unsigned long)buf[pos]        |
          ((unsigned long)buf[pos+1] << 8) |
          ((unsigned long)buf[pos+2] << 16)|
          ((unsigned long)buf[pos+3] << 24);
    pos += 4;
    if (pos + len > size) { free(buf); return -1; }
    *out_text = (char*)malloc(len + 1);
    if (!*out_text) { free(buf); return -1; }
    memcpy(*out_text, buf + pos, len);
    (*out_text)[len] = '\0';
    pos += len;

    if (pos + 4 > size) { free(*out_text); *out_text = NULL; free(buf); return -1; }
    len = (unsigned long)buf[pos]        |
          ((unsigned long)buf[pos+1] << 8) |
          ((unsigned long)buf[pos+2] << 16)|
          ((unsigned long)buf[pos+3] << 24);
    pos += 4;
    if (pos + len > size) { free(*out_text); *out_text = NULL; free(buf); return -1; }
    *out_tmpl = (char*)malloc(len + 1);
    if (!*out_tmpl) { free(*out_text); *out_text = NULL; free(buf); return -1; }
    memcpy(*out_tmpl, buf + pos, len);
    (*out_tmpl)[len] = '\0';
    pos += len;

    if (pos + 4 > size) {
        free(*out_text); *out_text = NULL;
        free(*out_tmpl); *out_tmpl = NULL;
        free(buf); return -1;
    }
    len = (unsigned long)buf[pos]        |
          ((unsigned long)buf[pos+1] << 8) |
          ((unsigned long)buf[pos+2] << 16)|
          ((unsigned long)buf[pos+3] << 24);
    pos += 4;
    if (pos + len > size) {
        free(*out_text); *out_text = NULL;
        free(*out_tmpl); *out_tmpl = NULL;
        free(buf); return -1;
    }
    *out_html = (char*)malloc(len + 1);
    if (!*out_html) {
        free(*out_text); *out_text = NULL;
        free(*out_tmpl); *out_tmpl = NULL;
        free(buf); return -1;
    }
    memcpy(*out_html, buf + pos, len);
    (*out_html)[len] = '\0';

    free(buf);
    return 0;
}

static int read_license_detail_from_dir(SpdxLicenseEntry *e) {
    char path[2048];
    char *text;
    JsonNode *root, *n;
    snprintf(path, sizeof(path), "%s/%s.json", g_details_dir, e->id);
    text = spdx_read_file_all(path, NULL);
    if (!text) return -1;
    root = json_parse(text);
    free(text);
    if (!root) return -1;
    n = json_find_child(root, "licenseText");
    if (n) e->text = dup_str(json_get_string(n));
    n = json_find_child(root, "standardLicenseTemplate");
    if (n) e->template = dup_str(json_get_string(n));
    n = json_find_child(root, "licenseTextHtml");
    if (n) e->text_html = dup_str(json_get_string(n));
    json_free(root);
    return 0;
}

static int read_exception_detail_from_dir(SpdxExceptionEntry *e) {
    char path[2048];
    char *text;
    JsonNode *root, *n;
    snprintf(path, sizeof(path), "%s/%s.json", g_exceptions_dir, e->id);
    text = spdx_read_file_all(path, NULL);
    if (!text) return -1;
    root = json_parse(text);
    free(text);
    if (!root) return -1;
    n = json_find_child(root, "licenseExceptionText");
    if (n) e->text = dup_str(json_get_string(n));
    n = json_find_child(root, "licenseExceptionTemplate");
    if (n) e->template = dup_str(json_get_string(n));
    n = json_find_child(root, "exceptionTextHtml");
    if (n) e->text_html = dup_str(json_get_string(n));
    json_free(root);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Публичный API                                                       */
/* ------------------------------------------------------------------ */

int spdx_db_init(const char *licenses_json,
                 const char *exceptions_json,
                 const char *details_dir,
                 const char *exceptions_dir,
                 const char *cache_file) {
    unsigned char sha1_lic[20], sha1_exc[20];
    int errs = 0;
    int cache_ok = 0;

    license_list_init(&g_licenses);
    exception_list_init(&g_exceptions);
    g_details_dir = details_dir ? dup_str(details_dir) : NULL;
    g_exceptions_dir = exceptions_dir ? dup_str(exceptions_dir) : NULL;

    if (!licenses_json) {
        errs |= SPDX_DB_ERR_LICENSES;
        memset(sha1_lic, 0, 20);
    } else if (compute_sha1_raw(licenses_json, sha1_lic) != 0) {
        errs |= SPDX_DB_ERR_LICENSES;
        memset(sha1_lic, 0, 20);
    }

    if (!exceptions_json) {
        errs |= SPDX_DB_ERR_EXCEPTIONS;
        memset(sha1_exc, 0, 20);
    } else if (compute_sha1_raw(exceptions_json, sha1_exc) != 0) {
        errs |= SPDX_DB_ERR_EXCEPTIONS;
        memset(sha1_exc, 0, 20);
    }

    if (cache_file) {
        if (load_cache(cache_file, sha1_lic, sha1_exc) == 0)
            cache_ok = 1;
    }

    if (!cache_ok) {
        if (licenses_json) {
            if (load_licenses_index(licenses_json) != 0)
                errs |= SPDX_DB_ERR_LICENSES;
        }
        if (exceptions_json) {
            if (load_exceptions_index(exceptions_json) != 0)
                errs |= SPDX_DB_ERR_EXCEPTIONS;
        }
        qsort(g_licenses.items, (size_t)g_licenses.count,
              sizeof(SpdxLicenseEntry), cmp_lic);
        qsort(g_exceptions.items, (size_t)g_exceptions.count,
              sizeof(SpdxExceptionEntry), cmp_exc);

        if (cache_file) {
            if (build_cache(cache_file, sha1_lic, sha1_exc) != 0) {
                errs |= SPDX_DB_ERR_CACHE;
                fprintf(stderr, "Warning: cannot write cache: %s\n", cache_file);
            }
        }
    }

    return errs;
}

void spdx_db_free(void) {
    license_list_free(&g_licenses);
    exception_list_free(&g_exceptions);
    if (g_cache_fp) { fclose(g_cache_fp); g_cache_fp = NULL; }
    free(g_details_dir); g_details_dir = NULL;
    free(g_exceptions_dir); g_exceptions_dir = NULL;
}

const SpdxLicenseEntry *spdx_license_lookup(const char *id) {
    int idx;
    if (!id || g_licenses.count == 0) return NULL;
    idx = lic_lower_bound(id);
    if (idx < g_licenses.count &&
        strcmp(g_licenses.items[idx].id, id) == 0)
        return &g_licenses.items[idx];
    return NULL;
}

const SpdxExceptionEntry *spdx_exception_lookup(const char *id) {
    int idx;
    if (!id || g_exceptions.count == 0) return NULL;
    idx = exc_lower_bound(id);
    if (idx < g_exceptions.count &&
        strcmp(g_exceptions.items[idx].id, id) == 0)
        return &g_exceptions.items[idx];
    return NULL;
}

int spdx_license_load_detail(const char *id) {
    int idx;
    SpdxLicenseEntry *e;
    idx = lic_lower_bound(id);
    if (idx >= g_licenses.count ||
        strcmp(g_licenses.items[idx].id, id) != 0) return -1;
    e = &g_licenses.items[idx];
    if (e->detail_loaded) return 0;

    if (g_cache_fp && e->detail_offset && e->detail_size) {
        if (read_detail_from_cache(e->detail_offset, e->detail_size,
                                   &e->text, &e->template, &e->text_html) == 0) {
            e->detail_loaded = 1;
            return 0;
        }
    }
    if (g_details_dir) {
        if (read_license_detail_from_dir(e) == 0) {
            e->detail_loaded = 1;
            return 0;
        }
    }
    return -1;
}

int spdx_exception_load_detail(const char *id) {
    int idx;
    SpdxExceptionEntry *e;
    idx = exc_lower_bound(id);
    if (idx >= g_exceptions.count ||
        strcmp(g_exceptions.items[idx].id, id) != 0) return -1;
    e = &g_exceptions.items[idx];
    if (e->detail_loaded) return 0;

    if (g_cache_fp && e->detail_offset && e->detail_size) {
        if (read_detail_from_cache(e->detail_offset, e->detail_size,
                                   &e->text, &e->template, &e->text_html) == 0) {
            e->detail_loaded = 1;
            return 0;
        }
    }
    if (g_exceptions_dir) {
        if (read_exception_detail_from_dir(e) == 0) {
            e->detail_loaded = 1;
            return 0;
        }
    }
    return -1;
}

int spdx_license_is_valid(const char *id) {
    if (!id || id[0] == '\0') return 0;
    if (strncmp(id, "LicenseRef-", 11) == 0) return 1;
    if (strncmp(id, "DocumentRef-", 12) == 0) {
        const char *colon = strchr(id, ':');
        if (colon && strncmp(colon + 1, "LicenseRef-", 11) == 0) return 1;
        return 0;
    }
    if (g_licenses.count == 0) return 1;
    return spdx_license_lookup(id) != NULL;
}

int spdx_exception_is_valid(const char *id) {
    if (!id || id[0] == '\0') return 0;
    if (g_exceptions.count == 0) return 1;
    return spdx_exception_lookup(id) != NULL;
}

int spdx_license_is_deprecated(const char *id) {
    const SpdxLicenseEntry *e = spdx_license_lookup(id);
    return e && (e->flags & SPDX_DB_FLAG_DEPRECATED) ? 1 : 0;
}
int spdx_exception_is_deprecated(const char *id) {
    const SpdxExceptionEntry *e = spdx_exception_lookup(id);
    return e && (e->flags & SPDX_DB_FLAG_DEPRECATED) ? 1 : 0;
}
int spdx_license_is_osi_approved(const char *id) {
    const SpdxLicenseEntry *e = spdx_license_lookup(id);
    return e && (e->flags & SPDX_DB_FLAG_OSI) ? 1 : 0;
}
int spdx_license_is_fsf_libre(const char *id) {
    const SpdxLicenseEntry *e = spdx_license_lookup(id);
    return e && (e->flags & SPDX_DB_FLAG_FSF_LIBRE) ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Разбор SPDX-выражений                                               */
/* ------------------------------------------------------------------ */

typedef struct {
    const char *p;
    const char *bad_token_start;
    int err;
} ExprParser;

static void skip_ws(ExprParser *pp) {
    while (*pp->p && isspace((unsigned char)*pp->p)) pp->p++;
}
static int is_kw_at(const char *p, const char *kw, int kwlen) {
    if (strncmp(p, kw, kwlen) != 0) return 0;
    if (p[kwlen] == '\0') return 1;
    if (isspace((unsigned char)p[kwlen])) return 1;
    if (p[kwlen] == '(' || p[kwlen] == ')') return 1;
    return 0;
}

static int parse_expression(ExprParser *pp);

static int parse_term(ExprParser *pp) {
    skip_ws(pp);
    if (*pp->p == '(') {
        pp->p++;
        if (!parse_expression(pp)) return 0;
        skip_ws(pp);
        if (*pp->p != ')') {
            if (pp->err == SPDX_EXPR_OK) pp->err = SPDX_EXPR_SYNTAX_ERROR;
            return 0;
        }
        pp->p++;
        return 1;
    }
    {
        const char *id_start = pp->p;
        int id_len;
        char id_buf[256];
        const char *save;
        while (*pp->p && !isspace((unsigned char)*pp->p) &&
               *pp->p != '(' && *pp->p != ')') pp->p++;
        id_len = (int)(pp->p - id_start);
        if (id_len == 0) {
            if (pp->err == SPDX_EXPR_OK) pp->err = SPDX_EXPR_SYNTAX_ERROR;
            return 0;
        }
        if (id_len >= (int)sizeof(id_buf)) id_len = (int)sizeof(id_buf) - 1;
        memcpy(id_buf, id_start, (size_t)id_len);
        id_buf[id_len] = '\0';
        if (!spdx_license_is_valid(id_buf)) {
            pp->err = SPDX_EXPR_UNKNOWN_TOKEN;
            pp->bad_token_start = id_start;
            return 0;
        }
        save = pp->p;
        skip_ws(pp);
        if (is_kw_at(pp->p, "WITH", 4)) {
            const char *e_start;
            int e_len;
            char e_buf[256];
            pp->p += 4;
            skip_ws(pp);
            e_start = pp->p;
            while (*pp->p && !isspace((unsigned char)*pp->p) &&
                   *pp->p != '(' && *pp->p != ')') pp->p++;
            e_len = (int)(pp->p - e_start);
            if (e_len == 0) {
                if (pp->err == SPDX_EXPR_OK) pp->err = SPDX_EXPR_SYNTAX_ERROR;
                return 0;
            }
            if (e_len >= (int)sizeof(e_buf)) e_len = (int)sizeof(e_buf) - 1;
            memcpy(e_buf, e_start, (size_t)e_len);
            e_buf[e_len] = '\0';
            if (!spdx_exception_is_valid(e_buf)) {
                pp->err = SPDX_EXPR_UNKNOWN_TOKEN;
                pp->bad_token_start = e_start;
                return 0;
            }
        } else {
            pp->p = save;
        }
    }
    return 1;
}

static int parse_expression(ExprParser *pp) {
    if (!parse_term(pp)) return 0;
    while (1) {
        const char *save;
        skip_ws(pp);
        save = pp->p;
        if (is_kw_at(pp->p, "AND", 3)) {
            pp->p += 3;
            if (!parse_term(pp)) return 0;
        } else if (is_kw_at(pp->p, "OR", 2)) {
            pp->p += 2;
            if (!parse_term(pp)) return 0;
        } else {
            pp->p = save;
            break;
        }
    }
    return 1;
}

int spdx_expression_validate(const char *expr, const char **bad_token) {
    ExprParser pp;
    if (bad_token) *bad_token = NULL;
    if (!expr) return SPDX_EXPR_SYNTAX_ERROR;
    pp.p = expr;
    pp.bad_token_start = NULL;
    pp.err = SPDX_EXPR_OK;
    if (!parse_expression(&pp)) {
        if (pp.err == SPDX_EXPR_OK) pp.err = SPDX_EXPR_SYNTAX_ERROR;
        if (bad_token && pp.bad_token_start) *bad_token = pp.bad_token_start;
        return pp.err;
    }
    skip_ws(&pp);
    if (*pp.p != '\0') return SPDX_EXPR_SYNTAX_ERROR;
    return SPDX_EXPR_OK;
}
