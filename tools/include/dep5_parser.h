/* dep5_parser.h - парсер .reuse/dep5 (DEP5, deprecated) для REUSE (C89) */
#ifndef DEP5_PARSER_H
#define DEP5_PARSER_H

#include "reuse.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Парсит файл .reuse/dep5.
 * source_dir - каталог, относительно которого сопоставляются шаблоны
 *              Files: (обычно корень проекта).
 * Возвращает ReuseConfig* (caller освобождает через free_reuse_config)
 * или NULL при ошибке/отсутствии файла.
 * depth устанавливается в -1, что даёт dep5 более низкий приоритет,
 * чем любой REUSE.toml. */
ReuseConfig *dep5_parse(const char *filename, const char *source_dir);

/* Проверяет наличие <repo_root>/.reuse/dep5 и парсит его.
 * NULL, если файла нет или repo_root == NULL. */
ReuseConfig *reuse_load_dep5(const char *repo_root);

#ifdef __cplusplus
}
#endif

#endif /* DEP5_PARSER_H */
