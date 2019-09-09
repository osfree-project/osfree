/*  
 
 @(#)IOProc.c	1.2 	multimedia file I/O driver
  
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/


#include <windows.h>
#include <mmsystem.h>

/* [XxXxXxXx] BEGIN ******************************************************** */

static LRESULT XXXX_Rename(LPMMIOINFO lpMmIOInfo,
	LPSTR lpszFromFilename, LPSTR lpszToFilename)
{
	return (MMIOERR_FILENOTFOUND);
}

static LRESULT XXXX_Open(LPMMIOINFO lpMmIOInfo,
	LPSTR lpszOpenFilename)
{
	return (MMIOERR_CANNOTOPEN);
}

static LRESULT XXXX_Close(LPMMIOINFO lpMmIOInfo,
	UINT uCloseFlags)
{
	return (MMIOERR_CANNOTCLOSE);
}

static LRESULT XXXX_Read(LPMMIOINFO lpMmIOInfo,
	HPSTR hpBuffer, LONG lBufferSize)
{
	return (-1L);
}

static LRESULT XXXX_Write(LPMMIOINFO lpMmIOInfo,
	HPSTR hpBuffer, LONG lBufferSize)
{
	return (-1L);
}

static LRESULT XXXX_WriteFlush(LPMMIOINFO lpMmIOInfo,
	HPSTR hpBuffer, LONG lBufferSize)
{
	return (-1L);
}

static LRESULT XXXX_Seek(LPMMIOINFO lpMmIOInfo,
	LONG lSeekOffset, int nSeekOrigin)
{
	return (-1L);
}

/* [XxXxXxXx] END ********************************************************** */

LRESULT	CALLBACK
IOProc(LPSTR lpMmIOInfo, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	LPSTR lpszFromFilename, lpszToFilename, lpszOpenFilename;
	UINT uCloseFlags;
	HPSTR hpBuffer;
	LONG lBufferSize, lSeekOffset;
	int nSeekOrigin;

	switch (uMsg)
	{
	case MMIOM_RENAME:
		lpszFromFilename = (LPSTR) lParam1;
		lpszToFilename = (LPSTR) lParam2;
		return XXXX_Rename((LPMMIOINFO) lpMmIOInfo,
			lpszFromFilename, lpszToFilename);
	case MMIOM_OPEN:
		lpszOpenFilename = (LPSTR) lParam1;
		return XXXX_Open((LPMMIOINFO) lpMmIOInfo,
			lpszOpenFilename);
	case MMIOM_CLOSE:
		uCloseFlags = (UINT) lParam1;
		return XXXX_((LPMMIOINFO) lpMmIOInfo,
			uCloseFlags);
	case MMIOM_READ:
		hpBuffer = (HPSTR) lParam1;
		lBufferSize = (LONG) lParam2;
		return XXXX_Read((LPMMIOINFO) lpMmIOInfo,
			hpBuffer, lBufferSize);
	case MMIOM_WRITE:
		hpBuffer = (HPSTR) lParam1;
		lBufferSize = (LONG) lParam2;
		return XXXX_Write((LPMMIOINFO) lpMmIOInfo,
			hpBuffer, lBufferSize);
	case MMIOM_WRITEFLUSH:
		hpBuffer = (HPSTR) lParam1;
		lBufferSize = (LONG) lParam2;
		return XXXX_WriteFlush((LPMMIOINFO) lpMmIOInfo,
			hpBuffer, lBufferSize);
	case MMIOM_SEEK:
		lSeekOffset = (LONG) lParam1;
		nSeekOrigin = (int) lParam2;
		return XXXX_Seek((LPMMIOINFO) lpMmIOInfo,
			lSeekOffset, nSeekOrigin);
	}

	return (0);

}

