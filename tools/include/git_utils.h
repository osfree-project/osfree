/* git_utils.h - работа с репозиторием Git без вызова внешних утилит (C89) */
#ifndef GIT_UTILS_H
#define GIT_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Одно правило из .gitignore */
typedef struct {
    char *pattern;     /* шаблон без '!', без начального '/', без хвостового '/' */
    char *base_rel;    /* путь от корня репозитория до каталога, где лежит правило; "" для корня */
    int   negate;      /* 1, если правило начинается с '!' */
    int   anchored;    /* 1, если правило начинается с '/' */
    int   dir_only;    /* 1, если правило заканчивается на '/' */
} GitIgnoreRule;

typedef struct {
    GitIgnoreRule *items;
    int count;
    int capacity;
} GitIgnoreList;

void git_ignore_list_init(GitIgnoreList *list);
void git_ignore_list_free(GitIgnoreList *list);

/* Ищет корень Git-репозитория, поднимаясь от start_dir вверх.
 * Ищет каталог или файл .git.
 * Возвращает malloc-строку (caller free) или NULL, если репозиторий не найден. */
char *git_find_repo_root(const char *start_dir);

/* 1, если dir находится под Git-репозиторием. */
int git_is_repo(const char *dir);

/* Собирает .gitignore из всех родительских каталогов от repo_root до
 * target_dir включительно. Правила добавляются в порядке от корня к
 * целевому каталогу.
 * Если repo_root == NULL, читается только <target_dir>/.gitignore.
 * Возвращает 0 при успехе (даже если файлов нет), -1 при ошибке. */
int git_collect_gitignores(const char *repo_root,
                           const char *target_dir,
                           GitIgnoreList *out);

/* Проверяет, игнорируется ли файл или каталог по правилам.
 * rel_path — путь относительно корня репозитория (или target_dir, если
 * repo_root был NULL при сборе).
 * is_dir — 1 для каталогов, 0 для файлов.
 * Возвращает 1, если путь игнорируется, иначе 0. */
int git_is_ignored(const GitIgnoreList *rules,
                   const char *rel_path,
                   int is_dir);

#ifdef __cplusplus
}
#endif

#endif /* GIT_UTILS_H */
