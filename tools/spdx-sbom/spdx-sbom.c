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

/* Структура для хранения информации о файле */
typedef struct {
    char name[512];
    char sha1[41];
    char license[256];
    char copyright[512];
    char file_type[32];
} FileInfo;

/* Динамический массив FileInfo */
typedef struct {
    FileInfo *items;
    int count;
    int capacity;
} FileList;

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

/* Вспомогательные функции */
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

    /* Удаляем ведущие и хвостовые дефисы */
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
        if (str[i] == '\0')
            return 0;
        if (tolower((unsigned char)str[i]) != tolower((unsigned char)prefix[i]))
            return 0;
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

/* Определяет лицензию и копирайт для файла */
static void get_file_license_copyright(const char *fullpath, const char *filename,
                                       ReuseConfig *config,
                                       char *license_out, size_t license_size,
                                       char *copyright_out, size_t copyright_size) {
    char *tag_lic;
    char *tag_copy;
    char sidecar_path[1024];
    int sidecar_exists;
    char *side_lic;
    char *side_copy;

    license_out[0] = '\0';
    copyright_out[0] = '\0';

    /* 1. SPDX-теги внутри файла */
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

    /* 2. .license sidecar */
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

    /* 3. REUSE.toml annotations (включая default) */
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

    /* 4. --default-license/copyright */
    if (license_out[0] == '\0' && default_license) {
        strncpy(license_out, default_license, license_size-1);
        license_out[license_size-1] = '\0';
    }
    if (copyright_out[0] == '\0' && default_copyright) {
        strncpy(copyright_out, default_copyright, copyright_size-1);
        copyright_out[copyright_size-1] = '\0';
    }
}

/* Сбор информации о файлах из каталога */
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
    char pattern[1024];
    char fullpath[1024];
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

/* Извлекает список исходных файлов из OBJ-файлов, переданных в object_files */
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
            /* Проверка на дубликаты */
            found = 0;
            for (k = 0; k < *sources_count; k++) {
                if (strcmp((*sources)[k], obj_sources[j]) == 0) {
                    found = 1;
                    break;
                }
            }
            if (found)
                continue;

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

