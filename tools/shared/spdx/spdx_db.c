/* spdx_db.c - SPDX license list database (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_db.h"
#include "json.h"
#include "sha1.h"
#include "spdx.h"

/**
 * @file spdx_db.c
 * @brief Implementation of the SPDX license list database.
 *
 * Conforms to:
 *   - SPDX License List.
 *     https://spdx.org/licenses/
 *   - SPDX 2.3, Annex D.2 (case-insensitive identifier comparison).
 *   - SPDX 2.3, Annex D (license expression grammar).
 *
 * The database is loaded from a directory that contains:
 *   - licenses.json
 *   - exceptions.json
 *   - details/<id>.json
 *   - exceptions/<id>.json
 *
 * A binary cache stores both the index and the details to avoid
 * re-parsing the JSON files on every run.
 */

#define CACHE_MAGIC   "SPDXDB06"
#define CACHE_VERSION 7

/* ------------------------------------------------------------------ */
/* Internal structures                                                 */
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
/* Utilities                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Lower-case an ASCII letter.
 *
 * Unlike tolower(), the result does not depend on the current locale.
 * SPDX 2.3 Annex D.2 requires case-insensitive identifier comparison
 * on ASCII characters only.
 *
 * @param[in] c  Character.
 *
 * @return Lower-case equivalent for A-Z, unchanged otherwise.
 */
