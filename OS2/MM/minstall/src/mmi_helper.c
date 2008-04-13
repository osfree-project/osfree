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

#define INCL_BASE
#define INCL_DOSMODULEMGR
#define INCL_WINSHELLDATA
#include <os2.h>
#include <malloc.h>

#include <global.h>
#include <stdarg.h>
#include <crcs.h>
#include <dll.h>
#include <file.h>
#include <globstr.h>
#include <msg.h>
#include <mmi_public.h>
#include <mmi_types.h>
#include <mmi_main.h>
#include <mmi_helper.h>
#include <mmi_msg.h>

// ****************************************************************************

PMINSTDIR MINSTALL_SearchSourceDirID (ULONG DirectoryID) {
   PMINSTDIR CurDirPtr = MCF_SourceDirArrayPtr;
   USHORT       CurDirNo  = 0;

   if (MCF_SourceDirCount==0)
      return NULL;

   while (CurDirNo<MCF_SourceDirCount) {
      if (CurDirPtr->ID==DirectoryID)
         return CurDirPtr;
      CurDirPtr++; CurDirNo++;
    }
   return NULL;
 }

// This will search the ID of the root source directory ("\")
PMINSTDIR MINSTALL_SearchRootSourceDirID (void) {
   PMINSTDIR CurDirPtr = MCF_SourceDirArrayPtr;
   USHORT       CurDirNo  = 0;

   if (MCF_SourceDirCount==0)
      return NULL;

   while (CurDirNo<MCF_SourceDirCount) {
      if (strcmp(CurDirPtr->Name, "\\")==0)
         return CurDirPtr;
      CurDirPtr++; CurDirNo++;
    }
   return NULL;
 }

PMINSTDIR MINSTALL_SearchDestinDirID (ULONG DirectoryID) {
   PMINSTDIR CurDirPtr = MCF_DestinDirArrayPtr;
   USHORT       CurDirNo  = 0;

   if (MCF_DestinDirCount==0)
      return NULL;

   while (CurDirNo<MCF_DestinDirCount) {
      if (CurDirPtr->ID==DirectoryID)
         return CurDirPtr;
      CurDirPtr++; CurDirNo++;
    }
   return NULL;
 }

PMINSTGRP MINSTALL_SearchGroupID (ULONG GroupID) {
   PMINSTGRP CurGrpPtr = MCF_GroupArrayPtr;
   USHORT       CurGrpNo  = 0;

   if (MCF_GroupCount==0)
      return NULL;

   while (CurGrpNo<MCF_GroupCount) {
      if (CurGrpPtr->ID==GroupID)
         return CurGrpPtr;
      CurGrpPtr++; CurGrpNo++;
    }
   return NULL;
 }

PMINSTGRP MINSTALL_SearchGroupGeninID (PMINSTFILE CARDINFOFilePtr, ULONG GeninID) {
   PMINSTGRP CurGrpPtr = MCF_GroupArrayPtr;
   USHORT       CurGrpNo  = 0;

   if (MCF_GroupCount==0)
      return NULL;

   while (CurGrpNo<MCF_GroupCount) {
      if ((CurGrpPtr->GeninDLLFilePtr==CARDINFOFilePtr) && (CurGrpPtr->GeninID==GeninID))
         return CurGrpPtr;
      CurGrpPtr++; CurGrpNo++;
    }
   return NULL;
 }

PMINSTFILE MINSTALL_SearchFileCRC32 (ULONG FileCRC32) {
   PMINSTFILE CurFilePtr = FCF_FileArrayPtr;
   USHORT        CurFileNo  = 0;

   if (FCF_FileCount==0)
      return NULL;

   while (CurFileNo<FCF_FileCount) {
      if (CurFilePtr->NameCRC32==FileCRC32)
         return CurFilePtr;
      CurFilePtr++; CurFileNo++;
    }
   return NULL;
 }

