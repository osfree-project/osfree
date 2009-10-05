/*    
	int_10.c	1.8
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

 */

/*******************************************
**
**	Bios Interrupt 0x10 Video BIOS
**
**
********************************************/

#include "platform.h"

#include "xdos.h"
#include "BinTypes.h"
#include "Log.h"

void
int_10(int nNumber, register ENV *envp)
{
    int nMajor,nMinor;

    nMajor = (LOWORD(envp->reg.ax)) >> 8;
    switch (nMajor) {
	case 0x1a:	/* Display combination */
	    nMinor = LOWORD(envp->reg.ax) & 0xff;
	    if (nMinor == 0) {	/* Read display combination */
		envp->reg.ax = 0x1a1a;
		envp->reg.bx = 0x0008; /* Primary VGA w/color display */
	    }
	    break;

	default:
	    break;
    }
}

