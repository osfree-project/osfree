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

#ifndef _MMI_HELPER_H_
#define _MMI_HELPER_H_

PMINSTDIR  MINSTALL_SearchSourceDirID (ULONG DirectoryID);
PMINSTDIR  MINSTALL_SearchRootSourceDirID (void);
PMINSTDIR  MINSTALL_SearchDestinDirID (ULONG DirectoryID);
PMINSTGRP  MINSTALL_SearchGroupID (ULONG GroupID);
PMINSTGRP  MINSTALL_SearchGroupGeninID (PMINSTFILE CARDINFOFilePtr, ULONG GeninID);
PMINSTFILE MINSTALL_SearchFileCRC32 (ULONG FileCRC32);
PCHAR      MINSTALL_ExtractValue (PULONG DestPtr, PCHAR StartPos, PCHAR EndPos);
PSZ        MINSTALL_GetPointerToMacro (PCHAR *CurPosPtr, PCHAR EndPos);
PCHAR      MINSTALL_GetMacrodString (PCHAR DestPtr, ULONG DestMaxSize, PCHAR StartPos, PCHAR EndPos);
PCHAR      MINSTALL_GetNumericValue (PULONG DestPtr, PCHAR StartPos, PCHAR EndPos);
ULONG      MINSTALL_GetVersionCode (PSZ VersionString);
VOID       MINSTALL_GetInstalledVersion (PSZ GroupName, PSZ DestVersionInstalled);
VOID       MINSTALL_SetInstalledVersion (PSZ GroupName, PSZ Version);
VOID       MINSTALL_printf (PSZ FormatStr, ...);

VOID       MINSTALL_SaveCurrentDirectory (void);
VOID       MINSTALL_RestoreCurrentDirectory (void);
VOID       MINSTALL_SetCurrentDirectoryToSource (void);

VOID       MINSTALL_TrappedError (ULONG ErrorMsgID);
VOID       MINSTALL_TrappedWarning (ULONG ErrorMsgID);

VOID       MINSTLOG_OpenFile (void);
VOID       MINSTLOG_CloseFile (void);
VOID       MINSTLOG_ToFile (PSZ FormatStr,...);
VOID       MINSTLOG_ToScreen (PSZ FormatStr,...);
VOID       MINSTLOG_ToAll (PSZ FormatStr,...);

#endif
