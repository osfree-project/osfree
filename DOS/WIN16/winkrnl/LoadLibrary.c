/*    
	LoadLibrary.c	2.51
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
#include <string.h>

#include "windows.h"

#include "Endian.h"
#include "dos.h"
#include "Log.h"
#include "kerndef.h"
#include "Resources.h"
#include "Module.h"
#include "Kernel.h"
#include "BinTypes.h"
#include "WinConfig.h"
#include "WinDefs.h"
#include <ctype.h>
#include "compat.h"
#include "Driver.h"

extern HMODULE CreateModuleInstance(LPSTR);
extern void FreeModuleInstance(HMODULE);
extern TYPEINFO *ReadResourceTable(HINSTANCE,USERDLL *);
extern LPSTR strpbrkr(LPCSTR,LPCSTR);
extern BOOL InitBinary(LPSTR,UINT);

extern BOOL InitClasses(void);

/* From LoadDLL.c */
extern int LoadNewInstance(MODULEINFO *, LPSTR, WORD);
extern void LoadModuleFromDscr(MODULEINFO *,LPMODULETAB,WORD);
extern int  LoadModuleFromFile(MODULEINFO *,LPSTR,LPSTR,WORD);
extern int GetCompatibilityFlags(int);

extern BOOL TWIN_InitDriver();


#ifdef STRICT
typedef FARPROC (CALLBACK* GETPADDRPROC)(LPMODULEINFO, UINT);
#else
typedef FARPROC GETPADDRPROC;
#endif

GETPADDRPROC lpfnOEMGetPAddr = NULL;

HANDLE       InternalLoadLibrary(WORD,LPSTR,LPARAM);
#ifdef TWIN32
DWORD          GetModuleFileName(HMODULE,LPTSTR,DWORD);
#else
int          GetModuleFileName(HINSTANCE,LPSTR,int);
#endif
FARPROC      GetProcAddress(HINSTANCE,LPCSTR);
HINSTANCE    InternalLoadDLL(MODULETAB *);
static DWORD  InternalLoadNativeDLL(LPCSTR);

static WORD nCmdShow = SW_SHOWNORMAL;

extern LPOBJHEAD lpModuleHead;
extern MODULETAB TWIN_ModuleTable[];


LPINITT
WindowsInitTask(LPINITT InitStruct)
{
    TASKINFO *lpTaskInfo;
    HANDLE hTask;

    hTask = GetCurrentTask();
    if((lpTaskInfo = (TASKINFO *)GETTASKINFO(hTask))) {
	InitStruct->lpCmdLine = lpTaskInfo->lpCmdLine;
	InitStruct->hInstance = lpTaskInfo->hInst;
    }
    InitStruct->hPrevInstance = 0;
    InitStruct->nCmdShow = nCmdShow;
    RELEASETASKINFO(lpTaskInfo);
    return InitStruct;
}

/*
 * 	According to documentation...
 *	If we have a fully qualified name, then load it, otherwise...
 *
 *	We first look in the current directory, then
 *	We look in the WindowsDirectory, then 
 *	We look in the WindowsSystemDirectory, then
 *	We look in the directory containing the current executable, then
 *	We look in the $PATH, then
 *	We look in directories mapped in a network
 * 
 *	Note: This has nothing to do with WINDIR.  WINDIR is only used to
 *	provide an alternate location for the Windows Directory.
 */

