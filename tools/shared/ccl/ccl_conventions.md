# CCL conventions

Инженерные контракты, на которые опираются модули CCL и потребители
(SOM-слой, legacy dlist/all_dlist). Дополнение к документу
`CCL: архитектура и план реализации` (v0.9.5). При расхождении
приоритет у документа, этот файл — рабочая шпаргалка для ежедневной
работы с кодом.

---

## 1. Аллокатор (SOM-style, 4 хука)

CCL использует четыре глобальных указателя на функции, точно как
`SOMMalloc` / `SOMCalloc` / `SOMRealloc` / `SOMFree` в SOMobjects.
Объявления — в `ccl_alloc.h` (публичный), дефолтные реализации — в
`ccl_alloc.c` (единственный translation unit нового кода, где
вызываются `malloc`/`calloc`/`realloc`/`free`).

```c
typedef PVOID (APIENTRY *CCL_MALLOC_FN) (size_t cbSize);
typedef PVOID (APIENTRY *CCL_CALLOC_FN) (size_t cbNum, size_t cbSize);
typedef PVOID (APIENTRY *CCL_REALLOC_FN)(PVOID pPtr, size_t cbSize);
typedef void  (APIENTRY *CCL_FREE_FN)   (PVOID pPtr);

extern CCL_MALLOC_FN  CclMalloc;
extern CCL_CALLOC_FN  CclCalloc;
extern CCL_REALLOC_FN CclRealloc;
extern CCL_FREE_FN    CclFree;
```

**Контракт:**

- **Замена** — прямым присваиванием, до создания первого контейнера:

  ```c
  CclMalloc  = som_malloc_adapter;
  CclCalloc  = som_calloc_adapter;
  CclRealloc = som_realloc_adapter;
  CclFree    = som_free_adapter;
  ```

  Сеттеров нет. Поведение после первой аллокации — undefined;
  SOM-init должен произойти до `LinkedNodeAlloc`, `VectorCreate`,
  `StrSetCreate`, или любой другой CCL-функции, которая может
  выделить память.

- Блок, выделенный одним хуком, освобождается тем же хуком. После
  замены прежние блоки освобождать нельзя.

- CCL однопоточный. Один глобальный набор указателей достаточен.
  OS/2 и host-32-bit — по одному heap на процесс.

**Почему `size_t` + `APIENTRY` + wrapper'ы:**

Сигнатура SOM-хуков в `somapi.h`:

```c
typedef somToken SOMLINK somTD_SOMMalloc(size_t nbytes);
typedef somToken SOMLINK somTD_SOMCalloc(size_t, size_t);
typedef somToken SOMLINK somTD_SOMRealloc(somToken, size_t);
typedef void     SOMLINK somTD_SOMFree(somToken);
```

`SOMLINK` — calling convention (`_System` на OS/2), `size_t` — тип
параметра. Прямое `SOMMalloc = malloc` **не работает**: `malloc`
объявлен с `__cdecl`, `SOMLINK` — `_System`. Поэтому SOM в
дефолтной инициализации использует `SOMLINK`-wrapper
(`somDefaultMalloc` в SOM-ядре), который внутри вызывает `malloc`.

CCL делает то же: дефолтные реализации в `ccl_alloc.c` —
`APIENTRY`-wrapper'ы (`ccl_default_malloc`, `ccl_default_calloc`,
`ccl_default_realloc`, `ccl_default_free`), каждый внутри вызывает
свой C runtime аналог. На OS/2 `APIENTRY = _System = SOMLINK`. На
host `APIENTRY` — пусто, wrapper — обычная функция, совместимая с
`malloc`.

**Правило для нового кода:** `malloc`/`calloc`/`realloc`/`free`
только в `ccl_alloc.c` и в legacy `dlist.c`/`all_dlist.c`. Везде
иначе — `(*CclMalloc)`, `(*CclCalloc)`, `(*CclRealloc)`,
`(*CclFree)`.

**Legacy-исключение (см. §10):** payload, handle и временные массивы
в `dlist.c`/`all_dlist.c` остаются на `malloc`/`free`/`calloc` по
legacy-контракту. Узлы `LINKED_NODE` во всех трёх модулях (dlist,
all_dlist, list) — через `LinkedNodeAlloc`/`LinkedNodeFree`, то есть
через `(*CclMalloc)`/`(*CclFree)`.

