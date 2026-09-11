/* git_utils.c - работа с репозиторием Git без вызова внешних утилит (C89)
 *
 * Поддерживает синтаксис .gitignore:
 *   '*'  - любые символы кроме '/'
 *   '**' - любые символы включая '/'
 *   '?'  - один символ кроме '/'
 *   '/' в начале правила - привязка к каталогу правила
 *   '/' в конце правила  - только каталоги
 *   '!' в начале правила  - отрицание
 *   '#' в начале строки   - комментарий
 *   '\#' и '\!'           - экранированные # и !
 *
 * Порядок правил: сверху вниз, последнее совпавшее побеждает (last-match-wins).
 * Регистр: на Linux — чувствительный, на Windows — нет.
 */

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

#include "git_utils.h"

/* ------------------------------------------------------------------ */
/* Служебное                                                           */
/* ------------------------------------------------------------------ */

static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

static int has_git_entry(const char *path) {
    char p[1024];
    struct stat st;
    snprintf(p, sizeof(p), "%s/.git", path);
    if (stat(p, &st) == 0) return 1;
    snprintf(p, sizeof(p), "%s\\.git", path);
    if (stat(p, &st) == 0) return 1;
    return 0;
}

/* Убирает хвостовые разделители (кроме "C:\" на Windows). */
static void trim_separators(char *s) {
    size_t len = strlen(s);
#ifdef _WIN32
    if (len == 3 && s[1] == ':' && (s[2] == '\\' || s[2] == '/')) return;
#endif
    while (len > 1 && (s[len-1] == '/' || s[len-1] == '\\')) {
        s[--len] = '\0';
    }
#ifdef _WIN32
    if (len == 2 && s[1] == ':') {
        s[2] = '\\';
        s[3] = '\0';
    }
#endif
}

/* ------------------------------------------------------------------ */
/* Поиск корня репозитория                                             */
/* ------------------------------------------------------------------ */

