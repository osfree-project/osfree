/*!
 *
 * @file toml_internal.h
 *
 * @brief Private interface of the TOML parser.
 *
 * Internal structures of the TOML parser. Do not include from
 * consumer code.
 */

#ifndef TOML_INTERNAL_H
#define TOML_INTERNAL_H

#include "toml.h"
#include "ccl.h"

/*!
 * @brief Forward declaration of a value node.
 */
typedef struct _TOMLVALUE TOMLVALUE, *PTOMLVALUE;

/*!
 * @brief Forward declaration of an array node.
 */
typedef struct _TOMLARRAY TOMLARRAY, *PTOMLARRAY;

/*!
 * @brief Forward declaration of a table node.
 */
typedef struct _TOMLTABLE TOMLTABLE, *PTOMLTABLE;

/*!
 * @brief Forward declaration of a table entry.
 */
typedef struct _TOMLENTRY TOMLENTRY, *PTOMLENTRY;

/*!
 * @brief Forward declaration of an open document.
 */
typedef struct _TOMLDOC   TOMLDOC,   *PTOMLDOC;

/*!
 * @brief Forward declaration of an enumeration cursor.
 */
typedef struct _TOMLFIND  TOMLFIND,  *PTOMLFIND;

/*!
 * @brief Dynamic array of values.
 */
struct _TOMLARRAY {
    PTOMLVALUE *paItems;   /*!< Item storage, or NULL.       */
    ULONG       ulCount;   /*!< Number of items stored.      */
    ULONG       ulCapacity;/*!< Allocated item slots.        */
    ULONG       flFlags;   /*!< TOML_ARRAY_* flags.          */
};

/*!
 * @brief Key/value pair inside a table.
 */
struct _TOMLENTRY {
    PSZ         pszKey;    /*!< Key, owned by the entry.     */
    PTOMLVALUE  pValue;    /*!< Value, owned by the entry.   */
};

/*!
 * @brief Table.
 *
 * Flags:
 *  - TOML_TABLE_INLINE: created from an inline table { ... }.
 *  - TOML_TABLE_ARRAY_ELEM: element of [[array of tables]].
 *  - TOML_TABLE_EXPLICIT: defined by a [table] or [[table]] header.
 *  - TOML_TABLE_FROM_DOTTED: created by a dotted key.
 *
 * @todo Replace @c hEntries with a map (associative array) for O(1)
 *       key lookup. Currently an HVECTOR with linear search is used.
 */
struct _TOMLTABLE {
    HVECTOR  hEntries;   /*!< Entries of type TOMLENTRY. */
    ULONG    flFlags;    /*!< TOML_TABLE_* flags.        */
};

/*!
 * @brief Tagged value.
 */
struct _TOMLVALUE {
    ULONG       ulType;  /*!< One of TOML_TYPE_*.              */
    union {
        PSZ         pszString; /*!< String or date-time text.  */
        LONGLONG    llInteger; /*!< Integer value.             */
        double      dblFloat;  /*!< Float value.               */
        BOOL        fBoolean;  /*!< Boolean value.             */
        PTOMLARRAY  pArray;    /*!< Array value.               */
        PTOMLTABLE  pTable;    /*!< Table value.               */
    } u;
};

/*!
 * @brief Open document.
 */
struct _TOMLDOC {
    PTOMLTABLE  pRoot;      /*!< Root table.                         */
    PTOMLVALUE  pRootNode;  /*!< Synthetic value wrapping pRoot.     */
    PTOMLFIND   pFirstFind; /*!< Head of active Find cursors list.   */
};

/*!
 * @brief Enumeration cursor.
 */
struct _TOMLFIND {
    PTOMLDOC    pDoc;            /*!< Owning document.               */
    PTOMLTABLE  pTable;          /*!< Table being scanned.           */
    ULONG       ulCurrent;       /*!< Next entry index to inspect.   */
    PSZ         pszPattern;      /*!< Pattern, owned by the cursor.  */
    PTOMLFIND   pNext;           /*!< Next cursor of the document.   */
    PCSZ        pszCurrentKey;   /*!< Current key, or NULL.          */
    PTOMLVALUE  pCurrentValue;   /*!< Current value, or NULL.        */
};

/*!
 * @brief Inline table flag: created from { ... }.
 */
#define TOML_TABLE_INLINE        0x0001

/*!
 * @brief Array element flag: element of [[array of tables]].
 */
#define TOML_TABLE_ARRAY_ELEM    0x0002

/*!
 * @brief Explicit flag: defined by a [table] or [[table]] header.
 */
#define TOML_TABLE_EXPLICIT      0x0004

/*!
 * @brief Dotted key flag: created by a dotted key.
 */
#define TOML_TABLE_FROM_DOTTED   0x0008

/*!
 * @brief Array of tables flag: array of tables.
 */
#define TOML_ARRAY_OF_TABLES     0x0001

/*!
 * @brief Parse TOML text and build the tree.
 *
 * @param[in]  pszText    NUL-terminated UTF-8 TOML text. Not NULL.
 * @param[out] ppRoot     Receiver for the root table. Not NULL.
 * @param[out] ppszError  Optional. May be NULL. Receives a static
 *                        error description on failure.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      pszText or ppRoot is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Memory allocation failure.
 * @retval TOML_ERROR_INVALID_UTF8      File content is not valid
 *                                      UTF-8.
 * @retval TOML_ERROR_INVALID_SYNTAX    TOML syntax error.
 * @retval TOML_ERROR_DUPLICATE_KEY     Duplicate key.
 */
APIRET TomlInternalParse(PCSZ pszText, PTOMLTABLE *ppRoot,
                         PCSZ *ppszError);

/*!
 * @brief Release a tree built by TomlInternalParse.
 *
 * @param[in] pRoot  Root table. May be NULL.
 */
void TomlInternalFreeTree(PTOMLTABLE pRoot);

#endif /* TOML_INTERNAL_H */
