/*    
	int_2f.c	1.10
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*	Multiplex Interrupt 0x2f

 */

#include "platform.h"

#include "xdos.h"
#include "BinTypes.h"
#include "Log.h"

/*
 *	Details from Microsoft MS-DOS Programmers Reference
 *	page 147, plus individual int2f opcodes
 *
 *	For windows, the only one actually detected are
 *		function 10	get SHARE.EXE Installed state
 *				note: winword may require this?
 *		function 15	seen, but we report error
 *	Others were added for completeness, to avoid possible errors
 *	later.
 */	

void
int_2f(int nNumber, ENV *envp)
{
	int func;

	func = HIBYTE(LOWORD(envp->reg.ax));

	switch (func) {
	    case 0x01:	
		/* al = 0x00	get PRINT.EXE Installed State   */
		/* al = 0x01	Add file to queue 		*/
		/* al = 0x02	Remove file from queue 		*/
		/* al = 0x03	Cancel all files from queue     */
		/* al = 0x04	Hold Print Jobs and Get Status  */
		/* al = 0x05	Release Print Jobs		*/
		/* al = 0x06	Get Printer Device		*/

	    case 0x06:		/* Get ASSIGN.COM Installed State */	
		envp->reg.flags |= CARRY_FLAG >> 16;
		envp->reg.ax = 0x000;
		break;

            case 0x10:
                /* return that share IS installed */
		envp->reg.ax |= 0x00ff;
	        break;

	    case 0x11:		/* Get Network Installed State  */
		envp->reg.flags |= CARRY_FLAG >> 16;
                /* return that network IS NOT installed */
		envp->reg.ax = 0x000;
		break;

	    case 0x14:		/* Get NLSFUNC.EXE Installed State  */
		envp->reg.flags |= CARRY_FLAG >> 16;
                /* return that NLSFUNC IS NOT installed */
		envp->reg.ax = 0x000;
		break;

	    case 0x15:	/* god knows what (mscdex requests) */
		envp->reg.flags &= ~(CARRY_FLAG >> 16);
		envp->reg.ax = 0x001;
		break;

	    case 0x16:
		/* al = 0x80	MS-DOS Idle Call */
                /* return that program suspension IS NOT supported */
		envp->reg.ax = 0x001;
		break;

	    case 0x1a:		/* Get ANSI.SYS installed State */
                /* return that ANSI.SYS IS NOT loaded */
		envp->reg.ax = 0x000;
		break;

	    case 0x43:		
		/* al = 0x00	Get HIMEM.SYS Installed State */
		/* al = 0x01	Get HIMEM.SYS Entry Point */
                /* return that HIMEM.SYS IS NOT loaded */
		envp->reg.ax = 0x000;
		break;

	    case 0x48:		
		/* al = 0x00	Get DOSKEY Installed State */
		/* al = 0x10	Read Command Line */
                /* return that DOSKEY.COM IS NOT loaded */
		envp->reg.ax = 0x000;
		break;

	    default:
		envp->reg.flags |= CARRY_FLAG >> 16;
		envp->reg.ax = 0x001;
		break;
	}
}