char *git_find_repo_root(const char *start_dir) {
    char *current = dup_str(start_dir);
    if (!current) return NULL;

    while (1) {
        char *slash;
        char *backslash;

        trim_separators(current);

        if (has_git_entry(current)) return current;

        slash = strrchr(current, '/');
        backslash = strrchr(current, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
        if (!slash) break;

#ifdef _WIN32
        if (slash == current + 2 && current[1] == ':') break;
#endif
        if (slash == current) break;   /* уже в корне ФС */

        *slash = '\0';
    }

    free(current);
    return NULL;
}

int git_is_repo(const char *dir) {
    char *root = git_find_repo_root(dir);
    if (root) {
        free(root);
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Список правил                                                       */
/* ------------------------------------------------------------------ */

void git_ignore_list_init(GitIgnoreList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void git_ignore_list_free(GitIgnoreList *list) {
    int i;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].pattern);
        free(list->items[i].base_rel);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static GitIgnoreRule *git_ignore_list_add(GitIgnoreList *list) {
    GitIgnoreRule *r;
    if (list->count >= list->capacity) {
        int new_cap = (list->capacity == 0) ? 16 : list->capacity * 2;
        GitIgnoreRule *ni = (GitIgnoreRule*)realloc(list->items,
                              (size_t)new_cap * sizeof(GitIgnoreRule));
        if (!ni) { fprintf(stderr, "OOM\n"); exit(EXIT_FAILURE); }
        list->items = ni;
        list->capacity = new_cap;
    }
    r = &list->items[list->count++];
    memset(r, 0, sizeof(*r));
    return r;
}

/* ------------------------------------------------------------------ */
/* Разбор одной строки .gitignore                                      */
/* ------------------------------------------------------------------ */

static void trim_inplace(char *s) {
    char *p = s;
    char *end;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' ||
                       end[-1] == '\r' || end[-1] == '\n')) {
        *--end = '\0';
    }
}

static void parse_rule(const char *line, const char *base_rel,
                       GitIgnoreList *out) {
    GitIgnoreRule *r;
    char buf[1024];
    char *p;
    size_t len;

    if (!line || !line[0]) return;

    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim_inplace(buf);
    if (buf[0] == '\0') return;

    p = buf;

    /* Экранированные \# и \! */
    if (p[0] == '\\' && (p[1] == '#' || p[1] == '!')) {
        p++;
    } else if (p[0] == '#') {
        return;   /* комментарий */
    }

    r = git_ignore_list_add(out);

    if (p[0] == '!') {
        r->negate = 1;
        p++;
    }
    if (p[0] == '/') {
        r->anchored = 1;
        p++;
    }

    len = strlen(p);
    if (len > 0 && p[len-1] == '/') {
        r->dir_only = 1;
        p[len-1] = '\0';
    }

    /* Хвостовые пробелы, экранированные backslash+space, не поддерживаются:
       trim уже убрал их. Это допустимое ограничение. */

    if (p[0] == '\0') {
        /* Правило пустое после обработки — откатываем */
        free(r->pattern);
        free(r->base_rel);
        out->count--;
        return;
    }

    r->pattern = dup_str(p);
    r->base_rel = dup_str(base_rel ? base_rel : "");
}

static int read_gitignore_file(const char *dir, const char *base_rel,
                               GitIgnoreList *out) {
    char path[1024];
    FILE *f;
    char line[1024];

    snprintf(path, sizeof(path), "%s/.gitignore", dir);
    f = fopen(path, "r");
    if (!f) {
        snprintf(path, sizeof(path), "%s\\.gitignore", dir);
        f = fopen(path, "r");
        if (!f) return 0;
    }
    while (fgets(line, sizeof(line), f)) {
        parse_rule(line, base_rel, out);
    }
    fclose(f);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Сбор из иерархии                                                    */
/* ------------------------------------------------------------------ */

/* Строит относительный путь от from до to (через '/'). */
static char *rel_path_from(const char *from, const char *to) {
    size_t flen = strlen(from);
    size_t tlen = strlen(to);
    if (flen > tlen) return NULL;
    if (strncmp(from, to, flen) != 0) return NULL;
    if (flen == tlen) return dup_str("");
    if (to[flen] != '/' && to[flen] != '\\') return NULL;
    return dup_str(to + flen + 1);
}

int git_collect_gitignores(const char *repo_root,
                           const char *target_dir,
                           GitIgnoreList *out) {
    git_ignore_list_init(out);

    if (!repo_root) {
        read_gitignore_file(target_dir, "", out);
        return 0;
    }

    read_gitignore_file(repo_root, "", out);

    {
        char *rel = rel_path_from(repo_root, target_dir);
        if (!rel) return 0;

        if (rel[0] != '\0') {
            char current[1024];
            char base_rel[1024];
            size_t ri = 0;
            size_t rlen = strlen(rel);

            strncpy(current, repo_root, sizeof(current) - 1);
            current[sizeof(current) - 1] = '\0';
            base_rel[0] = '\0';

            while (ri < rlen) {
                size_t start = ri;
                size_t seglen;
                while (ri < rlen && rel[ri] != '/' && rel[ri] != '\\') ri++;
                seglen = ri - start;
                if (seglen > 0) {
                    size_t clen = strlen(current);
                    size_t blen = strlen(base_rel);
                    if (clen + 1 + seglen + 1 > sizeof(current)) break;
                    if (blen + 1 + seglen + 1 > sizeof(base_rel)) break;

                    current[clen] = '/';
                    memcpy(current + clen + 1, rel + start, seglen);
                    current[clen + 1 + seglen] = '\0';

                    if (blen == 0) {
                        memcpy(base_rel, rel + start, seglen);
                        base_rel[seglen] = '\0';
                    } else {
                        base_rel[blen] = '/';
                        memcpy(base_rel + blen + 1, rel + start, seglen);
                        base_rel[blen + 1 + seglen] = '\0';
                    }

                    read_gitignore_file(current, base_rel, out);
                }
                if (ri < rlen) ri++;
            }
        }
        free(rel);
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Матчинг                                                             */
/* ------------------------------------------------------------------ */

static int to_lower(int c) {
#ifdef _WIN32
    return tolower((unsigned char)c);
#else
    return (unsigned char)c;
#endif
}

/* Матч внутри одного компонента: '*' и '?' не пересекают '/' (его в строке нет). */
static int match_component(const char *pat, const char *str) {
    if (*pat == '\0') return *str == '\0';
    if (*pat == '*') {
        while (1) {
            if (match_component(pat + 1, str)) return 1;
            if (*str == '\0') return 0;
            str++;
        }
    }
    if (*pat == '?') {
        if (*str == '\0') return 0;
        return match_component(pat + 1, str + 1);
    }
    if (to_lower(*pat) != to_lower(*str)) return 0;
    return match_component(pat + 1, str + 1);
}

/* Матч пути с поддержкой '/', '**' и одиночных '*'. */
static int match_path(const char *pat, const char *path) {
    if (pat[0] == '*' && pat[1] == '*') {
        if (pat[2] == '\0') return 1;
        if (pat[2] == '/') {
            if (match_path(pat + 3, path)) return 1;
            while (*path) {
                if (*path == '/') {
                    if (match_path(pat + 3, path + 1)) return 1;
                }
                path++;
            }
            return 0;
        }
    }
    if (*pat == '\0') return *path == '\0';
    if (*pat == '*') {
        while (1) {
            if (match_path(pat + 1, path)) return 1;
            if (*path == '\0' || *path == '/') return 0;
            path++;
        }
    }
    if (*pat == '?') {
        if (*path == '\0' || *path == '/') return 0;
        return match_path(pat + 1, path + 1);
    }
    if (to_lower(*pat) != to_lower(*path)) return 0;
    return match_path(pat + 1, path + 1);
}

/* Матчит pattern в любом каталоге пути. */
static int match_any_level(const char *pat, const char *path) {
    if (match_path(pat, path)) return 1;
    while (*path) {
        if (*path == '/') {
            path++;
            if (match_path(pat, path)) return 1;
        } else {
            path++;
        }
    }
    return 0;
}

/* Матчит, содержит ли путь компонент-каталог с именем, подходящим под pat.
 * include_last: если 0, последний компонент (имя файла) не проверяется. */
static int path_has_matching_dir(const char *pat, const char *path,
                                 int include_last) {
    while (*path) {
        const char *slash = strchr(path, '/');
        size_t len;
        char seg[512];
        if (!slash && !include_last) return 0;
        len = slash ? (size_t)(slash - path) : strlen(path);
        if (len >= sizeof(seg)) len = sizeof(seg) - 1;
        memcpy(seg, path, len);
        seg[len] = '\0';
        if (match_component(pat, seg)) return 1;
        if (!slash) break;
        path = slash + 1;
    }
    return 0;
}

int git_is_ignored(const GitIgnoreList *rules,
                   const char *rel_path,
                   int is_dir) {
    int ignored = 0;
    int i;

    if (!rules || rules->count == 0) return 0;
    if (!rel_path) return 0;

    for (i = 0; i < rules->count; i++) {
        const GitIgnoreRule *r = &rules->items[i];
        const char *sub = rel_path;
        int matched = 0;

        /* Обрезаем префикс, если правило лежит в подкаталоге */
        if (r->base_rel && r->base_rel[0]) {
            size_t blen = strlen(r->base_rel);
            if (strncmp(rel_path, r->base_rel, blen) != 0) continue;
            if (rel_path[blen] != '/' && rel_path[blen] != '\\') continue;
            sub = rel_path + blen + 1;
        }

        if (r->dir_only) {
            if (is_dir) {
                matched = r->anchored
                    ? match_path(r->pattern, sub)
                    : path_has_matching_dir(r->pattern, sub, 1);
            } else {
                matched = r->anchored
                    ? path_has_matching_dir(r->pattern, sub, 0)
                    : path_has_matching_dir(r->pattern, sub, 0);
            }
        } else {
            if (r->anchored) {
                matched = match_path(r->pattern, sub);
            } else if (strchr(r->pattern, '/') != NULL) {
                matched = match_path(r->pattern, sub) ||
                          match_any_level(r->pattern, sub);
            } else {
                matched = match_any_level(r->pattern, sub);
            }
        }

        if (matched) {
            ignored = r->negate ? 0 : 1;
        }
    }
    return ignored;
}
