/* dep5_internal.h - internal structures of the DEP5 parser.
 * Do not include from consumer code. */
#ifndef DEP5_INTERNAL_H
#define DEP5_INTERNAL_H

#include "dep5.h"

/**
 * @file dep5_internal.h
 * @brief Private interface of the DEP5 parser.
 */

typedef struct _DEP5FIELD  DEP5FIELD,  *PDEP5FIELD;
typedef struct _DEP5STANZA DEP5STANZA, *PDEP5STANZA;
typedef struct _DEP5DOC    DEP5DOC,    *PDEP5DOC;
typedef struct _DEP5FIND   DEP5FIND,   *PDEP5FIND;

/** @brief One "Name: value" pair inside a stanza. */
struct _DEP5FIELD {
    PSZ pszName;
    PSZ pszValue;      /* lines joined by '\n' */
};

/** @brief Stanza kinds. */
#define DEP5_STANZA_HEADER   1
#define DEP5_STANZA_FILES    2
#define DEP5_STANZA_LICENSE  3

/** @brief One stanza. */
struct _DEP5STANZA {
    DEP5FIELD *paFields;
    ULONG      ulCount;
    ULONG      ulCapacity;
    ULONG      ulKind;
};

/** @brief Parsed document. */
struct _DEP5DOC {
    DEP5STANZA *paStanzas;
    ULONG       ulCount;
    ULONG       ulCapacity;

    ULONG       ulHeaderIndex;    /* DEP5_NO_HEADER if none  */
    ULONG       ulFilesCount;     /* precomputed             */
    ULONG       ulLicenseCount;   /* precomputed             */
};

#define DEP5_NO_HEADER  0xFFFFFFFFUL

/**
 * @brief Enumeration cursor. Tracks a subset of stanzas (Files or
 * License) and caches derived data for the current one.
 */
struct _DEP5FIND {
    PDEP5DOC  pDoc;
    ULONG     ulKind;            /* DEP5_STANZA_FILES or LICENSE */
    ULONG     ulCurrent;         /* index in pDoc->paStanzas      */
    ULONG     ulSeen;

    /* Derived data for the current stanza (owned by this cursor). */
    char    **paPatterns;        /* unescaped Files patterns       */
    ULONG     ulPatternCount;
    PSZ       pszShortName;      /* License synopsis               */
    PSZ       pszLicenseText;    /* License body, '\n'-joined      */
};

/* ------------------------------------------------------------------
 * Internal helpers (visible only to dep5.c).
 * ------------------------------------------------------------------ */

PDEP5DOC Dep5InternalGetDoc(HDEP5DOC hDoc);
PDEP5FIND Dep5InternalGetFind(HDEP5FIND hFind);

/* Free per-stanza derived data cached in a cursor. */
void Dep5InternalFindFreeCache(PDEP5FIND pFind);

#endif /* DEP5_INTERNAL_H */
