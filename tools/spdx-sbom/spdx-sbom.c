/* spdx-sbom.c - генератор SPDX SBOM (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __LINUX__
#include <dirent.h>
#else
#include <direct.h>
#endif
#include <time.h>
#include <reuse_parser.h>

/* НОВОЕ: глобальные значения по умолчанию */
static char *default_license = NULL;
static char *default_copyright = NULL;

const char* get_file_type(const char *filename) {
    const char *ext;
    ext = strrchr(filename, '.');
    if (!ext) return "OTHER";
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0) return "SOURCE";
    if (strcmp(ext, ".h") == 0) return "HEADER";
    if (strcmp(ext, ".asm") == 0) return "ASSEMBLY";
    if (strcmp(ext, ".rc") == 0) return "RESOURCE";
    if (strcmp(ext, ".ico") == 0 || strcmp(ext, ".bmp") == 0 ||
        strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0) return "BINARY";
    return "OTHER";
}

void generate_spdx_json(const char *dir, ReuseConfig *config) {
    time_t now;
    struct tm *tm;
    char date[32];
    DIR *d;
    struct dirent *entry;
    const char *license, *copyright;
    int first;

    now = time(NULL);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);

    printf("{\n");
    printf("  \"spdxVersion\": \"SPDX-2.3\",\n");
    printf("  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n");
    printf("  \"name\": \"%s\",\n", dir);
    printf("  \"creationInfo\": {\n");
    printf("    \"created\": \"%s\",\n", date);
    printf("    \"creators\": [\"Tool: osFree SPDX SBOM Generator\"]\n");
    printf("  },\n");
    printf("  \"dataLicense\": \"CC0-1.0\",\n");
    printf("  \"documentNamespace\": \"https://spdx.org/spdxdocs/osfree/%s\",\n", dir);
    printf("  \"files\": [\n");

    d = opendir(dir);
    if (!d) return;

    first = 1;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        license = find_license_for_file(config, entry->d_name);
        /* НОВОЕ: если не нашли в REUSE.toml, используем default_license */
        if (!license && default_license) license = default_license;
        if (!license) continue;

        copyright = find_copyright_for_file(config, entry->d_name);
        /* НОВОЕ: если не нашли в REUSE.toml, используем default_copyright */
        if (!copyright && default_copyright) copyright = default_copyright;

        if (!first) printf(",\n");
        first = 0;
        printf("    {\n");
        printf("      \"fileName\": \"%s\",\n", entry->d_name);
        printf("      \"SPDXID\": \"SPDXRef-File-%s\",\n", entry->d_name);
        printf("      \"fileType\": \"%s\",\n", get_file_type(entry->d_name));
        printf("      \"licenseConcluded\": \"%s\"\n", license);
        if (copyright) printf("      \"copyrightText\": \"%s\"\n", copyright);
        printf("    }");
    }

    closedir(d);
    printf("\n  ]\n}\n");
}

int main(int argc, char *argv[]) {
    const char *dir;
    const char *output;
    int i;
    char *toml_path;
    ReuseConfig *config;

    dir = ".";
    output = NULL;
    /* НОВОЕ: обнуляем глобальные значения */
    default_license = NULL;
    default_copyright = NULL;

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--output=", 9) == 0) {
            output = argv[i] + 9;
        } else if (strncmp(argv[i], "--default-license=", 19) == 0) {
            /* НОВОЕ: установка лицензии по умолчанию */
            default_license = argv[i] + 19;
        } else if (strncmp(argv[i], "--default-copyright=", 21) == 0) {
            /* НОВОЕ: установка копирайта по умолчанию */
            default_copyright = argv[i] + 21;
        } else {
            dir = argv[i];
        }
    }

    toml_path = find_reuse_toml_upwards(dir);
    config = NULL;
    if (toml_path) config = parse_reuse_toml(toml_path);

    if (output) freopen(output, "w", stdout);
    generate_spdx_json(dir, config);

    free_reuse_config(config);
    return 0;
}
