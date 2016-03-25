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

#define INCL_NOPMAPI
#define INCL_BASE
#define INCL_DOSMODULEMGR
#define INCL_OS2MM
#include <os2.h>
//#include <pmdef.h>
#include <os2me.h>

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <global.h>
#include <cfgsys.h>
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

BOOL MINSTALL_ExtractDirOptions (PMINSTDIR CurDirPtr) {
   PCHAR CurPos   = CurDirPtr->Name;
   PCHAR StartPos = 0;
   ULONG VarID    = 0;

   while ((*CurPos==0x24) && (*(CurPos+1)==0x28)) { // Check for '$('
      CurPos += 2; StartPos = CurPos;
      while (*CurPos!=0x29) {               // Wait till ')'
         if (*CurPos==0)                    // EOS? -> Exit, due error
            return FALSE;
         CurPos++;
       }
      VarID = CRC32_GetFromString (StartPos, CurPos-StartPos);
      switch (VarID) {
       case 0x70784370:                     // BOOT
         CurDirPtr->Flags |= MINSTDIR_Flags_Boot;
         break;
       case 0xCAEABB01:                     // DELETE
         CurDirPtr->Flags |= MINSTDIR_Flags_Delete;
         break;
       case 0x1A2A0F17:                     // LOCKED
         CurDirPtr->Flags |= MINSTDIR_Flags_Locked;
         break;
       default:
         return FALSE;                      // Unknown -> Exit, due error
       }
      CurPos++;
    }
   if ((CurDirPtr->Flags & MINSTDIR_Flags_Delete) && (CurDirPtr->Flags & MINSTDIR_Flags_Locked))
      return FALSE;                         // Delete & Locked set? -> Error
   if (CurPos!=CurDirPtr->Name)
      strcpy (CurDirPtr->Name, CurPos);
   return TRUE;
 }

BOOL MINSTALL_BuildDestinDir (PCHAR DestString, ULONG DestMaxLength, PMINSTDIR CurDirPtr) {
   if (CurDirPtr->Flags & MINSTDIR_Flags_Locked) {
      if (!(DestString = STRING_BuildEscaped(DestString, &DestMaxLength, "$(LOCKED)")))
         return FALSE;
    }
   if (CurDirPtr->Flags & MINSTDIR_Flags_Delete) {
      if (!(DestString = STRING_BuildEscaped(DestString, &DestMaxLength, "$(DELETE)")))
         return FALSE;
    }
   if (CurDirPtr->Flags & MINSTDIR_Flags_Boot) {
      if (!(DestString = STRING_BuildEscaped(DestString, &DestMaxLength, "$(BOOT)")))
         return FALSE;
    } else if (CurDirPtr->Flags & MINSTDIR_Flags_MMBase) {
      if (!(DestString = STRING_BuildEscaped(DestString, &DestMaxLength, MINSTALL_MMBase+2)))
         return FALSE;
    }
   if (!(DestString = STRING_BuildEscaped(DestString, &DestMaxLength, CurDirPtr->Name)))
      return FALSE;
   return TRUE;
 }

// Finalizes MINSTALL-Directory Names (detailed logic please look at comments)
BOOL MINSTALL_FillDirFQName (PMINSTDIR CurDirPtr) {
   if (CurDirPtr->Flags & MINSTDIR_Flags_Source) {
      // We will ignore any flags here, but just add Source-Directory to the
      //  whole mess...
      if (!STRING_CombinePSZ((PCHAR)CurDirPtr->FQName, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_SourcePath, (PCHAR)CurDirPtr->Name))
         return FALSE;                      // due buffer-overflow...
    } else {
      //         No Flags Set    BOOT            MMBASE
      // \DLL -> M:\DLL       -> C:\DLL       -> M:\MMOS2\DLL
      if (CurDirPtr->Flags & MINSTDIR_Flags_Boot) {
         if (!STRING_CombinePSZ((PCHAR)CurDirPtr->FQName, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_BootDrive, (PCHAR)CurDirPtr->Name))
            return FALSE;                   // due buffer-overflow...
       } else if (CurDirPtr->Flags & MINSTDIR_Flags_MMBase) {
         if (!STRING_CombinePSZ((PCHAR)CurDirPtr->FQName, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_MMBase, (PCHAR)CurDirPtr->Name))
            return FALSE;                   // due buffer-overflow...
       } else {
         if (!STRING_CombinePSZ((PCHAR)CurDirPtr->FQName, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_MMBaseDrive, (PCHAR)CurDirPtr->Name))
            return FALSE;                   // due buffer-overflow...
       }
    }
   return TRUE;
 }

// ****************************************************************************

// This will pre-process a Master-Control-File (MCF)
//  It will count any occurance of Groups and Source/Destination directories
//  This is done, cause we do not rely on variables to be valid *and* we dont
//  want to hard-limit any buffers.
VOID MINSTALL_PreProcessMasterControlFile (PFILECONTROL ControlFile) {
   PCHAR  CurPos       = ControlFile->BufferPtr;
   PCHAR  EndPos       = ControlFile->BufferEndPtr;
   PCHAR  LineStartPos = 0;
   PCHAR  LineEndPos   = 0;
   CHAR   CurChar      = 0;
   ULONG  ValueID      = 0;
   ULONG  CurLineNo    = 1;

   // Reset Count-variables...
   MCF_GroupCount      = 0;
   MCF_SourceDirCount  = 0;
   MCF_DestinDirCount  = 0;

   while (CurPos<EndPos) {
      if (!(CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo)))
         break;
      LineStartPos = CurPos;
      LineEndPos   = STRING_GetEndOfLinePtr (CurPos, EndPos);
      while (CurPos<LineEndPos) {
         CurChar = *CurPos;
         if (CurChar=='=') {
            // Found an equal sign... Seek back over spaces...
            do {
               CurPos--;
             } while ((CurPos>LineStartPos) && (*CurPos==' '));
            CurPos++;
            ValueID = CRC32_GetFromString(LineStartPos, CurPos-LineStartPos);
            switch (ValueID) {
             case 0x0D14BDAE: // sourcedir
               MCF_SourceDirCount++; break;
             case 0x7C4DC759: // destindir
               MCF_DestinDirCount++; break;
             case 0x59FD2765: // ssgroup
               MCF_GroupCount++; break;
             default:
               break;
             }
            break;
          }
         *CurPos = toupper(CurChar);
         CurPos++;
       }
      CurPos = LineEndPos; CurLineNo++;
    }

   if (MCF_SourceDirCount==0) {
      // If no Sources defined, assume 1 source directory (current directory)
      MCF_SourceDirCount = 1;
    }
 }

