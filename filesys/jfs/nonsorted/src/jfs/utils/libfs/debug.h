/* $Id: debug.h,v 1.2 2004/03/21 02:43:17 pasha Exp $ */

/**********************************************************************/
/*   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
/*                                                                    */
/*====================================================================*/
/*  debug.h  - debug header for JFS Utilities                         */
/* -------------------------------------------------------------------*/
/*  DESCRIPTION:                                                      */
/*                                                                    */
/*   Debug info                                                       */
/*                                                                    */
/*                                                                    */
/*====================================================================*/
/*  REVISION HISTORY                                                  */
/*  ------------------------------------------------------------------*/
/**********************************************************************/

#include <stdio.h>

#ifdef TRACE
#define DBG_TRACE(a) {printf a; fflush(stdout);}
#else
#define DBG_TRACE(a) ;
#endif

#ifdef TRACE_IO
#define DBG_IO(a) {printf a; fflush(stderr);}
#else
#define DBG_IO(a) ;
#endif

#ifdef TRACE_ERROR
#define DBG_ERROR(a) {printf a; fflush(stdout);}
#else
#define DBG_ERROR(a) ;
#endif
