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
 * @file omfflhn.h
 * @brief Link/unlink OMF file handles in the OMF handle's list.
 */

#ifndef OMF_FILE_HANDLES_INCLUDED
#define OMF_FILE_HANDLES_INCLUDED

#include "omfglbl.h"

/*!
 * @brief Push a new OMF file handle onto the head of the OMF handle list.
 * @param[in,out] oh   OMF handle. Not NULL.
 * @param[in,out] ofh  File handle to link. Not NULL.
 */
void                    OmfAddFileLinks( omf_handle oh, omf_file_handle ofh );

/*!
 * @brief Unlink an OMF file handle from its OMF handle and free it.
 * @param[in,out] ofh  File handle to remove. Not NULL.
 * @retval ORL_OKAY Always returns ORL_OKAY.
 * @return ORL_OKAY.
 */
orl_return              OmfRemoveFileLinks( omf_file_handle ofh );

#endif
