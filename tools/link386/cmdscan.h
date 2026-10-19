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
 * @file cmdscan.h
 * @brief Low-level token scanners driven by a context stream.
 */

#ifndef _CMDSCAN_H
#define _CMDSCAN_H


/*!
 * @brief Skip whitespace in the current context.
 */
extern void     CmdScanWhitespace( void );

/*!
 * @brief Recognise a character in either case.
 * @param[in] ch  Character to recognise.
 * @retval 1  Matched and consumed.
 * @retval 0  Not matched.
 */
extern int      CmdScanRecogChar( int ch );
/*!
 * @brief Recognise a character exactly.
 * @param[in] ch  Character to recognise.
 * @retval 1  Matched and consumed.
 * @retval 0  Not matched.
 */
extern int      CmdScanRecogCharExact( int ch );

/*!
 * @brief Scan a (possibly quoted) string.
 * @retval NULL  No string (leading whitespace or EOF).
 * @return Newly allocated string.
 */
extern char *   CmdScanString( void );
/*!
 * @brief Scan a filename with quote processing.
 * @retval NULL  No filename.
 * @return Newly allocated filename.
 */
extern char *   CmdScanFileName( void );
/*!
 * @brief Scan a filename without quote processing.
 * @retval NULL  No filename.
 * @return Newly allocated filename.
 */
extern char *   CmdScanFileNameWithoutQuotes( void );
/*!
 * @brief Scan a number (decimal or 0x-prefixed hexadecimal).
 * @param[out] num  Receives the parsed value.
 * @retval 1  Number parsed.
 * @retval 0  No number.
 */
extern int      CmdScanNumber( unsigned *num );

/*!
 * @brief Non-zero while inside a quoted string.
 */
extern int      Quoted;

#endif
