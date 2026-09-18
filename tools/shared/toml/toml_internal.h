/* toml_internal.h - internal structures of the TOML parser.
 * Do not include from consumer code. */
#ifndef TOML_INTERNAL_H
#define TOML_INTERNAL_H

#include "toml.h"
#include "ccl.h"

/**
 * @file toml_internal.h
 * @brief Private interface of the TOML parser.
 */

typedef struct _TOMLVALUE TOMLVALUE, *PTOMLVALUE;
typedef struct _TOMLARRAY TOMLARRAY, *PTOMLARRAY;
typedef struct _TOMLTABLE TOMLTABLE, *PTOMLTABLE;
typedef struct _TOMLENTRY TOMLENTRY, *PTOMLENTRY;
typedef struct _TOMLDOC   TOMLDOC,   *PTOMLDOC;
typedef struct _TOMLFIND  TOMLFIND,  *PTOMLFIND;

/** @brief Dynamic array of values. */
struct _TOMLARRAY {
    PTOMLVALUE *paItems;
    ULONG       ulCount;
    ULONG       ulCapacity;
    ULONG       flFlags;
};

/** @brief Key/value pair inside a table. */
struct _TOMLENTRY {
    PSZ         pszKey;
    PTOMLVALUE  pValue;
};

/**
 * @brief Table.
 *
 * Flags:
 *  - TOML_TABLE_INLINE: created from an inline table { ... }.
 *  - TOML_TABLE_ARRAY_ELEM: element of [[array of tables]].
 *  - TOML_TABLE_EXPLICIT: defined by a [table] or [[table]] header.
 *  - TOML_TABLE_FROM_DOTTED: created by a dotted key.
 *
 * @todo Заменить @c hEntries на map (ассоциативный массив) для O(1)
 *       поиска по ключу. Сейчас используется HVECTOR с линейным
 *       поиском.
 */
struct _TOMLTABLE {
    HVECTOR  hEntries;   /**< Элементы типа TOMLENTRY. */
    ULONG    flFlags;    /**< Флаги TOML_TABLE_*. */
};

/** @brief Tagged value. */
struct _TOMLVALUE {
    ULONG       ulType;
    union {
        PSZ         pszString;
        LONGLONG    llInteger;
        double      dblFloat;
        BOOL        fBoolean;
        PTOMLARRAY  pArray;
        PTOMLTABLE  pTable;
    } u;
};

/** @brief Open document. */
struct _TOMLDOC {
    PTOMLTABLE  pRoot;      /* Root table                        */
    PTOMLVALUE  pRootNode;  /* Synthetic value wrapping pRoot    */
    PTOMLFIND   pFirstFind; /* Head of active Find cursors list  */
};

/** @brief Enumeration cursor. */
struct _TOMLFIND {
    PTOMLDOC    pDoc;
    PTOMLTABLE  pTable;
    ULONG       ulCurrent;
    PSZ         pszPattern;
    PTOMLFIND   pNext;
    PCSZ        pszCurrentKey;
    PTOMLVALUE  pCurrentValue;
};

#define TOML_TABLE_INLINE        0x0001
#define TOML_TABLE_ARRAY_ELEM    0x0002
#define TOML_TABLE_EXPLICIT      0x0004
#define TOML_TABLE_FROM_DOTTED   0x0008

#define TOML_ARRAY_OF_TABLES     0x0001

/**
 * @brief Parse TOML text and build the tree.
 * @param[in]  pszText    NUL-terminated UTF-8 TOML text.
 * @param[out] ppRoot     Receiver for the root table.
 * @param[out] ppszError  Optional static error description.
 * @return APIRET
 */
APIRET TomlInternalParse(PCSZ pszText, PTOMLTABLE *ppRoot,
                         PCSZ *ppszError);

/**
 * @brief Release a tree built by TomlInternalParse.
 * @param[in] pRoot  Root table. May be NULL.
 */
void TomlInternalFreeTree(PTOMLTABLE pRoot);

#endif /* TOML_INTERNAL_H */
