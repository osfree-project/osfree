/*    
	ModTable.c	1.44
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

#include "platform.h"

#include "ModTable.h"
#include "Segment.h"

typedef struct {
	char     *rcstypename;
	unsigned short  rcscount;
	void *rcsinfo;
} TYPEINFO;

extern long Trap();

extern long int ExitWindows();
extern long int OffsetClipRgn();
extern long int OpenIcon();
extern long int IsGDIObject();
extern long int SetEnvironment();
extern long int GetEnvironment();
extern long int SetDIBits();
extern long int GetDIBits();
extern long int StretchDIBits();
extern long int SetDIBitsToDevice();

extern long int FatalExit();
extern long int GetVersion();
extern long int TWIN_LocalInit();
extern long int TWIN_LocalAlloc();
extern long int TWIN_LocalReAlloc();
extern long int TWIN_LocalFree();
extern long int TWIN_LocalLock();
extern long int TWIN_LocalUnlock();
extern long int TWIN_LocalSize();
extern long int TWIN_LocalHandle();
extern long int GlobalAlloc();
extern long int GlobalReAlloc();
extern long int GlobalFree();
extern long int GlobalLock();
extern long int GlobalUnlock();
extern long int GlobalSize();
extern long int GlobalFlags();
extern long int LockSegment();
extern long int UnlockSegment();
extern long int GlobalCompact();
extern long int GlobalNotify();
extern long int Yield();
extern long int WaitEvent();
extern long int GetCurrentTask();
extern long int LoadModule();
extern long int GetModuleHandle();
extern long int GetModuleUsage();
extern long int GetModuleFileName();
extern long int GetProcAddress();
extern long int MakeProcInstance();
extern long int FreeProcInstance();
extern long int GetProfileInt();
extern long int GetProfileString();
extern long int WriteProfileString();
extern long int FindResource();
extern long int LoadResource();
extern long int LockResource();
extern long int FreeResource();
extern long int AccessResource();
extern long int InitAtomTable();
extern long int FindAtom();
extern long int AddAtom();
extern long int DeleteAtom();
extern long int GetAtomName();
#ifdef	LATER
extern long int GetAtomHandle();
#endif
extern long int OpenFile();
extern long int lstrcmp();
extern long int AnsiNext();
extern long int AnsiPrev();
extern long int AnsiUpper();
extern long int AnsiLower();
extern long int _lclose();
extern long int _lread();
extern long int _lcreat();
extern long int _llseek();
extern long int _lopen();
extern long int _lwrite();
extern long int _hread();
extern long int _hwrite();
extern long int lstrcpy();
extern long int lstrcat();
extern long int lstrlen();
extern long int InitTask();
extern long int DefineHandleTable();
extern long int LoadLibrary();
extern long int FreeLibrary();
extern long int SetSwapAreaSize();
extern long int SetErrorMode();
extern long int OutputDebugString();
extern long int TWIN_LocalShrink();
extern long int TWIN_LocalCompact();
extern long int GetPrivateProfileInt();
extern long int GetPrivateProfileString();
extern long int WritePrivateProfileString();
extern long int GetDOSEnvironment();
extern long int GetWinFlags();
extern long int GetWindowsDirectory();
extern long int GetSystemDirectory();
#ifdef TWIN32
extern long int GetDriveType16();
extern long int GetTempFileName16();
#else
extern long int GetDriveType();
extern long int GetTempFileName();
#endif
extern long int FatalAppExit();
extern long int GetNumTasks();
extern long int GlobalLRUOldest();
extern long int GlobalLRUNewest();
extern long int WinExec();
extern long int GetFreeSpace();
extern long int SetHandleCount();
extern long int IsDBCSLeadByte();
extern long int TWIN_LocalHandleDelta();
extern long int IsROMModule();
extern long int IsROMFile();
extern long int lstrcpyn();
extern long int hmemcpy();
extern long int IsBadPtr();
extern long int IsBadHugePtr();
extern long int FileCdr();
extern long int IsTask();
extern long int CreateScalableFontResource();

 /* Entry Table KERNEL: */

static ENTRYTAB entry_tab_KERNEL[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "KERNEL", 0, 0, 0 },
	/* 001 */	{ "FATALEXIT", 0x0008, 0x0008, FatalExit },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "GETVERSION", 0x0008, 0x0018, GetVersion },
	/* 004 */	{ "LOCALINIT", 0x0008, 0x0020, TWIN_LocalInit },
	/* 005 */	{ "LOCALALLOC", 0x0008, 0x0028, TWIN_LocalAlloc },
	/* 006 */	{ "LOCALREALLOC", 0x0008, 0x0030, TWIN_LocalReAlloc },
	/* 007 */	{ "LOCALFREE", 0x0008, 0x0038, TWIN_LocalFree },
	/* 008 */	{ "LOCALLOCK", 0x0008, 0x0040, TWIN_LocalLock },
	/* 009 */	{ "LOCALUNLOCK", 0x0008, 0x0048, TWIN_LocalUnlock },
	/* 00a */	{ "LOCALSIZE", 0x0008, 0x0050, TWIN_LocalSize },
	/* 00b */	{ "LOCALHANDLE", 0x0008, 0x0058, TWIN_LocalHandle },
	/* 00c */	{ "LOCALFLAGS", 0x0008, 0x0060, 0 },
	/* 00d */	{ "LOCALCOMPACT", 0x0008, 0x0068, TWIN_LocalCompact },
	/* 00e */	{ "LOCALNOTIFY", 0x0008, 0x0070, 0 },
	/* 00f */	{ "GLOBALALLOC", 0x0008, 0x0078, GlobalAlloc },
	/* 010 */	{ "GLOBALREALLOC", 0x0008, 0x0080, GlobalReAlloc },
	/* 011 */	{ "GLOBALFREE", 0x0008, 0x0088, GlobalFree },
	/* 012 */	{ "GLOBALLOCK", 0x0008, 0x0090, GlobalLock },
	/* 013 */	{ "GLOBALUNLOCK", 0x0008, 0x0098, GlobalUnlock },
	/* 014 */	{ "GLOBALSIZE", 0x0008, 0x00a0, GlobalSize },
	/* 015 */	{ "GLOBALHANDLE", 0x0008, 0x00a8, 0 },
	/* 016 */	{ "GLOBALFLAGS", 0x0008, 0x00b0, GlobalFlags },
	/* 017 */	{ "LOCKSEGMENT", 0x0008, 0x00b8, LockSegment },
	/* 018 */	{ "UNLOCKSEGMENT", 0x0008, 0x00c0, UnlockSegment },
	/* 019 */	{ "GLOBALCOMPACT", 0x0008, 0x00c8, GlobalCompact },
	/* 01a */	{ "", 0, 0, 0 },
	/* 01b */	{ "", 0, 0, 0 },
	/* 01c */	{ "", 0, 0, 0 },
	/* 01d */	{ "YIELD", 0x0008, 0x00e8, Yield },
	/* 01e */	{ "WAITEVENT", 0x0008, 0x00f0, WaitEvent },
	/* 01f */	{ "", 0, 0, 0 },
	/* 020 */	{ "", 0, 0, 0 },
	/* 021 */	{ "", 0, 0, 0 },
	/* 022 */	{ "", 0, 0, 0 },
	/* 023 */	{ "", 0, 0, 0 },
	/* 024 */	{ "GETCURRENTTASK", 0x0008, 0x0120, GetCurrentTask },
	/* 025 */	{ "GETCURRENTPDB", 0x0008, 0x0128, 0 },
	/* 026 */	{ "", 0, 0, 0 },
	/* 027 */	{ "", 0, 0, 0 },
	/* 028 */	{ "", 0, 0, 0 },
	/* 029 */	{ "", 0, 0, 0 },
	/* 02a */	{ "", 0, 0, 0 },
	/* 02b */	{ "", 0, 0, 0 },
	/* 02c */	{ "", 0, 0, 0 },
	/* 02d */	{ "LOADMODULE", 0x0008, 0x0168, LoadModule },
	/* 02e */	{ "FREEMODULE", 0x0008, 0x0170, 0 },
	/* 02f */	{ "GETMODULEHANDLE", 0x0008, 0x0178, GetModuleHandle },
	/* 030 */	{ "GETMODULEUSAGE", 0x0008, 0x0180, GetModuleUsage },
	/* 031 */	{ "GETMODULEFILENAME", 0x0008, 0x0188, GetModuleFileName },
	/* 032 */	{ "GETPROCADDRESS", 0x0008, 0x0190, GetProcAddress },
	/* 033 */	{ "MAKEPROCINSTANCE", 0x0008, 0x0198, MakeProcInstance },
	/* 034 */	{ "FREEPROCINSTANCE", 0x0008, 0x01a0, FreeProcInstance },
	/* 035 */	{ "", 0, 0, 0 },
	/* 036 */	{ "GETINSTANCEDATA", 0x0008, 0x01b0, 0 },
	/* 037 */	{ "CATCH", 0x0008, 0x01b8, 0 },
	/* 038 */	{ "THROW", 0x0008, 0x01c0, 0 },
	/* 039 */	{ "GETPROFILEINT", 0x0008, 0x01c8, GetProfileInt },
	/* 03a */	{ "GETPROFILESTRING", 0x0008, 0x01d0, GetProfileString },
	/* 03b */	{ "WRITEPROFILESTRING", 0x0008, 0x01d8, WriteProfileString },
	/* 03c */	{ "FINDRESOURCE", 0x0008, 0x01e0, FindResource },
	/* 03d */	{ "LOADRESOURCE", 0x0008, 0x01e8, LoadResource },
	/* 03e */	{ "LOCKRESOURCE", 0x0008, 0x01f0, LockResource },
	/* 03f */	{ "FREERESOURCE", 0x0008, 0x01f8, FreeResource },
	/* 040 */	{ "ACCESSRESOURCE", 0x0008, 0x0200, AccessResource },
	/* 041 */	{ "SIZEOFRESOURCE", 0x0008, 0x0208, 0 },
	/* 042 */	{ "ALLOCRESOURCE", 0x0008, 0x0210, 0 },
	/* 043 */	{ "SETRESOURCEHANDLER", 0x0008, 0x0218, 0 },
	/* 044 */	{ "INITATOMTABLE", 0x0008, 0x0220, InitAtomTable },
	/* 045 */	{ "FINDATOM", 0x0008, 0x0228, FindAtom },
	/* 046 */	{ "ADDATOM", 0x0008, 0x0230, AddAtom },
	/* 047 */	{ "DELETEATOM", 0x0008, 0x0238, DeleteAtom },
	/* 048 */	{ "GETATOMNAME", 0x0008, 0x0240, GetAtomName },
#ifdef	LATER
	/* 049 */	{ "GETATOMHANDLE", 0x0008, 0x0248, GetAtomHandle },
#else
	/* 049 */	{ "GETATOMHANDLE", 0x0008, 0x0248, 0 },
#endif
	/* 04a */	{ "OPENFILE", 0x0008, 0x0250, OpenFile },
	/* 04b */	{ "", 0, 0, 0 },
	/* 04c */	{ "", 0, 0, 0 },
	/* 04d */	{ "RESERVED1", 0x0008, 0x0268, AnsiNext },
	/* 04e */	{ "RESERVED2", 0x0008, 0x0270, AnsiPrev },
	/* 04f */	{ "RESERVED3", 0x0008, 0x0278, AnsiUpper },
	/* 050 */	{ "RESERVED4", 0x0008, 0x0280, AnsiLower },
	/* 051 */	{ "_LCLOSE", 0x0008, 0x0288, _lclose},
	/* 052 */	{ "_LREAD", 0x0008, 0x0290, _lread},
	/* 053 */	{ "_LCREAT", 0x0008, 0x0298, _lcreat},
	/* 054 */	{ "_LLSEEK", 0x0008, 0x02a0, _llseek},
	/* 055 */	{ "_LOPEN", 0x0008, 0x02a8, _lopen},
	/* 056 */	{ "_LWRITE", 0x0008, 0x02b0, _lwrite},
	/* 057 */	{ "RESERVED5", 0x0008, 0x02b8, lstrcmp },
	/* 058 */	{ "LSTRCPY", 0x0008, 0x02c0, lstrcpy },
	/* 059 */	{ "LSTRCAT", 0x0008, 0x02c8, lstrcat },
	/* 05a */	{ "LSTRLEN", 0x0008, 0x02d0, lstrlen },
	/* 05b */	{ "INITTASK", 0x0008, 0x02d8, InitTask },
	/* 05c */	{ "GETTEMPDRIVE", 0x0008, 0x02e0, 0 },
	/* 05d */	{ "GETCODEHANDLE", 0x0008, 0x02e8, 0 },
	/* 05e */	{ "DEFINEHANDLETABLE", 0x0008, 0x02f0, DefineHandleTable },
	/* 05f */	{ "LOADLIBRARY", 0x0008, 0x02f8, LoadLibrary },
	/* 060 */	{ "FREELIBRARY", 0x0008, 0x0300, FreeLibrary },
#ifdef TWIN32
	/* 061 */	{ "GETTEMPFILENAME", 0x0008, 0x0308, GetTempFileName16 },
#else
	/* 061 */	{ "GETTEMPFILENAME", 0x0008, 0x0308, GetTempFileName },
#endif
	/* 062 */	{ "", 0, 0, 0 },
	/* 063 */	{ "", 0, 0, 0 },
	/* 064 */	{ "VALIDATECODESEGMENTS", 0x0008, 0x0320, 0 },
	/* 065 */	{ "", 0, 0, 0 },
	/* 066 */	{ "DOS3CALL", 0x0008, 0x0330, 0 },
	/* 067 */	{ "NETBIOSCALL", 0x0008, 0x0338, 0 },
	/* 068 */	{ "GETCODEINFO", 0x0008, 0x0340, 0 },
	/* 069 */	{ "", 0, 0, 0 },
	/* 06a */	{ "SETSWAPAREASIZE", 0x0008, 0x0350, SetSwapAreaSize },
	/* 06b */	{ "SETERRORMODE", 0x0008, 0x0358, SetErrorMode },
	/* 06c */	{ "SWITCHSTACKTO", 0x0008, 0x0360, 0 },
	/* 06d */	{ "SWITCHSTACKBACK", 0x0008, 0x0368, 0 },
	/* 06e */	{ "", 0, 0, 0 },
	/* 06f */	{ "GLOBALWIRE", 0x0008, 0x0378, 0 },
	/* 070 */	{ "GLOBALUNWIRE", 0x0008, 0x0380, 0 },
	/* 071 */	{ "", 0, 0, 0 },
	/* 072 */	{ "", 0, 0, 0 },
	/* 073 */	{ "OUTPUTDEBUGSTRING", 0x0008, 0x0398, OutputDebugString },
	/* 074 */	{ "", 0, 0, 0 },
	/* 075 */	{ "", 0, 0, 0 },
	/* 076 */	{ "", 0, 0, 0 },
	/* 077 */	{ "", 0, 0, 0 },
	/* 078 */	{ "", 0, 0, 0 },
	/* 079 */	{ "LOCALSHRINK", 0x0008, 0x03c8, TWIN_LocalShrink },
	/* 07a */	{ "", 0, 0, 0 },
	/* 07b */	{ "", 0, 0, 0 },
	/* 07c */	{ "", 0, 0, 0 },
	/* 07d */	{ "", 0, 0, 0 },
	/* 07e */	{ "", 0, 0, 0 },
	/* 07f */	{ "GETPRIVATEPROFILEINT", 0x0008, 0x03f8, GetPrivateProfileInt },
	/* 080 */	{ "GETPRIVATEPROFILESTRING", 0x0008, 0x0400, GetPrivateProfileString },
	/* 081 */	{ "WRITEPRIVATEPROFILESTRING", 0x0008, 0x0408, WritePrivateProfileString },
	/* 082 */	{ "FILECDR", 0x0008, 0x0410, FileCdr },
	/* 083 */	{ "GETDOSENVIRONMENT", 0x0008, 0x0418, GetDOSEnvironment },
	/* 084 */	{ "GETWINFLAGS", 0x0008, 0x0420, GetWinFlags },
	/* 085 */	{ "", 0, 0, 0 },
	/* 086 */	{ "GETWINDOWSDIRECTORY", 0x0008, 0x0430, GetWindowsDirectory },
	/* 087 */	{ "GETSYSTEMDIRECTORY", 0x0008, 0x0438, GetSystemDirectory },
#ifdef TWIN32
	/* 088 */	{ "GETDRIVETYPE", 0x0008, 0x0440, GetDriveType16 },
#else
	/* 088 */	{ "GETDRIVETYPE", 0x0008, 0x0440, GetDriveType },
#endif
	/* 089 */	{ "FATALAPPEXIT", 0x0008, 0x0448, FatalAppExit },
	/* 08a */	{ "GETHEAPSPACES", 0x0008, 0x0450, 0 },
	/* 08b */	{ "", 0, 0, 0 },
	/* 08c */	{ "", 0, 0, 0 },
	/* 08d */	{ "", 0, 0, 0 },
	/* 08e */	{ "", 0, 0, 0 },
	/* 08f */	{ "", 0, 0, 0 },
	/* 090 */	{ "", 0, 0, 0 },
	/* 091 */	{ "", 0, 0, 0 },
	/* 092 */	{ "", 0, 0, 0 },
	/* 093 */	{ "", 0, 0, 0 },
	/* 094 */	{ "", 0, 0, 0 },
	/* 095 */	{ "", 0, 0, 0 },
	/* 096 */	{ "DIRECTEDYIELD", 0x0008, 0x04b0, 0 },
	/* 097 */	{ "", 0, 0, 0 },
	/* 098 */	{ "GETNUMTASKS", 0x0008, 0x04c0, GetNumTasks },
	/* 099 */	{ "", 0, 0, 0 },
	/* 09a */	{ "GLOBALNOTIFY", 0x0008, 0x04d0, GlobalNotify },
	/* 09b */	{ "", 0, 0, 0 },
	/* 09c */	{ "LIMITEMSPAGES", 0x0008, 0x04e0, 0 },
	/* 09d */	{ "", 0, 0, 0 },
	/* 09e */	{ "", 0, 0, 0 },
	/* 09f */	{ "", 0, 0, 0 },
	/* 0a0 */	{ "", 0, 0, 0 },
	/* 0a1 */	{ "", 0, 0, 0 },
	/* 0a2 */	{ "", 0, 0, 0 },
	/* 0a3 */	{ "GLOBALLRUOLDEST", 0x0008, 0x0518, GlobalLRUOldest },
	/* 0a4 */	{ "GLOBALLRUNEWEST", 0x0008, 0x0520, GlobalLRUNewest },
	/* 0a5 */	{ "", 0, 0, 0 },
	/* 0a6 */	{ "WINEXEC", 0x0008, 0x0530, WinExec },
	/* 0a7 */	{ "", 0, 0, 0 },
	/* 0a8 */	{ "", 0, 0, 0 },
	/* 0a9 */	{ "GETFREESPACE", 0x0008, 0x0548, GetFreeSpace },
	/* 0aa */	{ "ALLOCCSTODSALIAS", 0x0008, 0x0550, 0 },
	/* 0ab */	{ "ALLOCDSTOCSALIAS", 0x0008, 0x0558, 0 },
	/* 0ac */	{ "", 0, 0, 0 },
	/* 0ad */	{ "", 0, 0, 0 },
	/* 0ae */	{ "", 0, 0, 0 },
	/* 0af */	{ "ALLOCSELECTOR", 0x0008, 0x0578, 0 },
	/* 0b0 */	{ "FREESELECTOR", 0x0008, 0x0580, 0 },
	/* 0b1 */	{ "PRESTOCHANGOSELECTOR", 0x0008, 0x0588, 0 },
	/* 0b2 */	{ "", 0, 0, 0 },
	/* 0b3 */	{ "", 0, 0, 0 },
	/* 0b4 */	{ "", 0, 0, 0 },
	/* 0b5 */	{ "", 0, 0, 0 },
	/* 0b6 */	{ "", 0, 0, 0 },
	/* 0b7 */	{ "", 0, 0, 0 },
	/* 0b8 */	{ "GLOBALDOSALLOC", 0x0008, 0x05c0, 0 },
	/* 0b9 */	{ "GLOBALDOSFREE", 0x0008, 0x05c8, 0 },
	/* 0ba */	{ "GETSELECTORBASE", 0x0008, 0x05d0, 0 },
	/* 0bb */	{ "SETSELECTORBASE", 0x0008, 0x05d8, 0 },
	/* 0bc */	{ "GETSELECTORLIMIT", 0x0008, 0x05e0, 0 },
	/* 0bd */	{ "SETSELECTORLIMIT", 0x0008, 0x05e8, 0 },
	/* 0be */	{ "", 0, 0, 0 },
	/* 0bf */	{ "GLOBALPAGELOCK", 0x0008, 0x05f8, 0 },
	/* 0c0 */	{ "GLOBALPAGEUNLOCK", 0x0008, 0x0600, 0 },
	/* 0c1 */	{ "", 0, 0, 0 },
	/* 0c2 */	{ "", 0, 0, 0 },
	/* 0c3 */	{ "", 0, 0, 0 },
	/* 0c4 */	{ "SELECTORACCESSRIGHTS", 0x0008, 0x0620, 0 },
	/* 0c5 */	{ "GLOBALFIX", 0x0008, 0x0628, 0 },
	/* 0c6 */	{ "GLOBALUNFIX", 0x0008, 0x0630, 0 },
	/* 0c7 */	{ "SETHANDLECOUNT", 0x0008, 0x0638, SetHandleCount },
	/* 0c8 */	{ "VALIDATEFREESPACES", 0x0008, 0x0640, 0 },
	/* 0c9 */	{ "", 0, 0, 0 },
	/* 0ca */	{ "", 0, 0, 0 },
	/* 0cb */	{ "DEBUGBREAK", 0x0008, 0x0658, 0 },
	/* 0cc */	{ "SWAPRECORDING", 0x0008, 0x0660, 0 },
	/* 0cd */	{ "", 0, 0, 0 },
	/* 0ce */	{ "ALLOCSELECTORARRAY", 0x0008, 0x0670, 0 },
	/* 0cf */	{ "ISDBCSLEADBYTE", 0x0008, 0x0678, IsDBCSLeadByte },
	/* 0d0 */	{ "", 0, 0, 0 },
	/* 0d1 */	{ "", 0, 0, 0 },
	/* 0d2 */	{ "", 0, 0, 0 },
	/* 0d3 */	{ "", 0, 0, 0 },
	/* 0d4 */	{ "", 0, 0, 0 },
	/* 0d5 */	{ "", 0, 0, 0 },
	/* 0d6 */	{ "", 0, 0, 0 },
	/* 0d7 */	{ "", 0, 0, 0 },
	/* 0d8 */	{ "", 0, 0, 0 },
	/* 0d9 */	{ "", 0, 0, 0 },
	/* 0da */	{ "", 0, 0, 0 },
	/* 0db */	{ "", 0, 0, 0 },
	/* 0dc */	{ "", 0, 0, 0 },
	/* 0dd */	{ "", 0, 0, 0 },
	/* 0de */	{ "", 0, 0, 0 },
	/* 0df */	{ "", 0, 0, 0 },
	/* 0e0 */	{ "", 0, 0, 0 },
	/* 0e1 */	{ "", 0, 0, 0 },
	/* 0e2 */	{ "", 0, 0, 0 },
	/* 0e3 */	{ "", 0, 0, 0 },
	/* 0e4 */	{ "", 0, 0, 0 },
	/* 0e5 */	{ "", 0, 0, 0 },
	/* 0e6 */	{ "", 0, 0, 0 },
	/* 0e7 */	{ "", 0, 0, 0 },
	/* 0e8 */	{ "", 0, 0, 0 },
	/* 0e9 */	{ "", 0, 0, 0 },
	/* 0ea */	{ "", 0, 0, 0 },
	/* 0eb */	{ "", 0, 0, 0 },
	/* 0ec */	{ "", 0, 0, 0 },
	/* 0ed */	{ "", 0, 0, 0 },
	/* 0ee */	{ "", 0, 0, 0 },
	/* 0ef */	{ "", 0, 0, 0 },
	/* 0f0 */	{ "", 0, 0, 0 },
	/* 0f1 */	{ "", 0, 0, 0 },
	/* 0f2 */	{ "", 0, 0, 0 },
	/* 0f3 */	{ "", 0, 0, 0 },
	/* 0f4 */	{ "", 0, 0, 0 },
	/* 0f5 */	{ "", 0, 0, 0 },
	/* 0f6 */	{ "", 0, 0, 0 },
	/* 0f7 */	{ "", 0, 0, 0 },
	/* 0f8 */	{ "", 0, 0, 0 },
	/* 0f9 */	{ "", 0, 0, 0 },
	/* 0fa */	{ "", 0, 0, 0 },
	/* 0fb */	{ "", 0, 0, 0 },
	/* 0fc */	{ "", 0, 0, 0 },
	/* 0fd */	{ "", 0, 0, 0 },
	/* 0fe */	{ "", 0, 0, 0 },
	/* 0ff */	{ "", 0, 0, 0 },
	/* 100 */	{ "", 0, 0, 0 },
	/* 101 */	{ "", 0, 0, 0 },
	/* 102 */	{ "", 0, 0, 0 },
	/* 103 */	{ "", 0, 0, 0 },
	/* 104 */	{ "", 0, 0, 0 },
	/* 105 */	{ "", 0, 0, 0 },
	/* 106 */	{ "", 0, 0, 0 },
	/* 107 */	{ "", 0, 0, 0 },
	/* 108 */	{ "", 0, 0, 0 },
	/* 109 */	{ "", 0, 0, 0 },
	/* 10a */	{ "", 0, 0, 0 },
	/* 10b */	{ "", 0, 0, 0 },
	/* 10c */	{ "", 0, 0, 0 },
	/* 10d */	{ "", 0, 0, 0 },
	/* 10e */	{ "", 0, 0, 0 },
	/* 10f */	{ "", 0, 0, 0 },
	/* 110 */	{ "", 0, 0, 0 },
	/* 111 */	{ "", 0, 0, 0 },
	/* 112 */	{ "", 0, 0, 0 },
	/* 113 */	{ "", 0, 0, 0 },
	/* 114 */	{ "", 0, 0, 0 },
	/* 115 */	{ "", 0, 0, 0 },
	/* 116 */	{ "", 0, 0, 0 },
	/* 117 */	{ "", 0, 0, 0 },
	/* 118 */	{ "", 0, 0, 0 },
	/* 119 */	{ "", 0, 0, 0 },
	/* 11a */	{ "", 0, 0, 0 },
	/* 11b */	{ "", 0, 0, 0 },
	/* 11c */	{ "", 0, 0, 0 },
	/* 11d */	{ "", 0, 0, 0 },
	/* 11e */	{ "", 0, 0, 0 },
	/* 11f */	{ "", 0, 0, 0 },
	/* 120 */	{ "", 0, 0, 0 },
	/* 121 */	{ "", 0, 0, 0 },
	/* 122 */	{ "", 0, 0, 0 },
	/* 123 */	{ "", 0, 0, 0 },
	/* 124 */	{ "", 0, 0, 0 },
	/* 125 */	{ "", 0, 0, 0 },
	/* 126 */	{ "", 0, 0, 0 },
	/* 127 */	{ "", 0, 0, 0 },
	/* 128 */	{ "", 0, 0, 0 },
	/* 129 */	{ "", 0, 0, 0 },
	/* 12a */	{ "", 0, 0, 0 },
	/* 12b */	{ "", 0, 0, 0 },
	/* 12c */	{ "", 0, 0, 0 },
	/* 12d */	{ "", 0, 0, 0 },
	/* 12e */	{ "", 0, 0, 0 },
	/* 12f */	{ "", 0, 0, 0 },
	/* 130 */	{ "", 0, 0, 0 },
	/* 131 */	{ "", 0, 0, 0 },
	/* 132 */	{ "", 0, 0, 0 },
	/* 133 */	{ "", 0, 0, 0 },
	/* 134 */	{ "", 0, 0, 0 },
	/* 135 */	{ "", 0, 0, 0 },
	/* 136 */	{ "LOCALHANDLEDELTA", 0x0008, 0x09b0, TWIN_LocalHandleDelta },
	/* 137 */	{ "", 0, 0, 0 },
	/* 138 */	{ "", 0, 0, 0 },
	/* 139 */	{ "", 0, 0, 0 },
	/* 13a */	{ "", 0, 0, 0 },
	/* 13b */	{ "", 0, 0, 0 },
	/* 13c */	{ "", 0, 0, 0 },
	/* 13d */	{ "", 0, 0, 0 },
	/* 13e */	{ "", 0, 0, 0 },
	/* 13f */	{ "", 0, 0, 0 },
	/* 140 */	{ "ISTASK", 0x0008, 0x0a00, IsTask },
	/* 141 */	{ "", 0, 0, 0 },
	/* 142 */	{ "", 0, 0, 0 },
	/* 143 */	{ "ISROMMODULE", 0x0008, 0x0a18, IsROMModule },
	/* 144 */	{ "LOGERROR", 0x0008, 0x0a20, 0 },
	/* 145 */	{ "LOGPARAMERROR", 0x0008, 0x0a28, 0 },
	/* 146 */	{ "ISROMFILE", 0x0008, 0x0a30, IsROMFile },
	/* 147 */	{ "", 0, 0, 0 },
	/* 148 */	{ "", 0, 0, 0 },
	/* 149 */	{ "", 0, 0, 0 },
	/* 14a */	{ "", 0, 0, 0 },
	/* 14b */	{ "", 0, 0, 0 },
	/* 14c */	{ "", 0, 0, 0 },
	/* 14d */	{ "", 0, 0, 0 },
	/* 14e */	{ "ISBADREADPTR", 0x0008, 0x0a70, IsBadPtr },
	/* 14f */	{ "ISBADWRITEPTR", 0x0008, 0x0a78, IsBadPtr },
	/* 150 */	{ "ISBADCODEPTR", 0x0008, 0x0a80, IsBadPtr },
	/* 151 */	{ "ISBADSTRINGPTR", 0x0008, 0x0a88, IsBadPtr },
	/* 152 */	{ "", 0, 0, 0 },
	/* 153 */	{ "", 0, 0, 0 },
	/* 154 */	{ "", 0, 0, 0 },
	/* 155 */	{ "", 0, 0, 0 },
	/* 156 */	{ "", 0, 0, 0 },
	/* 157 */	{ "", 0, 0, 0 },
	/* 158 */	{ "", 0, 0, 0 },
	/* 159 */	{ "", 0, 0, 0 },
	/* 15a */	{ "ISBADHUGEREADPTR", 0x0008, 0x0ad0, IsBadHugePtr },
	/* 15b */	{ "ISBADHUGEWRITEPTR", 0x0008, 0x0ad8, IsBadHugePtr },
	/* 15c */	{ "HMEMCPY", 0x0008, 0x0ae0, hmemcpy },
	/* 15d */	{ "_HREAD", 0x0008, 0x0ae8, _hread},
	/* 15e */	{ "_HWRITE", 0x0008, 0x0af0, _hwrite},
	/* 15f */	{ "", 0, 0, 0 },
	/* 160 */	{ "", 0, 0, 0 },
	/* 161 */	{ "LSTRCPYN", 0x0008, 0x0b08, lstrcpyn },
	/* 162 */	{ "GETAPPCOMPATFLAGS", 0x0008, 0xb10, 0 },
	/* 163 */	{ "GETWINDEBUGINFO", 0x0008, 0x0b18, 0 },
	/* 164 */	{ "SETWINDEBUGINFO", 0x0008, 0x0b20, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "", 0, 0, 0 },
	/* 00b */	{ "", 0, 0, 0 },
	/* 00c */	{ "", 0, 0, 0 },
	/* 00d */	{ "", 0, 0, 0 },
	/* 00e */	{ "", 0, 0, 0 },
	/* 00f */	{ "", 0, 0, 0 },
	/* 010 */	{ "", 0, 0, 0 },
	/* 011 */	{ "", 0, 0, 0 },
	/* 012 */	{ "", 0, 0, 0 },
	/* 013 */	{ "", 0, 0, 0 },
	/* 014 */	{ "", 0, 0, 0 },
	/* 015 */	{ "", 0, 0, 0 },
	/* 016 */	{ "", 0, 0, 0 },
	/* 017 */	{ "", 0, 0, 0 },
	/* 018 */	{ "", 0, 0, 0 },
	/* 019 */	{ "", 0, 0, 0 },
	/* 01a */	{ "", 0, 0, 0 },
	/* 01b */	{ "", 0, 0, 0 },
	/* 01c */	{ "", 0, 0, 0 },
	/* 01d */	{ "", 0, 0, 0 },
	/* 01e */	{ "", 0, 0, 0 },
	/* 01f */	{ "", 0, 0, 0 },
	/* 020 */	{ "", 0, 0, 0 },
	/* 021 */	{ "", 0, 0, 0 },
	/* 022 */	{ "", 0, 0, 0 },
	/* 023 */	{ "", 0, 0, 0 },
	/* 024 */	{ "", 0, 0, 0 },
	/* 025 */	{ "", 0, 0, 0 },
	/* 026 */	{ "", 0, 0, 0 },
	/* 027 */	{ "", 0, 0, 0 },
	/* 028 */	{ "", 0, 0, 0 },
	/* 029 */	{ "", 0, 0, 0 },
	/* 02a */	{ "", 0, 0, 0 },
	/* 02b */	{ "", 0, 0, 0 },
	/* 02c */	{ "", 0, 0, 0 },
	/* 02d */	{ "", 0, 0, 0 },
	/* 02e */	{ "", 0, 0, 0 },
	/* 02f */	{ "", 0, 0, 0 },
	/* 030 */	{ "", 0, 0, 0 },
	/* 031 */	{ "", 0, 0, 0 },
	/* 032 */	{ "", 0, 0, 0 },
	/* 033 */	{ "", 0, 0, 0 },
	/* 034 */	{ "", 0, 0, 0 },
	/* 035 */	{ "", 0, 0, 0 },
	/* 036 */	{ "", 0, 0, 0 },
	/* 037 */	{ "", 0, 0, 0 },
	/* 038 */	{ "", 0, 0, 0 },
	/* 039 */	{ "", 0, 0, 0 },
	/* 03a */	{ "", 0, 0, 0 },
	/* 03b */	{ "", 0, 0, 0 },
	/* 03c */	{ "", 0, 0, 0 },
	/* 03d */	{ "", 0, 0, 0 },
	/* 03e */	{ "", 0, 0, 0 },
	/* 03f */	{ "", 0, 0, 0 },
	/* 040 */	{ "", 0, 0, 0 },
	/* 041 */	{ "", 0, 0, 0 },
	/* 042 */	{ "", 0, 0, 0 },
	/* 043 */	{ "", 0, 0, 0 },
	/* 044 */	{ "", 0, 0, 0 },
	/* 045 */	{ "", 0, 0, 0 },
	/* 046 */	{ "", 0, 0, 0 },
	/* 047 */	{ "", 0, 0, 0 },
	/* 048 */	{ "", 0, 0, 0 },
	/* 049 */	{ "", 0, 0, 0 },
	/* 04a */	{ "", 0, 0, 0 },
	/* 04b */	{ "", 0, 0, 0 },
	/* 04c */	{ "", 0, 0, 0 },
	/* 04d */	{ "", 0, 0, 0 },
	/* 04e */	{ "", 0, 0, 0 },
	/* 04f */	{ "", 0, 0, 0 },
	/* 050 */	{ "", 0, 0, 0 },
	/* 051 */	{ "", 0, 0, 0 },
	/* 052 */	{ "", 0, 0, 0 },
	/* 053 */	{ "", 0, 0, 0 },
	/* 054 */	{ "", 0, 0, 0 },
	/* 055 */	{ "", 0, 0, 0 },
	/* 056 */	{ "", 0, 0, 0 },
	/* 057 */	{ "", 0, 0, 0 },
	/* 058 */	{ "", 0, 0, 0 },
	/* 059 */	{ "", 0, 0, 0 },
	/* 05a */	{ "", 0, 0, 0 },
	/* 05b */	{ "", 0, 0, 0 },
	/* 05c */	{ "", 0, 0, 0 },
	/* 05d */	{ "", 0, 0, 0 },
	/* 05e */	{ "", 0, 0, 0 },
	/* 05f */	{ "", 0, 0, 0 },
	/* 060 */	{ "", 0, 0, 0 },
	/* 061 */	{ "", 0, 0, 0 },
	/* 062 */	{ "", 0, 0, 0 },
	/* 063 */	{ "", 0, 0, 0 },
	/* 064 */	{ "", 0, 0, 0 },
	/* 065 */	{ "", 0, 0, 0 },
	/* 066 */	{ "", 0, 0, 0 },
	/* 067 */	{ "", 0, 0, 0 },
	/* 068 */	{ "", 0, 0, 0 },
	/* 069 */	{ "", 0, 0, 0 },
	/* 06a */	{ "", 0, 0, 0 },
	/* 06b */	{ "", 0, 0, 0 },
	/* 06c */	{ "", 0, 0, 0 },
	/* 06d */	{ "", 0, 0, 0 },
	/* 06e */	{ "", 0, 0, 0 },
	/* 06f */	{ "", 0, 0, 0 },
	/* 070 */	{ "", 0, 0, 0 },
	/* 071 */	{ "", 0, 0, 0 },
	/* 072 */	{ "", 0, 0, 0 },
	/* 073 */	{ "", 0, 0, 0 },
	/* 074 */	{ "", 0, 0, 0 },
	/* 075 */	{ "", 0, 0, 0 },
	/* 076 */	{ "", 0, 0, 0 },
	/* 077 */	{ "", 0, 0, 0 },
	/* 078 */	{ "", 0, 0, 0 },
	/* 079 */	{ "", 0, 0, 0 },
	/* 07a */	{ "", 0, 0, 0 },
	/* 07b */	{ "", 0, 0, 0 },
	/* 07c */	{ "", 0, 0, 0 },
	/* 07d */	{ "", 0, 0, 0 },
	/* 07e */	{ "", 0, 0, 0 },
	/* 07f */	{ "", 0, 0, 0 },
	/* 080 */	{ "", 0, 0, 0 },
	/* 081 */	{ "", 0, 0, 0 },
	/* 082 */	{ "", 0, 0, 0 },
	/* 083 */	{ "", 0, 0, 0 },
	/* 084 */	{ "", 0, 0, 0 },
	/* 085 */	{ "", 0, 0, 0 },
	/* 086 */	{ "", 0, 0, 0 },
	/* 087 */	{ "", 0, 0, 0 },
	/* 088 */	{ "", 0, 0, 0 },
	/* 089 */	{ "", 0, 0, 0 },
	/* 08a */	{ "", 0, 0, 0 },
	/* 08b */	{ "", 0, 0, 0 },
	/* 08c */	{ "", 0, 0, 0 },
	/* 08d */	{ "", 0, 0, 0 },
	/* 08e */	{ "", 0, 0, 0 },
	/* 08f */	{ "", 0, 0, 0 },
	/* 090 */	{ "", 0, 0, 0 },
	/* 091 */	{ "", 0, 0, 0 },
	/* 092 */	{ "", 0, 0, 0 },
	/* 093 */	{ "", 0, 0, 0 },
	/* 094 */	{ "", 0, 0, 0 },
	/* 095 */	{ "", 0, 0, 0 },
	/* 096 */	{ "", 0, 0, 0 },
	/* 097 */	{ "", 0, 0, 0 },
	/* 098 */	{ "", 0, 0, 0 },
	/* 099 */	{ "", 0, 0, 0 },
	/* 09a */	{ "", 0, 0, 0 },
	/* 09b */	{ "", 0, 0, 0 },
	/* 09c */	{ "", 0, 0, 0 },
	/* 09d */	{ "", 0, 0, 0 },
	/* 09e */	{ "", 0, 0, 0 },
	/* 09f */	{ "", 0, 0, 0 },
	/* 0a0 */	{ "", 0, 0, 0 },
	/* 0a1 */	{ "", 0, 0, 0 },
	/* 0a2 */	{ "", 0, 0, 0 },
	/* 0a3 */	{ "", 0, 0, 0 },
	/* 0a4 */	{ "", 0, 0, 0 },
	/* 0a5 */	{ "", 0, 0, 0 },
	/* 0a6 */	{ "", 0, 0, 0 },
	/* 0a7 */	{ "", 0, 0, 0 },
	/* 0a8 */	{ "", 0, 0, 0 },
	/* 0a9 */	{ "", 0, 0, 0 },
	/* 0aa */	{ "", 0, 0, 0 },
	/* 0ab */	{ "", 0, 0, 0 },
	/* 0ac */	{ "", 0, 0, 0 },
	/* 0ad */	{ "", 0, 0, 0 },
	/* 0ae */	{ "", 0, 0, 0 },
	/* 0af */	{ "", 0, 0, 0 },
	/* 0b0 */	{ "", 0, 0, 0 },
	/* 0b1 */	{ "", 0, 0, 0 },
	/* 0b2 */	{ "", 0, 0, 0 },
	/* 0b3 */	{ "", 0, 0, 0 },
	/* 0b4 */	{ "", 0, 0, 0 },
	/* 0b5 */	{ "", 0, 0, 0 },
	/* 0b6 */	{ "", 0, 0, 0 },
	/* 0b7 */	{ "", 0, 0, 0 },
	/* 0b8 */	{ "", 0, 0, 0 },
	/* 0b9 */	{ "", 0, 0, 0 },
	/* 0ba */	{ "", 0, 0, 0 },
	/* 0bb */	{ "", 0, 0, 0 },
	/* 0bc */	{ "", 0, 0, 0 },
	/* 0bd */	{ "", 0, 0, 0 },
	/* 0be */	{ "", 0, 0, 0 },
	/* 0bf */	{ "", 0, 0, 0 },
	/* 0c0 */	{ "", 0, 0, 0 },
	/* 0c1 */	{ "", 0, 0, 0 },
	/* 0c2 */	{ "", 0, 0, 0 },
	/* 0c3 */	{ "", 0, 0, 0 },
	/* 0c4 */	{ "", 0, 0, 0 },
	/* 0c5 */	{ "", 0, 0, 0 },
	/* 0c6 */	{ "", 0, 0, 0 },
	/* 0c7 */	{ "", 0, 0, 0 },
	/* 0c8 */	{ "", 0, 0, 0 },
	/* 0c9 */	{ "", 0, 0, 0 },
	/* 0ca */	{ "", 0, 0, 0 },
	/* 0cb */	{ "", 0, 0, 0 },
	/* 0cc */	{ "", 0, 0, 0 },
	/* 0cd */	{ "", 0, 0, 0 },
	/* 0ce */	{ "", 0, 0, 0 },
	/* 0cf */	{ "", 0, 0, 0 },
	/* 0d0 */	{ "", 0, 0, 0 },
	/* 0d1 */	{ "", 0, 0, 0 },
	/* 0d2 */	{ "", 0, 0, 0 },
	/* 0d3 */	{ "", 0, 0, 0 },
	/* 0d4 */	{ "", 0, 0, 0 },
	/* 0d5 */	{ "", 0, 0, 0 },
	/* 0d6 */	{ "", 0, 0, 0 },
	/* 0d7 */	{ "", 0, 0, 0 },
	/* 0d8 */	{ "", 0, 0, 0 },
	/* 0d9 */	{ "", 0, 0, 0 },
	/* 0da */	{ "", 0, 0, 0 },
	/* 0db */	{ "", 0, 0, 0 },
	/* 0dc */	{ "", 0, 0, 0 },
	/* 0dd */	{ "", 0, 0, 0 },
	/* 0de */	{ "", 0, 0, 0 },
	/* 0df */	{ "", 0, 0, 0 },
	/* 0e0 */	{ "", 0, 0, 0 },
	/* 0e1 */	{ "", 0, 0, 0 },
	/* 0e2 */	{ "", 0, 0, 0 },
	/* 0e3 */	{ "", 0, 0, 0 },
	/* 0e4 */	{ "", 0, 0, 0 },
	/* 0e5 */	{ "", 0, 0, 0 },
	/* 0e6 */	{ "", 0, 0, 0 },
	/* 0e7 */	{ "", 0, 0, 0 },
	/* 0e8 */	{ "", 0, 0, 0 },
	/* 0e9 */	{ "", 0, 0, 0 },
	/* 0ea */	{ "", 0, 0, 0 },
	/* 0eb */	{ "", 0, 0, 0 },
	/* 0ec */	{ "", 0, 0, 0 },
	/* 0ed */	{ "", 0, 0, 0 },
	/* 0ee */	{ "", 0, 0, 0 },
	/* 0ef */	{ "", 0, 0, 0 },
	/* 0f0 */	{ "", 0, 0, 0 },
	/* 0f1 */	{ "", 0, 0, 0 },
	/* 0f2 */	{ "", 0, 0, 0 },
	/* 0f3 */	{ "", 0, 0, 0 },
	/* 0f4 */	{ "", 0, 0, 0 },
	/* 0f5 */	{ "", 0, 0, 0 },
	/* 0f6 */	{ "", 0, 0, 0 },
	/* 0f7 */	{ "", 0, 0, 0 },
	/* 0f8 */	{ "", 0, 0, 0 },
	/* 0f9 */	{ "", 0, 0, 0 },
	/* 0fa */	{ "", 0, 0, 0 },
	/* 0fb */	{ "", 0, 0, 0 },
	/* 0fc */	{ "", 0, 0, 0 },
	/* 0fd */	{ "", 0, 0, 0 },
	/* 0fe */	{ "", 0, 0, 0 },
	/* 0ff */	{ "", 0, 0, 0 },
	/* 100 */	{ "", 0, 0, 0 },
	/* 101 */	{ "", 0, 0, 0 },
	/* 102 */	{ "", 0, 0, 0 },
	/* 103 */	{ "", 0, 0, 0 },
	/* 104 */	{ "", 0, 0, 0 },
	/* 105 */	{ "", 0, 0, 0 },
	/* 106 */	{ "", 0, 0, 0 },
	/* 107 */	{ "", 0, 0, 0 },
	/* 108 */	{ "", 0, 0, 0 },
	/* 109 */	{ "", 0, 0, 0 },
	/* 10a */	{ "", 0, 0, 0 },
	/* 10b */	{ "", 0, 0, 0 },
	/* 10c */	{ "", 0, 0, 0 },
	/* 10d */	{ "", 0, 0, 0 },
	/* 10e */	{ "", 0, 0, 0 },
	/* 10f */	{ "", 0, 0, 0 },
	/* 110 */	{ "", 0, 0, 0 },
	/* 111 */	{ "", 0, 0, 0 },
	/* 112 */	{ "", 0, 0, 0 },
	/* 113 */	{ "", 0, 0, 0 },
	/* 114 */	{ "", 0, 0, 0 },
	/* 115 */	{ "", 0, 0, 0 },
	/* 116 */	{ "", 0, 0, 0 },
	/* 117 */	{ "", 0, 0, 0 },
	/* 118 */	{ "", 0, 0, 0 },
	/* 119 */	{ "", 0, 0, 0 },
	/* 11a */	{ "", 0, 0, 0 },
	/* 11b */	{ "", 0, 0, 0 },
	/* 11c */	{ "", 0, 0, 0 },
	/* 11d */	{ "", 0, 0, 0 },
	/* 11e */	{ "", 0, 0, 0 },
	/* 11f */	{ "", 0, 0, 0 },
	/* 120 */	{ "", 0, 0, 0 },
	/* 121 */	{ "", 0, 0, 0 },
	/* 122 */	{ "", 0, 0, 0 },
	/* 123 */	{ "", 0, 0, 0 },
	/* 124 */	{ "", 0, 0, 0 },
	/* 125 */	{ "", 0, 0, 0 },
	/* 126 */	{ "", 0, 0, 0 },
	/* 127 */	{ "", 0, 0, 0 },
	/* 128 */	{ "", 0, 0, 0 },
	/* 129 */	{ "", 0, 0, 0 },
	/* 12a */	{ "", 0, 0, 0 },
	/* 12b */	{ "", 0, 0, 0 },
	/* 12c */	{ "", 0, 0, 0 },
	/* 12d */	{ "", 0, 0, 0 },
	/* 12e */	{ "", 0, 0, 0 },
	/* 12f */	{ "", 0, 0, 0 },
	/* 130 */	{ "", 0, 0, 0 },
	/* 131 */	{ "", 0, 0, 0 },
	/* 132 */	{ "", 0, 0, 0 },
	/* 133 */	{ "", 0, 0, 0 },
	/* 134 */	{ "", 0, 0, 0 },
	/* 135 */	{ "", 0, 0, 0 },
	/* 136 */	{ "", 0, 0, 0 },
	/* 137 */	{ "", 0, 0, 0 },
	/* 138 */	{ "", 0, 0, 0 },
	/* 139 */	{ "", 0, 0, 0 },
	/* 13a */	{ "", 0, 0, 0 },
	/* 13b */	{ "", 0, 0, 0 },
	/* 13c */	{ "", 0, 0, 0 },
	/* 13d */	{ "", 0, 0, 0 },
	/* 13e */	{ "", 0, 0, 0 },
	/* 13f */	{ "", 0, 0, 0 },
	/* 140 */	{ "", 0, 0, 0 },
	/* 141 */	{ "", 0, 0, 0 },
	/* 142 */	{ "", 0, 0, 0 },
	/* 143 */	{ "", 0, 0, 0 },
	/* 144 */	{ "", 0, 0, 0 },
	/* 145 */	{ "", 0, 0, 0 },
	/* 146 */	{ "", 0, 0, 0 },
	/* 147 */	{ "", 0, 0, 0 },
	/* 148 */	{ "", 0, 0, 0 },
	/* 149 */	{ "", 0, 0, 0 },
	/* 14a */	{ "", 0, 0, 0 },
	/* 14b */	{ "", 0, 0, 0 },
	/* 14c */	{ "", 0, 0, 0 },
	/* 14d */	{ "", 0, 0, 0 },
	/* 14e */	{ "", 0, 0, 0 },
	/* 14f */	{ "", 0, 0, 0 },
	/* 150 */	{ "", 0, 0, 0 },
	/* 151 */	{ "", 0, 0, 0 },
	/* 152 */	{ "", 0, 0, 0 },
	/* 153 */	{ "", 0, 0, 0 },
	/* 154 */	{ "", 0, 0, 0 },
	/* 155 */	{ "", 0, 0, 0 },
	/* 156 */	{ "", 0, 0, 0 },
	/* 157 */	{ "", 0, 0, 0 },
	/* 158 */	{ "", 0, 0, 0 },
	/* 159 */	{ "", 0, 0, 0 },
	/* 15a */	{ "", 0, 0, 0 },
	/* 15b */	{ "", 0, 0, 0 },
	/* 15c */	{ "", 0, 0, 0 },
	/* 15d */	{ "", 0, 0, 0 },
	/* 15e */	{ "", 0, 0, 0 },
	/* 15f */	{ "", 0, 0, 0 },
	/* 160 */	{ "", 0, 0, 0 },
	/* 161 */	{ "", 0, 0, 0 },
	/* 162 */	{ "", 0, 0, 0 },
	/* 163 */	{ "", 0, 0, 0 },
	/* 164 */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif

extern long int IT_1I();
extern long int IT_V();
extern long int IT_LOCALINIT();
extern long int IT_LOCALALLOC();
extern long int IT_LOCALREALLOC();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1H();
extern long int _86_LocalNotify();
extern long int IT_GLOBALALLOC();
extern long int IT_GLOBALREALLOC();
extern long int IT_GLOBALFREE();
extern long int IT_GLOBALLOCK();
extern long int IT_GLOBALUNLOCK();
extern long int IT_GLOBALSIZE();
extern long int _86_GlobalHandle();
extern long int IT_GLOBALUNLOCK();
extern long int IT_LOCKSEGMENT();
extern long int IT_1UI();
extern long int IT_1UI();
extern long int IT_1D();
extern long int IT_V();
extern long int IT_1H();
extern long int IT_V();
extern long int _86_GetCurrentPDB();
extern long int IT_1LP();
extern long int IT_1H();
extern long int IT_1H1LP1I();
extern long int IT_GETMODULEFNAME();
extern long int IT_GETPADDR();
extern long int IT_MAKEPRIN();
extern long int IT_FREEPRIN();
extern long int IT_1H1LP1I();
extern long int IT_2LP1I();
extern long int IT_4LP1I();
extern long int IT_3LP();
extern long int IT_1H2LP();
extern long int IT_LOADRESOURCE();
extern long int IT_LOCKRESOURCE();
extern long int IT_FREERESOURCE();
extern long int IT_ACCESSRESOURCE();
extern long int IT_SIZEOFRESOURCE();
extern long int IT_2H();
extern long int IT_1I();
extern long int IT_1LP();
extern long int IT_1LP();
extern long int IT_1H();
extern long int IT_1H1LP1I();
extern long int IT_1H();
extern long int IT_OPENFILE();
extern long int IT_LP1LP();
extern long int IT_2LP();
extern long int IT_1H();
extern long int IT_LREADWR();
extern long int IT_1LP1I();
extern long int IT_1H1L1I();
extern long int IT_LOPEN();
extern long int IT_LP2LP();
extern long int IT_LP2LP();
extern long int IT_1LP();
extern long int IT_INITTASK();
extern long int IT_LOADMODULE();
extern long int IT_GETMODULEHANDLE();
extern long int _86_GetCodeHandle();
extern long int IT_1H();
extern long int IT_LOADLIBRARY();
extern long int IT_V();
extern long int IT_GETTEMPFN();
extern long int _86_DOS3Call();
extern long int _86_NetBIOSCall();
extern long int _86_Catch();
extern long int _86_Throw();
extern long int IT_1H();
extern long int IT_1UI();
extern long int IT_1LP();
extern long int IT_1LPSTR();
extern long int IT_1H1UI();
extern long int IT_GETPRIVATEPRINT();
extern long int IT_GETPRIVATEPRSTR();
extern long int IT_WRITEPRIVPRSTR();
extern long int IT_GETDOSENV();
extern long int IT_V();
extern long int IT_1LP1I();
extern long int IT_1I();
extern long int IT_1UI1LP();
extern long int _86_GetHeapSpaces();
extern long int IT_V();
extern long int IT_GLOBALLRU();
extern long int IT_GLOBALLRU();
extern long int IT_WINEXEC();
extern long int _86_AllocDStoCSAlias();
extern long int _86_AllocCStoDSAlias();
extern long int _86_AllocSelector();
extern long int _86_AllocSelectorArray();
extern long int _86_FreeSelector();
extern long int _86_PrestoChangoSelector();
extern long int _86_GlobalDosAlloc();
extern long int _86_GlobalDosFree();
extern long int _86_GlobalPageUnlock();
extern long int _86_GlobalUnfix();
extern long int _86_GetSelectorBase();
extern long int _86_SetSelectorBase();
extern long int _86_GetSelectorLimit();
extern long int _86_SetSelectorLimit();
extern long int _86_SelectorAccessRights();
extern long int IT_GLOBALNOTIFY();
extern long int IT_1UI();
extern long int IT_1UI();
extern long int IT_1B();
extern long int IT_GETDIR();
extern long int IT_ISBADPTR();
extern long int IT_ISBADHUGEPTR();
extern long int IT_ISBADCODEPTR();
extern long int _86_GetAppCompatFlags();
extern long int IT_1H1LP();
extern long int IT_2LP1L();
extern long int _86_GetInstanceData();
extern long int IT_HREADWR();
extern long int IT_GETCURTASK();
extern long int IT_1HTASK();
extern long int IT_TIMERCOUNT();
extern long int IT_WINHELP();
extern long int IT_2UI();
extern long int IT_2LP1UI1H();




 /* Interface Segment Image KERNEL: */

static long int (*seg_image_KERNEL_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	IT_1I,  FatalExit,
	/* 002 */	Trap, 0,
	/* 003 */	IT_V,  GetVersion,
	/* 004 */	IT_LOCALINIT,  TWIN_LocalInit,
	/* 005 */	IT_LOCALALLOC,  TWIN_LocalAlloc,
	/* 006 */	IT_LOCALREALLOC,  TWIN_LocalReAlloc,
	/* 007 */	IT_1UI,  TWIN_LocalFree,
	/* 008 */	IT_1UI,  TWIN_LocalLock,
	/* 009 */	IT_1UI,  TWIN_LocalUnlock,
	/* 00a */	IT_1UI,  TWIN_LocalSize,
	/* 00b */	IT_1UI, TWIN_LocalHandle,
	/* 00c */	Trap, 0,
	/* 00d */	IT_1UI, TWIN_LocalCompact,
	/* 00e */	_86_LocalNotify, 0,
	/* 00f */	IT_GLOBALALLOC,  GlobalAlloc,
	/* 010 */	IT_GLOBALREALLOC,  GlobalReAlloc,
	/* 011 */	IT_GLOBALFREE,  GlobalFree,
	/* 012 */	IT_GLOBALLOCK,  GlobalLock,
	/* 013 */	IT_GLOBALUNLOCK,  GlobalUnlock,
	/* 014 */	IT_GLOBALSIZE,  GlobalSize,
	/* 015 */	_86_GlobalHandle, 0,
	/* 016 */	IT_GLOBALUNLOCK,  GlobalFlags,
	/* 017 */	IT_LOCKSEGMENT,  LockSegment,
	/* 018 */	IT_1UI,  UnlockSegment,
	/* 019 */	IT_1D,  GlobalCompact,
	/* 01a */	Trap, 0,
	/* 01b */	Trap, 0,
	/* 01c */	Trap, 0,
	/* 01d */	IT_V,  Yield,
	/* 01e */	IT_1H,  WaitEvent,
	/* 01f */	Trap, 0,
	/* 020 */	Trap, 0,
	/* 021 */	Trap, 0,
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	IT_GETCURTASK,  GetCurrentTask,
	/* 025 */	_86_GetCurrentPDB, 0,
	/* 026 */	Trap, 0,
	/* 027 */	Trap, 0,
	/* 028 */	Trap, 0,
	/* 029 */	Trap, 0,
	/* 02a */	Trap, 0,
	/* 02b */	Trap, 0,
	/* 02c */	Trap, 0,
	/* 02d */	IT_LOADMODULE, LoadModule,
	/* 02e */	Trap, 0,
	/* 02f */	IT_GETMODULEHANDLE,  GetModuleHandle,
	/* 030 */	IT_1H,  GetModuleUsage,
	/* 031 */	IT_GETMODULEFNAME,  GetModuleFileName,
	/* 032 */	IT_GETPADDR,  GetProcAddress,
	/* 033 */	IT_MAKEPRIN,  MakeProcInstance,
	/* 034 */	IT_FREEPRIN,  FreeProcInstance,
	/* 035 */	Trap, 0,
	/* 036 */	_86_GetInstanceData,  0,
	/* 037 */	_86_Catch, 0,
	/* 038 */	_86_Throw, 0,
	/* 039 */	IT_2LP1I,  GetProfileInt,
	/* 03a */	IT_4LP1I,  GetProfileString,
	/* 03b */	IT_3LP,  WriteProfileString,
	/* 03c */	IT_1H2LP,  FindResource,
	/* 03d */	IT_LOADRESOURCE,  LoadResource,
	/* 03e */	IT_LOCKRESOURCE,  LockResource,
	/* 03f */	IT_FREERESOURCE,  FreeResource,
	/* 040 */	IT_ACCESSRESOURCE,  AccessResource,
	/* 041 */	IT_SIZEOFRESOURCE, 0,
	/* 042 */	Trap, 0,
	/* 043 */	Trap, 0,
	/* 044 */	IT_1I,  InitAtomTable,
	/* 045 */	IT_1LP,  FindAtom,
	/* 046 */	IT_1LP,  AddAtom,
	/* 047 */	IT_1H,  DeleteAtom,
	/* 048 */	IT_1H1LP1I,  GetAtomName,
#ifdef	LATER
	/* 049 */	IT_1H,  GetAtomHandle,
#else
	/* 049 */	Trap,  0,
#endif
	/* 04a */	IT_OPENFILE,  OpenFile,
	/* 04b */	Trap, 0,
	/* 04c */	Trap, 0,
	/* 04d */	IT_LP1LP,  AnsiNext,
	/* 04e */	IT_LP2LP,  AnsiPrev,
	/* 04f */	IT_LP1LP,  AnsiUpper,
	/* 050 */	IT_LP1LP,  AnsiLower,
	/* 051 */	IT_1I,  _lclose,
	/* 052 */	IT_LREADWR,  _lread,
	/* 053 */	IT_1LP1I,  _lcreat,
	/* 054 */	IT_1H1L1I,  _llseek,
	/* 055 */	IT_LOPEN,  _lopen,
	/* 056 */	IT_LREADWR,  _lwrite,
	/* 057 */	IT_2LP, lstrcmp,
	/* 058 */	IT_LP2LP,  lstrcpy,
	/* 059 */	IT_LP2LP,  lstrcat,
	/* 05a */	IT_1LPSTR,  lstrlen,
	/* 05b */	IT_INITTASK,  InitTask,
	/* 05c */	Trap, 0,
	/* 05d */	_86_GetCodeHandle, 0,
	/* 05e */	IT_1H,  DefineHandleTable,
	/* 05f */	IT_LOADLIBRARY,  LoadLibrary,
	/* 060 */	IT_1H,  FreeLibrary,
#ifdef TWIN32
	/* 061 */	IT_GETTEMPFN,  GetTempFileName16,
#else
	/* 061 */	IT_GETTEMPFN,  GetTempFileName,
#endif
	/* 062 */	Trap, 0,
	/* 063 */	Trap, 0,
	/* 064 */	Trap, 0,
	/* 065 */	Trap, 0,
	/* 066 */	_86_DOS3Call, 0,
	/* 067 */	_86_NetBIOSCall, 0,
	/* 068 */	Trap, 0,
	/* 069 */	Trap, 0,
	/* 06a */	IT_1H,  SetSwapAreaSize,
	/* 06b */	IT_1UI,  SetErrorMode,
	/* 06c */	Trap, 0,
	/* 06d */	Trap, 0,
	/* 06e */	Trap, 0,
	/* 06f */	IT_GLOBALLOCK, GlobalLock,
	/* 070 */	IT_GLOBALUNLOCK, GlobalUnlock,
	/* 071 */	Trap, 0,
	/* 072 */	Trap, 0,
	/* 073 */	IT_1LP,  OutputDebugString,
	/* 074 */	Trap, 0,
	/* 075 */	Trap, 0,
	/* 076 */	Trap, 0,
	/* 077 */	Trap, 0,
	/* 078 */	Trap, 0,
	/* 079 */	IT_2UI,  TWIN_LocalShrink,
	/* 07a */	Trap, 0,
	/* 07b */	Trap, 0,
	/* 07c */	Trap, 0,
	/* 07d */	Trap, 0,
	/* 07e */	Trap, 0,
	/* 07f */	IT_GETPRIVATEPRINT,  GetPrivateProfileInt,
	/* 080 */	IT_GETPRIVATEPRSTR,  GetPrivateProfileString,
	/* 081 */	IT_WRITEPRIVPRSTR,  WritePrivateProfileString,
	/* 082 */	IT_1LP, FileCdr, /* !!! should be 1LPFN */
	/* 083 */	IT_GETDOSENV,  GetDOSEnvironment,
	/* 084 */	IT_V,  GetWinFlags,
	/* 085 */	Trap, 0,
	/* 086 */	IT_GETDIR,  GetWindowsDirectory,
	/* 087 */	IT_GETDIR,  GetSystemDirectory,
#ifdef TWIN32
	/* 088 */	IT_1I,  GetDriveType16,
#else
	/* 088 */	IT_1I,  GetDriveType,
#endif
	/* 089 */	IT_1UI1LP,  FatalAppExit,
	/* 08a */	_86_GetHeapSpaces, 0,
	/* 08b */	Trap, 0,
	/* 08c */	Trap, 0,
	/* 08d */	Trap, 0,
	/* 08e */	Trap, 0,
	/* 08f */	Trap, 0,
	/* 090 */	Trap, 0,
	/* 091 */	Trap, 0,
	/* 092 */	Trap, 0,
	/* 093 */	Trap, 0,
	/* 094 */	Trap, 0,
	/* 095 */	Trap, 0,
	/* 096 */	Trap, 0,
	/* 097 */	Trap, 0,
	/* 098 */	IT_V,  GetNumTasks,
	/* 099 */	Trap, 0,
	/* 09a */	IT_GLOBALNOTIFY, GlobalNotify,
	/* 09b */	Trap, 0,
	/* 09c */	Trap, 0,
	/* 09d */	Trap, 0,
	/* 09e */	Trap, 0,
	/* 09f */	Trap, 0,
	/* 0a0 */	Trap, 0,
	/* 0a1 */	Trap, 0,
	/* 0a2 */	Trap, 0,
	/* 0a3 */	IT_GLOBALLRU,  GlobalLRUOldest,
	/* 0a4 */	IT_GLOBALLRU,  GlobalLRUNewest,
	/* 0a5 */	Trap, 0,
	/* 0a6 */	IT_WINEXEC, WinExec,
	/* 0a7 */	Trap, 0,
	/* 0a8 */	Trap, 0,
	/* 0a9 */	IT_1UI,  GetFreeSpace,
	/* 0aa */	_86_AllocCStoDSAlias, 0,
	/* 0ab */	_86_AllocDStoCSAlias, 0,
	/* 0ac */	Trap, 0,
	/* 0ad */	Trap, 0,
	/* 0ae */	Trap, 0,
	/* 0af */	_86_AllocSelector, 0,
	/* 0b0 */	_86_FreeSelector, 0,
	/* 0b1 */	_86_PrestoChangoSelector, 0,
	/* 0b2 */	Trap, 0,
	/* 0b3 */	Trap, 0,
	/* 0b4 */	Trap, 0,
	/* 0b5 */	Trap, 0,
	/* 0b6 */	Trap, 0,
	/* 0b7 */	Trap, 0,
	/* 0b8 */	_86_GlobalDosAlloc, 0,
	/* 0b9 */	_86_GlobalDosFree, 0,
	/* 0ba */	_86_GetSelectorBase, 0,
	/* 0bb */	_86_SetSelectorBase, 0,
	/* 0bc */	_86_GetSelectorLimit, 0,
	/* 0bd */	_86_SetSelectorLimit, 0,
	/* 0be */	Trap, 0,
	/* 0bf */	IT_GLOBALLOCK, GlobalLock,
	/* 0c0 */	_86_GlobalPageUnlock, 0,
	/* 0c1 */	Trap, 0,
	/* 0c2 */	Trap, 0,
	/* 0c3 */	Trap, 0,
	/* 0c4 */	_86_SelectorAccessRights, 0,
	/* 0c5 */	IT_GLOBALLOCK, GlobalLock,
	/* 0c6 */	_86_GlobalUnfix, 0,
	/* 0c7 */	IT_1UI,  SetHandleCount,
	/* 0c8 */	Trap, 0,
	/* 0c9 */	Trap, 0,
	/* 0ca */	Trap, 0,
	/* 0cb */	Trap, 0,
	/* 0cc */	Trap, 0,
	/* 0cd */	Trap, 0,
	/* 0ce */	_86_AllocSelectorArray, 0,
	/* 0cf */	IT_1B,  IsDBCSLeadByte,
	/* 0d0 */	Trap, 0,
	/* 0d1 */	Trap, 0,
	/* 0d2 */	Trap, 0,
	/* 0d3 */	Trap, 0,
	/* 0d4 */	Trap, 0,
	/* 0d5 */	Trap, 0,
	/* 0d6 */	Trap, 0,
	/* 0d7 */	Trap, 0,
	/* 0d8 */	Trap, 0,
	/* 0d9 */	Trap, 0,
	/* 0da */	Trap, 0,
	/* 0db */	Trap, 0,
	/* 0dc */	Trap, 0,
	/* 0dd */	Trap, 0,
	/* 0de */	Trap, 0,
	/* 0df */	Trap, 0,
	/* 0e0 */	Trap, 0,
	/* 0e1 */	Trap, 0,
	/* 0e2 */	Trap, 0,
	/* 0e3 */	Trap, 0,
	/* 0e4 */	Trap, 0,
	/* 0e5 */	Trap, 0,
	/* 0e6 */	Trap, 0,
	/* 0e7 */	Trap, 0,
	/* 0e8 */	Trap, 0,
	/* 0e9 */	Trap, 0,
	/* 0ea */	Trap, 0,
	/* 0eb */	Trap, 0,
	/* 0ec */	Trap, 0,
	/* 0ed */	Trap, 0,
	/* 0ee */	Trap, 0,
	/* 0ef */	Trap, 0,
	/* 0f0 */	Trap, 0,
	/* 0f1 */	Trap, 0,
	/* 0f2 */	Trap, 0,
	/* 0f3 */	Trap, 0,
	/* 0f4 */	Trap, 0,
	/* 0f5 */	Trap, 0,
	/* 0f6 */	Trap, 0,
	/* 0f7 */	Trap, 0,
	/* 0f8 */	Trap, 0,
	/* 0f9 */	Trap, 0,
	/* 0fa */	Trap, 0,
	/* 0fb */	Trap, 0,
	/* 0fc */	Trap, 0,
	/* 0fd */	Trap, 0,
	/* 0fe */	Trap, 0,
	/* 0ff */	Trap, 0,
	/* 100 */	Trap, 0,
	/* 101 */	Trap, 0,
	/* 102 */	Trap, 0,
	/* 103 */	Trap, 0,
	/* 104 */	Trap, 0,
	/* 105 */	Trap, 0,
	/* 106 */	Trap, 0,
	/* 107 */	Trap, 0,
	/* 108 */	Trap, 0,
	/* 109 */	Trap, 0,
	/* 10a */	Trap, 0,
	/* 10b */	Trap, 0,
	/* 10c */	Trap, 0,
	/* 10d */	Trap, 0,
	/* 10e */	Trap, 0,
	/* 10f */	Trap, 0,
	/* 110 */	Trap, 0,
	/* 111 */	Trap, 0,
	/* 112 */	Trap, 0,
	/* 113 */	Trap, 0,
	/* 114 */	Trap, 0,
	/* 115 */	Trap, 0,
	/* 116 */	Trap, 0,
	/* 117 */	Trap, 0,
	/* 118 */	Trap, 0,
	/* 119 */	Trap, 0,
	/* 11a */	Trap, 0,
	/* 11b */	Trap, 0,
	/* 11c */	Trap, 0,
	/* 11d */	Trap, 0,
	/* 11e */	Trap, 0,
	/* 11f */	Trap, 0,
	/* 120 */	Trap, 0,
	/* 121 */	Trap, 0,
	/* 122 */	Trap, 0,
	/* 123 */	Trap, 0,
	/* 124 */	Trap, 0,
	/* 125 */	Trap, 0,
	/* 126 */	Trap, 0,
	/* 127 */	Trap, 0,
	/* 128 */	Trap, 0,
	/* 129 */	Trap, 0,
	/* 12a */	Trap, 0,
	/* 12b */	Trap, 0,
	/* 12c */	Trap, 0,
	/* 12d */	Trap, 0,
	/* 12e */	Trap, 0,
	/* 12f */	Trap, 0,
	/* 130 */	Trap, 0,
	/* 131 */	Trap, 0,
	/* 132 */	Trap, 0,
	/* 133 */	Trap, 0,
	/* 134 */	Trap, 0,
	/* 135 */	Trap, 0,
	/* 136 */	IT_1I, TWIN_LocalHandleDelta,
	/* 137 */	Trap, 0,
	/* 138 */	Trap, 0,
	/* 139 */	Trap, 0,
	/* 13a */	Trap, 0,
	/* 13b */	Trap, 0,
	/* 13c */	Trap, 0,
	/* 13d */	Trap, 0,
	/* 13e */	Trap, 0,
	/* 13f */	Trap, 0,
	/* 140 */	IT_1HTASK, IsTask,
	/* 141 */	Trap, 0,
	/* 142 */	Trap, 0,
	/* 143 */	IT_1H1LP, IsROMModule,
	/* 144 */	Trap, 0,
	/* 145 */	Trap, 0,
	/* 146 */	IT_1H1LP, IsROMFile,
	/* 147 */	Trap, 0,
	/* 148 */	Trap, 0,
	/* 149 */	Trap, 0,
	/* 14a */	Trap, 0,
	/* 14b */	Trap, 0,
	/* 14c */	Trap, 0,
	/* 14d */	Trap, 0,
	/* 14e */	IT_ISBADPTR, IsBadPtr,
	/* 14f */	IT_ISBADPTR, IsBadPtr,
	/* 150 */	IT_ISBADCODEPTR, IsBadPtr,
	/* 151 */	IT_ISBADPTR, IsBadPtr,
	/* 152 */	Trap, 0,
	/* 153 */	Trap, 0,
	/* 154 */	Trap, 0,
	/* 155 */	Trap, 0,
	/* 156 */	Trap, 0,
	/* 157 */	Trap, 0,
	/* 158 */	Trap, 0,
	/* 159 */	Trap, 0,
	/* 15a */	IT_ISBADHUGEPTR, IsBadHugePtr,
	/* 15b */	IT_ISBADHUGEPTR, IsBadHugePtr,
	/* 15c */	IT_2LP1L, hmemcpy,
	/* 15d */	IT_HREADWR, _hread,
	/* 15e */	IT_HREADWR, _hwrite,
	/* 15f */	Trap, 0,
	/* 160 */	Trap, 0,
	/* 161 */	IT_2LP1I, lstrcpyn,
	/* 162 */	_86_GetAppCompatFlags, 0,
	/* 163 */	Trap, 0,
	/* 164 */	Trap, 0,
	0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
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
	/* 014 */	Trap, 0,
	/* 015 */	Trap, 0,
	/* 016 */	Trap, 0,
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
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	Trap, 0,
	/* 025 */	Trap, 0,
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
	/* 066 */	Trap, 0,
	/* 067 */	Trap, 0,
	/* 068 */	Trap, 0,
	/* 069 */	Trap, 0,
	/* 06a */	Trap, 0,
	/* 06b */	Trap, 0,
	/* 06c */	Trap, 0,
	/* 06d */	Trap, 0,
	/* 06e */	Trap, 0,
	/* 06f */	Trap, 0,
	/* 070 */	Trap, 0,
	/* 071 */	Trap, 0,
	/* 072 */	Trap, 0,
	/* 073 */	Trap, 0,
	/* 074 */	Trap, 0,
	/* 075 */	Trap, 0,
	/* 076 */	Trap, 0,
	/* 077 */	Trap, 0,
	/* 078 */	Trap, 0,
	/* 079 */	Trap, 0,
	/* 07a */	Trap, 0,
	/* 07b */	Trap, 0,
	/* 07c */	Trap, 0,
	/* 07d */	Trap, 0,
	/* 07e */	Trap, 0,
	/* 07f */	Trap, 0,
	/* 080 */	Trap, 0,
	/* 081 */	Trap, 0,
	/* 082 */	Trap, 0,
	/* 083 */	Trap, 0,
	/* 084 */	Trap, 0,
	/* 085 */	Trap, 0,
	/* 086 */	Trap, 0,
	/* 087 */	Trap, 0,
	/* 088 */	Trap, 0,
	/* 089 */	Trap, 0,
	/* 08a */	Trap, 0,
	/* 08b */	Trap, 0,
	/* 08c */	Trap, 0,
	/* 08d */	Trap, 0,
	/* 08e */	Trap, 0,
	/* 08f */	Trap, 0,
	/* 090 */	Trap, 0,
	/* 091 */	Trap, 0,
	/* 092 */	Trap, 0,
	/* 093 */	Trap, 0,
	/* 094 */	Trap, 0,
	/* 095 */	Trap, 0,
	/* 096 */	Trap, 0,
	/* 097 */	Trap, 0,
	/* 098 */	Trap, 0,
	/* 099 */	Trap, 0,
	/* 09a */	Trap, 0,
	/* 09b */	Trap, 0,
	/* 09c */	Trap, 0,
	/* 09d */	Trap, 0,
	/* 09e */	Trap, 0,
	/* 09f */	Trap, 0,
	/* 0a0 */	Trap, 0,
	/* 0a1 */	Trap, 0,
	/* 0a2 */	Trap, 0,
	/* 0a3 */	Trap, 0,
	/* 0a4 */	Trap, 0,
	/* 0a5 */	Trap, 0,
	/* 0a6 */	Trap, 0,
	/* 0a7 */	Trap, 0,
	/* 0a8 */	Trap, 0,
	/* 0a9 */	Trap, 0,
	/* 0aa */	Trap, 0,
	/* 0ab */	Trap, 0,
	/* 0ac */	Trap, 0,
	/* 0ad */	Trap, 0,
	/* 0ae */	Trap, 0,
	/* 0af */	Trap, 0,
	/* 0b0 */	Trap, 0,
	/* 0b1 */	Trap, 0,
	/* 0b2 */	Trap, 0,
	/* 0b3 */	Trap, 0,
	/* 0b4 */	Trap, 0,
	/* 0b5 */	Trap, 0,
	/* 0b6 */	Trap, 0,
	/* 0b7 */	Trap, 0,
	/* 0b8 */	Trap, 0,
	/* 0b9 */	Trap, 0,
	/* 0ba */	Trap, 0,
	/* 0bb */	Trap, 0,
	/* 0bc */	Trap, 0,
	/* 0bd */	Trap, 0,
	/* 0be */	Trap, 0,
	/* 0bf */	Trap, 0,
	/* 0c0 */	Trap, 0,
	/* 0c1 */	Trap, 0,
	/* 0c2 */	Trap, 0,
	/* 0c3 */	Trap, 0,
	/* 0c4 */	Trap, 0,
	/* 0c5 */	Trap, 0,
	/* 0c6 */	Trap, 0,
	/* 0c7 */	Trap, 0,
	/* 0c8 */	Trap, 0,
	/* 0c9 */	Trap, 0,
	/* 0ca */	Trap, 0,
	/* 0cb */	Trap, 0,
	/* 0cc */	Trap, 0,
	/* 0cd */	Trap, 0,
	/* 0ce */	Trap, 0,
	/* 0cf */	Trap, 0,
	/* 0d0 */	Trap, 0,
	/* 0d1 */	Trap, 0,
	/* 0d2 */	Trap, 0,
	/* 0d3 */	Trap, 0,
	/* 0d4 */	Trap, 0,
	/* 0d5 */	Trap, 0,
	/* 0d6 */	Trap, 0,
	/* 0d7 */	Trap, 0,
	/* 0d8 */	Trap, 0,
	/* 0d9 */	Trap, 0,
	/* 0da */	Trap, 0,
	/* 0db */	Trap, 0,
	/* 0dc */	Trap, 0,
	/* 0dd */	Trap, 0,
	/* 0de */	Trap, 0,
	/* 0df */	Trap, 0,
	/* 0e0 */	Trap, 0,
	/* 0e1 */	Trap, 0,
	/* 0e2 */	Trap, 0,
	/* 0e3 */	Trap, 0,
	/* 0e4 */	Trap, 0,
	/* 0e5 */	Trap, 0,
	/* 0e6 */	Trap, 0,
	/* 0e7 */	Trap, 0,
	/* 0e8 */	Trap, 0,
	/* 0e9 */	Trap, 0,
	/* 0ea */	Trap, 0,
	/* 0eb */	Trap, 0,
	/* 0ec */	Trap, 0,
	/* 0ed */	Trap, 0,
	/* 0ee */	Trap, 0,
	/* 0ef */	Trap, 0,
	/* 0f0 */	Trap, 0,
	/* 0f1 */	Trap, 0,
	/* 0f2 */	Trap, 0,
	/* 0f3 */	Trap, 0,
	/* 0f4 */	Trap, 0,
	/* 0f5 */	Trap, 0,
	/* 0f6 */	Trap, 0,
	/* 0f7 */	Trap, 0,
	/* 0f8 */	Trap, 0,
	/* 0f9 */	Trap, 0,
	/* 0fa */	Trap, 0,
	/* 0fb */	Trap, 0,
	/* 0fc */	Trap, 0,
	/* 0fd */	Trap, 0,
	/* 0fe */	Trap, 0,
	/* 0ff */	Trap, 0,
	/* 100 */	Trap, 0,
	/* 101 */	Trap, 0,
	/* 102 */	Trap, 0,
	/* 103 */	Trap, 0,
	/* 104 */	Trap, 0,
	/* 105 */	Trap, 0,
	/* 106 */	Trap, 0,
	/* 107 */	Trap, 0,
	/* 108 */	Trap, 0,
	/* 109 */	Trap, 0,
	/* 10a */	Trap, 0,
	/* 10b */	Trap, 0,
	/* 10c */	Trap, 0,
	/* 10d */	Trap, 0,
	/* 10e */	Trap, 0,
	/* 10f */	Trap, 0,
	/* 110 */	Trap, 0,
	/* 111 */	Trap, 0,
	/* 112 */	Trap, 0,
	/* 113 */	Trap, 0,
	/* 114 */	Trap, 0,
	/* 115 */	Trap, 0,
	/* 116 */	Trap, 0,
	/* 117 */	Trap, 0,
	/* 118 */	Trap, 0,
	/* 119 */	Trap, 0,
	/* 11a */	Trap, 0,
	/* 11b */	Trap, 0,
	/* 11c */	Trap, 0,
	/* 11d */	Trap, 0,
	/* 11e */	Trap, 0,
	/* 11f */	Trap, 0,
	/* 120 */	Trap, 0,
	/* 121 */	Trap, 0,
	/* 122 */	Trap, 0,
	/* 123 */	Trap, 0,
	/* 124 */	Trap, 0,
	/* 125 */	Trap, 0,
	/* 126 */	Trap, 0,
	/* 127 */	Trap, 0,
	/* 128 */	Trap, 0,
	/* 129 */	Trap, 0,
	/* 12a */	Trap, 0,
	/* 12b */	Trap, 0,
	/* 12c */	Trap, 0,
	/* 12d */	Trap, 0,
	/* 12e */	Trap, 0,
	/* 12f */	Trap, 0,
	/* 130 */	Trap, 0,
	/* 131 */	Trap, 0,
	/* 132 */	Trap, 0,
	/* 133 */	Trap, 0,
	/* 134 */	Trap, 0,
	/* 135 */	Trap, 0,
	/* 136 */	Trap, 0,
	/* 137 */	Trap, 0,
	/* 138 */	Trap, 0,
	/* 139 */	Trap, 0,
	/* 13a */	Trap, 0,
	/* 13b */	Trap, 0,
	/* 13c */	Trap, 0,
	/* 13d */	Trap, 0,
	/* 13e */	Trap, 0,
	/* 13f */	Trap, 0,
	/* 140 */	Trap, 0,
	/* 141 */	Trap, 0,
	/* 142 */	Trap, 0,
	/* 143 */	Trap, 0,
	/* 144 */	Trap, 0,
	/* 145 */	Trap, 0,
	/* 146 */	Trap, 0,
	/* 147 */	Trap, 0,
	/* 148 */	Trap, 0,
	/* 149 */	Trap, 0,
	/* 14a */	Trap, 0,
	/* 14b */	Trap, 0,
	/* 14c */	Trap, 0,
	/* 14d */	Trap, 0,
	/* 14e */	Trap, 0,
	/* 14f */	Trap, 0,
	/* 150 */	Trap, 0,
	/* 151 */	Trap, 0,
	/* 152 */	Trap, 0,
	/* 153 */	Trap, 0,
	/* 154 */	Trap, 0,
	/* 155 */	Trap, 0,
	/* 156 */	Trap, 0,
	/* 157 */	Trap, 0,
	/* 158 */	Trap, 0,
	/* 159 */	Trap, 0,
	/* 15a */	Trap, 0,
	/* 15b */	Trap, 0,
	/* 15c */	Trap, 0,
	/* 15d */	Trap, 0,
	/* 15e */	Trap, 0,
	/* 15f */	Trap, 0,
	/* 160 */	Trap, 0,
	/* 161 */	Trap, 0,
	/* 162 */	Trap, 0,
	/* 163 */	Trap, 0,
	/* 164 */	Trap, 0,
	0
};
#endif

 /* Segment Table KERNEL: */

static SEGTAB seg_tab_KERNEL[] =
{	{ (char *) seg_image_KERNEL_0, 2848, TRANSFER_CALLBACK, 2848, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

 /* Module Descriptor for KERNEL: */

static MODULEDSCR hsmt_mod_dscr_KERNEL =
{	"KERNEL",
	entry_tab_KERNEL,
	seg_tab_KERNEL,
	0
};


extern long int SubtractRect();

extern long int SetBkColor();
extern long int SetBkMode();
extern long int SetMapMode();
extern long int SetROP2();
extern long int SetPolyFillMode();
extern long int SetStretchBltMode();
extern long int SetTextCharacterExtra();
extern long int SetTextColor();
extern long int SetTextJustification();
extern long int SetWindowOrg();
extern long int SetWindowExt();
extern long int SetViewportOrg();
extern long int SetViewportExt();
extern long int OffsetWindowOrg();
extern long int ScaleWindowExt();
extern long int OffsetViewportOrg();
extern long int ScaleViewportExt();
extern long int LineTo();
extern long int MoveTo();
extern long int ExcludeClipRect();
extern long int IntersectClipRect();
extern long int Arc();
extern long int Ellipse();
extern long int FloodFill();
extern long int Pie();
extern long int Rectangle();
extern long int RoundRect();
extern long int PatBlt();
extern long int SaveDC();
extern long int SetPixel();
extern long int TextOut();
extern long int BitBlt();
extern long int StretchBlt();
extern long int Polygon();
extern long int Polyline();
extern long int Escape();
extern long int RestoreDC();
extern long int FillRgn();
extern long int FrameRgn();
extern long int InvertRgn();
extern long int PaintRgn();
extern long int SelectClipRgn();
extern long int SelectObject();
extern long int CombineRgn();
extern long int CreateBitmap();
extern long int CreateBitmapIndirect();
extern long int CreateBrushIndirect();
extern long int CreateCompatibleBitmap();
extern long int CreateCompatibleDC();
extern long int CreateDC();
extern long int CreateEllipticRgn();
extern long int CreateEllipticRgnIndirect();
extern long int CreateFont();
extern long int CreateFontIndirect();
extern long int CreateHatchBrush();
extern long int CreatePatternBrush();
extern long int CreatePen();
extern long int CreatePenIndirect();
extern long int CreatePolygonRgn();
extern long int CreatePolyPolygonRgn();
extern long int CreateRectRgn();
extern long int CreateRectRgnIndirect();
extern long int CreateSolidBrush();
extern long int DPtoLP();
extern long int DeleteDC();
extern long int DeleteObject();
extern long int EnumFonts();
extern long int EnumObjects();
extern long int EqualRgn();
extern long int GetBitmapBits();
extern long int GetBkColor();
extern long int GetBkMode();
extern long int GetClipBox();
extern long int GetCurrentPosition();
extern long int GetDCOrg();
extern long int GetDeviceCaps();
extern long int GetMapMode();
extern long int GetObject();
extern long int GetPixel();
extern long int GetPolyFillMode();
extern long int GetROP2();
extern long int GetRelAbs();
extern long int GetStockObject();
extern long int GetStretchBltMode();
extern long int GetTextCharacterExtra();
extern long int GetTextColor();
extern long int GetTextExtent();
extern long int GetTextFace();
extern long int GetTextMetrics();
extern long int GetViewportExt();
extern long int GetViewportOrg();
extern long int GetWindowExt();
extern long int GetWindowOrg();
extern long int LPtoDP();
extern long int LineDDA();
extern long int OffsetRgn();
extern long int PtVisible();
extern long int RectVisible();
extern long int SetBitmapBits();
extern long int AddFontResource();
extern long int PlayMetaFile();
extern long int EnumMetaFile();
extern long int PlayMetaFileRecord();
extern long int GetMetaFile();
extern long int CreateMetaFile();
extern long int CopyMetaFile();
extern long int CloseMetaFile();
extern long int DeleteMetaFile();
extern long int SetMetaFileBits();
extern long int SetMetaFileBitsBetter();
extern long int IsValidMetaFile();
extern long int MulDiv();
extern long int GetRgnBox();
extern long int RemoveFontResource();
extern long int SetBrushOrg();
extern long int GetBrushOrg();
extern long int UnrealizeObject();
extern long int CreateIC();
extern long int GetNearestColor();
extern long int GetNearestPaletteIndex();
extern long int CreateDiscardableBitmap();
extern long int PtInRegion();
extern long int GetBitmapDimension();
extern long int SetBitmapDimension();
extern long int SetRectRgn();
extern long int RectInRegion();
extern long int EnumFontFamilies();
extern long int GetRasterizerCaps();
extern long int GetTextAlign();
extern long int SetTextAlign();
extern long int Chord();
extern long int SetMapperFlags();
extern long int GetCharWidth();
extern long int GetCharABCWidths();
extern long int GetKerningPairs();
extern long int ExtTextOut();
extern long int GetAspectRatioFilter();
extern long int CreatePalette();
extern long int ExtFloodFill();
extern long int ResetDC();
extern long int CreateDIBitmap();
extern long int CreateRoundRectRgn();
extern long int CreateDIBPatternBrush();
extern long int PolyPolygon();
extern long int GetBitmapDimensionEx();
extern long int GetBrushOrgEx();
extern long int GetCurrentPositionEx();
extern long int GetTextExtentPoint();
extern long int GetViewportExtEx();
extern long int GetViewportOrgEx();
extern long int GetWindowExtEx();
extern long int GetWindowOrgEx();
extern long int OffsetViewportOrgEx();
extern long int OffsetWindowOrgEx();
extern long int SetBitmapDimensionEx();
extern long int SetViewportExtEx();
extern long int SetViewportOrgEx();
extern long int SetWindowExtEx();
extern long int SetWindowOrgEx();
extern long int MoveToEx();
extern long int ScaleViewportExtEx();
extern long int ScaleWindowExtEx();
extern long int GetAspectRatioFilterEx();
extern long int ResizePalette();
extern long int SetPaletteEntries();
extern long int OpenJob();
extern long int WriteSpool();
extern long int CloseJob();
extern long int DeleteJob();
extern long int StartSpoolPage();
extern long int EndSpoolPage();
extern long int dmBitBlt();
extern long int dmColorInfo();
extern long int dmOutput();
extern long int dmPixel();
extern long int dmRealizeObject();
extern long int dmStrBlt();
extern long int dmScanLR();
extern long int EngineRealizeFont();
extern long int EngineEnumerateFont();
extern long int EngineDeleteFont();
extern long int EngineGetCharWidth();
extern long int EngineGetGlyphBmp();
extern long int EngineSetFontContext();
extern long int GetFontData();
extern long int SetAbortProc();
extern long int StartDoc();
extern long int EndDoc();
extern long int StartPage();
extern long int EndPage();
extern long int AbortDoc();

extern long int SetSystemPaletteUse();
extern long int GetSystemPaletteUse();
extern long int SetPaletteEntries();
extern long int GetPaletteEntries();
extern long int GetSystemPaletteEntries();
extern long int AnimatePalette();
extern long int UpdateColors();

extern long int FastWindowFrame();

extern long int SetBoundsRect();
extern long int GetBoundsRect();

 /* Entry Table GDI: */

static ENTRYTAB entry_tab_GDI[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "GDI", 0, 0, 0 },
	/* 001 */	{ "SETBKCOLOR", 0x0008, 0x0008, SetBkColor },
	/* 002 */	{ "SETBKMODE", 0x0008, 0x0010, SetBkMode },
	/* 003 */	{ "SETMAPMODE", 0x0008, 0x0018, SetMapMode },
	/* 004 */	{ "SETROP2", 0x0008, 0x0020, SetROP2 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "SETPOLYFILLMODE", 0x0008, 0x0030, SetPolyFillMode },
	/* 007 */	{ "SETSTRETCHBLTMODE", 0x0008, 0x0038, SetStretchBltMode },
	/* 008 */	{ "SETTEXTCHARACTEREXTRA", 0x0008, 0x0040, SetTextCharacterExtra },
	/* 009 */	{ "SETTEXTCOLOR", 0x0008, 0x0048, SetTextColor },
	/* 00a */	{ "SETTEXTJUSTIFICATION", 0x0008, 0x0050, SetTextJustification },
	/* 00b */	{ "SETWINDOWORG", 0x0008, 0x0058, SetWindowOrg },
	/* 00c */	{ "SETWINDOWEXT", 0x0008, 0x0060, SetWindowExt },
	/* 00d */	{ "SETVIEWPORTORG", 0x0008, 0x0068, SetViewportOrg },
	/* 00e */	{ "SETVIEWPORTEXT", 0x0008, 0x0070, SetViewportExt },
	/* 00f */	{ "OFFSETWINDOWORG", 0x0008, 0x0078, OffsetWindowOrg },
	/* 010 */	{ "SCALEWINDOWEXT", 0x0008, 0x0080, ScaleWindowExt },
	/* 011 */	{ "OFFSETVIEWPORTORG", 0x0008, 0x0088, OffsetViewportOrg },
	/* 012 */	{ "SCALEVIEWPORTEXT", 0x0008, 0x0090, ScaleViewportExt },
	/* 013 */	{ "LINETO", 0x0008, 0x0098, LineTo },
	/* 014 */	{ "MOVETO", 0x0008, 0x00a0, MoveTo },
	/* 015 */	{ "EXCLUDECLIPRECT", 0x0008, 0x00a8, ExcludeClipRect },
	/* 016 */	{ "INTERSECTCLIPRECT", 0x0008, 0x00b0, IntersectClipRect },
	/* 017 */	{ "ARC", 0x0008, 0x00b8, Arc },
	/* 018 */	{ "ELLIPSE", 0x0008, 0x00c0, Ellipse },
	/* 019 */	{ "FLOODFILL", 0x0008, 0x00c8, FloodFill },
	/* 01a */	{ "PIE", 0x0008, 0x00d0, Pie },
	/* 01b */	{ "RECTANGLE", 0x0008, 0x00d8, Rectangle },
	/* 01c */	{ "ROUNDRECT", 0x0008, 0x00e0, RoundRect },
	/* 01d */	{ "PATBLT", 0x0008, 0x00e8, PatBlt },
	/* 01e */	{ "SAVEDC", 0x0008, 0x00f0, SaveDC },
	/* 01f */	{ "SETPIXEL", 0x0008, 0x00f8, SetPixel },
	/* 020 */	{ "OFFSETCLIPRGN", 0x0008, 0x0100, OffsetClipRgn },
	/* 021 */	{ "TEXTOUT", 0x0008, 0x0108, TextOut },
	/* 022 */	{ "BITBLT", 0x0008, 0x0110, BitBlt },
	/* 023 */	{ "STRETCHBLT", 0x0008, 0x0118, StretchBlt },
	/* 024 */	{ "POLYGON", 0x0008, 0x0120, Polygon },
	/* 025 */	{ "POLYLINE", 0x0008, 0x0128, Polyline },
	/* 026 */	{ "ESCAPE", 0x0008, 0x0130, Escape },
	/* 027 */	{ "RESTOREDC", 0x0008, 0x0138, RestoreDC },
	/* 028 */	{ "FILLRGN", 0x0008, 0x0140, FillRgn },
	/* 029 */	{ "FRAMERGN", 0x0008, 0x0148, FrameRgn },
	/* 02a */	{ "INVERTRGN", 0x0008, 0x0150, InvertRgn },
	/* 02b */	{ "PAINTRGN", 0x0008, 0x0158, PaintRgn },
	/* 02c */	{ "SELECTCLIPRGN", 0x0008, 0x0160, SelectClipRgn },
	/* 02d */	{ "SELECTOBJECT", 0x0008, 0x0168, SelectObject },
	/* 02e */	{ "", 0, 0, 0 },
	/* 02f */	{ "COMBINERGN", 0x0008, 0x0178, CombineRgn },
	/* 030 */	{ "CREATEBITMAP", 0x0008, 0x0180, CreateBitmap },
	/* 031 */	{ "CREATEBITMAPINDIRECT", 0x0008, 0x0188, CreateBitmapIndirect },
	/* 032 */	{ "CREATEBRUSHINDIRECT", 0x0008, 0x0190, CreateBrushIndirect },
	/* 033 */	{ "CREATECOMPATIBLEBITMAP", 0x0008, 0x0198, CreateCompatibleBitmap },
	/* 034 */	{ "CREATECOMPATIBLEDC", 0x0008, 0x01a0, CreateCompatibleDC },
	/* 035 */	{ "CREATEDC", 0x0008, 0x01a8, CreateDC },
	/* 036 */	{ "CREATEELLIPTICRGN", 0x0008, 0x01b0, CreateEllipticRgn },
	/* 037 */	{ "CREATEELLIPTICRGNINDIRECT", 0x0008, 0x01b8, CreateEllipticRgnIndirect },
	/* 038 */	{ "CREATEFONT", 0x0008, 0x01c0, CreateFont },
	/* 039 */	{ "CREATEFONTINDIRECT", 0x0008, 0x01c8, CreateFontIndirect },
	/* 03a */	{ "CREATEHATCHBRUSH", 0x0008, 0x01d0, CreateHatchBrush },
	/* 03b */	{ "", 0, 0, 0 },
	/* 03c */	{ "CREATEPATTERNBRUSH", 0x0008, 0x01e0, CreatePatternBrush },
	/* 03d */	{ "CREATEPEN", 0x0008, 0x01e8, CreatePen },
	/* 03e */	{ "CREATEPENINDIRECT", 0x0008, 0x01f0, CreatePenIndirect },
	/* 03f */	{ "CREATEPOLYGONRGN", 0x0008, 0x01f8, CreatePolygonRgn },
	/* 040 */	{ "CREATERECTRGN", 0x0008, 0x0200, CreateRectRgn },
	/* 041 */	{ "CREATERECTRGNINDIRECT", 0x0008, 0x0208, CreateRectRgnIndirect },
	/* 042 */	{ "CREATESOLIDBRUSH", 0x0008, 0x0210, CreateSolidBrush },
	/* 043 */	{ "DPTOLP", 0x0008, 0x0218, DPtoLP },
	/* 044 */	{ "DELETEDC", 0x0008, 0x0220, DeleteDC },
	/* 045 */	{ "DELETEOBJECT", 0x0008, 0x0228, DeleteObject },
	/* 046 */	{ "ENUMFONTS", 0x0008, 0x0230, EnumFonts },
	/* 047 */	{ "ENUMOBJECTS", 0x0008, 0x0238, EnumObjects },
	/* 048 */	{ "EQUALRGN", 0x0008, 0x0240, EqualRgn },
	/* 049 */	{ "", 0, 0, 0 },
	/* 04a */	{ "GETBITMAPBITS", 0x0008, 0x0250, GetBitmapBits },
	/* 04b */	{ "GETBKCOLOR", 0x0008, 0x0258, GetBkColor },
	/* 04c */	{ "GETBKMODE", 0x0008, 0x0260, GetBkMode },
	/* 04d */	{ "GETCLIPBOX", 0x0008, 0x0268, GetClipBox },
	/* 04e */	{ "GETCURRENTPOSITION", 0x0008, 0x0270, GetCurrentPosition },
	/* 04f */	{ "GETDCORG", 0x0008, 0x0278, GetDCOrg },
	/* 050 */	{ "GETDEVICECAPS", 0x0008, 0x0280, GetDeviceCaps },
	/* 051 */	{ "GETMAPMODE", 0x0008, 0x0288, GetMapMode },
	/* 052 */	{ "GETOBJECT", 0x0008, 0x0290, GetObject },
	/* 053 */	{ "GETPIXEL", 0x0008, 0x0298, GetPixel },
	/* 054 */	{ "GETPOLYFILLMODE", 0x0008, 0x02a0, GetPolyFillMode },
	/* 055 */	{ "GETROP2", 0x0008, 0x02a8, GetROP2 },
	/* 056 */	{ "GETRELABS", 0x0008, 0x02b0, GetRelAbs },
	/* 057 */	{ "GETSTOCKOBJECT", 0x0008, 0x02b8, GetStockObject },
	/* 058 */	{ "GETSTRETCHBLTMODE", 0x0008, 0x02c0, GetStretchBltMode },
	/* 059 */	{ "GETTEXTCHARACTEREXTRA", 0x0008, 0x02c8, GetTextCharacterExtra },
	/* 05a */	{ "GETTEXTCOLOR", 0x0008, 0x02d0, GetTextColor },
	/* 05b */	{ "GETTEXTEXTENT", 0x0008, 0x02d8, GetTextExtent },
	/* 05c */	{ "GETTEXTFACE", 0x0008, 0x02e0, GetTextFace },
	/* 05d */	{ "GETTEXTMETRICS", 0x0008, 0x02e8, GetTextMetrics },
	/* 05e */	{ "GETVIEWPORTEXT", 0x0008, 0x02f0, GetViewportExt },
	/* 05f */	{ "GETVIEWPORTORG", 0x0008, 0x02f8, GetViewportOrg },
	/* 060 */	{ "GETWINDOWEXT", 0x0008, 0x0300, GetWindowExt },
	/* 061 */	{ "GETWINDOWORG", 0x0008, 0x0308, GetWindowOrg },
	/* 062 */	{ "", 0, 0, 0 },
	/* 063 */	{ "LPTODP", 0x0008, 0x0318, LPtoDP },
	/* 064 */	{ "LINEDDA", 0x0008, 0x0320, LineDDA },
	/* 065 */	{ "OFFSETRGN", 0x0008, 0x0328, OffsetRgn },
	/* 066 */	{ "", 0, 0, 0 },
	/* 067 */	{ "PTVISIBLE", 0x0008, 0x0338, PtVisible },
	/* 068 */	{ "RECTVISIBLE", 0x0008, 0x0340, RectVisible },
	/* 069 */	{ "", 0, 0, 0 },
	/* 06a */	{ "SETBITMAPBITS", 0x0008, 0x0350, SetBitmapBits },
	/* 06b */	{ "", 0, 0, 0 },
	/* 06c */	{ "", 0, 0, 0 },
	/* 06d */	{ "", 0, 0, 0 },
	/* 06e */	{ "", 0, 0, 0 },
	/* 06f */	{ "", 0, 0, 0 },
	/* 070 */	{ "", 0, 0, 0 },
	/* 071 */	{ "", 0, 0, 0 },
	/* 072 */	{ "", 0, 0, 0 },
	/* 073 */	{ "", 0, 0, 0 },
	/* 074 */	{ "", 0, 0, 0 },
	/* 075 */	{ "", 0, 0, 0 },
	/* 076 */	{ "", 0, 0, 0 },
	/* 077 */	{ "ADDFONTRESOURCE", 0x0008, 0x03b8, AddFontResource },
	/* 078 */	{ "", 0, 0, 0 },
	/* 079 */	{ "", 0, 0, 0 },
	/* 07a */	{ "", 0, 0, 0 },
	/* 07b */	{ "PLAYMETAFILE", 0x0008, 0x03d8, PlayMetaFile },
	/* 07c */	{ "GETMETAFILE", 0x0008, 0x03e0, GetMetaFile },
	/* 07d */	{ "CREATEMETAFILE", 0x0008, 0x03e8, CreateMetaFile },
	/* 07e */	{ "CLOSEMETAFILE", 0x0008, 0x03f0, CloseMetaFile },
	/* 07f */	{ "DELETEMETAFILE", 0x0008, 0x03f8, DeleteMetaFile },
	/* 080 */	{ "MULDIV", 0x0008, 0x0400, MulDiv },
	/* 081 */	{ "", 0, 0, 0 },
	/* 082 */	{ "", 0, 0, 0 },
	/* 083 */	{ "", 0, 0, 0 },
	/* 084 */	{ "SETENVIRONMENT", 0x0008, 0x0420, SetEnvironment },
	/* 085 */	{ "GETENVIRONMENT", 0x0008, 0x0428, GetEnvironment },
	/* 086 */	{ "GETRGNBOX", 0x0008, 0x0430, GetRgnBox },
	/* 087 */	{ "", 0, 0, 0 },
	/* 088 */	{ "REMOVEFONTRESOURCE", 0x0008, 0x0440, RemoveFontResource },
	/* 089 */	{ "", 0, 0, 0 },
	/* 08a */	{ "", 0, 0, 0 },
	/* 08b */	{ "", 0, 0, 0 },
	/* 08c */	{ "", 0, 0, 0 },
	/* 08d */	{ "", 0, 0, 0 },
	/* 08e */	{ "", 0, 0, 0 },
	/* 08f */	{ "", 0, 0, 0 },
	/* 090 */	{ "", 0, 0, 0 },
	/* 091 */	{ "", 0, 0, 0 },
	/* 092 */	{ "", 0, 0, 0 },
	/* 093 */	{ "", 0, 0, 0 },
	/* 094 */	{ "SETBRUSHORG", 0x0008, 0x04a0, SetBrushOrg },
	/* 095 */	{ "GETBRUSHORG", 0x0008, 0x04a8, GetBrushOrg },
	/* 096 */	{ "UNREALIZEOBJECT", 0x0008, 0x04b0, UnrealizeObject },
	/* 097 */	{ "COPYMETAFILE", 0x0008, 0x04b8, CopyMetaFile },
	/* 098 */	{ "", 0, 0, 0 },
	/* 099 */	{ "CREATEIC", 0x0008, 0x04c8, CreateIC },
	/* 09a */	{ "GETNEARESTCOLOR", 0x0008, 0x04d0, GetNearestColor },
	/* 09b */	{ "QUERYABORT", 0x0008, 0x04d8, 0 },
	/* 09c */	{ "CREATEDISCARDABLEBITMAP", 0x0008, 0x04e0, CreateDiscardableBitmap },
	/* 09d */	{ "", 0, 0, 0 },
	/* 09e */	{ "", 0, 0, 0 },
	/* 09f */	{ "GETMETAFILEBITS", 0x0008, 0x04f8, 0 },
	/* 0a0 */	{ "SETMETAFILEBITS", 0x0008, 0x0500, SetMetaFileBits },
	/* 0a1 */	{ "PTINREGION", 0x0008, 0x0508, PtInRegion },
	/* 0a2 */	{ "GETBITMAPDIMENSION", 0x0008, 0x0510, GetBitmapDimension },
	/* 0a3 */	{ "SETBITMAPDIMENSION", 0x0008, 0x0518, SetBitmapDimension },
	/* 0a4 */	{ "", 0, 0, 0 },
	/* 0a5 */	{ "", 0, 0, 0 },
	/* 0a6 */	{ "", 0, 0, 0 },
	/* 0a7 */	{ "", 0, 0, 0 },
	/* 0a8 */	{ "", 0, 0, 0 },
	/* 0a9 */	{ "", 0, 0, 0 },
	/* 0aa */	{ "", 0, 0, 0 },
	/* 0ab */	{ "", 0, 0, 0 },
	/* 0ac */	{ "SETRECTRGN", 0x0008, 0x0560, SetRectRgn },
	/* 0ad */	{ "", 0, 0, 0 },
	/* 0ae */	{ "", 0, 0, 0 },
	/* 0af */	{ "ENUMMETAFILE", 0x0008, 0x0578, EnumMetaFile },
	/* 0b0 */	{ "PLAYMETAFILERECORD", 0x0008, 0x0580, PlayMetaFileRecord },
	/* 0b1 */	{ "", 0, 0, 0 },
	/* 0b2 */	{ "", 0, 0, 0 },
	/* 0b3 */	{ "", 0, 0, 0 },
	/* 0b4 */	{ "", 0, 0, 0 },
	/* 0b5 */	{ "RECTINREGION", 0x0008, 0x05a8, RectInRegion },
	/* 0b6 */	{ "", 0, 0, 0 },
	/* 0b7 */	{ "", 0, 0, 0 },
	/* 0b8 */	{ "", 0, 0, 0 },
	/* 0b9 */	{ "", 0, 0, 0 },
	/* 0ba */	{ "", 0, 0, 0 },
	/* 0bb */	{ "", 0, 0, 0 },
	/* 0bc */	{ "", 0, 0, 0 },
	/* 0bd */	{ "", 0, 0, 0 },
	/* 0be */	{ "", 0, 0, 0 },
	/* 0bf */	{ "", 0, 0, 0 },
	/* 0c0 */	{ "", 0, 0, 0 },
	/* 0c1 */	{ "SETBOUNDSRECT", 0x0008, 0x0608, SetBoundsRect },
	/* 0c2 */	{ "GETBOUNDSRECT", 0x0008, 0x0610, GetBoundsRect },
	/* 0c3 */	{ "", 0, 0, 0 },
	/* 0c4 */	{ "SETMETAFILEBITSBETTER", 0x0008, 0x0620, SetMetaFileBitsBetter },
	/* 0c5 */	{ "", 0, 0, 0 },
	/* 0c6 */	{ "", 0, 0, 0 },
	/* 0c7 */	{ "", 0, 0, 0 },
	/* 0c8 */	{ "", 0, 0, 0 },
	/* 0c9 */	{ "DMBITBLT", 0x0008, 0x0648, dmBitBlt },
	/* 0ca */	{ "DMCOLORINFO", 0x0008, 0x0650, dmColorInfo },
	/* 0cb */	{ "", 0, 0, 0 },
	/* 0cc */	{ "", 0, 0, 0 },
	/* 0cd */	{ "", 0, 0, 0 },
	/* 0ce */	{ "", 0, 0, 0 },
	/* 0cf */	{ "", 0, 0, 0 },
	/* 0d0 */	{ "DMOUTPUT", 0x0008, 0x0680, dmOutput },
	/* 0d1 */	{ "DMPIXEL", 0x0008, 0x0688, dmPixel },
	/* 0d2 */	{ "DMREALIZEOBJECT", 0x0008, 0x0690, dmRealizeObject },
	/* 0d3 */	{ "DMSTRBLT", 0x0008, 0x0698, dmStrBlt },
	/* 0d4 */	{ "DMSCANLR", 0x0008, 0x06a0, dmScanLR },
	/* 0d5 */	{ "", 0, 0, 0 },
	/* 0d6 */	{ "", 0, 0, 0 },
	/* 0d7 */	{ "", 0, 0, 0 },
	/* 0d8 */	{ "", 0, 0, 0 },
	/* 0d9 */	{ "", 0, 0, 0 },
	/* 0da */	{ "", 0, 0, 0 },
	/* 0db */	{ "", 0, 0, 0 },
	/* 0dc */	{ "", 0, 0, 0 },
	/* 0dd */	{ "", 0, 0, 0 },
	/* 0de */	{ "", 0, 0, 0 },
	/* 0df */	{ "", 0, 0, 0 },
	/* 0e0 */	{ "", 0, 0, 0 },
	/* 0e1 */	{ "", 0, 0, 0 },
	/* 0e2 */	{ "", 0, 0, 0 },
	/* 0e3 */	{ "", 0, 0, 0 },
	/* 0e4 */	{ "", 0, 0, 0 },
	/* 0e5 */	{ "", 0, 0, 0 },
	/* 0e6 */	{ "", 0, 0, 0 },
	/* 0e7 */	{ "", 0, 0, 0 },
	/* 0e8 */	{ "", 0, 0, 0 },
	/* 0e9 */	{ "", 0, 0, 0 },
	/* 0ea */	{ "", 0, 0, 0 },
	/* 0eb */	{ "", 0, 0, 0 },
	/* 0ec */	{ "", 0, 0, 0 },
	/* 0ed */	{ "", 0, 0, 0 },
	/* 0ee */	{ "", 0, 0, 0 },
	/* 0ef */	{ "", 0, 0, 0 },
	/* 0f0 */	{ "OPENJOB", 0x0008, 0x0780, OpenJob },
	/* 0f1 */	{ "WRITESPOOL", 0x0008, 0x0788, WriteSpool },
	/* 0f2 */	{ "", 0, 0, 0 },
	/* 0f3 */	{ "CLOSEJOB", 0x0008, 0x0798, CloseJob },
	/* 0f4 */	{ "DELETEJOB", 0x0008, 0x07a0, DeleteJob },
	/* 0f5 */	{ "", 0, 0, 0 },
	/* 0f6 */	{ "STARTSPOOLPAGE", 0x0008, 0x07b0, StartSpoolPage },
	/* 0f7 */	{ "ENDSPOOLPAGE", 0x0008, 0x07b8, EndSpoolPage },
	/* 0f8 */	{ "", 0, 0, 0 },
	/* 0f9 */	{ "", 0, 0, 0 },
	/* 0fa */	{ "", 0, 0, 0 },
	/* 0fb */	{ "", 0, 0, 0 },
	/* 0fc */	{ "", 0, 0, 0 },
	/* 0fd */	{ "", 0, 0, 0 },
	/* 0fe */	{ "SPOOLFILE", 0x0008, 0x07f0, 0 },
	/* 0ff */	{ "", 0, 0, 0 },
	/* 100 */	{ "", 0, 0, 0 },
	/* 101 */	{ "", 0, 0, 0 },
	/* 102 */	{ "", 0, 0, 0 },
	/* 103 */	{ "", 0, 0, 0 },
	/* 104 */	{ "", 0, 0, 0 },
	/* 105 */	{ "", 0, 0, 0 },
	/* 106 */	{ "", 0, 0, 0 },
	/* 107 */	{ "", 0, 0, 0 },
	/* 108 */	{ "", 0, 0, 0 },
	/* 109 */	{ "", 0, 0, 0 },
	/* 10a */	{ "", 0, 0, 0 },
	/* 10b */	{ "", 0, 0, 0 },
	/* 10c */	{ "", 0, 0, 0 },
	/* 10d */	{ "", 0, 0, 0 },
	/* 10e */	{ "", 0, 0, 0 },
	/* 10f */	{ "", 0, 0, 0 },
	/* 110 */	{ "", 0, 0, 0 },
	/* 111 */	{ "", 0, 0, 0 },
	/* 112 */	{ "", 0, 0, 0 },
	/* 113 */	{ "", 0, 0, 0 },
	/* 114 */	{ "", 0, 0, 0 },
	/* 115 */	{ "", 0, 0, 0 },
	/* 116 */	{ "", 0, 0, 0 },
	/* 117 */	{ "", 0, 0, 0 },
	/* 118 */	{ "", 0, 0, 0 },
	/* 119 */	{ "", 0, 0, 0 },
	/* 11a */	{ "", 0, 0, 0 },
	/* 11b */	{ "", 0, 0, 0 },
	/* 11c */	{ "", 0, 0, 0 },
	/* 11d */	{ "", 0, 0, 0 },
	/* 11e */	{ "", 0, 0, 0 },
	/* 11f */	{ "", 0, 0, 0 },
	/* 120 */	{ "", 0, 0, 0 },
	/* 121 */	{ "", 0, 0, 0 },
	/* 122 */	{ "", 0, 0, 0 },
	/* 123 */	{ "", 0, 0, 0 },
	/* 124 */	{ "", 0, 0, 0 },
	/* 125 */	{ "", 0, 0, 0 },
	/* 126 */	{ "", 0, 0, 0 },
	/* 127 */	{ "", 0, 0, 0 },
	/* 128 */	{ "", 0, 0, 0 },
	/* 129 */	{ "", 0, 0, 0 },
	/* 12a */	{ "", 0, 0, 0 },
	/* 12b */	{ "", 0, 0, 0 },
	/* 12c */	{ "ENGINEENUMERATEFONT", 0x0008, 0x09680, EngineEnumerateFont },
	/* 12d */	{ "ENGINEDELETEFONT", 0x0008, 0x0970, EngineDeleteFont },
	/* 12e */	{ "ENGINEREALIZEFONT", 0x0008, 0x0978, EngineRealizeFont },
	/* 12f */	{ "ENGINEGETCHARWIDTH", 0x0008, 0x0980, EngineGetCharWidth },
	/* 130 */	{ "ENGINESETFONTCONTEXT", 0x0008, 0x0988, EngineSetFontContext },
	/* 131 */	{ "ENGINEGETGLYPHBMP", 0x0008, 0x0990, EngineGetGlyphBmp },
	/* 132 */	{ "", 0, 0, 0 },
	/* 133 */	{ "GETCHARABCWIDTHS", 0x0008, 0x0998, GetCharABCWidths },
	/* 134 */	{ "GETOUTLINETEXTMETRICS", 0x0008, 0x09a0, 0 },
	/* 135 */	{ "GETGLYPHOUTLINE", 0x0008, 0x09a8, 0 },
	/* 136 */	{ "CREATESCALABLEFONTRESOURCE", 0x0008, 0x09b0, CreateScalableFontResource },
	/* 137 */	{ "GETFONTDATA", 0x0008, 0x09b8, GetFontData },
	/* 138 */	{ "", 0, 0, 0 },
	/* 139 */	{ "GETRASTERIZERCAPS", 0x0008, 0x09c8, GetRasterizerCaps },
	/* 13a */	{ "", 0, 0, 0 },
	/* 13b */	{ "", 0, 0, 0 },
	/* 13c */	{ "", 0, 0, 0 },
	/* 13d */	{ "", 0, 0, 0 },
	/* 13e */	{ "", 0, 0, 0 },
	/* 13f */	{ "", 0, 0, 0 },
	/* 140 */	{ "", 0, 0, 0 },
	/* 141 */	{ "", 0, 0, 0 },
	/* 142 */	{ "", 0, 0, 0 },
	/* 143 */	{ "", 0, 0, 0 },
	/* 144 */	{ "", 0, 0, 0 },
	/* 145 */	{ "", 0, 0, 0 },
	/* 146 */	{ "", 0, 0, 0 },
	/* 147 */	{ "", 0, 0, 0 },
	/* 148 */	{ "", 0, 0, 0 },
	/* 149 */	{ "", 0, 0, 0 },
	/* 14a */	{ "ENUMFONTFAMILIES", 0x0008, 0x0a50, EnumFontFamilies },
	/* 14b */	{ "", 0, 0, 0 },
	/* 14c */	{ "GETKERNINGPAIRS", 0x0008, 0x0a60, GetKerningPairs },
	/* 14d */	{ "", 0, 0, 0 },
	/* 14e */	{ "", 0, 0, 0 },
	/* 14f */	{ "", 0, 0, 0 },
	/* 150 */	{ "", 0, 0, 0 },
	/* 151 */	{ "", 0, 0, 0 },
	/* 152 */	{ "", 0, 0, 0 },
	/* 153 */	{ "", 0, 0, 0 },
	/* 154 */	{ "", 0, 0, 0 },
	/* 155 */	{ "", 0, 0, 0 },
	/* 156 */	{ "", 0, 0, 0 },
	/* 157 */	{ "", 0, 0, 0 },
	/* 158 */	{ "", 0, 0, 0 },
	/* 159 */	{ "GETTEXTALIGN", 0x0008, 0x0ac8, GetTextAlign },
	/* 15a */	{ "SETTEXTALIGN", 0x0008, 0x0ad0, SetTextAlign },
	/* 15b */	{ "", 0, 0, 0 },
	/* 15c */	{ "CHORD", 0x0008, 0x0ae0, Chord },
	/* 15d */	{ "SETMAPPERFLAGS", 0x0008, 0x0ae8, SetMapperFlags },
	/* 15e */	{ "GETCHARWIDTH", 0x0008, 0x0af0, GetCharWidth },
	/* 15f */	{ "EXTTEXTOUT", 0x0008, 0x0af8, ExtTextOut },
	/* 160 */	{ "", 0, 0, 0 },
	/* 161 */	{ "GETASPECTRATIOFILTER", 0x0008, 0x0b08, GetAspectRatioFilter },
	/* 162 */	{ "", 0, 0, 0 },
	/* 163 */	{ "", 0, 0, 0 },
	/* 164 */	{ "", 0, 0, 0 },
	/* 165 */	{ "", 0, 0, 0 },
	/* 166 */	{ "", 0, 0, 0 },
	/* 167 */	{ "", 0, 0, 0 },
	/* 168 */	{ "CREATEPALETTE", 0x0008, 0x0b40, CreatePalette },
	/* 169 */	{ "", 0, 0, 0 },
	/* 16a */	{ "", 0, 0, 0 },
	/* 16b */	{ "GETPALETTEENTRIES", 0x0008, 0x0b58, GetPaletteEntries },
	/* 16c */	{ "SETPALETTEENTRIES", 0x0008, 0x0b60, SetPaletteEntries },
	/* 16d */	{ "", 0, 0, 0 },
	/* 16e */	{ "UPDATECOLORS", 0x0008, 0x0b70, UpdateColors },
	/* 16f */	{ "ANIMATEPALETTE", 0x0008, 0x0b78, AnimatePalette },
	/* 170 */	{ "RESIZEPALETTE", 0x0008, 0x0b80, ResizePalette },
	/* 171 */	{ "", 0, 0, 0 },
	/* 172 */	{ "GETNEARESTPALETTEINDEX", 0x0008, 0x0b90, GetNearestPaletteIndex },
	/* 173 */	{ "", 0, 0, 0 },
	/* 174 */	{ "EXTFLOODFILL", 0x0008, 0x0ba0, ExtFloodFill },
	/* 175 */	{ "SETSYSTEMPALETTEUSE", 0x0008, 0x0ba8, SetSystemPaletteUse },
	/* 176 */	{ "GETSYSTEMPALETTEUSE", 0x0008, 0x0bb0, GetSystemPaletteUse },
	/* 177 */	{ "GETSYSTEMPALETTEENTRIES", 0x0008, 0x0bb8, GetSystemPaletteEntries},
	/* 178 */	{ "RESETDC", 0x0008, 0x0bc0, ResetDC },
	/* 179 */	{ "STARTDOC", 0x0008, 0x0bc8, StartDoc },
	/* 17a */	{ "ENDDOC", 0x0008, 0x0bd0, EndDoc },
	/* 17b */	{ "STARTPAGE", 0x0008, 0x0bd8, StartPage },
	/* 17c */	{ "ENDPAGE", 0x0008, 0x0be0, EndPage },
	/* 17d */	{ "SETABORTPROC", 0x0008, 0x0be8, SetAbortProc },
	/* 17e */	{ "ABORTDOC", 0x0008, 0x0bf0, AbortDoc },
	/* 17f */	{ "", 0, 0, 0 },
	/* 180 */	{ "", 0, 0, 0 },
	/* 181 */	{ "", 0, 0, 0 },
	/* 182 */	{ "", 0, 0, 0 },
	/* 183 */	{ "", 0, 0, 0 },
	/* 184 */	{ "", 0, 0, 0 },
	/* 185 */	{ "", 0, 0, 0 },
	/* 186 */	{ "", 0, 0, 0 },
	/* 187 */	{ "", 0, 0, 0 },
	/* 188 */	{ "", 0, 0, 0 },
	/* 189 */	{ "", 0, 0, 0 },
	/* 18a */	{ "", 0, 0, 0 },
	/* 18b */	{ "", 0, 0, 0 },
	/* 18c */	{ "", 0, 0, 0 },
	/* 18d */	{ "", 0, 0, 0 },
	/* 18e */	{ "", 0, 0, 0 },
	/* 18f */	{ "", 0, 0, 0 },
	/* 190 */	{ "FASTWINDOWFRAME", 0x0008, 0x0c80, FastWindowFrame },
	/* 191 */	{ "", 0, 0, 0 },
	/* 192 */	{ "", 0, 0, 0 },
	/* 193 */	{ "", 0, 0, 0 },
	/* 194 */	{ "", 0, 0, 0 },
	/* 195 */	{ "", 0, 0, 0 },
	/* 196 */	{ "", 0, 0, 0 },
	/* 197 */	{ "", 0, 0, 0 },
	/* 198 */	{ "", 0, 0, 0 },
	/* 199 */	{ "", 0, 0, 0 },
        /* 19a */       { "ISVALIDMETAFILE", 0x0008, 0x0cd0, IsValidMetaFile },
	/* 19b */	{ "", 0, 0, 0 },
	/* 19c */	{ "", 0, 0, 0 },
	/* 19d */	{ "", 0, 0, 0 },
	/* 19e */	{ "", 0, 0, 0 },
	/* 19f */	{ "", 0, 0, 0 },
	/* 1a0 */	{ "", 0, 0, 0 },
	/* 1a1 */	{ "", 0, 0, 0 },
	/* 1a2 */	{ "", 0, 0, 0 },
	/* 1a3 */	{ "", 0, 0, 0 },
	/* 1a4 */	{ "", 0, 0, 0 },
	/* 1a5 */	{ "", 0, 0, 0 },
	/* 1a6 */	{ "", 0, 0, 0 },
	/* 1a7 */	{ "", 0, 0, 0 },
	/* 1a8 */	{ "", 0, 0, 0 },
	/* 1a9 */	{ "", 0, 0, 0 },
	/* 1aa */	{ "", 0, 0, 0 },
	/* 1ab */	{ "", 0, 0, 0 },
	/* 1ac */	{ "", 0, 0, 0 },
	/* 1ad */	{ "", 0, 0, 0 },
	/* 1ae */	{ "", 0, 0, 0 },
	/* 1af */	{ "", 0, 0, 0 },
	/* 1b0 */	{ "", 0, 0, 0 },
	/* 1b1 */	{ "", 0, 0, 0 },
	/* 1b2 */	{ "", 0, 0, 0 },
	/* 1b3 */	{ "", 0, 0, 0 },
	/* 1b4 */	{ "", 0, 0, 0 },
	/* 1b5 */	{ "", 0, 0, 0 },
	/* 1b6 */	{ "", 0, 0, 0 },
	/* 1b7 */	{ "STRETCHDIBITS", 0x0008, 0x0db8, StretchDIBits },
	/* 1b8 */	{ "SETDIBITS", 0x0008, 0x0dc0, SetDIBits },
	/* 1b9 */	{ "GETDIBITS", 0x0008, 0x0dc8, GetDIBits },
	/* 1ba */	{ "CREATEDIBITMAP", 0x0008, 0x0dd0, CreateDIBitmap },
	/* 1bb */	{ "SETDIBITSTODEVICE", 0x0008, 0x0dd8, SetDIBitsToDevice },
	/* 1bc */	{ "CREATEROUNDRECTRGN", 0x0008, 0x0de0, CreateRoundRectRgn },
	/* 1bd */	{ "CREATEDIBPATTERNBRUSH", 0x0008, 0x0de8, CreateDIBPatternBrush },
	/* 1be */	{ "", 0, 0, 0 },
	/* 1bf */	{ "", 0, 0, 0 },
	/* 1c0 */	{ "", 0, 0, 0 },
	/* 1c1 */	{ "", 0, 0, 0 },
	/* 1c2 */	{ "POLYPOLYGON", 0x0008, 0x0e10, PolyPolygon },
	/* 1c3 */	{ "CREATEPOLYPOLYGONRGN", 0x0008, 0x0e18, CreatePolyPolygonRgn },
	/* 1c4 */	{ "", 0, 0, 0 },
	/* 1c5 */	{ "", 0, 0, 0 },
	/* 1c6 */	{ "", 0, 0, 0 },
	/* 1c7 */	{ "", 0, 0, 0 },
	/* 1c8 */	{ "", 0, 0, 0 },
	/* 1c9 */	{ "", 0, 0, 0 },
	/* 1ca */	{ "", 0, 0, 0 },
	/* 1cb */	{ "", 0, 0, 0 },
	/* 1cc */	{ "", 0, 0, 0 },
	/* 1cd */	{ "", 0, 0, 0 },
	/* 1ce */	{ "ISGDIOBJECT", 0x0008, 0x0e70, IsGDIObject },
	/* 1cf */	{ "", 0, 0, 0 },
	/* 1d0 */	{ "", 0, 0, 0 },
	/* 1d1 */	{ "RECTVISIBLE_EHH", 0x0008, 0x0e88, RectVisible },
	/* 1d2 */	{ "", 0, 0, 0 },
	/* 1d3 */	{ "", 0, 0, 0 },
	/* 1d4 */	{ "GETBITMAPDIMENSIONEX", 0x0008, 0x0ea0, GetBitmapDimensionEx },
	/* 1d5 */	{ "GETBRUSHORGEX", 0x0008, 0x0ea8, GetBrushOrgEx },
	/* 1d6 */	{ "GETCURRENTPOSITIONEX", 0x0008, 0x0eb0, GetCurrentPositionEx },
	/* 1d7 */	{ "GETTEXTEXTENTPOINT", 0x0008, 0x0eb8, GetTextExtentPoint },
	/* 1d8 */	{ "GETVIEWPORTEXTEX", 0x0008, 0x0ec0, GetViewportExtEx },
	/* 1d9 */	{ "GETVIEWPORTORGEX", 0x0008, 0x0ec8, GetViewportOrgEx },
	/* 1da */	{ "GETWINDOWEXTEX", 0x0008, 0x0ed0, GetWindowExtEx },
	/* 1db */	{ "GETWINDOWORGEX", 0x0008, 0x0ed8, GetWindowOrgEx },
	/* 1dc */	{ "OFFSETVIEWPORTORGEX", 0x0008, 0x0ee0, OffsetViewportOrgEx },
	/* 1dd */	{ "OFFSETWINDOWORGEX", 0x0008, 0x0ee8, OffsetWindowOrgEx },
	/* 1de */	{ "SETBITMAPDIMENSIONEX", 0x0008, 0x0ef0, SetBitmapDimensionEx },
	/* 1df */	{ "SETVIEWPORTEXTEX", 0x0008, 0x0ef8, SetViewportExtEx },
	/* 1e0 */	{ "SETVIEWPORTORGEX", 0x0008, 0x0f00, SetViewportOrgEx },
	/* 1e1 */	{ "SETWINDOWEXTEX", 0x0008, 0x0f08, SetWindowExtEx },
	/* 1e2 */	{ "SETWINDOWORGEX", 0x0008, 0x0f10, SetWindowOrgEx },
	/* 1e3 */	{ "MOVETOEX", 0x0008, 0x0f18, MoveToEx },
	/* 1e4 */	{ "SCALEVIEWPORTEXTEX", 0x0008, 0x0f20, ScaleViewportExtEx },
	/* 1e5 */	{ "SCALEWINDOWEXTEX", 0x0008, 0x0f28, ScaleWindowExtEx },
	/* 1e6 */	{ "GETASPECTRATIOFILTEREX", 0x0008, 0x0f30, GetAspectRatioFilterEx },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "", 0, 0, 0 },
	/* 00b */	{ "", 0, 0, 0 },
	/* 00c */	{ "", 0, 0, 0 },
	/* 00d */	{ "", 0, 0, 0 },
	/* 00e */	{ "", 0, 0, 0 },
	/* 00f */	{ "", 0, 0, 0 },
	/* 010 */	{ "", 0, 0, 0 },
	/* 011 */	{ "", 0, 0, 0 },
	/* 012 */	{ "", 0, 0, 0 },
	/* 013 */	{ "", 0, 0, 0 },
	/* 014 */	{ "", 0, 0, 0 },
	/* 015 */	{ "", 0, 0, 0 },
	/* 016 */	{ "", 0, 0, 0 },
	/* 017 */	{ "", 0, 0, 0 },
	/* 018 */	{ "", 0, 0, 0 },
	/* 019 */	{ "", 0, 0, 0 },
	/* 01a */	{ "", 0, 0, 0 },
	/* 01b */	{ "", 0, 0, 0 },
	/* 01c */	{ "", 0, 0, 0 },
	/* 01d */	{ "", 0, 0, 0 },
	/* 01e */	{ "", 0, 0, 0 },
	/* 01f */	{ "", 0, 0, 0 },
	/* 020 */	{ "", 0, 0, 0 },
	/* 021 */	{ "", 0, 0, 0 },
	/* 022 */	{ "", 0, 0, 0 },
	/* 023 */	{ "", 0, 0, 0 },
	/* 024 */	{ "", 0, 0, 0 },
	/* 025 */	{ "", 0, 0, 0 },
	/* 026 */	{ "", 0, 0, 0 },
	/* 027 */	{ "", 0, 0, 0 },
	/* 028 */	{ "", 0, 0, 0 },
	/* 029 */	{ "", 0, 0, 0 },
	/* 02a */	{ "", 0, 0, 0 },
	/* 02b */	{ "", 0, 0, 0 },
	/* 02c */	{ "", 0, 0, 0 },
	/* 02d */	{ "", 0, 0, 0 },
	/* 02e */	{ "", 0, 0, 0 },
	/* 02f */	{ "", 0, 0, 0 },
	/* 030 */	{ "", 0, 0, 0 },
	/* 031 */	{ "", 0, 0, 0 },
	/* 032 */	{ "", 0, 0, 0 },
	/* 033 */	{ "", 0, 0, 0 },
	/* 034 */	{ "", 0, 0, 0 },
	/* 035 */	{ "", 0, 0, 0 },
	/* 036 */	{ "", 0, 0, 0 },
	/* 037 */	{ "", 0, 0, 0 },
	/* 038 */	{ "", 0, 0, 0 },
	/* 039 */	{ "", 0, 0, 0 },
	/* 03a */	{ "", 0, 0, 0 },
	/* 03b */	{ "", 0, 0, 0 },
	/* 03c */	{ "", 0, 0, 0 },
	/* 03d */	{ "", 0, 0, 0 },
	/* 03e */	{ "", 0, 0, 0 },
	/* 03f */	{ "", 0, 0, 0 },
	/* 040 */	{ "", 0, 0, 0 },
	/* 041 */	{ "", 0, 0, 0 },
	/* 042 */	{ "", 0, 0, 0 },
	/* 043 */	{ "", 0, 0, 0 },
	/* 044 */	{ "", 0, 0, 0 },
	/* 045 */	{ "", 0, 0, 0 },
	/* 046 */	{ "", 0, 0, 0 },
	/* 047 */	{ "", 0, 0, 0 },
	/* 048 */	{ "", 0, 0, 0 },
	/* 049 */	{ "", 0, 0, 0 },
	/* 04a */	{ "", 0, 0, 0 },
	/* 04b */	{ "", 0, 0, 0 },
	/* 04c */	{ "", 0, 0, 0 },
	/* 04d */	{ "", 0, 0, 0 },
	/* 04e */	{ "", 0, 0, 0 },
	/* 04f */	{ "", 0, 0, 0 },
	/* 050 */	{ "", 0, 0, 0 },
	/* 051 */	{ "", 0, 0, 0 },
	/* 052 */	{ "", 0, 0, 0 },
	/* 053 */	{ "", 0, 0, 0 },
	/* 054 */	{ "", 0, 0, 0 },
	/* 055 */	{ "", 0, 0, 0 },
	/* 056 */	{ "", 0, 0, 0 },
	/* 057 */	{ "", 0, 0, 0 },
	/* 058 */	{ "", 0, 0, 0 },
	/* 059 */	{ "", 0, 0, 0 },
	/* 05a */	{ "", 0, 0, 0 },
	/* 05b */	{ "", 0, 0, 0 },
	/* 05c */	{ "", 0, 0, 0 },
	/* 05d */	{ "", 0, 0, 0 },
	/* 05e */	{ "", 0, 0, 0 },
	/* 05f */	{ "", 0, 0, 0 },
	/* 060 */	{ "", 0, 0, 0 },
	/* 061 */	{ "", 0, 0, 0 },
	/* 062 */	{ "", 0, 0, 0 },
	/* 063 */	{ "", 0, 0, 0 },
	/* 064 */	{ "", 0, 0, 0 },
	/* 065 */	{ "", 0, 0, 0 },
	/* 066 */	{ "", 0, 0, 0 },
	/* 067 */	{ "", 0, 0, 0 },
	/* 068 */	{ "", 0, 0, 0 },
	/* 069 */	{ "", 0, 0, 0 },
	/* 06a */	{ "", 0, 0, 0 },
	/* 06b */	{ "", 0, 0, 0 },
	/* 06c */	{ "", 0, 0, 0 },
	/* 06d */	{ "", 0, 0, 0 },
	/* 06e */	{ "", 0, 0, 0 },
	/* 06f */	{ "", 0, 0, 0 },
	/* 070 */	{ "", 0, 0, 0 },
	/* 071 */	{ "", 0, 0, 0 },
	/* 072 */	{ "", 0, 0, 0 },
	/* 073 */	{ "", 0, 0, 0 },
	/* 074 */	{ "", 0, 0, 0 },
	/* 075 */	{ "", 0, 0, 0 },
	/* 076 */	{ "", 0, 0, 0 },
	/* 077 */	{ "", 0, 0, 0 },
	/* 078 */	{ "", 0, 0, 0 },
	/* 079 */	{ "", 0, 0, 0 },
	/* 07a */	{ "", 0, 0, 0 },
	/* 07b */	{ "", 0, 0, 0 },
	/* 07c */	{ "", 0, 0, 0 },
	/* 07d */	{ "", 0, 0, 0 },
	/* 07e */	{ "", 0, 0, 0 },
	/* 07f */	{ "", 0, 0, 0 },
	/* 080 */	{ "", 0, 0, 0 },
	/* 081 */	{ "", 0, 0, 0 },
	/* 082 */	{ "", 0, 0, 0 },
	/* 083 */	{ "", 0, 0, 0 },
	/* 084 */	{ "", 0, 0, 0 },
	/* 085 */	{ "", 0, 0, 0 },
	/* 086 */	{ "", 0, 0, 0 },
	/* 087 */	{ "", 0, 0, 0 },
	/* 088 */	{ "", 0, 0, 0 },
	/* 089 */	{ "", 0, 0, 0 },
	/* 08a */	{ "", 0, 0, 0 },
	/* 08b */	{ "", 0, 0, 0 },
	/* 08c */	{ "", 0, 0, 0 },
	/* 08d */	{ "", 0, 0, 0 },
	/* 08e */	{ "", 0, 0, 0 },
	/* 08f */	{ "", 0, 0, 0 },
	/* 090 */	{ "", 0, 0, 0 },
	/* 091 */	{ "", 0, 0, 0 },
	/* 092 */	{ "", 0, 0, 0 },
	/* 093 */	{ "", 0, 0, 0 },
	/* 094 */	{ "", 0, 0, 0 },
	/* 095 */	{ "", 0, 0, 0 },
	/* 096 */	{ "", 0, 0, 0 },
	/* 097 */	{ "", 0, 0, 0 },
	/* 098 */	{ "", 0, 0, 0 },
	/* 099 */	{ "", 0, 0, 0 },
	/* 09a */	{ "", 0, 0, 0 },
	/* 09b */	{ "", 0, 0, 0 },
	/* 09c */	{ "", 0, 0, 0 },
	/* 09d */	{ "", 0, 0, 0 },
	/* 09e */	{ "", 0, 0, 0 },
	/* 09f */	{ "", 0, 0, 0 },
	/* 0a0 */	{ "", 0, 0, 0 },
	/* 0a1 */	{ "", 0, 0, 0 },
	/* 0a2 */	{ "", 0, 0, 0 },
	/* 0a3 */	{ "", 0, 0, 0 },
	/* 0a4 */	{ "", 0, 0, 0 },
	/* 0a5 */	{ "", 0, 0, 0 },
	/* 0a6 */	{ "", 0, 0, 0 },
	/* 0a7 */	{ "", 0, 0, 0 },
	/* 0a8 */	{ "", 0, 0, 0 },
	/* 0a9 */	{ "", 0, 0, 0 },
	/* 0aa */	{ "", 0, 0, 0 },
	/* 0ab */	{ "", 0, 0, 0 },
	/* 0ac */	{ "", 0, 0, 0 },
	/* 0ad */	{ "", 0, 0, 0 },
	/* 0ae */	{ "", 0, 0, 0 },
	/* 0af */	{ "", 0, 0, 0 },
	/* 0b0 */	{ "", 0, 0, 0 },
	/* 0b1 */	{ "", 0, 0, 0 },
	/* 0b2 */	{ "", 0, 0, 0 },
	/* 0b3 */	{ "", 0, 0, 0 },
	/* 0b4 */	{ "", 0, 0, 0 },
	/* 0b5 */	{ "", 0, 0, 0 },
	/* 0b6 */	{ "", 0, 0, 0 },
	/* 0b7 */	{ "", 0, 0, 0 },
	/* 0b8 */	{ "", 0, 0, 0 },
	/* 0b9 */	{ "", 0, 0, 0 },
	/* 0ba */	{ "", 0, 0, 0 },
	/* 0bb */	{ "", 0, 0, 0 },
	/* 0bc */	{ "", 0, 0, 0 },
	/* 0bd */	{ "", 0, 0, 0 },
	/* 0be */	{ "", 0, 0, 0 },
	/* 0bf */	{ "", 0, 0, 0 },
	/* 0c0 */	{ "", 0, 0, 0 },
	/* 0c1 */	{ "", 0, 0, 0 },
	/* 0c2 */	{ "", 0, 0, 0 },
	/* 0c3 */	{ "", 0, 0, 0 },
	/* 0c4 */	{ "", 0, 0, 0 },
	/* 0c5 */	{ "", 0, 0, 0 },
	/* 0c6 */	{ "", 0, 0, 0 },
	/* 0c7 */	{ "", 0, 0, 0 },
	/* 0c8 */	{ "", 0, 0, 0 },
	/* 0c9 */	{ "", 0, 0, 0 },
	/* 0ca */	{ "", 0, 0, 0 },
	/* 0cb */	{ "", 0, 0, 0 },
	/* 0cc */	{ "", 0, 0, 0 },
	/* 0cd */	{ "", 0, 0, 0 },
	/* 0ce */	{ "", 0, 0, 0 },
	/* 0cf */	{ "", 0, 0, 0 },
	/* 0d0 */	{ "", 0, 0, 0 },
	/* 0d1 */	{ "", 0, 0, 0 },
	/* 0d2 */	{ "", 0, 0, 0 },
	/* 0d3 */	{ "", 0, 0, 0 },
	/* 0d4 */	{ "", 0, 0, 0 },
	/* 0d5 */	{ "", 0, 0, 0 },
	/* 0d6 */	{ "", 0, 0, 0 },
	/* 0d7 */	{ "", 0, 0, 0 },
	/* 0d8 */	{ "", 0, 0, 0 },
	/* 0d9 */	{ "", 0, 0, 0 },
	/* 0da */	{ "", 0, 0, 0 },
	/* 0db */	{ "", 0, 0, 0 },
	/* 0dc */	{ "", 0, 0, 0 },
	/* 0dd */	{ "", 0, 0, 0 },
	/* 0de */	{ "", 0, 0, 0 },
	/* 0df */	{ "", 0, 0, 0 },
	/* 0e0 */	{ "", 0, 0, 0 },
	/* 0e1 */	{ "", 0, 0, 0 },
	/* 0e2 */	{ "", 0, 0, 0 },
	/* 0e3 */	{ "", 0, 0, 0 },
	/* 0e4 */	{ "", 0, 0, 0 },
	/* 0e5 */	{ "", 0, 0, 0 },
	/* 0e6 */	{ "", 0, 0, 0 },
	/* 0e7 */	{ "", 0, 0, 0 },
	/* 0e8 */	{ "", 0, 0, 0 },
	/* 0e9 */	{ "", 0, 0, 0 },
	/* 0ea */	{ "", 0, 0, 0 },
	/* 0eb */	{ "", 0, 0, 0 },
	/* 0ec */	{ "", 0, 0, 0 },
	/* 0ed */	{ "", 0, 0, 0 },
	/* 0ee */	{ "", 0, 0, 0 },
	/* 0ef */	{ "", 0, 0, 0 },
	/* 0f0 */	{ "", 0, 0, 0 },
	/* 0f1 */	{ "", 0, 0, 0 },
	/* 0f2 */	{ "", 0, 0, 0 },
	/* 0f3 */	{ "", 0, 0, 0 },
	/* 0f4 */	{ "", 0, 0, 0 },
	/* 0f5 */	{ "", 0, 0, 0 },
	/* 0f6 */	{ "", 0, 0, 0 },
	/* 0f7 */	{ "", 0, 0, 0 },
	/* 0f8 */	{ "", 0, 0, 0 },
	/* 0f9 */	{ "", 0, 0, 0 },
	/* 0fa */	{ "", 0, 0, 0 },
	/* 0fb */	{ "", 0, 0, 0 },
	/* 0fc */	{ "", 0, 0, 0 },
	/* 0fd */	{ "", 0, 0, 0 },
	/* 0fe */	{ "", 0, 0, 0 },
	/* 0ff */	{ "", 0, 0, 0 },
	/* 100 */	{ "", 0, 0, 0 },
	/* 101 */	{ "", 0, 0, 0 },
	/* 102 */	{ "", 0, 0, 0 },
	/* 103 */	{ "", 0, 0, 0 },
	/* 104 */	{ "", 0, 0, 0 },
	/* 105 */	{ "", 0, 0, 0 },
	/* 106 */	{ "", 0, 0, 0 },
	/* 107 */	{ "", 0, 0, 0 },
	/* 108 */	{ "", 0, 0, 0 },
	/* 109 */	{ "", 0, 0, 0 },
	/* 10a */	{ "", 0, 0, 0 },
	/* 10b */	{ "", 0, 0, 0 },
	/* 10c */	{ "", 0, 0, 0 },
	/* 10d */	{ "", 0, 0, 0 },
	/* 10e */	{ "", 0, 0, 0 },
	/* 10f */	{ "", 0, 0, 0 },
	/* 110 */	{ "", 0, 0, 0 },
	/* 111 */	{ "", 0, 0, 0 },
	/* 112 */	{ "", 0, 0, 0 },
	/* 113 */	{ "", 0, 0, 0 },
	/* 114 */	{ "", 0, 0, 0 },
	/* 115 */	{ "", 0, 0, 0 },
	/* 116 */	{ "", 0, 0, 0 },
	/* 117 */	{ "", 0, 0, 0 },
	/* 118 */	{ "", 0, 0, 0 },
	/* 119 */	{ "", 0, 0, 0 },
	/* 11a */	{ "", 0, 0, 0 },
	/* 11b */	{ "", 0, 0, 0 },
	/* 11c */	{ "", 0, 0, 0 },
	/* 11d */	{ "", 0, 0, 0 },
	/* 11e */	{ "", 0, 0, 0 },
	/* 11f */	{ "", 0, 0, 0 },
	/* 120 */	{ "", 0, 0, 0 },
	/* 121 */	{ "", 0, 0, 0 },
	/* 122 */	{ "", 0, 0, 0 },
	/* 123 */	{ "", 0, 0, 0 },
	/* 124 */	{ "", 0, 0, 0 },
	/* 125 */	{ "", 0, 0, 0 },
	/* 126 */	{ "", 0, 0, 0 },
	/* 127 */	{ "", 0, 0, 0 },
	/* 128 */	{ "", 0, 0, 0 },
	/* 129 */	{ "", 0, 0, 0 },
	/* 12a */	{ "", 0, 0, 0 },
	/* 12b */	{ "", 0, 0, 0 },
	/* 12c */	{ "", 0, 0, 0 },
	/* 12d */	{ "", 0, 0, 0 },
	/* 12e */	{ "", 0, 0, 0 },
	/* 12f */	{ "", 0, 0, 0 },
	/* 130 */	{ "", 0, 0, 0 },
	/* 131 */	{ "", 0, 0, 0 },
	/* 132 */	{ "", 0, 0, 0 },
	/* 133 */	{ "", 0, 0, 0 },
	/* 134 */	{ "", 0, 0, 0 },
	/* 135 */	{ "", 0, 0, 0 },
	/* 136 */	{ "", 0, 0, 0 },
	/* 137 */	{ "", 0, 0, 0 },
	/* 138 */	{ "", 0, 0, 0 },
	/* 139 */	{ "", 0, 0, 0 },
	/* 13a */	{ "", 0, 0, 0 },
	/* 13b */	{ "", 0, 0, 0 },
	/* 13c */	{ "", 0, 0, 0 },
	/* 13d */	{ "", 0, 0, 0 },
	/* 13e */	{ "", 0, 0, 0 },
	/* 13f */	{ "", 0, 0, 0 },
	/* 140 */	{ "", 0, 0, 0 },
	/* 141 */	{ "", 0, 0, 0 },
	/* 142 */	{ "", 0, 0, 0 },
	/* 143 */	{ "", 0, 0, 0 },
	/* 144 */	{ "", 0, 0, 0 },
	/* 145 */	{ "", 0, 0, 0 },
	/* 146 */	{ "", 0, 0, 0 },
	/* 147 */	{ "", 0, 0, 0 },
	/* 148 */	{ "", 0, 0, 0 },
	/* 149 */	{ "", 0, 0, 0 },
	/* 14a */	{ "", 0, 0, 0 },
	/* 14b */	{ "", 0, 0, 0 },
	/* 14c */	{ "", 0, 0, 0 },
	/* 14d */	{ "", 0, 0, 0 },
	/* 14e */	{ "", 0, 0, 0 },
	/* 14f */	{ "", 0, 0, 0 },
	/* 150 */	{ "", 0, 0, 0 },
	/* 151 */	{ "", 0, 0, 0 },
	/* 152 */	{ "", 0, 0, 0 },
	/* 153 */	{ "", 0, 0, 0 },
	/* 154 */	{ "", 0, 0, 0 },
	/* 155 */	{ "", 0, 0, 0 },
	/* 156 */	{ "", 0, 0, 0 },
	/* 157 */	{ "", 0, 0, 0 },
	/* 158 */	{ "", 0, 0, 0 },
	/* 159 */	{ "", 0, 0, 0 },
	/* 15a */	{ "", 0, 0, 0 },
	/* 15b */	{ "", 0, 0, 0 },
	/* 15c */	{ "", 0, 0, 0 },
	/* 15d */	{ "", 0, 0, 0 },
	/* 15e */	{ "", 0, 0, 0 },
	/* 15f */	{ "", 0, 0, 0 },
	/* 160 */	{ "", 0, 0, 0 },
	/* 161 */	{ "", 0, 0, 0 },
	/* 162 */	{ "", 0, 0, 0 },
	/* 163 */	{ "", 0, 0, 0 },
	/* 164 */	{ "", 0, 0, 0 },
	/* 165 */	{ "", 0, 0, 0 },
	/* 166 */	{ "", 0, 0, 0 },
	/* 167 */	{ "", 0, 0, 0 },
	/* 168 */	{ "", 0, 0, 0 },
	/* 169 */	{ "", 0, 0, 0 },
	/* 16a */	{ "", 0, 0, 0 },
	/* 16b */	{ "", 0, 0, 0 },
	/* 16c */	{ "", 0, 0, 0 },
	/* 16d */	{ "", 0, 0, 0 },
	/* 16e */	{ "", 0, 0, 0 },
	/* 16f */	{ "", 0, 0, 0 },
	/* 170 */	{ "", 0, 0, 0 },
	/* 171 */	{ "", 0, 0, 0 },
	/* 172 */	{ "", 0, 0, 0 },
	/* 173 */	{ "", 0, 0, 0 },
	/* 174 */	{ "", 0, 0, 0 },
	/* 175 */	{ "", 0, 0, 0 },
	/* 176 */	{ "", 0, 0, 0 },
	/* 177 */	{ "", 0, 0, 0 },
	/* 178 */	{ "", 0, 0, 0 },
	/* 179 */	{ "", 0, 0, 0 },
	/* 17a */	{ "", 0, 0, 0 },
	/* 17b */	{ "", 0, 0, 0 },
	/* 17c */	{ "", 0, 0, 0 },
	/* 17d */	{ "", 0, 0, 0 },
	/* 17e */	{ "", 0, 0, 0 },
	/* 17f */	{ "", 0, 0, 0 },
	/* 180 */	{ "", 0, 0, 0 },
	/* 181 */	{ "", 0, 0, 0 },
	/* 182 */	{ "", 0, 0, 0 },
	/* 183 */	{ "", 0, 0, 0 },
	/* 184 */	{ "", 0, 0, 0 },
	/* 185 */	{ "", 0, 0, 0 },
	/* 186 */	{ "", 0, 0, 0 },
	/* 187 */	{ "", 0, 0, 0 },
	/* 188 */	{ "", 0, 0, 0 },
	/* 189 */	{ "", 0, 0, 0 },
	/* 18a */	{ "", 0, 0, 0 },
	/* 18b */	{ "", 0, 0, 0 },
	/* 18c */	{ "", 0, 0, 0 },
	/* 18d */	{ "", 0, 0, 0 },
	/* 18e */	{ "", 0, 0, 0 },
	/* 18f */	{ "", 0, 0, 0 },
	/* 190 */	{ "", 0, 0, 0 },
	/* 191 */	{ "", 0, 0, 0 },
	/* 192 */	{ "", 0, 0, 0 },
	/* 193 */	{ "", 0, 0, 0 },
	/* 194 */	{ "", 0, 0, 0 },
	/* 195 */	{ "", 0, 0, 0 },
	/* 196 */	{ "", 0, 0, 0 },
	/* 197 */	{ "", 0, 0, 0 },
	/* 198 */	{ "", 0, 0, 0 },
	/* 199 */	{ "", 0, 0, 0 },
	/* 19a */	{ "", 0, 0, 0 },
	/* 19b */	{ "", 0, 0, 0 },
	/* 19c */	{ "", 0, 0, 0 },
	/* 19d */	{ "", 0, 0, 0 },
	/* 19e */	{ "", 0, 0, 0 },
	/* 19f */	{ "", 0, 0, 0 },
	/* 1a0 */	{ "", 0, 0, 0 },
	/* 1a1 */	{ "", 0, 0, 0 },
	/* 1a2 */	{ "", 0, 0, 0 },
	/* 1a3 */	{ "", 0, 0, 0 },
	/* 1a4 */	{ "", 0, 0, 0 },
	/* 1a5 */	{ "", 0, 0, 0 },
	/* 1a6 */	{ "", 0, 0, 0 },
	/* 1a7 */	{ "", 0, 0, 0 },
	/* 1a8 */	{ "", 0, 0, 0 },
	/* 1a9 */	{ "", 0, 0, 0 },
	/* 1aa */	{ "", 0, 0, 0 },
	/* 1ab */	{ "", 0, 0, 0 },
	/* 1ac */	{ "", 0, 0, 0 },
	/* 1ad */	{ "", 0, 0, 0 },
	/* 1ae */	{ "", 0, 0, 0 },
	/* 1af */	{ "", 0, 0, 0 },
	/* 1b0 */	{ "", 0, 0, 0 },
	/* 1b1 */	{ "", 0, 0, 0 },
	/* 1b2 */	{ "", 0, 0, 0 },
	/* 1b3 */	{ "", 0, 0, 0 },
	/* 1b4 */	{ "", 0, 0, 0 },
	/* 1b5 */	{ "", 0, 0, 0 },
	/* 1b6 */	{ "", 0, 0, 0 },
	/* 1b7 */	{ "", 0, 0, 0 },
	/* 1b8 */	{ "", 0, 0, 0 },
	/* 1b9 */	{ "", 0, 0, 0 },
	/* 1ba */	{ "", 0, 0, 0 },
	/* 1bb */	{ "", 0, 0, 0 },
	/* 1bc */	{ "", 0, 0, 0 },
	/* 1bd */	{ "", 0, 0, 0 },
	/* 1be */	{ "", 0, 0, 0 },
	/* 1bf */	{ "", 0, 0, 0 },
	/* 1c0 */	{ "", 0, 0, 0 },
	/* 1c1 */	{ "", 0, 0, 0 },
	/* 1c2 */	{ "", 0, 0, 0 },
	/* 1c3 */	{ "", 0, 0, 0 },
	/* 1c4 */	{ "", 0, 0, 0 },
	/* 1c5 */	{ "", 0, 0, 0 },
	/* 1c6 */	{ "", 0, 0, 0 },
	/* 1c7 */	{ "", 0, 0, 0 },
	/* 1c8 */	{ "", 0, 0, 0 },
	/* 1c9 */	{ "", 0, 0, 0 },
	/* 1ca */	{ "", 0, 0, 0 },
	/* 1cb */	{ "", 0, 0, 0 },
	/* 1cc */	{ "", 0, 0, 0 },
	/* 1cd */	{ "", 0, 0, 0 },
	/* 1ce */	{ "", 0, 0, 0 },
	/* 1cf */	{ "", 0, 0, 0 },
	/* 1d0 */	{ "", 0, 0, 0 },
	/* 1d1 */	{ "", 0, 0, 0 },
	/* 1d2 */	{ "", 0, 0, 0 },
	/* 1d3 */	{ "", 0, 0, 0 },
	/* 1d4 */	{ "", 0, 0, 0 },
	/* 1d5 */	{ "", 0, 0, 0 },
	/* 1d6 */	{ "", 0, 0, 0 },
	/* 1d7 */	{ "", 0, 0, 0 },
	/* 1d8 */	{ "", 0, 0, 0 },
	/* 1d9 */	{ "", 0, 0, 0 },
	/* 1da */	{ "", 0, 0, 0 },
	/* 1db */	{ "", 0, 0, 0 },
	/* 1dc */	{ "", 0, 0, 0 },
	/* 1dd */	{ "", 0, 0, 0 },
	/* 1de */	{ "", 0, 0, 0 },
	/* 1df */	{ "", 0, 0, 0 },
	/* 1e0 */	{ "", 0, 0, 0 },
	/* 1e1 */	{ "", 0, 0, 0 },
	/* 1e2 */	{ "", 0, 0, 0 },
	/* 1e3 */	{ "", 0, 0, 0 },
	/* 1e4 */	{ "", 0, 0, 0 },
	/* 1e5 */	{ "", 0, 0, 0 },
	/* 1e6 */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif

extern long int IT_1H1D();
extern long int IT_1H1I();
extern long int IT_1H2I();
extern long int IT_1H4I();
extern long int IT_1H2I();
extern long int IT_1H4I();
extern long int IT_1H2I();
extern long int IT_1H2I();
extern long int IT_1H4I();
extern long int IT_1H4I();
extern long int IT_1H8I();
extern long int IT_1H4I();
extern long int IT_1H2I1D();
extern long int IT_1H8I();
extern long int IT_1H4I();
extern long int IT_1H6I();
extern long int IT_1H4I1D();
extern long int IT_1H();
extern long int IT_1H2I1D();
extern long int IT_TEXTOUT();
extern long int IT_BITBLT();
extern long int IT_STRETCHBLT();
extern long int IT_POLY();
extern long int IT_ESCAPE();
extern long int IT_1H1I();
extern long int IT_3H();
extern long int IT_3H2I();
extern long int IT_2H();
extern long int IT_2H();
extern long int IT_2H();
extern long int IT_2H();
extern long int IT_3H1I();
extern long int IT_2I2UI1LPB();
extern long int IT_1LPBMc();
extern long int IT_CRBRUSHIND();
extern long int IT_1H2I();
extern long int IT_1H();
extern long int IT_CREATEDC();
extern long int IT_4I();
extern long int IT_1LPRc();
extern long int IT_CREATEFONT();
extern long int IT_1LPLFc();
extern long int IT_1I1D();
extern long int IT_1H();
extern long int IT_2I1D();
extern long int IT_1LPLPc();
extern long int IT_CREATEPOLYRGN();
extern long int IT_CREATEPOLYPOLYRGN();
extern long int IT_4I();
extern long int IT_1LPRc();
extern long int IT_1D();
extern long int IT_1H1LPPA1I();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_2H();
extern long int IT_1H1L1LPB();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1H1LPR();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1H1I();
extern long int IT_1H();
extern long int IT_GETOBJECT();
extern long int IT_1H2I();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1I();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1H1LP1I();
extern long int IT_1H1I1LP();
extern long int IT_1H1LPTM();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1H1LPPA1I();
extern long int IT_LINEDDA();
extern long int IT_1H2I();
extern long int IT_1H2I();
extern long int IT_1H1LPRc();
extern long int IT_1H1L1LPB();
extern long int IT_2H();
extern long int IT_1LP();
extern long int IT_1LP();
extern long int IT_1H();
extern long int IT_PLAYMETAFILE();
extern long int IT_ENUMMETAFILE();
extern long int IT_CLOSEMF();
extern long int IT_GETMF();
extern long int IT_SETMETAFILEBITS();
extern long int IT_3I();
extern long int IT_1H1LPR();
extern long int IT_1H2I();
extern long int IT_1H1UI2L();
extern long int IT_1H();
extern long int IT_CREATEDC();
extern long int IT_1H1D();
extern long int IT_1H2I();
extern long int IT_1H2I();
extern long int IT_1H4I();
extern long int IT_1H1LPRc();
extern long int IT_ENUMFONTS();
extern long int IT_ENUMOBJECTS();
extern long int IT_1H();
extern long int IT_1H1UI();
extern long int IT_1H8I();
extern long int IT_1H1D();
extern long int IT_GETCHARWID();
extern long int IT_EXTTEXTOUT();
extern long int IT_CREATEPAL();
extern long int IT_1H2I1D1UI();
extern long int IT_RESETDC();
extern long int IT_CREATEDIBM();
extern long int IT_SETDIBITS();
extern long int IT_GETDIBITS();
extern long int IT_SETDIBTD();
extern long int IT_6I();
extern long int IT_CREATEDIBPB();
extern long int IT_POLYPOLY();
extern long int IT_1H1LPP();
extern long int IT_1H1LPSZ();
extern long int IT_1H1LPP();
extern long int IT_1H1LPSZ();
extern long int IT_1H1LPP();
extern long int IT_1H1LP1I1LPSZ();
extern long int IT_1H2I1LPP();
extern long int IT_1H2I1LPP();
extern long int IT_1H2I1LPSZ();
extern long int IT_1H2I1LPP();
extern long int IT_1H2I1LPSZ();
extern long int IT_1H2I1LPP();
extern long int IT_1H2I1LPP();
extern long int IT_1H4I1LPSZ();
extern long int IT_1H4I1LPSZ();
extern long int IT_STRETCHDIBITS();
extern long int IT_GETRCAPS();
extern long int IT_GETPALENTRIES();
extern long int IT_SETPALENTRIES();
extern long int IT_2LP1H();
extern long int IT_1H1LP1W();
extern long int IT_1H();
extern long int IT_1H1W();
extern long int IT_1UI1LP1I();
extern long int IT_PLAYMETAREC();
extern long int IT_ENUMMETAFILE();
extern long int IT_COPYMETAFILE();
extern long int IT_SETABORTPROC();
extern long int IT_STARTDOC();
extern long int IT_GETCHARABC();
extern long int IT_1UI3LP();
extern long int IT_GETKERNPAIRS();
extern long int IT_1H1LPRc2I1D();
extern long int IT_1H1LPRc1UI();
extern long int IT_1H1LPR1UI();
extern long int IT_DMREALIZE();
extern long int IT_DMBITBLT();
extern long int IT_DMOUTPUT();
extern long int IT_DMCOLORINFO();
extern long int IT_DMPIXEL();

 /* Interface Segment Image GDI: */

static long int (*seg_image_GDI_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	IT_1H1D,  SetBkColor,
	/* 002 */	IT_1H1I,  SetBkMode,
	/* 003 */	IT_1H1I,  SetMapMode,
	/* 004 */	IT_1H1I,  SetROP2,
	/* 005 */	Trap, 0,
	/* 006 */	IT_1H1I,  SetPolyFillMode,
	/* 007 */	IT_1H1I,  SetStretchBltMode,
	/* 008 */	IT_1H1I,  SetTextCharacterExtra,
	/* 009 */	IT_1H1D,  SetTextColor,
	/* 00a */	IT_1H2I,  SetTextJustification,
	/* 00b */	IT_1H2I,  SetWindowOrg,
	/* 00c */	IT_1H2I,  SetWindowExt,
	/* 00d */	IT_1H2I,  SetViewportOrg,
	/* 00e */	IT_1H2I,  SetViewportExt,
	/* 00f */	IT_1H2I,  OffsetWindowOrg,
	/* 010 */	IT_1H4I,  ScaleWindowExt,
	/* 011 */	IT_1H2I,  OffsetViewportOrg,
	/* 012 */	IT_1H4I,  ScaleViewportExt,
	/* 013 */	IT_1H2I,  LineTo,
	/* 014 */	IT_1H2I,  MoveTo,
	/* 015 */	IT_1H4I,  ExcludeClipRect,
	/* 016 */	IT_1H4I,  IntersectClipRect,
	/* 017 */	IT_1H8I,  Arc,
	/* 018 */	IT_1H4I,  Ellipse,
	/* 019 */	IT_1H2I1D,  FloodFill,
	/* 01a */	IT_1H8I,  Pie,
	/* 01b */	IT_1H4I,  Rectangle,
	/* 01c */	IT_1H6I,  RoundRect,
	/* 01d */	IT_1H4I1D,  PatBlt,
	/* 01e */	IT_1H,  SaveDC,
	/* 01f */	IT_1H2I1D,  SetPixel,
	/* 020 */	IT_1H2I, OffsetClipRgn,
	/* 021 */	IT_TEXTOUT,  TextOut,
	/* 022 */	IT_BITBLT,  BitBlt,
	/* 023 */	IT_STRETCHBLT,  StretchBlt,
	/* 024 */	IT_POLY,  Polygon,
	/* 025 */	IT_POLY,  Polyline,
	/* 026 */	IT_ESCAPE,  Escape,
	/* 027 */	IT_1H1I,  RestoreDC,
	/* 028 */	IT_3H,  FillRgn,
	/* 029 */	IT_3H2I,  FrameRgn,
	/* 02a */	IT_2H,  InvertRgn,
	/* 02b */	IT_2H,  PaintRgn,
	/* 02c */	IT_2H,  SelectClipRgn,
	/* 02d */	IT_2H,  SelectObject,
	/* 02e */	Trap, 0,
	/* 02f */	IT_3H1I,  CombineRgn,
	/* 030 */	IT_2I2UI1LPB,  CreateBitmap,
	/* 031 */	IT_1LPBMc,  CreateBitmapIndirect,
	/* 032 */	IT_CRBRUSHIND,  CreateBrushIndirect,
	/* 033 */	IT_1H2I,  CreateCompatibleBitmap,
	/* 034 */	IT_1H,  CreateCompatibleDC,
	/* 035 */	IT_CREATEDC,  CreateDC,
	/* 036 */	IT_4I,  CreateEllipticRgn,
	/* 037 */	IT_1LPRc,  CreateEllipticRgnIndirect,
	/* 038 */	IT_CREATEFONT,  CreateFont,
	/* 039 */	IT_1LPLFc,  CreateFontIndirect,
	/* 03a */	IT_1I1D,  CreateHatchBrush,
	/* 03b */	Trap, 0,
	/* 03c */	IT_1H,  CreatePatternBrush,
	/* 03d */	IT_2I1D,  CreatePen,
	/* 03e */	IT_1LPLPc,  CreatePenIndirect,
	/* 03f */	IT_CREATEPOLYRGN,  CreatePolygonRgn,
	/* 040 */	IT_4I,  CreateRectRgn,
	/* 041 */	IT_1LPRc,  CreateRectRgnIndirect,
	/* 042 */	IT_1D,  CreateSolidBrush,
	/* 043 */	IT_1H1LPPA1I,  DPtoLP,
	/* 044 */	IT_1H,  DeleteDC,
	/* 045 */	IT_1H,  DeleteObject,
	/* 046 */	IT_ENUMFONTS,  EnumFonts,
	/* 047 */	IT_ENUMOBJECTS, EnumObjects,
	/* 048 */	IT_2H,  EqualRgn,
	/* 049 */	Trap, 0,
	/* 04a */	IT_1H1L1LPB,  GetBitmapBits,
	/* 04b */	IT_1H,  GetBkColor,
	/* 04c */	IT_1H,  GetBkMode,
	/* 04d */	IT_1H1LPR,  GetClipBox,
	/* 04e */	IT_1H,  GetCurrentPosition,
	/* 04f */	IT_1H,  GetDCOrg,
	/* 050 */	IT_1H1I,  GetDeviceCaps,
	/* 051 */	IT_1H,  GetMapMode,
	/* 052 */	IT_GETOBJECT,  GetObject,
	/* 053 */	IT_1H2I,  GetPixel,
	/* 054 */	IT_1H,  GetPolyFillMode,
	/* 055 */	IT_1H,  GetROP2,
	/* 056 */	IT_1H, GetRelAbs,
	/* 057 */	IT_1I,  GetStockObject,
	/* 058 */	IT_1H,  GetStretchBltMode,
	/* 059 */	IT_1H, GetTextCharacterExtra,
	/* 05a */	IT_1H,  GetTextColor,
	/* 05b */	IT_1H1LP1I,  GetTextExtent,
	/* 05c */	IT_1H1I1LP,  GetTextFace,
	/* 05d */	IT_1H1LPTM,  GetTextMetrics,
	/* 05e */	IT_1H,  GetViewportExt,
	/* 05f */	IT_1H,  GetViewportOrg,
	/* 060 */	IT_1H,  GetWindowExt,
	/* 061 */	IT_1H,  GetWindowOrg,
	/* 062 */	Trap, 0,
	/* 063 */	IT_1H1LPPA1I,  LPtoDP,
	/* 064 */	IT_LINEDDA,  LineDDA,
	/* 065 */	IT_1H2I,  OffsetRgn,
	/* 066 */	Trap, 0,
	/* 067 */	IT_1H2I,  PtVisible,
	/* 068 */	IT_1H1LPRc,  RectVisible,
	/* 069 */	Trap, 0,
	/* 06a */	IT_1H1L1LPB,  SetBitmapBits,
	/* 06b */	Trap, 0,
	/* 06c */	Trap, 0,
	/* 06d */	Trap, 0,
	/* 06e */	Trap, 0,
	/* 06f */	Trap, 0,
	/* 070 */	Trap, 0,
	/* 071 */	Trap, 0,
	/* 072 */	Trap, 0,
	/* 073 */	Trap, 0,
	/* 074 */	Trap, 0,
	/* 075 */	Trap, 0,
	/* 076 */	Trap, 0,
	/* 077 */	IT_1LP, AddFontResource,
	/* 078 */	Trap, 0,
	/* 079 */	Trap, 0,
	/* 07a */	Trap, 0,
	/* 07b */	IT_PLAYMETAFILE, PlayMetaFile,
	/* 07c */	IT_GETMF,  GetMetaFile,
	/* 07d */	IT_1LP,  CreateMetaFile,
	/* 07e */	IT_CLOSEMF,  CloseMetaFile,
	/* 07f */	IT_GLOBALFREE,  DeleteMetaFile,
	/* 080 */	IT_3I,  MulDiv,
	/* 081 */	Trap, 0,
	/* 082 */	Trap, 0,
	/* 083 */	Trap, 0,
	/* 084 */	IT_2LP1I, SetEnvironment,
	/* 085 */	IT_2LP1I, GetEnvironment,
	/* 086 */	IT_1H1LPR,  GetRgnBox,
	/* 087 */	Trap, 0,
	/* 088 */	IT_1LP, RemoveFontResource,
	/* 089 */	Trap, 0,
	/* 08a */	Trap, 0,
	/* 08b */	Trap, 0,
	/* 08c */	Trap, 0,
	/* 08d */	Trap, 0,
	/* 08e */	Trap, 0,
	/* 08f */	Trap, 0,
	/* 090 */	Trap, 0,
	/* 091 */	Trap, 0,
	/* 092 */	Trap, 0,
	/* 093 */	Trap, 0,
	/* 094 */	IT_1H2I,  SetBrushOrg,
	/* 095 */	IT_1H, GetBrushOrg,
	/* 096 */	IT_1H,  UnrealizeObject,
	/* 097 */	IT_COPYMETAFILE, CopyMetaFile,
	/* 098 */	Trap, 0,
	/* 099 */	IT_CREATEDC,  CreateIC,
	/* 09a */	IT_1H1D,  GetNearestColor,
	/* 09b */	Trap, 0,
	/* 09c */	IT_1H2I,  CreateDiscardableBitmap,
	/* 09d */	Trap, 0,
	/* 09e */	Trap, 0,
	/* 09f */	Trap, 0,
        /* 0a0 */       IT_SETMETAFILEBITS, SetMetaFileBits,
	/* 0a1 */	IT_1H2I,  PtInRegion,
	/* 0a2 */	IT_1H, GetBitmapDimension,
	/* 0a3 */	IT_1H2I, SetBitmapDimension,
	/* 0a4 */	Trap, 0,
	/* 0a5 */	Trap, 0,
	/* 0a6 */	Trap, 0,
	/* 0a7 */	Trap, 0,
	/* 0a8 */	Trap, 0,
	/* 0a9 */	Trap, 0,
	/* 0aa */	Trap, 0,
	/* 0ab */	Trap, 0,
	/* 0ac */	IT_1H4I,  SetRectRgn,
	/* 0ad */	Trap, 0,
	/* 0ae */	Trap, 0,
	/* 0af */	IT_ENUMMETAFILE, EnumMetaFile,
	/* 0b0 */	IT_PLAYMETAREC, PlayMetaFileRecord,
	/* 0b1 */	Trap, 0,
	/* 0b2 */	Trap, 0,
	/* 0b3 */	Trap, 0,
	/* 0b4 */	Trap, 0,
	/* 0b5 */	IT_1H1LPRc,  RectInRegion,
	/* 0b6 */	Trap, 0,
	/* 0b7 */	Trap, 0,
	/* 0b8 */	Trap, 0,
	/* 0b9 */	Trap, 0,
	/* 0ba */	Trap, 0,
	/* 0bb */	Trap, 0,
	/* 0bc */	Trap, 0,
	/* 0bd */	Trap, 0,
	/* 0be */	Trap, 0,
	/* 0bf */	Trap, 0,
	/* 0c0 */	Trap, 0,
	/* 0c1 */	IT_1H1LPRc1UI, SetBoundsRect,
	/* 0c2 */	IT_1H1LPR1UI, GetBoundsRect,
	/* 0c3 */	Trap, 0,
        /* 0c4 */       IT_SETMETAFILEBITS, SetMetaFileBitsBetter,
	/* 0c5 */	Trap, 0,
	/* 0c6 */	Trap, 0,
	/* 0c7 */	Trap, 0,
	/* 0c8 */	Trap, 0,
	/* 0c9 */	IT_DMBITBLT, dmBitBlt,
	/* 0ca */	IT_DMCOLORINFO, dmColorInfo,
	/* 0cb */	Trap, 0,
	/* 0cc */	Trap, 0,
	/* 0cd */	Trap, 0,
	/* 0ce */	Trap, 0,
	/* 0cf */	Trap, 0,
	/* 0d0 */	IT_DMOUTPUT, dmOutput,
	/* 0d1 */	IT_DMPIXEL, dmPixel,
	/* 0d2 */	IT_DMREALIZE, dmRealizeObject,
	/* 0d3 */	Trap, 0,
	/* 0d4 */	Trap, 0,
	/* 0d5 */	Trap, 0,
	/* 0d6 */	Trap, 0,
	/* 0d7 */	Trap, 0,
	/* 0d8 */	Trap, 0,
	/* 0d9 */	Trap, 0,
	/* 0da */	Trap, 0,
	/* 0db */	Trap, 0,
	/* 0dc */	Trap, 0,
	/* 0dd */	Trap, 0,
	/* 0de */	Trap, 0,
	/* 0df */	Trap, 0,
	/* 0e0 */	Trap, 0,
	/* 0e1 */	Trap, 0,
	/* 0e2 */	Trap, 0,
	/* 0e3 */	Trap, 0,
	/* 0e4 */	Trap, 0,
	/* 0e5 */	Trap, 0,
	/* 0e6 */	Trap, 0,
	/* 0e7 */	Trap, 0,
	/* 0e8 */	Trap, 0,
	/* 0e9 */	Trap, 0,
	/* 0ea */	Trap, 0,
	/* 0eb */	Trap, 0,
	/* 0ec */	Trap, 0,
	/* 0ed */	Trap, 0,
	/* 0ee */	Trap, 0,
	/* 0ef */	Trap, 0,
	/* 0f0 */	IT_2LP1H, OpenJob,
	/* 0f1 */	IT_1UI1LP1I, WriteSpool,
	/* 0f2 */	Trap, 0,
	/* 0f3 */	IT_1UI, CloseJob,
	/* 0f4 */	IT_2UI, DeleteJob,
	/* 0f5 */	Trap, 0,
	/* 0f6 */	IT_1UI, StartSpoolPage,
	/* 0f7 */	IT_1UI, EndSpoolPage,
	/* 0f8 */	Trap, 0,
	/* 0f9 */	Trap, 0,
	/* 0fa */	Trap, 0,
	/* 0fb */	Trap, 0,
	/* 0fc */	Trap, 0,
	/* 0fd */	Trap, 0,
	/* 0fe */	Trap, 0,
	/* 0ff */	Trap, 0,
	/* 100 */	Trap, 0,
	/* 101 */	Trap, 0,
	/* 102 */	Trap, 0,
	/* 103 */	Trap, 0,
	/* 104 */	Trap, 0,
	/* 105 */	Trap, 0,
	/* 106 */	Trap, 0,
	/* 107 */	Trap, 0,
	/* 108 */	Trap, 0,
	/* 109 */	Trap, 0,
	/* 10a */	Trap, 0,
	/* 10b */	Trap, 0,
	/* 10c */	Trap, 0,
	/* 10d */	Trap, 0,
	/* 10e */	Trap, 0,
	/* 10f */	Trap, 0,
	/* 110 */	Trap, 0,
	/* 111 */	Trap, 0,
	/* 112 */	Trap, 0,
	/* 113 */	Trap, 0,
	/* 114 */	Trap, 0,
	/* 115 */	Trap, 0,
	/* 116 */	Trap, 0,
	/* 117 */	Trap, 0,
	/* 118 */	Trap, 0,
	/* 119 */	Trap, 0,
	/* 11a */	Trap, 0,
	/* 11b */	Trap, 0,
	/* 11c */	Trap, 0,
	/* 11d */	Trap, 0,
	/* 11e */	Trap, 0,
	/* 11f */	Trap, 0,
	/* 120 */	Trap, 0,
	/* 121 */	Trap, 0,
	/* 122 */	Trap, 0,
	/* 123 */	Trap, 0,
	/* 124 */	Trap, 0,
	/* 125 */	Trap, 0,
	/* 126 */	Trap, 0,
	/* 127 */	Trap, 0,
	/* 128 */	Trap, 0,
	/* 129 */	Trap, 0,
	/* 12a */	Trap, 0,
	/* 12b */	Trap, 0,
	/* 12c */	Trap, 0,
	/* 12d */	IT_1LP, EngineDeleteFont,
	/* 12e */	IT_3LP, EngineRealizeFont,
	/* 12f */	Trap, 0,
	/* 130 */	Trap, 0,
	/* 131 */	Trap, 0,
	/* 132 */	Trap, 0,
	/* 133 */	IT_GETCHARABC, GetCharABCWidths,
	/* 134 */	Trap, 0,
	/* 135 */	Trap, 0,
	/* 136 */	IT_1UI3LP, CreateScalableFontResource,
	/* 137 */	Trap, 0,
	/* 138 */	Trap, 0,
	/* 139 */	IT_GETRCAPS, GetRasterizerCaps,
	/* 13a */	Trap, 0,
	/* 13b */	Trap, 0,
	/* 13c */	Trap, 0,
	/* 13d */	Trap, 0,
	/* 13e */	Trap, 0,
	/* 13f */	Trap, 0,
	/* 140 */	Trap, 0,
	/* 141 */	Trap, 0,
	/* 142 */	Trap, 0,
	/* 143 */	Trap, 0,
	/* 144 */	Trap, 0,
	/* 145 */	Trap, 0,
	/* 146 */	Trap, 0,
	/* 147 */	Trap, 0,
	/* 148 */	Trap, 0,
	/* 149 */	Trap, 0,
	/* 14a */	IT_ENUMFONTS,  EnumFontFamilies,
	/* 14b */	Trap, 0,
	/* 14c */	IT_GETKERNPAIRS, GetKerningPairs,
	/* 14d */	Trap, 0,
	/* 14e */	Trap, 0,
	/* 14f */	Trap, 0,
	/* 150 */	Trap, 0,
	/* 151 */	Trap, 0,
	/* 152 */	Trap, 0,
	/* 153 */	Trap, 0,
	/* 154 */	Trap, 0,
	/* 155 */	Trap, 0,
	/* 156 */	Trap, 0,
	/* 157 */	Trap, 0,
	/* 158 */	Trap, 0,
	/* 159 */	IT_1H,  GetTextAlign,
	/* 15a */	IT_1H1UI,  SetTextAlign,
	/* 15b */	Trap, 0,
	/* 15c */	IT_1H8I,  Chord,
	/* 15d */	IT_1H1D,  SetMapperFlags,
	/* 15e */	IT_GETCHARWID,  GetCharWidth,
	/* 15f */	IT_EXTTEXTOUT,  ExtTextOut,
	/* 160 */	Trap, 0,
	/* 161 */	IT_1H, GetAspectRatioFilter,
	/* 162 */	Trap, 0,
	/* 163 */	Trap, 0,
	/* 164 */	Trap, 0,
	/* 165 */	Trap, 0,
	/* 166 */	Trap, 0,
	/* 167 */	Trap, 0,
	/* 168 */	IT_CREATEPAL,  CreatePalette,
	/* 169 */	Trap, 0,
	/* 16a */	Trap, 0,
	/* 16b */	IT_GETPALENTRIES, GetPaletteEntries,
	/* 16c */	IT_SETPALENTRIES, SetPaletteEntries,
	/* 16d */	Trap, 0,
	/* 16e */	IT_1H, UpdateColors,
	/* 16f */	IT_SETPALENTRIES, AnimatePalette,
	/* 170 */	IT_1H1UI, ResizePalette,
	/* 171 */	Trap, 0,
	/* 172 */	IT_1H1D, GetNearestPaletteIndex,
	/* 173 */	Trap, 0,
	/* 174 */	IT_1H2I1D1UI,  ExtFloodFill,
	/* 175 */	IT_1H1I, SetSystemPaletteUse,
	/* 176 */	IT_1H, GetSystemPaletteUse,
	/* 177 */	IT_GETPALENTRIES, GetSystemPaletteEntries,
	/* 178 */	IT_RESETDC,  ResetDC,
	/* 179 */	IT_STARTDOC, StartDoc,
	/* 17a */	IT_1H, EndDoc,
	/* 17b */	IT_1H, StartPage,
	/* 17c */	IT_1H, EndPage,
	/* 17d */	IT_SETABORTPROC, SetAbortProc,
	/* 17e */	IT_1H, AbortDoc,
	/* 17f */	Trap, 0,
	/* 180 */	Trap, 0,
	/* 181 */	Trap, 0,
	/* 182 */	Trap, 0,
	/* 183 */	Trap, 0,
	/* 184 */	Trap, 0,
	/* 185 */	Trap, 0,
	/* 186 */	Trap, 0,
	/* 187 */	Trap, 0,
	/* 188 */	Trap, 0,
	/* 189 */	Trap, 0,
	/* 18a */	Trap, 0,
	/* 18b */	Trap, 0,
	/* 18c */	Trap, 0,
	/* 18d */	Trap, 0,
	/* 18e */	Trap, 0,
	/* 18f */	Trap, 0,
	/* 190 */	IT_1H1LPRc2I1D, FastWindowFrame,
	/* 191 */	Trap, 0,
	/* 192 */	Trap, 0,
	/* 193 */	Trap, 0,
	/* 194 */	Trap, 0,
	/* 195 */	Trap, 0,
	/* 196 */	Trap, 0,
	/* 197 */	Trap, 0,
	/* 198 */	Trap, 0,
	/* 199 */	Trap, 0,
        /* 19a */       IT_GLOBALFREE, IsValidMetaFile,
	/* 19b */	Trap, 0,
	/* 19c */	Trap, 0,
	/* 19d */	Trap, 0,
	/* 19e */	Trap, 0,
	/* 19f */	Trap, 0,
	/* 1a0 */	Trap, 0,
	/* 1a1 */	Trap, 0,
	/* 1a2 */	Trap, 0,
	/* 1a3 */	Trap, 0,
	/* 1a4 */	Trap, 0,
	/* 1a5 */	Trap, 0,
	/* 1a6 */	Trap, 0,
	/* 1a7 */	Trap, 0,
	/* 1a8 */	Trap, 0,
	/* 1a9 */	Trap, 0,
	/* 1aa */	Trap, 0,
	/* 1ab */	Trap, 0,
	/* 1ac */	Trap, 0,
	/* 1ad */	Trap, 0,
	/* 1ae */	Trap, 0,
	/* 1af */	Trap, 0,
	/* 1b0 */	Trap, 0,
	/* 1b1 */	Trap, 0,
	/* 1b2 */	Trap, 0,
	/* 1b3 */	Trap, 0,
	/* 1b4 */	Trap, 0,
	/* 1b5 */	Trap, 0,
	/* 1b6 */	Trap, 0,
	/* 1b7 */	IT_STRETCHDIBITS, StretchDIBits,
	/* 1b8 */	IT_SETDIBITS, SetDIBits,
	/* 1b9 */	IT_GETDIBITS, GetDIBits,
	/* 1ba */	IT_CREATEDIBM,  CreateDIBitmap,
	/* 1bb */	IT_SETDIBTD, SetDIBitsToDevice,
	/* 1bc */	IT_6I,  CreateRoundRectRgn,
	/* 1bd */	IT_CREATEDIBPB, CreateDIBPatternBrush,
	/* 1be */	Trap, 0,
	/* 1bf */	Trap, 0,
	/* 1c0 */	Trap, 0,
	/* 1c1 */	Trap, 0,
	/* 1c2 */	IT_POLYPOLY,  PolyPolygon,
	/* 1c3 */	IT_CREATEPOLYPOLYRGN, CreatePolyPolygonRgn,
	/* 1c4 */	Trap, 0,
	/* 1c5 */	Trap, 0,
	/* 1c6 */	Trap, 0,
	/* 1c7 */	Trap, 0,
	/* 1c8 */	Trap, 0,
	/* 1c9 */	Trap, 0,
	/* 1ca */	Trap, 0,
	/* 1cb */	Trap, 0,
	/* 1cc */	Trap, 0,
	/* 1cd */	Trap, 0,
	/* 1ce */	IT_1H, IsGDIObject,
	/* 1cf */	Trap, 0,
	/* 1d0 */	Trap, 0,
	/* 1d1 */	IT_1H1LPRc,  RectVisible,
	/* 1d2 */	Trap, 0,
	/* 1d3 */	Trap, 0,
	/* 1d4 */	IT_1H1LPSZ, GetBitmapDimensionEx,
	/* 1d5 */	IT_1H1LPP, GetBrushOrgEx,
	/* 1d6 */	IT_1H1LPP,  GetCurrentPositionEx,
	/* 1d7 */	IT_1H1LP1I1LPSZ, GetTextExtentPoint,
	/* 1d8 */	IT_1H1LPSZ,  GetViewportExtEx,
	/* 1d9 */	IT_1H1LPP,  GetViewportOrgEx,
	/* 1da */	IT_1H1LPSZ,  GetWindowExtEx,
	/* 1db */	IT_1H1LPP,  GetWindowOrgEx,
	/* 1dc */	IT_1H2I1LPP,  OffsetViewportOrgEx,
	/* 1dd */	IT_1H2I1LPP,  OffsetWindowOrgEx,
	/* 1de */	IT_1H2I1LPSZ, SetBitmapDimensionEx,
	/* 1df */	IT_1H2I1LPSZ,  SetViewportExtEx,
	/* 1e0 */	IT_1H2I1LPP,  SetViewportOrgEx,
	/* 1e1 */	IT_1H2I1LPSZ,  SetWindowExtEx,
	/* 1e2 */	IT_1H2I1LPP,  SetWindowOrgEx,
	/* 1e3 */	IT_1H2I1LPP,  MoveToEx,
	/* 1e4 */	IT_1H4I1LPSZ,  ScaleViewportExtEx,
	/* 1e5 */	IT_1H4I1LPSZ,  ScaleWindowExtEx,
	/* 1e6 */	IT_1H1LPSZ, GetAspectRatioFilterEx,
	0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
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
	/* 014 */	Trap, 0,
	/* 015 */	Trap, 0,
	/* 016 */	Trap, 0,
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
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	Trap, 0,
	/* 025 */	Trap, 0,
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
	/* 066 */	Trap, 0,
	/* 067 */	Trap, 0,
	/* 068 */	Trap, 0,
	/* 069 */	Trap, 0,
	/* 06a */	Trap, 0,
	/* 06b */	Trap, 0,
	/* 06c */	Trap, 0,
	/* 06d */	Trap, 0,
	/* 06e */	Trap, 0,
	/* 06f */	Trap, 0,
	/* 070 */	Trap, 0,
	/* 071 */	Trap, 0,
	/* 072 */	Trap, 0,
	/* 073 */	Trap, 0,
	/* 074 */	Trap, 0,
	/* 075 */	Trap, 0,
	/* 076 */	Trap, 0,
	/* 077 */	Trap, 0,
	/* 078 */	Trap, 0,
	/* 079 */	Trap, 0,
	/* 07a */	Trap, 0,
	/* 07b */	Trap, 0,
	/* 07c */	Trap, 0,
	/* 07d */	Trap, 0,
	/* 07e */	Trap, 0,
	/* 07f */	Trap, 0,
	/* 080 */	Trap, 0,
	/* 081 */	Trap, 0,
	/* 082 */	Trap, 0,
	/* 083 */	Trap, 0,
	/* 084 */	Trap, 0,
	/* 085 */	Trap, 0,
	/* 086 */	Trap, 0,
	/* 087 */	Trap, 0,
	/* 088 */	Trap, 0,
	/* 089 */	Trap, 0,
	/* 08a */	Trap, 0,
	/* 08b */	Trap, 0,
	/* 08c */	Trap, 0,
	/* 08d */	Trap, 0,
	/* 08e */	Trap, 0,
	/* 08f */	Trap, 0,
	/* 090 */	Trap, 0,
	/* 091 */	Trap, 0,
	/* 092 */	Trap, 0,
	/* 093 */	Trap, 0,
	/* 094 */	Trap, 0,
	/* 095 */	Trap, 0,
	/* 096 */	Trap, 0,
	/* 097 */	Trap, 0,
	/* 098 */	Trap, 0,
	/* 099 */	Trap, 0,
	/* 09a */	Trap, 0,
	/* 09b */	Trap, 0,
	/* 09c */	Trap, 0,
	/* 09d */	Trap, 0,
	/* 09e */	Trap, 0,
	/* 09f */	Trap, 0,
	/* 0a0 */	Trap, 0,
	/* 0a1 */	Trap, 0,
	/* 0a2 */	Trap, 0,
	/* 0a3 */	Trap, 0,
	/* 0a4 */	Trap, 0,
	/* 0a5 */	Trap, 0,
	/* 0a6 */	Trap, 0,
	/* 0a7 */	Trap, 0,
	/* 0a8 */	Trap, 0,
	/* 0a9 */	Trap, 0,
	/* 0aa */	Trap, 0,
	/* 0ab */	Trap, 0,
	/* 0ac */	Trap, 0,
	/* 0ad */	Trap, 0,
	/* 0ae */	Trap, 0,
	/* 0af */	Trap, 0,
	/* 0b0 */	Trap, 0,
	/* 0b1 */	Trap, 0,
	/* 0b2 */	Trap, 0,
	/* 0b3 */	Trap, 0,
	/* 0b4 */	Trap, 0,
	/* 0b5 */	Trap, 0,
	/* 0b6 */	Trap, 0,
	/* 0b7 */	Trap, 0,
	/* 0b8 */	Trap, 0,
	/* 0b9 */	Trap, 0,
	/* 0ba */	Trap, 0,
	/* 0bb */	Trap, 0,
	/* 0bc */	Trap, 0,
	/* 0bd */	Trap, 0,
	/* 0be */	Trap, 0,
	/* 0bf */	Trap, 0,
	/* 0c0 */	Trap, 0,
	/* 0c1 */	Trap, 0,
	/* 0c2 */	Trap, 0,
	/* 0c3 */	Trap, 0,
	/* 0c4 */	Trap, 0,
	/* 0c5 */	Trap, 0,
	/* 0c6 */	Trap, 0,
	/* 0c7 */	Trap, 0,
	/* 0c8 */	Trap, 0,
	/* 0c9 */	Trap, 0,
	/* 0ca */	Trap, 0,
	/* 0cb */	Trap, 0,
	/* 0cc */	Trap, 0,
	/* 0cd */	Trap, 0,
	/* 0ce */	Trap, 0,
	/* 0cf */	Trap, 0,
	/* 0d0 */	Trap, 0,
	/* 0d1 */	Trap, 0,
	/* 0d2 */	Trap, 0,
	/* 0d3 */	Trap, 0,
	/* 0d4 */	Trap, 0,
	/* 0d5 */	Trap, 0,
	/* 0d6 */	Trap, 0,
	/* 0d7 */	Trap, 0,
	/* 0d8 */	Trap, 0,
	/* 0d9 */	Trap, 0,
	/* 0da */	Trap, 0,
	/* 0db */	Trap, 0,
	/* 0dc */	Trap, 0,
	/* 0dd */	Trap, 0,
	/* 0de */	Trap, 0,
	/* 0df */	Trap, 0,
	/* 0e0 */	Trap, 0,
	/* 0e1 */	Trap, 0,
	/* 0e2 */	Trap, 0,
	/* 0e3 */	Trap, 0,
	/* 0e4 */	Trap, 0,
	/* 0e5 */	Trap, 0,
	/* 0e6 */	Trap, 0,
	/* 0e7 */	Trap, 0,
	/* 0e8 */	Trap, 0,
	/* 0e9 */	Trap, 0,
	/* 0ea */	Trap, 0,
	/* 0eb */	Trap, 0,
	/* 0ec */	Trap, 0,
	/* 0ed */	Trap, 0,
	/* 0ee */	Trap, 0,
	/* 0ef */	Trap, 0,
	/* 0f0 */	Trap, 0,
	/* 0f1 */	Trap, 0,
	/* 0f2 */	Trap, 0,
	/* 0f3 */	Trap, 0,
	/* 0f4 */	Trap, 0,
	/* 0f5 */	Trap, 0,
	/* 0f6 */	Trap, 0,
	/* 0f7 */	Trap, 0,
	/* 0f8 */	Trap, 0,
	/* 0f9 */	Trap, 0,
	/* 0fa */	Trap, 0,
	/* 0fb */	Trap, 0,
	/* 0fc */	Trap, 0,
	/* 0fd */	Trap, 0,
	/* 0fe */	Trap, 0,
	/* 0ff */	Trap, 0,
	/* 100 */	Trap, 0,
	/* 101 */	Trap, 0,
	/* 102 */	Trap, 0,
	/* 103 */	Trap, 0,
	/* 104 */	Trap, 0,
	/* 105 */	Trap, 0,
	/* 106 */	Trap, 0,
	/* 107 */	Trap, 0,
	/* 108 */	Trap, 0,
	/* 109 */	Trap, 0,
	/* 10a */	Trap, 0,
	/* 10b */	Trap, 0,
	/* 10c */	Trap, 0,
	/* 10d */	Trap, 0,
	/* 10e */	Trap, 0,
	/* 10f */	Trap, 0,
	/* 110 */	Trap, 0,
	/* 111 */	Trap, 0,
	/* 112 */	Trap, 0,
	/* 113 */	Trap, 0,
	/* 114 */	Trap, 0,
	/* 115 */	Trap, 0,
	/* 116 */	Trap, 0,
	/* 117 */	Trap, 0,
	/* 118 */	Trap, 0,
	/* 119 */	Trap, 0,
	/* 11a */	Trap, 0,
	/* 11b */	Trap, 0,
	/* 11c */	Trap, 0,
	/* 11d */	Trap, 0,
	/* 11e */	Trap, 0,
	/* 11f */	Trap, 0,
	/* 120 */	Trap, 0,
	/* 121 */	Trap, 0,
	/* 122 */	Trap, 0,
	/* 123 */	Trap, 0,
	/* 124 */	Trap, 0,
	/* 125 */	Trap, 0,
	/* 126 */	Trap, 0,
	/* 127 */	Trap, 0,
	/* 128 */	Trap, 0,
	/* 129 */	Trap, 0,
	/* 12a */	Trap, 0,
	/* 12b */	Trap, 0,
	/* 12c */	Trap, 0,
	/* 12d */	Trap, 0,
	/* 12e */	Trap, 0,
	/* 12f */	Trap, 0,
	/* 130 */	Trap, 0,
	/* 131 */	Trap, 0,
	/* 132 */	Trap, 0,
	/* 133 */	Trap, 0,
	/* 134 */	Trap, 0,
	/* 135 */	Trap, 0,
	/* 136 */	Trap, 0,
	/* 137 */	Trap, 0,
	/* 138 */	Trap, 0,
	/* 139 */	Trap, 0,
	/* 13a */	Trap, 0,
	/* 13b */	Trap, 0,
	/* 13c */	Trap, 0,
	/* 13d */	Trap, 0,
	/* 13e */	Trap, 0,
	/* 13f */	Trap, 0,
	/* 140 */	Trap, 0,
	/* 141 */	Trap, 0,
	/* 142 */	Trap, 0,
	/* 143 */	Trap, 0,
	/* 144 */	Trap, 0,
	/* 145 */	Trap, 0,
	/* 146 */	Trap, 0,
	/* 147 */	Trap, 0,
	/* 148 */	Trap, 0,
	/* 149 */	Trap, 0,
	/* 14a */	Trap, 0,
	/* 14b */	Trap, 0,
	/* 14c */	Trap, 0,
	/* 14d */	Trap, 0,
	/* 14e */	Trap, 0,
	/* 14f */	Trap, 0,
	/* 150 */	Trap, 0,
	/* 151 */	Trap, 0,
	/* 152 */	Trap, 0,
	/* 153 */	Trap, 0,
	/* 154 */	Trap, 0,
	/* 155 */	Trap, 0,
	/* 156 */	Trap, 0,
	/* 157 */	Trap, 0,
	/* 158 */	Trap, 0,
	/* 159 */	Trap, 0,
	/* 15a */	Trap, 0,
	/* 15b */	Trap, 0,
	/* 15c */	Trap, 0,
	/* 15d */	Trap, 0,
	/* 15e */	Trap, 0,
	/* 15f */	Trap, 0,
	/* 160 */	Trap, 0,
	/* 161 */	Trap, 0,
	/* 162 */	Trap, 0,
	/* 163 */	Trap, 0,
	/* 164 */	Trap, 0,
	/* 165 */	Trap, 0,
	/* 166 */	Trap, 0,
	/* 167 */	Trap, 0,
	/* 168 */	Trap, 0,
	/* 169 */	Trap, 0,
	/* 16a */	Trap, 0,
	/* 16b */	Trap, 0,
	/* 16c */	Trap, 0,
	/* 16d */	Trap, 0,
	/* 16e */	Trap, 0,
	/* 16f */	Trap, 0,
	/* 170 */	Trap, 0,
	/* 171 */	Trap, 0,
	/* 172 */	Trap, 0,
	/* 173 */	Trap, 0,
	/* 174 */	Trap, 0,
	/* 175 */	Trap, 0,
	/* 176 */	Trap, 0,
	/* 177 */	Trap, 0,
	/* 178 */	Trap, 0,
	/* 179 */	Trap, 0,
	/* 17a */	Trap, 0,
	/* 17b */	Trap, 0,
	/* 17c */	Trap, 0,
	/* 17d */	Trap, 0,
	/* 17e */	Trap, 0,
	/* 17f */	Trap, 0,
	/* 180 */	Trap, 0,
	/* 181 */	Trap, 0,
	/* 182 */	Trap, 0,
	/* 183 */	Trap, 0,
	/* 184 */	Trap, 0,
	/* 185 */	Trap, 0,
	/* 186 */	Trap, 0,
	/* 187 */	Trap, 0,
	/* 188 */	Trap, 0,
	/* 189 */	Trap, 0,
	/* 18a */	Trap, 0,
	/* 18b */	Trap, 0,
	/* 18c */	Trap, 0,
	/* 18d */	Trap, 0,
	/* 18e */	Trap, 0,
	/* 18f */	Trap, 0,
	/* 190 */	Trap, 0,
	/* 191 */	Trap, 0,
	/* 192 */	Trap, 0,
	/* 193 */	Trap, 0,
	/* 194 */	Trap, 0,
	/* 195 */	Trap, 0,
	/* 196 */	Trap, 0,
	/* 197 */	Trap, 0,
	/* 198 */	Trap, 0,
	/* 199 */	Trap, 0,
	/* 19a */	Trap, 0,
	/* 19b */	Trap, 0,
	/* 19c */	Trap, 0,
	/* 19d */	Trap, 0,
	/* 19e */	Trap, 0,
	/* 19f */	Trap, 0,
	/* 1a0 */	Trap, 0,
	/* 1a1 */	Trap, 0,
	/* 1a2 */	Trap, 0,
	/* 1a3 */	Trap, 0,
	/* 1a4 */	Trap, 0,
	/* 1a5 */	Trap, 0,
	/* 1a6 */	Trap, 0,
	/* 1a7 */	Trap, 0,
	/* 1a8 */	Trap, 0,
	/* 1a9 */	Trap, 0,
	/* 1aa */	Trap, 0,
	/* 1ab */	Trap, 0,
	/* 1ac */	Trap, 0,
	/* 1ad */	Trap, 0,
	/* 1ae */	Trap, 0,
	/* 1af */	Trap, 0,
	/* 1b0 */	Trap, 0,
	/* 1b1 */	Trap, 0,
	/* 1b2 */	Trap, 0,
	/* 1b3 */	Trap, 0,
	/* 1b4 */	Trap, 0,
	/* 1b5 */	Trap, 0,
	/* 1b6 */	Trap, 0,
	/* 1b7 */	Trap, 0,
	/* 1b8 */	Trap, 0,
	/* 1b9 */	Trap, 0,
	/* 1ba */	Trap, 0,
	/* 1bb */	Trap, 0,
	/* 1bc */	Trap, 0,
	/* 1bd */	Trap, 0,
	/* 1be */	Trap, 0,
	/* 1bf */	Trap, 0,
	/* 1c0 */	Trap, 0,
	/* 1c1 */	Trap, 0,
	/* 1c2 */	Trap, 0,
	/* 1c3 */	Trap, 0,
	/* 1c4 */	Trap, 0,
	/* 1c5 */	Trap, 0,
	/* 1c6 */	Trap, 0,
	/* 1c7 */	Trap, 0,
	/* 1c8 */	Trap, 0,
	/* 1c9 */	Trap, 0,
	/* 1ca */	Trap, 0,
	/* 1cb */	Trap, 0,
	/* 1cc */	Trap, 0,
	/* 1cd */	Trap, 0,
	/* 1ce */	Trap, 0,
	/* 1cf */	Trap, 0,
	/* 1d0 */	Trap, 0,
	/* 1d1 */	Trap, 0,
	/* 1d2 */	Trap, 0,
	/* 1d3 */	Trap, 0,
	/* 1d4 */	Trap, 0,
	/* 1d5 */	Trap, 0,
	/* 1d6 */	Trap, 0,
	/* 1d7 */	Trap, 0,
	/* 1d8 */	Trap, 0,
	/* 1d9 */	Trap, 0,
	/* 1da */	Trap, 0,
	/* 1db */	Trap, 0,
	/* 1dc */	Trap, 0,
	/* 1dd */	Trap, 0,
	/* 1de */	Trap, 0,
	/* 1df */	Trap, 0,
	/* 1e0 */	Trap, 0,
	/* 1e1 */	Trap, 0,
	/* 1e2 */	Trap, 0,
	/* 1e3 */	Trap, 0,
	/* 1e4 */	Trap, 0,
	/* 1e5 */	Trap, 0,
	/* 1e6 */	Trap, 0,
	0
};
#endif


 /* Segment Table GDI: */

static SEGTAB seg_tab_GDI[] =
{	{ (char *) seg_image_GDI_0, 3888, TRANSFER_CALLBACK, 3888, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

 /* Module Descriptor for GDI: */

static MODULEDSCR hsmt_mod_dscr_GDI =
{	"GDI",
	entry_tab_GDI,
	seg_tab_GDI,
	0
};

extern long int MessageBox();
extern long int OldExitWindows();
extern long int InitApp();
extern long int PostQuitMessage();
extern long int SetTimer();
extern long int SetSystemTimer();
extern long int KillTimer();
extern long int UserKillSystemTimer();
extern long int GetTickCount();
extern long int GetCurrentTime();
extern long int GetCursorPos();
extern long int SetCapture();
extern long int ReleaseCapture();
extern long int SetDoubleClickTime();
extern long int GetDoubleClickTime();
extern long int SetFocus();
extern long int GetFocus();
extern long int RemoveProp();
extern long int GetProp();
extern long int SetProp();
extern long int EnumProps();
extern long int ClientToScreen();
extern long int ScreenToClient();
extern long int WindowFromPoint();
extern long int IsIconic();
extern long int GetWindowRect();
extern long int GetClientRect();
extern long int EnableWindow();
extern long int IsWindowEnabled();
extern long int GetWindowText();
extern long int SetWindowText();
extern long int GetWindowTextLength();
extern long int BeginPaint();
extern long int EndPaint();
extern long int CreateWindow();
extern long int ShowWindow();
extern long int BringWindowToTop();
extern long int GetParent();
extern long int IsWindow();
extern long int IsChild();
extern long int IsWindowVisible();
extern long int FindWindow();
extern long int AnyPopup();
extern long int DestroyWindow();
extern long int EnumWindows();
extern long int EnumChildWindows();
extern long int MoveWindow();
extern long int RegisterClass();
extern long int GetClassName();
extern long int SetActiveWindow();
extern long int GetActiveWindow();
extern long int ScrollWindow();
extern long int SetScrollPos();
extern long int GetScrollPos();
extern long int SetScrollRange();
extern long int GetScrollRange();
extern long int GetDC();
extern long int GetDCEx();
extern long int GetWindowDC();
extern long int ReleaseDC();
extern long int ClipCursor();
extern long int SetCursor();
extern long int SetCursorPos();
extern long int ShowCursor();
extern long int SetRect();
extern long int SetRectEmpty();
extern long int CopyRect();
extern long int IsRectEmpty();
extern long int PtInRect();
extern long int OffsetRect();
extern long int InflateRect();
extern long int IntersectRect();
extern long int UnionRect();
extern long int FillRect();
extern long int InvertRect();
extern long int FrameRect();
extern long int DrawIcon();
extern long int DrawText();
extern long int DialogBox();
extern long int EndDialog();
extern long int CreateDialog();
extern long int IsDialogMessage();
extern long int GetDlgItem();
extern long int SetDlgItemText();
extern long int GetDlgItemText();
extern long int SetDlgItemInt();
extern long int GetDlgItemInt();
extern long int CheckRadioButton();
extern long int CheckDlgButton();
extern long int IsDlgButtonChecked();
extern long int DlgDirSelect();
extern long int DlgDirList();
extern long int DlgDirListComboBox();
extern long int SendDlgItemMessage();
extern long int AdjustWindowRect();
extern long int MessageBeep();
extern long int FlashWindow();
extern long int GetKeyState();
extern long int DefWindowProc();
extern long int GetMessage();
extern long int PeekMessage();
extern long int PostMessage();
extern long int SendMessage();
extern long int WaitMessage();
extern long int TranslateMessage();
extern long int DispatchMessage();
extern long int ReplyMessage();
extern long int PostAppMessage();
extern long int RegisterWindowMessage();
extern long int GetMessagePos();
extern long int GetMessageTime();
extern long int SetWindowsHook();
extern long int SetWindowsHookEx();
extern long int CallWindowProc();
extern long int UpdateWindow();
extern long int InvalidateRect();
extern long int InvalidateRgn();
extern long int ValidateRect();
extern long int ValidateRgn();
extern long int GetClassWord();
extern long int SetClassWord();
extern long int GetClassLong();
extern long int SetClassLong();
extern long int GetWindowWord();
extern long int SetWindowWord();
extern long int GetWindowLong();
extern long int SetWindowLong();
extern long int OpenClipboard();
extern long int CloseClipboard();
extern long int EmptyClipboard();
extern long int GetClipboardOwner();
extern long int SetClipboardData();
extern long int GetClipboardData();
extern long int CountClipboardFormats();
extern long int EnumClipboardFormats();
extern long int RegisterClipboardFormat();
extern long int GetClipboardFormatName();
extern long int SetClipboardViewer();
extern long int GetClipboardViewer();
extern long int ChangeClipboardChain();
extern long int LoadMenu();
extern long int CreateMenu();
extern long int DestroyMenu();
extern long int ChangeMenu();
extern long int CheckMenuItem();
extern long int EnableMenuItem();
extern long int GetSystemMenu();
extern long int GetMenu();
extern long int SetMenu();
extern long int GetSubMenu();
extern long int DrawMenuBar();
extern long int GetMenuString();
extern long int HiliteMenuItem();
extern long int CreateCaret();
extern long int DestroyCaret();
extern long int SetCaretPos();
extern long int HideCaret();
extern long int ShowCaret();
extern long int SetCaretBlinkTime();
extern long int GetCaretBlinkTime();
extern long int ArrangeIconicWindows();
extern long int WinHelp();
extern long int LoadCursor();
extern long int LoadIcon();
extern long int LoadBitmap();
extern long int LoadString();
extern long int LoadAccelerators();
extern long int TranslateAccelerator();
extern long int GetSystemMetrics();
extern long int GetSysColor();
extern long int SetSysColors();
extern long int GetCaretPos();
extern long int QuerySendMessage();
extern long int GrayString();
extern long int SwapMouseButton();
extern long int GetSysModalWindow();
extern long int SetSysModalWindow();
extern long int GetUpdateRect();
extern long int ChildWindowFromPoint();
extern long int InSendMessage();
extern long int IsClipboardFormatAvailable();
extern long int DlgDirSelectComboBox();
extern long int DialogBoxIndirect();
extern long int CreateDialogIndirect();
extern long int ScrollDC();
extern long int GetKeyboardState();
extern long int SetKeyboardState();
extern long int GetWindowTask();
extern long int EnumTaskWindows();
extern long int GetNextDlgGroupItem();
extern long int GetNextDlgTabItem();
extern long int GetTopWindow();
extern long int GetNextWindow();
extern long int SetWindowPos();
extern long int SetParent();
extern long int UnhookWindowsHook();
extern long int UnhookWindowsHookEx();
extern long int CallNextHookEx();
extern long int GetCapture();
extern long int GetUpdateRgn();
extern long int ExcludeUpdateRgn();
extern long int DialogBoxParam();
extern long int DialogBoxIndirectParam();
extern long int CreateDialogParam();
extern long int CreateDialogIndirectParam();
extern long int LoadMenuIndirect();
extern long int GetDialogBaseUnits();
extern long int EqualRect();
extern long int GetCursor();
extern long int GetOpenClipboardWindow();
extern long int GetAsyncKeyState();
extern long int GetMenuState();
extern long int MapWindowPoints();
extern long int BeginDeferWindowPos();
extern long int DeferWindowPos();
extern long int EndDeferWindowPos();
extern long int GetWindow();
extern long int GetMenuItemCount();
extern long int GetMenuItemID();
extern long int ShowOwnedPopups();
extern long int SetMessageQueue();
extern long int ShowScrollBar();
extern long int GlobalAddAtom();
extern long int GlobalDeleteAtom();
extern long int GlobalFindAtom();
extern long int GlobalGetAtomName();
extern long int IsZoomed();
extern long int GetDlgCtrlID();
extern long int SelectPalette();
extern long int RealizePalette();
extern long int GetDesktopWindow();
extern long int DefDlgProc();
extern long int GetClipCursor();
extern long int ScrollWindowEx();
extern long int IsMenu();
extern long int GetPriorityClipboardFormat();
extern long int UnregisterClass();
extern long int GetClassInfo();
extern long int CreateCursor();
extern long int CreateIcon();
extern long int InsertMenu();
extern long int AppendMenu();
extern long int RemoveMenu();
extern long int DeleteMenu();
extern long int ModifyMenu();
extern long int CreatePopupMenu();
extern long int TrackPopupMenu();
extern long int GetMenuCheckMarkDimensions();
extern long int SetMenuItemBitmaps();
extern long int wsprintf();
extern long int wvsprintf();
extern long int lstrcmpi();
extern long int IsCharAlpha();
extern long int IsCharAlphaNumeric();
extern long int IsCharUpper();
extern long int IsCharLower();
extern long int AnsiUpperBuff();
extern long int AnsiLowerBuff();
extern long int DefFrameProc();
extern long int DefMDIChildProc();
extern long int TranslateMDISysAccel();
extern long int CreateWindowEx();
extern long int AdjustWindowRectEx();
extern long int MapDialogRect();
extern long int DrawFocusRect();
extern long int TabbedTextOut();
extern long int GetTabbedTextExtent();
extern long int CascadeChildWindows();
extern long int TileChildWindows();
extern long int GetQueueStatus();
extern long int GetInputState();
extern long int GetLastActivePopup();
extern long int EnableScrollBar();
extern long int SystemParametersInfo();
extern long int DestroyIcon();
extern long int DestroyCursor();
extern long int OpenComm();
extern long int SetCommState();
extern long int GetCommState();
extern long int GetCommError();
extern long int ReadComm();
extern long int WriteComm();
extern long int TransmitCommChar();
extern long int CloseComm();
extern long int SetCommEventMask();
extern long int GetCommEventMask();
extern long int SetCommBreak();
extern long int ClearCommBreak();
extern long int UngetCommChar();
extern long int BuildCommDCB();
extern long int EscapeCommFunction();
extern long int EnableCommNotification();
extern long int FlushComm();
extern long int WNetGetConnection();
extern long int WNetGetCaps();
extern long int WNetGetUser();
extern long int SetWindowPlacement();
extern long int GetWindowPlacement();
extern long int DefHookProc();
extern long int CallMsgFilter();
extern long int RedrawWindow();
extern long int GetNextDriver();
extern long int CalcChildScroll();

 /* Entry Table USER: */

static ENTRYTAB entry_tab_USER[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "USER", 0, 0, 0 },
	/* 001 */	{ "MESSAGEBOX", 0x0008, 0x0008, MessageBox },
	/* 002 */	{ "OLDEXITWINDOWS", 0x0008, 0x0010, OldExitWindows },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "INITAPP", 0x0008, 0x0028, InitApp },
	/* 006 */	{ "POSTQUITMESSAGE", 0x0008, 0x0030, PostQuitMessage },
	/* 007 */	{ "EXITWINDOWS", 0x0008, 0x0038, ExitWindows },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "SETTIMER", 0x0008, 0x0050, SetTimer },
	/* 00b */	{ "BEAR11", 0x0008, 0x0058, SetSystemTimer },
	/* 00c */	{ "KILLTIMER", 0x0008, 0x0060, KillTimer },
	/* 00d */	{ "GETTICKCOUNT", 0x0008, 0x0068, GetTickCount },
	/* 00e */	{ "GETTIMERRESOLUTION", 0x0008, 0x0070, 0 },
	/* 00f */	{ "GETCURRENTTIME", 0x0008, 0x0078, GetCurrentTime },
	/* 010 */	{ "CLIPCURSOR", 0x0008, 0x0080, ClipCursor },
	/* 011 */	{ "GETCURSORPOS", 0x0008, 0x0088, GetCursorPos },
	/* 012 */	{ "SETCAPTURE", 0x0008, 0x0090, SetCapture },
	/* 013 */	{ "RELEASECAPTURE", 0x0008, 0x0098, ReleaseCapture },
	/* 014 */	{ "SETDOUBLECLICKTIME", 0x0008, 0x00a0, SetDoubleClickTime },
	/* 015 */	{ "GETDOUBLECLICKTIME", 0x0008, 0x00a8, GetDoubleClickTime },
	/* 016 */	{ "SETFOCUS", 0x0008, 0x00b0, SetFocus },
	/* 017 */	{ "GETFOCUS", 0x0008, 0x00b8, GetFocus },
	/* 018 */	{ "REMOVEPROP", 0x0008, 0x00c0, RemoveProp },
	/* 019 */	{ "GETPROP", 0x0008, 0x00c8, GetProp },
	/* 01a */	{ "SETPROP", 0x0008, 0x00d0, SetProp },
	/* 01b */	{ "ENUMPROPS", 0x0008, 0x00d8, EnumProps },
	/* 01c */	{ "CLIENTTOSCREEN", 0x0008, 0x00e0, ClientToScreen },
	/* 01d */	{ "SCREENTOCLIENT", 0x0008, 0x00e8, ScreenToClient },
	/* 01e */	{ "WINDOWFROMPOINT", 0x0008, 0x00f0, WindowFromPoint },
	/* 01f */	{ "ISICONIC", 0x0008, 0x00f8, IsIconic },
	/* 020 */	{ "GETWINDOWRECT", 0x0008, 0x0100, GetWindowRect },
	/* 021 */	{ "GETCLIENTRECT", 0x0008, 0x0108, GetClientRect },
	/* 022 */	{ "ENABLEWINDOW", 0x0008, 0x0110, EnableWindow },
	/* 023 */	{ "ISWINDOWENABLED", 0x0008, 0x0118, IsWindowEnabled },
	/* 024 */	{ "GETWINDOWTEXT", 0x0008, 0x0120, GetWindowText },
	/* 025 */	{ "SETWINDOWTEXT", 0x0008, 0x0128, SetWindowText },
	/* 026 */	{ "GETWINDOWTEXTLENGTH", 0x0008, 0x0130, GetWindowTextLength },
	/* 027 */	{ "BEGINPAINT", 0x0008, 0x0138, BeginPaint },
	/* 028 */	{ "ENDPAINT", 0x0008, 0x0140, EndPaint },
	/* 029 */	{ "CREATEWINDOW", 0x0008, 0x0148, CreateWindow },
	/* 02a */	{ "SHOWWINDOW", 0x0008, 0x0150, ShowWindow },
	/* 02b */	{ "CLOSEWINDOW", 0x0008, 0x0158, 0 },
	/* 02c */	{ "OPENICON", 0x0008, 0x0160, OpenIcon },
	/* 02d */	{ "BRINGWINDOWTOTOP", 0x0008, 0x0168, BringWindowToTop },
	/* 02e */	{ "GETPARENT", 0x0008, 0x0170, GetParent },
	/* 02f */	{ "ISWINDOW", 0x0008, 0x0178, IsWindow },
	/* 030 */	{ "ISCHILD", 0x0008, 0x0180, IsChild },
	/* 031 */	{ "ISWINDOWVISIBLE", 0x0008, 0x0188, IsWindowVisible },
	/* 032 */	{ "FINDWINDOW", 0x0008, 0x0190, FindWindow },
	/* 033 */	{ "", 0, 0, 0 },
	/* 034 */	{ "ANYPOPUP", 0x0008, 0x01a0, AnyPopup },
	/* 035 */	{ "DESTROYWINDOW", 0x0008, 0x01a8, DestroyWindow },
	/* 036 */	{ "ENUMWINDOWS", 0x0008, 0x01b0, EnumWindows },
	/* 037 */	{ "ENUMCHILDWINDOWS", 0x0008, 0x01b8, EnumChildWindows },
	/* 038 */	{ "MOVEWINDOW", 0x0008, 0x01c0, MoveWindow },
	/* 039 */	{ "REGISTERCLASS", 0x0008, 0x01c8, RegisterClass },
	/* 03a */	{ "GETCLASSNAME", 0x0008, 0x01d0, GetClassName },
	/* 03b */	{ "SETACTIVEWINDOW", 0x0008, 0x01d8, SetActiveWindow },
	/* 03c */	{ "GETACTIVEWINDOW", 0x0008, 0x01e0, GetActiveWindow },
	/* 03d */	{ "SCROLLWINDOW", 0x0008, 0x01e8, ScrollWindow },
	/* 03e */	{ "SETSCROLLPOS", 0x0008, 0x01f0, SetScrollPos },
	/* 03f */	{ "GETSCROLLPOS", 0x0008, 0x01f8, GetScrollPos },
	/* 040 */	{ "SETSCROLLRANGE", 0x0008, 0x0200, SetScrollRange },
	/* 041 */	{ "GETSCROLLRANGE", 0x0008, 0x0208, GetScrollRange },
	/* 042 */	{ "GETDC", 0x0008, 0x0210, GetDC },
	/* 043 */	{ "GETWINDOWDC", 0x0008, 0x0218, GetWindowDC },
	/* 044 */	{ "RELEASEDC", 0x0008, 0x0220, ReleaseDC },
	/* 045 */	{ "SETCURSOR", 0x0008, 0x0228, SetCursor },
	/* 046 */	{ "SETCURSORPOS", 0x0008, 0x0230, SetCursorPos },
	/* 047 */	{ "SHOWCURSOR", 0x0008, 0x0238, ShowCursor },
	/* 048 */	{ "SETRECT", 0x0008, 0x0240, SetRect },
	/* 049 */	{ "SETRECTEMPTY", 0x0008, 0x0248, SetRectEmpty },
	/* 04a */	{ "COPYRECT", 0x0008, 0x0250, CopyRect },
	/* 04b */	{ "ISRECTEMPTY", 0x0008, 0x0258, IsRectEmpty },
	/* 04c */	{ "PTINRECT", 0x0008, 0x0260, PtInRect },
	/* 04d */	{ "OFFSETRECT", 0x0008, 0x0268, OffsetRect },
	/* 04e */	{ "INFLATERECT", 0x0008, 0x0270, InflateRect },
	/* 04f */	{ "INTERSECTRECT", 0x0008, 0x0278, IntersectRect },
	/* 050 */	{ "UNIONRECT", 0x0008, 0x0280, UnionRect },
	/* 051 */	{ "FILLRECT", 0x0008, 0x0288, FillRect },
	/* 052 */	{ "INVERTRECT", 0x0008, 0x0290, InvertRect },
	/* 053 */	{ "FRAMERECT", 0x0008, 0x0298, FrameRect },
	/* 054 */	{ "DRAWICON", 0x0008, 0x02a0, DrawIcon },
	/* 055 */	{ "DRAWTEXT", 0x0008, 0x02a8, DrawText },
	/* 056 */	{ "BEAR86", 0, 0, 0 },
	/* 057 */	{ "DIALOGBOX", 0x0008, 0x02b8, DialogBox },
	/* 058 */	{ "ENDDIALOG", 0x0008, 0x02c0, EndDialog },
	/* 059 */	{ "CREATEDIALOG", 0x0008, 0x02c8, CreateDialog },
	/* 05a */	{ "ISDIALOGMESSAGE", 0x0008, 0x02d0, IsDialogMessage },
	/* 05b */	{ "GETDLGITEM", 0x0008, 0x02d8, GetDlgItem },
	/* 05c */	{ "SETDLGITEMTEXT", 0x0008, 0x02e0, SetDlgItemText },
	/* 05d */	{ "GETDLGITEMTEXT", 0x0008, 0x02e8, GetDlgItemText },
	/* 05e */	{ "SETDLGITEMINT", 0x0008, 0x02f0, SetDlgItemInt },
	/* 05f */	{ "GETDLGITEMINT", 0x0008, 0x02f8, GetDlgItemInt },
	/* 060 */	{ "CHECKRADIOBUTTON", 0x0008, 0x0300, CheckRadioButton },
	/* 061 */	{ "CHECKDLGBUTTON", 0x0008, 0x0308, CheckDlgButton },
	/* 062 */	{ "ISDLGBUTTONCHECKED", 0x0008, 0x0310, IsDlgButtonChecked },
	/* 063 */	{ "DLGDIRSELECT", 0x0008, 0x0318, DlgDirSelect },
	/* 064 */	{ "DLGDIRLIST", 0x0008, 0x0320, DlgDirList },
	/* 065 */	{ "SENDDLGITEMMESSAGE", 0x0008, 0x0328, SendDlgItemMessage },
	/* 066 */	{ "ADJUSTWINDOWRECT", 0x0008, 0x0330, AdjustWindowRect },
	/* 067 */	{ "MAPDIALOGRECT", 0x0008, 0x0338, MapDialogRect },
	/* 068 */	{ "MESSAGEBEEP", 0x0008, 0x0340, MessageBeep },
	/* 069 */	{ "FLASHWINDOW", 0x0008, 0x0348, FlashWindow },
	/* 06a */	{ "GETKEYSTATE", 0x0008, 0x0350, GetKeyState },
	/* 06b */	{ "DEFWINDOWPROC", 0x0008, 0x0358, DefWindowProc },
	/* 06c */	{ "GETMESSAGE", 0x0008, 0x0360, GetMessage },
	/* 06d */	{ "PEEKMESSAGE", 0x0008, 0x0368, PeekMessage },
	/* 06e */	{ "POSTMESSAGE", 0x0008, 0x0370, PostMessage },
	/* 06f */	{ "SENDMESSAGE", 0x0008, 0x0378, SendMessage },
	/* 070 */	{ "WAITMESSAGE", 0x0008, 0x0380, WaitMessage },
	/* 071 */	{ "TRANSLATEMESSAGE", 0x0008, 0x0388, TranslateMessage },
	/* 072 */	{ "DISPATCHMESSAGE", 0x0008, 0x0390, DispatchMessage },
	/* 073 */	{ "REPLYMESSAGE", 0x0008, 0x0398, ReplyMessage },
	/* 074 */	{ "POSTAPPMESSAGE", 0x0008, 0x03a0, PostAppMessage },
	/* 075 */	{ "", 0, 0, 0 },
	/* 076 */	{ "REGISTERWINDOWMESSAGE", 0x0008, 0x03b0, RegisterWindowMessage },
	/* 077 */	{ "GETMESSAGEPOS", 0x0008, 0x03b8, GetMessagePos },
	/* 078 */	{ "GETMESSAGETIME", 0x0008, 0x03c0, GetMessageTime },
	/* 079 */	{ "SETWINDOWSHOOK", 0x0008, 0x03c8, SetWindowsHook },
	/* 07a */	{ "CALLWINDOWPROC", 0x0008, 0x03d0, CallWindowProc },
	/* 07b */	{ "CALLMSGFILTER", 0x0008, 0x03d8, CallMsgFilter },
	/* 07c */	{ "UPDATEWINDOW", 0x0008, 0x03e0, UpdateWindow },
	/* 07d */	{ "INVALIDATERECT", 0x0008, 0x03e8, InvalidateRect },
	/* 07e */	{ "INVALIDATERGN", 0x0008, 0x03f0, InvalidateRgn },
	/* 07f */	{ "VALIDATERECT", 0x0008, 0x03f8, ValidateRect },
	/* 080 */	{ "VALIDATERGN", 0x0008, 0x0400, ValidateRgn },
	/* 081 */	{ "GETCLASSWORD", 0x0008, 0x0408, GetClassWord },
	/* 082 */	{ "SETCLASSWORD", 0x0008, 0x0410, SetClassWord },
	/* 083 */	{ "GETCLASSLONG", 0x0008, 0x0418, GetClassLong },
	/* 084 */	{ "SETCLASSLONG", 0x0008, 0x0420, SetClassLong },
	/* 085 */	{ "GETWINDOWWORD", 0x0008, 0x0428, GetWindowWord },
	/* 086 */	{ "SETWINDOWWORD", 0x0008, 0x0430, SetWindowWord },
	/* 087 */	{ "GETWINDOWLONG", 0x0008, 0x0438, GetWindowLong },
	/* 088 */	{ "SETWINDOWLONG", 0x0008, 0x0440, SetWindowLong },
	/* 089 */	{ "OPENCLIPBOARD", 0x0008, 0x0448, OpenClipboard },
	/* 08a */	{ "CLOSECLIPBOARD", 0x0008, 0x0450, CloseClipboard },
	/* 08b */	{ "EMPTYCLIPBOARD", 0x0008, 0x0458, EmptyClipboard },
	/* 08c */	{ "GETCLIPBOARDOWNER", 0x0008, 0x0460, GetClipboardOwner },
	/* 08d */	{ "SETCLIPBOARDDATA", 0x0008, 0x0468, SetClipboardData },
	/* 08e */	{ "GETCLIPBOARDDATA", 0x0008, 0x0470, GetClipboardData },
	/* 08f */	{ "COUNTCLIPBOARDFORMATS", 0x0008, 0x0478, CountClipboardFormats },
	/* 090 */	{ "ENUMCLIPBOARDFORMATS", 0x0008, 0x0480, EnumClipboardFormats },
	/* 091 */	{ "REGISTERCLIPBOARDFORMAT", 0x0008, 0x0488, RegisterClipboardFormat },
	/* 092 */	{ "GETCLIPBOARDFORMATNAME", 0x0008, 0x0490, GetClipboardFormatName },
	/* 093 */	{ "SETCLIPBOARDVIEWER", 0x0008, 0x0498, SetClipboardViewer },
	/* 094 */	{ "GETCLIPBOARDVIEWER", 0x0008, 0x04a0, GetClipboardViewer },
	/* 095 */	{ "CHANGECLIPBOARDCHAIN", 0x0008, 0x04a8, ChangeClipboardChain },
	/* 096 */	{ "LOADMENU", 0x0008, 0x04b0, LoadMenu },
	/* 097 */	{ "CREATEMENU", 0x0008, 0x04b8, CreateMenu },
	/* 098 */	{ "DESTROYMENU", 0x0008, 0x04c0, DestroyMenu },
	/* 099 */	{ "CHANGEMENU", 0x0008, 0x04c8, ChangeMenu },
	/* 09a */	{ "CHECKMENUITEM", 0x0008, 0x04d0, CheckMenuItem },
	/* 09b */	{ "ENABLEMENUITEM", 0x0008, 0x04d8, EnableMenuItem },
	/* 09c */	{ "GETSYSTEMMENU", 0x0008, 0x04e0, GetSystemMenu },
	/* 09d */	{ "GETMENU", 0x0008, 0x04e8, GetMenu },
	/* 09e */	{ "SETMENU", 0x0008, 0x04f0, SetMenu },
	/* 09f */	{ "GETSUBMENU", 0x0008, 0x04f8, GetSubMenu },
	/* 0a0 */	{ "DRAWMENUBAR", 0x0008, 0x0500, DrawMenuBar },
	/* 0a1 */	{ "GETMENUSTRING", 0x0008, 0x0508, GetMenuString },
	/* 0a2 */	{ "HILITEMENUITEM", 0x0008, 0x0510, HiliteMenuItem },
	/* 0a3 */	{ "CREATECARET", 0x0008, 0x0518, CreateCaret },
	/* 0a4 */	{ "DESTROYCARET", 0x0008, 0x0520, DestroyCaret },
	/* 0a5 */	{ "SETCARETPOS", 0x0008, 0x0528, SetCaretPos },
	/* 0a6 */	{ "HIDECARET", 0x0008, 0x0530, HideCaret },
	/* 0a7 */	{ "SHOWCARET", 0x0008, 0x0538, ShowCaret },
	/* 0a8 */	{ "SETCARETBLINKTIME", 0x0008, 0x0540, SetCaretBlinkTime },
	/* 0a9 */	{ "GETCARETBLINKTIME", 0x0008, 0x0548, GetCaretBlinkTime },
	/* 0aa */	{ "ARRANGEICONICWINDOWS", 0x0008, 0x0550, ArrangeIconicWindows },
	/* 0ab */	{ "WINHELP", 0x0008, 0x0558, WinHelp },
	/* 0ac */	{ "", 0, 0, 0 },
	/* 0ad */	{ "LOADCURSOR", 0x0008, 0x0568, LoadCursor },
	/* 0ae */	{ "LOADICON", 0x0008, 0x0570, LoadIcon },
	/* 0af */	{ "LOADBITMAP", 0x0008, 0x0578, LoadBitmap },
	/* 0b0 */	{ "LOADSTRING", 0x0008, 0x0580, LoadString },
	/* 0b1 */	{ "LOADACCELERATORS", 0x0008, 0x0588, LoadAccelerators },
	/* 0b2 */	{ "TRANSLATEACCELERATOR", 0x0008, 0x0590, TranslateAccelerator },
	/* 0b3 */	{ "GETSYSTEMMETRICS", 0x0008, 0x0598, GetSystemMetrics },
	/* 0b4 */	{ "GETSYSCOLOR", 0x0008, 0x05a0, GetSysColor },
	/* 0b5 */	{ "SETSYSCOLORS", 0x0008, 0x05a8, SetSysColors },
	/* 0b6 */	{ "BEAR182", 0x0008, 0x05b0, UserKillSystemTimer },
	/* 0b7 */	{ "GETCARETPOS", 0x0008, 0x05b8, GetCaretPos },
	/* 0b8 */	{ "QUERYSENDMESSAGE", 0x0008, 0x05c0, QuerySendMessage },
	/* 0b9 */	{ "GRAYSTRING", 0x0008, 0x05c8, GrayString },
	/* 0ba */	{ "SWAPMOUSEBUTTON", 0x0008, 0x05d0, SwapMouseButton },
	/* 0bb */	{ "", 0, 0, 0 },
	/* 0bc */	{ "SETSYSMODALWINDOW", 0x0008, 0x05e0, SetSysModalWindow },
	/* 0bd */	{ "GETSYSMODALWINDOW", 0x0008, 0x05e8, GetSysModalWindow },
	/* 0be */	{ "GETUPDATERECT", 0x0008, 0x05f0, GetUpdateRect },
	/* 0bf */	{ "CHILDWINDOWFROMPOINT", 0x0008, 0x05f8, ChildWindowFromPoint },
	/* 0c0 */	{ "INSENDMESSAGE", 0x0008, 0x0600, InSendMessage },
	/* 0c1 */	{ "ISCLIPBOARDFORMATAVAILABLE", 0x0008, 0x0608, IsClipboardFormatAvailable },
	/* 0c2 */	{ "DLGDIRSELECTCOMBOBOX", 0x0008, 0x0610, DlgDirSelectComboBox },
	/* 0c3 */	{ "DLGDIRLISTCOMBOBOX", 0x0008, 0x0618, DlgDirListComboBox },
	/* 0c4 */	{ "TABBEDTEXTOUT", 0x0008, 0x0620, TabbedTextOut },
	/* 0c5 */	{ "GETTABBEDTEXTEXTENT", 0x0008, 0x0628, GetTabbedTextExtent },
	/* 0c6 */	{ "CASCADECHILDWINDOWS", 0x0008, 0x630, CascadeChildWindows },
	/* 0c7 */	{ "TILECHILDWINDOWS", 0x0008, 0x638, TileChildWindows },
	/* 0c8 */	{ "OPENCOMM", 0x0008, 0x0640, OpenComm },
	/* 0c9 */	{ "SETCOMMSTATE", 0x0008, 0x0648, SetCommState },
	/* 0ca */	{ "GETCOMMSTATE", 0x0008, 0x0650, GetCommState },
	/* 0cb */	{ "GETCOMMERROR", 0x0008, 0x0658, GetCommError },
	/* 0cc */	{ "READCOMM", 0x0008, 0x0660, ReadComm },
	/* 0cd */	{ "WRITECOMM", 0x0008, 0x0668, WriteComm },
	/* 0ce */	{ "TRANSMITCOMMCHAR", 0x0008, 0x0670, TransmitCommChar },
	/* 0cf */	{ "CLOSECOMM", 0x0008, 0x0678, CloseComm },
	/* 0d0 */	{ "SETCOMMEVENTMASK", 0x0008, 0x0680, SetCommEventMask },
	/* 0d1 */	{ "GETCOMMEVENTMASK", 0x0008, 0x0688, GetCommEventMask },
	/* 0d2 */	{ "SETCOMMBREAK", 0x0008, 0x0690, SetCommBreak },
	/* 0d3 */	{ "CLEARCOMMBREAK", 0x0008, 0x0698, ClearCommBreak },
	/* 0d4 */	{ "UNGETCOMMCHAR", 0x0008, 0x06a0, UngetCommChar },
	/* 0d5 */	{ "BUILDCOMMDCB", 0x0008, 0x06a8, BuildCommDCB },
	/* 0d6 */	{ "ESCAPECOMMFUNCTION", 0x0008, 0x06b0, EscapeCommFunction },
	/* 0d7 */	{ "FLUSHCOMM", 0x0008, 0x06b8, FlushComm },
	/* 0d8 */	{ "", 0, 0, 0 },
	/* 0d9 */	{ "", 0, 0, 0 },
	/* 0da */	{ "DIALOGBOXINDIRECT", 0x0008, 0x06d0, DialogBoxIndirect },
	/* 0db */	{ "CREATEDIALOGINDIRECT", 0x0008, 0x06d8, CreateDialogIndirect },
	/* 0dc */	{ "LOADMENUINDIRECT", 0x0008, 0x06e0, LoadMenuIndirect },
	/* 0dd */	{ "SCROLLDC", 0x0008, 0x06e8, ScrollDC },
	/* 0de */	{ "GETKEYBOARDSTATE", 0x0008, 0x06f0, GetKeyboardState },
	/* 0df */	{ "SETKEYBOARDSTATE", 0x0008, 0x06f8, SetKeyboardState },
	/* 0e0 */	{ "GETWINDOWTASK", 0x0008, 0x0700, GetWindowTask },
	/* 0e1 */	{ "ENUMTASKWINDOWS", 0x0008, 0x0708, EnumTaskWindows },
	/* 0e2 */	{ "LOCKINPUT", 0x0008, 0x0710, 0 },
	/* 0e3 */	{ "GETNEXTDLGGROUPITEM", 0x0008, 0x0718, GetNextDlgGroupItem },
	/* 0e4 */	{ "GETNEXTDLGTABITEM", 0x0008, 0x0720, GetNextDlgTabItem },
	/* 0e5 */	{ "GETTOPWINDOW", 0x0008, 0x0728, GetTopWindow },
	/* 0e6 */	{ "GETNEXTWINDOW", 0x0008, 0x0730, GetNextWindow },
	/* 0e7 */	{ "GETSYSTEMDEBUGSTATE", 0x0008, 0x0738, 0 },
	/* 0e8 */	{ "SETWINDOWPOS", 0x0008, 0x0740, SetWindowPos },
	/* 0e9 */	{ "SETPARENT", 0x0008, 0x0748, SetParent },
	/* 0ea */	{ "UNHOOKWINDOWSHOOK", 0x0008, 0x0750, UnhookWindowsHook },
	/* 0eb */	{ "DEFHOOKPROC", 0x0008, 0x0758, DefHookProc },
	/* 0ec */	{ "GETCAPTURE", 0x0008, 0x0760, GetCapture },
	/* 0ed */	{ "GETUPDATERGN", 0x0008, 0x0768, GetUpdateRgn },
	/* 0ee */	{ "EXCLUDEUPDATERGN", 0x0008, 0x0770, ExcludeUpdateRgn },
	/* 0ef */	{ "DIALOGBOXPARAM", 0x0008, 0x0778, DialogBoxParam },
	/* 0f0 */	{ "DIALOGBOXINDIRECTPARAM", 0x0008, 0x0780, DialogBoxIndirectParam },
	/* 0f1 */	{ "CREATEDIALOGPARAM", 0x0008, 0x0788, CreateDialogParam },
	/* 0f2 */	{ "CREATEDIALOGINDIRECTPARAM", 0x0008, 0x0790, CreateDialogIndirectParam },
	/* 0f3 */	{ "GETDIALOGBASEUNITS", 0x0008, 0x0798, GetDialogBaseUnits },
	/* 0f4 */	{ "EQUALRECT", 0x0008, 0x07a0, EqualRect },
	/* 0f5 */	{ "ENABLECOMMNOTIFICATION", 0x0008, 0x07a8, EnableCommNotification },
	/* 0f6 */	{ "EXITWINDOWSEXEC", 0x0008, 0x07b0, 0 },
	/* 0f7 */	{ "GETCURSOR", 0x0008, 0x07b8, GetCursor },
	/* 0f8 */	{ "GETOPENCLIPBOARDWINDOW", 0x0008, 0x07c0, GetOpenClipboardWindow },
	/* 0f9 */	{ "GETASYNCKEYSTATE", 0x0008, 0x07c8, GetAsyncKeyState },
	/* 0fa */	{ "GETMENUSTATE", 0x0008, 0x07d0, GetMenuState },
	/* 0fb */	{ "SENDDRIVERMESSAGE", 0x0008, 0x07d8, 0 },
	/* 0fc */	{ "OPENDRIVER", 0x0008, 0x07e0, 0 },
	/* 0fd */	{ "CLOSEDRIVER", 0x0008, 0x07e8, 0 },
	/* 0fe */	{ "GETDRIVERMODULEHANDLE", 0x0008, 0x07f0, 0 },
	/* 0ff */	{ "DEFDRIVERPROC", 0x0008, 0x07f8, 0 },
	/* 100 */	{ "GETDRIVERINFO", 0x0008, 0x0800, 0 },
	/* 101 */	{ "GETNEXTDRIVER", 0x0008, 0x0808, GetNextDriver },
	/* 102 */	{ "MAPWINDOWPOINTS", 0x0008, 0x0810, MapWindowPoints },
	/* 103 */	{ "BEGINDEFERWINDOWPOS", 0x0008, 0x0818, BeginDeferWindowPos },
	/* 104 */	{ "DEFERWINDOWPOS", 0x0008, 0x0820, DeferWindowPos },
	/* 105 */	{ "ENDDEFERWINDOWPOS", 0x0008, 0x0828, EndDeferWindowPos },
	/* 106 */	{ "GETWINDOW", 0x0008, 0x0830, GetWindow },
	/* 107 */	{ "GETMENUITEMCOUNT", 0x0008, 0x0838, GetMenuItemCount },
	/* 108 */	{ "GETMENUITEMID", 0x0008, 0x0840, GetMenuItemID },
	/* 109 */	{ "SHOWOWNEDPOPUPS", 0x0008, 0x0848, ShowOwnedPopups },
	/* 10a */	{ "SETMESSAGEQUEUE", 0x0008, 0x0850, SetMessageQueue },
	/* 10b */	{ "SHOWSCROLLBAR", 0x0008, 0x0858, ShowScrollBar },
	/* 10c */	{ "GLOBALADDATOM", 0x0008, 0x0860, GlobalAddAtom },
	/* 10d */	{ "GLOBALDELETEATOM", 0x0008, 0x0868, GlobalDeleteAtom },
	/* 10e */	{ "GLOBALFINDATOM", 0x0008, 0x0870, GlobalFindAtom },
	/* 10f */	{ "GLOBALGETATOMNAME", 0x0008, 0x0878, GlobalGetAtomName },
	/* 110 */	{ "ISZOOMED", 0x0008, 0x0880, IsZoomed },
	/* 111 */	{ "", 0, 0, 0 },
	/* 112 */	{ "", 0, 0, 0 },
	/* 113 */	{ "", 0, 0, 0 },
	/* 114 */	{ "", 0, 0, 0 },
	/* 115 */	{ "GETDLGCTRLID", 0x0008, 0x08a8, GetDlgCtrlID },
	/* 116 */	{ "GETDESKTOPHWND", 0x0008, 0x08b0, GetDesktopWindow },
	/* 117 */	{ "", 0, 0, 0 },
	/* 118 */	{ "", 0, 0, 0 },
	/* 119 */	{ "", 0, 0, 0 },
	/* 11a */	{ "SELECTPALETTE", 0x0008, 0x08d0, SelectPalette },
	/* 11b */	{ "REALIZEPALETTE", 0x0008, 0x08d8, RealizePalette },
	/* 11c */	{ "GETFREESYSTEMRESOURCES", 0x0008, 0x08e0, 0 },
	/* 11d */	{ "", 0, 0, 0 },
	/* 11e */	{ "GETDESKTOPWINDOW", 0x0008, 0x08f0, GetDesktopWindow },
	/* 11f */	{ "GETLASTACTIVEPOPUP", 0x0008, 0x08f8, GetLastActivePopup },
	/* 120 */	{ "GETMESSAGEEXTRAINFO", 0x0008, 0x0900, 0 },
	/* 121 */	{ "", 0, 0, 0 },
	/* 122 */	{ "REDRAWWINDOW", 0x0008, 0x0910, RedrawWindow },
	/* 123 */	{ "SETWINDOWSHOOKEX", 0x0008, 0x0918, SetWindowsHookEx },
	/* 124 */	{ "UNHOOKWINDOWSHOOKEX", 0x0008, 0x0920, UnhookWindowsHookEx },
	/* 125 */	{ "CALLNEXTHOOKEX", 0x0008, 0x0928, CallNextHookEx },
	/* 126 */	{ "LOCKWINDOWUPDATE", 0x0008, 0x0930, 0 },
	/* 127 */	{ "", 0, 0, 0 },
	/* 128 */	{ "", 0, 0, 0 },
	/* 129 */	{ "", 0, 0, 0 },
	/* 12a */	{ "", 0, 0, 0 },
	/* 12b */	{ "", 0, 0, 0 },
	/* 12c */	{ "", 0, 0, 0 },
	/* 12d */	{ "", 0, 0, 0 },
	/* 12e */	{ "", 0, 0, 0 },
	/* 12f */	{ "", 0, 0, 0 },
	/* 130 */	{ "", 0, 0, 0 },
	/* 131 */	{ "", 0, 0, 0 },
	/* 132 */	{ "", 0, 0, 0 },
	/* 133 */	{ "", 0, 0, 0 },
	/* 134 */	{ "DEFDLGPROC", 0x0008, 0x09a0, DefDlgProc },
	/* 135 */	{ "GETCLIPCURSOR", 0x0008, 0x09a8, GetClipCursor },
	/* 136 */	{ "", 0, 0, 0 },
	/* 137 */	{ "", 0, 0, 0 },
	/* 138 */	{ "", 0, 0, 0 },
	/* 139 */	{ "", 0, 0, 0 },
	/* 13a */	{ "", 0, 0, 0 },
	/* 13b */	{ "", 0, 0, 0 },
	/* 13c */	{ "", 0, 0, 0 },
	/* 13d */	{ "", 0, 0, 0 },
	/* 13e */	{ "", 0, 0, 0 },
	/* 13f */	{ "SCROLLWINDOWEX", 0x0008, 0x09f8, ScrollWindowEx },
	/* 140 */	{ "", 0, 0, 0 },
	/* 141 */	{ "", 0, 0, 0 },
	/* 142 */	{ "", 0, 0, 0 },
	/* 143 */	{ "", 0, 0, 0 },
	/* 144 */	{ "", 0, 0, 0 },
	/* 145 */	{ "", 0, 0, 0 },
	/* 146 */	{ "", 0, 0, 0 },
	/* 147 */	{ "", 0, 0, 0 },
	/* 148 */	{ "", 0, 0, 0 },
	/* 149 */	{ "", 0, 0, 0 },
	/* 14a */	{ "", 0, 0, 0 },
	/* 14b */	{ "ENABLEHARDWAREINPUT", 0x0008, 0x0a58, 0 },
	/* 14c */	{ "", 0, 0, 0 },
	/* 14d */	{ "", 0, 0, 0 },
	/* 14e */	{ "GETQUEUESTATUS", 0x0008, 0x0a70, GetQueueStatus },
	/* 14f */	{ "GETINPUTSTATE", 0x0008, 0x0a78, GetInputState },
	/* 150 */	{ "", 0, 0, 0 },
	/* 151 */	{ "", 0, 0, 0 },
	/* 152 */	{ "", 0, 0, 0 },
	/* 153 */	{ "", 0, 0, 0 },
	/* 154 */	{ "", 0, 0, 0 },
	/* 155 */	{ "", 0, 0, 0 },
	/* 156 */	{ "", 0, 0, 0 },
	/* 157 */	{ "", 0, 0, 0 },
	/* 158 */	{ "", 0, 0, 0 },
	/* 159 */	{ "", 0, 0, 0 },
	/* 15a */	{ "", 0, 0, 0 },
	/* 15b */	{ "", 0, 0, 0 },
	/* 15c */	{ "", 0, 0, 0 },
	/* 15d */	{ "", 0, 0, 0 },
	/* 15e */	{ "", 0, 0, 0 },
	/* 15f */	{ "", 0, 0, 0 },
	/* 160 */	{ "", 0, 0, 0 },
	/* 161 */	{ "", 0, 0, 0 },
	/* 162 */	{ "", 0, 0, 0 },
	/* 163 */	{ "", 0, 0, 0 },
	/* 164 */	{ "", 0, 0, 0 },
	/* 165 */	{ "", 0, 0, 0 },
	/* 166 */	{ "ISMENU", 0x0008, 0x0b30, IsMenu },
	/* 167 */	{ "GETDCEX", 0x0008, 0x0b38, GetDCEx },
	/* 168 */	{ "", 0, 0, 0 },
	/* 169 */	{ "", 0, 0, 0 },
	/* 16a */	{ "", 0, 0, 0 },
	/* 16b */	{ "", 0, 0, 0 },
	/* 16c */	{ "", 0, 0, 0 },
	/* 16d */	{ "", 0, 0, 0 },
	/* 16e */	{ "", 0, 0, 0 },
	/* 16f */	{ "", 0, 0, 0 },
	/* 170 */	{ "COPYICON", 0x0008, 0x0b80, 0 },
	/* 171 */	{ "COPYCURSOR", 0x0008, 0x0b88, 0 },
	/* 172 */	{ "GETWINDOWPLACEMENT", 0x0008, 0x0b90, GetWindowPlacement },
	/* 173 */	{ "SETWINDOWPLACEMENT", 0x0008, 0x0b98, SetWindowPlacement },
	/* 174 */	{ "", 0, 0, 0 },
	/* 175 */	{ "SUBTRACTRECT", 0x0008, 0x0ba8, SubtractRect },
	/* 176 */	{ "", 0, 0, 0 },
	/* 177 */	{ "", 0, 0, 0 },
	/* 178 */	{ "", 0, 0, 0 },
	/* 179 */	{ "", 0, 0, 0 },
	/* 17a */	{ "", 0, 0, 0 },
	/* 17b */	{ "", 0, 0, 0 },
	/* 17c */	{ "", 0, 0, 0 },
	/* 17d */	{ "", 0, 0, 0 },
	/* 17e */	{ "", 0, 0, 0 },
	/* 17f */	{ "", 0, 0, 0 },
	/* 180 */	{ "", 0, 0, 0 },
	/* 181 */	{ "", 0, 0, 0 },
	/* 182 */	{ "", 0, 0, 0 },
	/* 183 */	{ "", 0, 0, 0 },
	/* 184 */	{ "", 0, 0, 0 },
	/* 185 */	{ "", 0, 0, 0 },
	/* 186 */	{ "", 0, 0, 0 },
	/* 187 */	{ "", 0, 0, 0 },
	/* 188 */	{ "", 0, 0, 0 },
	/* 189 */	{ "", 0, 0, 0 },
	/* 18a */	{ "", 0, 0, 0 },
	/* 18b */	{ "", 0, 0, 0 },
	/* 18c */	{ "", 0, 0, 0 },
	/* 18d */	{ "", 0, 0, 0 },
	/* 18e */	{ "", 0, 0, 0 },
	/* 18f */	{ "", 0, 0, 0 },
	/* 190 */	{ "", 0, 0, 0 },
	/* 191 */	{ "", 0, 0, 0 },
	/* 192 */	{ "GETPRIORITYCLIPBOARDFORMAT", 0x0008, 0x0c90, GetPriorityClipboardFormat },
	/* 193 */	{ "UNREGISTERCLASS", 0x0008, 0x0c98, UnregisterClass },
	/* 194 */	{ "GETCLASSINFO", 0x0008, 0x0ca0, GetClassInfo },
	/* 195 */	{ "", 0, 0, 0 },
	/* 196 */	{ "CREATECURSOR", 0x0008, 0x0cb0, CreateCursor },
	/* 197 */	{ "CREATEICON", 0x0008, 0x0cb8, CreateIcon },
	/* 198 */	{ "", 0, 0, 0 },
	/* 199 */	{ "", 0, 0, 0 },
	/* 19a */	{ "INSERTMENU", 0x0008, 0x0cd0, InsertMenu },
	/* 19b */	{ "APPENDMENU", 0x0008, 0x0cd8, AppendMenu },
	/* 19c */	{ "REMOVEMENU", 0x0008, 0x0ce0, RemoveMenu },
	/* 19d */	{ "DELETEMENU", 0x0008, 0x0ce8, DeleteMenu },
	/* 19e */	{ "MODIFYMENU", 0x0008, 0x0cf0, ModifyMenu },
	/* 19f */	{ "CREATEPOPUPMENU", 0x0008, 0x0cf8, CreatePopupMenu },
	/* 1a0 */	{ "TRACKPOPUPMENU", 0x0008, 0x0d00, TrackPopupMenu },
	/* 1a1 */	{ "GETMENUCHECKMARKDIMENSIONS", 0x0008, 0x0d08, GetMenuCheckMarkDimensions },
	/* 1a2 */	{ "SETMENUITEMBITMAPS", 0x0008, 0x0d10, SetMenuItemBitmaps },
	/* 1a3 */	{ "", 0, 0, 0 },
	/* 1a4 */	{ "_WSPRINTF", 0x0008, 0x0d20, wsprintf },
	/* 1a5 */	{ "WVSPRINTF", 0x0008, 0x0d28, wvsprintf },
	/* 1a6 */	{ "DLGDIRSELECTEX", 0x0008, 0x0d30, 0 },
	/* 1a7 */	{ "DLGDIRSELECTCOMBOBOXEX", 0x0008, 0x0d38, 0 },
	/* 1a8 */	{ "", 0, 0, 0 },
	/* 1a9 */	{ "", 0, 0, 0 },
	/* 1aa */	{ "", 0, 0, 0 },
	/* 1ab */	{ "", 0, 0, 0 },
	/* 1ac */	{ "", 0, 0, 0 },
	/* 1ad */	{ "", 0, 0, 0 },
	/* 1ae */	{ "LSTRCMP", 0x0008, 0x0d70, lstrcmp },
	/* 1af */	{ "ANSIUPPER", 0x0008, 0x0d78, AnsiUpper },
	/* 1b0 */	{ "ANSILOWER", 0x0008, 0x0d80, AnsiLower },
	/* 1b1 */	{ "ISCHARALPHA", 0x0008, 0x0d88, IsCharAlpha },
	/* 1b2 */	{ "ISCHARALPHANUMERIC", 0x0008, 0x0d90, IsCharAlphaNumeric },
	/* 1b3 */	{ "ISCHARUPPER", 0x0008, 0x0d98, IsCharUpper },
	/* 1b4 */	{ "ISCHARLOWER", 0x0008, 0x0da0, IsCharLower },
	/* 1b5 */	{ "ANSIUPPERBUFF", 0x0008, 0x0da8, AnsiUpperBuff },
	/* 1b6 */	{ "ANSILOWERBUFF", 0x0008, 0x0db0, AnsiLowerBuff },
	/* 1b7 */	{ "", 0, 0, 0 },
	/* 1b8 */	{ "", 0, 0, 0 },
	/* 1b9 */	{ "", 0, 0, 0 },
	/* 1ba */	{ "", 0, 0, 0 },
	/* 1bb */	{ "", 0, 0, 0 },
	/* 1bc */	{ "", 0, 0, 0 },
	/* 1bd */	{ "DEFFRAMEPROC", 0x0008, 0x0de8, DefFrameProc },
	/* 1be */	{ "", 0, 0, 0 },
	/* 1bf */	{ "DEFMDICHILDPROC", 0x0008, 0x0df8, DefMDIChildProc },
	/* 1c0 */	{ "", 0, 0, 0 },
	/* 1c1 */	{ "", 0, 0, 0 },
	/* 1c2 */	{ "", 0, 0, 0 },
	/* 1c3 */	{ "TRANSLATEMDISYSACCEL", 0x0008, 0x0e18, TranslateMDISysAccel },
	/* 1c4 */	{ "CREATEWINDOWEX", 0x0008, 0x0e20, CreateWindowEx },
	/* 1c5 */	{ "", 0, 0, 0 },
	/* 1c6 */	{ "ADJUSTWINDOWRECTEX", 0x0008, 0x0e30, AdjustWindowRectEx },
	/* 1c7 */	{ "", 0, 0, 0 },
	/* 1c8 */	{ "", 0, 0, 0 },
	/* 1c9 */	{ "DESTROYICON", 0x0008, 0x0e48, DestroyIcon },
	/* 1ca */	{ "DESTROYCURSOR", 0x0008, 0x0e50, DestroyCursor },
	/* 1cb */	{ "", 0, 0, 0 },
	/* 1cc */	{ "", 0, 0, 0 },
	/* 1cd */	{ "", 0, 0, 0 },
	/* 1ce */	{ "CALCCHILDSCROLL", 0x0008, 0x0e70, CalcChildScroll },
	/* 1cf */	{ "", 0, 0, 0 },
	/* 1d0 */	{ "", 0, 0, 0 },
	/* 1d1 */	{ "", 0, 0, 0 },
	/* 1d2 */	{ "DRAWFOCUSRECT", 0x0008, 0x0e90, DrawFocusRect },
	/* 1d3 */	{ "", 0, 0, 0 },
	/* 1d4 */	{ "", 0, 0, 0 },
	/* 1d5 */	{ "", 0, 0, 0 },
	/* 1d6 */	{ "", 0, 0, 0 },
	/* 1d7 */	{ "LSTRCMPI", 0x0008, 0x0eb8, lstrcmpi },
	/* 1d8 */	{ "ANSINEXT", 0x0008, 0x0ec0, AnsiNext },
	/* 1d9 */	{ "ANSIPREV", 0x0008, 0x0ec8, AnsiPrev },
	/* 1da */	{ "", 0, 0, 0 },
	/* 1db */	{ "", 0, 0, 0 },
	/* 1dc */	{ "", 0, 0, 0 },
	/* 1dd */	{ "", 0, 0, 0 },
	/* 1de */	{ "", 0, 0, 0 },
	/* 1df */	{ "", 0, 0, 0 },
	/* 1e0 */	{ "", 0, 0, 0 },
	/* 1e1 */	{ "", 0, 0, 0 },
	/* 1e2 */	{ "ENABLESCROLLBAR", 0x0008, 0x0f10, EnableScrollBar },
	/* 1e3 */	{ "SYSTEMPARAMETERSINFO", 0x0008, 0x0f18, SystemParametersInfo },
	/* 1e4 */	{ "", 0, 0, 0 },
	/* 1e5 */	{ "", 0, 0, 0 },
	/* 1e6 */	{ "", 0, 0, 0 },
	/* 1e7 */	{ "", 0, 0, 0 },
	/* 1e8 */	{ "", 0, 0, 0 },
	/* 1e9 */	{ "", 0, 0, 0 },
	/* 1ea */	{ "", 0, 0, 0 },
	/* 1eb */	{ "", 0, 0, 0 },
	/* 1ec */	{ "", 0, 0, 0 },
	/* 1ed */	{ "", 0, 0, 0 },
	/* 1ee */	{ "", 0, 0, 0 },
	/* 1ef */	{ "", 0, 0, 0 },
	/* 1f0 */	{ "", 0, 0, 0 },
	/* 1f1 */	{ "", 0, 0, 0 },
	/* 1f2 */	{ "", 0, 0, 0 },
	/* 1f3 */	{ "WNETERRORTEXT", 0x0008, 0x0f98, 0 },
	/* 1f4 */	{ "", 0, 0, 0 },
	/* 1f5 */	{ "WNETOPENJOB", 0x0008, 0x0fa8, 0 },
	/* 1f6 */	{ "WNETCLOSEJOB", 0x0008, 0x0fb0, 0 },
	/* 1f7 */	{ "WNETABORTJOB", 0x0008, 0x0fb8, 0 },
	/* 1f8 */	{ "WNETHOLDJOB", 0x0008, 0x0fc0, 0 },
	/* 1f9 */	{ "WNETRELEASEJOB", 0x0008, 0x0fc8, 0 },
	/* 1fa */	{ "WNETCANCELJOB", 0x0008, 0x0fd0, 0 },
	/* 1fb */	{ "WNETSETJOBCOPIES", 0x0008, 0x0fd8, 0 },
	/* 1fc */	{ "WNETWATCHQUEUE", 0x0008, 0x0fe0, 0 },
	/* 1fd */	{ "WNETUNWATCHQUEUE", 0x0008, 0x0fe8, 0 },
	/* 1fe */	{ "WNETLOCKQUEUEDATA", 0x0008, 0x0ff0, 0 },
	/* 1ff */	{ "WNETUNLOCKQUEUEDATA", 0x0008, 0x0ff8, 0 },
	/* 200 */	{ "WNETGETCONNECTION", 0x0008, 0x1000, WNetGetConnection },
	/* 201 */	{ "WNETGETCAPS", 0x0008, 0x1008, WNetGetCaps },
	/* 202 */	{ "WNETDEVICEMODE", 0x0008, 0x1010, 0 },
	/* 203 */	{ "WNETBROWSEDIALOG", 0x0008, 0x1018, 0 },
	/* 204 */	{ "WNETGETUSER", 0x0008, 0x1020, WNetGetUser },
	/* 205 */	{ "WNETADDCONNECTION", 0x0008, 0x1028, 0 },
	/* 206 */	{ "WNETCANCELCONNECTION", 0x0008, 0x1030, 0 },
	/* 207 */	{ "WNETGETERROR", 0x0008, 0x1038, 0 },
	/* 208 */	{ "WNETGETERRORTEXT", 0x0008, 0x1040, 0 },
	/* 209 */	{ "WNETENABLE", 0x0008, 0x1048, 0 },
	/* 20a */	{ "WNETDISABLE", 0x0008, 0x1050, 0 },
	/* 20b */	{ "WNETRESTORECONNECTION", 0x0008, 0x1058, 0 },
	/* 20c */	{ "WNETWRITEJOB", 0x0008, 0x1060, 0 },
	/* 20d */	{ "WNETCONNECTDIALOG", 0x0008, 0x1068, 0 },
	/* 20e */	{ "WNETDISCONNECTDIALOG", 0x0008, 0x1070, 0 },
	/* 20f */	{ "WNETCONNECTIONDIALOG", 0x0008, 0x1078, 0 },
	/* 210 */	{ "WNETVIEWQUEUEDIALOG", 0x0008, 0x1080, 0 },
	/* 211 */	{ "WNETPROPERTYDIALOG", 0x0008, 0x1088, 0 },
	/* 212 */	{ "WNETGETDIRECTORYTYPE", 0x0008, 0x1090, 0 },
	/* 213 */	{ "WNETDIRECTORYNOTIFY", 0x0008, 0x1098, 0 },
	/* 214 */	{ "WNETGETPROPERTYTEXT", 0x0008, 0x10a0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "", 0, 0, 0 },
	/* 00b */	{ "", 0, 0, 0 },
	/* 00c */	{ "", 0, 0, 0 },
	/* 00d */	{ "", 0, 0, 0 },
	/* 00e */	{ "", 0, 0, 0 },
	/* 00f */	{ "", 0, 0, 0 },
	/* 010 */	{ "", 0, 0, 0 },
	/* 011 */	{ "", 0, 0, 0 },
	/* 012 */	{ "", 0, 0, 0 },
	/* 013 */	{ "", 0, 0, 0 },
	/* 014 */	{ "", 0, 0, 0 },
	/* 015 */	{ "", 0, 0, 0 },
	/* 016 */	{ "", 0, 0, 0 },
	/* 017 */	{ "", 0, 0, 0 },
	/* 018 */	{ "", 0, 0, 0 },
	/* 019 */	{ "", 0, 0, 0 },
	/* 01a */	{ "", 0, 0, 0 },
	/* 01b */	{ "", 0, 0, 0 },
	/* 01c */	{ "", 0, 0, 0 },
	/* 01d */	{ "", 0, 0, 0 },
	/* 01e */	{ "", 0, 0, 0 },
	/* 01f */	{ "", 0, 0, 0 },
	/* 020 */	{ "", 0, 0, 0 },
	/* 021 */	{ "", 0, 0, 0 },
	/* 022 */	{ "", 0, 0, 0 },
	/* 023 */	{ "", 0, 0, 0 },
	/* 024 */	{ "", 0, 0, 0 },
	/* 025 */	{ "", 0, 0, 0 },
	/* 026 */	{ "", 0, 0, 0 },
	/* 027 */	{ "", 0, 0, 0 },
	/* 028 */	{ "", 0, 0, 0 },
	/* 029 */	{ "", 0, 0, 0 },
	/* 02a */	{ "", 0, 0, 0 },
	/* 02b */	{ "", 0, 0, 0 },
	/* 02c */	{ "", 0, 0, 0 },
	/* 02d */	{ "", 0, 0, 0 },
	/* 02e */	{ "", 0, 0, 0 },
	/* 02f */	{ "", 0, 0, 0 },
	/* 030 */	{ "", 0, 0, 0 },
	/* 031 */	{ "", 0, 0, 0 },
	/* 032 */	{ "", 0, 0, 0 },
	/* 033 */	{ "", 0, 0, 0 },
	/* 034 */	{ "", 0, 0, 0 },
	/* 035 */	{ "", 0, 0, 0 },
	/* 036 */	{ "", 0, 0, 0 },
	/* 037 */	{ "", 0, 0, 0 },
	/* 038 */	{ "", 0, 0, 0 },
	/* 039 */	{ "", 0, 0, 0 },
	/* 03a */	{ "", 0, 0, 0 },
	/* 03b */	{ "", 0, 0, 0 },
	/* 03c */	{ "", 0, 0, 0 },
	/* 03d */	{ "", 0, 0, 0 },
	/* 03e */	{ "", 0, 0, 0 },
	/* 03f */	{ "", 0, 0, 0 },
	/* 040 */	{ "", 0, 0, 0 },
	/* 041 */	{ "", 0, 0, 0 },
	/* 042 */	{ "", 0, 0, 0 },
	/* 043 */	{ "", 0, 0, 0 },
	/* 044 */	{ "", 0, 0, 0 },
	/* 045 */	{ "", 0, 0, 0 },
	/* 046 */	{ "", 0, 0, 0 },
	/* 047 */	{ "", 0, 0, 0 },
	/* 048 */	{ "", 0, 0, 0 },
	/* 049 */	{ "", 0, 0, 0 },
	/* 04a */	{ "", 0, 0, 0 },
	/* 04b */	{ "", 0, 0, 0 },
	/* 04c */	{ "", 0, 0, 0 },
	/* 04d */	{ "", 0, 0, 0 },
	/* 04e */	{ "", 0, 0, 0 },
	/* 04f */	{ "", 0, 0, 0 },
	/* 050 */	{ "", 0, 0, 0 },
	/* 051 */	{ "", 0, 0, 0 },
	/* 052 */	{ "", 0, 0, 0 },
	/* 053 */	{ "", 0, 0, 0 },
	/* 054 */	{ "", 0, 0, 0 },
	/* 055 */	{ "", 0, 0, 0 },
	/* 056 */	{ "", 0, 0, 0 },
	/* 057 */	{ "", 0, 0, 0 },
	/* 058 */	{ "", 0, 0, 0 },
	/* 059 */	{ "", 0, 0, 0 },
	/* 05a */	{ "", 0, 0, 0 },
	/* 05b */	{ "", 0, 0, 0 },
	/* 05c */	{ "", 0, 0, 0 },
	/* 05d */	{ "", 0, 0, 0 },
	/* 05e */	{ "", 0, 0, 0 },
	/* 05f */	{ "", 0, 0, 0 },
	/* 060 */	{ "", 0, 0, 0 },
	/* 061 */	{ "", 0, 0, 0 },
	/* 062 */	{ "", 0, 0, 0 },
	/* 063 */	{ "", 0, 0, 0 },
	/* 064 */	{ "", 0, 0, 0 },
	/* 065 */	{ "", 0, 0, 0 },
	/* 066 */	{ "", 0, 0, 0 },
	/* 067 */	{ "", 0, 0, 0 },
	/* 068 */	{ "", 0, 0, 0 },
	/* 069 */	{ "", 0, 0, 0 },
	/* 06a */	{ "", 0, 0, 0 },
	/* 06b */	{ "", 0, 0, 0 },
	/* 06c */	{ "", 0, 0, 0 },
	/* 06d */	{ "", 0, 0, 0 },
	/* 06e */	{ "", 0, 0, 0 },
	/* 06f */	{ "", 0, 0, 0 },
	/* 070 */	{ "", 0, 0, 0 },
	/* 071 */	{ "", 0, 0, 0 },
	/* 072 */	{ "", 0, 0, 0 },
	/* 073 */	{ "", 0, 0, 0 },
	/* 074 */	{ "", 0, 0, 0 },
	/* 075 */	{ "", 0, 0, 0 },
	/* 076 */	{ "", 0, 0, 0 },
	/* 077 */	{ "", 0, 0, 0 },
	/* 078 */	{ "", 0, 0, 0 },
	/* 079 */	{ "", 0, 0, 0 },
	/* 07a */	{ "", 0, 0, 0 },
	/* 07b */	{ "", 0, 0, 0 },
	/* 07c */	{ "", 0, 0, 0 },
	/* 07d */	{ "", 0, 0, 0 },
	/* 07e */	{ "", 0, 0, 0 },
	/* 07f */	{ "", 0, 0, 0 },
	/* 080 */	{ "", 0, 0, 0 },
	/* 081 */	{ "", 0, 0, 0 },
	/* 082 */	{ "", 0, 0, 0 },
	/* 083 */	{ "", 0, 0, 0 },
	/* 084 */	{ "", 0, 0, 0 },
	/* 085 */	{ "", 0, 0, 0 },
	/* 086 */	{ "", 0, 0, 0 },
	/* 087 */	{ "", 0, 0, 0 },
	/* 088 */	{ "", 0, 0, 0 },
	/* 089 */	{ "", 0, 0, 0 },
	/* 08a */	{ "", 0, 0, 0 },
	/* 08b */	{ "", 0, 0, 0 },
	/* 08c */	{ "", 0, 0, 0 },
	/* 08d */	{ "", 0, 0, 0 },
	/* 08e */	{ "", 0, 0, 0 },
	/* 08f */	{ "", 0, 0, 0 },
	/* 090 */	{ "", 0, 0, 0 },
	/* 091 */	{ "", 0, 0, 0 },
	/* 092 */	{ "", 0, 0, 0 },
	/* 093 */	{ "", 0, 0, 0 },
	/* 094 */	{ "", 0, 0, 0 },
	/* 095 */	{ "", 0, 0, 0 },
	/* 096 */	{ "", 0, 0, 0 },
	/* 097 */	{ "", 0, 0, 0 },
	/* 098 */	{ "", 0, 0, 0 },
	/* 099 */	{ "", 0, 0, 0 },
	/* 09a */	{ "", 0, 0, 0 },
	/* 09b */	{ "", 0, 0, 0 },
	/* 09c */	{ "", 0, 0, 0 },
	/* 09d */	{ "", 0, 0, 0 },
	/* 09e */	{ "", 0, 0, 0 },
	/* 09f */	{ "", 0, 0, 0 },
	/* 0a0 */	{ "", 0, 0, 0 },
	/* 0a1 */	{ "", 0, 0, 0 },
	/* 0a2 */	{ "", 0, 0, 0 },
	/* 0a3 */	{ "", 0, 0, 0 },
	/* 0a4 */	{ "", 0, 0, 0 },
	/* 0a5 */	{ "", 0, 0, 0 },
	/* 0a6 */	{ "", 0, 0, 0 },
	/* 0a7 */	{ "", 0, 0, 0 },
	/* 0a8 */	{ "", 0, 0, 0 },
	/* 0a9 */	{ "", 0, 0, 0 },
	/* 0aa */	{ "", 0, 0, 0 },
	/* 0ab */	{ "", 0, 0, 0 },
	/* 0ac */	{ "", 0, 0, 0 },
	/* 0ad */	{ "", 0, 0, 0 },
	/* 0ae */	{ "", 0, 0, 0 },
	/* 0af */	{ "", 0, 0, 0 },
	/* 0b0 */	{ "", 0, 0, 0 },
	/* 0b1 */	{ "", 0, 0, 0 },
	/* 0b2 */	{ "", 0, 0, 0 },
	/* 0b3 */	{ "", 0, 0, 0 },
	/* 0b4 */	{ "", 0, 0, 0 },
	/* 0b5 */	{ "", 0, 0, 0 },
	/* 0b6 */	{ "", 0, 0, 0 },
	/* 0b7 */	{ "", 0, 0, 0 },
	/* 0b8 */	{ "", 0, 0, 0 },
	/* 0b9 */	{ "", 0, 0, 0 },
	/* 0ba */	{ "", 0, 0, 0 },
	/* 0bb */	{ "", 0, 0, 0 },
	/* 0bc */	{ "", 0, 0, 0 },
	/* 0bd */	{ "", 0, 0, 0 },
	/* 0be */	{ "", 0, 0, 0 },
	/* 0bf */	{ "", 0, 0, 0 },
	/* 0c0 */	{ "", 0, 0, 0 },
	/* 0c1 */	{ "", 0, 0, 0 },
	/* 0c2 */	{ "", 0, 0, 0 },
	/* 0c3 */	{ "", 0, 0, 0 },
	/* 0c4 */	{ "", 0, 0, 0 },
	/* 0c5 */	{ "", 0, 0, 0 },
	/* 0c6 */	{ "", 0, 0, 0 },
	/* 0c7 */	{ "", 0, 0, 0 },
	/* 0c8 */	{ "", 0, 0, 0 },
	/* 0c9 */	{ "", 0, 0, 0 },
	/* 0ca */	{ "", 0, 0, 0 },
	/* 0cb */	{ "", 0, 0, 0 },
	/* 0cc */	{ "", 0, 0, 0 },
	/* 0cd */	{ "", 0, 0, 0 },
	/* 0ce */	{ "", 0, 0, 0 },
	/* 0cf */	{ "", 0, 0, 0 },
	/* 0d0 */	{ "", 0, 0, 0 },
	/* 0d1 */	{ "", 0, 0, 0 },
	/* 0d2 */	{ "", 0, 0, 0 },
	/* 0d3 */	{ "", 0, 0, 0 },
	/* 0d4 */	{ "", 0, 0, 0 },
	/* 0d5 */	{ "", 0, 0, 0 },
	/* 0d6 */	{ "", 0, 0, 0 },
	/* 0d7 */	{ "", 0, 0, 0 },
	/* 0d8 */	{ "", 0, 0, 0 },
	/* 0d9 */	{ "", 0, 0, 0 },
	/* 0da */	{ "", 0, 0, 0 },
	/* 0db */	{ "", 0, 0, 0 },
	/* 0dc */	{ "", 0, 0, 0 },
	/* 0dd */	{ "", 0, 0, 0 },
	/* 0de */	{ "", 0, 0, 0 },
	/* 0df */	{ "", 0, 0, 0 },
	/* 0e0 */	{ "", 0, 0, 0 },
	/* 0e1 */	{ "", 0, 0, 0 },
	/* 0e2 */	{ "", 0, 0, 0 },
	/* 0e3 */	{ "", 0, 0, 0 },
	/* 0e4 */	{ "", 0, 0, 0 },
	/* 0e5 */	{ "", 0, 0, 0 },
	/* 0e6 */	{ "", 0, 0, 0 },
	/* 0e7 */	{ "", 0, 0, 0 },
	/* 0e8 */	{ "", 0, 0, 0 },
	/* 0e9 */	{ "", 0, 0, 0 },
	/* 0ea */	{ "", 0, 0, 0 },
	/* 0eb */	{ "", 0, 0, 0 },
	/* 0ec */	{ "", 0, 0, 0 },
	/* 0ed */	{ "", 0, 0, 0 },
	/* 0ee */	{ "", 0, 0, 0 },
	/* 0ef */	{ "", 0, 0, 0 },
	/* 0f0 */	{ "", 0, 0, 0 },
	/* 0f1 */	{ "", 0, 0, 0 },
	/* 0f2 */	{ "", 0, 0, 0 },
	/* 0f3 */	{ "", 0, 0, 0 },
	/* 0f4 */	{ "", 0, 0, 0 },
	/* 0f5 */	{ "", 0, 0, 0 },
	/* 0f6 */	{ "", 0, 0, 0 },
	/* 0f7 */	{ "", 0, 0, 0 },
	/* 0f8 */	{ "", 0, 0, 0 },
	/* 0f9 */	{ "", 0, 0, 0 },
	/* 0fa */	{ "", 0, 0, 0 },
	/* 0fb */	{ "", 0, 0, 0 },
	/* 0fc */	{ "", 0, 0, 0 },
	/* 0fd */	{ "", 0, 0, 0 },
	/* 0fe */	{ "", 0, 0, 0 },
	/* 0ff */	{ "", 0, 0, 0 },
	/* 100 */	{ "", 0, 0, 0 },
	/* 101 */	{ "", 0, 0, 0 },
	/* 102 */	{ "", 0, 0, 0 },
	/* 103 */	{ "", 0, 0, 0 },
	/* 104 */	{ "", 0, 0, 0 },
	/* 105 */	{ "", 0, 0, 0 },
	/* 106 */	{ "", 0, 0, 0 },
	/* 107 */	{ "", 0, 0, 0 },
	/* 108 */	{ "", 0, 0, 0 },
	/* 109 */	{ "", 0, 0, 0 },
	/* 10a */	{ "", 0, 0, 0 },
	/* 10b */	{ "", 0, 0, 0 },
	/* 10c */	{ "", 0, 0, 0 },
	/* 10d */	{ "", 0, 0, 0 },
	/* 10e */	{ "", 0, 0, 0 },
	/* 10f */	{ "", 0, 0, 0 },
	/* 110 */	{ "", 0, 0, 0 },
	/* 111 */	{ "", 0, 0, 0 },
	/* 112 */	{ "", 0, 0, 0 },
	/* 113 */	{ "", 0, 0, 0 },
	/* 114 */	{ "", 0, 0, 0 },
	/* 115 */	{ "", 0, 0, 0 },
	/* 116 */	{ "", 0, 0, 0 },
	/* 117 */	{ "", 0, 0, 0 },
	/* 118 */	{ "", 0, 0, 0 },
	/* 119 */	{ "", 0, 0, 0 },
	/* 11a */	{ "", 0, 0, 0 },
	/* 11b */	{ "", 0, 0, 0 },
	/* 11c */	{ "", 0, 0, 0 },
	/* 11d */	{ "", 0, 0, 0 },
	/* 11e */	{ "", 0, 0, 0 },
	/* 11f */	{ "", 0, 0, 0 },
	/* 120 */	{ "", 0, 0, 0 },
	/* 121 */	{ "", 0, 0, 0 },
	/* 122 */	{ "", 0, 0, 0 },
	/* 123 */	{ "", 0, 0, 0 },
	/* 124 */	{ "", 0, 0, 0 },
	/* 125 */	{ "", 0, 0, 0 },
	/* 126 */	{ "", 0, 0, 0 },
	/* 127 */	{ "", 0, 0, 0 },
	/* 128 */	{ "", 0, 0, 0 },
	/* 129 */	{ "", 0, 0, 0 },
	/* 12a */	{ "", 0, 0, 0 },
	/* 12b */	{ "", 0, 0, 0 },
	/* 12c */	{ "", 0, 0, 0 },
	/* 12d */	{ "", 0, 0, 0 },
	/* 12e */	{ "", 0, 0, 0 },
	/* 12f */	{ "", 0, 0, 0 },
	/* 130 */	{ "", 0, 0, 0 },
	/* 131 */	{ "", 0, 0, 0 },
	/* 132 */	{ "", 0, 0, 0 },
	/* 133 */	{ "", 0, 0, 0 },
	/* 134 */	{ "", 0, 0, 0 },
	/* 135 */	{ "", 0, 0, 0 },
	/* 136 */	{ "", 0, 0, 0 },
	/* 137 */	{ "", 0, 0, 0 },
	/* 138 */	{ "", 0, 0, 0 },
	/* 139 */	{ "", 0, 0, 0 },
	/* 13a */	{ "", 0, 0, 0 },
	/* 13b */	{ "", 0, 0, 0 },
	/* 13c */	{ "", 0, 0, 0 },
	/* 13d */	{ "", 0, 0, 0 },
	/* 13e */	{ "", 0, 0, 0 },
	/* 13f */	{ "", 0, 0, 0 },
	/* 140 */	{ "", 0, 0, 0 },
	/* 141 */	{ "", 0, 0, 0 },
	/* 142 */	{ "", 0, 0, 0 },
	/* 143 */	{ "", 0, 0, 0 },
	/* 144 */	{ "", 0, 0, 0 },
	/* 145 */	{ "", 0, 0, 0 },
	/* 146 */	{ "", 0, 0, 0 },
	/* 147 */	{ "", 0, 0, 0 },
	/* 148 */	{ "", 0, 0, 0 },
	/* 149 */	{ "", 0, 0, 0 },
	/* 14a */	{ "", 0, 0, 0 },
	/* 14b */	{ "", 0, 0, 0 },
	/* 14c */	{ "", 0, 0, 0 },
	/* 14d */	{ "", 0, 0, 0 },
	/* 14e */	{ "", 0, 0, 0 },
	/* 14f */	{ "", 0, 0, 0 },
	/* 150 */	{ "", 0, 0, 0 },
	/* 151 */	{ "", 0, 0, 0 },
	/* 152 */	{ "", 0, 0, 0 },
	/* 153 */	{ "", 0, 0, 0 },
	/* 154 */	{ "", 0, 0, 0 },
	/* 155 */	{ "", 0, 0, 0 },
	/* 156 */	{ "", 0, 0, 0 },
	/* 157 */	{ "", 0, 0, 0 },
	/* 158 */	{ "", 0, 0, 0 },
	/* 159 */	{ "", 0, 0, 0 },
	/* 15a */	{ "", 0, 0, 0 },
	/* 15b */	{ "", 0, 0, 0 },
	/* 15c */	{ "", 0, 0, 0 },
	/* 15d */	{ "", 0, 0, 0 },
	/* 15e */	{ "", 0, 0, 0 },
	/* 15f */	{ "", 0, 0, 0 },
	/* 160 */	{ "", 0, 0, 0 },
	/* 161 */	{ "", 0, 0, 0 },
	/* 162 */	{ "", 0, 0, 0 },
	/* 163 */	{ "", 0, 0, 0 },
	/* 164 */	{ "", 0, 0, 0 },
	/* 165 */	{ "", 0, 0, 0 },
	/* 166 */	{ "", 0, 0, 0 },
	/* 167 */	{ "", 0, 0, 0 },
	/* 168 */	{ "", 0, 0, 0 },
	/* 169 */	{ "", 0, 0, 0 },
	/* 16a */	{ "", 0, 0, 0 },
	/* 16b */	{ "", 0, 0, 0 },
	/* 16c */	{ "", 0, 0, 0 },
	/* 16d */	{ "", 0, 0, 0 },
	/* 16e */	{ "", 0, 0, 0 },
	/* 16f */	{ "", 0, 0, 0 },
	/* 170 */	{ "", 0, 0, 0 },
	/* 171 */	{ "", 0, 0, 0 },
	/* 172 */	{ "", 0, 0, 0 },
	/* 173 */	{ "", 0, 0, 0 },
	/* 174 */	{ "", 0, 0, 0 },
	/* 175 */	{ "", 0, 0, 0 },
	/* 176 */	{ "", 0, 0, 0 },
	/* 177 */	{ "", 0, 0, 0 },
	/* 178 */	{ "", 0, 0, 0 },
	/* 179 */	{ "", 0, 0, 0 },
	/* 17a */	{ "", 0, 0, 0 },
	/* 17b */	{ "", 0, 0, 0 },
	/* 17c */	{ "", 0, 0, 0 },
	/* 17d */	{ "", 0, 0, 0 },
	/* 17e */	{ "", 0, 0, 0 },
	/* 17f */	{ "", 0, 0, 0 },
	/* 180 */	{ "", 0, 0, 0 },
	/* 181 */	{ "", 0, 0, 0 },
	/* 182 */	{ "", 0, 0, 0 },
	/* 183 */	{ "", 0, 0, 0 },
	/* 184 */	{ "", 0, 0, 0 },
	/* 185 */	{ "", 0, 0, 0 },
	/* 186 */	{ "", 0, 0, 0 },
	/* 187 */	{ "", 0, 0, 0 },
	/* 188 */	{ "", 0, 0, 0 },
	/* 189 */	{ "", 0, 0, 0 },
	/* 18a */	{ "", 0, 0, 0 },
	/* 18b */	{ "", 0, 0, 0 },
	/* 18c */	{ "", 0, 0, 0 },
	/* 18d */	{ "", 0, 0, 0 },
	/* 18e */	{ "", 0, 0, 0 },
	/* 18f */	{ "", 0, 0, 0 },
	/* 190 */	{ "", 0, 0, 0 },
	/* 191 */	{ "", 0, 0, 0 },
	/* 192 */	{ "", 0, 0, 0 },
	/* 193 */	{ "", 0, 0, 0 },
	/* 194 */	{ "", 0, 0, 0 },
	/* 195 */	{ "", 0, 0, 0 },
	/* 196 */	{ "", 0, 0, 0 },
	/* 197 */	{ "", 0, 0, 0 },
	/* 198 */	{ "", 0, 0, 0 },
	/* 199 */	{ "", 0, 0, 0 },
	/* 19a */	{ "", 0, 0, 0 },
	/* 19b */	{ "", 0, 0, 0 },
	/* 19c */	{ "", 0, 0, 0 },
	/* 19d */	{ "", 0, 0, 0 },
	/* 19e */	{ "", 0, 0, 0 },
	/* 19f */	{ "", 0, 0, 0 },
	/* 1a0 */	{ "", 0, 0, 0 },
	/* 1a1 */	{ "", 0, 0, 0 },
	/* 1a2 */	{ "", 0, 0, 0 },
	/* 1a3 */	{ "", 0, 0, 0 },
	/* 1a4 */	{ "", 0, 0, 0 },
	/* 1a5 */	{ "", 0, 0, 0 },
	/* 1a6 */	{ "", 0, 0, 0 },
	/* 1a7 */	{ "", 0, 0, 0 },
	/* 1a8 */	{ "", 0, 0, 0 },
	/* 1a9 */	{ "", 0, 0, 0 },
	/* 1aa */	{ "", 0, 0, 0 },
	/* 1ab */	{ "", 0, 0, 0 },
	/* 1ac */	{ "", 0, 0, 0 },
	/* 1ad */	{ "", 0, 0, 0 },
	/* 1ae */	{ "", 0, 0, 0 },
	/* 1af */	{ "", 0, 0, 0 },
	/* 1b0 */	{ "", 0, 0, 0 },
	/* 1b1 */	{ "", 0, 0, 0 },
	/* 1b2 */	{ "", 0, 0, 0 },
	/* 1b3 */	{ "", 0, 0, 0 },
	/* 1b4 */	{ "", 0, 0, 0 },
	/* 1b5 */	{ "", 0, 0, 0 },
	/* 1b6 */	{ "", 0, 0, 0 },
	/* 1b7 */	{ "", 0, 0, 0 },
	/* 1b8 */	{ "", 0, 0, 0 },
	/* 1b9 */	{ "", 0, 0, 0 },
	/* 1ba */	{ "", 0, 0, 0 },
	/* 1bb */	{ "", 0, 0, 0 },
	/* 1bc */	{ "", 0, 0, 0 },
	/* 1bd */	{ "", 0, 0, 0 },
	/* 1be */	{ "", 0, 0, 0 },
	/* 1bf */	{ "", 0, 0, 0 },
	/* 1c0 */	{ "", 0, 0, 0 },
	/* 1c1 */	{ "", 0, 0, 0 },
	/* 1c2 */	{ "", 0, 0, 0 },
	/* 1c3 */	{ "", 0, 0, 0 },
	/* 1c4 */	{ "", 0, 0, 0 },
	/* 1c5 */	{ "", 0, 0, 0 },
	/* 1c6 */	{ "", 0, 0, 0 },
	/* 1c7 */	{ "", 0, 0, 0 },
	/* 1c8 */	{ "", 0, 0, 0 },
	/* 1c9 */	{ "", 0, 0, 0 },
	/* 1ca */	{ "", 0, 0, 0 },
	/* 1cb */	{ "", 0, 0, 0 },
	/* 1cc */	{ "", 0, 0, 0 },
	/* 1cd */	{ "", 0, 0, 0 },
	/* 1ce */	{ "", 0, 0, 0 },
	/* 1cf */	{ "", 0, 0, 0 },
	/* 1d0 */	{ "", 0, 0, 0 },
	/* 1d1 */	{ "", 0, 0, 0 },
	/* 1d2 */	{ "", 0, 0, 0 },
	/* 1d3 */	{ "", 0, 0, 0 },
	/* 1d4 */	{ "", 0, 0, 0 },
	/* 1d5 */	{ "", 0, 0, 0 },
	/* 1d6 */	{ "", 0, 0, 0 },
	/* 1d7 */	{ "", 0, 0, 0 },
	/* 1d8 */	{ "", 0, 0, 0 },
	/* 1d9 */	{ "", 0, 0, 0 },
	/* 1da */	{ "", 0, 0, 0 },
	/* 1db */	{ "", 0, 0, 0 },
	/* 1dc */	{ "", 0, 0, 0 },
	/* 1dd */	{ "", 0, 0, 0 },
	/* 1de */	{ "", 0, 0, 0 },
	/* 1df */	{ "", 0, 0, 0 },
	/* 1e0 */	{ "", 0, 0, 0 },
	/* 1e1 */	{ "", 0, 0, 0 },
	/* 1e2 */	{ "", 0, 0, 0 },
	/* 1e3 */	{ "", 0, 0, 0 },
	/* 1e4 */	{ "", 0, 0, 0 },
	/* 1e5 */	{ "", 0, 0, 0 },
	/* 1e6 */	{ "", 0, 0, 0 },
	/* 1e7 */	{ "", 0, 0, 0 },
	/* 1e8 */	{ "", 0, 0, 0 },
	/* 1e9 */	{ "", 0, 0, 0 },
	/* 1ea */	{ "", 0, 0, 0 },
	/* 1eb */	{ "", 0, 0, 0 },
	/* 1ec */	{ "", 0, 0, 0 },
	/* 1ed */	{ "", 0, 0, 0 },
	/* 1ee */	{ "", 0, 0, 0 },
	/* 1ef */	{ "", 0, 0, 0 },
	/* 1f0 */	{ "", 0, 0, 0 },
	/* 1f1 */	{ "", 0, 0, 0 },
	/* 1f2 */	{ "", 0, 0, 0 },
	/* 1f3 */	{ "", 0, 0, 0 },
	/* 1f4 */	{ "", 0, 0, 0 },
	/* 1f5 */	{ "", 0, 0, 0 },
	/* 1f6 */	{ "", 0, 0, 0 },
	/* 1f7 */	{ "", 0, 0, 0 },
	/* 1f8 */	{ "", 0, 0, 0 },
	/* 1f9 */	{ "", 0, 0, 0 },
	/* 1fa */	{ "", 0, 0, 0 },
	/* 1fb */	{ "", 0, 0, 0 },
	/* 1fc */	{ "", 0, 0, 0 },
	/* 1fd */	{ "", 0, 0, 0 },
	/* 1fe */	{ "", 0, 0, 0 },
	/* 1ff */	{ "", 0, 0, 0 },
	/* 200 */	{ "", 0, 0, 0 },
	/* 201 */	{ "", 0, 0, 0 },
	/* 202 */	{ "", 0, 0, 0 },
	/* 203 */	{ "", 0, 0, 0 },
	/* 204 */	{ "", 0, 0, 0 },
	/* 205 */	{ "", 0, 0, 0 },
	/* 206 */	{ "", 0, 0, 0 },
	/* 207 */	{ "", 0, 0, 0 },
	/* 208 */	{ "", 0, 0, 0 },
	/* 209 */	{ "", 0, 0, 0 },
	/* 20a */	{ "", 0, 0, 0 },
	/* 20b */	{ "", 0, 0, 0 },
	/* 20c */	{ "", 0, 0, 0 },
	/* 20d */	{ "", 0, 0, 0 },
	/* 20e */	{ "", 0, 0, 0 },
	/* 20f */	{ "", 0, 0, 0 },
	/* 210 */	{ "", 0, 0, 0 },
	/* 211 */	{ "", 0, 0, 0 },
	/* 212 */	{ "", 0, 0, 0 },
	/* 213 */	{ "", 0, 0, 0 },
	/* 214 */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif

extern long int IT_1H2LP1UI();
extern long int IT_1H();
extern long int IT_1I();
extern long int IT_SETTIMER();
extern long int IT_1H1UI();
extern long int IT_V();
extern long int IT_V();
extern long int IT_1LPP();
extern long int IT_1H();
extern long int IT_V();
extern long int IT_1UI();
extern long int IT_V();
extern long int IT_1H();
extern long int IT_V();
extern long int IT_1H1LP();
extern long int IT_1H1LP();
extern long int IT_1H1LP1H();
extern long int IT_ENUMPROPS();
extern long int IT_1H1LPP();
extern long int IT_1H1LPP();
extern long int IT_WINFROMPT();
extern long int IT_1H();
extern long int IT_1H1LPR();
extern long int IT_1H1LPR();
extern long int IT_1H1I();
extern long int IT_1H();
extern long int IT_1H1LP1I();
extern long int IT_1H1LP();
extern long int IT_1H();
extern long int IT_1H1LPPS();
extern long int IT_1H1LPPSc();
extern long int IT_CREATEWIN();
extern long int IT_1H1I();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_2H();
extern long int IT_1H();
extern long int IT_2LP();
extern long int IT_V();
extern long int IT_1H();
extern long int IT_ENUMWINDOWS();
extern long int IT_ENUMCHILDWIN();
extern long int IT_1H5I();
extern long int IT_REGCLASS();
extern long int IT_1H1LP1I();
extern long int IT_1H();
extern long int IT_V();
extern long int IT_1H2I2LPRc();
extern long int IT_1H3I();
extern long int IT_1H1I();
extern long int IT_1H4I();
extern long int IT_1H1I2LPI();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_2H();
extern long int IT_1H();
extern long int IT_2I();
extern long int IT_1I();
extern long int IT_1LPR4I();
extern long int IT_1LPR();
extern long int IT_1LPR1LPRc();
extern long int IT_1LPRc();
extern long int IT_PTINRECT();
extern long int IT_1LPR2I();
extern long int IT_1LPR2I();
extern long int IT_1LPR2LPRc();
extern long int IT_1LPR2LPRc();
extern long int IT_1H1LPRc1H();
extern long int IT_1H1LPRc();
extern long int IT_1H1LPRc1H();
extern long int IT_1H2I1H();
extern long int IT_DRAWTEXT();
extern long int IT_DIALOG();
extern long int IT_1H1I();
extern long int IT_DIALOG();
extern long int IT_ISDIALOGMSG();
extern long int IT_1H1I();
extern long int IT_1H1I1LP();
extern long int IT_1H1I1LP1I();
extern long int IT_1H1I1UI1I();
extern long int IT_1H1I1LPI1I();
extern long int IT_1H3I();
extern long int IT_1H1I1UI();
extern long int IT_1H1I();
extern long int IT_1H1LP1I();
extern long int IT_DLGDIRLIST();
extern long int IT_SENDDLGITEMMSG();
extern long int IT_1LPR1D1I();
extern long int IT_1UI();
extern long int IT_1H1I();
extern long int IT_1I();
extern long int IT_SENDMESSAGE();
extern long int IT_GETMESSAGE();
extern long int IT_PEEKMESSAGE();
extern long int IT_SENDMESSAGE();
extern long int IT_SENDMESSAGE();
extern long int IT_SCROLLWINEX();
extern long int IT_V();
extern long int IT_TRANSLATEMSG();
extern long int IT_DISPATCHMSG();
extern long int IT_1D();
extern long int IT_SENDMESSAGE();
extern long int IT_1LP();
extern long int IT_V();
extern long int IT_V();
extern long int IT_SETWINDOWSHOOK();
extern long int IT_UNHOOKWHOOK();
extern long int IT_SETWINDOWSHOOKEX();
extern long int IT_CALLNEXTHOOKEX();
extern long int IT_CALLWINPROC();
extern long int IT_1H();
extern long int IT_1H1LPRc1I();
extern long int IT_2H1I();
extern long int IT_1H1LPRc();
extern long int IT_2H();
extern long int IT_GCWORD();
extern long int IT_SCWORD();
extern long int IT_1H1I1W();
extern long int IT_GCLONG();
extern long int IT_SCLONG();
extern long int IT_1H1I();
extern long int IT_1H1I1W();
extern long int IT_GWLONG();
extern long int IT_SWLONG();
extern long int IT_1H();
extern long int IT_V();
extern long int IT_V();
extern long int IT_V();
extern long int IT_SETCLIPBDATA();
extern long int IT_GETCLIPBDATA();
extern long int IT_V();
extern long int IT_1UI();
extern long int IT_1LP();
extern long int IT_1UI1LP1I();
extern long int IT_1H();
extern long int IT_V();
extern long int IT_2H();
extern long int IT_1H1LP();
extern long int IT_V();
extern long int IT_1H();
extern long int IT_1H1UI1LP2UI();
extern long int IT_1H2UI();
extern long int IT_1H2UI();
extern long int IT_1H1I();
extern long int IT_1H();
extern long int IT_2H();
extern long int IT_1H1I();
extern long int IT_1H();
extern long int IT_GETMENUSTRING();
extern long int IT_2H2UI();
extern long int IT_2H2I();
extern long int IT_V();
extern long int IT_2I();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_1UI();
extern long int IT_V();
extern long int IT_1H1LP1UI1D();
extern long int IT_1H1LP();
extern long int IT_1H1LP();
extern long int IT_1H1LP();
extern long int IT_1H1UI1LP1I();
extern long int IT_1H1LP();
extern long int IT_LOADACCEL();
extern long int IT_TRANSLATEACCEL();
extern long int IT_1I();
extern long int IT_1I();
extern long int IT_SETSYSCOLORS();
extern long int IT_1LPP();
extern long int IT_QUERYSENDMSG();
extern long int IT_GRAYSTRING();
extern long int IT_1I();
extern long int IT_1H1LPR1I();
extern long int IT_CHWINFROMPT();
extern long int IT_V();
extern long int IT_1UI();
extern long int IT_1H1LP1I();
extern long int IT_DLGBOXIN();
extern long int IT_CREATEDLGIN();
extern long int IT_LOADMENUIND();
extern long int IT_SCROLLDC();
extern long int IT_1LP();
extern long int IT_1LP();
extern long int IT_1H();
extern long int IT_ENUMTASKWIN();
extern long int IT_2H1I();
extern long int IT_2H1I();
extern long int IT_1H();
extern long int IT_1H1UI();
extern long int IT_2H4I1UI();
extern long int IT_2H();
extern long int IT_V();
extern long int IT_2H1I();
extern long int IT_2H();
extern long int IT_DEFHOOKPROC();
extern long int IT_DLGPARAM();
extern long int IT_DLGBOXIP();
extern long int IT_DLGPARAM();
extern long int IT_CREATEDLGIP();
extern long int IT_V();
extern long int IT_2LPRc();
extern long int IT_1I();
extern long int IT_1H2UI();
extern long int IT_1I();
extern long int IT_3H4I1UI();
extern long int IT_1H();
extern long int IT_1H1UI();
extern long int IT_1H();
extern long int IT_1H1I();
extern long int IT_1I();
extern long int IT_1H2I();
extern long int IT_1LP();
extern long int IT_1H();
extern long int IT_1LP();
extern long int IT_1H1LP1I();
extern long int IT_1H();
extern long int IT_1H();
extern long int IT_2H1I();
extern long int IT_1H();
extern long int IT_V();
extern long int IT_SENDMESSAGE();
extern long int IT_1H();
extern long int IT_1LP1H();
extern long int IT_1LPUI1I();
extern long int IT_GETCLASSINFO();
extern long int IT_1H4I2LPB();
extern long int IT_1H2I2B2LPB();
extern long int IT_INSERTMENU();
extern long int IT_APPENDMENU();
extern long int IT_1H2UI();
extern long int IT_1H2UI();
extern long int IT_1H3UI1LP();
extern long int IT_V();
extern long int IT_1H1UI3I1H1LPRc();
extern long int IT_V();
extern long int IT_1H2UI2H();
extern long int IT_2H1LPP1UI();
extern long int IT_WSPRINTF();
extern long int IT_WVSPRINTF();
extern long int IT_1H1LP2I();
extern long int IT_2LP();
extern long int IT_LP1LP();
extern long int IT_LP1LP();
extern long int IT_1B();
extern long int IT_1B();
extern long int IT_1B();
extern long int IT_1B();
extern long int IT_1LP1UI();
extern long int IT_1LP1UI();
extern long int IT_DEFFRAMEPROC();
extern long int IT_CREATEWINEX();
extern long int IT_TABTEXTOUT();
extern long int IT_GETTABBEDTEXTEXT();
extern long int IT_1H1W();
extern long int IT_1LPR1D1I1D();
extern long int IT_2LP();
extern long int IT_WNETGETCONN();
extern long int IT_WNETGETUSER();
extern long int IT_LP1LP();
extern long int IT_LP2LP();
extern long int IT_SYSPARAMINFO();
extern long int _86_GetFreeSystemResources();
extern long int IT_1H1LPWP();
extern long int IT_1H1LPWPc();
extern long int IT_1H1LPRc1H1UI();
extern long int IT_CALLMSGFILTER();
extern long int IT_1LP2UI();
extern long int IT_1LPDCB();
extern long int IT_1I1LPDCB();
extern long int IT_1I1LPCOMSTAT();
extern long int IT_1I1LPV1I();
extern long int IT_1I1UI();
extern long int IT_1I1H2I();
extern long int IT_GETWINTASK();
extern long int IT_POSTAPPMSG();
extern long int IT_2H1D();
extern long int IT_GETWINDOWWORD();
extern long int IT_SETWINDOWWORD();
extern long int IT_1H1LP2I1UI();
extern long int IT_CALCCHILDSCROLL();

 /* Interface Segment Image USER: */

static long int (*seg_image_USER_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	IT_1H2LP1UI,  MessageBox,
	/* 002 */	IT_V, OldExitWindows,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	IT_1H,  InitApp,
	/* 006 */	IT_1I,  PostQuitMessage,
	/* 007 */	IT_1D,  ExitWindows,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
	/* 00a */	IT_SETTIMER,  SetTimer,
	/* 00b */	IT_SETTIMER, SetSystemTimer,
	/* 00c */	IT_1H1UI,  KillTimer,
	/* 00d */	IT_V,  GetTickCount,
	/* 00e */	Trap, 0,
	/* 00f */	IT_V,  GetCurrentTime,
	/* 010 */	IT_1LPRc, ClipCursor,
	/* 011 */	IT_1LPP,  GetCursorPos,
	/* 012 */	IT_1H,  SetCapture,
	/* 013 */	IT_V,  ReleaseCapture,
	/* 014 */	IT_1UI,  SetDoubleClickTime,
	/* 015 */	IT_V,  GetDoubleClickTime,
	/* 016 */	IT_1H,  SetFocus,
	/* 017 */	IT_V,  GetFocus,
	/* 018 */	IT_1H1LP,  RemoveProp,
	/* 019 */	IT_1H1LP,  GetProp,
	/* 01a */	IT_1H1LP1H,  SetProp,
	/* 01b */	IT_ENUMPROPS,  EnumProps,
	/* 01c */	IT_1H1LPP,  ClientToScreen,
	/* 01d */	IT_1H1LPP,  ScreenToClient,
	/* 01e */	IT_WINFROMPT,  WindowFromPoint,
	/* 01f */	IT_1H,  IsIconic,
	/* 020 */	IT_1H1LPR,  GetWindowRect,
	/* 021 */	IT_1H1LPR,  GetClientRect,
	/* 022 */	IT_1H1I,  EnableWindow,
	/* 023 */	IT_1H,  IsWindowEnabled,
	/* 024 */	IT_1H1LP1I,  GetWindowText,
	/* 025 */	IT_1H1LP,  SetWindowText,
	/* 026 */	IT_1H,  GetWindowTextLength,
	/* 027 */	IT_1H1LPPS,  BeginPaint,
	/* 028 */	IT_1H1LPPSc,  EndPaint,
	/* 029 */	IT_CREATEWIN,  CreateWindow,
	/* 02a */	IT_1H1I,  ShowWindow,
	/* 02b */	Trap, 0,
	/* 02c */	IT_1H, OpenIcon,
	/* 02d */	IT_1H, BringWindowToTop,
	/* 02e */	IT_1H,  GetParent,
	/* 02f */	IT_1H,  IsWindow,
	/* 030 */	IT_2H,  IsChild,
	/* 031 */	IT_1H,  IsWindowVisible,
	/* 032 */	IT_2LP,  FindWindow,
	/* 033 */	Trap, 0,
	/* 034 */	IT_V,  AnyPopup,
	/* 035 */	IT_1H,  DestroyWindow,
	/* 036 */	IT_ENUMWINDOWS,  EnumWindows,
	/* 037 */	IT_ENUMCHILDWIN,  EnumChildWindows,
	/* 038 */	IT_1H5I,  MoveWindow,
	/* 039 */	IT_REGCLASS,  RegisterClass,
	/* 03a */	IT_1H1LP1I,  GetClassName,
	/* 03b */	IT_1H,  SetActiveWindow,
	/* 03c */	IT_V,  GetActiveWindow,
	/* 03d */	IT_1H2I2LPRc,  ScrollWindow,
	/* 03e */	IT_1H3I,  SetScrollPos,
	/* 03f */	IT_1H1I,  GetScrollPos,
	/* 040 */	IT_1H4I,  SetScrollRange,
	/* 041 */	IT_1H1I2LPI,  GetScrollRange,
	/* 042 */	IT_1H,  GetDC,
	/* 043 */	IT_1H,  GetWindowDC,
	/* 044 */	IT_2H,  ReleaseDC,
	/* 045 */	IT_1H,  SetCursor,
	/* 046 */	IT_2I,  SetCursorPos,
	/* 047 */	IT_1I,  ShowCursor,
	/* 048 */	IT_1LPR4I,  SetRect,
	/* 049 */	IT_1LPR,  SetRectEmpty,
	/* 04a */	IT_1LPR1LPRc,  CopyRect,
	/* 04b */	IT_1LPRc,  IsRectEmpty,
	/* 04c */	IT_PTINRECT,  PtInRect,
	/* 04d */	IT_1LPR2I,  OffsetRect,
	/* 04e */	IT_1LPR2I,  InflateRect,
	/* 04f */	IT_1LPR2LPRc,  IntersectRect,
	/* 050 */	IT_1LPR2LPRc,  UnionRect,
	/* 051 */	IT_1H1LPRc1H,  FillRect,
	/* 052 */	IT_1H1LPRc,  InvertRect,
	/* 053 */	IT_1H1LPRc1H,  FrameRect,
	/* 054 */	IT_1H2I1H,  DrawIcon,
	/* 055 */	IT_DRAWTEXT,  DrawText,
	/* 056 */	Trap, 0,
	/* 057 */	IT_DIALOG,  DialogBox,
	/* 058 */	IT_1H1I,  EndDialog,
	/* 059 */	IT_DIALOG,  CreateDialog,
	/* 05a */	IT_ISDIALOGMSG,  IsDialogMessage,
	/* 05b */	IT_1H1I,  GetDlgItem,
	/* 05c */	IT_1H1I1LP,  SetDlgItemText,
	/* 05d */	IT_1H1I1LP1I,  GetDlgItemText,
	/* 05e */	IT_1H1I1UI1I,  SetDlgItemInt,
	/* 05f */	IT_1H1I1LPI1I,  GetDlgItemInt,
	/* 060 */	IT_1H3I,  CheckRadioButton,
	/* 061 */	IT_1H1I1UI,  CheckDlgButton,
	/* 062 */	IT_1H1I,  IsDlgButtonChecked,
	/* 063 */	IT_1H1LP1I,  DlgDirSelect,
	/* 064 */	IT_DLGDIRLIST,  DlgDirList,
	/* 065 */	IT_SENDDLGITEMMSG,  SendDlgItemMessage,
	/* 066 */	IT_1LPR1D1I,  AdjustWindowRect,
	/* 067 */	IT_1H1LPR, MapDialogRect,
	/* 068 */	IT_1UI,  MessageBeep,
	/* 069 */	IT_1H1I,  FlashWindow,
	/* 06a */	IT_1I,  GetKeyState,
	/* 06b */	IT_SENDMESSAGE,  DefWindowProc,
	/* 06c */	IT_GETMESSAGE,  GetMessage,
	/* 06d */	IT_PEEKMESSAGE,  PeekMessage,
	/* 06e */	IT_SENDMESSAGE,  PostMessage,
	/* 06f */	IT_SENDMESSAGE,  SendMessage,
	/* 070 */	IT_V,  WaitMessage,
	/* 071 */	IT_TRANSLATEMSG,  TranslateMessage,
	/* 072 */	IT_DISPATCHMSG,  DispatchMessage,
	/* 073 */	IT_1D,  ReplyMessage,
	/* 074 */	IT_POSTAPPMSG,  PostAppMessage,
	/* 075 */	Trap, 0,
	/* 076 */	IT_1LP,  RegisterWindowMessage,
	/* 077 */	IT_V,  GetMessagePos,
	/* 078 */	IT_V,  GetMessageTime,
	/* 079 */	IT_SETWINDOWSHOOK,  SetWindowsHook,
	/* 07a */	IT_CALLWINPROC,  CallWindowProc,
	/* 07b */	IT_CALLMSGFILTER, CallMsgFilter,
	/* 07c */	IT_1H,  UpdateWindow,
	/* 07d */	IT_1H1LPRc1I,  InvalidateRect,
	/* 07e */	IT_2H1I,  InvalidateRgn,
	/* 07f */	IT_1H1LPRc,  ValidateRect,
	/* 080 */	IT_2H,  ValidateRgn,
	/* 081 */	IT_GCWORD,  GetClassWord,
	/* 082 */	IT_SCWORD,  SetClassWord,
	/* 083 */	IT_GCLONG,  GetClassLong,
	/* 084 */	IT_SCLONG,  SetClassLong,
	/* 085 */	IT_GETWINDOWWORD,  GetWindowWord,
	/* 086 */	IT_SETWINDOWWORD,  SetWindowWord,
	/* 087 */	IT_GWLONG,  GetWindowLong,
	/* 088 */	IT_SWLONG,  SetWindowLong,
	/* 089 */	IT_1H,  OpenClipboard,
	/* 08a */	IT_V,  CloseClipboard,
	/* 08b */	IT_V,  EmptyClipboard,
	/* 08c */	IT_V,  GetClipboardOwner,
	/* 08d */	IT_SETCLIPBDATA,  SetClipboardData,
	/* 08e */	IT_GETCLIPBDATA,  GetClipboardData,
	/* 08f */	IT_V,  CountClipboardFormats,
	/* 090 */	IT_1UI,  EnumClipboardFormats,
	/* 091 */	IT_1LP,  RegisterClipboardFormat,
	/* 092 */	IT_1UI1LP1I,  GetClipboardFormatName,
	/* 093 */	IT_1H,  SetClipboardViewer,
	/* 094 */	IT_V,  GetClipboardViewer,
	/* 095 */	IT_2H,  ChangeClipboardChain,
	/* 096 */	IT_1H1LP,  LoadMenu,
	/* 097 */	IT_V,  CreateMenu,
	/* 098 */	IT_1H,  DestroyMenu,
	/* 099 */	IT_1H1UI1LP2UI,  ChangeMenu,
	/* 09a */	IT_1H2UI,  CheckMenuItem,
	/* 09b */	IT_1H2UI,  EnableMenuItem,
	/* 09c */	IT_1H1I,  GetSystemMenu,
	/* 09d */	IT_1H,  GetMenu,
	/* 09e */	IT_2H,  SetMenu,
	/* 09f */	IT_1H1I,  GetSubMenu,
	/* 0a0 */	IT_1H,  DrawMenuBar,
	/* 0a1 */	IT_GETMENUSTRING,  GetMenuString,
	/* 0a2 */	IT_2H2UI,  HiliteMenuItem,
	/* 0a3 */	IT_2H2I,  CreateCaret,
	/* 0a4 */	IT_V,  DestroyCaret,
	/* 0a5 */	IT_2I,  SetCaretPos,
	/* 0a6 */	IT_1H,  HideCaret,
	/* 0a7 */	IT_1H,  ShowCaret,
	/* 0a8 */	IT_1UI,  SetCaretBlinkTime,
	/* 0a9 */	IT_V,  GetCaretBlinkTime,
	/* 0aa */	IT_1H, ArrangeIconicWindows,
	/* 0ab */	IT_WINHELP,  WinHelp,
	/* 0ac */	Trap, 0,
	/* 0ad */	IT_1H1LP,  LoadCursor,
	/* 0ae */	IT_1H1LP,  LoadIcon,
	/* 0af */	IT_1H1LP,  LoadBitmap,
	/* 0b0 */	IT_1H1UI1LP1I,  LoadString,
	/* 0b1 */	IT_LOADACCEL,  LoadAccelerators,
	/* 0b2 */	IT_TRANSLATEACCEL,  TranslateAccelerator,
	/* 0b3 */	IT_1I,  GetSystemMetrics,
	/* 0b4 */	IT_1I,  GetSysColor,
	/* 0b5 */	IT_SETSYSCOLORS,  SetSysColors,
	/* 0b6 */	IT_1H1UI, UserKillSystemTimer,
	/* 0b7 */	IT_1LPP,  GetCaretPos,
	/* 0b8 */	IT_QUERYSENDMSG,  QuerySendMessage,
	/* 0b9 */	IT_GRAYSTRING,  GrayString,
	/* 0ba */	IT_1I,  SwapMouseButton,
	/* 0bb */	Trap, 0,
	/* 0bc */	IT_1H, SetSysModalWindow,
	/* 0bd */	IT_V, GetSysModalWindow,
	/* 0be */	IT_1H1LPR1I,  GetUpdateRect,
	/* 0bf */	IT_CHWINFROMPT, ChildWindowFromPoint,
	/* 0c0 */	IT_V,  InSendMessage,
	/* 0c1 */	IT_1UI,  IsClipboardFormatAvailable,
	/* 0c2 */	IT_1H1LP1I,  DlgDirSelectComboBox,
	/* 0c3 */	IT_1H1LP2I1UI, DlgDirListComboBox,
	/* 0c4 */	IT_TABTEXTOUT, TabbedTextOut,
	/* 0c5 */	IT_GETTABBEDTEXTEXT, GetTabbedTextExtent,
	/* 0c6 */	IT_1H1W, CascadeChildWindows,
	/* 0c7 */	IT_1H1W, TileChildWindows,
	/* 0c8 */	IT_1LP2UI, OpenComm,
	/* 0c9 */	IT_1LPDCB, SetCommState,
	/* 0ca */	IT_1I1LPDCB, GetCommState,
	/* 0cb */	IT_1I1LPCOMSTAT, GetCommError,
	/* 0cc */	IT_1I1LPV1I, ReadComm,
	/* 0cd */	IT_1I1LPV1I, WriteComm,
	/* 0ce */	IT_2I, TransmitCommChar,
	/* 0cf */	IT_1I, CloseComm,
	/* 0d0 */	IT_1I1UI, SetCommEventMask,
	/* 0d1 */	IT_2I, GetCommEventMask,
	/* 0d2 */	IT_1I, SetCommBreak,
	/* 0d3 */	IT_1I, ClearCommBreak,
	/* 0d4 */	IT_2I, UngetCommChar,
#ifdef	LATER
	the second parameter here is NOT LPSTR!!!
#endif
	/* 0d5 */	IT_2LP, BuildCommDCB,
	/* 0d6 */	IT_2I, EscapeCommFunction,
	/* 0d7 */	IT_2I, FlushComm,
	/* 0d8 */	Trap, 0,
	/* 0d9 */	Trap, 0,
	/* 0da */	IT_DLGBOXIN,  DialogBoxIndirect,
	/* 0db */	IT_CREATEDLGIN,  CreateDialogIndirect,
	/* 0dc */	IT_LOADMENUIND, LoadMenuIndirect,
	/* 0dd */	IT_SCROLLDC,  ScrollDC,
	/* 0de */	IT_1LP,  GetKeyboardState,
	/* 0df */	IT_1LP,  SetKeyboardState,
	/* 0e0 */	IT_GETWINTASK,  GetWindowTask,
	/* 0e1 */	IT_ENUMTASKWIN,  EnumTaskWindows,
	/* 0e2 */	Trap, 0,
	/* 0e3 */	IT_2H1I,  GetNextDlgGroupItem,
	/* 0e4 */	IT_2H1I,  GetNextDlgTabItem,
	/* 0e5 */	IT_1H,  GetTopWindow,
	/* 0e6 */	IT_1H1UI,  GetNextWindow,
	/* 0e7 */	Trap, 0,
	/* 0e8 */	IT_2H4I1UI,  SetWindowPos,
	/* 0e9 */	IT_2H,  SetParent,
	/* 0ea */	IT_UNHOOKWHOOK, UnhookWindowsHook,
	/* 0eb */	IT_DEFHOOKPROC, DefHookProc,
	/* 0ec */	IT_V,  GetCapture,
	/* 0ed */	IT_2H1I,  GetUpdateRgn,
	/* 0ee */	IT_2H,  ExcludeUpdateRgn,
	/* 0ef */	IT_DLGPARAM,  DialogBoxParam,
	/* 0f0 */	IT_DLGBOXIP,  DialogBoxIndirectParam,
	/* 0f1 */	IT_DLGPARAM,  CreateDialogParam,
	/* 0f2 */	IT_CREATEDLGIP,  CreateDialogIndirectParam,
	/* 0f3 */	IT_V,  GetDialogBaseUnits,
	/* 0f4 */	IT_2LPRc,  EqualRect,
	/* 0f5 */	IT_1I1H2I, EnableCommNotification,
	/* 0f6 */	Trap, 0,
	/* 0f7 */	IT_V, GetCursor,
	/* 0f8 */	IT_V, GetOpenClipboardWindow,
	/* 0f9 */	IT_1I,  GetAsyncKeyState,
	/* 0fa */	IT_1H2UI,  GetMenuState,
	/* 0fb */	Trap, 0,
	/* 0fc */	Trap, 0,
	/* 0fd */	Trap, 0,
	/* 0fe */	Trap, 0,
	/* 0ff */	Trap, 0,
	/* 100 */	Trap, 0,
	/* 101 */	IT_1H1D, GetNextDriver,
	/* 102 */	IT_2H1LPP1UI, MapWindowPoints,
	/* 103 */	IT_1I,  BeginDeferWindowPos,
	/* 104 */	IT_3H4I1UI,  DeferWindowPos,
	/* 105 */	IT_1H,  EndDeferWindowPos,
	/* 106 */	IT_1H1UI,  GetWindow,
	/* 107 */	IT_1H,  GetMenuItemCount,
	/* 108 */	IT_1H1I,  GetMenuItemID,
	/* 109 */	IT_1H1I, ShowOwnedPopups,
	/* 10a */	IT_1I,  SetMessageQueue,
	/* 10b */	IT_1H2I,  ShowScrollBar,
	/* 10c */	IT_1LP,  GlobalAddAtom,
	/* 10d */	IT_1H,  GlobalDeleteAtom,
	/* 10e */	IT_1LP,  GlobalFindAtom,
	/* 10f */	IT_1H1LP1I,  GlobalGetAtomName,
	/* 110 */	IT_1H,  IsZoomed,
	/* 111 */	Trap, 0,
	/* 112 */	Trap, 0,
	/* 113 */	Trap, 0,
	/* 114 */	Trap, 0,
	/* 115 */	IT_1H,  GetDlgCtrlID,
	/* 116 */	IT_V, GetDesktopWindow,
	/* 117 */	Trap, 0,
	/* 118 */	Trap, 0,
	/* 119 */	Trap, 0,
	/* 11a */	IT_2H1I,  SelectPalette,
	/* 11b */	IT_1H,  RealizePalette,
	/* 11c */	_86_GetFreeSystemResources, 0,
	/* 11d */	Trap, 0,
	/* 11e */	IT_V,  GetDesktopWindow,
	/* 11f */	IT_1H, GetLastActivePopup,
	/* 120 */	Trap, 0,
	/* 121 */	Trap, 0,
	/* 122 */	IT_1H1LPRc1H1UI, RedrawWindow,
	/* 123 */	IT_SETWINDOWSHOOKEX, SetWindowsHookEx,
	/* 124 */	IT_1D, UnhookWindowsHookEx,
	/* 125 */	IT_CALLNEXTHOOKEX, CallNextHookEx,
	/* 126 */	Trap, 0,
	/* 127 */	Trap, 0,
	/* 128 */	Trap, 0,
	/* 129 */	Trap, 0,
	/* 12a */	Trap, 0,
	/* 12b */	Trap, 0,
	/* 12c */	Trap, 0,
	/* 12d */	Trap, 0,
	/* 12e */	Trap, 0,
	/* 12f */	Trap, 0,
	/* 130 */	Trap, 0,
	/* 131 */	Trap, 0,
	/* 132 */	Trap, 0,
	/* 133 */	Trap, 0,
	/* 134 */	IT_SENDMESSAGE,  DefDlgProc,
	/* 135 */	IT_1LPR, GetClipCursor,
	/* 136 */	Trap, 0,
	/* 137 */	Trap, 0,
	/* 138 */	Trap, 0,
	/* 139 */	Trap, 0,
	/* 13a */	Trap, 0,
	/* 13b */	Trap, 0,
	/* 13c */	Trap, 0,
	/* 13d */	Trap, 0,
	/* 13e */	Trap, 0,
	/* 13f */	IT_SCROLLWINEX, ScrollWindowEx,
	/* 140 */	Trap, 0,
	/* 141 */	Trap, 0,
	/* 142 */	Trap, 0,
	/* 143 */	Trap, 0,
	/* 144 */	Trap, 0,
	/* 145 */	Trap, 0,
	/* 146 */	Trap, 0,
	/* 147 */	Trap, 0,
	/* 148 */	Trap, 0,
	/* 149 */	Trap, 0,
	/* 14a */	Trap, 0,
	/* 14b */	Trap, 0,
	/* 14c */	Trap, 0,
	/* 14d */	Trap, 0,
	/* 14e */	IT_1UI, GetQueueStatus,
	/* 14f */	IT_V, GetInputState,
	/* 150 */	Trap, 0,
	/* 151 */	Trap, 0,
	/* 152 */	Trap, 0,
	/* 153 */	Trap, 0,
	/* 154 */	Trap, 0,
	/* 155 */	Trap, 0,
	/* 156 */	Trap, 0,
	/* 157 */	Trap, 0,
	/* 158 */	Trap, 0,
	/* 159 */	Trap, 0,
	/* 15a */	Trap, 0,
	/* 15b */	Trap, 0,
	/* 15c */	Trap, 0,
	/* 15d */	Trap, 0,
	/* 15e */	Trap, 0,
	/* 15f */	Trap, 0,
	/* 160 */	Trap, 0,
	/* 161 */	Trap, 0,
	/* 162 */	Trap, 0,
	/* 163 */	Trap, 0,
	/* 164 */	Trap, 0,
	/* 165 */	Trap, 0,
	/* 166 */	IT_1H,  IsMenu,
	/* 167 */	IT_2H1D, GetDCEx,
	/* 168 */	Trap, 0,
	/* 169 */	Trap, 0,
	/* 16a */	Trap, 0,
	/* 16b */	Trap, 0,
	/* 16c */	Trap, 0,
	/* 16d */	Trap, 0,
	/* 16e */	Trap, 0,
	/* 16f */	Trap, 0,
	/* 170 */	Trap, 0,
	/* 171 */	Trap, 0,
	/* 172 */	IT_1H1LPWP, GetWindowPlacement,
	/* 173 */	IT_1H1LPWPc, SetWindowPlacement,
	/* 174 */	Trap, 0,
	/* 175 */	IT_1LPR2LPRc,  SubtractRect,
	/* 176 */	Trap, 0,
	/* 177 */	Trap, 0,
	/* 178 */	Trap, 0,
	/* 179 */	Trap, 0,
	/* 17a */	Trap, 0,
	/* 17b */	Trap, 0,
	/* 17c */	Trap, 0,
	/* 17d */	Trap, 0,
	/* 17e */	Trap, 0,
	/* 17f */	Trap, 0,
	/* 180 */	Trap, 0,
	/* 181 */	Trap, 0,
	/* 182 */	Trap, 0,
	/* 183 */	Trap, 0,
	/* 184 */	Trap, 0,
	/* 185 */	Trap, 0,
	/* 186 */	Trap, 0,
	/* 187 */	Trap, 0,
	/* 188 */	Trap, 0,
	/* 189 */	Trap, 0,
	/* 18a */	Trap, 0,
	/* 18b */	Trap, 0,
	/* 18c */	Trap, 0,
	/* 18d */	Trap, 0,
	/* 18e */	Trap, 0,
	/* 18f */	Trap, 0,
	/* 190 */	Trap, 0,
	/* 191 */	Trap, 0,
	/* 192 */	IT_1LPUI1I, GetPriorityClipboardFormat,
	/* 193 */	IT_1LP1H,  UnregisterClass,
	/* 194 */	IT_GETCLASSINFO,  GetClassInfo,
	/* 195 */	Trap, 0,
	/* 196 */	IT_1H4I2LPB, CreateCursor,
	/* 197 */	IT_1H2I2B2LPB, CreateIcon,
	/* 198 */	Trap, 0,
	/* 199 */	Trap, 0,
	/* 19a */	IT_INSERTMENU,  InsertMenu,
	/* 19b */	IT_APPENDMENU,  AppendMenu,
	/* 19c */	IT_1H2UI,  RemoveMenu,
	/* 19d */	IT_1H2UI,  DeleteMenu,
	/* 19e */	IT_INSERTMENU,  ModifyMenu,
	/* 19f */	IT_V,  CreatePopupMenu,
	/* 1a0 */	IT_1H1UI3I1H1LPRc,  TrackPopupMenu,
	/* 1a1 */	IT_V,  GetMenuCheckMarkDimensions,
	/* 1a2 */	IT_1H2UI2H,  SetMenuItemBitmaps,
	/* 1a3 */	Trap, 0,
	/* 1a4 */	IT_WSPRINTF,  wsprintf,
	/* 1a5 */	IT_WVSPRINTF,  wvsprintf,
	/* 1a6 */	Trap, 0,
	/* 1a7 */	Trap, 0,
	/* 1a8 */	Trap, 0,
	/* 1a9 */	Trap, 0,
	/* 1aa */	Trap, 0,
	/* 1ab */	Trap, 0,
	/* 1ac */	Trap, 0,
	/* 1ad */	Trap, 0,
	/* 1ae */	IT_2LP,  lstrcmp,
	/* 1af */	IT_LP1LP,  AnsiUpper,
	/* 1b0 */	IT_LP1LP,  AnsiLower,
	/* 1b1 */	IT_1B,  IsCharAlpha,
	/* 1b2 */	IT_1B,  IsCharAlphaNumeric,
	/* 1b3 */	IT_1B,  IsCharUpper,
	/* 1b4 */	IT_1B,  IsCharLower,
	/* 1b5 */	IT_1LP1UI,  AnsiUpperBuff,
	/* 1b6 */	IT_1LP1UI,  AnsiLowerBuff,
	/* 1b7 */	Trap, 0,
	/* 1b8 */	Trap, 0,
	/* 1b9 */	Trap, 0,
	/* 1ba */	Trap, 0,
	/* 1bb */	Trap, 0,
	/* 1bc */	Trap, 0,
	/* 1bd */	IT_DEFFRAMEPROC, DefFrameProc,
	/* 1be */	Trap, 0,
	/* 1bf */	IT_SENDMESSAGE, DefMDIChildProc,
	/* 1c0 */	Trap, 0,
	/* 1c1 */	Trap, 0,
	/* 1c2 */	Trap, 0,
	/* 1c3 */	IT_1H1LP, TranslateMDISysAccel,
	/* 1c4 */	IT_CREATEWINEX,  CreateWindowEx,
	/* 1c5 */	Trap, 0,
	/* 1c6 */	IT_1LPR1D1I1D,  AdjustWindowRectEx,
	/* 1c7 */	Trap, 0,
	/* 1c8 */	Trap, 0,
	/* 1c9 */	IT_1H, DestroyIcon,
	/* 1ca */	IT_1H, DestroyCursor,
	/* 1cb */	Trap, 0,
	/* 1cc */	Trap, 0,
	/* 1cd */	Trap, 0,
	/* 1ce */	IT_CALCCHILDSCROLL, CalcChildScroll,
	/* 1cf */	Trap, 0,
	/* 1d0 */	Trap, 0,
	/* 1d1 */	Trap, 0,
	/* 1d2 */	IT_1H1LPRc, DrawFocusRect,
	/* 1d3 */	Trap, 0,
	/* 1d4 */	Trap, 0,
	/* 1d5 */	Trap, 0,
	/* 1d6 */	Trap, 0,
	/* 1d7 */	IT_2LP,  lstrcmpi,
	/* 1d8 */	IT_LP1LP,  AnsiNext,
	/* 1d9 */	IT_LP2LP,  AnsiPrev,
	/* 1da */	Trap, 0,
	/* 1db */	Trap, 0,
	/* 1dc */	Trap, 0,
	/* 1dd */	Trap, 0,
	/* 1de */	Trap, 0,
	/* 1df */	Trap, 0,
	/* 1e0 */	Trap, 0,
	/* 1e1 */	Trap, 0,
	/* 1e2 */	IT_1H1I1UI, EnableScrollBar,
	/* 1e3 */	IT_SYSPARAMINFO, SystemParametersInfo,
	/* 1e4 */	Trap, 0,
	/* 1e5 */	Trap, 0,
	/* 1e6 */	Trap, 0,
	/* 1e7 */	Trap, 0,
	/* 1e8 */	Trap, 0,
	/* 1e9 */	Trap, 0,
	/* 1ea */	Trap, 0,
	/* 1eb */	Trap, 0,
	/* 1ec */	Trap, 0,
	/* 1ed */	Trap, 0,
	/* 1ee */	Trap, 0,
	/* 1ef */	Trap, 0,
	/* 1f0 */	Trap, 0,
	/* 1f1 */	Trap, 0,
	/* 1f2 */	Trap, 0,
	/* 1f3 */	Trap, 0,
	/* 1f4 */	Trap, 0,
	/* 1f5 */	Trap, 0,
	/* 1f6 */	Trap, 0,
	/* 1f7 */	Trap, 0,
	/* 1f8 */	Trap, 0,
	/* 1f9 */	Trap, 0,
	/* 1fa */	Trap, 0,
	/* 1fb */	Trap, 0,
	/* 1fc */	Trap, 0,
	/* 1fd */	Trap, 0,
	/* 1fe */	Trap, 0,
	/* 1ff */	Trap, 0,
	/* 200 */	IT_WNETGETCONN, WNetGetConnection,
	/* 201 */	IT_1UI, WNetGetCaps,
	/* 202 */	Trap, 0,
	/* 203 */	Trap, 0,
	/* 204 */	IT_WNETGETUSER, WNetGetUser,
	/* 205 */	Trap, 0,
	/* 206 */	Trap, 0,
	/* 207 */	Trap, 0,
	/* 208 */	Trap, 0,
	/* 209 */	Trap, 0,
	/* 20a */	Trap, 0,
	/* 20b */	Trap, 0,
	/* 20c */	Trap, 0,
	/* 20d */	Trap, 0,
	/* 20e */	Trap, 0,
	/* 20f */	Trap, 0,
	/* 210 */	Trap, 0,
	/* 211 */	Trap, 0,
	/* 212 */	Trap, 0,
	/* 213 */	Trap, 0,
	/* 214 */	Trap, 0,
	0,0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
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
	/* 014 */	Trap, 0,
	/* 015 */	Trap, 0,
	/* 016 */	Trap, 0,
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
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	Trap, 0,
	/* 025 */	Trap, 0,
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
	/* 066 */	Trap, 0,
	/* 067 */	Trap, 0,
	/* 068 */	Trap, 0,
	/* 069 */	Trap, 0,
	/* 06a */	Trap, 0,
	/* 06b */	Trap, 0,
	/* 06c */	Trap, 0,
	/* 06d */	Trap, 0,
	/* 06e */	Trap, 0,
	/* 06f */	Trap, 0,
	/* 070 */	Trap, 0,
	/* 071 */	Trap, 0,
	/* 072 */	Trap, 0,
	/* 073 */	Trap, 0,
	/* 074 */	Trap, 0,
	/* 075 */	Trap, 0,
	/* 076 */	Trap, 0,
	/* 077 */	Trap, 0,
	/* 078 */	Trap, 0,
	/* 079 */	Trap, 0,
	/* 07a */	Trap, 0,
	/* 07b */	Trap, 0,
	/* 07c */	Trap, 0,
	/* 07d */	Trap, 0,
	/* 07e */	Trap, 0,
	/* 07f */	Trap, 0,
	/* 080 */	Trap, 0,
	/* 081 */	Trap, 0,
	/* 082 */	Trap, 0,
	/* 083 */	Trap, 0,
	/* 084 */	Trap, 0,
	/* 085 */	Trap, 0,
	/* 086 */	Trap, 0,
	/* 087 */	Trap, 0,
	/* 088 */	Trap, 0,
	/* 089 */	Trap, 0,
	/* 08a */	Trap, 0,
	/* 08b */	Trap, 0,
	/* 08c */	Trap, 0,
	/* 08d */	Trap, 0,
	/* 08e */	Trap, 0,
	/* 08f */	Trap, 0,
	/* 090 */	Trap, 0,
	/* 091 */	Trap, 0,
	/* 092 */	Trap, 0,
	/* 093 */	Trap, 0,
	/* 094 */	Trap, 0,
	/* 095 */	Trap, 0,
	/* 096 */	Trap, 0,
	/* 097 */	Trap, 0,
	/* 098 */	Trap, 0,
	/* 099 */	Trap, 0,
	/* 09a */	Trap, 0,
	/* 09b */	Trap, 0,
	/* 09c */	Trap, 0,
	/* 09d */	Trap, 0,
	/* 09e */	Trap, 0,
	/* 09f */	Trap, 0,
	/* 0a0 */	Trap, 0,
	/* 0a1 */	Trap, 0,
	/* 0a2 */	Trap, 0,
	/* 0a3 */	Trap, 0,
	/* 0a4 */	Trap, 0,
	/* 0a5 */	Trap, 0,
	/* 0a6 */	Trap, 0,
	/* 0a7 */	Trap, 0,
	/* 0a8 */	Trap, 0,
	/* 0a9 */	Trap, 0,
	/* 0aa */	Trap, 0,
	/* 0ab */	Trap, 0,
	/* 0ac */	Trap, 0,
	/* 0ad */	Trap, 0,
	/* 0ae */	Trap, 0,
	/* 0af */	Trap, 0,
	/* 0b0 */	Trap, 0,
	/* 0b1 */	Trap, 0,
	/* 0b2 */	Trap, 0,
	/* 0b3 */	Trap, 0,
	/* 0b4 */	Trap, 0,
	/* 0b5 */	Trap, 0,
	/* 0b6 */	Trap, 0,
	/* 0b7 */	Trap, 0,
	/* 0b8 */	Trap, 0,
	/* 0b9 */	Trap, 0,
	/* 0ba */	Trap, 0,
	/* 0bb */	Trap, 0,
	/* 0bc */	Trap, 0,
	/* 0bd */	Trap, 0,
	/* 0be */	Trap, 0,
	/* 0bf */	Trap, 0,
	/* 0c0 */	Trap, 0,
	/* 0c1 */	Trap, 0,
	/* 0c2 */	Trap, 0,
	/* 0c3 */	Trap, 0,
	/* 0c4 */	Trap, 0,
	/* 0c5 */	Trap, 0,
	/* 0c6 */	Trap, 0,
	/* 0c7 */	Trap, 0,
	/* 0c8 */	Trap, 0,
	/* 0c9 */	Trap, 0,
	/* 0ca */	Trap, 0,
	/* 0cb */	Trap, 0,
	/* 0cc */	Trap, 0,
	/* 0cd */	Trap, 0,
	/* 0ce */	Trap, 0,
	/* 0cf */	Trap, 0,
	/* 0d0 */	Trap, 0,
	/* 0d1 */	Trap, 0,
	/* 0d2 */	Trap, 0,
	/* 0d3 */	Trap, 0,
	/* 0d4 */	Trap, 0,
	/* 0d5 */	Trap, 0,
	/* 0d6 */	Trap, 0,
	/* 0d7 */	Trap, 0,
	/* 0d8 */	Trap, 0,
	/* 0d9 */	Trap, 0,
	/* 0da */	Trap, 0,
	/* 0db */	Trap, 0,
	/* 0dc */	Trap, 0,
	/* 0dd */	Trap, 0,
	/* 0de */	Trap, 0,
	/* 0df */	Trap, 0,
	/* 0e0 */	Trap, 0,
	/* 0e1 */	Trap, 0,
	/* 0e2 */	Trap, 0,
	/* 0e3 */	Trap, 0,
	/* 0e4 */	Trap, 0,
	/* 0e5 */	Trap, 0,
	/* 0e6 */	Trap, 0,
	/* 0e7 */	Trap, 0,
	/* 0e8 */	Trap, 0,
	/* 0e9 */	Trap, 0,
	/* 0ea */	Trap, 0,
	/* 0eb */	Trap, 0,
	/* 0ec */	Trap, 0,
	/* 0ed */	Trap, 0,
	/* 0ee */	Trap, 0,
	/* 0ef */	Trap, 0,
	/* 0f0 */	Trap, 0,
	/* 0f1 */	Trap, 0,
	/* 0f2 */	Trap, 0,
	/* 0f3 */	Trap, 0,
	/* 0f4 */	Trap, 0,
	/* 0f5 */	Trap, 0,
	/* 0f6 */	Trap, 0,
	/* 0f7 */	Trap, 0,
	/* 0f8 */	Trap, 0,
	/* 0f9 */	Trap, 0,
	/* 0fa */	Trap, 0,
	/* 0fb */	Trap, 0,
	/* 0fc */	Trap, 0,
	/* 0fd */	Trap, 0,
	/* 0fe */	Trap, 0,
	/* 0ff */	Trap, 0,
	/* 100 */	Trap, 0,
	/* 101 */	Trap, 0,
	/* 102 */	Trap, 0,
	/* 103 */	Trap, 0,
	/* 104 */	Trap, 0,
	/* 105 */	Trap, 0,
	/* 106 */	Trap, 0,
	/* 107 */	Trap, 0,
	/* 108 */	Trap, 0,
	/* 109 */	Trap, 0,
	/* 10a */	Trap, 0,
	/* 10b */	Trap, 0,
	/* 10c */	Trap, 0,
	/* 10d */	Trap, 0,
	/* 10e */	Trap, 0,
	/* 10f */	Trap, 0,
	/* 110 */	Trap, 0,
	/* 111 */	Trap, 0,
	/* 112 */	Trap, 0,
	/* 113 */	Trap, 0,
	/* 114 */	Trap, 0,
	/* 115 */	Trap, 0,
	/* 116 */	Trap, 0,
	/* 117 */	Trap, 0,
	/* 118 */	Trap, 0,
	/* 119 */	Trap, 0,
	/* 11a */	Trap, 0,
	/* 11b */	Trap, 0,
	/* 11c */	Trap, 0,
	/* 11d */	Trap, 0,
	/* 11e */	Trap, 0,
	/* 11f */	Trap, 0,
	/* 120 */	Trap, 0,
	/* 121 */	Trap, 0,
	/* 122 */	Trap, 0,
	/* 123 */	Trap, 0,
	/* 124 */	Trap, 0,
	/* 125 */	Trap, 0,
	/* 126 */	Trap, 0,
	/* 127 */	Trap, 0,
	/* 128 */	Trap, 0,
	/* 129 */	Trap, 0,
	/* 12a */	Trap, 0,
	/* 12b */	Trap, 0,
	/* 12c */	Trap, 0,
	/* 12d */	Trap, 0,
	/* 12e */	Trap, 0,
	/* 12f */	Trap, 0,
	/* 130 */	Trap, 0,
	/* 131 */	Trap, 0,
	/* 132 */	Trap, 0,
	/* 133 */	Trap, 0,
	/* 134 */	Trap, 0,
	/* 135 */	Trap, 0,
	/* 136 */	Trap, 0,
	/* 137 */	Trap, 0,
	/* 138 */	Trap, 0,
	/* 139 */	Trap, 0,
	/* 13a */	Trap, 0,
	/* 13b */	Trap, 0,
	/* 13c */	Trap, 0,
	/* 13d */	Trap, 0,
	/* 13e */	Trap, 0,
	/* 13f */	Trap, 0,
	/* 140 */	Trap, 0,
	/* 141 */	Trap, 0,
	/* 142 */	Trap, 0,
	/* 143 */	Trap, 0,
	/* 144 */	Trap, 0,
	/* 145 */	Trap, 0,
	/* 146 */	Trap, 0,
	/* 147 */	Trap, 0,
	/* 148 */	Trap, 0,
	/* 149 */	Trap, 0,
	/* 14a */	Trap, 0,
	/* 14b */	Trap, 0,
	/* 14c */	Trap, 0,
	/* 14d */	Trap, 0,
	/* 14e */	Trap, 0,
	/* 14f */	Trap, 0,
	/* 150 */	Trap, 0,
	/* 151 */	Trap, 0,
	/* 152 */	Trap, 0,
	/* 153 */	Trap, 0,
	/* 154 */	Trap, 0,
	/* 155 */	Trap, 0,
	/* 156 */	Trap, 0,
	/* 157 */	Trap, 0,
	/* 158 */	Trap, 0,
	/* 159 */	Trap, 0,
	/* 15a */	Trap, 0,
	/* 15b */	Trap, 0,
	/* 15c */	Trap, 0,
	/* 15d */	Trap, 0,
	/* 15e */	Trap, 0,
	/* 15f */	Trap, 0,
	/* 160 */	Trap, 0,
	/* 161 */	Trap, 0,
	/* 162 */	Trap, 0,
	/* 163 */	Trap, 0,
	/* 164 */	Trap, 0,
	/* 165 */	Trap, 0,
	/* 166 */	Trap, 0,
	/* 167 */	Trap, 0,
	/* 168 */	Trap, 0,
	/* 169 */	Trap, 0,
	/* 16a */	Trap, 0,
	/* 16b */	Trap, 0,
	/* 16c */	Trap, 0,
	/* 16d */	Trap, 0,
	/* 16e */	Trap, 0,
	/* 16f */	Trap, 0,
	/* 170 */	Trap, 0,
	/* 171 */	Trap, 0,
	/* 172 */	Trap, 0,
	/* 173 */	Trap, 0,
	/* 174 */	Trap, 0,
	/* 175 */	Trap, 0,
	/* 176 */	Trap, 0,
	/* 177 */	Trap, 0,
	/* 178 */	Trap, 0,
	/* 179 */	Trap, 0,
	/* 17a */	Trap, 0,
	/* 17b */	Trap, 0,
	/* 17c */	Trap, 0,
	/* 17d */	Trap, 0,
	/* 17e */	Trap, 0,
	/* 17f */	Trap, 0,
	/* 180 */	Trap, 0,
	/* 181 */	Trap, 0,
	/* 182 */	Trap, 0,
	/* 183 */	Trap, 0,
	/* 184 */	Trap, 0,
	/* 185 */	Trap, 0,
	/* 186 */	Trap, 0,
	/* 187 */	Trap, 0,
	/* 188 */	Trap, 0,
	/* 189 */	Trap, 0,
	/* 18a */	Trap, 0,
	/* 18b */	Trap, 0,
	/* 18c */	Trap, 0,
	/* 18d */	Trap, 0,
	/* 18e */	Trap, 0,
	/* 18f */	Trap, 0,
	/* 190 */	Trap, 0,
	/* 191 */	Trap, 0,
	/* 192 */	Trap, 0,
	/* 193 */	Trap, 0,
	/* 194 */	Trap, 0,
	/* 195 */	Trap, 0,
	/* 196 */	Trap, 0,
	/* 197 */	Trap, 0,
	/* 198 */	Trap, 0,
	/* 199 */	Trap, 0,
	/* 19a */	Trap, 0,
	/* 19b */	Trap, 0,
	/* 19c */	Trap, 0,
	/* 19d */	Trap, 0,
	/* 19e */	Trap, 0,
	/* 19f */	Trap, 0,
	/* 1a0 */	Trap, 0,
	/* 1a1 */	Trap, 0,
	/* 1a2 */	Trap, 0,
	/* 1a3 */	Trap, 0,
	/* 1a4 */	Trap, 0,
	/* 1a5 */	Trap, 0,
	/* 1a6 */	Trap, 0,
	/* 1a7 */	Trap, 0,
	/* 1a8 */	Trap, 0,
	/* 1a9 */	Trap, 0,
	/* 1aa */	Trap, 0,
	/* 1ab */	Trap, 0,
	/* 1ac */	Trap, 0,
	/* 1ad */	Trap, 0,
	/* 1ae */	Trap, 0,
	/* 1af */	Trap, 0,
	/* 1b0 */	Trap, 0,
	/* 1b1 */	Trap, 0,
	/* 1b2 */	Trap, 0,
	/* 1b3 */	Trap, 0,
	/* 1b4 */	Trap, 0,
	/* 1b5 */	Trap, 0,
	/* 1b6 */	Trap, 0,
	/* 1b7 */	Trap, 0,
	/* 1b8 */	Trap, 0,
	/* 1b9 */	Trap, 0,
	/* 1ba */	Trap, 0,
	/* 1bb */	Trap, 0,
	/* 1bc */	Trap, 0,
	/* 1bd */	Trap, 0,
	/* 1be */	Trap, 0,
	/* 1bf */	Trap, 0,
	/* 1c0 */	Trap, 0,
	/* 1c1 */	Trap, 0,
	/* 1c2 */	Trap, 0,
	/* 1c3 */	Trap, 0,
	/* 1c4 */	Trap, 0,
	/* 1c5 */	Trap, 0,
	/* 1c6 */	Trap, 0,
	/* 1c7 */	Trap, 0,
	/* 1c8 */	Trap, 0,
	/* 1c9 */	Trap, 0,
	/* 1ca */	Trap, 0,
	/* 1cb */	Trap, 0,
	/* 1cc */	Trap, 0,
	/* 1cd */	Trap, 0,
	/* 1ce */	Trap, 0,
	/* 1cf */	Trap, 0,
	/* 1d0 */	Trap, 0,
	/* 1d1 */	Trap, 0,
	/* 1d2 */	Trap, 0,
	/* 1d3 */	Trap, 0,
	/* 1d4 */	Trap, 0,
	/* 1d5 */	Trap, 0,
	/* 1d6 */	Trap, 0,
	/* 1d7 */	Trap, 0,
	/* 1d8 */	Trap, 0,
	/* 1d9 */	Trap, 0,
	/* 1da */	Trap, 0,
	/* 1db */	Trap, 0,
	/* 1dc */	Trap, 0,
	/* 1dd */	Trap, 0,
	/* 1de */	Trap, 0,
	/* 1df */	Trap, 0,
	/* 1e0 */	Trap, 0,
	/* 1e1 */	Trap, 0,
	/* 1e2 */	Trap, 0,
	/* 1e3 */	Trap, 0,
	/* 1e4 */	Trap, 0,
	/* 1e5 */	Trap, 0,
	/* 1e6 */	Trap, 0,
	/* 1e7 */	Trap, 0,
	/* 1e8 */	Trap, 0,
	/* 1e9 */	Trap, 0,
	/* 1ea */	Trap, 0,
	/* 1eb */	Trap, 0,
	/* 1ec */	Trap, 0,
	/* 1ed */	Trap, 0,
	/* 1ee */	Trap, 0,
	/* 1ef */	Trap, 0,
	/* 1f0 */	Trap, 0,
	/* 1f1 */	Trap, 0,
	/* 1f2 */	Trap, 0,
	/* 1f3 */	Trap, 0,
	/* 1f4 */	Trap, 0,
	/* 1f5 */	Trap, 0,
	/* 1f6 */	Trap, 0,
	/* 1f7 */	Trap, 0,
	/* 1f8 */	Trap, 0,
	/* 1f9 */	Trap, 0,
	/* 1fa */	Trap, 0,
	/* 1fb */	Trap, 0,
	/* 1fc */	Trap, 0,
	/* 1fd */	Trap, 0,
	/* 1fe */	Trap, 0,
	/* 1ff */	Trap, 0,
	/* 200 */	Trap, 0,
	/* 201 */	Trap, 0,
	/* 202 */	Trap, 0,
	/* 203 */	Trap, 0,
	/* 204 */	Trap, 0,
	/* 205 */	Trap, 0,
	/* 206 */	Trap, 0,
	/* 207 */	Trap, 0,
	/* 208 */	Trap, 0,
	/* 209 */	Trap, 0,
	/* 20a */	Trap, 0,
	/* 20b */	Trap, 0,
	/* 20c */	Trap, 0,
	/* 20d */	Trap, 0,
	/* 20e */	Trap, 0,
	/* 20f */	Trap, 0,
	/* 210 */	Trap, 0,
	/* 211 */	Trap, 0,
	/* 212 */	Trap, 0,
	/* 213 */	Trap, 0,
	/* 214 */	Trap, 0,
	0,0
};
#endif

 /* Segment Table USER: */

static SEGTAB seg_tab_USER[] =
{	{ (char *) seg_image_USER_0, 4144, TRANSFER_CALLBACK, 4144, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

extern TYPEINFO hsmt_resource_USER[1];

 /* Module Descriptor for USER: */

static MODULEDSCR hsmt_mod_dscr_USER =
{	"USER",
	entry_tab_USER,
	seg_tab_USER,
	(long *)hsmt_resource_USER
};
extern long int OleRegisterClientDoc();
extern long int OleRenameClientDoc();
extern long int OleRevokeClientDoc();
extern long int OleRevertClientDoc();
extern long int OleSavedClientDoc();
extern long int OleQueryClientVersion();
extern long int OleQueryLinkFromClip();
extern long int OleQueryCreateFromClip();
extern long int OleLoadFromStream();
extern long int OleEnumObjects();

 /* Entry Table OLECLI: */

static ENTRYTAB entry_tab_OLECLI[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "OLECLI", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "OLEDELETE", 0x0008, 0x0010, 0 },
	/* 003 */	{ "OLESAVETOSTREAM", 0x0008, 0x0018, 0 },
	/* 004 */	{ "OLELOADFROMSTREAM", 0x0008, 0x0020, OleLoadFromStream },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "OLECLONE", 0x0008, 0x0030, 0 },
	/* 007 */	{ "OLECOPYFROMLINK", 0x0008, 0x0038, 0 },
	/* 008 */	{ "OLEEQUAL", 0x0008, 0x0040, 0 },
	/* 009 */	{ "OLEQUERYLINKFROMCLIP", 0x0008, 0x0048, OleQueryLinkFromClip },
	/* 00a */	{ "OLEQUERYCREATEFROMCLIP", 0x0008, 0x0050, OleQueryCreateFromClip },
	/* 00b */	{ "OLECREATELINKFROMCLIP", 0x0008, 0x0058, 0 },
	/* 00c */	{ "OLECREATEFROMCLIP", 0x0008, 0x0060, 0 },
	/* 00d */	{ "OLECOPYTOCLIPBOARD", 0x0008, 0x0068, 0 },
	/* 00e */	{ "OLEQUERYTYPE", 0x0008, 0x0070, 0 },
	/* 00f */	{ "OLESETHOSTNAMES", 0x0008, 0x0078, 0 },
	/* 010 */	{ "OLESETTARGETDEVICE", 0x0008, 0x0080, 0 },
	/* 011 */	{ "OLESETBOUNDS", 0x0008, 0x0088, 0 },
	/* 012 */	{ "OLEQUERYBOUNDS", 0x0008, 0x0090, 0 },
	/* 013 */	{ "OLEDRAW", 0x0008, 0x0098, 0 },
	/* 014 */	{ "OLEQUERYOPEN", 0x0008, 0x00a0, 0 },
	/* 015 */	{ "OLEACTIVATE", 0x0008, 0x00a8, 0 },
	/* 016 */	{ "OLEUPDATE", 0x0008, 0x00b0, 0 },
	/* 017 */	{ "OLERECONNECT", 0x0008, 0x00b8, 0 },
	/* 018 */	{ "OLEGETLINKUPDATEOPTIONS", 0x0008, 0x00c0, 0 },
	/* 019 */	{ "OLESETLINKUPDATEOPTIONS", 0x0008, 0x00c8, 0 },
	/* 01a */	{ "OLEENUMFORMATS", 0x0008, 0x00d0, 0 },
	/* 01b */	{ "OLECLOSE", 0x0008, 0x00d8, 0 },
	/* 01c */	{ "OLEGETDATA", 0x0008, 0x00e0, 0 },
	/* 01d */	{ "OLESETDATA", 0x0008, 0x00e8, 0 },
	/* 01e */	{ "OLEQUERYPROTOCOL", 0x0008, 0x00f0, 0 },
	/* 01f */	{ "OLEQUERYOUTOFDATE", 0x0008, 0x00f8, 0 },
	/* 020 */	{ "OLEOBJECTCONVERT", 0x0008, 0x0100, 0 },
	/* 021 */	{ "OLECREATEFROMTEMPLATE", 0x0008, 0x0108, 0 },
	/* 022 */	{ "OLECREATE", 0x0008, 0x0110, 0 },
	/* 023 */	{ "OLEQUERYRELEASESTATUS", 0x0008, 0x0118, 0 },
	/* 024 */	{ "OLEQUERYRELEASEERROR", 0x0008, 0x0120, 0 },
	/* 025 */	{ "OLEQUERYRELEASEMETHOD", 0x0008, 0x0128, 0 },
	/* 026 */	{ "OLECREATEFROMFILE", 0x0008, 0x0130, 0 },
	/* 027 */	{ "OLECREATELINKFROMFILE", 0x0008, 0x0138, 0 },
	/* 028 */	{ "OLERELEASE", 0x0008, 0x0140, 0 },
	/* 029 */	{ "OLEREGISTERCLIENTDOC", 0x0008, 0x0148, OleRegisterClientDoc },
	/* 02a */	{ "OLEREVOKECLIENTDOC", 0x0008, 0x0150, OleRevokeClientDoc },
	/* 02b */	{ "OLERENAMECLIENTDOC", 0x0008, 0x0158, OleRenameClientDoc },
	/* 02c */	{ "OLEREVERTCLIENTDOC", 0x0008, 0x0160, OleRevertClientDoc },
	/* 02d */	{ "OLESAVEDCLIENTDOC", 0x0008, 0x0168, OleSavedClientDoc },
	/* 02e */	{ "OLERENAME", 0x0008, 0x0170, 0 },
	/* 02f */	{ "OLEENUMOBJECTS", 0x0008, 0x0178, OleEnumObjects },
	/* 030 */	{ "OLEQUERYNAME", 0x0008, 0x0180, 0 },
	/* 031 */	{ "OLESETCOLORSCHEME", 0x0008, 0x0188, 0 },
	/* 032 */	{ "OLEREQUESTDATA", 0x0008, 0x0190, 0 },
	/* 033 */	{ "", 0, 0, 0 },
	/* 034 */	{ "", 0, 0, 0 },
	/* 035 */	{ "", 0, 0, 0 },
	/* 036 */	{ "OLELOCKSERVER", 0x0008, 0x01b0, 0 },
	/* 037 */	{ "OLEUNLOCKSERVER", 0x0008, 0x01b8, 0 },
	/* 038 */	{ "OLEQUERYSIZE", 0x0008, 0x01c0, 0 },
	/* 039 */	{ "OLEEXECUTE", 0x0008, 0x01c8, 0 },
	/* 03a */	{ "OLECREATEINVISIBLE", 0x0008, 0x01d0, 0 },
	/* 03b */	{ "OLEQUERYCLIENTVERSION", 0x0008, 0x01d8, OleQueryClientVersion },
	/* 03c */	{ "OLEISDCMETA", 0x0008, 0x01e0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "", 0, 0, 0 },
	/* 00b */	{ "", 0, 0, 0 },
	/* 00c */	{ "", 0, 0, 0 },
	/* 00d */	{ "", 0, 0, 0 },
	/* 00e */	{ "", 0, 0, 0 },
	/* 00f */	{ "", 0, 0, 0 },
	/* 010 */	{ "", 0, 0, 0 },
	/* 011 */	{ "", 0, 0, 0 },
	/* 012 */	{ "", 0, 0, 0 },
	/* 013 */	{ "", 0, 0, 0 },
	/* 014 */	{ "", 0, 0, 0 },
	/* 015 */	{ "", 0, 0, 0 },
	/* 016 */	{ "", 0, 0, 0 },
	/* 017 */	{ "", 0, 0, 0 },
	/* 018 */	{ "", 0, 0, 0 },
	/* 019 */	{ "", 0, 0, 0 },
	/* 01a */	{ "", 0, 0, 0 },
	/* 01b */	{ "", 0, 0, 0 },
	/* 01c */	{ "", 0, 0, 0 },
	/* 01d */	{ "", 0, 0, 0 },
	/* 01e */	{ "", 0, 0, 0 },
	/* 01f */	{ "", 0, 0, 0 },
	/* 020 */	{ "", 0, 0, 0 },
	/* 021 */	{ "", 0, 0, 0 },
	/* 022 */	{ "", 0, 0, 0 },
	/* 023 */	{ "", 0, 0, 0 },
	/* 024 */	{ "", 0, 0, 0 },
	/* 025 */	{ "", 0, 0, 0 },
	/* 026 */	{ "", 0, 0, 0 },
	/* 027 */	{ "", 0, 0, 0 },
	/* 028 */	{ "", 0, 0, 0 },
	/* 029 */	{ "", 0, 0, 0 },
	/* 02a */	{ "", 0, 0, 0 },
	/* 02b */	{ "", 0, 0, 0 },
	/* 02c */	{ "", 0, 0, 0 },
	/* 02d */	{ "", 0, 0, 0 },
	/* 02e */	{ "", 0, 0, 0 },
	/* 02f */	{ "", 0, 0, 0 },
	/* 030 */	{ "", 0, 0, 0 },
	/* 031 */	{ "", 0, 0, 0 },
	/* 032 */	{ "", 0, 0, 0 },
	/* 033 */	{ "", 0, 0, 0 },
	/* 034 */	{ "", 0, 0, 0 },
	/* 035 */	{ "", 0, 0, 0 },
	/* 036 */	{ "", 0, 0, 0 },
	/* 037 */	{ "", 0, 0, 0 },
	/* 038 */	{ "", 0, 0, 0 },
	/* 039 */	{ "", 0, 0, 0 },
	/* 03a */	{ "", 0, 0, 0 },
	/* 03b */	{ "", 0, 0, 0 },
	/* 03c */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif

extern long int IT_REGCLIENTDOC();
extern long int IT_1LP1I1W();
extern long int IT_1D1LP();
extern long int IT_1D();
extern long int IT_OLELOADFROMSTREAM();
extern long int IT_OLEENUMOBJ();


 /* Interface Segment Image OLECLI: */

static long int (*seg_image_OLECLI_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	IT_OLELOADFROMSTREAM, OleLoadFromStream,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	IT_1LP1I1W, OleQueryLinkFromClip,
	/* 00a */	IT_1LP1I1W, OleQueryCreateFromClip,
	/* 00b */	Trap, 0,
	/* 00c */	Trap, 0,
	/* 00d */	Trap, 0,
	/* 00e */	Trap, 0,
	/* 00f */	Trap, 0,
	/* 010 */	Trap, 0,
	/* 011 */	Trap, 0,
	/* 012 */	Trap, 0,
	/* 013 */	Trap, 0,
	/* 014 */	Trap, 0,
	/* 015 */	Trap, 0,
	/* 016 */	Trap, 0,
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
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	Trap, 0,
	/* 025 */	Trap, 0,
	/* 026 */	Trap, 0,
	/* 027 */	Trap, 0,
	/* 028 */	Trap, 0,
	/* 029 */	IT_REGCLIENTDOC,  OleRegisterClientDoc,
	/* 02a */	IT_1D, OleRevokeClientDoc,
	/* 02b */	IT_1D1LP, OleRenameClientDoc,
	/* 02c */	IT_1D, OleRevertClientDoc,
	/* 02d */	IT_1D, OleSavedClientDoc,
	/* 02e */	Trap, 0,
	/* 02f */	IT_OLEENUMOBJ, OleEnumObjects,
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
	/* 03b */	IT_V, OleQueryClientVersion,
	/* 03c */	Trap, 0,
	0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
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
	/* 014 */	Trap, 0,
	/* 015 */	Trap, 0,
	/* 016 */	Trap, 0,
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
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	Trap, 0,
	/* 025 */	Trap, 0,
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
	0, 0
};
#endif

 /* Segment Table OLECLI: */

static SEGTAB seg_tab_OLECLI[] =
{	{ (char *) seg_image_OLECLI_0, 480, TRANSFER_CALLBACK, 480, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

 /* Module Descriptor for OLECLI: */

static MODULEDSCR hsmt_mod_dscr_OLECLI =
{	"OLECLI",
	entry_tab_OLECLI,
	seg_tab_OLECLI,
	0
};

extern long int OleRegisterServer();
extern long int OleRegisterServerDoc();
extern long int OleRevokeServerDoc();
extern long int OleRevokeServer();
extern long int OleRenameServerDoc();
extern long int OleBlockServer();
extern long int OleUnblockServer();

 /* Entry Table OLESVR: */

static ENTRYTAB entry_tab_OLESVR[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "OLESVR", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "OLEREGISTERSERVER", 0x0008, 0x0010, OleRegisterServer },
	/* 003 */	{ "OLEREVOKESERVER", 0x0008, 0x0018, OleRevokeServer },
	/* 004 */	{ "OLEBLOCKSERVER", 0x0008, 0x0020, OleBlockServer },
	/* 005 */	{ "OLEUNBLOCKSERVER", 0x0008, 0x0028, OleUnblockServer },
	/* 006 */	{ "OLEREGISTERSERVERDOC", 0x0008, 0x0030, OleRegisterServerDoc },
	/* 007 */	{ "OLEREVOKESERVERDOC", 0x0008, 0x0038, OleRevokeServerDoc },
	/* 008 */	{ "OLERENAMESERVERDOC", 0x0008, 0x0040, OleRenameServerDoc },
	/* 009 */	{ "OLEREVERTSERVERDOC", 0x0008, 0x0048, 0 },
	/* 00a */	{ "OLESAVEDSERVERDOC", 0x0008, 0x0050, 0 },
	/* 00b */	{ "OLEREVOKEOBJECT", 0x0008, 0x0058, 0 },
	/* 00c */	{ "OLEQUERYSERVERVERSION", 0x0008, 0x0060, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "", 0, 0, 0 },
	/* 00b */	{ "", 0, 0, 0 },
	/* 00c */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif

extern long int IT_REGSERVER();
extern long int IT_REGSERVERDOC();
extern long int IT_1D();
extern long int IT_1D1LP();
extern long int IT_UNBLOCKSERVER();


 /* Interface Segment Image OLESVR: */

static long int (*seg_image_OLESVR_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	IT_REGSERVER,  OleRegisterServer,
	/* 003 */	IT_1D, OleRevokeServer,
	/* 004 */	IT_1D,  OleBlockServer,
	/* 005 */	IT_UNBLOCKSERVER, OleUnblockServer,
	/* 006 */	IT_REGSERVERDOC, OleRegisterServerDoc,
	/* 007 */	IT_1D, OleRevokeServerDoc,
	/* 008 */	IT_1D1LP, OleRenameServerDoc,
	/* 009 */	Trap, 0,
	/* 00a */	Trap, 0,
	/* 00b */	Trap, 0,
	/* 00c */	Trap, 0,
	0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
	/* 00a */	Trap, 0,
	/* 00b */	Trap, 0,
	/* 00c */	Trap, 0,
	0, 0
};
#endif

 /* Segment Table OLESVR: */

static SEGTAB seg_tab_OLESVR[] =
{	{ (char *) seg_image_OLESVR_0, 96, TRANSFER_CALLBACK, 96, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

 /* Module Descriptor for OLESVR: */

static MODULEDSCR hsmt_mod_dscr_OLESVR =
{	"OLESVR",
	entry_tab_OLESVR,
	seg_tab_OLESVR,
	0
};

extern long int NotifyRegister();
extern long int InterruptRegister();
extern long int NotifyUnRegister();
extern long int InterruptUnRegister();
extern long int ModuleFirst();
extern long int ModuleNext();
extern long int MemManInfo();
extern long int GlobalEntryHandle();
extern long int TimerCount();

 /* Entry Table TOOLHELP: */

static ENTRYTAB entry_tab_TOOLHELP[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "TOOLHELP", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "", 0, 0, 0 },
	/* 00b */	{ "", 0, 0, 0 },
	/* 00c */	{ "", 0, 0, 0 },
	/* 00d */	{ "", 0, 0, 0 },
	/* 00e */	{ "", 0, 0, 0 },
	/* 00f */	{ "", 0, 0, 0 },
	/* 010 */	{ "", 0, 0, 0 },
	/* 011 */	{ "", 0, 0, 0 },
	/* 012 */	{ "", 0, 0, 0 },
	/* 013 */	{ "", 0, 0, 0 },
	/* 014 */	{ "", 0, 0, 0 },
	/* 015 */	{ "", 0, 0, 0 },
	/* 016 */	{ "", 0, 0, 0 },
	/* 017 */	{ "", 0, 0, 0 },
	/* 018 */	{ "", 0, 0, 0 },
	/* 019 */	{ "", 0, 0, 0 },
	/* 01a */	{ "", 0, 0, 0 },
	/* 01b */	{ "", 0, 0, 0 },
	/* 01c */	{ "", 0, 0, 0 },
	/* 01d */	{ "", 0, 0, 0 },
	/* 01e */	{ "", 0, 0, 0 },
	/* 01f */	{ "", 0, 0, 0 },
	/* 020 */	{ "", 0, 0, 0 },
	/* 021 */	{ "", 0, 0, 0 },
	/* 022 */	{ "", 0, 0, 0 },
	/* 023 */	{ "", 0, 0, 0 },
	/* 024 */	{ "", 0, 0, 0 },
	/* 025 */	{ "", 0, 0, 0 },
	/* 026 */	{ "", 0, 0, 0 },
	/* 027 */	{ "", 0, 0, 0 },
	/* 028 */	{ "", 0, 0, 0 },
	/* 029 */	{ "", 0, 0, 0 },
	/* 02a */	{ "", 0, 0, 0 },
	/* 02b */	{ "", 0, 0, 0 },
	/* 02c */	{ "", 0, 0, 0 },
	/* 02d */	{ "", 0, 0, 0 },
	/* 02e */	{ "", 0, 0, 0 },
	/* 02f */	{ "", 0, 0, 0 },
	/* 030 */	{ "", 0, 0, 0 },
	/* 031 */	{ "", 0, 0, 0 },
	/* 032 */	{ "GLOBALHANDLETOSEL", 0x0008, 0x0190, 0 },
	/* 033 */	{ "GLOBALFIRST", 0x0008, 0x0198, 0 },
	/* 034 */	{ "GLOBALNEXT", 0x0008, 0x01a0, 0 },
	/* 035 */	{ "GLOBALINFO", 0x0008, 0x01a8, 0 },
	/* 036 */	{ "GLOBALENTRYHANDLE", 0x0008, 0x01b0, GlobalEntryHandle },
	/* 037 */	{ "GLOBALENTRYMODULE", 0x0008, 0x01b8, 0 },
	/* 038 */	{ "LOCALINFO", 0x0008, 0x01c0, 0 },
	/* 039 */	{ "LOCALFIRST", 0x0008, 0x01c8, 0 },
	/* 03a */	{ "LOCALNEXT", 0x0008, 0x01d0, 0 },
	/* 03b */	{ "MODULEFIRST", 0x0008, 0x01d8, ModuleFirst },
	/* 03c */	{ "MODULENEXT", 0x0008, 0x01e0, ModuleNext },
	/* 03d */	{ "MODULEFINDNAME", 0x0008, 0x01e8, 0 },
	/* 03e */	{ "MODULEFINDHANDLE", 0x0008, 0x01f0, 0 },
	/* 03f */	{ "TASKFIRST", 0x0008, 0x01f8, 0 },
	/* 040 */	{ "TASKNEXT", 0x0008, 0x0200, 0 },
	/* 041 */	{ "TASKFINDHANDLE", 0x0008, 0x0208, 0 },
	/* 042 */	{ "STACKTRACEFIRST", 0x0008, 0x0210, 0 },
	/* 043 */	{ "STACKTRACECSIPFIRST", 0x0008, 0x0218, 0 },
	/* 044 */	{ "STACKTRACENEXT", 0x0008, 0x0220, 0 },
	/* 045 */	{ "CLASSFIRST", 0x0008, 0x0228, 0 },
	/* 046 */	{ "CLASSNEXT", 0x0008, 0x0230, 0 },
	/* 047 */	{ "SYSTEMHEAPINFO", 0x0008, 0x0238, 0 },
	/* 048 */	{ "MEMMANINFO", 0x0008, 0x0240, MemManInfo },
	/* 049 */	{ "NOTIFYREGISTER", 0x0008, 0x0248, NotifyRegister },
	/* 04a */	{ "NOTIFYUNREGISTER", 0x0008, 0x0250, NotifyUnRegister },
	/* 04b */	{ "INTERRUPTREGISTER", 0x0008, 0x0258, InterruptRegister },
	/* 04c */	{ "INTERRUPTUNREGISTER", 0x0008, 0x0260, InterruptUnRegister },
	/* 04d */	{ "TERMINATEAPP", 0x0008, 0x0268, 0 },
	/* 04e */	{ "MEMORYREAD", 0x0008, 0x0270, 0 },
	/* 04f */	{ "MEMORYWRITE", 0x0008, 0x0278, 0 },
	/* 050 */	{ "TIMERCOUNT", 0x0008, 0x0280, TimerCount },
	/* 051 */	{ "TASKSETCSIP", 0x0008, 0x0288, 0 },
	/* 052 */	{ "TASKGETCSIP", 0x0008, 0x0290, 0 },
	/* 053 */	{ "TASKSWITCH", 0x0008, 0x0298, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "", 0, 0, 0 },
	/* 00b */	{ "", 0, 0, 0 },
	/* 00c */	{ "", 0, 0, 0 },
	/* 00d */	{ "", 0, 0, 0 },
	/* 00e */	{ "", 0, 0, 0 },
	/* 00f */	{ "", 0, 0, 0 },
	/* 010 */	{ "", 0, 0, 0 },
	/* 011 */	{ "", 0, 0, 0 },
	/* 012 */	{ "", 0, 0, 0 },
	/* 013 */	{ "", 0, 0, 0 },
	/* 014 */	{ "", 0, 0, 0 },
	/* 015 */	{ "", 0, 0, 0 },
	/* 016 */	{ "", 0, 0, 0 },
	/* 017 */	{ "", 0, 0, 0 },
	/* 018 */	{ "", 0, 0, 0 },
	/* 019 */	{ "", 0, 0, 0 },
	/* 01a */	{ "", 0, 0, 0 },
	/* 01b */	{ "", 0, 0, 0 },
	/* 01c */	{ "", 0, 0, 0 },
	/* 01d */	{ "", 0, 0, 0 },
	/* 01e */	{ "", 0, 0, 0 },
	/* 01f */	{ "", 0, 0, 0 },
	/* 020 */	{ "", 0, 0, 0 },
	/* 021 */	{ "", 0, 0, 0 },
	/* 022 */	{ "", 0, 0, 0 },
	/* 023 */	{ "", 0, 0, 0 },
	/* 024 */	{ "", 0, 0, 0 },
	/* 025 */	{ "", 0, 0, 0 },
	/* 026 */	{ "", 0, 0, 0 },
	/* 027 */	{ "", 0, 0, 0 },
	/* 028 */	{ "", 0, 0, 0 },
	/* 029 */	{ "", 0, 0, 0 },
	/* 02a */	{ "", 0, 0, 0 },
	/* 02b */	{ "", 0, 0, 0 },
	/* 02c */	{ "", 0, 0, 0 },
	/* 02d */	{ "", 0, 0, 0 },
	/* 02e */	{ "", 0, 0, 0 },
	/* 02f */	{ "", 0, 0, 0 },
	/* 030 */	{ "", 0, 0, 0 },
	/* 031 */	{ "", 0, 0, 0 },
	/* 032 */	{ "", 0, 0, 0 },
	/* 033 */	{ "", 0, 0, 0 },
	/* 034 */	{ "", 0, 0, 0 },
	/* 035 */	{ "", 0, 0, 0 },
	/* 036 */	{ "", 0, 0, 0 },
	/* 037 */	{ "", 0, 0, 0 },
	/* 038 */	{ "", 0, 0, 0 },
	/* 039 */	{ "", 0, 0, 0 },
	/* 03a */	{ "", 0, 0, 0 },
	/* 03b */	{ "", 0, 0, 0 },
	/* 03c */	{ "", 0, 0, 0 },
	/* 03d */	{ "", 0, 0, 0 },
	/* 03e */	{ "", 0, 0, 0 },
	/* 03f */	{ "", 0, 0, 0 },
	/* 040 */	{ "", 0, 0, 0 },
	/* 041 */	{ "", 0, 0, 0 },
	/* 042 */	{ "", 0, 0, 0 },
	/* 043 */	{ "", 0, 0, 0 },
	/* 044 */	{ "", 0, 0, 0 },
	/* 045 */	{ "", 0, 0, 0 },
	/* 046 */	{ "", 0, 0, 0 },
	/* 047 */	{ "", 0, 0, 0 },
	/* 048 */	{ "", 0, 0, 0 },
	/* 049 */	{ "", 0, 0, 0 },
	/* 04a */	{ "", 0, 0, 0 },
	/* 04b */	{ "", 0, 0, 0 },
	/* 04c */	{ "", 0, 0, 0 },
	/* 04d */	{ "", 0, 0, 0 },
	/* 04e */	{ "", 0, 0, 0 },
	/* 04f */	{ "", 0, 0, 0 },
	/* 050 */	{ "", 0, 0, 0 },
	/* 051 */	{ "", 0, 0, 0 },
	/* 052 */	{ "", 0, 0, 0 },
	/* 053 */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif

extern long int _86_SystemHeapInfo();
extern long int IT_1H();
extern long int IT_NOTIFYREG();
extern long int IT_INTERRUPTREG();
extern long int IT_1LPME();
extern long int IT_MEMMANINFO();
extern long int IT_GLOBALENTRYHANDLE();

 /* Interface Segment Image TOOLHELP: */

static long int (*seg_image_TOOLHELP_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
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
	/* 014 */	Trap, 0,
	/* 015 */	Trap, 0,
	/* 016 */	Trap, 0,
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
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	Trap, 0,
	/* 025 */	Trap, 0,
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
	/* 036 */	IT_GLOBALENTRYHANDLE, GlobalEntryHandle,
	/* 037 */	Trap, 0,
	/* 038 */	Trap, 0,
	/* 039 */	Trap, 0,
	/* 03a */	Trap, 0,
	/* 03b */	IT_1LPME, ModuleFirst,
	/* 03c */	IT_1LPME, ModuleNext,
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
	/* 047 */	_86_SystemHeapInfo, 0,
	/* 048 */	IT_MEMMANINFO, MemManInfo,
	/* 049 */	IT_NOTIFYREG, NotifyRegister,
	/* 04a */	IT_1HTASK, NotifyUnRegister,
	/* 04b */	IT_INTERRUPTREG, InterruptRegister,
	/* 04c */	IT_1HTASK, InterruptUnRegister,
	/* 04d */	Trap, 0,
	/* 04e */	Trap, 0,
	/* 04f */	Trap, 0,
	/* 050 */	IT_TIMERCOUNT, TimerCount,
	/* 051 */	Trap, 0,
	/* 052 */	Trap, 0,
	/* 053 */	Trap, 0,
	0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
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
	/* 014 */	Trap, 0,
	/* 015 */	Trap, 0,
	/* 016 */	Trap, 0,
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
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	Trap, 0,
	/* 025 */	Trap, 0,
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
	0, 0
};
#endif

 /* Segment Table TOOLHELP: */

static SEGTAB seg_tab_TOOLHELP[] =
{	{ (char *) seg_image_TOOLHELP_0, 664, TRANSFER_CALLBACK, 664, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

 /* Module Descriptor for TOOLHELP: */

static MODULEDSCR hsmt_mod_dscr_TOOLHELP =
{	"TOOLHELP",
	entry_tab_TOOLHELP,
	seg_tab_TOOLHELP,
	0
};

extern long int InquireSystem();
extern long int Get80x87SaveSize();
extern long int CreateSystemTimer();
extern long int KillSystemTimer();
extern long int EnableSystemTimers();
extern long int DisableSystemTimers();

 /* Entry Table SYSTEM: */

static ENTRYTAB entry_tab_SYSTEM[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "SYSTEM", 0, 0, 0 },
	/* 001 */	{ "INQUIRESYSTEM", 0x0008, 0x0008, InquireSystem },
	/* 002 */	{ "CREATESYSTEMTIMER", 0x0008, 0x0010, CreateSystemTimer },
	/* 003 */	{ "KILLSYSTEMTIMER", 0x0008, 0x0018, KillSystemTimer },
	/* 004 */	{ "ENABLESYSTEMTIMERS", 0x0008, 0x0020, EnableSystemTimers },
	/* 005 */	{ "DISABLESYSTEMTIMERS", 0x0008, 0x0028, DisableSystemTimers },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "GET80X87SAVESIZE", 0x0008, 0x0038, Get80x87SaveSize },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif

extern long int IT_INQSYSTEM();
extern long int IT_V();
extern long int IT_1H();
extern long int IT_1I1D();


 /* Interface Segment Image SYSTEM: */

static long int (*seg_image_SYSTEM_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	IT_INQSYSTEM,  InquireSystem,
	/* 002 */	IT_1I1D, CreateSystemTimer,
	/* 003 */	IT_1H, KillSystemTimer,
	/* 004 */	IT_V, EnableSystemTimers,
	/* 005 */	IT_V, DisableSystemTimers,
	/* 006 */	Trap, 0,
	/* 007 */	IT_V,  Get80x87SaveSize,
	0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	0, 0
};
#endif

 /* Segment Table SYSTEM: */

static SEGTAB seg_tab_SYSTEM[] =
{	{ (char *) seg_image_SYSTEM_0, 56, TRANSFER_CALLBACK, 56, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

 /* Module Descriptor for SYSTEM: */

static MODULEDSCR hsmt_mod_dscr_SYSTEM =
{	"SYSTEM",
	entry_tab_SYSTEM,
	seg_tab_SYSTEM,
	0
};


 /* Entry Table WIN87EM: */

static ENTRYTAB entry_tab_WIN87EM[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "WIN87EM", 0, 0, 0 },
	/* 001 */	{ "WIN87EM", 0x0008, 0x0008, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif

extern long int _86_Win87Em();


 /* Interface Segment Image WIN87EM: */

static long int (*seg_image_WIN87EM_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	_86_Win87Em, 0,
	0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	0, 0
};
#endif

 /* Segment Table WIN87EM: */

static SEGTAB seg_tab_WIN87EM[] =
{	{ (char *) seg_image_WIN87EM_0, 8, TRANSFER_CALLBACK, 8, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

 /* Module Descriptor for WIN87EM: */

static MODULEDSCR hsmt_mod_dscr_WIN87EM =
{	"WIN87EM",
	entry_tab_WIN87EM,
	seg_tab_WIN87EM,
	0
};

 /* Module Table: */

MODULETAB TWIN_ModuleTable[256] =
{	{ 0, 0},
	{ MODULE_SYSTEM, &hsmt_mod_dscr_KERNEL},
	{ MODULE_SYSTEM, &hsmt_mod_dscr_GDI},
	{ MODULE_SYSTEM, &hsmt_mod_dscr_USER},
	{ MODULE_NO_LOAD|MODULE_SYSTEM, &hsmt_mod_dscr_OLECLI},
	{ MODULE_NO_LOAD|MODULE_SYSTEM, &hsmt_mod_dscr_OLESVR},
	{ MODULE_NO_LOAD|MODULE_SYSTEM, &hsmt_mod_dscr_TOOLHELP},
	{ MODULE_NO_LOAD|MODULE_SYSTEM, &hsmt_mod_dscr_SYSTEM},
	{ MODULE_NO_LOAD|MODULE_SYSTEM, &hsmt_mod_dscr_WIN87EM},

	{ 0, 0 }
};
