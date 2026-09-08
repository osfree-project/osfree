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
    char *key;              /* имя ключа (для членов объекта) */
    char *string_value;     /* значение строки */
    double number_value;    /* числовое значение */
    int bool_value;         /* булево значение */
    struct JsonNode **children;  /* массив дочерних узлов */
    int child_count;
    int child_capacity;
    struct JsonNode *parent;
} JsonNode;

/* Разбирает JSON-строку, возвращает корневой узел (или NULL при ошибке).
   Вызывающий код должен освободить результат через json_free(). */
JsonNode *json_parse(const char *text);

/* Ищет первый дочерний узел с заданным ключом (только для OBJECT).
   Возвращает NULL, если не найден. */
JsonNode *json_find_child(JsonNode *object, const char *key);

/* Ищет узел по пути, состоящему из ключей, разделённых '/'.
   Например, "packages/0/name" вернёт имя первого пакета.
   Возвращает NULL, если путь не существует. */
JsonNode *json_find_path(JsonNode *root, const char *path);

/* Возвращает строковое значение узла (только для JSON_STRING).
   Если узел не строка, возвращает NULL. */
const char *json_get_string(JsonNode *node);

/* Освобождает память, занятую деревом JSON. */
void json_free(JsonNode *node);

#endif
