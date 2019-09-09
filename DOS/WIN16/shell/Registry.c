/*    
	Registry.c	1.6
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

#include <string.h>

#include "windows.h"
#include "windowsx.h"
#include "shellapi.h"
#include "Willows.h"

#include "Log.h"
#include "kerndef.h"
#include "KrnAtoms.h"

static ATOMTABLE AtomTable;

static BOOL fRegInitialized = FALSE;
static BOOL InitReg();

typedef struct keyKEYSTRUCT {
	HKEY hParentKey;
	ATOM atomKey;
	BOOL fOpen;
	HKEY hSubKey;
	HKEY hNext;
	LPSTR lpszValue;
} KEYSTRUCT;

typedef KEYSTRUCT *LPKEYSTRUCT;

static KEYSTRUCT RootKey;
static LPKEYSTRUCT InternalFindKey(LPKEYSTRUCT,LPCSTR,WORD);
static LPKEYSTRUCT InternalCreateKey(LPKEYSTRUCT,ATOM);

#define IFK_FIND	0
#define IFK_CREATE	1

LONG WINAPI
RegCreateKey(HKEY hKey, LPCSTR lpszSubKey, PHKEY phkResult)
{
    LPKEYSTRUCT lpKeyStruct;
    LPKEYSTRUCT lpSubKey;

    if (!fRegInitialized)
	InitReg();

    APISTR((LF_APICALL,"RegCreateKey(HKEY=%x,LPCSTR=%s,PHKEY=%x)\n",
		hKey,HIWORD(lpszSubKey)?lpszSubKey:"NULL",phkResult));

    if (hKey == HKEY_CLASSES_ROOT) 
	lpKeyStruct = &RootKey;
    else  {
	lpKeyStruct = (KEYSTRUCT *)hKey;
	if (!lpKeyStruct || !lpKeyStruct->fOpen) {
    	    APISTR((LF_APIRET,"RegCreateKey: returns LONG %d\n",ERROR_BADKEY));
	    return ERROR_BADKEY;
	}
    }
    lpSubKey = InternalFindKey(lpKeyStruct,lpszSubKey,IFK_CREATE);

    if ((*phkResult = (HKEY)lpSubKey)) {
        APISTR((LF_APIRET,"RegCreateKey: returns LONG %d\n",ERROR_SUCCESS));
	return (LONG)ERROR_SUCCESS;
    }

    APISTR((LF_APIRET,"RegCreateKey: returns LONG %d\n",ERROR_BADKEY));
    return ERROR_BADKEY;
}

LONG WINAPI
RegOpenKey(HKEY hKey, LPCSTR lpszSubKey, PHKEY phkResult)
{
    KEYSTRUCT *lpKeyStruct;
    LPKEYSTRUCT lpSubKey;
    LONG rc;

    if (!fRegInitialized)
	InitReg();

    APISTR((LF_APICALL,"RegOpenKey(HKEY=%x,LPCSTR=%s,PHKEY=%x)\n",
		hKey,HIWORD(lpszSubKey)?lpszSubKey:"NULL",phkResult));

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else
	lpKeyStruct = (KEYSTRUCT *)hKey;

    lpSubKey = InternalFindKey(lpKeyStruct,lpszSubKey,IFK_FIND);

    if ((*phkResult = (HKEY)lpSubKey))
	rc = ERROR_SUCCESS;
    else
	rc = ERROR_BADKEY;
    APISTR((LF_APIRET,"RegOpenKey: returns LONG %d\n",rc));
    return rc;
}

LONG WINAPI
RegOpenKeyEx
(
  HKEY  hKey,           /* handle of open key */
  LPCTSTR  lpSubKey,    /* address of name of subkey to open */
  DWORD  ulOptions,       /* reserved */
  REGSAM  samDesired,   /* security access mask */
  PHKEY  phkResult        /* address of handle of open key */
)
{
  return( RegOpenKey( hKey,     lpSubKey,       phkResult ) );
}


