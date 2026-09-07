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
#else
#include <direct.h>
#include <sys/stat.h>
#include <io.h>
#endif

#include <reuse_parser.h>
#include "spdx_tag.h"
#include "sha1_utils.h"

/* Глобальные значения по умолчанию */
static char *default_license = NULL;
static char *default_copyright = NULL;
static char *doc_name = NULL;
static char *package_version = NULL;
static char *package_supplier = NULL;
static char *creator = NULL;

/* Глобальные переменные для исключений */
static char **exclude_list = NULL;
static int exclude_count = 0;

/* Вспомогательные функции */
static const char *get_base_name(const char *path) {
    const char *slash = strrchr(path, '/');
    const char *backslash = strrchr(path, '\\');
    const char *base;

    if (backslash && (!slash || backslash > slash))
        base = backslash + 1;
    else if (slash)
        base = slash + 1;
    else
        base = path;

    if (base[0] == '\0')
        base = "SBOM";
    return base;
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
    static const char *exact[] = {
        "license", "licence", "copying", "unlicense", "copyright"
    };
    const char *base;
    const char *slash = strrchr(filename, '/');
    const char *backslash = strrchr(filename, '\\');
    size_t i;

    if (backslash && (!slash || backslash > slash))
        base = backslash + 1;
    else if (slash)
        base = slash + 1;
    else
        base = filename;

    for (i = 0; i < sizeof(exact)/sizeof(exact[0]); i++) {
        if (str_ieq(base, exact[i])) return 1;
    }

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

static void get_sidecar_path(const char *main_path, char *buf, size_t bufsize) {
    snprintf(buf, bufsize, "%s.license", main_path);
}

static int is_excluded(const char *filename) {
    int i;
    const char *base;
    const char *slash = strrchr(filename, '/');
    const char *backslash = strrchr(filename, '\\');
    if (backslash && (!slash || backslash > slash))
        base = backslash + 1;
    else if (slash)
        base = slash + 1;
    else
        base = filename;

    for (i = 0; i < exclude_count; i++) {
        if (strcmp(exclude_list[i], base) == 0) return 1;
    }
    return 0;
}

static char *json_escape(const char *src) {
    size_t len = strlen(src);
    size_t i, j;
    size_t extra = 0;
    char *dst;

    for (i = 0; i < len; i++) {
        switch (src[i]) {
            case '"':
            case '\\':
                extra += 1;
                break;
            case '\n':
            case '\r':
            case '\t':
                extra += 1;
                break;
            default:
                break;
        }
    }

    dst = (char *)malloc(len + extra + 1);
    if (!dst) return NULL;

    j = 0;
    for (i = 0; i < len; i++) {
        switch (src[i]) {
            case '"':
                dst[j++] = '\\';
                dst[j++] = '"';
                break;
            case '\\':
                dst[j++] = '\\';
                dst[j++] = '\\';
                break;
            case '\n':
                dst[j++] = '\\';
                dst[j++] = 'n';
                break;
            case '\r':
                dst[j++] = '\\';
                dst[j++] = 'r';
                break;
            case '\t':
                dst[j++] = '\\';
                dst[j++] = 't';
                break;
            default:
                dst[j++] = src[i];
                break;
        }
    }
    dst[j] = '\0';
    return dst;
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

typedef struct {
    char name[512];
    char sha1[41];
} FileHashEntry;

static int compare_file_entries(const void *a, const void *b) {
    const FileHashEntry *fa = (const FileHashEntry *)a;
    const FileHashEntry *fb = (const FileHashEntry *)b;
    return strcmp(fa->name, fb->name);
}

static char *compute_package_verification_code(FileHashEntry *entries, int count) {
    char *concatenated, *verification_code;
    size_t total_len = 0;
    int i;
    char combined_sha1[41];

    if (count == 0) return NULL;

    qsort(entries, count, sizeof(FileHashEntry), compare_file_entries);
    for (i = 0; i < count; i++) total_len += strlen(entries[i].sha1);

    concatenated = (char *)malloc(total_len + 1);
    if (!concatenated) return NULL;
    concatenated[0] = '\0';
    for (i = 0; i < count; i++) strcat(concatenated, entries[i].sha1);

    sha1_string(concatenated, combined_sha1);
    free(concatenated);

    verification_code = (char *)malloc(41);
    if (verification_code) memcpy(verification_code, combined_sha1, 41);
    return verification_code;
}

static int process_file(const char *fullpath, const char *filename,
                        ReuseConfig *config, int is_first) {
    char sidecar_path[1024];
    const char *license = NULL, *copyright = NULL;
    char *tag_license = NULL, *tag_copyright = NULL;
    char *sidecar_license = NULL, *sidecar_copyright = NULL;
    char *sha1 = NULL;
    char *escaped_fname = NULL, *escaped_license = NULL, *escaped_copyright = NULL;

    get_sidecar_path(fullpath, sidecar_path, sizeof(sidecar_path));
#ifdef __LINUX__
    if (access(sidecar_path, F_OK) == 0) {
        sidecar_license = file_get_spdx_license(sidecar_path);
        sidecar_copyright = file_get_spdx_copyright(sidecar_path);
    }
#else
    if (_access(sidecar_path, 0) == 0) {
        sidecar_license = file_get_spdx_license(sidecar_path);
        sidecar_copyright = file_get_spdx_copyright(sidecar_path);
    }
#endif

    license = find_license_for_file(config, filename);
    if (!license && sidecar_license) license = sidecar_license;
    if (!license) {
        tag_license = file_get_spdx_license(fullpath);
        if (tag_license) license = tag_license;
    }
    if (!license && default_license) license = default_license;
    if (!license) {
        free(tag_license);
        free(sidecar_license);
        free(sidecar_copyright);
        return 0;
    }

    copyright = find_copyright_for_file(config, filename);
    if (!copyright && sidecar_copyright) copyright = sidecar_copyright;
    if (!copyright) {
        tag_copyright = file_get_spdx_copyright(fullpath);
        if (tag_copyright) copyright = tag_copyright;
    }
    if (!copyright && default_copyright) copyright = default_copyright;

    sha1 = sha1_file(fullpath);
    escaped_fname = json_escape(filename);
    escaped_license = json_escape(license);
    if (copyright) escaped_copyright = json_escape(copyright);

    if (!is_first) printf(",\n");
    printf("    {\n");
    printf("      \"fileName\": \"%s\",\n", escaped_fname);
    printf("      \"SPDXID\": \"SPDXRef-File-%s\",\n", escaped_fname);
    printf("      \"fileTypes\": [\"%s\"],\n", get_file_type(filename));
    if (sha1) {
        printf("      \"checksums\": [{\"algorithm\": \"SHA1\", \"checksumValue\": \"%s\"}],\n", sha1);
    } else {
        printf("      \"checksums\": [{\"algorithm\": \"SHA1\", \"checksumValue\": \"\"}],\n");
    }
    printf("      \"licenseConcluded\": \"%s\",\n", escaped_license);
    printf("      \"licenseInfoInFiles\": [\"%s\"]", escaped_license);
    if (escaped_copyright) {
        printf(",\n      \"copyrightText\": \"%s\"", escaped_copyright);
    } else {
        printf(",\n      \"copyrightText\": \"NOASSERTION\"");
    }
    printf("\n    }");

    free(sha1);
    free(escaped_fname);
    free(escaped_license);
    if (escaped_copyright) free(escaped_copyright);
    free(tag_license);
    free(tag_copyright);
    free(sidecar_license);
    free(sidecar_copyright);

    return 1;
}

void generate_spdx_json(const char *dir, ReuseConfig *config, const char *name) {
    time_t now = time(NULL);
    struct tm *tm;
    char date[32];
    char doc_namespace[256];
    char fullpath[1024];
    const char *package_name;
    char *escaped_package_name, *escaped_doc_name;
    char doc_name_buf[512];
    int first;

    FileHashEntry *file_hashes = NULL;
    int file_hash_count = 0, file_hash_capacity = 0;
    char *verification_code = NULL;

#ifdef __LINUX__
    DIR *d;
    struct dirent *entry;
    struct stat st;
#else
    long hFile;
    struct _finddata_t fdata;
    struct stat st;
#endif

    now = time(NULL);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);

    if (name) package_name = name;
    else package_name = get_base_name(dir);

    sprintf(doc_name_buf, "%s SBOM", package_name);
    escaped_doc_name = json_escape(doc_name_buf);
    escaped_package_name = json_escape(package_name);

    {
        char safe_name[256];
        size_t idx;
        for (idx = 0; package_name[idx] != '\0' && idx < sizeof(safe_name)-1; idx++) {
            safe_name[idx] = (package_name[idx] == ' ') ? '-' : package_name[idx];
        }
        safe_name[idx] = '\0';
        sprintf(doc_namespace, "https://osfree.org/spdxdocs/%s-%ld", safe_name, (long)now);
    }

    /* Первый проход: сбор хэшей для verification code */
#ifdef __LINUX__
    d = opendir(dir);
    if (d) {
        while ((entry = readdir(d)) != NULL) {
            char sidecar_path[1024];
            const char *lic;
            char *tag_lic;
            char *sha1;

            if (entry->d_name[0] == '.') continue;
            if (entry->d_type == DT_DIR && is_ignored_dir(entry->d_name)) continue;
            if (is_license_sidecar(entry->d_name)) continue;
            sprintf(fullpath, "%s/%s", dir, entry->d_name);
            if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) continue;
            if (is_license_file(entry->d_name)) continue;
            if (is_excluded(entry->d_name)) continue;

            lic = find_license_for_file(config, entry->d_name);
            tag_lic = NULL;
            if (!lic) {
                get_sidecar_path(fullpath, sidecar_path, sizeof(sidecar_path));
                if (access(sidecar_path, F_OK) == 0) {
                    tag_lic = file_get_spdx_license(sidecar_path);
                    if (tag_lic) lic = tag_lic;
                }
            }
            if (!lic) {
                tag_lic = file_get_spdx_license(fullpath);
                if (tag_lic) lic = tag_lic;
            }
            if (!lic && default_license) lic = default_license;
            if (!lic) { free(tag_lic); continue; }

            sha1 = sha1_file(fullpath);
            if (sha1) {
                if (file_hash_count == file_hash_capacity) {
                    file_hash_capacity = file_hash_capacity ? file_hash_capacity * 2 : 16;
                    file_hashes = (FileHashEntry *)realloc(file_hashes, file_hash_capacity * sizeof(FileHashEntry));
                }
                strncpy(file_hashes[file_hash_count].name, entry->d_name, sizeof(file_hashes[file_hash_count].name) - 1);
                file_hashes[file_hash_count].name[sizeof(file_hashes[file_hash_count].name) - 1] = '\0';
                memcpy(file_hashes[file_hash_count].sha1, sha1, 40);
                file_hashes[file_hash_count].sha1[40] = '\0';
                file_hash_count++;
                free(sha1);
            }
            free(tag_lic);
        }
        closedir(d);
    }
#else
    sprintf(fullpath, "%s/*", dir);
    hFile = _findfirst(fullpath, &fdata);
    if (hFile != -1L) {
        do {
            char sidecar_path[1024];
            const char *lic;
            char *tag_lic;
            char *sha1;

            if (fdata.name[0] == '.') continue;
            if (is_license_sidecar(fdata.name)) continue;
            sprintf(fullpath, "%s/%s", dir, fdata.name);
            if (stat(fullpath, &st) == 0 && (st.st_mode & _S_IFDIR)) {
                if (is_ignored_dir(fdata.name)) continue;
                continue;
            }
            if (is_license_file(fdata.name)) continue;
            if (is_excluded(fdata.name)) continue;

            lic = find_license_for_file(config, fdata.name);
            tag_lic = NULL;
            if (!lic) {
                get_sidecar_path(fullpath, sidecar_path, sizeof(sidecar_path));
                if (_access(sidecar_path, 0) == 0) {
                    tag_lic = file_get_spdx_license(sidecar_path);
                    if (tag_lic) lic = tag_lic;
                }
            }
            if (!lic) {
                tag_lic = file_get_spdx_license(fullpath);
                if (tag_lic) lic = tag_lic;
            }
            if (!lic && default_license) lic = default_license;
            if (!lic) { free(tag_lic); continue; }

            sha1 = sha1_file(fullpath);
            if (sha1) {
                if (file_hash_count == file_hash_capacity) {
                    file_hash_capacity = file_hash_capacity ? file_hash_capacity * 2 : 16;
                    file_hashes = (FileHashEntry *)realloc(file_hashes, file_hash_capacity * sizeof(FileHashEntry));
                }
                strncpy(file_hashes[file_hash_count].name, fdata.name, sizeof(file_hashes[file_hash_count].name) - 1);
                file_hashes[file_hash_count].name[sizeof(file_hashes[file_hash_count].name) - 1] = '\0';
                memcpy(file_hashes[file_hash_count].sha1, sha1, 40);
                file_hashes[file_hash_count].sha1[40] = '\0';
                file_hash_count++;
                free(sha1);
            }
            free(tag_lic);
        } while (_findnext(hFile, &fdata) == 0);
        _findclose(hFile);
    }
#endif

    if (file_hash_count > 0) {
        verification_code = compute_package_verification_code(file_hashes, file_hash_count);
    }

    /* Вывод заголовка */
    printf("{\n");
    printf("  \"spdxVersion\": \"SPDX-2.3\",\n");
    printf("  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n");
    printf("  \"name\": \"%s\",\n", escaped_doc_name);
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
    printf("  \"documentNamespace\": \"%s\",\n", doc_namespace);
    printf("  \"packages\": [\n");

    printf("    {\n");
    printf("      \"SPDXID\": \"SPDXRef-Package\",\n");
    printf("      \"name\": \"%s\",\n", escaped_package_name);
    printf("      \"downloadLocation\": \"NOASSERTION\",\n");
    if (package_version) {
        char *esc_ver = json_escape(package_version);
        printf("      \"versionInfo\": \"%s\",\n", esc_ver);
        free(esc_ver);
    } else {
        printf("      \"versionInfo\": \"NOASSERTION\",\n");
    }
    if (package_supplier) {
        char *esc_sup = json_escape(package_supplier);
        printf("      \"supplier\": \"%s\",\n", esc_sup);
        free(esc_sup);
    } else {
        printf("      \"supplier\": \"NOASSERTION\",\n");
    }
    if (default_license) {
        char *esc_lic = json_escape(default_license);
        printf("      \"licenseConcluded\": \"%s\",\n", esc_lic);
        printf("      \"licenseDeclared\": \"%s\",\n", esc_lic);
        free(esc_lic);
    } else {
        printf("      \"licenseConcluded\": \"NOASSERTION\",\n");
        printf("      \"licenseDeclared\": \"NOASSERTION\",\n");
    }
    if (default_copyright) {
        char *esc_copy = json_escape(default_copyright);
        printf("      \"copyrightText\": \"%s\",\n", esc_copy);
        free(esc_copy);
    } else {
        printf("      \"copyrightText\": \"NOASSERTION\",\n");
    }
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
    printf("  \"files\": [\n");

    free(escaped_package_name);
    free(escaped_doc_name);
    if (verification_code) free(verification_code);
    free(file_hashes);

    /* Второй проход: вывод файлов */
    first = 1;
#ifdef __LINUX__
    d = opendir(dir);
    if (d) {
        while ((entry = readdir(d)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (entry->d_type == DT_DIR && is_ignored_dir(entry->d_name)) continue;
            if (is_license_sidecar(entry->d_name)) continue;
            sprintf(fullpath, "%s/%s", dir, entry->d_name);
            if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) continue;
            if (is_license_file(entry->d_name)) continue;
            if (is_excluded(entry->d_name)) continue;

            if (process_file(fullpath, entry->d_name, config, first)) {
                first = 0;
            }
        }
        closedir(d);
    }
#else
    sprintf(fullpath, "%s/*", dir);
    hFile = _findfirst(fullpath, &fdata);
    if (hFile != -1L) {
        do {
            if (fdata.name[0] == '.') continue;
            if (is_license_sidecar(fdata.name)) continue;
            sprintf(fullpath, "%s/%s", dir, fdata.name);
            if (stat(fullpath, &st) == 0 && (st.st_mode & _S_IFDIR)) {
                if (is_ignored_dir(fdata.name)) continue;
                continue;
            }
            if (is_license_file(fdata.name)) continue;
            if (is_excluded(fdata.name)) continue;

            if (process_file(fullpath, fdata.name, config, first)) {
                first = 0;
            }
        } while (_findnext(hFile, &fdata) == 0);
        _findclose(hFile);
    }
#endif

    printf("\n  ],\n");
    printf("  \"relationships\": [\n");
    printf("    {\n");
    printf("      \"spdxElementId\": \"SPDXRef-DOCUMENT\",\n");
    printf("      \"relatedSpdxElement\": \"SPDXRef-Package\",\n");
    printf("      \"relationshipType\": \"DESCRIBES\"\n");
    printf("    }\n");
    printf("  ]\n");
    printf("}\n");
}

