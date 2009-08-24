//
// MINSTALL.DLL (c) Copyright 2002-2005 Martin Kiewitz
//
// This file is part of MINSTALL.DLL for OS/2 / eComStation
//
// MINSTALL.DLL is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
// MINSTALL.DLL is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//  along with MINSTALL.DLL.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _MMI_MAIN_H_
#define _MMI_MAIN_H_

FILECONTROL   CONTROLSCR;
FILECONTROL   FILELISTSCR;
FILECONTROL   CHANGESCR;

HAB           MINSTALL_PMHandle;
HMQ           MINSTALL_MSGQHandle;
CHAR          MINSTALL_MMBase[MINSTMAX_PATHLENGTH];
CHAR          MINSTALL_BootDrive[3];
CHAR          MINSTALL_BootLetter[2];
CHAR          MINSTALL_MMBaseDrive[3];
CHAR          MINSTALL_MMBaseLetter[2];
// Path means pathname, Dir means Directory (directory includes ending '\'
CHAR          MINSTALL_SourcePath[MINSTMAX_PATHLENGTH];
CHAR          MINSTALL_SourceDir[MINSTMAX_PATHLENGTH];
CHAR          MINSTALL_InstallPath[MINSTMAX_PATHLENGTH];
CHAR          MINSTALL_InstallDir[MINSTMAX_PATHLENGTH];
CHAR          MINSTALL_DLLDir[MINSTMAX_PATHLENGTH];
CHAR          MINSTALL_CompListINI[MINSTMAX_PATHLENGTH];

CHAR           MINSTLOG_FileName[MINSTMAX_PATHLENGTH];
FILE          *MINSTLOG_FileHandle;

CHAR           MINSTALL_TempMacroSpace[MINSTMAX_PATHLENGTH];

ULONG          MINSTALL_PublicGroupCount;
PMINSTPUBGROUP MINSTALL_PublicGroupArrayPtr;

// Contains all processed things
ULONG          MINSTALL_Done;
ULONG          MINSTALL_ErrorMsgID;
CHAR           MINSTALL_ErrorMsg[1024];

// Master-Control-File Variables...
USHORT         MCF_GroupCount;
PMINSTGRP      MCF_GroupArrayPtr;
USHORT         MCF_SourceDirCount;
PMINSTDIR      MCF_SourceDirArrayPtr;
USHORT         MCF_DestinDirCount;
PMINSTDIR      MCF_DestinDirArrayPtr;
USHORT         FCF_FileCount;
PMINSTFILE     FCF_FileArrayPtr;
CHAR           MCF_PackageName[MINSTMAX_STRLENGTH];
ULONG          MCF_CodePage;
ULONG          MCF_MUnitCount;
CHAR           MCF_Medianame[MINSTMAX_STRLENGTH];

BOOL           MINSTALL_GeninUsed;
BOOL           MINSTALL_IsBaseInstallation;
BOOL           MINSTALL_IsFirstInit;
BOOL           MINSTALL_SystemShouldReboot;

/* HMODULE        FCF_CARDINFOHandle; */
/* PMINSTFILE     FCF_CARDINFOFilePtr; */
HMODULE         FCF_LastCARDINFOHandle;

// MINSTCID variables
USHORT         MINSTCID_GroupCount;
USHORT         MINSTCID_FileCount;

HEV            CustomAPI_InitEventHandle;
TID            CustomAPI_ThreadID;
BOOL           CustomAPI_ThreadCreated;
HAB            CustomAPI_PMHandle;
HMQ            CustomAPI_MSGQHandle;
HWND           CustomAPI_WindowHandle;
ULONG          CustomAPI_ConfigSysLine;

// Custom-DLL related functions (10.07.2005 - compatibility)
PVOID          CustomDLL_EntryPoint;
PSZ            CustomDLL_EntryParms;
PSZ            CustomDLL_CustomData;

PMINSTINI_DEFENTRY ICF_CheckFuncList;
PMINSTINI_DEFENTRY ICF_CheckParmList;
PMINSTINI_DEFENTRY ICF_CurFuncEntry;
PMINSTINI_DEFENTRY ICF_CurParmEntry;
ULONG              ICF_FilledParms;

// Public Internal Routines
CHAR           MINSTALL_GetValidChar (PCHAR *CurPosPtr, PCHAR EndPos, PULONG CurLineNo);
PMINSTDIR      MINSTALL_SearchSourceDirID (ULONG DirectoryID);
PMINSTDIR      MINSTALL_SearchDestinDirID (ULONG DirectoryID);
PMINSTGRP      MINSTALL_SearchGroupID (ULONG GroupID);
PMINSTFILE     MINSTALL_SearchFileCRC32 (ULONG FileCRC32);
PCHAR          MINSTALL_ExtractValue (PULONG DestPtr, PCHAR StartPos, PCHAR EndPos);
PSZ            MINSTALL_GetPointerToMacro (PCHAR *CurPosPtr, PCHAR EndPos);
PCHAR          MINSTALL_ExtractMacroString (PCHAR DestPtr, ULONG DestMaxSize, PCHAR CurPos, PCHAR EndPos);

#endif