// This one processes a Master-Control-File (MCF).
//  It will load all required variables into memory for later processing.
//  Please note that some variables are not loaded, because they are calculated
//  later (e.g. sssize).
BOOL MINSTALL_ProcessMasterControlFile (PFILECONTROL ControlFile) {
   PCHAR      CurPos          = ControlFile->BufferPtr;
   PCHAR      EndPos          = ControlFile->BufferEndPtr;
   PCHAR      LineStartPos    = 0;
   PCHAR      LineEquPos      = 0;
   PCHAR      LineEndPos      = 0;
   PCHAR      TmpPos          = 0;
   CHAR       CurChar         = 0;
   ULONG      TmpLen          = 0;
   ULONG      CurArray        = 0;
   ULONG      ValueID         = 0;
   ULONG      VarID           = 0;
   ULONG      CurLineNo       = 1;
   PMINSTGRP  CurGroupPtr     = MCF_GroupArrayPtr;
   PMINSTDIR  CurSourceDirPtr = MCF_SourceDirArrayPtr;
   PMINSTDIR  CurDestinDirPtr = MCF_DestinDirArrayPtr;
   CHAR       TempString[MINSTMAX_PATHLENGTH];

   // Reset some variables...
   MCF_PackageName[0]  = 0;
   MCF_Medianame[0]    = 0;
   MCF_CodePage        = 0;
   MCF_MUnitCount      = 0;
   MCF_GroupCount      = 0;
   MCF_SourceDirCount  = 0;
   MCF_DestinDirCount  = 0;

   FILELISTSCR.Name[0] = 0;

   while (CurPos<EndPos) {
      if (!(CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo)))
         break;
      LineStartPos = CurPos;
      LineEndPos   = STRING_GetEndOfLinePtr (CurPos, EndPos);
      LineEquPos   = 0;
      while (CurPos<LineEndPos) {
         CurChar = *CurPos;
         if (CurChar=='=') {
            // Found an equal sign... Seek back over spaces...
            LineEquPos = CurPos;
            do {
               CurPos--;
             } while ((CurPos>LineStartPos) && (*CurPos==' '));
            CurPos++;
            break;
          }
         *CurPos = toupper(CurChar);
         CurPos++;
       }

      if (LineEquPos) {
         // We found an '=', so identify value and extract contents...
         ValueID = CRC32_GetFromString(LineStartPos, CurPos-LineStartPos);
         CurPos  = LineEquPos+1;
         if (!(CurChar = STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo))) {
            MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break; }
         switch (ValueID) {
          case 0xE12D998F: // package
            if (!STRING_GetString(MCF_PackageName, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          case 0x7772043C: // codepage
            if (!STRING_GetNumericValue(&MCF_CodePage, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric;
            break;
          case 0xE0D3F337: // filelist
            if (!STRING_GetString(TempString, MINSTMAX_PATHLENGTH, CurPos, EndPos)) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            if (!STRING_CombinePSZ ((PCHAR)&FILELISTSCR.Name, MINSTMAX_PATHLENGTH, MINSTALL_SourceDir, (PCHAR)&TempString))
               MINSTALL_ErrorMsgID = MINSTMSG_StringTooLong;
            break;
          case 0x7E3F3B04: // munitcount
            if (!STRING_GetNumericValue(&MCF_MUnitCount, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric;
            break;
          case 0x1CDB0BEE: // medianame
            if (!STRING_GetString(MCF_Medianame, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          case 0x0D14BDAE: // sourcedir
            if (!(CurPos = STRING_GetString(CurSourceDirPtr->Name, MINSTMAX_PATHLENGTH, CurPos, EndPos))) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            if (STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)!='=') {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            CurPos++;
            if (!STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)) {
               MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break; }
            // We upper-case all directory-names...
            strupr (CurSourceDirPtr->Name);
            if (!(CurPos = STRING_GetNumericValue(&VarID, CurPos, EndPos))) {
               MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric; break; }

            CurSourceDirPtr->Flags = MINSTDIR_Flags_Source;
            if (!MINSTALL_ExtractDirOptions (CurSourceDirPtr)) {
               MSG_SetInsertViaPSZ (2, CurSourceDirPtr->Name);
               MINSTALL_ErrorMsgID = MINSTMSG_BadDirectoryFlags; break; }

            if (MINSTALL_SearchSourceDirID(VarID)) {
               MSG_SetInsertViaPSZ (2, TempString);
               MINSTALL_ErrorMsgID = MINSTMSG_DuplicateDirectoryID; break; }
            CurSourceDirPtr->ID = VarID;
            CurSourceDirPtr++; MCF_SourceDirCount++;
            break;
          case 0x7C4DC759: // destindir
            if (!(CurPos = STRING_GetString(CurDestinDirPtr->Name, MINSTMAX_PATHLENGTH, CurPos, EndPos))) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            if (STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)!='=') {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            CurPos++;
            if (!STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)) {
               MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break; }
            // We upper-case all directory-names...
            strupr (CurDestinDirPtr->Name);
            if (!(CurPos = STRING_GetNumericValue(&VarID, CurPos, EndPos))) {
               MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric; break; }

            CurDestinDirPtr->Flags = 0;
            if (!MINSTALL_ExtractDirOptions (CurDestinDirPtr)) {
               MSG_SetInsertViaPSZ (2, CurDestinDirPtr->Name);
               MINSTALL_ErrorMsgID = MINSTMSG_BadDirectoryFlags; break; }

            if (MINSTALL_SearchDestinDirID(VarID)) {
               MSG_SetInsertViaPSZ (2, TempString);
               MINSTALL_ErrorMsgID = MINSTMSG_DuplicateDirectoryID; break; }
            CurDestinDirPtr->ID = VarID;
            CurDestinDirPtr++; MCF_DestinDirCount++;
            break;
          case 0x59FD2765: // ssgroup
            if (!(CurPos = STRING_GetNumericValue(&VarID, CurPos, EndPos))) {
               MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric; break; }
            if (MINSTALL_SearchGroupID(VarID)) {
               MSG_SetInsertViaPSZ (2, TempString);
               MINSTALL_ErrorMsgID = MINSTMSG_DuplicateGroupID; break; }
            if (MCF_GroupCount>0) CurGroupPtr++;
            MCF_GroupCount++;
            // Reset that structure to NUL
            memset (CurGroupPtr, 0, sizeof(MINSTGRP));
            CurGroupPtr->ID = VarID;
            break;
          case 0xD99224F: // ssname
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->Name, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          case 0x6B4A6F9F: // ssversion
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->Version, MINSTMAX_STRLENGTH, CurPos, EndPos)) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            CurGroupPtr->VersionCode = MINSTALL_GetVersionCode (CurGroupPtr->Version);
            break;
          case 0x362E7592: // ssicon
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->Icon, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          case 0xA67E5483: // ssselect
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(TempString, MINSTMAX_STRLENGTH, CurPos, EndPos)) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            strupr (TempString);
            VarID = CRC32_GetFromPSZ(TempString);
            switch (VarID) {
             case 0x00000000:
             case 0x565CFB9D: CurGroupPtr->AutoSelect = MINSTGRP_Select_Always; break;
             case 0x26A83945: CurGroupPtr->AutoSelect = MINSTGRP_Select_Required; break;
             case 0x80592DD9: CurGroupPtr->AutoSelect = MINSTGRP_Select_Version; break;
             case 0xE3407723: CurGroupPtr->AutoSelect = MINSTGRP_Select_Yes; break;
             case 0xC9402C75: CurGroupPtr->AutoSelect = MINSTGRP_Select_No; break;
             case 0x94636823: CurGroupPtr->AutoSelect = MINSTGRP_Select_BaseNewer; break;
             case 0x598EB31F: CurGroupPtr->AutoSelect = MINSTGRP_Select_OnlyNewer; break;
             default:
               MSG_SetInsertViaPSZ (2, TempString);
               MINSTALL_ErrorMsgID = MINSTMSG_BadSelectValue;
               return FALSE;          // If Select unknown -> exit
             }
            break;
          case 0x3D5C7773: // ssconfigch
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->ConfigFileName, MINSTMAX_STRLENGTH, CurPos, EndPos)) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            strlwr (CurGroupPtr->ConfigFileName);
            break;
          case 0xF77B2010: // ssinich
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->INIFileName, MINSTMAX_STRLENGTH, CurPos, EndPos)) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            strlwr (CurGroupPtr->INIFileName);
            break;
          case 0xB94E8B65: // sscoreqs
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->CoReqs, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          case 0x9E43B251: // ssodinst
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->ODInst, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          case 0x341CCAE4: // ssdll
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->DLLFileName, MINSTMAX_STRLENGTH, CurPos, EndPos)) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            strlwr (CurGroupPtr->DLLFileName); // Lowercase value
            break;
          case 0x81E8FFA6: // sstermdll
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->TermDLLFileName, MINSTMAX_STRLENGTH, CurPos, EndPos)) {
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }
            strlwr (CurGroupPtr->TermDLLFileName); // Lowercase value
            break;
          case 0xB6C7995E: // ssdllentry
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->DLLEntry, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          case 0x787E1B15: // sstermdllentry
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->TermDLLEntry, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          case 0x834F590D: // ssdllinputparms
            if (MCF_GroupCount==0) break;
            if (!STRING_GetString(CurGroupPtr->DLLParms, MINSTMAX_STRLENGTH, CurPos, EndPos))
               MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine;
            break;
          default:
            break;
          }
         if (MINSTALL_ErrorMsgID) break;
       }
      CurPos = LineEndPos+1; CurLineNo++;
    }
   if (MCF_SourceDirCount==0) {
      // If no Sources defined, assume 1 source directory (current)
      MCF_SourceDirCount     = 1;
      CurSourceDirPtr->ID    = 0;
      CurSourceDirPtr->Flags = MINSTDIR_Flags_Source;
      strcpy (CurSourceDirPtr->Name, "\\");
    }

   if (MINSTALL_ErrorMsgID) {               // If Error-found during parsing...
      MSG_SetInsertFileLocation (1, ControlFile->Name, CurLineNo);
      MINSTALL_TrappedError (MINSTALL_ErrorMsgID);
      return FALSE;
    }
   return TRUE;
 }