HINSTANCE
LoadLibrary(LPCSTR lpszLibFileName)
{
    HANDLE hModule = 0;
    HINSTANCE hInst;
    char szModuleName[80];

    APISTR((LF_APICALL,"LoadLibrary(%s)\n",lpszLibFileName?lpszLibFileName:"NULL"));
    if (!strlen(lpszLibFileName)) {
	APISTR((LF_APIFAIL,"LoadLibrary: returns HINSTANCE %x\n",0)); 
	return (HINSTANCE)0;
    }

    _splitpath(lpszLibFileName,NULL,NULL,szModuleName,NULL);

    /* special case, this is a builtin module... */
    /* internally, it is called KERNEL, but this is the same... */
    if (!strcmp(szModuleName,"KRNL386")) {	/* KERNEL on 386 CPU */
	hInst =  GetInstanceFromModule(GetModuleHandle("KERNEL"));
        APISTR((LF_APIFAIL,"LoadLibrary: returns HINSTANCE %x\n",hInst)); 
        return hInst;
    }

    /* try to find the .dll that corresponds with the library */
    hModule = InternalLoadLibrary(ILL_NOERROR|ILL_NEWINST,"",
			      (LPARAM)lpszLibFileName);

    /* what is the data instance we will use for it? */
    hInst = GetInstanceFromModule(hModule);

    APISTR((LF_APIRET,"LoadLibrary: returns HINSTANCE %x\n",hInst)); 
    return hInst;
}

HINSTANCE
InternalLoadDLL(MODULETAB *lpModTable)
{
	int n;
	static int bLoadImmediate;

	TWIN_InitDriver();

	if(lpModTable == 0) {
		bLoadImmediate++;
		return 0;
	}
	if(bLoadImmediate) {
        	HMODULE hModule;
		HINSTANCE hInstance;

    		hModule = InternalLoadLibrary(ILL_NEWINST|ILL_DSCR,"",(LPARAM)lpModTable);
		hInstance = GetInstanceFromModule(hModule);

		return hInstance;
	}

	for(n=1;n<256;n++) {

		if(TWIN_ModuleTable[n].flags == 0) {

			TWIN_ModuleTable[n].flags = lpModTable->flags;
			TWIN_ModuleTable[n].dscr = lpModTable->dscr;
			break;
		}
	}

	return 0;
}

static DWORD
InternalLoadNativeDLL(LPCSTR lpszLibFileName)
{
	DWORD	hSO;

	hSO =  DRVCALL_SYSTEM(PSSH_LOADLIBRARY,0,0,lpszLibFileName);
	/*printf("InternalLoadNativeDL(%s) = %x\n",lpszLibFileName,hSO); */
	return hSO;
}

void WINAPI
FreeLibrary(HINSTANCE hInst)
{
    HMODULE hModule;
    MODULEINFO *modinfo;

    APISTR((LF_APICALL,"FreeLibrary(HINSTANCE=%x)\n",hInst));

    if (!(modinfo = GETMODULEINFO(hInst))) {
	hModule = GetModuleFromInstance(hInst);
	if (!(modinfo = GETMODULEINFO(hModule))) {
	    APISTR((LF_APIFAIL,"FreeLibrary: returns void\n"));
	    return;
	}
    }

#ifdef LATER
    decrement reference count, if NULL -- free module
#endif

    RELEASEMODULEINFO(modinfo);

    APISTR((LF_APIRET,"FreeLibrary: returns void\n"));
}