static int ascii_lower(int c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

/**
 * @brief Case-insensitive comparison of two ASCII strings.
 *
 * @param[in] a  First string. Not NULL.
 * @param[in] b  Second string. Not NULL.
 *
 * @return Negative, zero or positive, following the usual ordering
 *         contract.
 */
static int id_cmp_ci(const char *a, const char *b) {
    while (*a && *b) {
        int ca = ascii_lower((unsigned char)*a);
        int cb = ascii_lower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

/**
 * @brief Duplicate a NUL-terminated string.
 *
 * @param[in] s  Source string, or NULL.
 *
 * @return malloc'd copy, or NULL on OOM or if @p s is NULL.
 */
static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

/**
 * @brief Free a NULL-terminated array of strings.
 *
 * @param[in] list  Array, or NULL.
 */
static void free_strlist(char **list) {
    int i;
    if (!list) return;
    for (i = 0; list[i]; i++) free(list[i]);
    free(list);
}

/* ------------------------------------------------------------------ */
/* License and exception lists                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialize an empty license list.
 *
 * On OOM the process is terminated.
 *
 * @param[out] l  List. Not NULL.
 */
static void license_list_init(LicenseList *l) {
    l->count = 0;
    l->capacity = 16;
    l->items = (SpdxLicenseEntry*)calloc((size_t)l->capacity,
                                         sizeof(SpdxLicenseEntry));
    if (!l->items) { fprintf(stderr, "ERROR: out of memory\n"); exit(EXIT_FAILURE); }
}

/**
 * @brief Append a new entry to a license list, growing it if needed.
 *
 * On OOM the process is terminated.
 *
 * @param[in,out] l  List. Not NULL.
 *
 * @return Pointer to the new entry, zero-filled.
 */
static SpdxLicenseEntry *license_list_add(LicenseList *l) {
    SpdxLicenseEntry *e;
    if (l->count >= l->capacity) {
        l->capacity *= 2;
        l->items = (SpdxLicenseEntry*)realloc(l->items,
            (size_t)l->capacity * sizeof(SpdxLicenseEntry));
        if (!l->items) { fprintf(stderr, "ERROR: out of memory\n"); exit(EXIT_FAILURE); }
    }
    e = &l->items[l->count++];
    memset(e, 0, sizeof(*e));
    return e;
}

/**
 * @brief Release all memory owned by a license list.
 *
 * @param[in,out] l  List. Not NULL.
 */
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

/**
 * @brief Initialize an empty exception list.
 *
 * On OOM the process is terminated.
 *
 * @param[out] l  List. Not NULL.
 */
static void exception_list_init(ExceptionList *l) {
    l->count = 0;
    l->capacity = 16;
    l->items = (SpdxExceptionEntry*)calloc((size_t)l->capacity,
                                           sizeof(SpdxExceptionEntry));
    if (!l->items) { fprintf(stderr, "ERROR: out of memory\n"); exit(EXIT_FAILURE); }
}

/**
 * @brief Append a new entry to an exception list, growing it if needed.
 *
 * On OOM the process is terminated.
 *
 * @param[in,out] l  List. Not NULL.
 *
 * @return Pointer to the new entry, zero-filled.
 */
static SpdxExceptionEntry *exception_list_add(ExceptionList *l) {
    SpdxExceptionEntry *e;
    if (l->count >= l->capacity) {
        l->capacity *= 2;
        l->items = (SpdxExceptionEntry*)realloc(l->items,
            (size_t)l->capacity * sizeof(SpdxExceptionEntry));
        if (!l->items) { fprintf(stderr, "ERROR: out of memory\n"); exit(EXIT_FAILURE); }
    }
    e = &l->items[l->count++];
    memset(e, 0, sizeof(*e));
    return e;
}

/**
 * @brief Release all memory owned by an exception list.
 *
 * @param[in,out] l  List. Not NULL.
 */
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
/* Sorting and binary search                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief qsort comparator for license entries.
 */
static int cmp_lic(const void *a, const void *b) {
    return id_cmp_ci(((const SpdxLicenseEntry*)a)->id,
                     ((const SpdxLicenseEntry*)b)->id);
}

/**
 * @brief qsort comparator for exception entries.
 */
static int cmp_exc(const void *a, const void *b) {
    return id_cmp_ci(((const SpdxExceptionEntry*)a)->id,
                     ((const SpdxExceptionEntry*)b)->id);
}

/**
 * @brief Binary search lower bound in the license list.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return Index of the first entry not less than @p id.
 */
static int lic_lower_bound(const char *id) {
    int lo = 0, hi = g_licenses.count;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (id_cmp_ci(g_licenses.items[mid].id, id) < 0) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

/**
 * @brief Binary search lower bound in the exception list.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return Index of the first entry not less than @p id.
 */
static int exc_lower_bound(const char *id) {
    int lo = 0, hi = g_exceptions.count;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (id_cmp_ci(g_exceptions.items[mid].id, id) < 0) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

/* ------------------------------------------------------------------ */
/* JSON helpers                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Duplicate a JSON string node into a fresh buffer.
 *
 * @param[in] hNode  Node handle. May be NULLHANDLE.
 *
 * @return malloc'd string, or NULL if the node is absent, not a
 *         string, or on OOM.
 */
static char *json_dup_string(HJSONNODE hNode) {
    ULONG ulSize = 0;
    char *buf;
    if (hNode == NULLHANDLE) return NULL;
    if (JsonNodeGetString(hNode, NULL, 0, &ulSize) != NO_ERROR) return NULL;
    if (ulSize == 0) return NULL;
    buf = (char*)malloc(ulSize);
    if (!buf) return NULL;
    if (JsonNodeGetString(hNode, buf, ulSize, NULL) != NO_ERROR) {
        free(buf);
        return NULL;
    }
    return buf;
}

/**
 * @brief Read a boolean field from a JSON object.
 *
 * @param[in]  hParent  Object handle. Not NULLHANDLE.
 * @param[in]  pszKey   Field name. Not NULL.
 * @param[out] pfValue  Receiver. Not NULL.
 *
 * @return 1 if the field exists and is a boolean, 0 otherwise.
 */
static int json_get_bool(HJSONNODE hParent, PCSZ pszKey, PBOOL pfValue) {
    HJSONNODE hChild = NULLHANDLE;
    BOOL fVal = FALSE_;
    if (hParent == NULLHANDLE) return 0;
    if (JsonNodeGetChild(hParent, pszKey, &hChild) != NO_ERROR) return 0;
    if (JsonNodeGetBoolean(hChild, &fVal) != NO_ERROR) return 0;
    *pfValue = fVal;
    return 1;
}

/**
 * @brief Convert a JSON array of strings into a NULL-terminated
 *        array of malloc'd strings.
 *
 * @param[in] hArr  Array node, or NULLHANDLE.
 *
 * @return malloc'd array, or NULL if @p hArr is absent or not an
 *         array.
 */
static char **parse_string_array(HJSONNODE hArr) {
    ULONG ulCount, ulType, i;
    char **out;
    if (hArr == NULLHANDLE) return NULL;
    if (JsonNodeGetType(hArr, &ulType) != NO_ERROR) return NULL;
    if (ulType != (ULONG)JSON_ARRAY) return NULL;
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) return NULL;
    out = (char**)malloc((size_t)(ulCount + 1) * sizeof(char*));
    if (!out) return NULL;
    for (i = 0; i < ulCount; i++) {
        HJSONNODE hElem = NULLHANDLE;
        out[i] = NULL;
        if (JsonNodeGetElement(hArr, i, &hElem) == NO_ERROR) {
            out[i] = json_dup_string(hElem);
        }
    }
    out[ulCount] = NULL;
    return out;
}

/* ------------------------------------------------------------------ */
/* Index loading                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Load the licenses index from licenses.json.
 *
 * @param[in] path  Path to licenses.json. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int load_licenses_index(const char *path) {
    char *text = NULL;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    ULONG ulCount = 0;
    ULONG i;

    if (SpdxReadFileAll(path, &text, NULL) != NO_ERROR) return -1;

    if (JsonParse(text, &hDoc) != NO_ERROR) { free(text); return -1; }
    free(text);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }
    if (JsonNodeGetChild(hRoot, "licenses", &hArr) != NO_ERROR) {
        JsonClose(hDoc);
        return -1;
    }
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) {
        JsonClose(hDoc);
        return -1;
    }

    for (i = 0; i < ulCount; i++) {
        HJSONNODE hItem = NULLHANDLE;
        HJSONNODE hChild = NULLHANDLE;
        SpdxLicenseEntry *e;
        char *id;

        if (JsonNodeGetElement(hArr, i, &hItem) != NO_ERROR) continue;
        if (JsonNodeGetChild(hItem, "licenseId", &hChild) != NO_ERROR)
            continue;
        id = json_dup_string(hChild);
        if (!id) continue;

        e = license_list_add(&g_licenses);
        e->id = id;

        if (JsonNodeGetChild(hItem, "name", &hChild) == NO_ERROR)
            e->name = json_dup_string(hChild);

        {
            BOOL fVal = FALSE_;
            if (json_get_bool(hItem, "isOsiApproved", &fVal) && fVal)
                e->flags |= SPDX_DB_FLAG_OSI;
            if (json_get_bool(hItem, "isFsfLibre", &fVal) && fVal)
                e->flags |= SPDX_DB_FLAG_FSF_LIBRE;
            if (json_get_bool(hItem, "isDeprecatedLicenseId", &fVal) && fVal)
                e->flags |= SPDX_DB_FLAG_DEPRECATED;
        }

        if (JsonNodeGetChild(hItem, "seeAlso", &hChild) == NO_ERROR)
            e->see_also = parse_string_array(hChild);
    }

    JsonClose(hDoc);
    return 0;
}

/**
 * @brief Load the exceptions index from exceptions.json.
 *
 * @param[in] path  Path to exceptions.json. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int load_exceptions_index(const char *path) {
    char *text = NULL;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    ULONG ulCount = 0;
    ULONG i;

    if (SpdxReadFileAll(path, &text, NULL) != NO_ERROR) return -1;

    if (JsonParse(text, &hDoc) != NO_ERROR) { free(text); return -1; }
    free(text);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }
    if (JsonNodeGetChild(hRoot, "exceptions", &hArr) != NO_ERROR) {
        JsonClose(hDoc);
        return -1;
    }
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) {
        JsonClose(hDoc);
        return -1;
    }

    for (i = 0; i < ulCount; i++) {
        HJSONNODE hItem = NULLHANDLE;
        HJSONNODE hChild = NULLHANDLE;
        SpdxExceptionEntry *e;
        char *id;

        if (JsonNodeGetElement(hArr, i, &hItem) != NO_ERROR) continue;
        if (JsonNodeGetChild(hItem, "licenseExceptionId", &hChild) != NO_ERROR)
            continue;
        id = json_dup_string(hChild);
        if (!id) continue;

        e = exception_list_add(&g_exceptions);
        e->id = id;

        if (JsonNodeGetChild(hItem, "name", &hChild) == NO_ERROR)
            e->name = json_dup_string(hChild);

        {
            BOOL fVal = FALSE_;
            if (json_get_bool(hItem, "isDeprecatedLicenseId", &fVal) && fVal)
                e->flags |= SPDX_DB_FLAG_DEPRECATED;
        }

        if (JsonNodeGetChild(hItem, "seeAlso", &hChild) == NO_ERROR)
            e->see_also = parse_string_array(hChild);
    }

    JsonClose(hDoc);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Binary cache primitives                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Write a 32-bit value in little-endian order.
 */
static int write_u32(FILE *f, unsigned long v) {
    unsigned char b[4];
    b[0] = (unsigned char)(v & 0xFF);
    b[1] = (unsigned char)((v >> 8) & 0xFF);
    b[2] = (unsigned char)((v >> 16) & 0xFF);
    b[3] = (unsigned char)((v >> 24) & 0xFF);
    return fwrite(b, 1, 4, f) == 4 ? 0 : -1;
}

/**
 * @brief Read a 32-bit little-endian value.
 */
static int read_u32(FILE *f, unsigned long *out) {
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) return -1;
    *out = (unsigned long)b[0] |
           ((unsigned long)b[1] << 8) |
           ((unsigned long)b[2] << 16) |
           ((unsigned long)b[3] << 24);
    return 0;
}

/**
 * @brief Write one byte.
 */
static int write_u8v(FILE *f, unsigned char v) {
    return fwrite(&v, 1, 1, f) == 1 ? 0 : -1;
}

/**
 * @brief Read one byte.
 */
static int read_u8v(FILE *f, unsigned char *v) {
    return fread(v, 1, 1, f) == 1 ? 0 : -1;
}

/**
 * @brief Write a length-prefixed string.
 */
static int write_str32(FILE *f, const char *s) {
    size_t n = s ? strlen(s) : 0;
    if (write_u32(f, (unsigned long)n) != 0) return -1;
    if (n > 0 && fwrite(s, 1, n, f) != n) return -1;
    return 0;
}

/**
 * @brief Read a length-prefixed string.
 *
 * @param[in]  f    File. Not NULL.
 * @param[out] out  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
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
/* Cache writing                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Write a detail block (text + template + html) to the cache.
 *
 * @param[in]  f           File. Not NULL.
 * @param[in]  text        License text, or NULL.
 * @param[in]  tmpl        Standard template, or NULL.
 * @param[in]  html        HTML text, or NULL.
 * @param[out] out_offset  Offset of the block. Not NULL.
 * @param[out] out_size    Size of the block. Not NULL.
 *
 * @return 0 on success, -1 on write error.
 */
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

/**
 * @brief Write one license index record to the cache.
 */
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

/**
 * @brief Write one exception index record to the cache.
 */
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

/**
 * @brief Read a license detail from details/<id>.json and write it
 *        to the cache.
 */
static int write_license_detail_from_json(FILE *f, const char *id,
                                          unsigned long *out_offset,
                                          unsigned long *out_size) {
    char path[2048];
    char *text = NULL;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;
    char *s_text = NULL, *s_tmpl = NULL, *s_html = NULL;
    int rc = -1;

    snprintf(path, sizeof(path), "%s/%s.json", g_details_dir, id);
    if (SpdxReadFileAll(path, &text, NULL) != NO_ERROR) return -1;
    if (JsonParse(text, &hDoc) != NO_ERROR) { free(text); return -1; }
    free(text);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }

    if (JsonNodeGetChild(hRoot, "licenseText", &hChild) == NO_ERROR)
        s_text = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "standardLicenseTemplate", &hChild) == NO_ERROR)
        s_tmpl = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "licenseTextHtml", &hChild) == NO_ERROR)
        s_html = json_dup_string(hChild);

    if (write_detail_block(f, s_text, s_tmpl, s_html,
                           out_offset, out_size) == 0) {
        rc = 0;
    }

    free(s_text);
    free(s_tmpl);
    free(s_html);
    JsonClose(hDoc);
    return rc;
}

