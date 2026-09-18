/* reuse_toml_internal.h - internal structures of the REUSE.toml parser.
 * Do not include from consumer code. */
#ifndef REUSE_TOML_INTERNAL_H
#define REUSE_TOML_INTERNAL_H

#include "reuse_toml.h"

/**
 * @file reuse_toml_internal.h
 * @brief Private interface of the REUSE.toml parser.
 */

typedef struct _REUSEANN     REUSEANN,     *PREUSEANN;
typedef struct _REUSETOMLDOC REUSETOMLDOC, *PREUSETOMLDOC;

/** @brief One parsed [[annotations]] entry. */
struct _REUSEANN {
    char     **paPaths;
    ULONG      ulPathCount;
    char      *pszLicense;
    char      *pszCopyright;
    char     **paContributors;
    ULONG      ulContributorCount;
    char      *pszPackageName;
    char      *pszPackageSupplier;
    char      *pszPackageDownloadLocation;
    char      *pszPackageComment;
    ULONG      ulPrecedence;
    ULONG      ulOrderInFile;
};

/** @brief Parsed REUSE.toml document. */
struct _REUSETOMLDOC {
    char      *pszSourceDir;
    LONGLONG   llVersion;
    REUSEANN  *paAnnotations;
    ULONG      ulAnnotationCount;
    ULONG      ulAnnotationCapacity;
};

#endif /* REUSE_TOML_INTERNAL_H */