HANDLE
InternalLoadLibrary(WORD flags, LPSTR lpCmdLine, LPARAM lParam)
{
    HTASK hTask;
    LPMODULETAB mod_tab = 0,mdt;
    LPMODULEDSCR mod_dscr = 0;
    char szModuleName[80];
    char FileName[_MAX_PATH];
    LPSTR lpszFileName = 0, lpszModuleName;
    LPSTR ptr;
    HMODULE hModule;
    LPMODULEINFO modinfo = 0;
    HINSTANCE hPrevInst=0;
    TASKINFO *lpTaskInfo;
    int nCompatibility;
    int ret;
    DWORD hSO;

    hTask = GetCurrentTask();

    /* are we passed a descriptor or a filename... */
    if (flags & ILL_DSCR) {

	/* we have a module descriptor table */
	mod_tab = (LPMODULETAB)lParam;
	mod_dscr = (LPMODULEDSCR)mod_tab->dscr;

	/* what is the module name ? */
	lpszModuleName = mod_dscr->name;

	/* if its loaded already, return its hModule */
	if ((hModule = GetModuleHandle(lpszModuleName))) {
	    /* printf("%s already loaded\n",lpszModuleName); */
	    return hModule;
	}
    }
    else { 

	/* we have a filename... */
	lpszFileName = (LPSTR)lParam;

	/* get just the component part... */
	_splitpath(lpszFileName,NULL,NULL,szModuleName,NULL);

	/* check if we have already loaded it. */
	hModule = GetModuleHandle(szModuleName);
	if(hModule) {
#if 0
		/* we have, so now bump ref count, how? */
	    	/* printf("%s already loaded %x\n",szModuleName,hModule); */
		return hModule;
#else
		/*
		* Find if module descriptor is valid.
		* If valid then return handle, else reload module.
		* For future, we should somehow bump ref count.
		*/

		int j = 0;

		mdt = &TWIN_ModuleTable[0];
		while(mdt[++j].dscr)
		if (!strcasecmp(mdt[j].dscr->name,szModuleName))
		{
			mod_dscr = mdt[j].dscr;
			mod_tab = &mdt[j];
			/* printf("%s already loaded %x\n",szModuleName,hModule); */
			return hModule;
		}
	    	/* printf("%s reloaded %x\n",szModuleName,hModule); */
#endif
	}

	/********************************************************/
	/*  Step 1. Alias substitution				*/
	/* 	    if we have an alias for the file, use it    */
	/*	    this allows us to substitute binary dll's   */
	/* 	    in place of native dll's.			*/
	/*	    AND native dll's in place of explicit files */
	/********************************************************/

	if ((GetPrivateProfileString("boot",lpszFileName,"",FileName,
		sizeof(FileName), GetTwinFilename())))
	    lpszFileName = FileName;

	/********************************************************/
	/*  Step 2. Try to load a native version of the file.  	*/ 
	/*	    This can(should) be just a module reference */
	/*	    ie. commdlg, rather than commdlg.dll	*/
	/********************************************************/

	if ((hSO = InternalLoadNativeDLL(lpszFileName))) {
                
		/* we loaded a native library, so return its handle */
		hModule = GetModuleHandle(szModuleName);
		if(hModule == 0) {
			hModule = CreateModuleInstance(szModuleName);
			modinfo = GETMODULEINFO(hModule);
		}
		nCompatibility = GetCompatibilityFlags(0);

		return hModule;
	} 

	if ( !(GetCompatibilityFlags(0) & WD_NOCHANGECASE) ) {
	    for (ptr = szModuleName; *ptr; ptr++)
		*ptr = toupper(*ptr);
	}

	lpszModuleName = szModuleName;

	/********************************************************/
	/* Step 3. See if the module has already been loaded    */
	/* 	   This can be binary or native			*/
	/********************************************************/
	   
	if ((hModule = GetModuleHandle(lpszModuleName))) {
	    modinfo = GETMODULEINFO(hModule);

	    /* If modinfo->wSelBase is not set => */
	    /* it means that we returned to already loaded module to */
	    /* support.					  	     */

	    if (!(!modinfo->wSelBase ) &&
		(!(flags & ILL_NEWINST) || !modinfo->fMakeInstance))
	    {
	        RELEASEMODULEINFO(modinfo);
		return hModule;
	    }
	}

	/********************************************************/
    	/* Module alias check.					*/
    	/* e.g COMMDLG.DRV = COMMGLG.DRV			*/
    	/* Even if native module dscr is built-in, 		*/
	/*							*/
	/* NOTE: this refers to internal dll's, and NOT to      */
	/* those dll's which we have native shared libraries    */
	/* At one point, the library was monolithic, and all    */
	/* dll's were internal, including winsock, shell, and   */
	/* commdlg.  The purpose of this code is to allow real  */
	/* binary dll's to replace our remaining internal dll's */
	/* 							*/
	/* Use the above alias to use binary vs. native dll's   */
	/* Use this to use binary vs. built-in dll's		*/
	/* (see ModTable.c to see remaining built-in dll;s	*/
	/********************************************************/

	if (GetPrivateProfileString("boot",lpszModuleName,"",FileName,
			sizeof(FileName), GetTwinFilename())) 
	    lpszFileName = FileName;

	else { /* Try to find a descriptor in mdt */
	    int j = 0;

	    mdt = &TWIN_ModuleTable[0];
	    while(mdt[++j].dscr)
		if (!strcasecmp(mdt[j].dscr->name,lpszModuleName)) {
		    mod_dscr = mdt[j].dscr;
		    mod_tab = &mdt[j];
		    break;
		}
	}
    }

    /* if we don't have a module descriptor yet, prepare to load */
    if (!mod_dscr) {
        /* we don't have a module descriptor, so we must have a file? */
	InitBinary(lpCmdLine,1); 
    }

    /* if no hModule yet, create one for this module/filename... */
    if (!hModule) {
	hModule = CreateModuleInstance(lpszModuleName);
	modinfo = GETMODULEINFO(hModule);
    }

    /* we have a module descriptor */

    if (modinfo->fMakeInstance) {
	LoadNewInstance(modinfo, lpCmdLine, flags);
    }
    else if (mod_dscr) {
    	LONGPROC init_func;
	
	LoadModuleFromDscr(modinfo,mod_tab,flags);

	if (mod_dscr->entry_table) 
	    init_func = (LONGPROC)mod_dscr->entry_table[0].fun;
	else
	    init_func = 0;

	if (flags & ILL_APPL) {
 	    lpTaskInfo = (TASKINFO *)GETTASKINFO(hTask);
	    lpTaskInfo->hInst = modinfo->hInst;
	    (void) InitClasses();
	    if (!InitWindows()) {
		FatalAppExit(0,"Cannot initialize window manager\n");
	    }

	    /* Init routine is WinMain */
	    if (init_func) 
		init_func(modinfo->hInst, hPrevInst, lpCmdLine, nCmdShow);
	}
	else {
	    /* Init routine is LibMain */
	    if (init_func)
		init_func(modinfo->hInst, 0, 0, lpCmdLine);
	}
    }
    else {	/* no dscr, just name */

	/* add the file extension for a .exe vs. a .dll */
  	/* check here if in fact we should be doing .so */
	if (lpszFileName && strchr(lpszFileName,'.')) { /* filename w/ext. */
	    strcpy(FileName,lpszFileName);
	}
	else {
	    strcpy(FileName,modinfo->lpModuleName);
	    if (flags & ILL_APPL)
		strcat(FileName,".exe");
	    else
		strcat(FileName,".dll");
	}
	if (!(ret = LoadModuleFromFile(modinfo,FileName,lpCmdLine,flags)))
	    FreeModuleInstance(hModule);

	RELEASEMODULEINFO(modinfo);
	return (HMODULE)ret;
    }
    RELEASEMODULEINFO(modinfo);
    return hModule;
}