---

## 2. Выравнивание

### LINKED_NODE

`LINKED_NODE` (list, legacy dlist/all_dlist) — elemt-модель, inline
payload нет. Выравнивание к `elemt` не требуется: `elemt` — это
`PVOID`, указывающий на данные отдельно.

### TREENODE (Фаза 1, tree_private.h)

```c
typedef struct _TREENODE {
    struct _TREENODE *left;
    struct _TREENODE *right;
    struct _TREENODE *parent;
    ULONG             ulRed;
    union { double d; void *p; long l; char c[1]; } u;
} TREENODE;
```

Арифметика:

- 32-bit: `offsetof(u) = 16`, `16 % 8 = 0` — выровнено.
- 64-bit: `offsetof(u) = 32` (компилятор сам вставляет padding между
  `ulRed` и `u`), `32 % 8 = 0` — выровнено.

`ulPad` в структуре не нужен: на 32-bit он бы сломал выравнивание
(`offsetof` стал бы 20), на 64-bit — избыточен.

### HASHNODE (Фаза 1, hash_private.h)

```c
typedef struct _HASHNODE {
    struct _HASHNODE *next;
    union { double d; void *p; long l; char c[1]; } u;
} HASHNODE;
```

В узле `[key][pad][value]`:

- `keyOffset = 0`;
- `valueOffset = CCL_ALIGN_UP(ulKeySize, CCL_MAX_ALIGN)`;
- `CCL_MAX_ALIGN = sizeof(double) = 8`;
- `pad = valueOffset - ulKeySize`.

`GetKey` -> `[0, ulKeySize)`, `GetValue` -> `[valueOffset,
valueOffset + ulValueSize)`. Оба среза выровнены.

Замечание для Фазы 1: `offsetof(HASHNODE, u) = 4` на 32-bit, что не
кратно `CCL_MAX_ALIGN = 8`. Значит `hash_private.c` либо:
(а) вызовет `ccl_node_alloc(8, valueOffset + ulValueSize, 8)` и
получит `u` на offset 8 через `ulPad` в самом `HASHNODE`, либо
(б) примет `ulAlign = 4` (достаточно, если `key`/`value` — указатели
и `int`, но не `double`). Решение принимается в `hash_private.h`
Фазы 1.

### ccl_node_alloc (node_private.h)

Требования к `ccl_node_alloc(ulFixedPart, ulElemSize, ulAlign)`:

- `ulAlign` — степень двойки; если меньше `sizeof(PVOID)`,
  повышается до `sizeof(PVOID)`;
- `ulFixedPart % ulAlign == 0`;
- pNode выровнен на `max(ulAlign, sizeof(void*))` (через
  `(*CclMalloc)`);
- тогда `pNode + ulFixedPart` автоматически выровнен на `ulAlign`.

Нарушение требований — `NULL` (в debug-сборке — assert до возврата
`NULL`).

### Ограничение 32/64-bit

`ccl_node_alloc` использует `ULONG`-арифметику над указателями.
Текущие платформы — OS/2 32-bit и host 32-bit. Для 64-bit
понадобится ввести `CCL_PTR` (широкий pointer-sized integer) и
адаптировать выражение выравнивания. Сигнатура `ccl_node_alloc`
при этом не меняется.

---

## 3. Magic-валидация handle

Каждый control-блок начинается с 32-битного ASCII magic. Проверка
— `ccl_validate` из `ccl_base_private.h`, вызывается из
`static get_ctl` каждого модуля:

```c
static HVECTOR VectorValidate(HVECTOR hVector) {
    return (HVECTOR)ccl_validate(hVector, CCL_VECTOR_MAGIC);
}
```

`ccl_validate` возвращает `p`, если `p != NULL` и `*(ULONG *)p ==
ulMagic`, иначе `NULL`.

После `Destroy` magic переписывается в 0 (`pCtl->ulMagic = 0`).
Значение `"DEAD"` (0x44454144) не используется: оно не даёт
диагностического выигрыша по сравнению с 0, а занимает нишу
возможного будущего типа magic.