BOOL MINSTALL_LoadMasterControl (void) {
   // Remember this action for cleanup...
   MINSTALL_Done |= MINSTDONE_LOADMASTERCTRLSCR;

   // Open CONTROL.scr file in SourcePath
   if (!FILE_LoadFileControl(&CONTROLSCR, 131767)) {
      MSG_SetInsertViaPSZ (1, CONTROLSCR.Name);
      MINSTALL_TrappedError (MINSTMSG_CouldNotLoad);
      return FALSE;
    }
   FILE_PreProcessControlFile(&CONTROLSCR);

   // Now we process the control-file contents in 2 steps. First we count
   //  every "sourcedir", "destindir" and "ssgroup"-element.
   MINSTALL_PreProcessMasterControlFile(&CONTROLSCR);

   // Check, if we got at least one of everything...
   if (MCF_GroupCount==0) {
      MINSTALL_TrappedError (MINSTMSG_NoGroupsSpecified); return FALSE;
    } else if ((MCF_SourceDirCount==0) | (MCF_DestinDirCount==0)) {
      MINSTALL_TrappedError (MINSTMSG_NoDirectoriesSpecified); return FALSE;
    }

   // Now we allocate buffers for those variables...
   MCF_GroupArrayPtr = malloc(MCF_GroupCount*MINSTGRP_Length);
   if (MCF_GroupArrayPtr==NULL) {
      MINSTALL_TrappedError (MINSTMSG_OutOfMemory); return FALSE; }
   MCF_SourceDirArrayPtr = malloc(MCF_SourceDirCount*MINSTDIR_Length);
   if (MCF_SourceDirArrayPtr==NULL) {
      MINSTALL_TrappedError (MINSTMSG_OutOfMemory); return FALSE; }
   MCF_DestinDirArrayPtr = malloc(MCF_DestinDirCount*MINSTDIR_Length);
   if (MCF_DestinDirArrayPtr==NULL) {
      MINSTALL_TrappedError (MINSTMSG_OutOfMemory); return FALSE; }

   // ...and read all variables in the actual pass
   if (!MINSTALL_ProcessMasterControlFile(&CONTROLSCR))     return FALSE;
   return TRUE;
 }

// ****************************************************************************

