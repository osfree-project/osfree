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
 * @file stub.h
 * @brief Shared header for the multi-CPU stub launcher.
 */

#ifndef _CMDLINE_H
#define _CMDLINE_H


/*
 * Return codes for GetCpuTypeStub.
 */

/*!
 * @def STUB_FOUND_UNKNOWN
 * @brief GetCpuTypeStub() could not identify the host CPU.
 */
#define STUB_FOUND_UNKNOWN      0
/*!
 * @def STUB_FOUND_386
 * @brief Host CPU is 386-class.
 */
#define STUB_FOUND_386          1
/*!
 * @def STUB_FOUND_AXP
 * @brief Host CPU is Alpha (AXP).
 */
#define STUB_FOUND_AXP          2
/*!
 * @def STUB_FOUND_PPC
 * @brief Host CPU is PowerPC.
 */
#define STUB_FOUND_PPC          3


/*!
 * @brief Identify the host CPU family at run time.
 * @retval STUB_FOUND_UNKNOWN  CPU not recognized.
 * @retval STUB_FOUND_386      386-class host.
 * @retval STUB_FOUND_AXP      Alpha host.
 * @retval STUB_FOUND_PPC      PowerPC host.
 * @return One of the STUB_FOUND_* constants.
 */
extern int      GetCpuTypeStub( void );
/*!
 * @brief Spawn a specific CPU-targeted linker executable.
 * @param[in] progname  Executable name (not NULL).
 */
extern void     SpawnProgStub( const char *progname );


#endif
