/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/

/*!
 * @file omfglbl.h
 * @brief Global macros shared by the OMF reader.
 */

#ifndef OMF_GLOBAL_INCLUDED
#define OMF_GLOBAL_INCLUDED

#include "orlintnl.h"
#include "omftype.h"

/*!
 * @def _ClientRead
 * @brief Read @p b bytes from the current OMF file handle.
 */
#define _ClientRead( a, b )             ((a)->omf_hnd->funcs->read( a->file, b ))
/*!
 * @def _ClientSeek
 * @brief Seek within the current OMF file handle.
 */
#define _ClientSeek( a, b, c )          ((a)->omf_hnd->funcs->seek( a->file, b, c ))
/*!
 * @def _ClientAlloc
 * @brief Allocate memory through the current OMF file handle.
 */
#define _ClientAlloc( a, b )            ((a)->omf_hnd->funcs->alloc( b ))
/*!
 * @def _ClientFree
 * @brief Free memory through the current OMF file handle.
 */
#define _ClientFree( a, b )             ((a)->omf_hnd->funcs->free( b ))
/*!
 * @def _ClientReAlloc
 * @brief Reallocate memory through the current OMF file handle.
 */
#define _ClientReAlloc( a, b, c )       ((a)->omf_hnd->funcs->realloc( b, c ))

/*!
 * @def _ClientSecRead
 * @brief Read @p b bytes from a section's owning OMF file handle.
 */
#define _ClientSecRead( a, b, c )       ((a)->omf_file_hnd->omf_hnd->funcs->read( a->omf_file_hnd->file, b, c ))
/*!
 * @def _ClientSecSeek
 * @brief Seek within a section's owning OMF file handle.
 */
#define _ClientSecSeek( a, b, c )       ((a)->omf_file_hnd->omf_hnd->funcs->seek( a->omf_file_hnd->file, b, c ))
/*!
 * @def _ClientSecAlloc
 * @brief Allocate memory associated with a section.
 */
#define _ClientSecAlloc( a, b )         ((a)->omf_file_hnd->omf_hnd->funcs->alloc( b ))
/*!
 * @def _ClientSecFree
 * @brief Free memory associated with a section.
 */
#define _ClientSecFree( a, b )          ((a)->omf_file_hnd->omf_hnd->funcs->free( b ))

/*!
 * @def ORL_FILE_WORD_SIZE_MASK
 * @brief Mask for the word-size bits of the OMF file flags.
 */
#define ORL_FILE_WORD_SIZE_MASK         ( ORL_FILE_FLAG_16BIT_MACHINE | \
                                          ORL_FILE_FLAG_32BIT_MACHINE | \
                                          ORL_FILE_FLAG_64BIT_MACHINE )
/*!
 * @def _SetWordSize
 * @brief Replace the word-size bits of the OMF file flags.
 */
#define _SetWordSize( a, b )            (a=(a & ~ORL_FILE_WORD_SIZE_MASK) | b)

/*!
 * @def _Is32BitRec
 * @brief Test the low bit of an OMF record type (32-bit variant).
 */
#define _Is32BitRec( a )                (a & 1)

/*!
 * @def _TurnBitOn
 * @brief Set a bit in a bitmap array.
 */
#define _TurnBitOn( map, bit )          (map[bit >> 3] |= (1 << (bit & 0x1f)))
/*!
 * @def _TestBit
 * @brief Test a bit in a bitmap array.
 */
#define _TestBit( map, bit )            (map[bit >> 3] & (1 << (bit & 0x1f)))

#endif
