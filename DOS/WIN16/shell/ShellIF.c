/*    
	ShellIF.c	2.11
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define NOAPIPROTO
#include "windows.h"
#include "Willows.h"

#include "WinDefs.h"
#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "Resources.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "ModTable.h"

long int
IT_EXTRACTASSICON (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HWND  hwnd;
	WORD  wValue;

	hwnd = GETWORD(SP+6);
	wValue =   GETWORD(SP+4);

        retcode = (f)(hwnd,wValue);

	envp->reg.sp += 2*WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	return retcode;
}

long int
ExtractAssociatedIcon(/*HWND hWnd, WORD wIndex*/)
{
	return 0;
}

long int
IT_DOSENVSUBST(ENV *envp,LONGPROC f)
{
	/* do I need to fixup the stack?  looking at the function call */
	/* it does not look like I should, ie. there are already pops  */
	/* to match what was pushed. */
	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = 0;
	return 0;
}

long int
DosEnvSubst()
{
	return 0;
}

extern long int RegCreateKey();
extern long int RegOpenKey();
extern long int RegCloseKey();
extern long int RegDeleteKey();
extern long int RegEnumKey();
extern long int RegQueryValue();
extern long int RegSetValue();
extern long int DragAcceptFiles();
extern long int RegisterShellHook();
extern long int ExtractIcon();
extern long int ShellExecute();
extern long int ShellAbout();
extern long int FindExecutable();

extern long int IT_1H1LP1I();
extern long int IT_1H2LP1H();
extern long int IT_1H1I();
extern long int IT_1D();
extern long int IT_1D1LP();
extern long int IT_1D1LP1LPD();
extern long int IT_1D2LP1LPD();
extern long int IT_1D1LP1D1LP1D();
extern long int IT_2D1LP1D();
extern long int IT_SHELLEXECUTE();
extern long int IT_FINDEXECUTABLE();

extern long int Trap();

 /* Interface Segment Image SHELL: */

static long int (*seg_image_SHELL_0[])() =
{	/* nil */	0, 0,
	/* 001 */	IT_1D1LP1LPD, RegOpenKey,
	/* 002 */	IT_1D1LP1LPD,  RegCreateKey,
	/* 003 */	IT_1D, RegCloseKey,
	/* 004 */	IT_1D1LP, RegDeleteKey,
	/* 005 */	IT_1D1LP1D1LP1D, RegSetValue,
	/* 006 */	IT_1D2LP1LPD, RegQueryValue,
	/* 007 */	IT_2D1LP1D, RegEnumKey,
	/* 008 */	Trap, 0,
	/* 009 */	IT_1H1I,  DragAcceptFiles,
	/* 00a */	Trap, 0,
	/* 00b */	Trap, 0,
	/* 00c */	Trap, 0,
	/* 00d */	Trap, 0,
	/* 00e */	Trap, 0,
	/* 00f */	Trap, 0,
	/* 010 */	Trap, 0,
	/* 011 */	Trap, 0,
	/* 012 */	Trap, 0,
	/* 013 */	Trap, 0,
	/* 014 */	IT_SHELLEXECUTE, ShellExecute,
	/* 015 */	IT_FINDEXECUTABLE, FindExecutable,
	/* 016 */	IT_1H2LP1H,  ShellAbout,
	/* 017 */	Trap, 0,
	/* 018 */	Trap, 0,
	/* 019 */	Trap, 0,
	/* 01a */	Trap, 0,
	/* 01b */	Trap, 0,
	/* 01c */	Trap, 0,
	/* 01d */	Trap, 0,
	/* 01e */	Trap, 0,
	/* 01f */	Trap, 0,
	/* 020 */	Trap, 0,
	/* 021 */	Trap, 0,
	/* 022 */       IT_1H1LP1I, ExtractIcon,
	/* 023 */	Trap, 0,
	/* 024 */	IT_EXTRACTASSICON, ExtractAssociatedIcon,
	/* 025 */	IT_DOSENVSUBST, DosEnvSubst,
	/* 026 */	Trap, 0,
	/* 027 */	Trap, 0,
	/* 028 */	Trap, 0,
	/* 029 */	Trap, 0,
	/* 02a */	Trap, 0,
	/* 02b */	Trap, 0,
	/* 02c */	Trap, 0,
	/* 02d */	Trap, 0,
	/* 02e */	Trap, 0,
	/* 02f */	Trap, 0,
	/* 030 */	Trap, 0,
	/* 031 */	Trap, 0,
	/* 032 */	Trap, 0,
	/* 033 */	Trap, 0,
	/* 034 */	Trap, 0,
	/* 035 */	Trap, 0,
	/* 036 */	Trap, 0,
	/* 037 */	Trap, 0,
	/* 038 */	Trap, 0,
	/* 039 */	Trap, 0,
	/* 03a */	Trap, 0,
	/* 03b */	Trap, 0,
	/* 03c */	Trap, 0,
	/* 03d */	Trap, 0,
	/* 03e */	Trap, 0,
	/* 03f */	Trap, 0,
	/* 040 */	Trap, 0,
	/* 041 */	Trap, 0,
	/* 042 */	Trap, 0,
	/* 043 */	Trap, 0,
	/* 044 */	Trap, 0,
	/* 045 */	Trap, 0,
	/* 046 */	Trap, 0,
	/* 047 */	Trap, 0,
	/* 048 */	Trap, 0,
	/* 049 */	Trap, 0,
	/* 04a */	Trap, 0,
	/* 04b */	Trap, 0,
	/* 04c */	Trap, 0,
	/* 04d */	Trap, 0,
	/* 04e */	Trap, 0,
	/* 04f */	Trap, 0,
	/* 050 */	Trap, 0,
	/* 051 */	Trap, 0,
	/* 052 */	Trap, 0,
	/* 053 */	Trap, 0,
	/* 054 */	Trap, 0,
	/* 055 */	Trap, 0,
	/* 056 */	Trap, 0,
	/* 057 */	Trap, 0,
	/* 058 */	Trap, 0,
	/* 059 */	Trap, 0,
	/* 05a */	Trap, 0,
	/* 05b */	Trap, 0,
	/* 05c */	Trap, 0,
	/* 05d */	Trap, 0,
	/* 05e */	Trap, 0,
	/* 05f */	Trap, 0,
	/* 060 */	Trap, 0,
	/* 061 */	Trap, 0,
	/* 062 */	Trap, 0,
	/* 063 */	Trap, 0,
	/* 064 */	Trap, 0,
	/* 065 */	Trap, 0,
	/* 066 */	IT_1H1I, RegisterShellHook,
	/* 067 */	Trap, 0,
	0,0
};

 /* Segment Table SHELL: */

SEGTAB SegmentTableSHELL[] =
{	{ (char *) seg_image_SHELL_0, 272, TRANSFER_CALLBACK, 272, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

long int
Shell_LibMain()
{
        return(0);
}

