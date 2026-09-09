/* spdx-sbom.c - генератор SPDX SBOM (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <ctype.h>

#ifdef __LINUX__
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <direct.h>
#include <sys/stat.h>
#include <io.h>
#endif

#include <reuse_parser.h>
#include "spdx_tag.h"
#include "sha1_utils.h"
#include "omf_parser.h"

/* Глобальные параметры */
static char *default_license = NULL;
static char *default_copyright = NULL;
static char *doc_name = NULL;
static char *package_version = NULL;
static char *package_supplier = NULL;
static char *creator = NULL;
static char *package_purpose = NULL;
static char *binary_file = NULL;
static char **exclude_list = NULL;
static int exclude_count = 0;
static char **object_files = NULL;
static int object_count = 0;
static char *source_sbom_path = NULL; /* путь к SBOM исходников для внешней ссылки */

/* Внутренние структуры */
typedef struct {
    char name[512];
    char sha1[41];
    char license[256];
    char copyright[512];
    char file_type[32];
} FileInfo;

typedef struct {
    FileInfo *items;
    int count;
    int capacity;
} FileList;

typedef struct {
    char spdx_id[256];
    char name[512];
    char version[128];
    char supplier[256];
    char license[256];
    char copyright[512];
    char purpose[128];
    int files_analyzed;
    char verification_code[41];
} PackageInfo;

typedef struct {
    char element_id[256];
    char related_element[256];
    char relationship_type[64];
} Relationship;

typedef struct {
    char spdx_version[16];
    char document_id[256];
    char document_name[512];
    char document_namespace[512];
    char created[32];
    char creator[256];
    char data_license[64];
    PackageInfo package;
    FileList files;
    Relationship *relationships;
    int relationship_count;
    /* Поля для внешней ссылки */
    int has_external_ref;
    char external_doc_id[256];
    char external_doc_uri[512];
    char external_doc_checksum[128]; /* SHA1 в hex */
} SpdxDocument;

/* Прототипы функций */
static void filelist_init(FileList *list);
static void filelist_add(FileList *list, const FileInfo *info);
static void filelist_free(FileList *list);
static const char *get_file_name(const char *path);
static void remove_extension(char *str);
static void sanitize_id(const char *src, char *dst, size_t dst_size);
static void make_package_spdx_id(const char *base_name, const char *suffix,
                                 char *buf, size_t buf_size);
static char *json_escape(const char *src);
static int str_ieq(const char *a, const char *b);
static int str_ieq_prefix(const char *str, const char *prefix, size_t n);
static int is_license_file(const char *filename);
static int is_ignored_dir(const char *name);
static int is_license_sidecar(const char *name);
static int is_excluded(const char *filename);
const char* get_file_type(const char *filename);
static void get_file_license_copyright(const char *fullpath, const char *filename,
                                       ReuseConfig *config,
                                       char *license_out, size_t license_size,
                                       char *copyright_out, size_t copyright_size);
static void scan_source_files(const char *dir, ReuseConfig *config, FileList *list);
static int collect_sources_from_objects(char ***sources, int *sources_count);
static char *compute_verification_code(FileList *list);
static void build_document(SpdxDocument *doc, const char *dir, ReuseConfig *config,
                           const char *name, char **source_files,
                           int source_files_count, int use_objects);
static void init_relationships(SpdxDocument *doc, int binary_mode, const char *file_base);
static void output_json(const SpdxDocument *doc);
static void output_tagvalue(const SpdxDocument *doc);

/* --------------------------------------------------------------------------
 * Реализация вспомогательных функций
 * ------------------------------------------------------------------------ */

static void filelist_init(FileList *list) {
    list->count = 0;
    list->capacity = 16;
    list->items = (FileInfo*)malloc(list->capacity * sizeof(FileInfo));
    if (!list->items) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
}

static void filelist_add(FileList *list, const FileInfo *info) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (FileInfo*)realloc(list->items, list->capacity * sizeof(FileInfo));
        if (!list->items) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    list->items[list->count++] = *info;
}

static void filelist_free(FileList *list) {
    free(list->items);
}

static const char *get_file_name(const char *path) {
    const char *slash = strrchr(path, '/');
    const char *backslash = strrchr(path, '\\');
    if (backslash && (!slash || backslash > slash))
        return backslash + 1;
    if (slash)
        return slash + 1;
    return path;
}

static void remove_extension(char *str) {
    char *dot = strrchr(str, '.');
    if (dot) *dot = '\0';
}

