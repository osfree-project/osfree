/*    
	int_13.c	1.4
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

*   	DOS Interrupt 0x13
 */

#include "windows.h"
#include "kerndef.h"
#include "BinTypes.h"
#include "Kernel.h"
#include "xdos.h"
#include "dosdefn.h"
#include "xdosproc.h"
#include "xtime.h"
#include "Log.h"
#include "Endian.h"
#include "DPMI.h"
#include "dos.h"
#include "mfs_config.h"
#include "mfs_core.h"
#include "mfs_fileio.h"

void
int_13(int nNumber, register ENV *envp)
{
    unsigned long other_flags;
    int		  func;

    /*
    ** We need to preserve the flag bits. Many functions fiddle with
    ** CARRY. A very limited few fiddle with ZERO. We save all the flags
    ** other than CARRY in other_flags. A function that returns info
    ** in the flags register clears out all other flags. Then when
    ** interrupt processing is complete, we OR other_flags in with
    ** the resulting flags value.
    **
    ** There are four cases:
    ** 1) The function does not modify any flags. The OR at the end is
    **    then a no-op
    ** 2) The function modifies CARRY, but returns it clear. The OR at
    **    the end ORs the zero value in the flags register with the saved
    **    flags, therefore restoring all the flags other than CARRY.
    ** 3) The function returns CARRY set. The flags register contains
    **    only the CARRY, which is ORed with the saved flags.
    ** 4) The function fiddles with a flag other than CARRY. In this
    **    case, the code must manipulate regs->flags and other_flags
    **    directly. The easiest way to do this is to set other_flags
    **    to 0 so the OR at the end has no effect.
    */

	other_flags = envp->reg.flags & ~(CARRY_FLAG >> 16);
	func = HIBYTE(LOWORD(envp->reg.ax));

    	switch(func) {
        case 0x08:      /* GET DRIVE PARAMETERS  */
		    /* tell the "right" sort of lies here --tjh */
		    if (LOWORD(envp->reg.dx) & 0x80)
		      envp->reg.ax = 0x07;
		    else
		      envp->reg.ax = 0x01;
	            envp->reg.flags = CARRY_FLAG >> 16;
		    break;
	    default:
		    ERRSTR((LF_ERROR,"INT 13: Unsupported func %x\n",func));
		    envp->reg.ax = 0;
		    envp->reg.flags = CARRY_FLAG >> 16;
		    break;
	    }

    envp->reg.flags |= other_flags;
}
