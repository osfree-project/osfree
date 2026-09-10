/* json_parser.h - минимальный JSON-парсер для C89 */
#ifndef JSON_PARSER_H
#define JSON_PARSER_H

typedef enum {
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct JsonNode {
    JsonType type;
    char *key;              /* имя поля (для членов объекта) */
    char *string_value;     /* значение строки */
    double number_value;    /* значение числа */
    int bool_value;         /* значение boolean */
    struct JsonNode **children;  /* массив дочерних узлов */
    int child_count;
    int child_capacity;
    struct JsonNode *parent;
} JsonNode;

/* Разбирает JSON-текст, возвращает корневой узел (или NULL при ошибке).
   Вызывающий код должен освободить память через json_free(). */
JsonNode *json_parse(const char *text);

/* Ищет дочерний узел объекта по ключу (только для OBJECT).
   Возвращает NULL, если не найдено. */
JsonNode *json_find_child(JsonNode *object, const char *key);

/* Ищет узел по пути, разделённому '/', например "packages/0/name".
   Возвращает NULL, если путь не найден. */
JsonNode *json_find_path(JsonNode *root, const char *path);

/* Возвращает значение строкового узла (только для JSON_STRING). */
const char *json_get_string(JsonNode *node);

/* Освобождает дерево, включая само содержимое JSON. */
void json_free(JsonNode *node);

#endif /* JSON_PARSER_H */