static void sanitize_id(const char *src, char *dst, size_t dst_size) {
    size_t i = 0, j = 0, start, end;
    for (i = 0; src[i] != '\0' && j < dst_size - 1; i++) {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c) || c == '.' || c == '-')
            dst[j++] = (char)c;
        else
            dst[j++] = '-';
    }
    dst[j] = '\0';
    start = 0;
    end = j;
    while (start < end && dst[start] == '-') start++;
    while (end > start && dst[end-1] == '-') end--;
    if (start > 0 || end < j) {
        memmove(dst, dst + start, end - start);
        dst[end - start] = '\0';
    }
}

static void make_package_spdx_id(const char *base_name, const char *suffix,
                                 char *buf, size_t buf_size) {
    char sanitized[256];
    sanitize_id(base_name, sanitized, sizeof(sanitized));
    if (suffix && *suffix)
        snprintf(buf, buf_size, "SPDXRef-Package-%s-%s", sanitized, suffix);
    else
        snprintf(buf, buf_size, "SPDXRef-Package-%s", sanitized);
}

static char *json_escape(const char *src) {
    size_t len, extra, i, j;
    char *dst;
    len = strlen(src);
    extra = 0;
    for (i = 0; i < len; i++) {
        if (src[i] == '"' || src[i] == '\\' || src[i] == '\n' ||
            src[i] == '\r' || src[i] == '\t')
            extra++;
    }
    dst = (char*)malloc(len + extra + 1);
    if (!dst) return NULL;
    j = 0;
    for (i = 0; i < len; i++) {
        switch (src[i]) {
            case '"':  dst[j++] = '\\'; dst[j++] = '"'; break;
            case '\\': dst[j++] = '\\'; dst[j++] = '\\'; break;
            case '\n': dst[j++] = '\\'; dst[j++] = 'n'; break;
            case '\r': dst[j++] = '\\'; dst[j++] = 'r'; break;
            case '\t': dst[j++] = '\\'; dst[j++] = 't'; break;
            default:   dst[j++] = src[i]; break;
        }
    }
    dst[j] = '\0';
    return dst;
}

static int str_ieq(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static int str_ieq_prefix(const char *str, const char *prefix, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (str[i] == '\0') return 0;
        if (tolower((unsigned char)str[i]) != tolower((unsigned char)prefix[i])) return 0;
    }
    return 1;
}

static int is_license_file(const char *filename) {
    static const char *exact[] = {"license","licence","copying","unlicense","copyright"};
    const char *base = get_file_name(filename);
    size_t i;
    for (i = 0; i < sizeof(exact)/sizeof(exact[0]); i++)
        if (str_ieq(base, exact[i])) return 1;
    if (str_ieq_prefix(base, "license", 7) ||
        str_ieq_prefix(base, "licence", 7) ||
        str_ieq_prefix(base, "copying", 7)) {
        if (base[7] == '\0' || base[7] == '.' || base[7] == '-' || base[7] == '_')
            return 1;
    }
    return 0;
}

static int is_ignored_dir(const char *name) {
    return strcmp(name, "LICENSES") == 0 || strcmp(name, ".reuse") == 0;
}

static int is_license_sidecar(const char *name) {
    size_t len = strlen(name);
    return len > 8 && strcmp(name + len - 8, ".license") == 0;
}

static int is_excluded(const char *filename) {
    int i;
    const char *base = get_file_name(filename);
    for (i = 0; i < exclude_count; i++)
        if (strcmp(exclude_list[i], base) == 0) return 1;
    return 0;
}

const char* get_file_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return "OTHER";
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 ||
        strcmp(ext, ".h") == 0 || strcmp(ext, ".asm") == 0 ||
        strcmp(ext, ".rc") == 0) return "SOURCE";
    if (strcmp(ext, ".ico") == 0 || strcmp(ext, ".bmp") == 0 ||
        strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0) return "IMAGE";
    if (strcmp(ext, ".txt") == 0 || strcmp(ext, ".md") == 0) return "TEXT";
    if (strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
        strcmp(ext, ".sys") == 0 || strcmp(ext, ".lib") == 0) return "BINARY";
    return "OTHER";
}

