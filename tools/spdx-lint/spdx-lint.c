/* spdx-lint.c - проверка наличия SPDX-тегов (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __LINUX__
#include <dirent.h>
#else
#include <direct.h>
#endif
#include <reuse_parser.h>

#define MAX_LINE 4096

/* НОВОЕ: глобальные переменные для значений по умолчанию и списка исключений */
static char *default_license = NULL;
static char *default_copyright = NULL;
static char **exclude_list = NULL;
static int exclude_count = 0;

int file_has_spdx_tag(const char *filename) {
    FILE *f;
    char line[MAX_LINE];
    int found;

    f = fopen(filename, "r");
    if (!f) return 0;
    found = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "SPDX-License-Identifier: ")) { found = 1; break; }
    }
    fclose(f);
    return found;
}

/* НОВОЕ: функция проверки, исключён ли файл по базовому имени */
int is_excluded(const char *filename) {
    int i;
    const char *base;
    base = strrchr(filename, '/');
    if (base) base++; else base = filename;
    for (i = 0; i < exclude_count; i++) {
        if (strcmp(exclude_list[i], base) == 0) return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char *dir;
    int strict, errors, i;
    char *toml_path;
    ReuseConfig *config;
    DIR *d;
    struct dirent *entry;
    const char *ext;
    char fullpath[1024];
    const char *license;
    char *arg;

    dir = ".";
    strict = 0;
    errors = 0;

    /* НОВОЕ: обнуляем глобальные переменные */
    default_license = NULL;
    default_copyright = NULL;
    exclude_list = NULL;
    exclude_count = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--strict") == 0) {
            strict = 1;
        } else if (strncmp(argv[i], "--default-license=", 19) == 0) {
            /* НОВОЕ: установка лицензии по умолчанию */
            default_license = argv[i] + 19;
        } else if (strncmp(argv[i], "--default-copyright=", 21) == 0) {
            /* НОВОЕ: установка копирайта по умолчанию (пока не используется, но для совместимости) */
            default_copyright = argv[i] + 21;
        } else if (strncmp(argv[i], "--exclude=", 10) == 0) {
            /* НОВОЕ: разбор списка исключаемых файлов (через пробел) */
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

    d = opendir(dir);
    if (!d) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return 1;
    }

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        ext = strrchr(entry->d_name, '.');
        if (!ext) continue;
        if (strcmp(ext, ".c") && strcmp(ext, ".cpp") && strcmp(ext, ".asm") &&
            strcmp(ext, ".h") && strcmp(ext, ".rc") /*&& strcmp(ext, ".cmd") && strcmp(ext, ".sh")*/)
            continue;

        /* НОВОЕ: проверка исключения */
        if (is_excluded(entry->d_name)) continue;

        snprintf(fullpath, sizeof(fullpath), "%s%s", dir, entry->d_name);

        if (file_has_spdx_tag(fullpath)) continue;

        /* НОВОЕ: сначала ищем в REUSE.toml, затем используем default_license */
        license = find_license_for_file(config, entry->d_name);
        if (!license && default_license) license = default_license;

        if (license) continue;

        fprintf(stderr, "ERROR: %s has no SPDX-License-Identifier\n", fullpath);
        errors++;
    }

    closedir(d);
    free_reuse_config(config);
    return errors ? 1 : 0;
}