**Что magic отсеивает:**

- handle от другого контейнера (например, `SetAdd` получил handle
  от `List`);
- большинство неинициализированных значений (`NULL`, 0, случайный
  мусор);
- handle, освобождённый через Destroy.

**Что magic не отсеивает:** произвольный указатель, случайно
оказавшийся на читаемом адресе с корректным magic. Это
принципиальное ограничение любой схемы magic без полного
type-tagging.

**Значения magic:**

| Magic | Тип |
|---|---|
| `"VECT"` | vector |
| `"DLST"` | dlist (legacy) и list (Фаза 1) — оба |
| `"SET "` | set (SENU занят существующим StrSetEnum в strset.c) |
| `"HTBL"` | hashtable |
| `"SSTS"` | strset |
| `"STRM"` | strmap |
| `"STAK"` | stack |
| `"RBT "` | rbtree |
| `"HEAP"` | heap |
| `"ITER"` | iterator |
| `"HASH"` | HASHCTL (внутренний) |
| `"SENU"` | StrSetEnum (сохраняется из текущего strset.c) |

---

## 4. Size-query конвенция

Публичные функции с буфером (`VectorGetItem`, `StrSetGetItem`,
`StrSetEnumGet`, `StrMapGetIthKey`, `StrMapGetIthValue`, будущие
`IteratorGet*`) следуют правилу:

- `buf == NULL && size == 0` -> записать требуемый размер в
  `*pulUsed`, вернуть `NO_ERROR`;
- `size >= required` -> скопировать, вернуть `NO_ERROR`, `*pulUsed`
  = фактический размер;
- `size < required` -> вернуть `ERROR_BUFFER_OVERFLOW`, `*pulUsed` =
  требуемый размер.

`pulUsed` всегда опционален (`NULL` допустим).

Позволяет работать по схеме «узнать размер -> выделить -> прочитать»,
что критично для строк переменной длины (`StrMapGetIthKey`,
`StrMapGetIthValue`).

---

## 5. Generation (Фаза 3)

- `ulGeneration` в control-блоке каждого контейнера.
- Unmanaged-итератор хранит `ulGenAtStart`, сверяет его при каждой
  операции (`IteratorNext`, `IteratorPrevious`, `IteratorGet`,
  `IteratorRemove`).
- `IteratorRemove` — self-revalidation: `container->ulGeneration++`,
  `self->ulGenAtStart = container->ulGeneration`. Иначе собственный
  итератор сразу стал бы невалидным.
- Managed-итераторы живут в `MANAGED_CTX`, `ulGeneration` не
  инкрементируется; позиции корректируются через
  `ManagedNotify*`.

Дефолт для новых итераторов: unmanaged, не read-only. Режим
переключается через `IteratorSetMode` до первого `IteratorFirst`
или `IteratorLast`. После — `ERROR_INVALID_PARAMETER`.

---

## 6. Ownership

- **Generic-контейнеры** (vector, list, set, hashtable, rbtree,
  heap, stack) владеют только узлами и массивами. Содержимое
  элементов — ответственность вызывающего по умолчанию.

- **Opt-in через `pDestroyKey`/`pDestroyValue`** в `CCLOPTS`. При
  `Remove`/`Destroy`/`VectorSet` контейнер вызывает callback на
  **указателе на хранимые байты** (адрес `&array[i]`,
  `&node->elemt`, `&storedKey`), а не на значении. Callback читает
  байты и освобождает то, на что они указывают.

- **Строковые профили** (`strset`, `strmap`) владеют строками через
  `str_free_ccl` (`(*CclFree)(*(PCSZ*)p)`).

- **list.c (Фаза 1)** — SOM-указатели. SOM-обёртка решает:
  `pDestroyValue = NULL` (SOM владеет) или `pDestroyValue =
  som_free_adapter`.

- **`Remove`/`Destroy`** вызывают `pDestroy*`. **`Get`/`Pop`/`Top`**
  — не вызывают. Для получения удаляемого объекта сначала `Get`,
  потом `Remove`.

**Контракт `pDestroyKey`/`pDestroyValue`:**

Callback **не должен:**