static void get_file_license_copyright(const char *fullpath, const char *filename,
                                       ReuseConfig *config,
                                       char *license_out, size_t license_size,
                                       char *copyright_out, size_t copyright_size) {
    char *tag_lic, *tag_copy, sidecar_path[1024], *side_lic, *side_copy;
    int sidecar_exists;

    license_out[0] = '\0';
    copyright_out[0] = '\0';

    tag_lic = file_get_spdx_license(fullpath);
    tag_copy = file_get_spdx_copyright(fullpath);
    if (tag_lic) {
        strncpy(license_out, tag_lic, license_size-1);
        license_out[license_size-1] = '\0';
        free(tag_lic);
    }
    if (tag_copy) {
        strncpy(copyright_out, tag_copy, copyright_size-1);
        copyright_out[copyright_size-1] = '\0';
        free(tag_copy);
    }

    snprintf(sidecar_path, sizeof(sidecar_path), "%s.license", fullpath);
#ifdef __LINUX__
    sidecar_exists = (access(sidecar_path, F_OK) == 0);
#else
    sidecar_exists = (_access(sidecar_path, 0) == 0);
#endif
    if (sidecar_exists) {
        side_lic = file_get_spdx_license(sidecar_path);
        side_copy = file_get_spdx_copyright(sidecar_path);
        if (license_out[0] == '\0' && side_lic) {
            strncpy(license_out, side_lic, license_size-1);
            license_out[license_size-1] = '\0';
        }
        if (copyright_out[0] == '\0' && side_copy) {
            strncpy(copyright_out, side_copy, copyright_size-1);
            copyright_out[copyright_size-1] = '\0';
        }
        if (side_lic) free(side_lic);
        if (side_copy) free(side_copy);
    }

    if (license_out[0] == '\0') {
        const char *lic = find_license_for_file(config, filename);
        if (lic) {
            strncpy(license_out, lic, license_size-1);
            license_out[license_size-1] = '\0';
        }
    }
    if (copyright_out[0] == '\0') {
        const char *cop = find_copyright_for_file(config, filename);
        if (cop) {
            strncpy(copyright_out, cop, copyright_size-1);
            copyright_out[copyright_size-1] = '\0';
        }
    }

    if (license_out[0] == '\0' && default_license) {
        strncpy(license_out, default_license, license_size-1);
        license_out[license_size-1] = '\0';
    }
    if (copyright_out[0] == '\0' && default_copyright) {
        strncpy(copyright_out, default_copyright, copyright_size-1);
        copyright_out[copyright_size-1] = '\0';
    }
}

static void scan_source_files(const char *dir, ReuseConfig *config, FileList *list) {
#ifdef __LINUX__
    DIR *d;
    struct dirent *entry;
    struct stat st;
    char fullpath[1024];
    FileInfo info;
    char *sha1;

    d = opendir(dir);
    if (!d) return;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (entry->d_type == DT_DIR && is_ignored_dir(entry->d_name)) continue;
        if (is_license_sidecar(entry->d_name)) continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, entry->d_name);
        if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) continue;
        if (is_license_file(entry->d_name) || is_excluded(entry->d_name)) continue;

        memset(&info, 0, sizeof(info));
        strncpy(info.name, entry->d_name, sizeof(info.name)-1);
        sha1 = sha1_file(fullpath);
        if (sha1) {
            strncpy(info.sha1, sha1, sizeof(info.sha1)-1);
            free(sha1);
        }
        strncpy(info.file_type, get_file_type(entry->d_name), sizeof(info.file_type)-1);
        get_file_license_copyright(fullpath, entry->d_name, config,
                                   info.license, sizeof(info.license),
                                   info.copyright, sizeof(info.copyright));
        if (info.license[0] == '\0') {
            fprintf(stderr, "Error: No license found for file: %s\n", fullpath);
            exit(EXIT_FAILURE);
        }
        filelist_add(list, &info);
    }
    closedir(d);
#else
    long hFile;
    struct _finddata_t fdata;
    struct stat st;
    char pattern[1024], fullpath[1024];
    FileInfo info;
    char *sha1;

    snprintf(pattern, sizeof(pattern), "%s/*", dir);
    hFile = _findfirst(pattern, &fdata);
    if (hFile == -1L) return;
    do {
        if (fdata.name[0] == '.') continue;
        if (is_license_sidecar(fdata.name)) continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, fdata.name);
        if (stat(fullpath, &st) == 0 && (st.st_mode & _S_IFDIR)) continue;
        if (is_license_file(fdata.name) || is_excluded(fdata.name)) continue;

        memset(&info, 0, sizeof(info));
        strncpy(info.name, fdata.name, sizeof(info.name)-1);
        sha1 = sha1_file(fullpath);
        if (sha1) {
            strncpy(info.sha1, sha1, sizeof(info.sha1)-1);
            free(sha1);
        }
        strncpy(info.file_type, get_file_type(fdata.name), sizeof(info.file_type)-1);
        get_file_license_copyright(fullpath, fdata.name, config,
                                   info.license, sizeof(info.license),
                                   info.copyright, sizeof(info.copyright));
        if (info.license[0] == '\0') {
            fprintf(stderr, "Error: No license found for file: %s\n", fullpath);
            exit(EXIT_FAILURE);
        }
        filelist_add(list, &info);
    } while (_findnext(hFile, &fdata) == 0);
    _findclose(hFile);