PSZ MINSTALL_GetPointerToMacro (PCHAR *CurPosPtr, PCHAR EndPos) {
   PCHAR         CurPos     = *CurPosPtr+2;
   PCHAR         StartPos   = CurPos;
   ULONG         CRC32      = 0;
   ULONG         Temp       = 0;
   PMINSTFILE CurFilePtr = 0;
   PMINSTDIR  CurDirPtr  = 0;

   while ((CurPos<EndPos) && (*CurPos!=')')) {
      *CurPos = tolower(*CurPos);
      CurPos++;
    }
   CRC32 = CRC32_GetFromString (StartPos, CurPos-StartPos); CurPos++;

   // Update CurPos @ caller...
   *CurPosPtr = CurPos;

   switch (CRC32) {
    case 0xBAAB7A10: // $(DIR) -> Destination-Path by number, without ending "\"
      StartPos = CurPos;
      // Pass till non numeric digit or End-Of-Line
      while ((CurPos<EndPos) & (*CurPos>=0x30) & (*CurPos<=0x39))
         CurPos++;
      if (CurPos>StartPos) {                // we found any digits?
         Temp = atol(StartPos);
         if ((CurDirPtr = MINSTALL_SearchDestinDirID(Temp))!=0) {
            *CurPosPtr = CurPos;
            strcpy (MINSTALL_TempMacroSpace, CurDirPtr->FQName);
            Temp = strlen(CurDirPtr->FQName);
            if (Temp!=0) Temp--;
            if (MINSTALL_TempMacroSpace[Temp]=='\\') {
               // only remove last char, if last char is '\'
               MINSTALL_TempMacroSpace[Temp] = 0; // Remove last char '\'
             }
            return (PCHAR)&MINSTALL_TempMacroSpace;
          } else {
            MINSTALL_ErrorMsgID = MINSTMSG_UnknownDestinID;
            return 0;
          }
       } else {
         MINSTALL_ErrorMsgID = MINSTMSG_NumericValueExpected; return 0; }
    case 0x88662993: // $(DEST) -> Destination Path of filename
      StartPos = CurPos;
      // Pass till ';', '"', ',', ' ' or End-Of-Line
      while ((CurPos<EndPos) && (*CurPos!=0x22) && (*CurPos!=0x3B) && (*CurPos!=0x2C) && (*CurPos!=0x20)) {
         *CurPos = tolower(*CurPos);
         CurPos++;
       }
      if (CurPos>StartPos) {                // we found anything?
         CRC32 = CRC32_GetFromString (StartPos, CurPos-StartPos);
         if ((CurFilePtr = MINSTALL_SearchFileCRC32(CRC32))!=0) {
            // Got that file, now get destination directory for it
            return CurFilePtr->DestinPtr->FQName;
          } else {
            MSG_SetInsertViaString (2, StartPos, CurPos-StartPos);
            MINSTALL_ErrorMsgID = MINSTMSG_UnlistedFile;
            return 0;
          }
       } else {
         MINSTALL_ErrorMsgID = MINSTMSG_StringExpected; return 0; }
    case 0x46EDAEC4: // $(BOOT) -> Write Boot-Drive Letter (char only)
      if (*CurPos==0x3A)
         return (PCHAR)&MINSTALL_BootLetter; // char only (e.g. "D")
        else
         return (PCHAR)&MINSTALL_BootDrive;  // drive (e.g. "D:")
    case 0x681DF58F: // $(DRIVE) -> Write MMBase-Drive Letter (char only)
      if (*CurPos==0x3A)
         return (PCHAR)&MINSTALL_MMBaseLetter; // char only (e.g. "D")
        else
         return (PCHAR)&MINSTALL_BootDrive;    // drive (e.g. "D:")
    }
   // Unknown macro, so reply NUL string...
   return "";
 }

