/*    
	PrintManager.c	2.25
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

#define	PRINTDRIVER
#include "windows.h"
#include "commdlg.h"
#include "print.h"
#include "Driver.h"

#include "kerndef.h"
#include "Kernel.h"
#include "Log.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#ifdef NEEDED
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef	NETWARE
#include <unistd.h>
#endif
#endif /* NEEDED */

typedef struct tagJOBSTRUCT
{
	off_t	lPosition;
	FILE	*hPrnJob;
    char        FileName[128];
	int	hSpoolPage;
	int	iStatus;	/* Not used */
	BOOL	bFlags;
	LPSTR	lpszTitle;
	void *DrvStuff;
} JOBSTRUCT;

typedef JOBSTRUCT *LPJOBSTRUCT;

#define	SPOOL_STREAM		1
#define	SPOOL_PIPESTREAM	2

HPJOB
OpenJob(LPSTR lpOutput, LPSTR lpTitle, HDC hDC)
{
	LPJOBSTRUCT     lpJob;
	HGLOBAL		hJob;
	char cmd[128];

        /* make a grafpt & set up the hprint stuff */
	LOGSTR((LF_API,"OpenJob: Output=%s Title=%s hDC=0x%x\n",
		lpOutput, lpTitle, hDC));

	hJob = GlobalAlloc(GHND, sizeof(JOBSTRUCT));
	if ( !(lpJob = (LPJOBSTRUCT)GlobalLock(hJob)) )
		return (HPJOB)SP_OUTOFMEMORY;

#ifdef macintosh
	lpJob->DrvStuff = (void *) DRVCALL_PRINTER(PPH_STARTDOC, NULL, NULL, NULL);
#ifdef NEVER_COMPILED_CODE
	lpJob->DrvStuff  = (void *)OpenPrinterPort();
#endif
#endif

#ifndef macintosh
	if ( GetProfileString("PortAliases", lpOutput, "", cmd, sizeof(cmd)) ) {
		/* Alias exists: open pipe */
		if ( cmd[0] == '|' )
			cmd[0] = ' ';
		if ( !(lpJob->hPrnJob = (FILE *)popen(cmd, "w")) ) {
			GlobalUnlock(hJob);
			GlobalFree(hJob);
			return (HPJOB)SP_ERROR;
		}
		lpJob->bFlags |= SPOOL_PIPESTREAM;
	}
	else
#endif
		{
		/* No alias: open vanilla stream */
			remove(lpOutput); 			/*Trim the file if one exists */


		if ( !(lpJob->hPrnJob = fopen(lpOutput, "w")) ) {
			GlobalUnlock(hJob);
			GlobalFree(hJob);
			return (HPJOB)SP_ERROR;
		}
		lpJob->bFlags |= SPOOL_STREAM;
	}

	if (lpTitle && *lpTitle) {
		lpJob->lpszTitle = (LPSTR)WinMalloc(strlen(lpTitle)+1);
		strcpy(lpJob->lpszTitle, lpTitle);
	}
	else
		lpJob->lpszTitle = (LPSTR)NULL;
			
	GlobalUnlock(hJob);
	LOGSTR((LF_API,"OpenJob returned hJob = 0x%x (& ~0xC000 hack)\n",
		hJob));
	return (HPJOB)((DWORD)hJob & ~OM_MASK);
}

