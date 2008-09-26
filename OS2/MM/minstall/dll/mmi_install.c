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
#include <os2.h>

#include <malloc.h>
#include <string.h>

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
#include <mmi_cardinfo.h>
#include <mmi_ctrlprc.h>
#include <mmi_msg.h>

VOID MINSTALL_SelectFiles (VOID) {
   PMINSTFILE CurFilePtr  = FCF_FileArrayPtr;
   PMINSTGRP  CurGroupPtr = MCF_GroupArrayPtr;
   USHORT        CurNo       = 0;

   MINSTCID_FileCount  = 0;
   MINSTCID_GroupCount = 0;

   // Count selected groups...
   while (CurNo<MCF_GroupCount) {
      if (CurGroupPtr->Flags & MINSTGRP_Flags_Selected)
         MINSTCID_GroupCount++;
      CurGroupPtr++; CurNo++;
    }

   // Select files & Directories depending on selected groups...
   CurNo = 0;
   while (CurNo<FCF_FileCount) {
      if (CurFilePtr->GroupPtr->Flags & MINSTGRP_Flags_Selected) {
         CurFilePtr->Flags               |= MINSTFILE_Flags_Selected;
         CurFilePtr->SourcePtr->Flags    |= MINSTDIR_Flags_Selected;
         CurFilePtr->DestinPtr->Flags    |= MINSTDIR_Flags_Selected;

         // We count all MINSTCID-DLLs together...
         if ((CurFilePtr->Flags & MINSTFILE_Flags_INSTDLL) || (CurFilePtr->Flags & MINSTFILE_Flags_INSTTermDLL))
            MINSTCID_FileCount++;
       }
      CurFilePtr++; CurNo++;
    }
 }

// Will create destination directories...
BOOL MINSTALL_CreateDestinDirectories (VOID) {
   PMINSTDIR  CurDirPtr     = MCF_DestinDirArrayPtr;
   USHORT        CurDirNo      = 0;

   while (CurDirNo<MCF_DestinDirCount) {
      if (CurDirPtr->Flags & MINSTDIR_Flags_Selected) {
         if (CurDirPtr->FQName[0]!=0) {
            if (!FILE_CreateDirectory ((PCHAR)CurDirPtr->FQName)) {
               MSG_SetInsertViaPSZ (1, CurDirPtr->FQName);
               MINSTALL_TrappedError (MINSTMSG_CouldNotCreateDirectory);
               return FALSE;                // If no success
             }
          }
       }
      CurDirPtr++; CurDirNo++;
    }
   return TRUE;
 }

// Will copy source files to destination...
BOOL MINSTALL_CopyFiles (VOID) {
   PMINSTFILE CurFilePtr      = FCF_FileArrayPtr;
   USHORT     CurNo           = 0;
   PMINSTDIR  CurSourceDirPtr = 0;
   PMINSTGRP  CurGroupPtr     = 0;
   CHAR       ChangeControlFile[13];
   CHAR       TempSourceFile[MINSTMAX_PATHLENGTH];
   CHAR       TempDestinFile[MINSTMAX_PATHLENGTH];
   BOOL       AnythingDelayed = FALSE;
   APIRET     rc;
   CFGSYSRET  CfgSysRC;

   while (CurNo<FCF_FileCount) {
      if (CurFilePtr->Flags & MINSTFILE_Flags_Selected) {
         // This file is selected, so process it...
         if (!STRING_CombinePSZ ((PCHAR)&TempSourceFile, MINSTMAX_PATHLENGTH, CurFilePtr->SourcePtr->FQName, CurFilePtr->Name))
            return FALSE;

         if (CurFilePtr->Flags & MINSTFILE_Flags_Included) {
            // Is Included, so copy this file to its desired destination...
            if (!STRING_CombinePSZ ((PCHAR)&TempDestinFile, MINSTMAX_PATHLENGTH, CurFilePtr->DestinPtr->FQName, CurFilePtr->Name))
               return FALSE;
            MINSTLOG_ToAll (" -> %s\n", TempDestinFile);
            // Remove R/O attribute, if currently set
            FILE_ResetAttributes (TempDestinFile);
            // Copy file and replace if already there...
            rc = FILE_Replace (TempSourceFile, TempDestinFile);
            switch (rc) {
              case ERROR_SHARING_VIOLATION:
               CfgSysRC = CONFIGSYS_DelayCopyFile (TempSourceFile, TempDestinFile);
               MINSTLOG_ToFile ("Delaying RC %d\n", CfgSysRC);
               if (CfgSysRC!=CONFIGSYS_DONE) {
                  MSG_SetInsertViaPSZ (1, TempDestinFile);
                  MINSTALL_TrappedError (MINSTMSG_CouldNotCopyToFile);
                  return FALSE;                // Problem during Copying
                }
               AnythingDelayed = TRUE;
               break;
              case ERROR_DISK_FULL:
               MINSTALL_TrappedError (MINSTMSG_DiskFull);
               return FALSE;                   // Problem during Copying
              case 0:
               break;
              default:
               MSG_SetInsertViaPSZ (1, TempSourceFile);
               MINSTALL_TrappedError (MINSTMSG_CouldNotAccessFile);
               return FALSE;                   // Problem during Copy
             }
          }
       }
      CurFilePtr++; CurNo++;
    }

   if (AnythingDelayed) {
      if (CONFIGSYS_DelayFinalize()!=CONFIGSYS_DONE)
         return FALSE;
      MINSTLOG_ToAll ("Files got delayed. System should reboot.\n");
    }
   return TRUE;
 }