#endif
}

static int collect_sources_from_objects(char ***sources, int *sources_count) {
    int i, j, k;
    char **obj_sources;
    int obj_count;
    char *src;
    char **temp;
    int found;

    *sources = NULL;
    *sources_count = 0;

    for (i = 0; i < object_count; i++) {
        obj_sources = NULL;
        obj_count = 0;
        if (omf_extract_sources(object_files[i], &obj_sources, &obj_count) != 0) {
            fprintf(stderr, "Warning: cannot extract sources from %s\n", object_files[i]);
            continue;
        }
        for (j = 0; j < obj_count; j++) {
            found = 0;
            for (k = 0; k < *sources_count; k++) {
                if (strcmp((*sources)[k], obj_sources[j]) == 0) {
                    found = 1;
                    break;
                }
            }
            if (found) continue;

            src = (char*)malloc(strlen(obj_sources[j]) + 1);
            if (!src) {
                for (k = 0; k < *sources_count; k++) free((*sources)[k]);
                free(*sources);
                *sources = NULL;
                *sources_count = 0;
                for (k = 0; k < obj_count; k++) free(obj_sources[k]);
                free(obj_sources);
                return -1;
            }
            strcpy(src, obj_sources[j]);

            temp = (char**)realloc(*sources, (*sources_count + 1) * sizeof(char*));
            if (!temp) {
                free(src);
                for (k = 0; k < *sources_count; k++) free((*sources)[k]);
                free(*sources);
                *sources = NULL;
                *sources_count = 0;
                for (k = 0; k < obj_count; k++) free(obj_sources[k]);
                free(obj_sources);
                return -1;
            }
            *sources = temp;
            (*sources)[*sources_count] = src;
            (*sources_count)++;
        }
        for (k = 0; k < obj_count; k++) free(obj_sources[k]);
        free(obj_sources);
    }
    return (*sources_count > 0) ? 0 : -1;
}

static char *compute_verification_code(FileList *list) {
    FileInfo *entries;
    size_t total_len;
    int i;
    char *concat;
    char combined[41];
    char *code;

    if (list->count == 0) return NULL;

    entries = (FileInfo*)malloc(list->count * sizeof(FileInfo));
    if (!entries) return NULL;
    memcpy(entries, list->items, list->count * sizeof(FileInfo));

    /* простая сортировка пузырьком */
    for (i = 0; i < list->count - 1; i++) {
        int j;
        for (j = i + 1; j < list->count; j++) {
            if (strcmp(entries[i].name, entries[j].name) > 0) {
                FileInfo tmp = entries[i];
                entries[i] = entries[j];
                entries[j] = tmp;
            }
        }
    }

    total_len = 0;
    for (i = 0; i < list->count; i++)
        total_len += strlen(entries[i].sha1);

    concat = (char*)malloc(total_len + 1);
    if (!concat) {
        free(entries);
        return NULL;
    }
    concat[0] = '\0';
    for (i = 0; i < list->count; i++)
        strcat(concat, entries[i].sha1);

    sha1_string(concat, combined);
    free(concat);
    free(entries);

    code = (char*)malloc(41);
    if (code)
        memcpy(code, combined, 41);
    return code;
}

