/*    
	WinNat.c	2.51
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "windows.h"
#include "windowsx.h"

#include "Log.h"
#include "kerndef.h"
#include "ModTable.h"
#include "Driver.h"
#include "compat.h"
#include "mfs_config.h"

#include "WinConfig.h"
#include "Kernel.h"
#include "WinMalloc.h"
#include "Resources.h"
#include "Module.h"

/****************************************************************/
/*								*/
/* imported stuff 						*/
/*								*/
/****************************************************************/

extern MODULETAB TWIN_ModuleTable[256];

extern DWORD InitTwinDriver(DWORD *);
extern void TWIN_InitLSD();
extern BOOL GdiInitDC(void);
extern void InitSysColors(void);
extern void InitFileSystem(void);
extern DWORD TWIN_DrvCallback(DWORD,LPARAM,LPARAM,LPVOID);
extern short TWIN_TerminateCommdlg( void );
extern BOOL InitBinary(LPSTR,UINT);
extern DWORD PrivateInitDriver(DWORD , LPARAM , LPARAM , LPVOID );
extern BOOL TWIN_InitializeBinaryCode(void);
extern void InitTwinLibraries(ENTRYTAB *);
extern HANDLE InternalLoadLibrary(WORD , LPSTR , LPARAM );
extern LPMODULETAB ReadAppDscrTable(LPSTR );
extern BOOL bIsBinaryApp;
extern BOOL SetSystemMetrics(int, int);
extern int DrvGetCommandLine(int , char ***);
extern HINSTANCE    InternalLoadDLL(MODULETAB *);

#ifdef DRVTAB
TWINDRVSUBPROC **DrvEntryTab;
#else
TWINLIBCALLBACKPROC DrvEntry;
#endif

/****************************************************************/
/*								*/
/* exported stuff						*/
/*								*/
/****************************************************************/

/* this is exported to people like MFC */

/* this is exported to get compatibility */
LPARAM GetCompatibilityFlags(UINT);
LPARAM SetCompatibilityFlags(int);

/* these are the major entry/exit points of library */
BOOL   		TWIN_LibExit(int,DWORD);	
LPMODULETAB    	TWIN_LibEntry(char *,ENTRYTAB *,MODULEDSCR *);

int __argc;
char **__argv;

/* this should be moved down to the driver when we understand the MAC... */
static LPSTR SetCommandLine(int, char **);

DWORD TWIN_DriverCaps;

/****************************************************************/
/*								*/
/* implementation 						*/
/*								*/
/****************************************************************/

static BOOL
TWIN_InitSystem(void)
{
    static bInit = FALSE;
    UINT uiCompatibility;
    TWINDRVCONFIG DrvConfig;

    if (!bInit) {
	/*
	 *  Protect against multiple calls and recursive calls.  Various
	 *  API locations can make a call to us, in addition to the
	 *  normal startup path through main().  This is so that the
	 *  API's that require this initialization can be called from
	 *  constructors in the OWL and MFC libraries.  The API's must
	 *  in turn call us, so make sure we only initialize once.
	 */
	bInit = TRUE;
	
	/* Initialize the filesystem code */
	InitFileSystem();

	/* this initializes loadable software devices */
	TWIN_InitLSD();

	/* this gets low-level driver connections etc. */
	uiCompatibility = GetCompatibilityFlags(0);
	DrvConfig.dwDoubleClickTime = GetTwinInt(WCP_DBLCLICK);
	DrvConfig.lpDrvCallback = &TWIN_DrvCallback;

	PrivateInitDriver(MAKELONG(DSUBSYSTEM_INIT,DSUB_INITSYSTEM),
		uiCompatibility,1,&DrvConfig);

	/* set system metrics */
	/* we should set menu height, rather than hardcode at 25 */
	/* The rule of thumb is MULDIV(fontsize,7,4) 	     */
	/* Where fontsize is the actual font size, not points    */
	/* SetSystemMetrics(SM_CYCAPTION,7*12/4); */

        SetSystemMetrics(SM_CXSCREEN,DrvConfig.nScreenWidth);
	SetSystemMetrics(SM_CYSCREEN,DrvConfig.nScreenHeight);
	SetSystemMetrics(SM_CXFULLSCREEN,DrvConfig.nScreenHeight);
	SetSystemMetrics(SM_CYFULLSCREEN,DrvConfig.nScreenHeight-
				GetSystemMetrics(SM_CYCAPTION));

	/* this initializes DC cache */
	GdiInitDC();

	/* this initializes the system color table */
	InitSysColors();

	/* this initializes MFS layer */
	MFS_INIT();

	/* this initializes the binary machinery */
	TWIN_InitializeBinaryCode();
    }
    return bInit;
}

