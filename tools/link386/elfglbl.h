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
 * @file elfglbl.h
 * @brief Global macros shared by the ELF reader.
 */

#ifndef ELF_GLOBAL_INCLUDED
#define ELF_GLOBAL_INCLUDED

#include "orlintnl.h"
#include "elftype.h"

/*!
 * @def _ClientRead
 * @brief Read @p b bytes from the current ELF file handle.
 */
#define _ClientRead( a, b )             ((a)->elf_hnd->funcs->read( a->file, b ))
/*!
 * @def _ClientSeek
 * @brief Seek within the current ELF file handle.
 */
#define _ClientSeek( a, b, c )          ((a)->elf_hnd->funcs->seek( a->file, b, c ))
/*!
 * @def _ClientAlloc
 * @brief Allocate memory through the current ELF file handle.
 */
#define _ClientAlloc( a, b )            ((a)->elf_hnd->funcs->alloc( b ))
/*!
 * @def _ClientFree
 * @brief Free memory through the current ELF file handle.
 */
#define _ClientFree( a, b )             ((a)->elf_hnd->funcs->free( b ))
/*!
 * @def _ClientReAlloc
 * @brief Reallocate memory through the current ELF file handle.
 */
#define _ClientReAlloc( a, b, c )       ((a)->elf_hnd->funcs->realloc( b, c ))

/*!
 * @def _ClientSecRead
 * @brief Read @p b bytes from a section's owning ELF file handle.
 */
#define _ClientSecRead( a, b, c )       ((a)->elf_file_hnd->elf_hnd->funcs->read( a->elf_file_hnd->file, b, c ))
/*!
 * @def _ClientSecSeek
 * @brief Seek within a section's owning ELF file handle.
 */
#define _ClientSecSeek( a, b, c )       ((a)->elf_file_hnd->elf_hnd->funcs->seek( a->elf_file_hnd->file, b, c ))
/*!
 * @def _ClientSecAlloc
 * @brief Allocate memory associated with a section.
 */
#define _ClientSecAlloc( a, b )         ((a)->elf_file_hnd->elf_hnd->funcs->alloc( b ))
/*!
 * @def _ClientSecFree
 * @brief Free memory associated with a section.
 */
#define _ClientSecFree( a, b )          ((a)->elf_file_hnd->elf_hnd->funcs->free( b ))

#endif