/**
 * @brief Read an exception detail from exceptions/<id>.json and
 *        write it to the cache.
 */
static int write_exception_detail_from_json(FILE *f, const char *id,
                                            unsigned long *out_offset,
                                            unsigned long *out_size) {
    char path[2048];
    char *text = NULL;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;
    char *s_text = NULL, *s_tmpl = NULL, *s_html = NULL;
    int rc = -1;

    snprintf(path, sizeof(path), "%s/%s.json", g_exceptions_dir, id);
    if (SpdxReadFileAll(path, &text, NULL) != NO_ERROR) return -1;
    if (JsonParse(text, &hDoc) != NO_ERROR) { free(text); return -1; }
    free(text);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }

    if (JsonNodeGetChild(hRoot, "licenseExceptionText", &hChild) == NO_ERROR)
        s_text = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "licenseExceptionTemplate", &hChild) == NO_ERROR)
        s_tmpl = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "exceptionTextHtml", &hChild) == NO_ERROR)
        s_html = json_dup_string(hChild);

    if (write_detail_block(f, s_text, s_tmpl, s_html,
                           out_offset, out_size) == 0) {
        rc = 0;
    }

    free(s_text);
    free(s_tmpl);
    free(s_html);
    JsonClose(hDoc);
    return rc;
}

