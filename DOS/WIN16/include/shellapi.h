
#ifndef shellapi__h
#define shellapi__h

/*****************************************************************************\
*                                                                             *
* shellapi.h -  SHELL.DLL functions, types, and definitions                   *
*                                                                             *
*               Copyright (c) 1992, Microsoft Corp.  All rights reserved      *
*                                                                             *
\*****************************************************************************/

#ifndef _INC_SHELLAPI
#define _INC_SHELLAPI

#ifdef	MSC
#ifndef RC_INVOKED
#pragma pack(1)         /* Assume byte packing throughout */
#endif /* RC_INVOKED */
#endif

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */

/* If included with Windows 3.0 windows.h: define 3.1-compatible types */

#ifndef _INC_WINDOWS

#define HDROP   HANDLE
#define WINAPI  FAR PASCAL
#define LPCSTR  LPSTR
#define UINT    WORD

#else

DECLARE_HANDLE(HDROP);

#endif

#if !defined(TWIN32)
/* return codes from Registration functions */
#define ERROR_SUCCESS           0L
#define ERROR_BADDB             1L
#define ERROR_BADKEY            2L
#define ERROR_CANTOPEN          3L
#define ERROR_CANTREAD          4L
#define ERROR_CANTWRITE         5L
#define ERROR_OUTOFMEMORY       6L
#define ERROR_INVALID_PARAMETER 7L
#define ERROR_ACCESS_DENIED     8L

#ifndef HKEY_IS_DEFINED
typedef DWORD HKEY;
typedef HKEY FAR* PHKEY;
#define HKEY_IS_DEFINED
#endif /* HKEY_IS_DEFINED */

#define HKEY_CLASSES_ROOT	1

# else

#define HKEY_CLASSES_ROOT           ((HKEY)0x80000000UL)
#define HKEY_CURRENT_USER           ((HKEY)0x80000001UL)
#define HKEY_LOCAL_MACHINE          ((HKEY)0x80000002UL)
#define HKEY_USERS                  ((HKEY)0x80000003UL)
#define HKEY_PERFORMANCE_DATA       ((HKEY)0x80000004UL)
#define HKEY_CURRENT_CONFIG         ((HKEY)0x80000005UL)
#define HKEY_DYN_DATA               ((HKEY)0x80000006UL)

#endif /* TWIN32 */

#define REG_SZ			(1)

/*
** Requested Key access mask type.
*/
typedef ACCESS_MASK REGSAM;

#ifndef NOAPIPROTO

LONG WINAPI RegOpenKey(HKEY, LPCSTR, HKEY FAR*);
LONG WINAPI RegCreateKey(HKEY, LPCSTR, HKEY FAR*);
LONG WINAPI RegCloseKey(HKEY);
LONG WINAPI RegDeleteKey(HKEY, LPCSTR);
LONG WINAPI RegSetValue(HKEY, LPCSTR, DWORD, LPCSTR, DWORD);
LONG WINAPI RegQueryValue(HKEY, LPCSTR, LPSTR, LONG FAR*);
LONG WINAPI RegEnumKey(HKEY, DWORD, LPSTR, DWORD);
LONG WINAPI RegOpenKeyEx ( HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
LONG WINAPI RegQueryValueEx ( HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);

UINT WINAPI DragQueryFile(HDROP, UINT, LPSTR, UINT);
BOOL WINAPI DragQueryPoint(HDROP, POINT FAR*);
void WINAPI DragFinish(HDROP);
void WINAPI DragAcceptFiles(HWND, BOOL);

HICON WINAPI ExtractIcon(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex);

#endif	/* NOAPIPROTO */

/* error values for ShellExecute() beyond the regular WinExec() codes */
#define SE_ERR_SHARE            26
#define SE_ERR_ASSOCINCOMPLETE  27
#define SE_ERR_DDETIMEOUT       28
#define SE_ERR_DDEFAIL          29
#define SE_ERR_DDEBUSY          30
#define SE_ERR_NOASSOC          31

#ifndef NOAPIPROTO

HINSTANCE WINAPI ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, int iShowCmd);
HINSTANCE WINAPI FindExecutable(LPCSTR lpFile, LPCSTR lpDirectory, LPSTR lpResult);

#endif	/* NOAPIPROTO */

/* SHFILEINFO */

#ifndef UNICODE_ONLY
typedef struct _SHFILEINFOA
{
	HICON	hIcon;
	int	iIcon;
	DWORD	dwAttributes;
	CHAR	szDisplayName[MAX_PATH];
	CHAR	szTypeName[80];
} SHFILEINFOA;
#endif	/* UNICODE_ONLY */

#ifndef ANSI_ONLY
typedef struct _SHFILEINFOW
{
	HICON	hIcon;
	int	iIcon;
	DWORD	dwAttributes;
	WCHAR	szDisplayName[MAX_PATH];
	WCHAR	szTypeName[80];
} SHFILEINFOW;
#endif	/* ANSI_ONLY */

#ifdef UNICODE
typedef SHFILEINFOW	SHFILEINFO;
#else
typedef SHFILEINFOA	SHFILEINFO;
#endif	/* UNICODE */

#define SHGFI_LARGEICON		0x000000000
#define SHGFI_SMALLICON		0x000000001
#define SHGFI_OPENICON		0x000000002
#define SHGFI_SHELLICONSIZE	0x000000004
#define SHGFI_PIDL		0x000000008
#define SHGFI_USEFILEATTRIBUTES	0x000000010
#define SHGFI_ICON		0x000000100
#define SHGFI_DISPLAYNAME	0x000000200
#define SHGFI_TYPENAME		0x000000400
#define SHGFI_ATTRIBUTES	0x000000800
#define SHGFI_ICONLOCATION	0x000001000
#define SHGFI_EXETYPE		0x000002000
#define SHGFI_SYSICONINDEX	0x000004000
#define SHGFI_LINKOVERLAY	0x000008000
#define SHGFI_SELECTED		0x000010000

#define SFGAO_CANCOPY		DROPEFFECT_COPY
#define SFGAO_CANMOVE		DROPEFFECT_MOVE
#define SFGAO_CANLINK		DROPEFFECT_LINK
#define SFGAO_CANRENAME		0x00000010L
#define SFGAO_CANDELETE		0x00000020L
#define SFGAO_HASPROPSHEET	0x00000040L
#define SFGAO_DROPTARGET	0x00000100L
#define SFGAO_CAPABILITYMASK	0x00000177L
#define SFGAO_LINK		0x00010000L
#define SFGAO_SHARE		0x00020000L
#define SFGAO_READONLY		0x00040000L
#define SFGAO_GHOSTED		0x00080000L
#define SFGAO_DISPLAYATTRMASK	0x000F0000L
#define SFGAO_FILESYSANCESTOR	0x10000000L
#define SFGAO_FOLDER		0x20000000L
#define SFGAO_FILESYSTEM	0x40000000L
#define SFGAO_HASSUBFOLDER	0x80000000L
#define SFGAO_CONTENTSMASK	0x80000000L
#define SFGAO_VALIDATE		0x01000000L
#define SFGAO_REMOVABLE		0x02000000L

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#ifdef	MSC
#ifndef RC_INVOKED
#pragma pack()
#endif  /* RC_INVOKED */
#endif

#endif  /* _INC_SHELLAPI */
#endif /* shellapi__h */