BOOL MINSTALL_LoadFileControl (void) {
   PCHAR      CurPos            = 0;
   PCHAR      EndPos            = 0;
   PCHAR      LineStartPos      = 0;
   PCHAR      LineEndPos        = 0;
   PCHAR      TmpPos            = 0;
   CHAR       CurChar           = 0;
   CHAR       TempString[MINSTMAX_STRLENGTH];
   ULONG      SupposedFileCount = 0;
   ULONG      CurLineNo         = 1;
   BOOL       GotCount          = FALSE;
   PMINSTFILE CurFilePtr        = 0;
   ULONG      CurID             = 0;

   // Reset some variables...
   FCF_FileCount  = 0;

   // Remember this action for cleanup...
   MINSTALL_Done |= MINSTDONE_LOADFILECTRLSCR;

   // Open FILELIST
   if (!FILE_LoadFileControl(&FILELISTSCR, 131767)) {
      MSG_SetInsertViaPSZ (1, FILELISTSCR.Name);
      MINSTALL_TrappedError (MINSTMSG_CouldNotLoad);
      return FALSE;
    }
   FILE_PreProcessControlFile(&FILELISTSCR);

   // Count lines of Control-File (lines with content only)
   SupposedFileCount = FILE_CountControlFileLines (&FILELISTSCR);
   if (SupposedFileCount>0) SupposedFileCount--; // Ignore first-line...
   if (SupposedFileCount==0) {
      FILE_UnLoadFileControl(&FILELISTSCR);
      MINSTALL_TrappedError (MINSTMSG_NoFilesSpecified); return FALSE;
    }

   // Compatibility fix - include 4 extra file entries per group just in case
   // 29.05.2005           cardinfo/installation DLLs/scripts are not specified
   SupposedFileCount += MCF_GroupCount*4;

   // Now we allocate buffers for File-Array...
   FCF_FileArrayPtr = malloc(SupposedFileCount*MINSTFILE_Length);
   if (FCF_FileArrayPtr==NULL) {
      FILE_UnLoadFileControl(&FILELISTSCR);
      MINSTALL_TrappedError (MINSTMSG_OutOfMemory); return FALSE;
    }

   // ...now read the actual data...
   CurPos = FILELISTSCR.BufferPtr; EndPos = FILELISTSCR.BufferEndPtr;
   CurFilePtr = FCF_FileArrayPtr;
   while (CurPos<EndPos) {
      if (!(CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo)))
         break;
      LineStartPos = CurPos;
      LineEndPos   = STRING_GetEndOfLinePtr (CurPos, EndPos);

      // Reset MINSTFILE-Structure...
      memset (CurFilePtr, 0, MINSTFILE_Length);

      if (!STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)) {
         MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break;
       }
      if (!(CurPos = STRING_GetNumericValue(&CurID, CurPos, LineEndPos))) {
         MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric; break;
       }
      if (!GotCount)
         GotCount = TRUE;                // <- skip first valid value...
       else {
         if (!STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)) {
            MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break;
          }
         if (!(CurPos = STRING_GetNumericValue(&CurID, CurPos, LineEndPos))) {
            MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric; break;
          }
         if (!(CurFilePtr->GroupPtr = MINSTALL_SearchGroupID(CurID))) {
            MINSTALL_ErrorMsgID = MINSTMSG_UnknownGroupID; break;
          }
         CurFilePtr->GroupID = CurID;

         if (!STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)) {
            MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break;
          }
         if (!(CurPos = STRING_GetNumericValue(&CurID, CurPos, LineEndPos))) {
            MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric; break;
          }
         if (!(CurFilePtr->DestinPtr = MINSTALL_SearchDestinDirID(CurID))) {
            MINSTALL_ErrorMsgID = MINSTMSG_UnknownDestinID; break;
          }
         CurFilePtr->DestinID = CurID;

         if (!STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)) {
            MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break;
          }
         if (!(CurPos = STRING_GetNumericValue(&CurID, CurPos, LineEndPos))) {
            MINSTALL_ErrorMsgID = MINSTMSG_InvalidNumeric; break;
          }
         if (!(CurFilePtr->SourcePtr = MINSTALL_SearchSourceDirID(CurID))) {
            MINSTALL_ErrorMsgID = MINSTMSG_UnknownSourceID; break;
          }
         CurFilePtr->SourceID = CurID;

         if (!STRING_GetValidChar(&CurPos, LineEndPos, &CurLineNo)) {
            MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break;
          }
         if (!(CurPos = STRING_GetString((PCHAR)CurFilePtr->Name, MINSTMAX_PATHLENGTH, CurPos, LineEndPos))) {
            MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break;
          }

         // We lower-case all filenames...
         strlwr (CurFilePtr->Name);
         CurFilePtr->NameCRC32 = CRC32_GetFromPSZ(CurFilePtr->Name);

         FCF_FileCount++;
         // pointer to next file, cause we are done...
         CurFilePtr++;
       }

      // Set CurPos to end
      CurPos = LineEndPos;
    }

   // Remove that file from memory...
   FILE_UnLoadFileControl(&FILELISTSCR);

   if (MINSTALL_ErrorMsgID) {               // If Error-found during parsing...
      MSG_SetInsertFileLocation (1, FILELISTSCR.Name, CurLineNo);
      MINSTALL_TrappedError (MINSTALL_ErrorMsgID);
      return FALSE;
    }
   return TRUE;
 }

// ****************************************************************************