int
CloseJob(HPJOB hJob)
{
	LPJOBSTRUCT lpJob;
	HGLOBAL hGlobal;

	LOGSTR((LF_API,"CloseJob: hJob = 0x%x (& 0xC000 hack)\n",
		hJob));
	hGlobal = (HGLOBAL)((DWORD)hJob | OM_MASK);
	if ( !(lpJob = (LPJOBSTRUCT)GlobalLock(hGlobal)) )
		return SP_OUTOFMEMORY;

#ifndef	macintosh
	if ( lpJob->bFlags & SPOOL_PIPESTREAM )
		pclose(lpJob->hPrnJob);
	else
#endif
		fclose(lpJob->hPrnJob);

 	if(!strlen(lpJob->FileName)){
		_ldelete(lpJob->FileName);
		strcpy(lpJob->FileName, "");
	}
#ifdef macintosh
	DRVCALL_PRINTER(PPH_ENDDOC, NULL, NULL, lpJob->DrvStuff);
#ifdef NEVER_COMPILED_CODE
	ClosePrinterPort(lpJob->DrvStuff);
#endif
#endif

GlobalFree(hGlobal);
	return TRUE;
}

int
DeleteJob(HPJOB hJob, int iDummy)
{
	LPJOBSTRUCT lpJob;
	HGLOBAL hGlobal;

	LOGSTR((LF_API,"Printer DeleteJob: hJob = 0x%x (& 0xC000 hack)\n",
		hJob));
	hGlobal = (HGLOBAL)((DWORD)hJob | OM_MASK);
	if ( !(lpJob = (LPJOBSTRUCT)GlobalLock(hGlobal)) ) {
		return SP_OUTOFMEMORY;
	}

#ifndef	macintosh
	if ( lpJob->bFlags & SPOOL_PIPESTREAM )
		pclose(lpJob->hPrnJob);
	else
#endif
		fclose(lpJob->hPrnJob);

 	if(!strlen(lpJob->FileName)){
		_ldelete(lpJob->FileName);
		strcpy(lpJob->FileName, "");
	}

#ifdef macintosh
	DRVCALL_PRINTER(PPH_ENDDOC, NULL, NULL, lpJob->DrvStuff);
#ifdef NEVER_COMPILED_CODE
	ClosePrinterPort(lpJob->DrvStuff);
#endif
#endif

	GlobalFree(hGlobal);
	return TRUE;
}

int
StartSpoolPage(HPJOB hJob)
{
	LPJOBSTRUCT lpJob;
	int         rc = TRUE;
	HGLOBAL hGlobal;

	
	LOGSTR((LF_API,
		"Printer StartSpoolPage: hJob = 0x%x (& 0xC000 hack)\n",
		hJob));


	hGlobal = (HGLOBAL)((DWORD)hJob | OM_MASK);
	if ( !(lpJob = (LPJOBSTRUCT)GlobalLock(hGlobal)) ) {
		LOGSTR((LF_API,
			"Printer: hJob = 0x%x (& 0xC000 hack). %s",
			hGlobal, "StartSpoolPage failed\n" ));
		return SP_OUTOFMEMORY;
	}
	
#ifdef TWIN32
		GetTempFileName16(0, "prn", 0, lpJob->FileName);
#else
		GetTempFileName(0, "prn", 0, lpJob->FileName);
#endif

	if ((lpJob->hSpoolPage = _lopen(lpJob->FileName,READ_WRITE)) == -1) {
		LOGSTR((LF_API,
			"Printer: hJob = 0x%x (& 0xC000 hack). %s",
			hGlobal, "StartSpoolPage: open spool file failed\n" ));
		rc = SP_ERROR;
	}

	

	GlobalUnlock(hGlobal);
	return rc;
}

int
WriteSpool(HPJOB hJob, LPSTR lpData, int cch)
{
	LPJOBSTRUCT lpJob;
	int         rc = TRUE;
	HGLOBAL hGlobal;

	LOGSTR((LF_API,
		"WriteSpool: hJob = 0x%x (& 0xC000 hack), bytes = %d\n",
		hJob, cch));
	hGlobal = (HGLOBAL)((DWORD)hJob | OM_MASK);
	if ( !(lpJob = (LPJOBSTRUCT)GlobalLock(hGlobal)) )
		return SP_OUTOFMEMORY;

	if ( _lwrite(lpJob->hSpoolPage, (LPVOID)lpData, cch) != cch )
		rc = SP_OUTOFDISK;

	GlobalUnlock(hGlobal);
	return rc;
}