static void build_document(SpdxDocument *doc, const char *dir, ReuseConfig *config,
                           const char *name, char **source_files,
                           int source_files_count, int use_objects) {
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    char date[32];
    int binary_mode;
    char file_base[256];
    char pkg_spdx_id[256];
    const char *pkg_license;
    const char *pkg_copyright;
    FileList file_list;
    char *verification_code;
    char ns[256], safe[256], *sha1;
    int i;

    memset(doc, 0, sizeof(SpdxDocument));
    strcpy(doc->spdx_version, "SPDX-2.3");
    strcpy(doc->document_id, "SPDXRef-DOCUMENT");
    strcpy(doc->data_license, "CC0-1.0");

    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);
    strcpy(doc->created, date);

    if (creator) {
        strncpy(doc->creator, creator, sizeof(doc->creator)-1);
        doc->creator[sizeof(doc->creator)-1] = '\0';
    } else {
        strcpy(doc->creator, "Tool: osFree SPDX SBOM Generator");
    }

    snprintf(doc->document_name, sizeof(doc->document_name), "%s SBOM", name);

    sanitize_id(name, safe, sizeof(safe));
    sprintf(ns, "https://osfree.org/spdxdocs/%s-%ld", safe, (long)now);
    strcpy(doc->document_namespace, ns);

    binary_mode = (package_purpose && strcmp(package_purpose, "SOURCE") != 0);

    strncpy(file_base, get_file_name(binary_file), sizeof(file_base)-1);
    file_base[sizeof(file_base)-1] = '\0';
    remove_extension(file_base);

    make_package_spdx_id(file_base, binary_mode ? NULL : "Source",
                         pkg_spdx_id, sizeof(pkg_spdx_id));

    pkg_license = default_license;
    if (!pkg_license && config) pkg_license = config->default_license;

    if (!pkg_license || pkg_license[0] == '\0') {
        fprintf(stderr, "Error: No license specified for package '%s'. Use --default-license or REUSE.toml default.\n", name);
        exit(EXIT_FAILURE);
    }

    pkg_copyright = default_copyright;
    if (!pkg_copyright && config) pkg_copyright = config->default_copyright;

    strcpy(doc->package.spdx_id, pkg_spdx_id);
    strcpy(doc->package.name, name);
    if (package_version) strcpy(doc->package.version, package_version);
    if (package_supplier) strcpy(doc->package.supplier, package_supplier);
    strcpy(doc->package.license, pkg_license);
    if (pkg_copyright) strcpy(doc->package.copyright, pkg_copyright);
    if (package_purpose) strcpy(doc->package.purpose, package_purpose);

    filelist_init(&file_list);

    if (binary_mode) {
        FileInfo info;
        memset(&info, 0, sizeof(info));
        strncpy(info.name, get_file_name(binary_file), sizeof(info.name)-1);
        sha1 = sha1_file(binary_file);
        if (sha1) {
            strncpy(info.sha1, sha1, sizeof(info.sha1)-1);
            free(sha1);
        }
        strncpy(info.file_type, "BINARY", sizeof(info.file_type)-1);
        strcpy(info.license, pkg_license);
        if (pkg_copyright) strcpy(info.copyright, pkg_copyright);
        filelist_add(&file_list, &info);
    } else {
        if (use_objects) {
            for (i = 0; i < source_files_count; i++) {
                FileInfo info;
                memset(&info, 0, sizeof(info));
                strncpy(info.name, get_file_name(source_files[i]), sizeof(info.name)-1);
                sha1 = sha1_file(source_files[i]);
                if (sha1) {
                    strncpy(info.sha1, sha1, sizeof(info.sha1)-1);
                    free(sha1);
                }
                strncpy(info.file_type, get_file_type(source_files[i]), sizeof(info.file_type)-1);
                get_file_license_copyright(source_files[i], get_file_name(source_files[i]),
                                           config, info.license, sizeof(info.license),
                                           info.copyright, sizeof(info.copyright));
                if (info.license[0] == '\0') {
                    fprintf(stderr, "Error: No license found for file: %s\n", source_files[i]);
                    exit(EXIT_FAILURE);
                }
                filelist_add(&file_list, &info);
            }
        } else {
            scan_source_files(dir, config, &file_list);
        }
    }

    verification_code = compute_verification_code(&file_list);
    if (verification_code) {
        doc->package.files_analyzed = 1;
        strcpy(doc->package.verification_code, verification_code);
        free(verification_code);
    } else {
        doc->package.files_analyzed = 0;
        doc->package.verification_code[0] = '\0';
    }

    doc->files = file_list;
    doc->has_external_ref = 0;
}

static void init_relationships(SpdxDocument *doc, int binary_mode, const char *file_base) {
    doc->relationship_count = 0;
    doc->relationships = (Relationship*)malloc(sizeof(Relationship) * 2);
    if (!doc->relationships) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(doc->relationships[doc->relationship_count].element_id, doc->document_id);
    strcpy(doc->relationships[doc->relationship_count].related_element, doc->package.spdx_id);
    strcpy(doc->relationships[doc->relationship_count].relationship_type, "DESCRIBES");
    doc->relationship_count++;

    if (binary_mode) {
        char src_id[256];
        make_package_spdx_id(file_base, "Source", src_id, sizeof(src_id));
        strcpy(doc->relationships[doc->relationship_count].element_id, doc->package.spdx_id);
        strcpy(doc->relationships[doc->relationship_count].related_element, src_id);
        strcpy(doc->relationships[doc->relationship_count].relationship_type, "GENERATED_FROM");
        doc->relationship_count++;
    }
}