/* Вывод полей пакета */
static void print_package_fields(const char *spdx_id, const char *name,
                                 const char *version, const char *supplier,
                                 const char *license, const char *copyright,
                                 const char *purpose) {
    char *escaped;

    printf("    {\n");
    printf("      \"SPDXID\": \"%s\",\n", spdx_id);
    escaped = json_escape(name);
    printf("      \"name\": \"%s\",\n", escaped ? escaped : "");
    free(escaped);
    printf("      \"downloadLocation\": \"NOASSERTION\",\n");
    if (version) {
        escaped = json_escape(version);
        printf("      \"versionInfo\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
    } else
        printf("      \"versionInfo\": \"NOASSERTION\",\n");
    if (supplier) {
        escaped = json_escape(supplier);
        printf("      \"supplier\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
    } else
        printf("      \"supplier\": \"NOASSERTION\",\n");

    /* Лицензия обязательна, поэтому выводим без проверки */
    escaped = json_escape(license);
    printf("      \"licenseConcluded\": \"%s\",\n", escaped ? escaped : "");
    printf("      \"licenseDeclared\": \"%s\",\n", escaped ? escaped : "");
    free(escaped);

    if (copyright) {
        escaped = json_escape(copyright);
        printf("      \"copyrightText\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
    } else
        printf("      \"copyrightText\": \"NOASSERTION\",\n");
    if (purpose) {
        escaped = json_escape(purpose);
        printf("      \"primaryPackagePurpose\": \"%s\",\n", escaped ? escaped : "");
        free(escaped);
    } else
        printf("      \"primaryPackagePurpose\": \"NOASSERTION\",\n");
}

/* Вывод одного файла */
static void print_file_object(const FileInfo *info, int is_first) {
    char *escaped;

    if (!is_first) printf(",\n");
    printf("    {\n");
    escaped = json_escape(info->name);
    printf("      \"fileName\": \"%s\",\n", escaped ? escaped : "");
    free(escaped);
    escaped = json_escape(info->name);
    printf("      \"SPDXID\": \"SPDXRef-File-%s\",\n", escaped ? escaped : "");
    free(escaped);
    printf("      \"fileTypes\": [\"%s\"],\n", info->file_type);
    if (info->sha1[0])
        printf("      \"checksums\": [{\"algorithm\": \"SHA1\", \"checksumValue\": \"%s\"}],\n", info->sha1);
    else
        printf("      \"checksums\": [{\"algorithm\": \"SHA1\", \"checksumValue\": \"\"}],\n");
    escaped = json_escape(info->license);
    printf("      \"licenseConcluded\": \"%s\",\n", escaped ? escaped : "");
    printf("      \"licenseInfoInFiles\": [\"%s\"]", escaped ? escaped : "");
    free(escaped);
    if (info->copyright[0]) {
        escaped = json_escape(info->copyright);
        printf(",\n      \"copyrightText\": \"%s\"", escaped ? escaped : "");
        free(escaped);
    } else {
        printf(",\n      \"copyrightText\": \"NOASSERTION\"");
    }
    printf("\n    }");
}

/* Сравнение для сортировки */
static int compare_file_entries(const void *a, const void *b) {
    const FileInfo *fa = (const FileInfo *)a;
    const FileInfo *fb = (const FileInfo *)b;
    return strcmp(fa->name, fb->name);
}

/* Вычисление verification code */
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

    qsort(entries, list->count, sizeof(FileInfo), compare_file_entries);

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

/* Основная функция. Параметр use_objects указывает, что нужно использовать только
   source_files (полученные из OBJ) и не сканировать каталог. */
void generate_spdx_json(const char *dir, ReuseConfig *config, const char *name,
                        char **source_files, int source_files_count,
                        int use_objects) {
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    char date[32];
    int binary_mode;
    char file_base[256];
    char pkg_spdx_id[256];
    char *escaped_name;
    const char *pkg_license;
    const char *pkg_copyright;
    FileList file_list;
    char *verification_code;
    char ns[256];
    char safe[256];
    char *sha1;
    int i;

    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);

    binary_mode = (package_purpose && strcmp(package_purpose, "SOURCE") != 0);

    /* Базовое имя из --file */
    strncpy(file_base, get_file_name(binary_file), sizeof(file_base)-1);
    file_base[sizeof(file_base)-1] = '\0';
    remove_extension(file_base);

    make_package_spdx_id(file_base, binary_mode ? NULL : "Source",
                         pkg_spdx_id, sizeof(pkg_spdx_id));

    escaped_name = json_escape(name);
    if (!escaped_name) exit(1);

    pkg_license = default_license;
    if (!pkg_license && config) pkg_license = config->default_license;

    /* Лицензия пакета обязательна */
    if (!pkg_license || pkg_license[0] == '\0') {
        fprintf(stderr, "Error: No license specified for package '%s'. Use --default-license or REUSE.toml default.\n", name);
        exit(EXIT_FAILURE);
    }

    pkg_copyright = default_copyright;
    if (!pkg_copyright && config) pkg_copyright = config->default_copyright;

    /* Сбор файлов */
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
        if (pkg_license) strncpy(info.license, pkg_license, sizeof(info.license)-1);
        else strcpy(info.license, "NOASSERTION"); /* не выполнится */
        if (pkg_copyright) strncpy(info.copyright, pkg_copyright, sizeof(info.copyright)-1);
        else strcpy(info.copyright, "NOASSERTION");
        filelist_add(&file_list, &info);
    } else {
        if (use_objects) {
            /* Используем только список файлов из OBJ */
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
            /* Обычное сканирование каталога */
            scan_source_files(dir, config, &file_list);
        }
    }

    verification_code = compute_verification_code(&file_list);

    /* Вывод JSON */
    printf("{\n");
    printf("  \"spdxVersion\": \"SPDX-2.3\",\n");
    printf("  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n");
    printf("  \"name\": \"%s SBOM\",\n", escaped_name);
    printf("  \"creationInfo\": {\n");
    printf("    \"created\": \"%s\",\n", date);
    printf("    \"creators\": [");
    if (creator) {
        char *esc_creator = json_escape(creator);
        printf("\"%s\"", esc_creator);
        free(esc_creator);
    } else {
        printf("\"Tool: osFree SPDX SBOM Generator\"");
    }
    printf("]\n");
    printf("  },\n");
    printf("  \"dataLicense\": \"CC0-1.0\",\n");

    sanitize_id(name, safe, sizeof(safe));
    sprintf(ns, "https://osfree.org/spdxdocs/%s-%ld", safe, (long)now);
    printf("  \"documentNamespace\": \"%s\",\n", ns);

    /* Пакет */
    printf("  \"packages\": [\n");
    print_package_fields(pkg_spdx_id, name, package_version, package_supplier,
                         pkg_license, pkg_copyright, package_purpose);
    if (verification_code) {
        printf("      \"filesAnalyzed\": true,\n");
        printf("      \"packageVerificationCode\": {\n");
        printf("        \"packageVerificationCodeValue\": \"%s\",\n", verification_code);
        printf("        \"packageVerificationCodeExcludedFiles\": []\n");
        printf("      }\n");
    } else {
        printf("      \"filesAnalyzed\": false\n");
    }
    printf("    }\n");
    printf("  ],\n");

    /* Файлы */
    printf("  \"files\": [\n");
    for (i = 0; i < file_list.count; i++) {
        print_file_object(&file_list.items[i], i == 0);
    }
    printf("\n  ],\n");

    /* Отношения */
    printf("  \"relationships\": [\n");
    printf("    {\n");
    printf("      \"spdxElementId\": \"SPDXRef-DOCUMENT\",\n");
    printf("      \"relatedSpdxElement\": \"%s\",\n", pkg_spdx_id);
    printf("      \"relationshipType\": \"DESCRIBES\"\n");
    printf("    }");
    if (binary_mode) {
        char src_id[256];
        make_package_spdx_id(file_base, "Source", src_id, sizeof(src_id));
        printf(",\n    {\n");
        printf("      \"spdxElementId\": \"%s\",\n", pkg_spdx_id);
        printf("      \"relatedSpdxElement\": \"%s\",\n", src_id);
        printf("      \"relationshipType\": \"GENERATED_FROM\"\n");
        printf("    }");
    }
    printf("\n  ]\n");
    printf("}\n");

    free(escaped_name);
    if (verification_code) free(verification_code);
    filelist_free(&file_list);
}

int main(int argc, char *argv[]) {
    const char *dir = ".";
    const char *output = NULL;
    int i;
    ReuseConfig *config = NULL;
    char *arg;
    char toml_path_buf[1024];
    char **source_files = NULL;
    int source_files_count = 0;

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

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--output=", 9) == 0)
            output = argv[i] + 9;
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
            /* Разбиваем строку по пробелам */
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
            /* obj_list не освобождаем, т.к. object_files содержит указатели на его части */
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

    /* Если заданы объектные файлы, извлекаем исходники из них */
    if (object_count > 0) {
        if (collect_sources_from_objects(&source_files, &source_files_count) != 0) {
            /* Возможно, не удалось извлечь ни одного файла, но это не ошибка,
               просто будет пустой список. Сканирование каталога не выполняется. */
        }
    }

    /* Перенаправление stdout в файл, если указан --output */
    if (output) {
        if (!freopen(output, "w", stdout)) {
            fprintf(stderr, "Cannot open output file: %s\n", output);
            exit(EXIT_FAILURE);
        }
    }

    /* Генерация JSON */
    if (object_count > 0) {
        generate_spdx_json(dir, config, doc_name, source_files, source_files_count, 1);
    } else {
        generate_spdx_json(dir, config, doc_name, NULL, 0, 0);
    }

    /* Освобождение памяти */
    if (source_files) {
        for (i = 0; i < source_files_count; i++) free(source_files[i]);
        free(source_files);
    }

    free_reuse_config(config);
    return 0;
}
