/*    
	VerIF.c	2.4
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
#include "windows.h"

#include "kerndef.h"
#include "BinTypes.h"
#include "Kernel.h"
#include "Log.h"
#include "Endian.h"
#include "DPMI.h"

void
IT_GETFILERESOURCESIZE(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	BINADDR	lpbin;
	LPSTR	lp1, lp2, lp3;

	lpbin = (BINADDR)GETDWORD(SP+18);
	lp1 = (LPSTR)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+14);
	lp2 = (LPSTR)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+10);
	lp3 = (LPSTR)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	rc = f(lp1, lp2, lp3, GETDWORD(SP+6));
	envp->reg.sp += 3*LP_86 + DWORD_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


void
IT_GETFILERESOURCE(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	BINADDR	lpbin;
	LPBYTE	lp1, lp2, lp3, lp4;

	lpbin = (BINADDR)GETDWORD(SP+26);
	lp1 = (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+22);
	lp2 = (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+18);
	lp3 = (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+6);
	lp4 = (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	rc = f(lp1, lp2, lp3, GETDWORD(SP+14), GETDWORD(SP+10), lp4);
	envp->reg.sp += 4*LP_86 + 2*DWORD_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


void
IT_GETFILEVERSIONINFOSIZE(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	BINADDR	lpbin;
	LPSTR	lp;

	lpbin = (BINADDR)GETDWORD(SP+6);
	lp = (LPSTR)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	rc = f(lp, GETDWORD(SP+10));
	envp->reg.sp += LP_86 + DWORD_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


void
IT_GETFILEVERSIONINFO(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	BINADDR	lpbin;
	LPSTR	lp1, lp2;

	lpbin = (BINADDR)GETDWORD(SP+18);
	lp1= (LPSTR)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+6);
	lp2= (LPSTR)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	rc = f(lp1, GETDWORD(SP+14), GETWORD(SP+10), lp2);
	envp->reg.sp += 2*LP_86 + 2*DWORD_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


void
IT_1UI7LP(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	BINADDR	lpbin;
	LPBYTE	lp1, lp2, lp3, lp4, lp5, lp6, lp7;

	lpbin = (BINADDR)GETDWORD(SP+30);
	lp1= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+26);
	lp2= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+22);
	lp3= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+18);
	lp4= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+14);
	lp5= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+10);
	lp6= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+6);
	lp7= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	rc = f(GETWORD(SP+32), lp1, lp2, lp3, lp4, lp5, lp6, lp7);
	envp->reg.sp += 7*LP_86 + UINT_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}



void
IT_VERLANGUAGENAME(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	BINADDR	lpbin;
	LPSTR	lp;

	lpbin = (BINADDR)GETDWORD(SP+8);
	lp= (LPSTR)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	rc = f(GETWORD(SP+10), lp, GETWORD(SP+4));
	envp->reg.sp += LP_86 + 2*UINT_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


void
IT_VERQUERYVALUE(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	BINADDR	lpbin;
	LPBYTE	lp1, lp2, lp3, lp4;

	lpbin = (BINADDR)GETDWORD(SP+18);
	lp1= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+14);
	lp2= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+10);
	lp3= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	lpbin = (BINADDR)GETDWORD(SP+6);
	lp4= (LPBYTE)GetAddress(HIWORD(lpbin), LOWORD(lpbin));
	rc = f(lp1, lp2, lp3, lp4);
	envp->reg.sp += 4*LP_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}