static void output_json(const SpdxDocument *doc) {
    int i;
    char *escaped;
    const PackageInfo *pkg = &doc->package;

    printf("{\n");
    printf("  \"spdxVersion\": \"%s\",\n", doc->spdx_version);
    printf("  \"SPDXID\": \"%s\",\n", doc->document_id);
    printf("  \"name\": \"%s\",\n", doc->document_name);
    printf("  \"creationInfo\": {\n");
    printf("    \"created\": \"%s\",\n", doc->created);
    printf("    \"creators\": [\"%s\"]\n", doc->creator);
    printf("  },\n");
    printf("  \"dataLicense\": \"%s\",\n", doc->data_license);
    printf("  \"documentNamespace\": \"%s\",\n", doc->document_namespace);

    /* externalDocumentRefs */
    if (doc->has_external_ref) {
        printf("  \"externalDocumentRefs\": [\n");
        printf("    {\n");
        printf("      \"externalDocumentId\": \"%s\",\n", doc->external_doc_id);
        escaped = json_escape(doc->external_doc_uri);
        printf("      \"spdxDocument\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
        printf("      \"checksum\": {\n");
        printf("        \"algorithm\": \"SHA1\",\n");
        printf("        \"checksumValue\": \"%s\"\n", doc->external_doc_checksum);
        printf("      }\n");
        printf("    }\n");
        printf("  ],\n");
    }

    printf("  \"packages\": [\n");
    printf("    {\n");
    printf("      \"SPDXID\": \"%s\",\n", pkg->spdx_id);
    escaped = json_escape(pkg->name);
    printf("      \"name\": \"%s\",\n", escaped ? escaped : "");
    free(escaped);
    printf("      \"downloadLocation\": \"NOASSERTION\",\n");
    if (pkg->version[0]) {
        escaped = json_escape(pkg->version);
        printf("      \"versionInfo\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
    } else printf("      \"versionInfo\": \"NOASSERTION\",\n");
    if (pkg->supplier[0]) {
        escaped = json_escape(pkg->supplier);
        printf("      \"supplier\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
    } else printf("      \"supplier\": \"NOASSERTION\",\n");
    escaped = json_escape(pkg->license);
    printf("      \"licenseConcluded\": \"%s\",\n", escaped ? escaped : "");
    printf("      \"licenseDeclared\": \"%s\",\n", escaped ? escaped : "");
    free(escaped);
    if (pkg->copyright[0]) {
        escaped = json_escape(pkg->copyright);
        printf("      \"copyrightText\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
    } else printf("      \"copyrightText\": \"NOASSERTION\",\n");
    if (pkg->purpose[0]) {
        escaped = json_escape(pkg->purpose);
        printf("      \"primaryPackagePurpose\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
    } else printf("      \"primaryPackagePurpose\": \"NOASSERTION\",\n");
    if (pkg->files_analyzed) {
        printf("      \"filesAnalyzed\": true,\n");
        printf("      \"packageVerificationCode\": {\n");
        printf("        \"packageVerificationCodeValue\": \"%s\",\n", pkg->verification_code);
        printf("        \"packageVerificationCodeExcludedFiles\": []\n");
        printf("      }\n");
    } else printf("      \"filesAnalyzed\": false\n");
    printf("    }\n");
    printf("  ],\n");

    printf("  \"files\": [\n");
    for (i = 0; i < doc->files.count; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        escaped = json_escape(doc->files.items[i].name);
        printf("      \"fileName\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
        escaped = json_escape(doc->files.items[i].name);
        printf("      \"SPDXID\": \"SPDXRef-File-%s\",\n", escaped ? escaped : "");
        free(escaped);
        printf("      \"fileTypes\": [\"%s\"],\n", doc->files.items[i].file_type);
        if (doc->files.items[i].sha1[0])
            printf("      \"checksums\": [{\"algorithm\": \"SHA1\", \"checksumValue\": \"%s\"}],\n", doc->files.items[i].sha1);
        else
            printf("      \"checksums\": [{\"algorithm\": \"SHA1\", \"checksumValue\": \"\"}],\n");
        escaped = json_escape(doc->files.items[i].license);
        printf("      \"licenseConcluded\": \"%s\",\n", escaped ? escaped : "");
        printf("      \"licenseInfoInFiles\": [\"%s\"]", escaped ? escaped : "");
        free(escaped);
        if (doc->files.items[i].copyright[0]) {
            escaped = json_escape(doc->files.items[i].copyright);
            printf(",\n      \"copyrightText\": \"%s\"", escaped ? escaped : "");
            free(escaped);
        } else {
            printf(",\n      \"copyrightText\": \"NOASSERTION\"");
        }
        printf("\n    }");
    }
    printf("\n  ],\n");

    printf("  \"relationships\": [\n");
    for (i = 0; i < doc->relationship_count; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        escaped = json_escape(doc->relationships[i].element_id);
        printf("      \"spdxElementId\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
        escaped = json_escape(doc->relationships[i].related_element);
        printf("      \"relatedSpdxElement\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
        escaped = json_escape(doc->relationships[i].relationship_type);
        printf("      \"relationshipType\": \"%s\"\n", escaped ? escaped : "");
        free(escaped);
        printf("    }");
    }
    printf("\n  ]\n");
    printf("}\n");
}

static void output_tagvalue(const SpdxDocument *doc) {
    int i;
    const PackageInfo *pkg = &doc->package;

    printf("SPDXVersion: %s\n", doc->spdx_version);
    printf("DataLicense: %s\n", doc->data_license);
    printf("SPDXID: %s\n", doc->document_id);
    printf("DocumentName: %s\n", doc->document_name);
    printf("DocumentNamespace: %s\n", doc->document_namespace);
    printf("Creator: %s\n", doc->creator);
    printf("Created: %s\n", doc->created);
    printf("\n");

    printf("##### Package: %s\n", pkg->name);
    printf("PackageName: %s\n", pkg->name);
    printf("SPDXID: %s\n", pkg->spdx_id);
    printf("PackageDownloadLocation: NOASSERTION\n");
    if (pkg->version[0]) printf("PackageVersion: %s\n", pkg->version);
    if (pkg->supplier[0]) printf("PackageSupplier: %s\n", pkg->supplier);
    printf("PackageLicenseConcluded: %s\n", pkg->license);
    printf("PackageLicenseDeclared: %s\n", pkg->license);
    if (pkg->copyright[0]) printf("PackageCopyrightText: %s\n", pkg->copyright);
    if (pkg->purpose[0]) printf("PackagePrimaryPurpose: %s\n", pkg->purpose);
    if (pkg->files_analyzed) {
        printf("FilesAnalyzed: true\n");
        printf("PackageVerificationCode: %s\n", pkg->verification_code);
    } else {
        printf("FilesAnalyzed: false\n");
    }
    printf("\n");

    for (i = 0; i < doc->files.count; i++) {
        printf("FileName: %s\n", doc->files.items[i].name);
        printf("SPDXID: SPDXRef-File-%s\n", doc->files.items[i].name);
        printf("FileType: %s\n", doc->files.items[i].file_type);
        printf("FileChecksum: SHA1: %s\n", doc->files.items[i].sha1);
        printf("LicenseConcluded: %s\n", doc->files.items[i].license);
        printf("LicenseInfoInFile: %s\n", doc->files.items[i].license);
        if (doc->files.items[i].copyright[0])
            printf("FileCopyrightText: %s\n", doc->files.items[i].copyright);
        printf("\n");
    }

    for (i = 0; i < doc->relationship_count; i++) {
        printf("Relationship: %s %s %s\n",
               doc->relationships[i].element_id,
               doc->relationships[i].relationship_type,
               doc->relationships[i].related_element);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    const char *dir = ".";
    const char *output = NULL;
    const char *format = "json";
    int i;
    ReuseConfig *config = NULL;
    char *arg;
    char toml_path_buf[1024];
    char **source_files = NULL;
    int source_files_count = 0;
    SpdxDocument doc;
    int use_objects_flag;
    int binary_mode;
    char base_name_no_ext[256];

    /* Инициализация глобальных переменных */
    default_license = NULL;
    default_copyright = NULL;
    doc_name = NULL;
    package_version = NULL;
    package_supplier = NULL;
    creator = NULL;
    package_purpose = NULL;
    binary_file = NULL;
    exclude_list = NULL;
    exclude_count = 0;
    object_files = NULL;
    object_count = 0;
    source_sbom_path = NULL;

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--output=", 9) == 0)
            output = argv[i] + 9;
        else if (strncmp(argv[i], "--format=", 9) == 0)
            format = argv[i] + 9;
        else if (strncmp(argv[i], "--default-license=", 18) == 0)
            default_license = argv[i] + 18;
        else if (strncmp(argv[i], "--default-copyright=", 20) == 0)
            default_copyright = argv[i] + 20;
        else if (strncmp(argv[i], "--name=", 7) == 0)
            doc_name = argv[i] + 7;
        else if (strncmp(argv[i], "--version=", 10) == 0)
            package_version = argv[i] + 10;
        else if (strncmp(argv[i], "--supplier=", 11) == 0)
            package_supplier = argv[i] + 11;
        else if (strncmp(argv[i], "--creator=", 10) == 0)
            creator = argv[i] + 10;
        else if (strncmp(argv[i], "--purpose=", 10) == 0)
            package_purpose = argv[i] + 10;
        else if (strncmp(argv[i], "--file=", 7) == 0)
            binary_file = argv[i] + 7;
        else if (strncmp(argv[i], "--objects=", 10) == 0) {
            char *obj_list = (char*)malloc(strlen(argv[i] + 10) + 1);
            if (!obj_list) {
                fprintf(stderr, "Memory allocation failed\n");
                return 1;
            }
            strcpy(obj_list, argv[i] + 10);
            arg = strtok(obj_list, " ");
            while (arg) {
                object_files = (char**)realloc(object_files, (object_count + 1) * sizeof(char*));
                if (!object_files) {
                    fprintf(stderr, "Memory allocation failed\n");
                    free(obj_list);
                    return 1;
                }
                object_files[object_count] = arg;
                object_count++;
                arg = strtok(NULL, " ");
            }
        }
        else if (strncmp(argv[i], "--source-sbom=", 14) == 0) {
            source_sbom_path = argv[i] + 14;
        }
        else if (strncmp(argv[i], "--exclude=", 10) == 0) {
            arg = argv[i] + 10;
            while (*arg) {
                while (*arg && *arg == ' ') arg++;
                if (!*arg) break;
                exclude_list = (char**)realloc(exclude_list, (exclude_count + 1) * sizeof(char*));
                exclude_list[exclude_count++] = arg;
                while (*arg && *arg != ' ') arg++;
                if (*arg) { *arg = '\0'; arg++; }
            }
        } else {
            dir = argv[i];
        }
    }

    if (!binary_file) {
        fprintf(stderr, "Error: --file=<binary file> is required\n");
        return 1;
    }
    if (!doc_name) {
        fprintf(stderr, "Error: --name=<package name> is required\n");
        return 1;
    }

#ifdef __LINUX__
    snprintf(toml_path_buf, sizeof(toml_path_buf), "%s/REUSE.toml", dir);
#else
    snprintf(toml_path_buf, sizeof(toml_path_buf), "%s\\REUSE.toml", dir);
#endif
    config = parse_reuse_toml(toml_path_buf);

    if (object_count > 0) {
        if (collect_sources_from_objects(&source_files, &source_files_count) != 0) {
            /* пусто */
        }
        use_objects_flag = 1;
    } else {
        use_objects_flag = 0;
    }

    build_document(&doc, dir, config, doc_name, source_files, source_files_count, use_objects_flag);

    binary_mode = (package_purpose && strcmp(package_purpose, "SOURCE") != 0);

    strncpy(base_name_no_ext, get_file_name(binary_file), sizeof(base_name_no_ext)-1);
    base_name_no_ext[sizeof(base_name_no_ext)-1] = '\0';
    remove_extension(base_name_no_ext);

    init_relationships(&doc, binary_mode, base_name_no_ext);

    /* Если бинарный режим и указан source-sbom, заполняем внешнюю ссылку */
    if (binary_mode && source_sbom_path) {
        char *checksum = sha1_file(source_sbom_path);
        if (!checksum) {
            fprintf(stderr, "Error: Cannot compute SHA1 for source SBOM: %s\n", source_sbom_path);
            exit(EXIT_FAILURE);
        }
        strcpy(doc.external_doc_id, "DocumentRef-source");
        strcpy(doc.external_doc_uri, get_file_name(source_sbom_path));
        strcpy(doc.external_doc_checksum, checksum);
        free(checksum);
        doc.has_external_ref = 1;

        /* Перезаписываем relatedSpdxElement в отношении GENERATED_FROM */
        if (doc.relationship_count >= 2) {
            char full_ref[600];
            sprintf(full_ref, "%s:%s", doc.external_doc_id,
                    doc.relationships[1].related_element);
            strcpy(doc.relationships[1].related_element, full_ref);
        }
    } else {
        doc.has_external_ref = 0;
    }

    if (output) {
        if (!freopen(output, "w", stdout)) {
            fprintf(stderr, "Cannot open output file: %s\n", output);
            exit(EXIT_FAILURE);
        }
    }

    if (strcmp(format, "json") == 0) output_json(&doc);
    else if (strcmp(format, "tagvalue") == 0 || strcmp(format, "tag") == 0) output_tagvalue(&doc);
    else {
        fprintf(stderr, "Unsupported format: %s\n", format);
        exit(EXIT_FAILURE);
    }

    free(doc.files.items);
    free(doc.relationships);
    if (source_files) {
        for (i = 0; i < source_files_count; i++) free(source_files[i]);
        free(source_files);
    }
    free_reuse_config(config);
    return 0;
}
