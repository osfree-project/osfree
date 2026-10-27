/*!
 * @file rspinst_private.h
 *
 * @brief Internal structures shared between rspinst.c and
 *        rspinst_validate.c.
 *
 * This header is not part of the public API. It exists only because
 * rspinst_validate.c must reach into the internal document and
 * report structures to read counters and walk entries. Keeping the
 * layout in one place avoids the risk of two diverging copies.
 */

#ifndef RSPINST_PRIVATE_H
#define RSPINST_PRIVATE_H

#include "rspinst.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @def RSP_DOC_MAGIC
 * @brief Magic marker stored in every live document handle.
 *
 * Value: 0x52535044 ('RSPD').
 */
#define RSP_DOC_MAGIC       0x52535044UL

/*!
 * @def RSP_REPORT_MAGIC
 * @brief Magic marker stored in every live report handle.
 *
 * Value: 0x52535052 ('RSPR').
 */
#define RSP_REPORT_MAGIC    0x52535052UL

/*!
 * @struct _RSP_PAIR
 * @brief One parsed entry.
 *
 * Used for both scalar and block entries. For a scalar entry,
 * pszValue holds the value and pBlock is NULL. For a block entry,
 * pBlock points to the child list and pszValue is NULL.
 */
typedef struct _RSP_PAIR {
    PSZ               pszName;      /*!< Keyword name.                */
    PSZ               pszValue;     /*!< Value for scalar, NULL for
                                     *   a block.                     */
    struct _RSP_BLOCK *pBlock;      /*!< Children for a block,
                                     *   NULL for scalar.             */
    ULONG             ulLine;       /*!< 1-based line in pszFile.     */
    PSZ               pszFile;      /*!< Source file path.            */
} RSP_PAIR;

/*!
 * @struct _RSP_BLOCK
 * @brief Growable array of entries.
 *
 * Used both for the document root and for every nested block.
 */
typedef struct _RSP_BLOCK {
    RSP_PAIR *paItems;              /*!< Entries.                     */
    ULONG     ulCount;              /*!< Number of entries.           */
    ULONG     ulCapacity;           /*!< Allocated capacity.          */
} RSP_BLOCK;

/*!
 * @struct _RSP_INCQUEUE
 * @brief FIFO queue of deferred Include paths.
 *
 * Implements the deferred breadth-first include processing described
 * in IBM Redbook SG24-2010-00, section 7.1.5.3.
 */
typedef struct _RSP_INCQUEUE {
    PSZ   *papszPaths;              /*!< Paths.                       */
    ULONG *paulLines;               /*!< Line numbers.                */
    ULONG  ulHead;                  /*!< Pop index.                   */
    ULONG  ulTail;                  /*!< Push index.                  */
    ULONG  ulCapacity;              /*!< Allocated capacity.          */
} RSP_INCQUEUE;

/*!
 * @struct _RSP_DOC
 * @brief In-memory representation of a parsed document.
 */
typedef struct _RSP_DOC {
    ULONG        ulMagic;           /*!< RSP_DOC_MAGIC while alive.   */
    PSZ          pszSourcePath;     /*!< Path passed to RspOpen.      */
    RSP_BLOCK    stRoot;            /*!< Top-level entries.           */
    RSP_INCQUEUE stIncludes;        /*!< Deferred includes.           */
    PSZ         *papszVisited;      /*!< Files already visited.       */
    ULONG        ulVisitedCount;    /*!< Number of visited files.     */
    ULONG        ulVisitedCapacity; /*!< Allocated capacity.          */
    ULONG        ulErrorCount;      /*!< Parse error counter.         */
    ULONG        ulWarningCount;    /*!< Parse warning counter.       */
} RSP_DOC;

/*!
 * @struct _RSP_REPORT
 * @brief Growable array of diagnostic records.
 */
typedef struct _RSP_REPORT {
    ULONG   ulMagic;                /*!< RSP_REPORT_MAGIC.            */
    RSPERR *paItems;                /*!< Records.                     */
    ULONG   ulCount;                /*!< Number of records.           */
    ULONG   ulCapacity;             /*!< Allocated capacity.          */
    ULONG   ulErrors;               /*!< Records with SEV_ERROR.      */
    ULONG   ulWarnings;             /*!< Records with SEV_WARNING.    */
} RSP_REPORT;

/*!
 * @brief Obtain the RSP_DOC structure from a handle, or NULL.
 *
 * @param[in] hDoc  Handle.
 *
 * @return The internal structure, or NULL on invalid handle.
 *
 * @retval NULL     @p hDoc is NULL or its magic is wrong.
 * @retval other    A pointer to the live document.
 */
RSP_DOC * APIENTRY rsp_doc_from_handle(HRSPINST hDoc);

/*!
 * @brief Obtain the RSP_REPORT structure from a handle, or NULL.
 *
 * @param[in] hReport  Handle.
 *
 * @return The internal structure, or NULL on invalid handle.
 *
 * @retval NULL     @p hReport is NULL or its magic is wrong.
 * @retval other    A pointer to the live report.
 */
RSP_REPORT * APIENTRY rsp_report_from_handle(HRSPREPORT hReport);

#ifdef __cplusplus
}
#endif

#endif /* RSPINST_PRIVATE_H */
