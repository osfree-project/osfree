// Emulation of standard windows function via DOS ones
// for sharing of this code with WIN16 KERNEL.EXE
// List of emulated functions
//   GlobalAlloc
//   GlobalLock
//   GlobalUnlock

#include <malloc.h>
#include <string.h>
#include <dos.h>

#include "winemu.h"

// Emulation of GlobalAlloc. Actually returns segment allocated by int 21h
HGLOBAL GlobalAlloc(WORD flags, DWORD size)
{
	WORD segm;
	WORD s=(size >> 4) + 1;

	__asm
	{
		mov ax,48h
		mov bx, s
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
char far *  GlobalLock(HGLOBAL h)
{
	return MK_FP(h, 0);
}

// Actually, does nothing
BYTE GlobalUnlock(HGLOBAL h)
{
	return 1;
}
