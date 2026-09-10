/* res_parser.h - извлечение списка исходных файлов из .res (C89) */
#ifndef RES_PARSER_H
#define RES_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Извлекает список исходных файлов из .res-файла, собранного OpenWatcom
 * или Borland. Используется ресурс типа 0x79 с именем "EBWF_XFMMTUPPE",
 * содержащий массив записей DepInfo.
 *
 * @param res_path  путь к .res файлу
 * @param sources   [out] массив строк (каждая выделена через malloc)
 * @param count     [out] количество найденных имён
 * @return 0 при успехе, -1 при ошибке.
 */
int res_extract_sources(const char *res_path, char ***sources, int *count);

#ifdef __cplusplus
}
#endif

#endif /* RES_PARSER_H */