LONG WINAPI
RegEnumKey(HKEY hKey, DWORD iSubKey, LPSTR lpszBuffer, DWORD cbBuffer)
{
    KEYSTRUCT *lpKeyStruct;
    char buf[128];
    int i;

    if (!fRegInitialized)
	InitReg();

    APISTR((LF_APICALL,"RegEnumKey(HKEY=%x,DWORD=%x,LPSTR=%x,DWORD=%x)\n",
	hKey,iSubKey,lpszBuffer,cbBuffer));

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else
	lpKeyStruct = (KEYSTRUCT *)hKey;

    if (!lpKeyStruct) {
    	APISTR((LF_APIRET,"RegEnumKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }

    for (i=0,lpKeyStruct = (KEYSTRUCT *)lpKeyStruct->hSubKey; 
		i< iSubKey && lpKeyStruct;
		i++,lpKeyStruct = (KEYSTRUCT *)lpKeyStruct->hNext);

    if ((i != iSubKey) || (lpKeyStruct == NULL)) {
    	APISTR((LF_APIRET,"RegEnumKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }

    if (GetAtomNameEx(&AtomTable,lpKeyStruct->atomKey,buf,sizeof(buf)))
	strncpy(lpszBuffer,buf,min(cbBuffer,strlen(buf)+1));
    else
	lpszBuffer[0] = '\0';

    APISTR((LF_APIRET,"RegEnumKey: returns LONG %d\n",ERROR_SUCCESS));
    return ERROR_SUCCESS;
}

LONG WINAPI
RegQueryValue(HKEY hKey, LPCSTR lpszSubKey, LPSTR lpszValue, LONG *lpcb)
{
    KEYSTRUCT *lpKeyStruct;
    KEYSTRUCT *lpSubKeyStruct;

    if (!fRegInitialized)
	InitReg();

    APISTR((LF_APICALL,"RegQueryValue(HKEY=%x,LPCSTR=%s,LPSTR=%x,LONG *%x)\n",
		hKey,(lpszSubKey)?lpszSubKey:"NULL",lpszValue,lpcb));

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else 
	lpKeyStruct = (KEYSTRUCT *)hKey;

    if (!lpKeyStruct) {
    	APISTR((LF_APIRET,"RegQueryKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }

#ifdef LATER
    if (!(lpKeyStruct->fOpen)) {
    	APISTR((LF_APIRET,"RegQueryKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }
#endif


    lpSubKeyStruct = InternalFindKey(lpKeyStruct,lpszSubKey,IFK_FIND);

    if (!lpSubKeyStruct) {
    	APISTR((LF_APIRET,"RegQueryKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }

    if (lpSubKeyStruct->lpszValue && (*lpSubKeyStruct->lpszValue != 0)) {
	strncpy(lpszValue,lpSubKeyStruct->lpszValue,min(*lpcb,
		(int)strlen(lpSubKeyStruct->lpszValue)+1));
	*lpcb = strlen(lpszValue) + 1;
    }
    else {
	lpszValue[0] = '\0';
	*lpcb = 0;
    }
    
    APISTR((LF_APIRET,"RegQueryKey: returns LONG %d\n",ERROR_SUCCESS));
    return ERROR_SUCCESS;
}

LONG
WINAPI
RegQueryValueEx
(
  HKEY hKey,
  LPCSTR lpValueName,
  LPDWORD lpReserved,
  LPDWORD lpType,
  LPBYTE lpData,
  LPDWORD lpcbData
)
{
  /* Success is 0, so return arbitrary error. */
  return(1);
}


LONG WINAPI
RegSetValue(HKEY hKey, LPCSTR lpszSubKey,
		DWORD fdwType, LPCSTR lpszValue, DWORD cb)
{
    KEYSTRUCT *lpKeyStruct;
    KEYSTRUCT *lpSubKeyStruct;

    if (!fRegInitialized)
	InitReg();

    APISTR((LF_APICALL,
	"RegSetValue(HKEY=%x,LPCSTR=%x,DWORD=%x,LPCSTR=%s,DWORD=%x)\n",
		hKey,lpszSubKey,fdwType,
		HIWORD(lpszValue)?lpszValue:"NULL",cb));

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else
	lpKeyStruct = (KEYSTRUCT *)hKey;

    if (!lpKeyStruct || !(lpKeyStruct->fOpen)) {
        APISTR((LF_APIRET,"RegSetValue: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }

    lpSubKeyStruct = InternalFindKey(lpKeyStruct,lpszSubKey,IFK_FIND);

    if (!lpSubKeyStruct) {
        APISTR((LF_APIRET,"RegQueryKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }

    if (lpSubKeyStruct->lpszValue)
	WinFree(lpSubKeyStruct->lpszValue);

    if (lpszValue && (*lpszValue != 0)) {
	lpSubKeyStruct->lpszValue = (LPSTR) WinMalloc(strlen(lpszValue)+1);
	strcpy(lpSubKeyStruct->lpszValue,lpszValue);
    }
    
    APISTR((LF_APIRET,"RegQueryKey: returns LONG %d\n",ERROR_SUCCESS));
    return ERROR_SUCCESS;
}

LONG WINAPI
RegCloseKey(HKEY hKey)
{
    KEYSTRUCT *lpKeyStruct;

    APISTR((LF_APICALL,"RegCloseKey(HKEY=%x)\n",hKey));

    if (!fRegInitialized)
	InitReg();

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else
	lpKeyStruct = (KEYSTRUCT *)hKey;

    if (!lpKeyStruct) {
    	APISTR((LF_APIRET,"RegCreateKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }

    lpKeyStruct->fOpen = 0;

    APISTR((LF_APIRET,"RegCreateKey: returns LONG %d\n",ERROR_SUCCESS));
    return (LONG)ERROR_SUCCESS;
}

LONG WINAPI
RegDeleteKey(HKEY hKey, LPCSTR lpszSubKey)
{
    LPKEYSTRUCT lpKeyStruct,lpKeyParent,lpKeyTmp;

    if (!fRegInitialized)
	InitReg();

    APISTR((LF_APICALL,"RegDeleteKey(HKEY=%x,LPCSTR=%x)\n",
	hKey,lpszSubKey));

    lpKeyStruct = InternalFindKey((LPKEYSTRUCT)hKey,lpszSubKey,IFK_FIND);

    if (!lpKeyStruct || !lpKeyStruct->fOpen) {
    	APISTR((LF_APIRET,"RegDeleteKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_BADKEY;
    }

    if (lpKeyStruct == &RootKey) {
    	APISTR((LF_APIRET,"RegDeleteKey: returns LONG %d\n",ERROR_BADKEY));
	return ERROR_ACCESS_DENIED;
    }

    if (lpKeyStruct->lpszValue)
	WinFree(lpKeyStruct->lpszValue);

    while (lpKeyStruct->hSubKey) 
	RegDeleteKey(lpKeyStruct->hSubKey,"");

    lpKeyParent = (LPKEYSTRUCT)lpKeyStruct->hParentKey;

    if (lpKeyParent->hSubKey == (HKEY)lpKeyStruct)
	lpKeyParent->hSubKey = lpKeyStruct->hNext;
    else {
	for (lpKeyTmp = (LPKEYSTRUCT)lpKeyParent->hSubKey;
			lpKeyTmp;
			lpKeyTmp = (LPKEYSTRUCT)lpKeyTmp->hNext) 
	    if (lpKeyTmp->hNext == (HKEY)lpKeyStruct)
		lpKeyTmp->hNext = lpKeyStruct->hNext;
    }

    DeleteAtomEx(&AtomTable,lpKeyStruct->atomKey);

    WinFree((LPSTR)lpKeyStruct);

    APISTR((LF_APIRET,"RegDeleteKey: returns LONG %d\n",ERROR_SUCCESS));
    return ERROR_SUCCESS;
}

/******************************************************************************/

static void
ReadSetupReg()
{
    char buf[_MAX_PATH];
    HFILE hf;
    LPSTR lpBuffer,ptr;
    char lpTmp[0x100];
    DWORD dwFileSize;
    int i = 0,j;
    KEYSTRUCT *lpKeyStruct;
    HKEY hk;

    if (!GetWindowsDirectory(buf,_MAX_PATH)) 
	getcwd(buf,_MAX_PATH);
    lstrcat(buf,"/setup.reg");
    if ((hf = _lopen(buf,READ)) == HFILE_ERROR)
	return;
    dwFileSize = _llseek(hf,0,2);
    lpBuffer = (LPSTR) WinMalloc(dwFileSize+2);
    _llseek(hf,0,0);
    _lread(hf,lpBuffer,dwFileSize);
    _lclose(hf);

    ptr = lpBuffer;
    while (i < dwFileSize) {
	while ((lpBuffer[i] != '\n') && (i < dwFileSize))
	    i++;
	if (lpBuffer[i] == '\n')
	    lpBuffer[i] = '\0';
	else
	    lpBuffer[i+1] ='\0';
	if (lpBuffer[i-1] == 0xd)	/* strip ^M */
	    lpBuffer[i-1] = 0;
	strncpy(lpTmp,ptr,17);
	lpTmp[17] = 0;
	if (!lstrcmpi(lpTmp,"HKEY_CLASSES_ROOT")) { /* <key> <value> pair */
	    lstrcpy(lpTmp,ptr);
	    lpKeyStruct = &RootKey;
	    while (*ptr++ != '\\');
	    j = 0;
	    while (ptr[j] != ' ') j++;
	    ptr[j] = 0;
	    if (RegCreateKey((HKEY)lpKeyStruct,ptr,&hk) != ERROR_SUCCESS)
		break;
	    ptr += j+3;
	    if (RegSetValue(hk,NULL,REG_SZ,ptr,strlen(ptr)) != ERROR_SUCCESS)
		break;
	}
	ptr = &lpBuffer[i+1];
    }
    WinFree(lpBuffer);
}

static BOOL
InitReg()
{
    RootKey.hParentKey = (HKEY)0L;
    if (!RootKey.atomKey)
	RootKey.atomKey = AddAtomEx(&AtomTable,"HKEY_CLASSES_ROOT");
    RootKey.fOpen = TRUE;
    fRegInitialized = TRUE;
    ReadSetupReg();
    
    return TRUE;
}

static LPKEYSTRUCT
InternalFindKey(LPKEYSTRUCT lpKeyStruct, LPCSTR lpszSubKey, WORD wFlag)
{
    LPKEYSTRUCT lpKeyTmp;
    char buf[80];
    LPSTR ptr,lpTmp;
    ATOM atomSubKey;

    if (lpszSubKey)
	strcpy(buf,lpszSubKey);
    if (lpszSubKey && (strlen(lpszSubKey) != 0)) {
	ptr = buf;
	while (1) {
	    lpTmp = strchr(ptr,'\\');
	    if (lpTmp)
		*lpTmp = '\0';
	    atomSubKey = FindAtomEx(&AtomTable,ptr);
	    if (atomSubKey == 0) 
		if (wFlag == IFK_CREATE)
		    atomSubKey = AddAtomEx(&AtomTable,ptr);
		else
		    return (HKEY)0L;
	    for (lpKeyTmp = (KEYSTRUCT *)lpKeyStruct->hSubKey;
			lpKeyTmp;
			lpKeyTmp = (KEYSTRUCT *)lpKeyTmp->hNext) {
		if (lpKeyTmp->atomKey == atomSubKey)
		    break;
	    }
	    if (lpKeyTmp)
		lpKeyStruct = lpKeyTmp;
	    else 
		if (wFlag == IFK_CREATE)
		    lpKeyStruct = InternalCreateKey(lpKeyStruct,atomSubKey);
		else
		    return (HKEY)0L;
	    lpKeyStruct->fOpen = TRUE;
	    if (lpTmp)
		ptr = lpTmp + 1;
	    else
		break;
	}
    }
    return lpKeyStruct;
}

static LPKEYSTRUCT
InternalCreateKey(LPKEYSTRUCT lpKeyStruct, ATOM atomSubKey)
{
    LPKEYSTRUCT lpSubKey;
    LPKEYSTRUCT lpKeyTmp;

    lpSubKey = (KEYSTRUCT *)WinMalloc(sizeof(KEYSTRUCT));
    memset((LPSTR)lpSubKey,'\0',sizeof(KEYSTRUCT));
    if (!lpKeyStruct->hSubKey)
	lpKeyStruct->hSubKey = (HKEY)lpSubKey;
    else {
	for (lpKeyTmp = (KEYSTRUCT *)lpKeyStruct->hSubKey;
		lpKeyTmp->hNext;
		lpKeyTmp = (KEYSTRUCT *)lpKeyTmp->hNext);
	lpKeyTmp->hNext = (HKEY)lpSubKey;
    }
    lpSubKey->hParentKey = (HKEY)lpKeyStruct;
    lpSubKey->atomKey = atomSubKey;
    return lpSubKey;
}
