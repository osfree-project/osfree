/*! omf_private.h - private OMF file handle (C89)
 *
 *  Not for public use. Included only by the OMF implementation files:
 *  omf.c and omf_write.c.
 */

#ifndef OMF_PRIVATE_H
#define OMF_PRIVATE_H

#include <stdio.h>
#include "omf.h"

/*! @file omf_private.h
 *  @brief Private representation of HOMFFILE.
 *
 *  Defines the internal structure behind the opaque HOMFFILE handle
 *  and the accessor used by the implementation files. Neither the
 *  structure nor the accessor is visible to callers of the public
 *  OMF API.
 */

/*! @brief Internal representation behind HOMFFILE.
 *
 *  Not exposed to callers. omf.h declares the handle as HANDLE, so
 *  the layout of this structure may change freely between versions.
 */
struct OMFFILE {
    FILE *fp;       /*!< Underlying file stream. */
    ULONG flMode;   /*!< Access flags used at open time. */
};

/*! @brief Fetch the stream pointer if the handle is valid and has
 *         the expected access mode.
 *
 *  The access mode is compared against the low two bits of
 *  OMFFILE::flMode. A handle opened with OMF_OPEN_READ does not
 *  satisfy a request for OMF_OPEN_WRITE, and vice versa.
 *
 *  @param[in] hFile       Handle from OmfOpen.
 *  @param[in] flExpected  Expected access mode: OMF_OPEN_READ or
 *                         OMF_OPEN_WRITE.
 *
 *  @return The underlying FILE pointer, or NULL if @p hFile is
 *          NULLHANDLE, the access mode does not match, or the
 *          stream is missing.
 *
 *  @see OmfOpen
 */
FILE *omf_get_fp(HOMFFILE hFile, ULONG flExpected);

#endif /* OMF_PRIVATE_H */
