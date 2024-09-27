// Emulation of standard windows function via DOS ones
// for sharing of this code with WIN16 KERNEL.EXE
// List of emulated functions
//   GlobalAlloc
//   GlobalLock
//   GlobalUnlock

#include <malloc.h>
#include <string.h>
#include <dos.h>
#include <stdio.h>

#include "winemu.h"

// Emulation of GlobalAlloc. Actually returns segment allocated by int 21h
HGLOBAL WINAPI GlobalAlloc(WORD flags, DWORD size)
{
	WORD segm;
	WORD s=(size >> 4) + 1;

	__asm
	{
		mov ax,4800h
		mov bx, [s]
	}
	Dos3Call;
	  __asm {
		mov segm, ax
	}

	// Zero data
	if (flags & GMEM_ZEROINIT) _fmemset(MK_FP(segm, 0), 0, size);

	return segm;
}

// Produce far pointer from HGLOBAL
char FAR *  WINAPI GlobalLock(HGLOBAL h)
{
	return MK_FP(h, 0);
}

// Actually, does nothing
BYTE WINAPI GlobalUnlock(HGLOBAL h)
{
	return 1;
}