// Will analyse and included all valid and used files/groups/directories
BOOL MINSTALL_FilterControlFileData (VOID) {
   USHORT      CurNo           = 0;
   PMINSTFILE  CurFilePtr      = FCF_FileArrayPtr;
   PMINSTGRP   CurGroupPtr     = 0;
   PMINSTDIR   CurSourceDirPtr = 0;
   PMINSTDIR   CurDestinDirPtr = MCF_DestinDirArrayPtr;
   ULONG       CRC32           = 0;
   CHAR        TempSourceFile[MINSTMAX_PATHLENGTH];
   FILESTATUS3 CurFileInfo;
   BOOL        IsBannedFile    = FALSE;
   PCHAR       CurPos          = 0;
   PMINSTFILE  InsertFilePtr   = NULL;
   PMINSTDIR   InsertDirPtr    = NULL;

/* obsolete code 13.6.2004 */
/*   FCF_CARDINFOFilePtr         = 0; */
/*   FCF_CARDINFOHandle          = 0; */
   MINSTALL_GeninUsed          = FALSE;
   MINSTALL_IsBaseInstallation = FALSE;

   // Check, if minstall.dll, minstall.msg, minstall.exe & genin.dll are included
   if ((MINSTALL_SearchFileCRC32(0x44E1F798)) && (MINSTALL_SearchFileCRC32(0x11B81B01)) &&
       (MINSTALL_SearchFileCRC32(0x1251F25E)) && (MINSTALL_SearchFileCRC32(0x160F4433)))
      MINSTALL_IsBaseInstallation = TRUE;

   // Check, if path-flags needs to get fixed...
   while (CurNo<MCF_DestinDirCount) {
      // Destination-Directory fixups:
      if (strncmp(CurDestinDirPtr->Name, "\\OS2\\", 5)==0) {
         // "\OS2\*" -> Set BOOT-Flag
         CurDestinDirPtr->Flags |= MINSTDIR_Flags_Boot;
       } else if (strncmp(CurDestinDirPtr->Name, "\\MMOS2\\", 7)==0) {
         // "\MMOS2\* -> Reset BOOT-Flag, set MMBASE-Flag, remove "\MMOS2\"
         CurDestinDirPtr->Flags &= !MINSTDIR_Flags_Boot;
         CurDestinDirPtr->Flags |= MINSTDIR_Flags_MMBase;
         strcpy (CurDestinDirPtr->Name, CurDestinDirPtr->Name+6);
       }
      if (!MINSTALL_FillDirFQName(CurDestinDirPtr))
         return FALSE;                      // Due Buffer-Overflow
      // Include all Destination directories...
      CurDestinDirPtr->Flags |= MINSTDIR_Flags_Included;

      CurDestinDirPtr++; CurNo++;
    }

   // We are processing contents of FILELIST-Script (for error logging)...
   MSG_SetInsertViaPSZ (1, FILELISTSCR.Name);

   // Go through the whole file-listing and check the specified files
   CurNo = 0;
   while (CurNo<FCF_FileCount) {
      // Get Pointer from FILE->GROUP/DIR
      CurGroupPtr     = CurFilePtr->GroupPtr;
      CurSourceDirPtr = CurFilePtr->SourcePtr;
      CurDestinDirPtr = CurFilePtr->DestinPtr;

      // Filter out some files by checking their CRC32
      switch (CurFilePtr->NameCRC32) {
       case 0x46C5EEDD: // geninmri.dll (Generic Installer Text)
       case 0xDBD15D0E: // ibmoptns.dll (??)
       case 0x426A7266: // iterm.dll (Responsible To Generate WPS-Objects)
       case 0xB8E85432: // timer0.sys (Timing PDD)
       case 0x84F52EBF: // clock01.sys (Timing PDD)
       case 0xC355546F: // clock02.sys (Timing PDD)
       case 0x6DDDA492: // resource.sys (Resource Manager)
       case 0x6EE2C9DC: // mmpm2.ini (thx to the morons that did uniaud)
         IsBannedFile = TRUE;
         break;
       case 0x160F4433: // genin.dll (Generic Installer Code)
       case 0x1251F25E: // minstall.exe
       case 0xE29E8FAA: // audioif.dll
       case 0xFD7E670C: // ampmxmcd.dll
       case 0xFFE31036: // audiosh.dll
         // Install those one only, when being on base installation
         if (!MINSTALL_IsBaseInstallation)
            IsBannedFile = TRUE;
         break;
       default:
         IsBannedFile = FALSE;
         break;
       }

      if (!IsBannedFile) {
         // Source-Directory Pre-Processing
         //========================================
         // We check, if specified directory exists and fix bad directories
         if (!(CurSourceDirPtr->Flags & MINSTDIR_Flags_Included)) {
            if (!MINSTALL_FillDirFQName(CurSourceDirPtr))
               return FALSE;                // Due Buffer-Overflow
          }

         switch (CurFilePtr->NameCRC32) {
           case 0x15ED86B8: // cardinfo.dll (Generic Installer Custom Data)
            // This file won't get copied, but used by GENIN-compatibility code */
/* obsolete code 13.6.2004 */
/*            // Remember this file for later, but it wont get copied... */
/*            FCF_CARDINFOFilePtr = CurFilePtr; */
            break;

           default:
            CurFilePtr->Flags      |= MINSTFILE_Flags_Included;
            CurSourceDirPtr->Flags |= MINSTDIR_Flags_Included;

            if (!STRING_CombinePSZ ((PCHAR)&TempSourceFile, MINSTMAX_PATHLENGTH, CurSourceDirPtr->FQName, CurFilePtr->Name))
               return FALSE;
            if (DosQueryPathInfo (TempSourceFile, FIL_STANDARD, &CurFileInfo, sizeof(CurFileInfo))) {
               MSG_SetInsertViaPSZ (2, TempSourceFile);
               MINSTALL_TrappedError (MINSTMSG_CouldNotFindSourceFile);
               return FALSE;                // File not found or problem
             }
            CurGroupPtr->SpaceNeeded += CurFileInfo.cbFile;
          }
       }
      CurFilePtr++; CurNo++;
    }

   // We are processing contents of CONTROL-Script (for error logging)...
   MSG_SetInsertViaPSZ (1, CONTROLSCR.Name);

   CurGroupPtr = MCF_GroupArrayPtr; CurNo = 0;
   while (CurNo<MCF_GroupCount) {
      //  GroupID Processing
      // ====================

      if (CurGroupPtr->ID==0) {
         // If we find group 0, we will set some defaults and automatic select
         //  that group for installation (so the files of group 0 will get
         //  copied anytime). We also set some hard-coded values for safety.
         CurGroupPtr->Flags |= MINSTGRP_Flags_Selected;
         CurGroupPtr->Flags |= MINSTGRP_Flags_DontListPublic;
         strcpy (CurGroupPtr->Name, "mmbase");
         strcpy (CurGroupPtr->Version, "1.1.5");
         CurGroupPtr->VersionCode = 0x010105;
       } else {
         //  Installed-Version Checking and Pre-Selection
         // ==============================================
         // If the group id is not 0, we check if the group is already
         //  installed and process CurGroupPtr->Select to find out, if this
         //  group shall be shown in Public-Group. Also check, if it needs to
         //  get preselected.

         MINSTALL_GetInstalledVersion (CurGroupPtr->Name, CurGroupPtr->VersionInstalled);
         CurGroupPtr->VersionInstalledCode = MINSTALL_GetVersionCode (CurGroupPtr->VersionInstalled);

         switch (CurGroupPtr->AutoSelect) {
          case MINSTGRP_Select_Always:
            // Preselect, user may not choose
            CurGroupPtr->Flags |= MINSTGRP_Flags_Selected;
            CurGroupPtr->Flags |= MINSTGRP_Flags_SelectionForced;
            break;
          case MINSTGRP_Select_Required:
            // Preselect, user may only choose, if package was not installed
            CurGroupPtr->Flags |= MINSTGRP_Flags_Selected;
            if (strlen(CurGroupPtr->VersionInstalled)>0)
               CurGroupPtr->Flags |= MINSTGRP_Flags_SelectionForced;
            break;
          case MINSTGRP_Select_Version:
            // Preselected, if already installed and version older, user may
            //  choose anyway.
            if (strlen(CurGroupPtr->VersionInstalled)>0)
               // 10.02.2006 - Fixed, did only check if anything is already
               //               installed and then preselected the package.
               if (CurGroupPtr->VersionInstalledCode<CurGroupPtr->VersionCode)
                  CurGroupPtr->Flags |= MINSTGRP_Flags_Selected;
            break;
          case MINSTGRP_Select_Yes:
            // Preselected, but let user choose
            CurGroupPtr->Flags |= MINSTGRP_Flags_Selected;
            break;
          case MINSTGRP_Select_No:
            // Not preselected, user may choose
            break;
          case MINSTGRP_Select_BaseNewer:
            // Preselected only if group not installed or installed version
            //  older. Otherwise group will get forced to deselect-state
            if (strlen(CurGroupPtr->VersionInstalled)==0) {
               CurGroupPtr->Flags |= MINSTGRP_Flags_Selected;
             } else if (CurGroupPtr->VersionInstalledCode<CurGroupPtr->VersionCode) {
               CurGroupPtr->Flags |= MINSTGRP_Flags_Selected;
             } else {
               CurGroupPtr->Flags |= MINSTGRP_Flags_SelectionForced;
             }
            break;
          case MINSTGRP_Select_OnlyNewer:
            // Preselected only, if group already installed, but version older
            //  or equal to current version. Otherwise forced-deselect.
            if ((strlen(CurGroupPtr->VersionInstalled)!=0) && (CurGroupPtr->VersionInstalledCode<=CurGroupPtr->VersionCode)) {
               CurGroupPtr->Flags |= MINSTGRP_Flags_Selected;
             } else {
               CurGroupPtr->Flags |= MINSTGRP_Flags_SelectionForced;
             }
          }
       }

      // DLL and TermDLL Processing
      //============================
      // This code checks for specific DLLs and aborts installation if a banned
      //  one was found. Also it will check for the DLL(s) to be listed in file-
      //  listing and fail, if it isn't found.

      CRC32 = CRC32_GetFromPSZ(CurGroupPtr->DLLFileName);
      switch (CRC32) {
//       case 0x160F4432: // genin.dll (Generic Installation)
// DEBUGDEBUGDEBUG
       case 0x160F4433: // genin.dll (Generic Installation)
         if (strcmp(CurGroupPtr->DLLEntry, "InitGenin")==0) {
            CurGroupPtr->GeninID = atol(CurGroupPtr->DLLParms);
            /* Extract "cardinfo.dll" name, if available as well (13.06.2004) */
            /* Undocumented feature by IBM */
            CurPos = CurGroupPtr->DLLParms;
            while ((*CurPos!=0) && (*CurPos!=','))
               CurPos++;                 /* Search for ';' */
            if (*CurPos==',') {
               /* We found a "cardinfo.dll" name */
               CurPos++;
               /* Append '.dll' to name and lookup in files.scr table */
               STRING_CombinePSZ(CurGroupPtr->GeninDLLFileName, MINSTMAX_PATHLENGTH, CurPos, ".dll");
               strlwr(CurGroupPtr->GeninDLLFileName);
             } else {
               strcpy(CurGroupPtr->GeninDLLFileName, "cardinfo.dll");
             }
            /* Now search the "cardinfo.dll" file in file-listing */
            CRC32 = CRC32_GetFromPSZ(CurGroupPtr->GeninDLLFileName);
            if ((CurFilePtr = MINSTALL_SearchFileCRC32(CRC32))!=0) {
               // Got that file, set pointer to file...
               CurGroupPtr->GeninDLLFilePtr = CurFilePtr;
             } else {
               MSG_SetInsertViaPSZ (1, CurGroupPtr->GeninDLLFileName);
               // Check, if that file exists in source directory...
               if (!STRING_CombinePSZ ((PCHAR)&TempSourceFile, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_SourceDir, CurGroupPtr->GeninDLLFileName))
                  return FALSE;
               if (DosQueryPathInfo (TempSourceFile, FIL_STANDARD, &CurFileInfo, sizeof(CurFileInfo))) {
                  MINSTALL_TrappedError (MINSTMSG_CARDINFOnotFound); return FALSE;
                }
               // Compatibility fix - we add "cardinfo.dll" files to our
               //                      file-listing manually if they are not
               //                      mentioned.
               InsertFilePtr = FCF_FileArrayPtr;
               InsertFilePtr += FCF_FileCount; // Seek over last entry
               strcpy(InsertFilePtr->Name, CurGroupPtr->GeninDLLFileName);
               InsertFilePtr->NameCRC32 = CRC32;
               InsertFilePtr->Flags     = 0; // Not included nor selected
               InsertFilePtr->DiskID    = 0;
               InsertFilePtr->GroupID   = CurGroupPtr->ID;
               InsertFilePtr->GroupPtr  = CurGroupPtr;
               // We just use first Destin-Dir - don't care, because the file
               //  is never copied...
               InsertFilePtr->DestinID  = MCF_DestinDirArrayPtr->ID;
               InsertFilePtr->DestinPtr = MCF_DestinDirArrayPtr;
               // We search for the '\' source directory here and use that
               if (!(InsertDirPtr = MINSTALL_SearchRootSourceDirID())) {
                  // Really bad case, root directory was not found, so abort
                  MINSTALL_TrappedError (MINSTMSG_CARDINFOnotFound); return FALSE;
                }
               InsertFilePtr->SourceID  = InsertDirPtr->ID;
               InsertFilePtr->SourcePtr = InsertDirPtr;
               FCF_FileCount++;

               MINSTALL_TrappedWarning (MINSTMSG_CARDINFOunlistedWarning);
             }
            MINSTALL_GeninUsed   = TRUE;
          }
         break;
       case 0x5A2E7E78: // cwinst.dll       <-- Bad-ass Crystal drivers found
         if (strcmp(CurGroupPtr->DLLEntry, "CWINSTENTRY")==0) {
            MINSTALL_TrappedError (MINSTMSG_BannedDriverSet); return FALSE;
          }
         break;
       // Don't check those DLLs...
       case 0x00000000: // no DLL
       // F6F9DE7D qrycd.dll?!?!?
         break;
       case 0xCBDED9CC: // dialog.dll       <-- Bad-ass OPTi drivers found
         if (strcmp(CurGroupPtr->DLLEntry, "Dialog")==0) {
            // This is OPTi at work, so use the DLLParms as GeninID
            //  this custom DLL calls genin directly and this won't work here
            //  so we have to know it now and process it, still we also run
            //  the custom DLL as well.
            CurGroupPtr->GeninID = atol(CurGroupPtr->DLLParms);
            MINSTALL_GeninUsed   = TRUE;
          }
       default:
         if (!(CurFilePtr = MINSTALL_SearchFileCRC32(CRC32))) {
            MSG_SetInsertViaPSZ (2, CurGroupPtr->DLLFileName);
            // Check, if that file exists in source directory...
            if (!STRING_CombinePSZ ((PCHAR)&TempSourceFile, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_SourceDir, CurGroupPtr->DLLFileName))
               return FALSE;
            if (DosQueryPathInfo (TempSourceFile, FIL_STANDARD, &CurFileInfo, sizeof(CurFileInfo))) {
               MINSTALL_TrappedError (MINSTMSG_InstallDLLnotFound); return FALSE;
             }
            // Compatibility fix - we add the Term-DLL to our file-listing
            //                      manually if they are not mentioned.
            InsertFilePtr = FCF_FileArrayPtr;
            InsertFilePtr += FCF_FileCount; // Seek over last entry
            strcpy(InsertFilePtr->Name, CurGroupPtr->DLLFileName);
            InsertFilePtr->NameCRC32 = CRC32;
            InsertFilePtr->Flags     = 0; // Not included nor selected
            InsertFilePtr->DiskID    = 0;
            InsertFilePtr->GroupID   = CurGroupPtr->ID;
            InsertFilePtr->GroupPtr  = CurGroupPtr;
            // We just use first Destin-Dir - don't care, because the file
            //  is never copied...
            InsertFilePtr->DestinID  = MCF_DestinDirArrayPtr->ID;
            InsertFilePtr->DestinPtr = MCF_DestinDirArrayPtr;
            // We search for the '\' source directory here and use that
            if (!(InsertDirPtr = MINSTALL_SearchRootSourceDirID())) {
               // Really bad case, root directory was not found, so abort
               MINSTALL_TrappedError (MINSTMSG_InstallDLLnotFound); return FALSE;
             }
            InsertFilePtr->SourceID  = InsertDirPtr->ID;
            InsertFilePtr->SourcePtr = InsertDirPtr;
            CurFilePtr               = InsertFilePtr;
            FCF_FileCount++;

            MINSTALL_TrappedWarning (MINSTMSG_InstallDLLunlistedWarning);
          }
         switch (CRC32) {
           case 0x0178000F: // mmsnd.dll (base installation)
             break;
           default:
            CurFilePtr->Flags          &= !MINSTFILE_Flags_Included;
          }
         CurFilePtr->Flags      |= MINSTFILE_Flags_INSTDLL;
         CurGroupPtr->DLLFilePtr = CurFilePtr;
       }

      CRC32 = CRC32_GetFromPSZ(CurGroupPtr->TermDLLFileName);
      switch (CRC32) {
       case 0x426A7266: // iterm.dll
         if (strcmp(CurGroupPtr->TermDLLEntry, "ITermEntry")==0) {
            // Filter out that DLL. It issues WPS-object regeneration of all
            //  the stupid players and stuff that we want to get rid off.
            CurGroupPtr->TermDLLFileName[0] = 0;
          }
       // Don't check those DLLs...
       case 0x00000000: // no DLL
         break;
       default:
         if (!(CurFilePtr = MINSTALL_SearchFileCRC32(CRC32))) {
            MSG_SetInsertViaPSZ (2, CurGroupPtr->TermDLLFileName);
            // Check, if that file exists in source directory...
            if (!STRING_CombinePSZ ((PCHAR)&TempSourceFile, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_SourceDir, CurGroupPtr->TermDLLFileName))
               return FALSE;
            if (DosQueryPathInfo (TempSourceFile, FIL_STANDARD, &CurFileInfo, sizeof(CurFileInfo))) {
               MINSTALL_TrappedError (MINSTMSG_InstallDLLnotFound); return FALSE;
             }
            // Compatibility fix - we add the Term-DLL to our file-listing
            //                      manually if they are not mentioned.
            InsertFilePtr = FCF_FileArrayPtr;
            InsertFilePtr += FCF_FileCount; // Seek over last entry
            strcpy(InsertFilePtr->Name, CurGroupPtr->GeninDLLFileName);
            InsertFilePtr->NameCRC32 = CRC32;
            InsertFilePtr->Flags     = 0; // Not included nor selected
            InsertFilePtr->DiskID    = 0;
            InsertFilePtr->GroupID   = CurGroupPtr->ID;
            InsertFilePtr->GroupPtr  = CurGroupPtr;
            // We just use first Destin-Dir - don't care, because the file
            //  is never copied...
            InsertFilePtr->DestinID  = MCF_DestinDirArrayPtr->ID;
            InsertFilePtr->DestinPtr = MCF_DestinDirArrayPtr;
            // We search for the '\' source directory here and use that
            if (!(InsertDirPtr = MINSTALL_SearchRootSourceDirID())) {
               // Really bad case, root directory was not found, so abort
               MINSTALL_TrappedError (MINSTMSG_InstallDLLnotFound); return FALSE;
             }
            InsertFilePtr->SourceID  = InsertDirPtr->ID;
            InsertFilePtr->SourcePtr = InsertDirPtr;
            CurFilePtr               = InsertFilePtr;
            FCF_FileCount++;

            MINSTALL_TrappedWarning (MINSTMSG_InstallDLLunlistedWarning);
          }
         CurFilePtr->Flags          &= !MINSTFILE_Flags_Included;
         CurFilePtr->Flags          |= MINSTFILE_Flags_INSTTermDLL;
         CurGroupPtr->TermDLLFilePtr = CurFilePtr;
       }

      //  INI & Config Control File Processing
      // ======================================
      // If Control Files are specified, we remove "Included", so they wont get
      //  copied to the given destination. We tag both types with special
      //  flags so that they can get processed separately later.

      CRC32 = CRC32_GetFromPSZ(CurGroupPtr->ConfigFileName);
      if (CRC32) {
         if (!(CurFilePtr = MINSTALL_SearchFileCRC32(CRC32))) {
            MSG_SetInsertViaPSZ (2, CurGroupPtr->ConfigFileName);
            // Check, if that file exists in source directory...
            if (!STRING_CombinePSZ ((PCHAR)&TempSourceFile, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_SourceDir, CurGroupPtr->ConfigFileName))
               return FALSE;
            if (DosQueryPathInfo (TempSourceFile, FIL_STANDARD, &CurFileInfo, sizeof(CurFileInfo))) {
               MINSTALL_TrappedError (MINSTMSG_UnlistedScript); return FALSE;
             }
            // Compatibility fix - we add the Config-File to our file-listing
            //                      manually if they are not mentioned.
            InsertFilePtr = FCF_FileArrayPtr;
            InsertFilePtr += FCF_FileCount; // Seek over last entry
            strcpy(InsertFilePtr->Name, CurGroupPtr->ConfigFileName);
            InsertFilePtr->NameCRC32 = CRC32;
            InsertFilePtr->Flags     = 0; // Not included nor selected
            InsertFilePtr->DiskID    = 0;
            InsertFilePtr->GroupID   = CurGroupPtr->ID;
            InsertFilePtr->GroupPtr  = CurGroupPtr;
            // We just use first Destin-Dir - don't care, because the file
            //  is never copied...
            InsertFilePtr->DestinID  = MCF_DestinDirArrayPtr->ID;
            InsertFilePtr->DestinPtr = MCF_DestinDirArrayPtr;
            // We search for the '\' source directory here and use that
            if (!(InsertDirPtr = MINSTALL_SearchRootSourceDirID())) {
               // Really bad case, root directory was not found, so abort
               MINSTALL_TrappedError (MINSTMSG_UnlistedScript); return FALSE;
             }
            InsertFilePtr->SourceID  = InsertDirPtr->ID;
            InsertFilePtr->SourcePtr = InsertDirPtr;
            CurFilePtr               = InsertFilePtr;
            FCF_FileCount++;

            MINSTALL_TrappedWarning (MINSTMSG_UnlistedScriptWarning);
          }
         CurGroupPtr->ConfigFilePtr = CurFilePtr;
         CurFilePtr->Flags         &= !MINSTFILE_Flags_Included;
         CurFilePtr->Flags         |= MINSTFILE_Flags_CFGCF;
         /* We set the group of that file to the current group */
         /*  Normally files.scr should contain the correct group already! */
         CurFilePtr->GroupPtr       = CurGroupPtr;
       }

      CRC32 = CRC32_GetFromPSZ(CurGroupPtr->INIFileName);
      if (CRC32) {
         if (!(CurFilePtr = MINSTALL_SearchFileCRC32(CRC32))) {
            MSG_SetInsertViaPSZ (2, CurGroupPtr->INIFileName);
            // Check, if that file exists in source directory...
            if (!STRING_CombinePSZ ((PCHAR)&TempSourceFile, MINSTMAX_PATHLENGTH, (PCHAR)MINSTALL_SourceDir, CurGroupPtr->INIFileName))
               return FALSE;
            if (DosQueryPathInfo (TempSourceFile, FIL_STANDARD, &CurFileInfo, sizeof(CurFileInfo))) {
               MINSTALL_TrappedError (MINSTMSG_UnlistedScript); return FALSE;
             }
            // Compatibility fix - we add the INI-File to our file-listing
            //                      manually if they are not mentioned.
            InsertFilePtr = FCF_FileArrayPtr;
            InsertFilePtr += FCF_FileCount; // Seek over last entry
            strcpy(InsertFilePtr->Name, CurGroupPtr->INIFileName);
            InsertFilePtr->NameCRC32 = CRC32;
            InsertFilePtr->Flags     = 0; // Not included nor selected
            InsertFilePtr->DiskID    = 0;
            InsertFilePtr->GroupID   = CurGroupPtr->ID;
            InsertFilePtr->GroupPtr  = CurGroupPtr;
            // We just use first Destin-Dir - don't care, because the file
            //  is never copied...
            InsertFilePtr->DestinID  = MCF_DestinDirArrayPtr->ID;
            InsertFilePtr->DestinPtr = MCF_DestinDirArrayPtr;
            // We search for the '\' source directory here and use that
            if (!(InsertDirPtr = MINSTALL_SearchRootSourceDirID())) {
               // Really bad case, root directory was not found, so abort
               MINSTALL_TrappedError (MINSTMSG_UnlistedScript); return FALSE;
             }
            InsertFilePtr->SourceID  = InsertDirPtr->ID;
            InsertFilePtr->SourcePtr = InsertDirPtr;
            CurFilePtr               = InsertFilePtr;
            FCF_FileCount++;

            MINSTALL_TrappedWarning (MINSTMSG_UnlistedScriptWarning);
          }
         CurGroupPtr->INIFilePtr = CurFilePtr;
         CurFilePtr->Flags      &= !MINSTFILE_Flags_Included;
         CurFilePtr->Flags      |= MINSTFILE_Flags_INICF;
         /* We set the group of that file to the current group */
         /*  Normally files.scr should contain the correct group already! */
         CurFilePtr->GroupPtr       = CurGroupPtr;
       }

      // Include all groups...
      CurGroupPtr->Flags        |= MINSTGRP_Flags_Included;
      CurGroupPtr++; CurNo++;
    }

   /* Obsolete code 13.6.2004 */
/*   if ((!FCF_CARDINFOFilePtr) && (MINSTALL_GeninUsed)) { */
/*      MINSTALL_TrappedError (MINSTMSG_CARDINFOrequiredForGENIN); */
/*      return FALSE;                // if DLL not found, but GENIN used... */
/*    } */

   return TRUE;
 }

