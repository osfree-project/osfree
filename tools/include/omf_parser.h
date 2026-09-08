/* omf_parser.h - извлечение имён исходных файлов из OMF-объектов (C89) */

#ifndef OMF_PARSER_H
#define OMF_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Извлекает список исходных файлов из объектного файла OMF.
 * Функция возвращает динамический массив строк (каждая строка выделена через malloc).
 * Вызывающий код должен освободить память: для каждого элемента вызвать free(),
 * затем free() для самого массива.
 *
 * @param obj_path  путь к .obj файлу
 * @param sources   [out] указатель на массив строк (будет выделен)
 * @param count     [out] количество найденных имён
 * @return 0 при успехе, -1 при ошибке
 */
int omf_extract_sources(const char *obj_path, char ***sources, int *count);

#ifdef __cplusplus
}
#endif

#endif /* OMF_PARSER_H */
