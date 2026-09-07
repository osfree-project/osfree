/* spdx-lint.c - проверка соответствия REUSE (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define MAX_LINE 4096

/* Глобальные переменные */
static char *default_license = NULL;
static char *default_copyright = NULL;
static char **exclude_list = NULL;
static int exclude_count = 0;

/* Проверка исключений */
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

/* Регистронезависимое сравнение строк */
static int str_ieq(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* Регистронезависимое сравнение префикса */
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

/* Проверка: файл лицензии (LICENSE, COPYING и т.п.) */
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

    for (i = 0; i < sizeof(exact) / sizeof(exact[0]); i++) {
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

/* Проверка: игнорируемый каталог */
static int is_ignored_dir(const char *name) {
    return strcmp(name, "LICENSES") == 0 || strcmp(name, ".reuse") == 0;
}

/* Проверка: sidecar-файл .license */
static int is_license_sidecar(const char *name) {
    size_t len = strlen(name);
    return len > 8 && strcmp(name + len - 8, ".license") == 0;
}

/* Проверка: файл REUSE.toml (не требует тегов) */
static int is_reuse_config_file(const char *name) {
    return strcmp(name, "REUSE.toml") == 0;
}

/* Формирование пути к sidecar */
static void get_sidecar_path(const char *main_path, char *buf, size_t bufsize) {
    snprintf(buf, bufsize, "%s.license", main_path);
}

int main(int argc, char *argv[]) {
    const char *dir = ".";
    int strict = 0;
    int errors = 0;
    int i;
    char *toml_path;
    ReuseConfig *config;
    char fullpath[1024];
    char sidecar_path[1024];
    const char *license;
    const char *copyright;
    char *tag_license = NULL;
    char *tag_copyright = NULL;
    char *sidecar_license = NULL;
    char *sidecar_copyright = NULL;
    char *arg;
    char dir_buf[1024];

#ifdef __LINUX__
    DIR *d;
    struct dirent *entry;
    struct stat st;
#else
    long hFile;
    struct _finddata_t fdata;
    struct stat st;
#endif

    default_license = NULL;
    default_copyright = NULL;
    exclude_list = NULL;
    exclude_count = 0;

    /* Разбор аргументов */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--strict") == 0) {
            strict = 1;
        } else if (strncmp(argv[i], "--default-license=", 18) == 0) {
            default_license = argv[i] + 18;
        } else if (strncmp(argv[i], "--default-copyright=", 20) == 0) {
            default_copyright = argv[i] + 20;
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

    /* Нормализация пути: убрать завершающий слеш/обратный слеш */
    strncpy(dir_buf, dir, sizeof(dir_buf) - 1);
    dir_buf[sizeof(dir_buf) - 1] = '\0';
    {
        size_t dirlen = strlen(dir_buf);
        if (dirlen > 0 && (dir_buf[dirlen-1] == '\\' || dir_buf[dirlen-1] == '/')) {
            dir_buf[dirlen-1] = '\0';
        }
    }
    dir = dir_buf;

    toml_path = find_reuse_toml_upwards(dir);
    config = NULL;
    if (toml_path) config = parse_reuse_toml(toml_path);

#ifdef __LINUX__
    d = opendir(dir);
    if (!d) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return 1;
    }

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        if (entry->d_type == DT_DIR && is_ignored_dir(entry->d_name)) continue;
        if (is_license_sidecar(entry->d_name)) continue;
        if (is_reuse_config_file(entry->d_name)) continue;   /* <-- добавлено */

        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, entry->d_name);
        if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) continue;

        if (is_license_file(entry->d_name)) continue;
        if (is_excluded(entry->d_name)) continue;

        get_sidecar_path(fullpath, sidecar_path, sizeof(sidecar_path));
        sidecar_license = NULL;
        sidecar_copyright = NULL;
        if (access(sidecar_path, F_OK) == 0) {
            sidecar_license = file_get_spdx_license(sidecar_path);
            sidecar_copyright = file_get_spdx_copyright(sidecar_path);
        }

        license = find_license_for_file(config, entry->d_name);
        if (!license && sidecar_license) license = sidecar_license;
        if (!license) {
            tag_license = file_get_spdx_license(fullpath);
            if (tag_license) license = tag_license;
        }
        if (!license && default_license) license = default_license;

        copyright = find_copyright_for_file(config, entry->d_name);
        if (!copyright && sidecar_copyright) copyright = sidecar_copyright;
        if (!copyright) {
            tag_copyright = file_get_spdx_copyright(fullpath);
            if (tag_copyright) copyright = tag_copyright;
        }
        if (!copyright && default_copyright) copyright = default_copyright;

        if (!license) {
            fprintf(stderr, "ERROR: %s has no SPDX-License-Identifier\n", fullpath);
            errors++;
        }
        if (!copyright) {
            fprintf(stderr, "ERROR: %s has no SPDX-FileCopyrightText\n", fullpath);
            errors++;
        }

        free(tag_license);
        free(tag_copyright);
        free(sidecar_license);
        free(sidecar_copyright);
    }
    closedir(d);
#else
    /* Windows: используем _findfirst/_findnext */
    snprintf(fullpath, sizeof(fullpath), "%s/*", dir);
    hFile = _findfirst(fullpath, &fdata);
    if (hFile == -1L) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return 1;
    }

    do {
        if (fdata.name[0] == '.') continue;

        snprintf(fullpath, sizeof(fullpath), "%s\\%s", dir, fdata.name);
        if (stat(fullpath, &st) == 0 && (st.st_mode & _S_IFDIR)) {
            if (is_ignored_dir(fdata.name)) continue;
            continue;
        }

        if (is_license_sidecar(fdata.name)) continue;
        if (is_reuse_config_file(fdata.name)) continue;   /* <-- добавлено */
        if (is_license_file(fdata.name)) continue;
        if (is_excluded(fdata.name)) continue;

        get_sidecar_path(fullpath, sidecar_path, sizeof(sidecar_path));
        sidecar_license = NULL;
        sidecar_copyright = NULL;
        if (_access(sidecar_path, 0) == 0) {
            sidecar_license = file_get_spdx_license(sidecar_path);
            sidecar_copyright = file_get_spdx_copyright(sidecar_path);
        }

        license = find_license_for_file(config, fdata.name);
        if (!license && sidecar_license) license = sidecar_license;
        if (!license) {
            tag_license = file_get_spdx_license(fullpath);
            if (tag_license) license = tag_license;
        }
        if (!license && default_license) license = default_license;

        copyright = find_copyright_for_file(config, fdata.name);
        if (!copyright && sidecar_copyright) copyright = sidecar_copyright;
        if (!copyright) {
            tag_copyright = file_get_spdx_copyright(fullpath);
            if (tag_copyright) copyright = tag_copyright;
        }
        if (!copyright && default_copyright) copyright = default_copyright;

        if (!license) {
            fprintf(stderr, "ERROR: %s has no SPDX-License-Identifier\n", fullpath);
            errors++;
        }
        if (!copyright) {
            fprintf(stderr, "ERROR: %s has no SPDX-FileCopyrightText\n", fullpath);
            errors++;
        }

        free(tag_license);
        free(tag_copyright);
        free(sidecar_license);
        free(sidecar_copyright);
    } while (_findnext(hFile, &fdata) == 0);
    _findclose(hFile);
#endif

    free_reuse_config(config);
    return errors ? 1 : 0;
}