/**
 * @brief Compute a raw 20-byte SHA-1 of a file.
 *
 * @param[in]  path  Path to the file. Not NULL.
 * @param[out] out   20-byte digest.
 *
 * @return 0 on success, -1 on error.
 */
static int compute_sha1_raw(const char *path, unsigned char out[20]) {
    char hex[41];
    int i;
    memset(out, 0, 20);
    if (!path) return 0;
    if (Sha1File(path, hex, sizeof(hex), NULL) != NO_ERROR) return -1;
    for (i = 0; i < 20; i++) {
        char b[3];
        b[0] = hex[i*2]; b[1] = hex[i*2+1]; b[2] = '\0';
        out[i] = (unsigned char)strtol(b, NULL, 16);
    }
    return 0;
}

/**
 * @brief Build the binary cache file.
 *
 * @param[in] cache_path   Cache file path. Not NULL.
 * @param[in] sha1_lic     SHA-1 of licenses.json.
 * @param[in] sha1_exc     SHA-1 of exceptions.json.
 *
 * @return 0 on success, -1 on error.
 */
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
/* Cache reading                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Read one license index record from the cache.
 */
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

/**
 * @brief Read one exception index record from the cache.
 */
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

/**
 * @brief Load the binary cache file if its content matches the
 *        expected source hashes.
 *
 * @param[in] path         Cache file path. Not NULL.
 * @param[in] expect_lic   Expected SHA-1 of licenses.json.
 * @param[in] expect_exc   Expected SHA-1 of exceptions.json.
 *
 * @return 0 on success, -1 on error or mismatch.
 */
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
/* Lazy detail loading                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Read a detail block from the cache.
 */
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

