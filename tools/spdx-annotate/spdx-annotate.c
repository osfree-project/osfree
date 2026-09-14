/* spdx-annotate.c - аннотирование исходников SPDX-тегами (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef __LINUX__
#include <unistd.h>
#else
#include <direct.h>
#include <io.h>
#endif
#include <reuse_parser.h>
#include "spdx_db.h"
#include "spdx_utils.h"
#include "spdx_discover.h"
#include "git_utils.h"
#include "spdx_tag.h"
#include "dep5_parser.h"

#define MAX_LINE 4096
#define BINARY_PROBE 8192

typedef enum {
    STYLE_C,        /* C-стиль */
    STYLE_HASH,     /* # ... */
    STYLE_REM,      /* @echo off + rem ... */
    STYLE_SIDECAR,  /* <file>.license */
    STYLE_UNKNOWN
} CommentStyle;

typedef struct {
    char *ext;
    CommentStyle style;
} StyleOverride;

static StyleOverride *style_overrides = NULL;
static int style_overrides_count = 0;

/* ------------------------------------------------------------------ */
/* Утилиты                                                             */
/* ------------------------------------------------------------------ */

static int file_exists(const char *path) {
#ifdef __LINUX__
    return access(path, F_OK) == 0;
#else
    return _access(path, 0) == 0;
#endif
}

static int make_dir(const char *path) {
#ifdef __LINUX__
    if (mkdir(path, 0755) == 0) return 0;
    if (access(path, F_OK) == 0) return 0;
    return -1;
#else
    if (_mkdir(path) == 0) return 0;
    if (_access(path, 0) == 0) return 0;
    return -1;
#endif
}

static int write_file(const char *path, const char *text) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    if (text && fputs(text, f) == EOF) { fclose(f); return -1; }
    fclose(f);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Нормализация SPDX-выражения                                         */
/* ------------------------------------------------------------------ */

/* Возвращает каноническую форму выражения SPDX: каждый идентификатор
 * из SPDX License List / SPDX Exceptions заменяется на канонический
 * регистр (например, 'BSD-3-clause' -> 'BSD-3-Clause').
 * Операторы AND/OR/WITH, скобки и пробелы сохраняются как есть.
 * LicenseRef-* и DocumentRef-* остаются без изменений. */