- модифицировать контейнер (`Add`/`Remove`/`Set`/`Destroy`) —
  реентерабельность запрещена;
- сохранять указатель на элемент (он освобождается);
- предполагать, что контейнер жив (при `Destroy` control-блок
  может быть уже недоступен).

Callback **может:**

- освобождать ресурсы, на которые указывает элемент;
- вызывать `(*CclFree)` / `somFree` на содержимом.

---

## 7. Соглашение вызова

- **Публичный API** — `APIENTRY` (пусто на host, `_System` на
  OS/2). Единообразно во всех публичных функциях: `VectorCreate`,
  `StrSetAdd`, `CclOptsInit`, `CclValidateOpts`, `Iterator*` (Фаза
  3).

- **Хуки аллокатора** (`CCL_MALLOC_FN` и др.) — через `APIENTRY`,
  wrappers в `ccl_alloc.c` (см. §1). Это единственное место, где
  `APIENTRY` в typedef критичен: SOM-init делает прямое
  присваивание, типы должны совпадать точно.

- **Callbacks контейнеров** (`CCL_COMPARE_FN`, `CCL_HASH_FN`,
  `CCL_DESTROY_FN`, `CCL_KEY_FN`) — **без** `APIENTRY`. Присваивание
  `_System`-функции этим typedef'ам — задача адаптера вызывающего
  (SOM-слой), не CCL. Причина: контейнеры вызывают callbacks через
  обычный function pointer (`pOpts->pCompare(...)`), специальное
  calling convention не требуется. CCL не навязывает `APIENTRY`
  для callback'ов, чтобы не вынуждать пользователя писать
  wrapper'ы для обычных C-функций.

- **Legacy** `dlist`/`all_dlist` — `_System` на публичных функциях
  (`CreateList`, `InsertItem`, ...). Часть их ABI, не меняется.

---

## 8. Layout LINKED_NODE

```c
typedef struct _stack_component {
    PVOID                    elemt;  /* offset 0 */
    struct _stack_component *next;   /* offset 4 */
    struct _stack_component *prev;   /* offset 8 */
    ULONG                    size;   /* offset 12 */
    ULONG                    tag;    /* offset 16 */
} LINKED_NODE, *PLINKED_NODE;
```

- `elemt` на offset 0 — часть ABI `stack_cmp` (публичный тип
  `all_dlist.h`). Reorder полей недопустим.
- `stack_cmp` — `typedef LINKED_NODE stack_cmp` в
  `all_dlist_private.h`, один тип. Typecast — no-op.
- `size`/`tag` — метаданные элемента, интерпретируемые
  потребителем:
  - `dlist.c`: `size` = размер payload, `tag` = пользовательский
    kind;
  - `all_dlist.c`: передаёт 0 и игнорирует оба;
  - `list.c` (Фаза 1): `size = sizeof(SOMObject*)`, `tag = 0`.
- Примитивы `LinkedNodeInsertAfter`/`LinkedNodeInsertBefore`/
  `LinkedNodeUnlink` работают только на `next`/`prev`. `elemt`,
  `size`, `tag` не трогают.
- `LinkedNodeUnlink` обнуляет `pNode->prev` и `pNode->next` после
  связывания соседей. Это для отладки: после `Unlink` узел не
  достижим из контейнера, висячих back-link нет.

Bookkeeping контейнера (`pHead`/`pTail`/`ulCount`/`pCurrent`/
`ulGeneration`) — на вызывающем. Примитивы знают только next/prev.

---

## 9. Elemt-модель (list, Фаза 1)

- `list.c` хранит SOM-указатели. `elemt` — `SOMObject *`.
- `LinkedNodeAlloc(pSOMObject, sizeof(SOMObject*), 0)`.
- Per-element malloc нет: SOM-объект уже в куче SOM.
- `list.c` не имеет `pCurrent` (в отличие от legacy Dlist, где
  курсор — часть контракта IBM Dlist).
- `list.c` имеет свой control-блок `LISTCTL` с `ulGeneration` и
  `MANAGED_CTX` для итераторов.
- `list.c` не дублирует `dlist.c`: `LISTCTL` != `DLISTCTL`, разный
  набор полей, разная семантика (`list` не имеет курсора, но имеет
  generation).

