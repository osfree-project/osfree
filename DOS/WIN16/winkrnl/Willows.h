/*************************************************************************
	@(#)Willows.h	1.17
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

**************************************************************************/

/*============================================================================*/
#ifdef __COGUID_H__

GUID  GUID_NULL = { 0L , 0 , 0 , { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 } }; 

#endif /* __COGUID_H__ */

/*============================================================================*/

#ifdef __SHELLAPI_H__

#define HDROP   HANDLE

#if !defined(TWIN32)

#ifndef HKEY_IS_DEFINED
#define HKEY_IS_DEFINED
typedef HKEY FAR* PHKEY;
typedef DWORD HKEY;
#endif /* HKEY_IS_DEFINED */

#endif /* !defined(TWIN32) */

/* error values for ShellExecute() beyond the regular WinExec() codes */
#define SE_ERR_SHARE            26
#define SE_ERR_ASSOCINCOMPLETE  27
#define SE_ERR_DDETIMEOUT       28
#define SE_ERR_DDEFAIL          29
#define SE_ERR_DDEBUSY          30
#define SE_ERR_NOASSOC          31

#endif /* #ifdef __SHELLAPI_H__ */





/*============================================================================
*  This is a copy of definitions in toolhelp.h
*  But why are we doing this?
*  Either comment here why this is necessary, or remove this and include
*  toolhelp.h anyplace that defines __TOOLHELP_H__
*/

#ifdef __TOOLHELP_H__

#define MAX_DATA        11

#ifndef MAX_PATH
#define MAX_PATH        255
#endif

#define MAX_MODULE_NAME 8 + 1
#define MAX_CLASSNAME   255
#define HMODULE     HANDLE

typedef struct tagGLOBALENTRY
{
    DWORD dwSize;
    DWORD dwAddress;
    DWORD dwBlockSize;
    HGLOBAL hBlock;
    WORD wcLock;
    WORD wcPageLock;
    WORD wFlags;
    BOOL wHeapPresent;
    HGLOBAL hOwner;
    WORD wType;
    WORD wData;
    DWORD dwNext;
    DWORD dwNextAlt;
} GLOBALENTRY;

typedef struct tagMODULEENTRY
{
    DWORD dwSize;
    char szModule[MAX_MODULE_NAME + 1];
    HMODULE hModule;
    WORD wcUsage;
    char szExePath[MAX_PATH + 1];
    WORD wNext;
} MODULEENTRY;

typedef struct tagMEMMANINFO
{
    DWORD dwSize;
    DWORD dwLargestFreeBlock;
    DWORD dwMaxPagesAvailable;
    DWORD dwMaxPagesLockable;
    DWORD dwTotalLinearSpace;
    DWORD dwTotalUnlockedPages;
    DWORD dwFreePages;
    DWORD dwTotalPages;
    DWORD dwFreeLinearSpace;
    DWORD dwSwapFilePages;
    WORD wPageSize;
} MEMMANINFO;

typedef struct tagTIMERINFO
{
    DWORD dwSize;
    DWORD dwmsSinceStart;
    DWORD dwmsThisVM;
} TIMERINFO;

typedef BOOL (CALLBACK* LPFNNOTIFYCALLBACK)(WORD wID, DWORD dwData);

/* prototypes */

BOOL WINAPI NotifyRegister(HTASK, LPFNNOTIFYCALLBACK, WORD);
BOOL WINAPI NotifyUnRegister(HTASK);
BOOL WINAPI InterruptRegister(HTASK, FARPROC);
BOOL WINAPI InterruptUnRegister(HTASK);
BOOL WINAPI MemManInfo(MEMMANINFO FAR*);
BOOL WINAPI GlobalEntryHandle(GLOBALENTRY *, HGLOBAL);

#endif




/*============================================================================*/

#ifdef __LZEXPAND_H__

#define LZERROR_BADINHANDLE   (-1)  /* invalid input handle */
#define LZERROR_BADOUTHANDLE  (-2)  /* invalid output handle */
#define LZERROR_READ          (-3)  /* corrupt compressed file format */
#define LZERROR_WRITE         (-4)  /* out of space for output file */
#define LZERROR_GLOBALLOC     (-5)  /* insufficient memory for LZFile struct */
#define LZERROR_GLOBLOCK      (-6)  /* bad global handle */
#define LZERROR_BADVALUE      (-7)  /* input parameter out of range */
#define LZERROR_UNKNOWNALG    (-8)  /* compression algorithm not recognized */

#endif /* __LZEXPAND_H__ */