BOOL TWIN_InitDriver()
{
#ifdef DRVTAB
    if ( DrvEntryTab == NULL ) {
	if ( (DrvEntryTab = (TWINDRVSUBPROC **) DriverBootstrap()) == (TWINDRVSUBPROC **)NULL ) {
#else
    if ( DrvEntry == NULL) {
	if ( (DrvEntry = DriverBootstrap()) == NULL) {
#endif
	    return FALSE;
	}

	(void)TWIN_InitSystem();
    }

    return TRUE;
}

int
LoadTwinModDscr(int argc,char **argv, ENTRYTAB *LibInitTab,
		MODULEDSCR *ModDscrApp)
{
	int  rc;
	LPMODULETAB mdt;
	char *lpCmd;

        lpCmd = SetCommandLine(argc,argv);

    	/* Use argv[0] as the module name, rather than hard coded "module" */
    	if(( argv != NULL) &&  (argv[0] != NULL))
	    ModDscrApp->name = argv[0];	

	mdt = TWIN_LibEntry(lpCmd,LibInitTab,ModDscrApp);

        rc = (BOOL)InternalLoadLibrary(ILL_APPL|ILL_DSCR,lpCmd,(LPARAM)&mdt[0]);
	return rc;
}

LPMODULETAB
TWIN_LibEntry( char *lpCmd,ENTRYTAB *LibInitTab, MODULEDSCR *ModDscrApp)
{
    LPMODULETAB mdt;

    if ( !TWIN_InitDriver() )
	return 0;

    TWIN_DriverCaps = PrivateInitDriver(MAKELONG(DSUBSYSTEM_GETCAPS,DSUB_INITDRIVER),0,0,0);

    InitTwinLibraries(LibInitTab);

    TWIN_ModuleTable[0].flags = 0;      	/* flags for module */
    TWIN_ModuleTable[0].dscr = ModDscrApp;	/* ptr to mod descriptor */

    mdt = ReadAppDscrTable(lpCmd);

    return mdt;
}

BOOL
TWIN_LibExit(int bExit,DWORD dwreturn)
{
#ifdef WINMALLOC_CHECK
        logstr(LF_LOG,"Delete Stock Objects\n");
	TWIN_DeleteStockObjects ();

        logstr(LF_LOG,"Dumping GDI\n");
	TWIN_DumpGdiObjects ();
#endif

	PrivateInitDriver(MAKELONG(DSUBSYSTEM_INIT,DSUB_INITSYSTEM),0,0,0);

#ifdef WINMALLOC_CHECK
        logstr(LF_LOG,"Cleanup GlobalAlloc\n");
	TWIN_HandleCleanup();

	/* lets dump our memory chain */
        logstr(LF_LOG,"Check Memory Allocation\n");
#ifdef DRIVERMALLOCS
	(void) DrvMallocInfo(0, 0, 0, 0);
#else
	(void) WinMallocInfo(0, 0, 0, 0);
#endif

	/* lets free our memory chain */
	TWIN_FreeAllMemory();

#endif
    	if(bExit)
		exit(dwreturn);

	return(TRUE);   
}


void WINAPI
FatalExit(int Code)
{
	ExitWindows(Code,0);
}

void
OldExitWindows(void)
{
	ExitWindows(0,0);
}

BOOL WINAPI
ExitWindows(DWORD dwreturn,UINT reserved)
{
	if(reserved == 0) {
#ifdef	LATER
	/* according to definition we have to send WM_QUERYENDSESSION
	   to notify all apps; if all apps agree, send them WM_ENDSESSION
	   before finally shutting down */
#endif
	}
	return TWIN_LibExit(1,dwreturn);	
}

void WINAPI
FatalAppExit(UINT wAction,LPCSTR lpText)
{
    static int flag;

    APISTR((LF_APICALL,"FatalAppExit(UINT=%x,LPCSTR=%s)\n",
	wAction,lpText));

    /* this is in case of a double fault, do not repeat... */
    if(flag && GetTwinInt(WCP_FATAL)) {
	flag++;
	MessageBox(0,lpText,0,MB_ICONSTOP|MB_OK);
    }

    DeleteTask(0);
    APISTR((LF_APIRET,"FatalAppExit: returns void\n"));
}

void
DebugBreak()
{
#if defined(DEBUG) && !defined(TWIN_INTERP_NOT_SUPPORTED)
	extern void debuggerbreak(void);
	debuggerbreak();
#endif
}

WORD
InitApp(HINSTANCE hInstance)
{
    APISTR((LF_APICALL,"InitApp(HINSTANCE=%x)\n",hInstance));
    APISTR((LF_APIRET,"InitApp: returns WORD %d\n",1));
    return 1;
}

WORD
WaitEvent(HANDLE taskID)
{
    APISTR((LF_APICALL,"WaitEvent(HANDLE=%x)\n",taskID));
    APISTR((LF_APIRET,"WaitEvent: returns WORD %d\n",0));
    return 0;
}

static int Compatibility = -1;

LPARAM
GetCompatibilityFlags(UINT uID)
{
    if ( Compatibility < 0 )
	Compatibility = GetTwinInt(WCP_CONTROL);

    return (LPARAM)Compatibility;
}

LPARAM
SetCompatibilityFlags(int NewCompatibility)
{
    int OldCompatibility = GetCompatibilityFlags(0);

    if ( NewCompatibility >= 0 )
	Compatibility = NewCompatibility;

    return (LPARAM)OldCompatibility;
}



static LPSTR
SetCommandLine(int argc, char **argv)
{
    int nCount,nLen;
    LPSTR lpCmd;
    
    argc = DrvGetCommandLine(argc, &argv);

    __argc = argc;
    __argv = argv;

    if( argc == 0 )
    {
      return( NULL );
    }

    for (nCount = 0, nLen = 0; nCount < __argc; nCount ++)
	nLen += strlen(__argv[nCount]) + 1;

    lpCmd = WinMalloc(nLen + 1);
    if (__argv[0])
    	strcpy(lpCmd, __argv[0]);
    else
    	lpCmd[0] = '\0';

    if (__argc > 1) {
	for (nCount = 1; nCount < __argc; nCount++) {
	    strcat(lpCmd, " ");
	    strcat(lpCmd, __argv[nCount]);
	}
    }

    return lpCmd;
}

LPMODULETAB
ReadAppDscrTable(LPSTR lpCmd)
{
    HANDLE hModule,hTask;
    LPSTR lpCmdLine;
    LPMODULETAB mdt;
    register int i;

    if( lpCmd == NULL )
    {
      lpCmdLine = "";
    }
    else
    {
      lpCmdLine = (LPSTR)strchr(lpCmd,' ');
      if (lpCmdLine) lpCmdLine++;
      else lpCmdLine = "";
    }

    mdt = &TWIN_ModuleTable[0];
    hTask = InitializeNativeTask(lpCmdLine);

    if (bIsBinaryApp) {
        lpCmdLine = (LPSTR)strchr(lpCmdLine,' ');
        if (lpCmdLine)
            lpCmdLine++;
        else
            lpCmdLine = "";
        InitBinary(lpCmdLine,0);        /* bInitBinary gets set here    */
    }

    for (i=1; mdt[i].dscr; i++) {
        /* check if module should be pre-loaded */
        if (!(mdt[i].flags & MODULE_NO_LOAD))
            hModule = InternalLoadLibrary(ILL_DSCR,lpCmdLine,(LPARAM)&mdt[i]);
    }

    /* set flag to internalloadlibrary to not defer loading...*/
    InternalLoadDLL(0);

    return mdt;
}