int
EndSpoolPage(HPJOB hJob)
{
	LPJOBSTRUCT lpJob;
	LPBYTE      lpBuf;
	long        lPos;
	int         cbBytes, rc = TRUE;
	HGLOBAL hGlobal;
	int	fputsVal = 0;

	LOGSTR((LF_API,"EndSpoolPage: hJob = 0x%x (& 0xC000 hack)\n",
		hJob));
	hGlobal = (HGLOBAL)((DWORD)hJob | OM_MASK);


	if ( !(lpJob = (LPJOBSTRUCT)GlobalLock(hGlobal)) ) {
		return SP_OUTOFMEMORY;
	}
	if ( !(lPos = _llseek(lpJob->hSpoolPage, 0, SEEK_END)) ) {
		/* There's nothing in spool page */
		GlobalUnlock(hGlobal);
		return TRUE;
	}
	lPos = min(lPos, (UINT)(-2));
	lpBuf = (LPBYTE)WinMalloc(lPos + 1);
	_llseek(lpJob->hSpoolPage, 0, SEEK_SET);

	do {
		if ( (cbBytes = _lread(lpJob->hSpoolPage, (LPSTR)lpBuf, lPos)) == -1 ) {
			LOGSTR((LF_API,
				"Printer: hJob = 0x%x (& 0xC000 hack) %s",
				hGlobal, 
				"EndSpoolPage: read spool file failed\n" ));
			rc = SP_ERROR;
			break;
		}
		lpBuf[cbBytes] = '\0';		/* Make the string */


	fputsVal = fputs((LPSTR)lpBuf, lpJob->hPrnJob);
		if ( cbBytes  &&  (fputsVal==EOF)) {
			LOGSTR((LF_API,
				"Printer: hJob = 0x%x (& 0xC000 hack) %s",
				hGlobal,
				"WriteSpool: write output file failed\n" ));
			rc = SP_OUTOFDISK;
			break;
		}
		
#ifdef macintosh
		DRVCALL_PRINTER(PPH_SENDDATA, lpBuf, cbBytes, lpJob->DrvStuff);
/*		SendData(lpJob->DrvStuff, lpBuf, cbBytes); */
#endif

	} while ( cbBytes == lPos );

	WinFree((LPSTR)lpBuf);
	_lclose(lpJob->hSpoolPage);
 
	_ldelete(lpJob->FileName);
	strcpy(lpJob->FileName, "");

	lpJob->hSpoolPage = 0;	    /*Invalidate file descriptor */
	GlobalUnlock(hGlobal);
	
	return rc;
}