// Extracts a string including macro processing (e.g. '"TEST $(DIR)2"')
//  including error checking. Needs to be called using CurPos == Position of
//  trailing '"'
PCHAR MINSTALL_GetMacrodString (PCHAR DestPtr, ULONG DestMaxSize, PCHAR StartPos, PCHAR EndPos) {
   PCHAR  CurPos     = StartPos;
   ULONG  TmpLen     = 0;
   PCHAR  CurDestPtr = DestPtr;
   PCHAR  MacroPtr   = 0;
   ULONG  MacroLen   = 0;
   CHAR   CurChar;

   // One less, because of terminating Zero
   DestMaxSize--;

   if (*StartPos=='"') {
      CurPos++;                             // Skip over '"'
      // Now take over any chars till '"' including escape mechanismn
      while (CurPos<EndPos) {
         CurChar = *CurPos;
         if (CurChar=='"') {
            *CurDestPtr = 0;                // Set terminating NUL
            return CurPos+1;
          }
         if (CurChar==0x0D) break;          // End-Of-Line reached during String
         if (CurChar==0x5C) {               // is escape mechanismn
            CurPos++;
            if (CurPos<EndPos) {
               if (!DestMaxSize) {
                  MINSTALL_ErrorMsgID = MINSTMSG_StringTooBig; return 0; }
               *CurDestPtr++ = *CurPos; DestMaxSize--;
               CurPos++;
             }
          } else if (*(PUSHORT)CurPos==0x2824) {  // "$(" Macro found
            if (!(MacroPtr = MINSTALL_GetPointerToMacro (&CurPos, EndPos)))
               return 0;                    // Bad macro, so signal error
            MacroLen = strlen(MacroPtr);
            if (DestMaxSize>=MacroLen) {
               memcpy (CurDestPtr, MacroPtr, MacroLen);
               CurDestPtr += MacroLen; DestMaxSize -= MacroLen;
             }
          } else {
            if (!DestMaxSize) {
               MINSTALL_TrappedError (MINSTMSG_StringTooBig); return 0; }
            *CurDestPtr++ = CurChar; DestMaxSize--;
            CurPos++;
          }
       }
      MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
      return 0;                             // End-Of-Line reached during string
    }
   MINSTALL_ErrorMsgID = MINSTMSG_StringExpected;
   return 0;
 }

PCHAR MINSTALL_GetNumericValue (PULONG DestPtr, PCHAR StartPos, PCHAR EndPos) {
   PCHAR CurPos;

   CurPos = STRING_GetNumericValue(DestPtr, StartPos, EndPos);
   if (!CurPos)
      MINSTALL_ErrorMsgID = MINSTMSG_NumericValueExpected;
   return CurPos;
 }

ULONG MINSTALL_GetVersionCode (PSZ VersionString) {
   PCHAR CurPos;
   ULONG Number1;
   ULONG Number2 = 0;
   ULONG Number3 = 0;

   Number1 = strtol (VersionString, &CurPos, 10);
   if (*CurPos=='.') {
      CurPos++;
      if (*CurPos!=0) {
         Number2 = strtoul (CurPos, &CurPos, 10);
         if (*CurPos=='.') {
            CurPos++;
            if (*CurPos!=0) {
               Number3 = strtoul (CurPos, NULL, 10);
             }
          }
       }
    }
   if (Number1>255) Number1=255;
   if (Number2>255) Number2=255;
   if (Number3>255) Number3=255;
   return (Number1<<16)+(Number2<<8)+Number3;
 }

VOID MINSTALL_GetInstalledVersion (PSZ GroupName, PSZ DestVersionInstalled) {
   HINI  CompListHandle = 0;
   ULONG MaxSize        = MINSTMAX_STRLENGTH;

   DestVersionInstalled[0] = 0;
   // If GroupName is NULL, just set NULL Version
   if (strlen(GroupName)==0) return;

   // Open COMPLIST.ini...
   //  MINSTALL_PMHandle may be NULL in here, but this won't matter.
   CompListHandle = PrfOpenProfile (MINSTALL_PMHandle, MINSTALL_CompListINI);
   if (CompListHandle==NULLHANDLE) return;  // Exit, if Open failed

   // Query version-number...
   PrfQueryProfileData(CompListHandle, GroupName, "VERSION_NUMBER", (PVOID)DestVersionInstalled, &MaxSize);

   // Close COMPLIST.ini...
   PrfCloseProfile (CompListHandle);
 }

// No real error reporting here, because version saving is somewhat optional
//  we never abort installation because of errors here!
VOID MINSTALL_SetInstalledVersion (PSZ GroupName, PSZ Version) {
   HINI  CompListHandle = 0;
   ULONG VersionSize    = strlen(Version);

   // If GroupName is NULL, dont set anything
   if (strlen(GroupName)==0) return;

   // Open COMPLIST.ini...
   //  MINSTALL_PMHandle may be NULL in here, but this won't matter.
   CompListHandle = PrfOpenProfile (MINSTALL_PMHandle, MINSTALL_CompListINI);
   if (CompListHandle==NULLHANDLE) return;  // Exit, if Open failed

   // Set version-number...
   PrfWriteProfileData(CompListHandle, GroupName, "VERSION_NUMBER", (PVOID)Version, VersionSize);

   // Also set variable that shows we installed it
   PrfWriteProfileData(CompListHandle, GroupName, "INSTALLED_BY", (PVOID)"MINSTALL/REMAKE", 15);

   // Close COMPLIST.ini...
   PrfCloseProfile (CompListHandle);
 }

