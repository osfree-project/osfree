/* spdx-annotate.c - автоматическое добавление SPDX-тегов (C89) */

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

typedef enum { STYLE_C, STYLE_HASH } CommentStyle;

/* НОВОЕ: глобальные значения по умолчанию и список исключений */
static char *default_license = NULL;
static char *default_copyright = NULL;
static char **exclude_list = NULL;
static int exclude_count = 0;

CommentStyle detect_style(const char *filename) {
    const char *ext;
    ext = strrchr(filename, '.');
    if (!ext) return STYLE_HASH;
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 ||
        strcmp(ext, ".h") == 0 || strcmp(ext, ".asm") == 0)
        return STYLE_C;
    return STYLE_HASH;
}

/* НОВОЕ: функция проверки исключения по базовому имени */
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

void add_spdx_tags(const char *filename, const char *license, const char *copyright, int force) {
    FILE *f, *out;
    char line[MAX_LINE];
    int has;
    char tempname[1024];
    CommentStyle style;

    f = fopen(filename, "r");
    if (!f) return;
    has = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "SPDX-License-Identifier")) { has = 1; break; }
    }
    fclose(f);
    if (has && !force) return;

    snprintf(tempname, sizeof(tempname), "%s.tmp", filename);
    out = fopen(tempname, "w");
    if (!out) return;

    style = detect_style(filename);
    if (style == STYLE_C) {
        fprintf(out, "/*\n");
        fprintf(out, " * SPDX-FileCopyrightText: %s\n", copyright ? copyright : "osFree Project");
        fprintf(out, " * SPDX-License-Identifier: %s\n", license ? license : "BSD-3-Clause");
        fprintf(out, " */\n\n");
    } else {
        fprintf(out, "# SPDX-FileCopyrightText: %s\n", copyright ? copyright : "osFree Project");
        fprintf(out, "# SPDX-License-Identifier: %s\n", license ? license : "BSD-3-Clause");
        fprintf(out, "\n");
    }

    f = fopen(filename, "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) fputs(line, out);
        fclose(f);
    }
    fclose(out);
    remove(filename);
    rename(tempname, filename);
}

int main(int argc, char *argv[]) {
    const char *dir;
    int dry_run, force, i;
    const char *license_override, *copyright_override;
    char *toml_path;
    ReuseConfig *config;
    DIR *d;
    struct dirent *entry;
    const char *ext;
    const char *license, *copyright;
    char fullpath[1024];
    char *arg; /* НОВОЕ */

    dir = ".";
    dry_run = 0;
    force = 0;
    license_override = NULL;
    copyright_override = NULL;
    /* НОВОЕ: обнуляем глобальные переменные */
    default_license = NULL;
    default_copyright = NULL;
    exclude_list = NULL;
    exclude_count = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = 1;
        } else if (strcmp(argv[i], "--force") == 0) {
            force = 1;
        } else if (strncmp(argv[i], "--license=", 10) == 0) {
            license_override = argv[i] + 10;
        } else if (strncmp(argv[i], "--copyright=", 13) == 0) {
            copyright_override = argv[i] + 13;
        } else if (strncmp(argv[i], "--default-license=", 19) == 0) {
            /* НОВОЕ: установка лицензии по умолчанию */
            default_license = argv[i] + 19;
        } else if (strncmp(argv[i], "--default-copyright=", 21) == 0) {
            /* НОВОЕ: установка копирайта по умолчанию */
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
            strcmp(ext, ".h") && strcmp(ext, ".cmd") && strcmp(ext, ".sh"))
            continue;

        /* НОВОЕ: проверка исключения */
        if (is_excluded(entry->d_name)) continue;

        license = license_override ? license_override :
                  find_license_for_file(config, entry->d_name);
        /* НОВОЕ: если не найдено, используем default_license */
        if (!license && default_license) license = default_license;

        copyright = copyright_override ? copyright_override :
                    find_copyright_for_file(config, entry->d_name);
        /* НОВОЕ: если не найдено, используем default_copyright */
        if (!copyright && default_copyright) copyright = default_copyright;

        if (!license && !copyright) continue;

        if (dry_run) {
            printf("Would add tags to: %s (license: %s)\n",
                   entry->d_name, license ? license : "(none)");
        } else {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, entry->d_name);
            add_spdx_tags(fullpath, license, copyright, force);
        }
    }

    closedir(d);
    free_reuse_config(config);
    return 0;
}
