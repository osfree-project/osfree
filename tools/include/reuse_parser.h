/* reuse_parser.h - публичный заголовок для парсера REUSE.toml */

#ifndef REUSE_PARSER_H
#define REUSE_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char **paths;
    int path_count;
    char *license;
    char *copyright;
    char *precedence;
} Annotation;

typedef struct {
    char *default_license;
    char *default_copyright;
    Annotation *annotations;
    int annotation_count;
    char *source_dir;
} ReuseConfig;

ReuseConfig* parse_reuse_toml(const char *filename);
void free_reuse_config(ReuseConfig *config);
char* find_reuse_toml_upwards(const char *start_dir);
const char* find_license_for_file(ReuseConfig *config, const char *filename);
const char* find_copyright_for_file(ReuseConfig *config, const char *filename);
int matches_pattern(const char *pattern, const char *filename);

#endif
