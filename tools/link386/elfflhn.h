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
 * @file elfflhn.h
 * @brief Link/unlink ELF file handles in the ELF handle's list.
 */

#ifndef ELF_FLHN_INCLUDED
#define ELF_FLHN_INCLUDED

#include "elfglbl.h"

/*!
 * @brief Push a new ELF file handle onto the head of the ELF handle list.
 * @param[in,out] hnd       ELF handle. Not NULL.
 * @param[in,out] file_hnd  File handle to link. Not NULL.
 */
extern void                     ElfAddFileLinks( elf_handle, elf_file_handle );

/*!
 * @brief Unlink an ELF file handle and free it.
 * @param[in,out] file_hnd  File handle to remove. Not NULL.
 * @return ORL_OKAY.
 */
extern orl_return               ElfRemoveFileLinks( elf_file_handle );

#endif
