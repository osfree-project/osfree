/* spdx_db.h - библиотека SPDX license list (C89, OpenWatcom)
 *
 * Дисковый кеш хранит и индекс, и детали (полный текст, шаблон, HTML).
 * Индекс читается при spdx_db_init (быстро, ~сотни КБ).
 * Детали читаются по требованию через fseek (не парсятся заново).
 */
#ifndef SPDX_DB_H
#define SPDX_DB_H

#ifdef __cplusplus
extern "C" {
#endif

#define SPDX_DB_FLAG_OSI        0x01
#define SPDX_DB_FLAG_FSF_LIBRE  0x02
#define SPDX_DB_FLAG_DEPRECATED 0x04

#define SPDX_DB_ERR_LICENSES    0x01
#define SPDX_DB_ERR_EXCEPTIONS  0x02
#define SPDX_DB_ERR_CACHE       0x04

#define SPDX_EXPR_OK             0
#define SPDX_EXPR_SYNTAX_ERROR   1
#define SPDX_EXPR_UNKNOWN_TOKEN  2

typedef struct {
    char *id;
    unsigned char flags;
    char *name;
    char **see_also;             /* NULL-терминированный */

    unsigned long detail_offset; /* 0 - деталей нет */
    unsigned long detail_size;

    int  detail_loaded;
    char *text;
    char *template;
    char *text_html;
} SpdxLicenseEntry;

typedef struct {
    char *id;
    unsigned char flags;
    char *name;
    char **see_also;

    unsigned long detail_offset;
    unsigned long detail_size;

    int  detail_loaded;
    char *text;
    char *template;
    char *text_html;
} SpdxExceptionEntry;

/* Инициализация.
 *   licenses_json   - путь к licenses.json (индекс). NULL - пропустить.
 *   exceptions_json - путь к exceptions.json. NULL - пропустить.
 *   details_dir     - каталог с <ID>.json (детали лицензий). NULL - без деталей.
 *   exceptions_dir  - каталог с <ID>.json (детали исключений). NULL - без деталей.
 *   cache_file      - путь к кеш-файлу. NULL - без кеша.
 *
 * Возвращает 0 при полном успехе либо битовую маску SPDX_DB_ERR_*.
 */
int spdx_db_init(const char *licenses_json,
                 const char *exceptions_json,
                 const char *details_dir,
                 const char *exceptions_dir,
                 const char *cache_file);

void spdx_db_free(void);

const SpdxLicenseEntry   *spdx_license_lookup(const char *id);
const SpdxExceptionEntry *spdx_exception_lookup(const char *id);

/* Ленивая загрузка деталей. 0 - успех, -1 - ошибка. */
int spdx_license_load_detail(const char *id);
int spdx_exception_load_detail(const char *id);

int spdx_license_is_valid(const char *id);
int spdx_exception_is_valid(const char *id);
int spdx_license_is_deprecated(const char *id);
int spdx_exception_is_deprecated(const char *id);
int spdx_license_is_osi_approved(const char *id);
int spdx_license_is_fsf_libre(const char *id);

/* Возвращает SPDX_EXPR_*.
 * При UNKNOWN_TOKEN, если bad_token != NULL, туда пишется указатель
 * на начало проблемного токена внутри строки expr. Длина токена
 * определяется до ближайшего пробела, '(' или ')'. */
int spdx_expression_validate(const char *expr, const char **bad_token);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_DB_H */
