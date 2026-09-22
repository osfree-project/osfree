/*!
 *
 * @file dep5_internal.h
 *
 * @brief Private interface of the DEP5 parser.
 *
 * Internal structures of the DEP5 parser. Do not include from
 * consumer code.
 */

#ifndef DEP5_INTERNAL_H
#define DEP5_INTERNAL_H

#include "dep5.h"

/*!
 * @brief Forward declaration of a field.
 */
typedef struct _DEP5FIELD  DEP5FIELD,  *PDEP5FIELD;

/*!
 * @brief Forward declaration of a stanza.
 */
typedef struct _DEP5STANZA DEP5STANZA, *PDEP5STANZA;

/*!
 * @brief Forward declaration of a parsed document.
 */
typedef struct _DEP5DOC    DEP5DOC,    *PDEP5DOC;

/*!
 * @brief Forward declaration of an enumeration cursor.
 */
typedef struct _DEP5FIND   DEP5FIND,   *PDEP5FIND;

/*!
 * @brief One "Name: value" pair inside a stanza.
 */
struct _DEP5FIELD {
    PSZ pszName;   /*!< Field name, owned by the field.        */
    PSZ pszValue;  /*!< Field value, lines joined by '\n'.     */
};

/*!
 * @brief Header stanza kind.
 */
#define DEP5_STANZA_HEADER   1

/*!
 * @brief Files stanza kind.
 */
#define DEP5_STANZA_FILES    2

/*!
 * @brief Stand-alone License stanza kind.
 */
#define DEP5_STANZA_LICENSE  3

/*!
 * @brief One stanza.
 */
struct _DEP5STANZA {
    DEP5FIELD *paFields;   /*!< Field storage, or NULL.      */
    ULONG      ulCount;    /*!< Number of fields stored.     */
    ULONG      ulCapacity; /*!< Allocated field slots.       */
    ULONG      ulKind;     /*!< One of DEP5_STANZA_*.        */
};

/*!
 * @brief Parsed document.
 */
struct _DEP5DOC {
    DEP5STANZA *paStanzas;     /*!< Stanza storage, or NULL.      */
    ULONG       ulCount;       /*!< Number of stanzas stored.     */
    ULONG       ulCapacity;    /*!< Allocated stanza slots.       */

    ULONG       ulHeaderIndex; /*!< Index of header, or
                                    DEP5_NO_HEADER if none.       */
    ULONG       ulFilesCount;  /*!< Precomputed Files count.      */
    ULONG       ulLicenseCount;/*!< Precomputed License count.    */
};

/*!
 * @brief Sentinel value for "no header stanza".
 */
#define DEP5_NO_HEADER  0xFFFFFFFFUL

/*!
 * @brief Enumeration cursor.
 *
 * Tracks a subset of stanzas (Files or License) and caches derived
 * data for the current one.
 */
struct _DEP5FIND {
    PDEP5DOC  pDoc;              /*!< Owning document.              */
    ULONG     ulKind;            /*!< DEP5_STANZA_FILES or LICENSE. */
    ULONG     ulCurrent;         /*!< Index in pDoc->paStanzas.     */
    ULONG     ulSeen;            /*!< Cursors advanced so far.      */

    /* Derived data for the current stanza (owned by this cursor). */
    char    **paPatterns;        /*!< Unescaped Files patterns.     */
    ULONG     ulPatternCount;    /*!< Number of patterns.           */
    PSZ       pszShortName;      /*!< License synopsis.             */
    PSZ       pszLicenseText;    /*!< License body, '\n'-joined.    */
};

/* ------------------------------------------------------------------
 * Internal helpers (visible only to dep5.c).
 * ------------------------------------------------------------------ */

/*!
 * @brief Translate a public document handle into the internal
 *        pointer.
 *
 * @param[in] hDoc  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if the handle is NULLHANDLE.
 *
 * @retval NULL  hDoc is NULLHANDLE.
 */
PDEP5DOC Dep5InternalGetDoc(HDEP5DOC hDoc);

/*!
 * @brief Translate a public cursor handle into the internal pointer.
 *
 * @param[in] hFind  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if the handle is NULLHANDLE.
 *
 * @retval NULL  hFind is NULLHANDLE.
 */
PDEP5FIND Dep5InternalGetFind(HDEP5FIND hFind);

/*!
 * @brief Free per-stanza derived data cached in a cursor.
 *
 * @param[in,out] pFind  Cursor. May be NULL.
 */
void Dep5InternalFindFreeCache(PDEP5FIND pFind);

#endif /* DEP5_INTERNAL_H */