/**
 * @brief Read a license detail from details/<id>.json.
 */
static int read_license_detail_from_dir(SpdxLicenseEntry *e) {
    char path[2048];
    char *text = NULL;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;

    snprintf(path, sizeof(path), "%s/%s.json", g_details_dir, e->id);
    if (SpdxReadFileAll(path, &text, NULL) != NO_ERROR) return -1;
    if (JsonParse(text, &hDoc) != NO_ERROR) { free(text); return -1; }
    free(text);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }

    if (JsonNodeGetChild(hRoot, "licenseText", &hChild) == NO_ERROR)
        e->text = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "standardLicenseTemplate", &hChild) == NO_ERROR)
        e->template = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "licenseTextHtml", &hChild) == NO_ERROR)
        e->text_html = json_dup_string(hChild);

    JsonClose(hDoc);
    return 0;
}

/**
 * @brief Read an exception detail from exceptions/<id>.json.
 */
static int read_exception_detail_from_dir(SpdxExceptionEntry *e) {
    char path[2048];
    char *text = NULL;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;

    snprintf(path, sizeof(path), "%s/%s.json", g_exceptions_dir, e->id);
    if (SpdxReadFileAll(path, &text, NULL) != NO_ERROR) return -1;
    if (JsonParse(text, &hDoc) != NO_ERROR) { free(text); return -1; }
    free(text);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }

    if (JsonNodeGetChild(hRoot, "licenseExceptionText", &hChild) == NO_ERROR)
        e->text = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "licenseExceptionTemplate", &hChild) == NO_ERROR)
        e->template = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "exceptionTextHtml", &hChild) == NO_ERROR)
        e->text_html = json_dup_string(hChild);

    JsonClose(hDoc);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialize the SPDX license database.
 *
 * Reads the licenses and exceptions indexes, either from a binary
 * cache (if the source hashes match) or directly from the JSON
 * files. Details are loaded lazily.
 *
 * @param[in] spdx_db_root  Database root directory. Not NULL.
 * @param[in] cache_file    Cache file path, or NULL for no caching.
 *
 * @return Bitmask of SPDX_DB_ERR_* flags. Zero on full success.
 */
