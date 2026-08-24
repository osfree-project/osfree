/* spdx-lint.c - проверка наличия SPDX-тегов (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <reuse_parser.h>

#define MAX_LINE 4096

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

    dir = ".";
    strict = 0;
    errors = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--strict") == 0) {
            strict = 1;
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

        snprintf(fullpath, sizeof(fullpath), "%s%s", dir, entry->d_name);

        if (file_has_spdx_tag(fullpath)) continue;

        license = find_license_for_file(config, entry->d_name);
        if (license) continue;

        fprintf(stderr, "ERROR: %s has no SPDX-License-Identifier\n", fullpath);
        errors++;
    }

    closedir(d);
    free_reuse_config(config);
    return errors ? 1 : 0;
}
