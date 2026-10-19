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
 * @file omfload.h
 * @brief OMF record decoding and file loading.
 */

#ifndef OMF_LOAD_INCLUDED
#define OMF_LOAD_INCLUDED

#include "omfglbl.h"
#include "omftype.h"

/*!
 * @brief Return the word size for an OMF record.
 * @param[in] is32  Non-zero for a 32-bit record.
 * @return 4 for 32-bit, 2 for 16-bit.
 */
extern int              OmfGetWordSize( int is32 );

/*!
 * @brief Parse a scan-table disassembly directive.
 * @param[in]  buffer  Directive payload. Not NULL.
 * @param[in]  len     Payload length in bytes.
 * @param[out] entry   Receiver for the parsed entry. Not NULL.
 * @retval ORL_OKAY   Parsed successfully.
 * @retval ORL_ERROR  Malformed directive.
 * @return ORL_OKAY on success, ORL_ERROR on failure.
 */
extern orl_return       OmfParseScanTab( omf_bytes buffer, long len,
                                         omf_scan_tab_struct *entry );

/*!
 * @brief Load the whole structure of an OMF file.
 * @param[in,out] ofh  OMF file handle. Not NULL.
 * @retval ORL_OKAY   File parsed.
 * @retval ORL_ERROR  Malformed file or read error.
 * @return ORL_OKAY on success, ORL_ERROR on failure.
 */
orl_return              OmfLoadFileStructure( omf_file_handle ofh );

#endif
