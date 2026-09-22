/*!
 * @file reuse_toml_internal.h
 *
 * @brief Private interface of the REUSE.toml parser.
 *
 * Internal structures of the REUSE.toml parser. Do not include from
 * consumer code.
 */

#ifndef REUSE_TOML_INTERNAL_H
#define REUSE_TOML_INTERNAL_H

#include "reuse_toml.h"

/*!
 * @brief Forward declaration of one parsed annotation.
 */
typedef struct _REUSEANN     REUSEANN,     *PREUSEANN;

/*!
 * @brief Forward declaration of a parsed REUSE.toml document.
 */
typedef struct _REUSETOMLDOC REUSETOMLDOC, *PREUSETOMLDOC;

/*!
 * @struct _REUSEANN
 * @brief One parsed [[annotations]] entry.
 *
 * All string fields are malloc'd and owned by the annotation. They
 * are released by ann_free.
 */
struct _REUSEANN {
    PSZ  *papszPaths;                 /*!< Path patterns.                */
    ULONG ulPathCount;                /*!< Number of patterns.           */
    PSZ   pszLicense;                 /*!< SPDX-License-Identifier.      */
    PSZ   pszCopyright;               /*!< SPDX-FileCopyrightText.       */
    PSZ  *papszContributors;          /*!< SPDX-FileContributor.         */
    ULONG ulContributorCount;         /*!< Number of contributors.       */
    PSZ   pszPackageName;             /*!< SPDX-PackageName.             */
    PSZ   pszPackageSupplier;         /*!< SPDX-PackageSupplier.         */
    PSZ   pszPackageDownloadLocation; /*!< SPDX-PackageDownloadLocation. */
    PSZ   pszPackageComment;          /*!< SPDX-PackageComment.          */
    ULONG ulPrecedence;               /*!< REUSE_PRECEDENCE_*.           */
    ULONG ulOrderInFile;              /*!< Position in the file.         */
};

/*!
 * @struct _REUSETOMLDOC
 * @brief Parsed REUSE.toml document.
 */
struct _REUSETOMLDOC {
    PSZ       pszSourceDir;         /*!< Directory of the file.         */
    LONGLONG  llVersion;            /*!< "version" value.               */
    REUSEANN *paAnnotations;        /*!< Parsed annotations.            */
    ULONG     ulAnnotationCount;    /*!< Number of used entries.        */
    ULONG     ulAnnotationCapacity; /*!< Allocated capacity.            */
};

#endif /* REUSE_TOML_INTERNAL_H */