BOOL MINSTALL_GeneratePublicGroup (void) {
   PMINSTPUBGROUP CurPublicGroup = 0;
   PMINSTGRP      CurGroupPtr    = MCF_GroupArrayPtr;
   ULONG          CurGroupNo     = 0;
   ULONG          CurPromptNo    = 0;
   ULONG          RequiredSpace  = 0;

   MINSTALL_PublicGroupCount = 0;

   // Count all groups that are listed in Public-Groups
   while (CurGroupNo<MCF_GroupCount) {
      if (!(CurGroupPtr->Flags & MINSTGRP_Flags_DontListPublic)) {
         MINSTALL_PublicGroupCount++;
       }
      CurGroupNo++; CurGroupPtr++;
    }

   if (MINSTALL_PublicGroupCount) {
      RequiredSpace = MINSTALL_PublicGroupCount*sizeof(MINSTPUBGROUP);
      CurPublicGroup = malloc(RequiredSpace);
      if (!CurPublicGroup) {
         MINSTALL_TrappedError (MINSTMSG_OutOfMemory); return FALSE; }

      MINSTALL_PublicGroupArrayPtr = CurPublicGroup;
      memset (CurPublicGroup, 0, RequiredSpace);

      // Take data into Public-Group...
      CurGroupNo = 0; CurGroupPtr = MCF_GroupArrayPtr;
      while (CurGroupNo<MCF_GroupCount) {
         if (!(CurGroupPtr->Flags & MINSTGRP_Flags_DontListPublic)) {
            CurPublicGroup->ID = CurGroupPtr->ID;
            if (CurGroupPtr->Flags & MINSTGRP_Flags_Selected)
               CurPublicGroup->Selected = TRUE;
            if (CurGroupPtr->Flags & MINSTGRP_Flags_SelectionForced)
               CurPublicGroup->SelectionForced = TRUE;
            strcpy (CurPublicGroup->Name, CurGroupPtr->Name);
            strcpy (CurPublicGroup->Version, CurGroupPtr->Version);
            strcpy (CurPublicGroup->VersionInstalled, CurGroupPtr->VersionInstalled);
            CurPublicGroup->SpaceNeeded = CurGroupPtr->SpaceNeeded;
            if (CurGroupPtr->GeninPtr) {
               CurPublicGroup->MaxCardCount = CurGroupPtr->GeninPtr->MaxCardCount;
               CurPublicGroup->PromptsCount = CurGroupPtr->GeninPtr->PromptsCount;
               for (CurPromptNo=0; CurPromptNo<MINSTMAX_GENINPROMPTS; CurPromptNo++) {
                  CurPublicGroup->PromptTitlePtr[CurPromptNo] = CurGroupPtr->GeninPtr->PromptTitlePtr[CurPromptNo];
                  CurPublicGroup->PromptChoiceCount[CurPromptNo] = CurGroupPtr->GeninPtr->PromptChoiceCount[CurPromptNo];
                  CurPublicGroup->PromptChoiceDefault[CurPromptNo] = CurGroupPtr->GeninPtr->PromptChoiceDefault[CurPromptNo];
                  CurPublicGroup->PromptChoiceStrings[CurPromptNo] = CurGroupPtr->GeninPtr->PromptChoiceStrings[CurPromptNo];
                }
             }
            CurPublicGroup++;
          }
         CurGroupNo++; CurGroupPtr++;
       }
      MINSTALL_Done |= MINSTDONE_PUBLICGROUP;
    }
   return TRUE;
 }