---

## 10. Legacy-исключение

`dlist.c` и `all_dlist.c` не трогаются по публичному API. Внутри —
рефакторинг на общие примитивы `linked_list.c`, поведение
сохранено.

**Что остаётся на malloc/free/calloc:**

| Объект                              | Аллокатор |
|-------------------------------------|-----------|
| `LINKED_NODE` (все модули)          | `(*CclMalloc)` |
| Payload `dlist` (InsertItem)        | `malloc` (legacy) |
| Payload `dlist` (InsertObject)      | пользовательский (legacy) |
| Control block `dlist` (`DLISTCTL`)  | `calloc`/`free` (legacy) |
| Temp arrays `dlist` (`SortList`)    | `malloc`/`free` (legacy) |
| Payload `all_dlist`                 | пользовательский (legacy) |
| Handle `all_dlist` (`phStack`/`phList`) | `malloc`/`free` (legacy) |
| Payload `list.c` (SOM-объект)       | SOM runtime |
| Control block `vector`/`strset`     | `(*CclCalloc)` |
| Массивы `vector`                    | `(*CclMalloc)`/`(*CclRealloc)` |
| Строки `strset`/`strmap`            | `(*CclMalloc)` |

**Почему legacy-payload остаётся на `malloc`:** `dlist.c` копирует
payload через `malloc(ItemSize)` в `InsertItem` и освобождает через
`free` в `DeleteItem`/`ExtractItem`/`DestroyList(TRUE)`. Пользователь,
использующий `InsertObject`, должен передавать указатели, которые
можно освободить через `free`. Это часть контракта IBM Dlist, не
CCL. Переводить payload на `(*CclMalloc)` означало бы сломать
ABI-совместимость с существующим кодом, который использует `dlist`
с указателями из `malloc`.

**Узлы** `LINKED_NODE` при этом **идут через `(*CclMalloc)`** во
всех трёх модулях (dlist, all_dlist, list). Это внутренние узлы,
пользователь их не видит. SOM-init с `CclMalloc = som_malloc_adapter`
перенаправляет их в SOM-аллокатор без единой правки в
`dlist.c`/`all_dlist.c`.

---

## 11. 32/64-bit

- Текущие платформы: OS/2 32-bit, host 32-bit (OpenWatcom `binnt`).
- `ccl_node_alloc` использует `ULONG`-арифметику над указателями
  (§2). На 64-bit потребуется `CCL_PTR`.
- Offsets `4/8/12/16` в §8 актуальны для 32-bit. На 64-bit поля
  работают через имена (`pNode->elemt`, `pNode->next`), документация
  про 4/8 — только для 32-bit OS/2.
- `TREENODE` на 64-bit — `offsetof(u) = 32`, компилятор сам вставляет
  padding (§2).

---

## 12. Критерий core/profile

**Core** — модуль, который аллоцирует и освобождает собственную
структуру хранения через `(*CclMalloc)` / `(*CclRealloc)` /
`ccl_node_alloc`.

**Profile** — модуль, который только конфигурирует core (через
`CCLOPTS`, специализацию `ulValueSize`, ограничение публичного API)
и делегирует вызовы. Не содержит собственных аллокаций storage.
Единственная аллокация в profile — маленький control-блок через
`(*CclMalloc)`, не storage.

**Core-модули (владеют storage):**

- `vector.c` — массив элементов;
- `list.c` — узлы `LINKED_NODE`;
- `rbtree.c` — узлы `TREENODE`;
- `hash_private.c` — bucket array + `HASHNODE`.

**Profiles (не владеют storage):**

- `hashtable.c` — обёртка над `hash_private`;
- `set.c` — `hashtable` с `ulValueSize = 0`;
- `strmap.c` — owned-string профиль над `hash_private`;
- `strset.c` — owned-string профиль над `hash_private`,
  `ulValueSize = 0`;
- `stack.c` — обёртка над `vector` (LIFO);
- `heap.c` — обёртка над `vector` (binary heap, sift_up/sift_down).

`heap.c` и `stack.c` — оба profiles над `vector`, оба добавляют
только control-блок, storage не владеют. Алгоритмическая сложность
(`sift_up`/`sift_down` в heap) не делает модуль core: критерий —
владение storage, не алгоритмическая сложность.