static char *normalize_license_expression(const char *expr) {
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

/* ------------------------------------------------------------------ */
/* Определение бинарности                                              */
/* ------------------------------------------------------------------ */

static int is_binary_file(const char *path) {
    FILE *f = fopen(path, "rb");
    unsigned char buf[BINARY_PROBE];
    size_t n, i;

    if (!f) return 0;
    n = fread(buf, 1, sizeof(buf), f);
    fclose(f);

    for (i = 0; i < n; i++)
        if (buf[i] == 0)
            return 1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Определение стиля                                                   */
/* ------------------------------------------------------------------ */

static CommentStyle parse_style_name(const char *name) {
    if (strcmp(name, "c") == 0 || strcmp(name, "slash") == 0)
        return STYLE_C;
    if (strcmp(name, "hash") == 0)
        return STYLE_HASH;
    if (strcmp(name, "rem") == 0 || strcmp(name, "cmd") == 0)
        return STYLE_REM;
    if (strcmp(name, "binary") == 0 || strcmp(name, "sidecar") == 0)
        return STYLE_SIDECAR;
    return STYLE_UNKNOWN;
}

static void add_style_override(const char *arg) {
    const char *eq = strchr(arg, '=');
    char *ext;
    const char *name;
    CommentStyle style;
    size_t len;

    if (!eq) {
        printf("ERROR: invalid --comment-style: %s\n"
               "       Expected format: --comment-style=<ext-or-name>=<style>\n"
               "       Example: --comment-style=.rb=hash\n",
               arg);
        exit(EXIT_FAILURE);
    }
    len = (size_t)(eq - arg);
    ext = (char*)malloc(len + 1);
    if (!ext) { printf("ERROR: out of memory\n"); exit(EXIT_FAILURE); }
    memcpy(ext, arg, len);
    ext[len] = '\0';
    name = eq + 1;
    style = parse_style_name(name);
    if (style == STYLE_UNKNOWN) {
        printf("ERROR: unknown comment style: %s\n"
               "       Supported: slash (or c), hash, rem (or cmd), "
               "binary (or sidecar)\n", name);
        free(ext);
        exit(EXIT_FAILURE);
    }
    style_overrides = (StyleOverride*)realloc(style_overrides,
        (size_t)(style_overrides_count + 1) * sizeof(StyleOverride));
    if (!style_overrides) { printf("ERROR: out of memory\n"); exit(EXIT_FAILURE); }
    style_overrides[style_overrides_count].ext = ext;
    style_overrides[style_overrides_count].style = style;
    style_overrides_count++;
}

static CommentStyle detect_style(const char *filename) {
    const char *base = spdx_get_file_name(filename);
    const char *ext = strrchr(base, '.');
    static const char *c_ext[] = {
        ".c", ".cpp", ".h", ".hpp", ".cc", ".cxx",
        ".asm", ".rc", ".inc", NULL
    };
    static const char *hash_ext[] = {
        ".sh", ".py", ".pl", ".toml", ".yml", ".yaml",
        ".md", ".txt", ".ini", ".cfg", ".conf", ".mk", NULL
    };
    static const char *rem_ext[] = { ".cmd", ".bat", NULL };
    static const char *binary_ext[] = {
        ".exe", ".obj", ".lib", ".dll", ".res", ".ico",
        ".bmp", ".png", ".jpg", ".jpeg", ".gif", ".sys",
        ".com", ".o", ".a", ".so", ".zip", ".gz", ".tar",
        ".pdf", ".dat", ".bin", ".cur", ".fon", ".ttf",
        ".hlp", ".wnf", ".inf", ".chm", ".mo", ".qm", NULL
    };
    int i;

    for (i = 0; i < style_overrides_count; i++) {
        const char *pat = style_overrides[i].ext;
        if (pat[0] == '.') {
            if (ext && strcmp(pat, ext) == 0)
                return style_overrides[i].style;
        } else {
            if (strcmp(pat, base) == 0)
                return style_overrides[i].style;
        }
    }

    if (strcmp(base, "makefile") == 0 ||
        strcmp(base, "Makefile") == 0 ||
        strcmp(base, "GNUmakefile") == 0 ||
        strncmp(base, "README", 6) == 0 ||
        strncmp(base, "readme", 6) == 0 ||
        strcmp(base, "AUTHORS") == 0 ||
        strcmp(base, "NEWS") == 0 ||
        strcmp(base, "ChangeLog") == 0 ||
        strcmp(base, "CHANGELOG") == 0 ||
        strcmp(base, "TODO") == 0 ||
        strcmp(base, "INSTALL") == 0)
        return STYLE_HASH;

    if (ext) {
        for (i = 0; c_ext[i]; i++)
            if (strcmp(ext, c_ext[i]) == 0) return STYLE_C;
        for (i = 0; hash_ext[i]; i++)
            if (strcmp(ext, hash_ext[i]) == 0) return STYLE_HASH;
        for (i = 0; rem_ext[i]; i++)
            if (strcmp(ext, rem_ext[i]) == 0) return STYLE_REM;
        for (i = 0; binary_ext[i]; i++)
            if (strcmp(ext, binary_ext[i]) == 0) return STYLE_SIDECAR;
    }
    return STYLE_UNKNOWN;
}

static const char *style_name(CommentStyle s) {
    switch (s) {
    case STYLE_C:       return "C comment";
    case STYLE_HASH:    return "hash comment";
    case STYLE_REM:     return "REM comment";
    case STYLE_SIDECAR: return "sidecar";
    default:            return "unknown";
    }
}

/* ------------------------------------------------------------------ */
/* Формирование текста вставки                                         */
/* ------------------------------------------------------------------ */

static char *build_insertion(CommentStyle style,
                             const char *license,
                             const char *copyright) {
    size_t cap = 256;
    size_t len = 0;
    char *buf;

    if (license)   cap += strlen(license) * 2;
    if (copyright) cap += strlen(copyright) * 2;

    buf = (char*)malloc(cap);
    if (!buf) return NULL;
    buf[0] = '\0';

    switch (style) {
    case STYLE_C:
        len += (size_t)sprintf(buf + len, "/*\n");
        if (copyright)
            len += (size_t)sprintf(buf + len,
                    " * SPDX-FileCopyrightText: %s\n", copyright);
        if (license)
            len += (size_t)sprintf(buf + len,
                    " * SPDX-License-Identifier: %s\n", license);
        len += (size_t)sprintf(buf + len, " */\n\n");
        break;
    case STYLE_REM:
        len += (size_t)sprintf(buf + len, "@echo off\n");
        if (copyright)
            len += (size_t)sprintf(buf + len,
                    "rem SPDX-FileCopyrightText: %s\n", copyright);
        if (license)
            len += (size_t)sprintf(buf + len,
                    "rem SPDX-License-Identifier: %s\n", license);
        len += (size_t)sprintf(buf + len, "\n");
        break;
    case STYLE_SIDECAR:
        if (copyright)
            len += (size_t)sprintf(buf + len,
                    "SPDX-FileCopyrightText: %s\n", copyright);
        if (license)
            len += (size_t)sprintf(buf + len,
                    "SPDX-License-Identifier: %s\n", license);
        break;
    case STYLE_HASH:
    default:
        if (copyright)
            len += (size_t)sprintf(buf + len,
                    "# SPDX-FileCopyrightText: %s\n", copyright);
        if (license)
            len += (size_t)sprintf(buf + len,
                    "# SPDX-License-Identifier: %s\n", license);
        len += (size_t)sprintf(buf + len, "\n");
        break;
    }
    return buf;
}

static void print_block(const char *text, const char *indent) {
    const char *p = text;
    const char *line_start;

    while (*p) {
        line_start = p;
        while (*p && *p != '\n') p++;
        printf("%s", indent);
        fwrite(line_start, 1, (size_t)(p - line_start), stdout);
        printf("\n");
        if (*p == '\n') p++;
    }
}

/* ------------------------------------------------------------------ */
/* Основная операция аннотации                                         */
/* ------------------------------------------------------------------ */

static int annotate_one(const char *filename,
                        const char *license,
                        const char *copyright,
                        int force,
                        int dry_run) {
    CommentStyle style;
    CommentStyle declared;
    int is_bin;
    char *block;
    char sidecar[1200];
    FILE *f, *out;
    char line[MAX_LINE];
    int has;
    char tempname[1024];

    declared = detect_style(filename);

    if (declared == STYLE_UNKNOWN) {
        printf("ERROR: %s: unknown file type.\n"
               "       Fix one of:\n"
               "         - specify the comment style explicitly:\n"
               "           --comment-style=<ext-or-name>=<slash|hash|rem|binary>\n"
               "         - or remove the file from the project.\n",
               filename);
        return -1;
    }

    is_bin = is_binary_file(filename);

    if (declared == STYLE_SIDECAR && !is_bin) {
        printf("ERROR: %s: declared as binary but content is text.\n"
               "       Fix one of:\n"
               "         - specify the comment style explicitly:\n"
               "           --comment-style=<ext-or-name>=<slash|hash|rem>\n"
               "         - or fix the file if its content is wrong.\n",
               filename);
        return -1;
    }

    if (declared != STYLE_SIDECAR && is_bin) {
        printf("ERROR: %s: declared as text but content is binary.\n"
               "       Fix one of:\n"
               "         - specify the comment style explicitly:\n"
               "           --comment-style=<ext-or-name>=binary\n"
               "         - or fix the file if its content is wrong.\n",
               filename);
        return -1;
    }

    style = declared;
    block = build_insertion(style, license, copyright);
    if (!block) { printf("ERROR: out of memory\n"); return -1; }

    if (style == STYLE_SIDECAR) {
        int exists;
        int equal = 0;

        snprintf(sidecar, sizeof(sidecar), "%s.license", filename);
        exists = file_exists(sidecar);

        if (exists) {
            char *existing = spdx_read_file_all(sidecar, NULL);
            char *n1 = NULL, *n2 = NULL;
            if (existing) {
                n1 = spdx_normalize_text(existing);
                n2 = spdx_normalize_text(block);
                if (n1 && n2 && strcmp(n1, n2) == 0) equal = 1;
                free(n1); free(n2);
                free(existing);
            }
        }

        if (equal) {
            printf("Up to date (sidecar): %s\n", sidecar);
            free(block);
            return 0;
        }

        if (exists && !force) {
            printf("Outdated (sidecar):   %s\n", sidecar);
            printf("    reason:           sidecar content differs\n");
            printf("    action:           use --force to overwrite\n");
            free(block);
            return 0;
        }

        if (dry_run) {
            printf("%s %s\n",
                   exists ? "Would update (sidecar):"
                          : "Would create (sidecar):",
                   sidecar);
            printf("    reason:           binary file\n");
            printf("    tags:\n");
            print_block(block, "        ");
            free(block);
            return 0;
        }

        if (write_file(sidecar, block) != 0) {
            printf("ERROR: cannot write file: %s\n"
                   "       Check directory permissions.\n", sidecar);
            free(block);
            return -1;
        }
        printf("%s %s\n",
               exists ? "Updated (sidecar):   " : "Created (sidecar):   ",
               sidecar);
        free(block);
        return 0;
    }

    has = file_has_spdx_tag(filename);

    if (has && !force) {
        printf("Skipped (has tags):   %s\n", filename);
        free(block);
        return 0;
    }

    if (dry_run) {
        printf("%s %s\n",
               has ? "Would update tags:   " : "Would add tags:      ",
               filename);
        printf("    comment style:    %s\n", style_name(style));
        printf("    insertion at top of file:\n");
        print_block(block, "        ");
        free(block);
        return 0;
    }

    snprintf(tempname, sizeof(tempname), "%s.tmp", filename);
    out = fopen(tempname, "w");
    if (!out) {
        printf("ERROR: cannot open file for writing: %s\n"
               "       Check directory permissions.\n", tempname);
        free(block);
        return -1;
    }

    fputs(block, out);

    f = fopen(filename, "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) fputs(line, out);
        fclose(f);
    }
    fclose(out);

    remove(filename);
    if (rename(tempname, filename) != 0) {
        printf("ERROR: cannot rename %s to %s\n"
               "       Check file permissions.\n", tempname, filename);
        free(block);
        return -1;
    }
    printf("%s %s\n",
           has ? "Updated tags:        " : "Added tags:          ",
           filename);
    free(block);
    return 0;
}

/* ------------------------------------------------------------------ */
/* LICENSES/                                                           */
/* ------------------------------------------------------------------ */

static void build_licenses_path(char *dst, size_t dst_size,
                                const char *repo_root) {
#ifdef __LINUX__
    snprintf(dst, dst_size, "%s/LICENSES", repo_root);
#else
    snprintf(dst, dst_size, "%s\\LICENSES", repo_root);
#endif
}

static void build_license_file_path(char *dst, size_t dst_size,
                                    const char *lic_path, const char *id) {
#ifdef __LINUX__
    snprintf(dst, dst_size, "%s/%s.txt", lic_path, id);
#else
    snprintf(dst, dst_size, "%s\\%s.txt", lic_path, id);
#endif
}

static int ensure_licenses(const char *repo_root,
                           SpdxStrList *used_licenses,
                           int force,
                           int dry_run) {
    char lic_path[1024];
    char path[1200];
    int i;
    int errors = 0;
    int created_dir = 0;

    build_licenses_path(lic_path, sizeof(lic_path), repo_root);

    if (!file_exists(lic_path)) {
        if (dry_run) {
            printf("Would create dir:     %s\n", lic_path);
        } else {
            if (make_dir(lic_path) != 0) {
                printf("ERROR: cannot create directory: %s\n"
                       "       Check parent directory permissions.\n",
                       lic_path);
                return 1;
            }
            printf("Created dir:          %s\n", lic_path);
        }
        created_dir = 1;
    }

    if (used_licenses->count == 0) {
        if (created_dir && dry_run)
            printf("    (directory would be created empty)\n");
        return 0;
    }

    for (i = 0; i < used_licenses->count; i++) {
        const char *lic = used_licenses->items[i];
        const char *db_text;
        int exists;

        if (strncmp(lic, "LicenseRef-", 11) == 0 ||
            strncmp(lic, "DocumentRef-", 12) == 0) {
            printf("Manual (custom):      %s\n", lic);
            printf("    reason:           not in SPDX database\n");
            continue;
        }

        db_text = spdx_license_get_text(lic);
        if (!db_text) db_text = spdx_exception_get_text(lic);
        if (!db_text) {
            printf("ERROR: no text for %s in SPDX database.\n"
                   "       Expected at <spdx-db>/details/%s.json\n"
                   "       Check that the SPDX database is complete.\n",
                   lic, lic);
            errors++;
            continue;
        }

        build_license_file_path(path, sizeof(path), lic_path, lic);
        exists = file_exists(path);

        if (exists) {
            char *file_text = spdx_read_file_all(path, NULL);
            char *nf = NULL, *nd = NULL;
            int equal = 0;
            if (file_text) {
                nf = spdx_normalize_text(file_text);
                nd = spdx_normalize_text(db_text);
                if (nf && nd && strcmp(nf, nd) == 0) equal = 1;
                free(nf); free(nd); free(file_text);
            }
            if (equal) {
                printf("Up to date:           %s\n", path);
                continue;
            }
            if (!force) {
                printf("Outdated:             %s\n", path);
                printf("    reason:           text differs from SPDX database\n");
                printf("    action:           use --force to overwrite\n");
                continue;
            }
            if (dry_run) {
                printf("Would update:         %s\n", path);
                printf("    source:           SPDX database\n");
            } else {
                if (write_file(path, db_text) != 0) {
                    printf("ERROR: cannot write file: %s\n"
                           "       Check directory permissions.\n", path);
                    errors++;
                } else {
                    printf("Updated:              %s\n", path);
                }
            }
        } else {
            if (dry_run) {
                printf("Would create:         %s\n", path);
                printf("    source:           SPDX database\n");
            } else {
                if (write_file(path, db_text) != 0) {
                    printf("ERROR: cannot write file: %s\n"
                           "       Check directory permissions.\n", path);
                    errors++;
                } else {
                    printf("Created:              %s\n", path);
                }
            }
        }
    }

    return errors;
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[]) {
    const char *dir = ".";
    int dry_run = 1;
    int force = 0;
    int no_gitignore = 0;
    int i;
    const char *license_override = NULL;
    const char *copyright_override = NULL;
    const char *spdx_db_root = NULL;
    const char *cache_file = NULL;
    ReuseConfig **configs = NULL;
    int config_count = 0;
    SpdxStrList toml_paths;
    int db_errs;
    SpdxStrList used_licenses;
    SpdxStrList paths;
    SpdxWalkOptions walk_opts;
    char *repo_root = NULL;
    GitIgnoreList gitignore_rules;
    int has_gitignore = 0;
    int total_errors = 0;

    git_ignore_list_init(&gitignore_rules);

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: spdx-annotate [options] [<directory>]\n"
                   "  --write                    Apply changes (default is "
                   "dry-run)\n"
                   "  --dry-run                  Show what would be done, "
                   "do not write (default)\n"
                   "  --force                    Overwrite existing tags / "
                   "outdated sidecar / outdated license texts\n"
                   "  --license=<id>             Override license for all "
                   "files\n"
                   "  --copyright=<text>         Override copyright for all "
                   "files\n"
                   "  --spdx-db=<path>           SPDX database root "
                   "(required)\n"
                   "  --cache=<path>             SPDX database cache file\n"
                   "  --comment-style=<ext>=<style>\n"
                   "                             Set comment style for a "
                   "given extension\n"
                   "                             or base name. Styles: "
                   "slash, hash, rem, binary\n"
                   "  --no-gitignore             Do not apply .gitignore "
                   "rules\n"
                   "  --help, -h                 Show this help\n");
            git_ignore_list_free(&gitignore_rules);
            return 0;
        }
        if (strcmp(argv[i], "--write") == 0) {
            dry_run = 0;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = 1;
        } else if (strcmp(argv[i], "--force") == 0) {
            force = 1;
        } else if (strcmp(argv[i], "--no-gitignore") == 0) {
            no_gitignore = 1;
        } else if (strncmp(argv[i], "--license=", 10) == 0) {
            license_override = argv[i] + 10;
        } else if (strncmp(argv[i], "--copyright=", 12) == 0) {
            copyright_override = argv[i] + 12;
        } else if (strncmp(argv[i], "--spdx-db=", 10) == 0) {
            spdx_db_root = argv[i] + 10;
        } else if (strncmp(argv[i], "--cache=", 8) == 0) {
            cache_file = argv[i] + 8;
        } else if (strncmp(argv[i], "--comment-style=", 16) == 0) {
            add_style_override(argv[i] + 16);
        } else if (argv[i][0] != '-') {
            dir = argv[i];
        } else {
            printf("ERROR: unknown option: %s\n"
                   "       Run 'spdx-annotate --help' for usage.\n",
                   argv[i]);
            git_ignore_list_free(&gitignore_rules);
            return 1;
        }
    }

    if (!spdx_db_root) {
        printf("ERROR: SPDX database is not configured.\n"
               "       --spdx-db=<path> is required.\n"
               "       Run 'spdx-annotate --help' for usage.\n");
        git_ignore_list_free(&gitignore_rules);
        return 1;
    }

    db_errs = spdx_db_init(spdx_db_root, cache_file);
    if (db_errs & SPDX_DB_ERR_LICENSES) {
        printf("ERROR: SPDX license database is unavailable "
               "(licenses.json not loaded).\n"
               "       Expected at <spdx-db>/licenses.json.\n"
               "       Cannot validate SPDX identifiers. Aborting.\n");
        spdx_db_free();
        git_ignore_list_free(&gitignore_rules);
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_EXCEPTIONS) {
        printf("ERROR: SPDX exceptions database is unavailable "
               "(exceptions.json not loaded).\n"
               "       Expected at <spdx-db>/exceptions.json.\n"
               "       Cannot validate SPDX identifiers. Aborting.\n");
        spdx_db_free();
        git_ignore_list_free(&gitignore_rules);
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_CACHE)
        printf("WARNING: cache could not be written.\n"
               "         Next run will re-parse JSON indexes.\n");

    spdx_strlist_init(&used_licenses);

    repo_root = git_find_repo_root(dir);

    reuse_find_all_tomls(repo_root, dir, &toml_paths);
    configs = reuse_parse_all(&toml_paths, &config_count);
    spdx_strlist_free(&toml_paths);

    if (repo_root) {
        ReuseConfig *dep5 = reuse_load_dep5(repo_root);
        if (dep5) {
            ReuseConfig **na = (ReuseConfig**)realloc(configs,
                (size_t)(config_count + 1) * sizeof(ReuseConfig*));
            if (na) {
                configs = na;
                configs[config_count++] = dep5;
            } else {
                free_reuse_config(dep5);
            }
        }
    }

    if (!no_gitignore) {
        if (git_collect_gitignores(repo_root, dir, &gitignore_rules) == 0 &&
            gitignore_rules.count > 0) {
            has_gitignore = 1;
        }
    }

    spdx_walk_options_default(&walk_opts);
    walk_opts.recursive             = 0;
    walk_opts.skip_hidden           = 1;
    walk_opts.skip_vcs_dirs         = 1;
    walk_opts.skip_licenses_dir     = 1;
    walk_opts.skip_reuse_dir        = 1;
    walk_opts.skip_license_sidecars = 1;
    walk_opts.skip_reuse_toml       = 1;
    walk_opts.skip_license_files    = 1;
    if (has_gitignore) {
        walk_opts.use_gitignore   = 1;
        walk_opts.repo_root       = repo_root ? repo_root : dir;
        walk_opts.gitignore_rules = &gitignore_rules;
    }

    spdx_strlist_init(&paths);
    if (spdx_walk_tree(dir, &walk_opts, &paths) != 0) {
        printf("ERROR: cannot walk tree: %s\n"
               "       Check that the directory exists and is readable.\n",
               dir);
        spdx_strlist_free(&paths);
        spdx_strlist_free(&used_licenses);
        reuse_free_all(configs, config_count);
        git_ignore_list_free(&gitignore_rules);
        free(repo_root);
        spdx_db_free();
        return 1;
    }

    if (dry_run) {
        printf("Mode: dry-run (use --write to apply changes)\n");
    } else {
        printf("Mode: write\n");
    }

    printf("\n=== Tags ===\n");

    for (i = 0; i < paths.count; i++) {
        const char *fullpath = paths.items[i];
        const char *license;
        const char *copyright;
        char *reuse_license = NULL;
        char *reuse_copyright = NULL;
        char *normalized = NULL;
        int precedence = 0;
        int has_reuse = 0;
        int rc;

        reuse_resolve_for_file(configs, config_count, fullpath,
                               NULL, NULL,
                               &reuse_license, &reuse_copyright,
                               &precedence, &has_reuse);

        license = reuse_license ? reuse_license : license_override;
        copyright = reuse_copyright ? reuse_copyright : copyright_override;

        if (license) {
            normalized = normalize_license_expression(license);
            if (normalized) license = normalized;
        }

        if (!license || !copyright) {
            char mf_path[1100];
            size_t dlen = strlen(dir);

#ifdef __LINUX__
            snprintf(mf_path, sizeof(mf_path), "%s%smakefile", dir,
                     (dlen > 0 && dir[dlen-1] == '/') ? "" : "/");
#else
            snprintf(mf_path, sizeof(mf_path), "%s%smakefile", dir,
                     (dlen > 0 && (dir[dlen-1] == '\\' || dir[dlen-1] == '/'))
                     ? "" : "\\");
#endif

            printf("ERROR: %s: annotate requires both license and copyright.\n",
                   fullpath);
            printf("       Missing: %s%s%s\n",
                   !license ? "license" : "",
                   (!license && !copyright) ? " and " : "",
                   !copyright ? "copyright" : "");
            printf("       Fix one of:\n");
            printf("         - REUSE.toml anywhere from repo root to %s\n",
                   dir);
            printf("         - LICENSE and COPYRIGHT variables in %s\n",
                   mf_path);
            printf("         - --license=... --copyright=... on the command line\n");
            printf("       Example:\n");
            printf("         LICENSE = BSD-3-Clause\n");
            printf("         COPYRIGHT = Copyright (C) 2025 osFree Project\n");
            free(normalized);
            free(reuse_license);
            free(reuse_copyright);
            total_errors++;
            continue;
        }

        {
            SpdxStrList ids;
            int k;
            spdx_strlist_init(&ids);
            spdx_expression_collect_ids(license, &ids);
            for (k = 0; k < ids.count; k++) {
                const SpdxLicenseEntry *e = spdx_license_lookup(ids.items[k]);
                const SpdxExceptionEntry *ex = NULL;
                if (!e) ex = spdx_exception_lookup(ids.items[k]);
                if (e) spdx_strlist_add_unique(&used_licenses, e->id);
                else if (ex) spdx_strlist_add_unique(&used_licenses, ex->id);
                else spdx_strlist_add_unique(&used_licenses, ids.items[k]);
            }
            spdx_strlist_free(&ids);
        }

        rc = annotate_one(fullpath, license, copyright, force, dry_run);
        if (rc != 0) total_errors++;

        free(normalized);
        free(reuse_license);
        free(reuse_copyright);
    }

    spdx_strlist_free(&paths);

    printf("\n=== LICENSES/ ===\n");
    total_errors += ensure_licenses(repo_root ? repo_root : dir,
                                    &used_licenses, force, dry_run);

    spdx_strlist_free(&used_licenses);
    reuse_free_all(configs, config_count);
    git_ignore_list_free(&gitignore_rules);
    free(repo_root);
    spdx_db_free();
    {
        int k;
        for (k = 0; k < style_overrides_count; k++)
            free(style_overrides[k].ext);
        free(style_overrides);
    }

    return total_errors > 0 ? 1 : 0;
}
