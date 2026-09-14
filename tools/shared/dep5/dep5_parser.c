/* dep5_parser.c - парсер .reuse/dep5 (DEP5, deprecated) для REUSE (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dep5_parser.h"

#define MAX_LINE 4096

static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

static char *trim_inplace(char *s) {
    char *end;
    while (*s == ' ' || *s == '\t' ||
           *s == '\r' || *s == '\n') s++;
    if (*s == '\0') return s;
    end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' ||
                       *end == '\r' || *end == '\n'))
        end--;
    end[1] = '\0';
    return s;
}

static void add_path_token(Annotation *ann, const char *token, size_t len) {
    char **na;
    char *copy;
    if (len == 0) return;
    copy = (char*)malloc(len + 1);
    if (!copy) return;
    memcpy(copy, token, len);
    copy[len] = '\0';
    na = (char**)realloc(ann->paths,
        (size_t)(ann->path_count + 1) * sizeof(char*));
    if (!na) { free(copy); return; }
    ann->paths = na;
    ann->paths[ann->path_count++] = copy;
}

static void handle_files(ReuseConfig *cfg, int *cap,
                         Annotation **p_ann,
                         const char *value, int *p_order) {
    Annotation *ann;
    const char *p = value;

    if (cfg->annotation_count >= *cap) {
        int new_cap = (*cap) * 2;
        Annotation *na = (Annotation*)realloc(cfg->annotations,
            (size_t)new_cap * sizeof(Annotation));
        if (!na) return;
        cfg->annotations = na;
        *cap = new_cap;
    }

    ann = &cfg->annotations[cfg->annotation_count];
    memset(ann, 0, sizeof(Annotation));
    ann->precedence = REUSE_PRECEDENCE_CLOSEST;
    ann->order_in_file = (*p_order)++;

    while (*p) {
        const char *start;
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        start = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        add_path_token(ann, start, (size_t)(p - start));
    }

    cfg->annotation_count++;
    *p_ann = ann;
}

static void handle_copyright(Annotation *ann, const char *value) {
    if (!ann) return;
    free(ann->copyright);
    ann->copyright = dup_str(value);
}

static void handle_license(Annotation *ann, const char *value) {
    /* Debian's License field may contain the identifier followed by
     * indented license text. Take the first whitespace-separated token. */
    char buf[256];
    size_t i = 0;
    const char *p = value;

    if (!ann) return;
    while (*p == ' ' || *p == '\t') p++;
    while (p[i] && !isspace((unsigned char)p[i]) && i < sizeof(buf) - 1) {
        buf[i] = p[i];
        i++;
    }
    buf[i] = '\0';
    free(ann->license);
    ann->license = dup_str(buf);
}

static void flush_field(ReuseConfig *cfg, int *cap,
                        Annotation **p_ann,
                        const char *field, const char *value,
                        int *p_order) {
    if (strcmp(field, "Files") == 0) {
        handle_files(cfg, cap, p_ann, value, p_order);
    } else if (strcmp(field, "Copyright") == 0) {
        handle_copyright(*p_ann, value);
    } else if (strcmp(field, "License") == 0) {
        handle_license(*p_ann, value);
    }
    /* Format, Upstream-Name, Comment и прочие поля игнорируются */
}

ReuseConfig *dep5_parse(const char *filename, const char *source_dir) {
    FILE *f;
    ReuseConfig *cfg;
    char line[MAX_LINE];
    char field[64];
    char value[MAX_LINE];
    int in_field = 0;
    int cap = 8;
    int order = 0;
    Annotation *current_ann = NULL;

    f = fopen(filename, "r");
    if (!f) return NULL;

    cfg = (ReuseConfig*)calloc(1, sizeof(ReuseConfig));
    if (!cfg) { fclose(f); return NULL; }

    cfg->annotations = (Annotation*)malloc((size_t)cap * sizeof(Annotation));
    if (!cfg->annotations) { free(cfg); fclose(f); return NULL; }
    cfg->annotation_count = 0;
    cfg->source_dir = dup_str(source_dir);
    cfg->depth = -1;
    cfg->version = 1;

    field[0] = '\0';
    value[0] = '\0';

    while (fgets(line, sizeof(line), f)) {
        /* Continuation line — append to current value. */
        if (line[0] == ' ' || line[0] == '\t') {
            if (in_field) {
                char *v = trim_inplace(line);
                if (*v) {
                    size_t vl = strlen(value);
                    size_t add = strlen(v);
                    if (vl + 1 + add < sizeof(value)) {
                        value[vl] = ' ';
                        memcpy(value + vl + 1, v, add + 1);
                    }
                }
            }
            continue;
        }

        /* Non-continuation line: flush previous field, then parse. */
        if (in_field) {
            flush_field(cfg, &cap, &current_ann, field, value, &order);
            in_field = 0;
        }

        {
            char *s = trim_inplace(line);
            char *colon;
            if (*s == '\0') continue;

            colon = strchr(s, ':');
            if (!colon) continue;
            *colon = '\0';

            {
                char *fn = trim_inplace(s);
                if (strlen(fn) >= sizeof(field)) continue;
                strcpy(field, fn);
            }
            {
                char *fv = trim_inplace(colon + 1);
                if (strlen(fv) >= sizeof(value)) continue;
                strcpy(value, fv);
            }
            in_field = 1;
        }
    }

    if (in_field) {
        flush_field(cfg, &cap, &current_ann, field, value, &order);
    }

    fclose(f);
    return cfg;
}

ReuseConfig *reuse_load_dep5(const char *repo_root) {
    char path[1024];
    FILE *probe;

    if (!repo_root || !repo_root[0]) return NULL;

#ifdef __LINUX__
    snprintf(path, sizeof(path), "%s/.reuse/dep5", repo_root);
#else
    snprintf(path, sizeof(path), "%s\\.reuse\\dep5", repo_root);
#endif

    probe = fopen(path, "r");
    if (!probe) return NULL;
    fclose(probe);

    return dep5_parse(path, repo_root);
}