**Проверка CI:**

```sh
grep -l 'CclRealloc\|ccl_node_alloc' src/*.c
  # -> только vector.c, list.c, rbtree.c, hash_private.c
```

Если профиль (`hashtable.c`, `set.c`, `strset.c`, `strmap.c`,
`stack.c`, `heap.c`) появится в этом выводе — ошибка декомпозиции:
профиль не должен аллоцировать storage.

---

## 13. CI

Проверки, которые прогоняются на CI после Фазы 0:

```sh
# 1. Прямые аллокаторы только в разрешённых модулях.
grep -nE '\b(malloc|calloc|realloc|free)\s*\(' src/*.c \
  | grep -v -e '^src/ccl_alloc\.c' \
            -e '^src/dlist\.c' \
            -e '^src/all_dlist\.c' \
  && exit 1 || exit 0

# 2. Никаких остатков старого API.
grep -rnE 'ccl_mem_|CclSetAllocator|CclResetAllocator|mem_private\.' \
  src/ private/ inc/ \
  && exit 1 || exit 0

# 3. elemt только в разрешённых модулях.
grep -rn 'elemt' src/ \
  | grep -v -e '^src/linked_list\.c' \
            -e '^src/dlist\.c' \
            -e '^src/all_dlist\.c' \
            -e '^src/list\.c' \
  && exit 1 || exit 0

# 4. Профили не содержат CclRealloc/ccl_node_alloc.
grep -l 'CclRealloc\|ccl_node_alloc' src/*.c \
  | grep -v -e vector.c -e list.c -e rbtree.c -e hash_private.c \
  && exit 1 || exit 0
```

Правила 1–3 действуют с Фазы 0 (с оговорками по `vector.c`/`strset.c`,
которые ещё не мигрированы на `(*CclMalloc)` — они переезжают в Фазе
1/1.5). Правило 4 — с Фазы 1, когда появляются все четыре core-модуля.

---

## 14. Порядок сборки

Топологический, от листьев к корням (полный граф — в §2.3
архитектурного документа).

```
Уровень 0 (фундамент):
    ccl_alloc.c       — 4 хука + APIENTRY-wrapper'ы

Уровень 1:
    ccl_base.c        — CclOptsInit, CclValidateOpts, ccl_validate

Уровень 2:
    linked_list.c     — LinkedNodeAlloc/Free + 3 примитива
    node_private.c    — ccl_node_alloc/free

Уровень 3:
    str_private.c     — FNV-1a, strcmp-обёртки
    hash_private.c    — HASHCTL, HASHNODE, Hash*
    tree_private.c    — TREENODE, RB-примитивы

Уровень 4 (core-контейнеры):
    vector.c          — база для stack, heap
    list.c            — LINKED_NODE
    rbtree.c          — TREENODE

Уровень 5 (profiles):
    hashtable.c, set.c, stack.c, heap.c, strmap.c, strset.c

Уровень 6 (итераторы, Фаза 3):
    iterator.c, iterator_managed.c

Уровень 7 (legacy, параллельная ветка):
    dlist.c, all_dlist.c — линкуются с ccl.lib, используют
    linked_list.o и ccl_alloc.o
```

`ccl_alloc.c` первым: от него зависят все аллокации. `linked_list.c`
и `node_private.c` — после него (`(*CclMalloc)` на этапе линковки).
`ccl_base.c` — после `ccl_alloc.c` (сам не аллоцирует, но логически
базовый).

---

## 15. Ссылки

- `CCL: архитектура и план реализации` v0.9.5 — основной документ.
- `ccl_alloc.h`, `ccl_base.h` — публичные контракты.
- `linked_list_private.h`, `hash_private.h`, `tree_private.h`,
  `node_private.h`, `ccl_base_private.h`, `iterator_private.h`,
  `iterator_managed_private.h` — внутренние заголовки.
- `somapi.h` — объявления `somTD_SOMMalloc` / `SOMLINK`, образец
  для `ccl_alloc.h`.
- `all_dlist.h`, `dlist.h` — публичные типы legacy-модулей
  (`stack_cmp`, `DLIST`).
```

---