int spdx_db_init(const char *spdx_db_root, const char *cache_file) {
    unsigned char sha1_lic[20], sha1_exc[20];
    int errs = 0;
    int cache_ok = 0;
    char lic_path[2048];
    char exc_path[2048];
    char det_path[2048];
    char exc_det_path[2048];

    license_list_init(&g_licenses);
    exception_list_init(&g_exceptions);

    if (!spdx_db_root || !spdx_db_root[0]) {
        return SPDX_DB_ERR_LICENSES | SPDX_DB_ERR_EXCEPTIONS;
    }

    snprintf(lic_path, sizeof(lic_path), "%s/licenses.json", spdx_db_root);
    snprintf(exc_path, sizeof(exc_path), "%s/exceptions.json", spdx_db_root);
    snprintf(det_path, sizeof(det_path), "%s/details", spdx_db_root);
    snprintf(exc_det_path, sizeof(exc_det_path),
             "%s/exceptions", spdx_db_root);

    g_details_dir = dup_str(det_path);
    g_exceptions_dir = dup_str(exc_det_path);

    if (compute_sha1_raw(lic_path, sha1_lic) != 0) {
        errs |= SPDX_DB_ERR_LICENSES;
        memset(sha1_lic, 0, 20);
    }
    if (compute_sha1_raw(exc_path, sha1_exc) != 0) {
        errs |= SPDX_DB_ERR_EXCEPTIONS;
        memset(sha1_exc, 0, 20);
    }

    if (cache_file) {
        if (load_cache(cache_file, sha1_lic, sha1_exc) == 0)
            cache_ok = 1;
    }

    if (!cache_ok) {
        if (load_licenses_index(lic_path) != 0)
            errs |= SPDX_DB_ERR_LICENSES;
        if (load_exceptions_index(exc_path) != 0)
            errs |= SPDX_DB_ERR_EXCEPTIONS;
        qsort(g_licenses.items, (size_t)g_licenses.count,
              sizeof(SpdxLicenseEntry), cmp_lic);
        qsort(g_exceptions.items, (size_t)g_exceptions.count,
              sizeof(SpdxExceptionEntry), cmp_exc);

        if (cache_file) {
            if (build_cache(cache_file, sha1_lic, sha1_exc) != 0) {
                errs |= SPDX_DB_ERR_CACHE;
                fprintf(stderr,
                        "WARNING: cannot write cache: %s\n"
                        "         Next run will re-parse JSON indexes.\n",
                        cache_file);
            }
        }
    }

    return errs;
}

/**
 * @brief Release all memory owned by the database.
 */
void spdx_db_free(void) {
    license_list_free(&g_licenses);
    exception_list_free(&g_exceptions);
    if (g_cache_fp) { fclose(g_cache_fp); g_cache_fp = NULL; }
    free(g_details_dir); g_details_dir = NULL;
    free(g_exceptions_dir); g_exceptions_dir = NULL;
}

/**
 * @brief Look up a license entry by identifier.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return Pointer to the entry, or NULL if not found.
 */
const SpdxLicenseEntry *spdx_license_lookup(const char *id) {
    int idx;
    if (!id || g_licenses.count == 0) return NULL;
    idx = lic_lower_bound(id);
    if (idx < g_licenses.count &&
        id_cmp_ci(g_licenses.items[idx].id, id) == 0)
        return &g_licenses.items[idx];
    return NULL;
}

/**
 * @brief Look up an exception entry by identifier.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return Pointer to the entry, or NULL if not found.
 */
const SpdxExceptionEntry *spdx_exception_lookup(const char *id) {
    int idx;
    if (!id || g_exceptions.count == 0) return NULL;
    idx = exc_lower_bound(id);
    if (idx < g_exceptions.count &&
        id_cmp_ci(g_exceptions.items[idx].id, id) == 0)
        return &g_exceptions.items[idx];
    return NULL;
}