ULONG   SavedCurrentDiskNum = 0;
CHAR    SavedCurrentDirectory[MINSTMAX_PATHLENGTH];

/* 26.06.2005 - implemented compatibility code */
/*               saves, restores and sets current drive and directory */
/*               currently used in mmi_customdll.c */
VOID MINSTALL_SaveCurrentDirectory (void) {
   ULONG DriveMap;
   ULONG BufSize = MINSTMAX_PATHLENGTH;
   DosQueryCurrentDisk(&SavedCurrentDiskNum, &DriveMap);
   DosQueryCurrentDir(0, (PCHAR)&SavedCurrentDirectory, &BufSize);
 }

VOID MINSTALL_RestoreCurrentDirectory (void) {
   DosSetDefaultDisk(SavedCurrentDiskNum);
   DosSetCurrentDir((PCHAR)&SavedCurrentDirectory);
 }

VOID MINSTALL_SetCurrentDirectoryToSource (void) {
   ULONG DiskNum = 0;

   if ((MINSTALL_SourcePath[0]>='A') && (MINSTALL_SourcePath[0]<='Z'))
      DiskNum = MINSTALL_SourcePath[0]-'A'+1;
     else if ((MINSTALL_SourcePath[0]>='a') && (MINSTALL_SourcePath[0]<='z'))
      DiskNum = MINSTALL_SourcePath[0]-'a'+1;
   if ((DiskNum!=0) && (MINSTALL_SourcePath[0]!=0) && (MINSTALL_SourcePath[1]!=0)) {
      DosSetDefaultDisk(DiskNum);
      DosSetCurrentDir((PCHAR)(((ULONG)&MINSTALL_SourcePath)+2));
    }
 }

VOID MINSTALL_TrappedError (ULONG ErrorMsgID) {
   MINSTALL_ErrorMsgID = ErrorMsgID;
   MSG_Get (MINSTALL_ErrorMsg, 1024, MINSTALL_ErrorMsgID);
   MINSTLOG_ToFile (MINSTALL_ErrorMsg);
 }

VOID MINSTALL_TrappedWarning (ULONG ErrorMsgID) {
   MSG_Get (MINSTALL_ErrorMsg, 1024, ErrorMsgID);
   MINSTLOG_ToFile (MINSTALL_ErrorMsg);
 }

VOID MINSTLOG_OpenFile (void) {
   MINSTLOG_CloseFile();
   if (MINSTLOG_FileName[0]!=0)
      MINSTLOG_FileHandle = fopen(MINSTLOG_FileName, "w+b");
 }

VOID MINSTLOG_CloseFile (void) {
   if (MINSTLOG_FileHandle) {
      fclose (MINSTLOG_FileHandle);
      MINSTLOG_FileHandle = NULL;
    }
 }

VOID MINSTLOG_ToFile (PSZ FormatStr, ...) {
   va_list arglist;
   va_start (arglist, FormatStr);
   if (MINSTLOG_FileHandle) vfprintf (MINSTLOG_FileHandle, FormatStr, arglist);
/*   printf ("%s", FormatStr); */
   va_end (arglist);
 }

VOID MINSTLOG_ToScreen (PSZ FormatStr, ...) {
   va_list arglist;
   va_start (arglist, FormatStr);
   vprintf (FormatStr, arglist);
   va_end (arglist);
 }

VOID MINSTLOG_ToAll (PSZ FormatStr, ...) {
   va_list arglist;
   va_start (arglist, FormatStr);
   vprintf (FormatStr, arglist);
   if (MINSTLOG_FileHandle) vfprintf (MINSTLOG_FileHandle, FormatStr, arglist);
   va_end (arglist);
 }

VOID MINSTALL_printf (PSZ FormatStr, ...) {
   va_list arglist;
   va_start (arglist, FormatStr);
   vprintf (FormatStr, arglist);
   va_end (arglist);
 }