void
InitTwinLibraries(ENTRYTAB *lpLibInitTable)
{
    /*
     *  Systems which do not support automatically running
     *  init code before calling main will provide initialization
     *  entry points for linked native DLLs.  We run them from
     *  here;  they will setup the module description tables, and
     *  call each library's LibMain, if needed.
     */
    if(lpLibInitTable == 0)
	return;

    while (lpLibInitTable->name != NULL) {
	(*(lpLibInitTable->fun))();
	lpLibInitTable++;
    }
}


DWORD
TWIN_GetModuleFileName(HMODULE hInstance,LPTSTR lpszFileName, DWORD cbFileName,int flag)
{
    HMODULE hModule;
    LPMODULEINFO lpModuleInfo;
    int len;

    if (!(hModule = GetModuleFromInstance(hInstance)))
	hModule = hInstance;
    if (!(lpModuleInfo = GETMODULEINFO(hModule)))
	return 0;
    if (lpModuleInfo->lpFileName) {
	len = strlen(strncpy(lpszFileName,lpModuleInfo->lpFileName,cbFileName));
    } else {

	if (!strcasecmp(lpModuleInfo->lpModuleName,"COMMDLG"))
	    len = strlen(strncpy(lpszFileName,"commdlg.dll",cbFileName));
	else {
	    if(flag & 1) {
		    GetCurrentDirectory(cbFileName,lpszFileName);
		    strcat(lpszFileName,"/");
		    strcat(lpszFileName,lpModuleInfo->lpModuleName);
	    } else 
		    strcpy(lpszFileName,lpModuleInfo->lpModuleName);
	    len = strlen(lpszFileName);
	}
    }
    RELEASEMODULEINFO(lpModuleInfo);
    return len;
}
#ifdef TWIN32
DWORD
GetModuleFileName(HMODULE hInstance,LPTSTR lpszFileName, DWORD cbFileName)
#else
int
GetModuleFileName(HINSTANCE hInstance,LPSTR lpszFileName, int cbFileName)
#endif
{
	return TWIN_GetModuleFileName(hInstance,lpszFileName, cbFileName,1);
}