/**
 * @brief Get the canonical license text.
 *
 * Loads the detail on demand.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return NUL-terminated text, or NULL if unavailable.
 */
const char *spdx_license_get_text(const char *id) {
    int idx;
    SpdxLicenseEntry *e;
    if (!id || !id[0]) return NULL;
    idx = lic_lower_bound(id);
    if (idx >= g_licenses.count) return NULL;
    if (id_cmp_ci(g_licenses.items[idx].id, id) != 0) return NULL;
    e = &g_licenses.items[idx];
    if (!e->detail_loaded) {
        if (spdx_license_load_detail(e->id) != 0) return NULL;
    }
    return e->text;
}

/**
 * @brief Get the canonical exception text.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return NUL-terminated text, or NULL if unavailable.
 */
const char *spdx_exception_get_text(const char *id) {
    int idx;
    SpdxExceptionEntry *e;
    if (!id || !id[0]) return NULL;
    idx = exc_lower_bound(id);
    if (idx >= g_exceptions.count) return NULL;
    if (id_cmp_ci(g_exceptions.items[idx].id, id) != 0) return NULL;
    e = &g_exceptions.items[idx];
    if (!e->detail_loaded) {
        if (spdx_exception_load_detail(e->id) != 0) return NULL;
    }
    return e->text;
}

/**
 * @brief Load a license detail on demand.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
int spdx_license_load_detail(const char *id) {
    int idx;
    SpdxLicenseEntry *e;
    idx = lic_lower_bound(id);
    if (idx >= g_licenses.count ||
        id_cmp_ci(g_licenses.items[idx].id, id) != 0) return -1;
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

/**
 * @brief Load an exception detail on demand.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
int spdx_exception_load_detail(const char *id) {
    int idx;
    SpdxExceptionEntry *e;
    idx = exc_lower_bound(id);
    if (idx >= g_exceptions.count ||
        id_cmp_ci(g_exceptions.items[idx].id, id) != 0) return -1;
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

/**
 * @brief Check whether an identifier is a valid SPDX license.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return 1 if valid, 0 otherwise.
 */
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

/**
 * @brief Check whether an identifier is a valid SPDX exception.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return 1 if valid, 0 otherwise.
 */
int spdx_exception_is_valid(const char *id) {
    if (!id || id[0] == '\0') return 0;
    if (g_exceptions.count == 0) return 1;
    return spdx_exception_lookup(id) != NULL;
}

/**
 * @brief Check whether a license identifier is deprecated.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return 1 if deprecated, 0 otherwise.
 */
int spdx_license_is_deprecated(const char *id) {
    const SpdxLicenseEntry *e = spdx_license_lookup(id);
    return e && (e->flags & SPDX_DB_FLAG_DEPRECATED) ? 1 : 0;
}

/**
 * @brief Check whether an exception identifier is deprecated.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return 1 if deprecated, 0 otherwise.
 */
int spdx_exception_is_deprecated(const char *id) {
    const SpdxExceptionEntry *e = spdx_exception_lookup(id);
    return e && (e->flags & SPDX_DB_FLAG_DEPRECATED) ? 1 : 0;
}

/**
 * @brief Check whether a license is OSI-approved.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return 1 if OSI-approved, 0 otherwise.
 */
int spdx_license_is_osi_approved(const char *id) {
    const SpdxLicenseEntry *e = spdx_license_lookup(id);
    return e && (e->flags & SPDX_DB_FLAG_OSI) ? 1 : 0;
}

/**
 * @brief Check whether a license is FSF-libre.
 *
 * @param[in] id  Identifier. Not NULL.
 *
 * @return 1 if FSF-libre, 0 otherwise.
 */