// Simply cycles through all selected groups and saves version information into
//  COMPLIST.INI file. Returning a BOOL is done to remain in overall design,
//  also if this function is later extended, one may abort installation this
//  way.
BOOL MINSTALL_SaveInstalledVersions (void) {
   PMINSTGRP      CurGroupPtr    = MCF_GroupArrayPtr;
   ULONG          CurGroupNo     = 0;

   while (CurGroupNo<MCF_GroupCount) {
      if (CurGroupPtr->Flags & MINSTGRP_Flags_Selected) {
         MINSTALL_SetInstalledVersion(CurGroupPtr->Name, CurGroupPtr->Version);
       }
      CurGroupNo++; CurGroupPtr++;
    }
   return TRUE;
 }

VOID MINSTALL_CleanUpMasterControl (void) {
   // Just release all memory-blocks...
   free (MCF_GroupArrayPtr);     MCF_GroupCount     = 0;
   free (MCF_SourceDirArrayPtr); MCF_SourceDirCount = 0;
   free (MCF_DestinDirArrayPtr); MCF_DestinDirCount = 0;

   // Remove this action from cleanup...
   MINSTALL_Done &= !MINSTDONE_LOADMASTERCTRLSCR;
 }

VOID MINSTALL_CleanUpFileControl (VOID) {
   // Just release all memory-blocks...
   free (FCF_FileArrayPtr); FCF_FileCount = 0;

   // Remove this action from cleanup...
   MINSTALL_Done &= !MINSTDONE_LOADFILECTRLSCR;
 }

VOID MINSTALL_CleanUpPublicGroup (void) {
   free (MINSTALL_PublicGroupArrayPtr); MINSTALL_PublicGroupArrayPtr = 0;

   // Remove this action from cleanup...
   MINSTALL_Done &= !MINSTDONE_PUBLICGROUP;
 }