FARPROC WINAPI
GetProcAddress(HINSTANCE hInstance, LPCSTR lpszProc)
{
    HANDLE hModule;
    LPMODULEINFO lpModuleInfo;
    ENTRYTAB *lpEntry;
    int i;
    FARPROC retFunc = 0;

    if(hInstance == 0)
       hInstance = GetInstanceFromModule(GetModuleHandle("USER"));

    if (!(lpModuleInfo = GETMODULEINFO(hInstance))) {
	if (!(hModule = GetModuleFromInstance(hInstance)))
	    return (FARPROC)NULL;
	if (!(lpModuleInfo = GETMODULEINFO(hModule)))
	    return (FARPROC)NULL;
    }
    if (!HIWORD((DWORD)lpszProc)) {
	i = LOWORD((DWORD)lpszProc);
	lpEntry = &lpModuleInfo->lpEntryTable[i];
	retFunc =  (FARPROC)lpEntry->fun;
	RELEASEMODULEINFO(lpModuleInfo);
	if (!retFunc && lpfnOEMGetPAddr)
	    return (FARPROC)lpfnOEMGetPAddr(lpModuleInfo,i);
	else
	    return retFunc;
    }
    else {	/* search by name */
	for(i=0,lpEntry = lpModuleInfo->lpEntryTable;
	    lpEntry->name;
	    i++,lpEntry++) 
	    if (!strcasecmp(lpszProc,lpEntry->name)) { 
		    retFunc =  (FARPROC)lpEntry->fun;
		    RELEASEMODULEINFO(lpModuleInfo);
		    if (!retFunc && lpfnOEMGetPAddr)
			return (FARPROC)lpfnOEMGetPAddr(lpModuleInfo,i);
		    else
			return retFunc;
	    }
    }
    RELEASEMODULEINFO(lpModuleInfo);
    return NULL;
}

