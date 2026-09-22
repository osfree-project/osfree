/*!
 * @file spdx_db_private.h
 *
 * @brief Private interface of the SPDX database module.
 *
 * Internal structures of the SPDX database. Do not include from
 * consumer code.
 *
 * Types and constants used by spdx_db.c only. Not part of the
 * public interface; do not include from consumer code.
 */

#ifndef SPDX_DB_PRIVATE_H
#define SPDX_DB_PRIVATE_H

#include "spdx_db.h"

/* ==================================================================
 * Metadata flags
 * ================================================================== */

/*!
 * @def SPDXDB_FLAG_OSI
 * @brief License is OSI-approved.
 */
#define SPDXDB_FLAG_OSI         0x01

/*!
 * @def SPDXDB_FLAG_FSF_LIBRE
 * @brief License is FSF-libre.
 */
#define SPDXDB_FLAG_FSF_LIBRE   0x02

/*!
 * @def SPDXDB_FLAG_DEPRECATED
 * @brief Identifier is deprecated.
 */
#define SPDXDB_FLAG_DEPRECATED  0x04

/* ==================================================================
 * Entry types
 * ================================================================== */

/*!
 * @struct _SPDXLICENSEENTRY
 * @brief One SPDX license record.
 *
 * Holds the index data (identifier, flags, name, seeAlso list) and,
 * once the detail has been loaded, the full text, template and HTML.
 */
typedef struct _SPDXLICENSEENTRY {
    PSZ   pszId;             /*!< SPDX identifier.              */
    UCHAR uchFlags;          /*!< SPDXDB_FLAG_* bits.           */
    PSZ   pszName;           /*!< Human-readable name.          */
    PSZ  *papszSeeAlso;      /*!< NULL-terminated list.         */

    ULONG ulDetailOffset;    /*!< Cache offset, 0 if none.      */
    ULONG ulDetailSize;      /*!< Cache record size, 0 if none. */

    BOOL  fDetailLoaded;     /*!< Detail has been loaded.       */
    PSZ   pszText;           /*!< License text.                 */
    PSZ   pszTemplate;       /*!< Standard template.            */
    PSZ   pszTextHtml;       /*!< HTML representation.          */
} SPDXLICENSEENTRY, *PSPDXLICENSEENTRY;

/*!
 * @struct _SPDXEXCEPTIONENTRY
 * @brief One SPDX exception record.
 *
 * Same shape as SPDXLICENSEENTRY.
 */
typedef struct _SPDXEXCEPTIONENTRY {
    PSZ   pszId;
    UCHAR uchFlags;
    PSZ   pszName;
    PSZ  *papszSeeAlso;

    ULONG ulDetailOffset;
    ULONG ulDetailSize;

    BOOL  fDetailLoaded;
    PSZ   pszText;
    PSZ   pszTemplate;
    PSZ   pszTextHtml;
} SPDXEXCEPTIONENTRY, *PSPDXEXCEPTIONENTRY;

#endif /* SPDX_DB_PRIVATE_H */
