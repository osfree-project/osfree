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
 * @file omfdrctv.h
 * @brief Declaration of the OMF comment parser.
 */

#ifndef OMF_DIRECTIVE_INCLUDED
#define OMF_DIRECTIVE_INCLUDED

#include "omfglbl.h"
#include "omftype.h"

/*!
 * @brief Parse the comment records of an OMF note section.
 * @param[in] sh      Note section handle. Not NULL.
 * @param[in] cb      Note callback table. Not NULL.
 * @param[in] cookie  Callback cookie.
 * @retval ORL_OKAY   All comments parsed.
 * @return The first callback error, or ORL_OKAY.
 */
orl_return      OmfParseComments(omf_sec_handle, orl_note_callbacks *, void *);

#endif
