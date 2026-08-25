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

CommentStyle detect_style(const char *filename) {
    const char *ext;
    ext = strrchr(filename, '.');
    if (!ext) return STYLE_HASH;
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 ||
        strcmp(ext, ".h") == 0 || strcmp(ext, ".asm") == 0)
        return STYLE_C;
    return STYLE_HASH;
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

    dir = ".";
    dry_run = 0;
    force = 0;
    license_override = NULL;
    copyright_override = NULL;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = 1;
        } else if (strcmp(argv[i], "--force") == 0) {
            force = 1;
        } else if (strncmp(argv[i], "--license=", 10) == 0) {
            license_override = argv[i] + 10;
        } else if (strncmp(argv[i], "--copyright=", 13) == 0) {
            copyright_override = argv[i] + 13;
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

        license = license_override ? license_override :
                  find_license_for_file(config, entry->d_name);
        copyright = copyright_override ? copyright_override :
                    find_copyright_for_file(config, entry->d_name);
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