int main(int argc, char *argv[]) {
    const char *dir = ".";
    const char *output = NULL;
    int i;
    char *toml_path;
    ReuseConfig *config;
    char *arg;

    default_license = NULL;
    default_copyright = NULL;
    doc_name = NULL;
    package_version = NULL;
    package_supplier = NULL;
    creator = NULL;
    exclude_list = NULL;
    exclude_count = 0;

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--output=", 9) == 0) {
            output = argv[i] + 9;
        } else if (strncmp(argv[i], "--default-license=", 18) == 0) {
            default_license = argv[i] + 18;
        } else if (strncmp(argv[i], "--default-copyright=", 20) == 0) {
            default_copyright = argv[i] + 20;
        } else if (strncmp(argv[i], "--name=", 7) == 0) {
            doc_name = argv[i] + 7;
        } else if (strncmp(argv[i], "--version=", 10) == 0) {
            package_version = argv[i] + 10;
        } else if (strncmp(argv[i], "--supplier=", 11) == 0) {
            package_supplier = argv[i] + 11;
        } else if (strncmp(argv[i], "--creator=", 10) == 0) {
            creator = argv[i] + 10;
        } else if (strncmp(argv[i], "--exclude=", 10) == 0) {
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

    toml_path = find_reuse_toml_upwards(dir);
    config = NULL;
    if (toml_path) config = parse_reuse_toml(toml_path);

    if (output) {
        if (!freopen(output, "w", stdout)) {
            fprintf(stderr, "Cannot open output file: %s\n", output);
            exit(EXIT_FAILURE);
        }
    }

    generate_spdx_json(dir, config, doc_name);

    free_reuse_config(config);
    return 0;
}