/*
**	TWIN_CreateDevMode():
**
**	This routine creates an extended DEVMODE record that contains the device-independent
**	elements that are defined by Windows and also the extended elements that are used by
**	the driver.  The driver is called to see how many bytes are needed in it's portion of
**	the extended record.  The caller can provide a pre-existing extended DEVMODE record,
**	which is copied into the new handle.  Otherwise a new handle is created and filled in
**	with default values for the device-independent portion and the driver is called to
**	fill in the other elements.
**
**	TODO:	See if the DEVMODE input parameter matches what the driver is expecting
**			in terms of size and maybe version.
*/
HGLOBAL
TWIN_CreateDevMode( HGLOBAL hDevModeIn )
{
	HGLOBAL		hDevModeOut;
	DEVMODE*	lpDevModeIn;
	DEVMODE*	lpDevModeOut;
	int			cbBytes;

/*
**	See how many bytes the driver portion requires and allocate the handle
*/
	cbBytes = DRVCALL_PRINTER( PPH_GETDEVMODESIZE, NULL, NULL, NULL );
	
	hDevModeOut = GlobalAlloc( GHND, sizeof( DEVMODE ) + cbBytes );
	if ( hDevModeOut == 0 )
		return( (HGLOBAL) 0 );

/*
**	If a devMode was provided, copy the elements into the new handle
*/
	if ( hDevModeIn )
	{
		lpDevModeIn = (DEVMODE* )GlobalLock( hDevModeIn );
		lpDevModeOut = (DEVMODE* )GlobalLock( hDevModeOut );
		memcpy( lpDevModeOut, lpDevModeIn, sizeof( DEVMODE ) + cbBytes );
		GlobalUnlock( hDevModeOut );
		GlobalUnlock( hDevModeIn );
		GlobalFree( hDevModeIn );
	}
	else
	{
/*
**	Fill in the device-independent elements
*/
		lpDevModeOut = (DEVMODE* )GlobalLock( hDevModeOut );
		lpDevModeOut->dmSpecVersion		= 0x30A;
		lpDevModeOut->dmSize			= sizeof( DEVMODE );
		lpDevModeOut->dmDriverExtra		= cbBytes;
		lpDevModeOut->dmFields			= 0x00007F1F;	/* everything initialized */
		lpDevModeOut->dmOrientation		= DMORIENT_PORTRAIT;
		lpDevModeOut->dmPaperSize		= 0;			/* use length and width fields */
		lpDevModeOut->dmScale			= 100;
		lpDevModeOut->dmCopies			= 1;
		lpDevModeOut->dmDefaultSource	= DMBIN_AUTO;
		lpDevModeOut->dmColor			= DMCOLOR_COLOR;
		lpDevModeOut->dmDuplex			= DMDUP_HORIZONTAL;
		lpDevModeOut->dmTTOption		= DMTT_SUBDEV;
/*		lpDevModeOut->dmDeviceName		= Driver should fill this in */
/*		lpDevModeOut->dmDriverVersion	= Driver should fill this in */
/*		lpDevModeOut->dmPaperLength		= Driver should fill this in */
/*		lpDevModeOut->dmPaperWidth		= Driver should fill this in */
/*		lpDevModeOut->dmPrintQuality	= Driver should fill this in */
/*		lpDevModeOut->dmYResolution		= Driver should fill this in */
/*
**	Tell the driver to fill in it's elements of the record
*/
		DRVCALL_PRINTER( PPH_GETDEFAULT, NULL, NULL, lpDevModeOut );
		GlobalUnlock( hDevModeOut );
	}
	return( hDevModeOut );
}


/*
**	TWIN_PrintSetup():
**
**	This routine calls the driver to perform something akin to Windows PrintSetup.
**	The driver is provided a handle to a DEVMODE record that has been created on
**	behalf of the driver in TWIN_CreateDevMode().  This routine is exported and
**	used by the implementation of the COMMDLG DLL.
*/
DWORD
TWIN_PrintSetup( HGLOBAL hDevModeIn )
{
	DEVMODE*	lpDevMode;
	DWORD		result;
	
	lpDevMode = (DEVMODE* )GlobalLock( hDevModeIn );
	result = DRVCALL_PRINTER( PPH_SETUPDLG, NULL, NULL, lpDevMode );
	GlobalUnlock( hDevModeIn );
	return( result );
}


/*
**	TWIN_FillPrintDlg():
**
**	This routine calls down to the driver which converts an internal printer
**	record into the Windows device-independent PRINTDLG record.  Some elements
**	in the PRINTDLG record are not mappable to the driver's elements and should
**	be initialized to some acceptable values by the driver.  This routine is
**	exported and used by the implementation of the COMMDLG DLL.
*/
void
TWIN_FillPrintDlg( PRINTDLG* lpPrintDlg, HGLOBAL hDevModeIn )
{
	DEVMODE*	lpDevMode;
	
	lpDevMode = (DEVMODE* )GlobalLock( hDevModeIn );
	DRVCALL_PRINTER( PPH_FILLPRINTDLG, lpDevMode, NULL, lpPrintDlg );
	GlobalUnlock( hDevModeIn );
}