HMODULE
GetModuleHandle(LPCSTR lpszModule)
{
    MODULEINFO *mi;
    ATOM atom;
    LPOBJHEAD lpModuleTemp;
    LPSTR lp;
    LPSTR lpModName;
    LPSTR lpStr;

    if (!HIWORD(lpszModule)) {
	return GetModuleFromInstance((HINSTANCE)LOWORD((DWORD)lpszModule));
    }

    /* get a copy of the string, we are going to modify it */
    lpStr = WinMalloc(strlen(lpszModule)+1);
    strcpy(lpStr,lpszModule);

    /* extract the filename portion */
    if ((lpModName = strrchr(lpStr,'/'))) {
	lpModName++;
    } else
	lpModName = lpStr;

    /* drop off any extensions */
    if ((lp = strchr(lpModName,'.')))	/* strip off possible extension */
	*lp = '\0';

    if (!(atom = FindAtom(lpModName))) {
	WinFree(lpStr);
	return 0;
    }

    /* printf("GetModuleHandle(%s) %x\n",lpModName,atom); */
    /* lpModName points to our internal copy */

    for (lpModuleTemp = lpModuleHead; lpModuleTemp;
	lpModuleTemp = (LPOBJHEAD)lpModuleTemp->lpObjNext) {
	mi = (LPMODULEINFO)lpModuleTemp;

    	/* printf("checking ModuleName %s %x\n",
		mi->lpModuleName,mi->atmModuleName); */

	if (mi->atmModuleName == atom) {
	    WinFree(lpStr);
	    return lpModuleTemp->hObj;
        }
    }
    /* can nolonger use the string */
    WinFree(lpStr);
    return 0;
}

HINSTANCE 
LoadModule(LPCSTR lpszModuleName, LPVOID lpvParamBlock)
{
    HMODULE hModule = 0;
    PARAMBLOCK *lpb;

    lpb = (PARAMBLOCK *)lpvParamBlock;

    hModule = InternalLoadLibrary(ILL_NOERROR|ILL_EXEC|ILL_NEWINST,
				  lpb->lpszCmdLine,
				  (LPARAM)lpszModuleName);
    return GetInstanceFromModule(hModule);
}

HDRVR
GetNextDriver(HDRVR hDrvr, DWORD fdwFlags)
{
    APISTR((LF_APISTUB,"GetNextDriver(HDRVR=%x,DWORD=%x)\n",
	hDrvr,fdwFlags));
    return 0;
}

UINT WINAPI
WinExec(LPCSTR lpCmdLine, UINT fuCmdShow)
{
	LPSTR lpszProgramName,lpTemp;
	PARAMBLOCK pb;
	char buf[BUFFER];
	UINT uiShow;
	HMODULE hModule;

	APISTR((LF_APICALL,"WinExec(cmdline=%s,cmdshow=%x\n",
		lpCmdLine,fuCmdShow));

	strcpy(buf,lpCmdLine);
	if ((lpTemp = strchr(buf,' '))) {
	    *lpTemp = '\0';
	    pb.lpszCmdLine = (char *) WinStrdup(++lpTemp);
	}
	else pb.lpszCmdLine = (char *) WinStrdup("");

	uiShow = fuCmdShow;
	pb.lpShow = &uiShow;
	pb.wSegEnv = 0;
	if (!strchr(buf,'.'))
	    strcat(buf,".exe");

	lpszProgramName = WinMalloc(strlen(buf)+1);
	strcpy(lpszProgramName,buf);
	hModule = LoadModule(lpszProgramName,(LPVOID)&pb);
	WinFree(lpszProgramName);
	APISTR((LF_APIRET,"WinExec: returns UINT %x\n",hModule));
	return (UINT)hModule;
}

/* GetModuleDescription -- convenience function returning module description */
/* string read from the non-resident name table of the binary header */

LPSTR
GetModuleDescription(HINSTANCE hInstance)
{
    HMODULE hModule;
    LPMODULEINFO lpModuleInfo;
    LPSTR lpszResult;

    if (!(hModule = GetModuleFromInstance(hInstance)))
	hModule = hInstance;
    if (!(lpModuleInfo = GETMODULEINFO(hModule)))
	return 0;
    lpszResult = lpModuleInfo->lpModuleDescr;
    RELEASEMODULEINFO(lpModuleInfo);
    return lpszResult;
}

HINSTANCE
LoadDriver(LPCSTR lpszDriver)
{
	char buf[_MAX_PATH];
	int i;
 
	for (i=0;lpszDriver[i];i++)
		buf[i] = tolower(lpszDriver[i]);

	buf[i] = '\0';
	strcat(buf,".drv");

	return LoadLibrary(buf);
}

