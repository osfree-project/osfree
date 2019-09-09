/*    
	int_31.c	1.11
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

*	Bios Interrupt 0x31 - DPMI

 */

#include "platform.h"

#include "xdos.h"
#include "BinTypes.h"
#include "Log.h"
#include "DPMI.h"

void
int_31(int nNumber, register ENV *envp)
{
	DWORD dwSize,result;
	int nMajor,nMinor,n;

	nMajor = HIBYTE(LOWORD(envp->reg.ax));
	nMinor = LOBYTE(LOWORD(envp->reg.ax));

	switch (nMajor) {
	    case 0:	/* LDT descriptor management */
		switch (nMinor) {
		    case 0: /* allocate LDT selector range */
			/* issues: selectors have to be present, with base/limit ==0 */
			if (!(result = AssignSelRange(LOWORD(envp->reg.cx)))) {
		    	    envp->reg.flags |= CARRY_FLAG >> 16;
			    break;
			}
			for (n = 0; n < (int)LOWORD(envp->reg.cx); n++)
			    AssignSelector((LPBYTE)-1,(result+n)<<3,
				TRANSFER_DATA,0);
		    	envp->reg.flags &= ~(CARRY_FLAG >> 16);
			envp->reg.ax = ((result<<3) | 7) & 0xffff;
			break;

		    case 1: /* free LDT descriptor */
			result = FreeSelector(LOWORD(envp->reg.bx));
                        if (result)
		    	    envp->reg.flags |= CARRY_FLAG >> 16;
			else
		    	    envp->reg.flags &= ~(CARRY_FLAG >> 16);
			envp->reg.ax = result & 0xffff;
			break;

		    case 2: /* real-mode segment to descriptor */
			envp->reg.flags |= CARRY_FLAG >> 16;
			break;

		    case 3: /* Get AHINCR value */
			envp->reg.flags &= ~(CARRY_FLAG >> 16);
			envp->reg.ax = GetAHINCR();
			break;

		    case 4:
		    case 5:
			envp->reg.flags |= CARRY_FLAG >> 16;
			break;

		    case 6: /* Get segment base address */
			result = GetSelectorBase(LOWORD(envp->reg.bx));
			if (result == 0)
			    envp->reg.flags |= CARRY_FLAG >> 16;
			else {
			    if (result == (DWORD)-1)
				result = 0;
			    envp->reg.cx = HIWORD(result);
			    envp->reg.dx = LOWORD(result);
			    envp->reg.flags &= ~(CARRY_FLAG >> 16);
			}
			break;

		    case 7:	/* Set segment base address */
			result = MAKELONG(LOWORD(envp->reg.dx),LOWORD(envp->reg.cx));
			SetPhysicalAddress(LOWORD(envp->reg.bx),result);
			DPMI_Notify(DN_MODIFY,LOWORD(envp->reg.bx));
			envp->reg.flags &= ~(CARRY_FLAG >> 16);
			break;

		    case 8:	/* Set segment limit */
			dwSize = MAKELONG(LOWORD(envp->reg.dx),LOWORD(envp->reg.cx));
			SetSelectorLimit(LOWORD(envp->reg.bx),dwSize);
			DPMI_Notify(DN_MODIFY,LOWORD(envp->reg.bx));
			envp->reg.flags &= ~(CARRY_FLAG >> 16);
			break;

		    case 9:	/* Set descriptor access rights */
			SetSelectorFlags(LOWORD(envp->reg.bx),LOWORD(envp->reg.cx));
			DPMI_Notify(DN_MODIFY,LOWORD(envp->reg.bx));
			envp->reg.flags &= ~(CARRY_FLAG >> 16);
			break;

		    default:
			envp->reg.flags |= CARRY_FLAG >> 16;
			break;
		}
		break;

	    case 4:	/* Get DPMI version */
		envp->reg.ax = 0x5a;	/* DPMI version 0.90 */
		envp->reg.bx = 5;	/* 386 with virtual memory support */
		envp->reg.cx = 3;	/* 80386 CPU */
		envp->reg.dx = 0;	/* PIC base interrupts??? */
		envp->reg.flags &= ~(CARRY_FLAG >> 16);
		break;

	    case 5:	/* Memory management */
		switch (nMinor) {
		    case 1:	/* Allocate memory block */
			dwSize = MAKELONG(LOWORD(envp->reg.cx),LOWORD(envp->reg.bx));
			result = (DWORD)WinMalloc(dwSize);
			envp->reg.flags &= ~(CARRY_FLAG >> 16);
			envp->reg.bx = envp->reg.si = HIWORD(result);
			envp->reg.cx = envp->reg.di = LOWORD(result);
			break;

		    case 2:	/* Free memory block */
			result = MAKELONG(LOWORD(envp->reg.di),LOWORD(envp->reg.si));
			WinFree((LPSTR)result);
			envp->reg.flags &= ~(CARRY_FLAG >> 16);
			break;

		    case 3:	/* Resize memory block */
			dwSize = MAKELONG(LOWORD(envp->reg.cx),LOWORD(envp->reg.bx));
			if (dwSize == 0) {
			    envp->reg.flags |= CARRY_FLAG >> 16;
			    break;
			}
			result = MAKELONG(LOWORD(envp->reg.di),LOWORD(envp->reg.si));
			result = (DWORD)WinRealloc((LPSTR)result,dwSize);
			envp->reg.flags &= ~(CARRY_FLAG >> 16);
			envp->reg.bx = envp->reg.si = HIWORD(result);
			envp->reg.cx = envp->reg.di = LOWORD(result);
			break;

		    default:
			envp->reg.flags |= CARRY_FLAG >> 16;
			break;
		}
		break;

	    default:
		envp->reg.flags |= CARRY_FLAG >> 16;
		break;
	}
}