int spdx_license_is_fsf_libre(const char *id) {
    const SpdxLicenseEntry *e = spdx_license_lookup(id);
    return e && (e->flags & SPDX_DB_FLAG_FSF_LIBRE) ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* SPDX expression parser                                              */
/* ------------------------------------------------------------------ */

/**
 * @struct _EXPRPARSER
 * @brief Recursive descent parser state for SPDX expressions.
 */
typedef struct {
    const char *p;                 /**< Current position.              */
    const char *bad_token_start;   /**< Start of the offending token.  */
    int         err;               /**< SPDX_EXPR_* error code.        */
} EXPRPARSER;

/**
 * @brief Skip whitespace.
 */
static void skip_ws(EXPRPARSER *pp) {
    while (*pp->p && isspace((unsigned char)*pp->p)) pp->p++;
}

/**
 * @brief Check whether a keyword appears at @p p.
 */
static int is_kw_at(const char *p, const char *kw, int kwlen) {
    if (strncmp(p, kw, kwlen) != 0) return 0;
    if (p[kwlen] == '\0') return 1;
    if (isspace((unsigned char)p[kwlen])) return 1;
    if (p[kwlen] == '(' || p[kwlen] == ')') return 1;
    return 0;
}

static int parse_expression(EXPRPARSER *pp);

/**
 * @brief Parse one term.
 */
static int parse_term(EXPRPARSER *pp) {
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

/**
 * @brief Parse a full expression (terms joined by AND/OR).
 */
static int parse_expression(EXPRPARSER *pp) {
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

/**
 * @brief Validate an SPDX license expression.
 *
 * @param[in]  expr       Expression. Not NULL.
 * @param[out] bad_token  Optional. On SPDX_EXPR_UNKNOWN_TOKEN, receives
 *                        a pointer to the offending token inside
 *                        @p expr. May be NULL.
 *
 * @return SPDX_EXPR_OK, SPDX_EXPR_SYNTAX_ERROR or
 *         SPDX_EXPR_UNKNOWN_TOKEN.
 */
int spdx_expression_validate(const char *expr, const char **bad_token) {
    EXPRPARSER pp;
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

/* ------------------------------------------------------------------ */
/* SPDX expression normalization                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Normalize an SPDX license expression to canonical case.
 *
 * Every identifier from the SPDX License List or the SPDX Exceptions
 * list is replaced with its canonical case (for example,
 * 'BSD-3-clause' becomes 'BSD-3-Clause'). AND/OR/WITH, parentheses and
 * whitespace are preserved as-is. LicenseRef-* and DocumentRef-*
 * identifiers are left unchanged.
 *
 * @param[in] expr  Expression, or NULL.
 *
 * @return malloc'd normalized string, or NULL on OOM or NULL input.
 */
char *spdx_normalize_license_expression(const char *expr) {
    size_t cap = 128;
    size_t len = 0;
    char *out;
    const char *p;

    if (!expr) return NULL;

    out = (char*)malloc(cap);
    if (!out) return NULL;
    out[0] = '\0';

    p = expr;
    while (*p) {
        const char *start;
        size_t tok_len;
        char tok[256];
        const char *canonical;

        if (*p == ' ' || *p == '\t' || *p == '(' || *p == ')') {
            if (len + 1 >= cap) {
                size_t ncap = cap * 2;
                char *no = (char*)realloc(out, ncap);
                if (!no) { free(out); return NULL; }
                out = no; cap = ncap;
            }
            out[len++] = *p++;
            out[len] = '\0';
            continue;
        }

        start = p;
        while (*p && !isspace((unsigned char)*p) &&
               *p != '(' && *p != ')')
            p++;
        tok_len = (size_t)(p - start);
        if (tok_len >= sizeof(tok)) tok_len = sizeof(tok) - 1;
        memcpy(tok, start, tok_len);
        tok[tok_len] = '\0';

        if (strcmp(tok, "AND") == 0 ||
            strcmp(tok, "OR") == 0 ||
            strcmp(tok, "WITH") == 0) {
            canonical = tok;
        } else if (strncmp(tok, "LicenseRef-", 11) == 0 ||
                   strncmp(tok, "DocumentRef-", 12) == 0) {
            canonical = tok;
        } else {
            const SpdxLicenseEntry *e = spdx_license_lookup(tok);
            const SpdxExceptionEntry *ex = NULL;
            if (!e) ex = spdx_exception_lookup(tok);
            if (e) canonical = e->id;
            else if (ex) canonical = ex->id;
            else canonical = tok;
        }

        {
            size_t clen = strlen(canonical);
            if (len + clen + 1 > cap) {
                size_t ncap = cap * 2 + clen;
                char *no = (char*)realloc(out, ncap);
                if (!no) { free(out); return NULL; }
                out = no; cap = ncap;
            }
            memcpy(out + len, canonical, clen);
            len += clen;
            out[len] = '\0';
        }
    }

    return out;
}
