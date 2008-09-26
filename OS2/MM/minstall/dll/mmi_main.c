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
// #define INCL_OS2MM
#include <os2.h>
// #include <os2me.h>

#include <malloc.h>
#include <string.h>

#include <global.h>
#include <cfgsys.h>
#include <cid.h>
#include <crcs.h>
#include <dll.h>
#include <file.h>
#include <globstr.h>
#include <msg.h>
#include <mmi_public.h>
#include <mmi_types.h>
#include <mmi_main.h>
#include <mmi_helper.h>
#include <mmi_basescr.h>
#include <mmi_ctrlscr.h>
#include <mmi_ctrlprc.h>
#include <mmi_cardinfo.h>
#include <mmi_install.h>
#include <mmi_msg.h>

#include <mmi_inistuff.h>
#include <mmi_imports.h>
#include <mmi_customdll.h>

FILECONTROL     CONTROLSCR;
FILECONTROL     FILELISTSCR;
FILECONTROL     CHANGESCR;

HAB             MINSTALL_PMHandle           = NULLHANDLE;
HMQ             MINSTALL_MSGQHandle         = NULLHANDLE;
HMTX            MINSTALL_MutexSemamorph     = NULLHANDLE;
CHAR            MINSTALL_MMBase[MINSTMAX_PATHLENGTH];
CHAR            MINSTALL_BootDrive[3];
CHAR            MINSTALL_BootLetter[2];
CHAR            MINSTALL_MMBaseDrive[3];
CHAR            MINSTALL_MMBaseLetter[2];
// Path means pathname, Dir means Directory (directory includes ending '\'
CHAR            MINSTALL_SourcePath[MINSTMAX_PATHLENGTH];
CHAR            MINSTALL_SourceDir[MINSTMAX_PATHLENGTH];
CHAR            MINSTALL_InstallPath[MINSTMAX_PATHLENGTH];
CHAR            MINSTALL_InstallDir[MINSTMAX_PATHLENGTH];
CHAR            MINSTALL_DLLDir[MINSTMAX_PATHLENGTH];
CHAR            MINSTALL_CompListINI[MINSTMAX_PATHLENGTH];

CHAR            MINSTLOG_FileName[MINSTMAX_PATHLENGTH];
FILE           *MINSTLOG_FileHandle         = 0;

CHAR            MINSTALL_TempMacroSpace[MINSTMAX_PATHLENGTH];

ULONG           MINSTALL_PublicGroupCount   = 0;
PMINSTPUBGROUP  MINSTALL_PublicGroupArrayPtr = 0;

// Contains all processed things
ULONG           MINSTALL_Done               = 0;
ULONG           MINSTALL_ErrorMsgID         = 0;
CHAR            MINSTALL_ErrorMsg[1024];
CHAR            MINSTALL_CIDDescription[1024];

// Master-Control-File Variables...
USHORT          MCF_GroupCount              = 0; // Total count in MCF_GroupArray
PMINSTGRP       MCF_GroupArrayPtr           = 0;
USHORT          MCF_SourceDirCount          = 0; //  ... in MCF_SourceDirArray
PMINSTDIR       MCF_SourceDirArrayPtr       = 0;
USHORT          MCF_DestinDirCount          = 0; //  ... in MCF_DestinDirArray
PMINSTDIR       MCF_DestinDirArrayPtr       = 0;
USHORT          FCF_FileCount               = 0; //  ... in FCF_FileArray
PMINSTFILE      FCF_FileArrayPtr            = 0;
CHAR            MCF_PackageName[MINSTMAX_STRLENGTH];
ULONG           MCF_CodePage                = 0;
ULONG           MCF_MUnitCount              = 0;
CHAR            MCF_Medianame[MINSTMAX_STRLENGTH];

BOOL            MINSTALL_GeninUsed          = FALSE;
BOOL            MINSTALL_IsBaseInstallation = FALSE;
// This is set till first MINSTALL_Init() was completed.
BOOL            MINSTALL_IsFirstInit        = TRUE;
BOOL            MINSTALL_SystemShouldReboot = FALSE;

/* HMODULE         FCF_CARDINFOHandle          = 0; */
/* PMINSTFILE      FCF_CARDINFOFilePtr         = 0; */
HMODULE         FCF_LastCARDINFOHandle      = NULLHANDLE;

// Custom-API related public variables...
HEV             CustomAPI_InitEventHandle   = 0;
TID             CustomAPI_ThreadID          = 0;
BOOL            CustomAPI_ThreadCreated     = FALSE;
HAB             CustomAPI_PMHandle          = 0;
HMQ             CustomAPI_MSGQHandle        = 0;
HWND            CustomAPI_WindowHandle      = 0;
ULONG           CustomAPI_ConfigSysLine     = 0;

// Custom-DLL related public variables...
PVOID           CustomDLL_EntryPoint        = 0;
PSZ             CustomDLL_EntryParms        = NULL;
PSZ             CustomDLL_CustomData        = NULL;

// INI-Control-File
PMINSTINI_DEFENTRY ICF_CheckFuncList   = NULL;
PMINSTINI_DEFENTRY ICF_CheckParmList   = NULL;
PMINSTINI_DEFENTRY ICF_CurFuncEntry    = NULL;
PMINSTINI_DEFENTRY ICF_CurParmEntry    = NULL;
ULONG              ICF_FilledParms     = 0;


// ****************************************************************************

BOOL EXPENTRY MINSTALL_Init (ULONG BootDrive, HAB PMHandle, HMQ MSGQHandle, PSZ MMBase, PSZ LogFileName) {
   PCHAR   CurPos;
   CHAR    DelayedDir[MINSTMAX_PATHLENGTH];
   APIRET  rc;
   HMODULE GeninDLLHandle = 0;

   // Remember PM and MSGQ-Handles...
   MINSTALL_PMHandle               = PMHandle;
   MINSTALL_MSGQHandle             = MSGQHandle;

   // Reset some strings...
   MINSTALL_MMBase[0]              = 0;
   MINSTALL_MMBaseDrive[0]         = 0;
   MINSTALL_MMBaseLetter[0]        = 0;
   MINSTALL_InstallPath[0]         = 0;
   MINSTALL_InstallDir[0]          = 0;
   MINSTALL_DLLDir[0]              = 0;
   MINSTALL_CompListINI[0]         = 0;
   MINSTLOG_FileName[0]            = 0;
   MINSTALL_Done                   = 0;

   if (!MSG_Init ("minstall.msg"))
      return FALSE;

   if (!MMBase[0]) {
      // MMBase not set, so check environment
      if (DosScanEnv("MMBASE", &MMBase)) {
         MINSTALL_TrappedError (MINSTMSG_MMBaseNotFound); return FALSE; }
    }
   // Copy MMBase over to MINSTALL-variable...
   if (!STRING_CombinePSZ ((PCHAR)&MINSTALL_MMBase, MINSTMAX_PATHLENGTH, MMBase, ""))
      return FALSE;

   CurPos = MINSTALL_MMBase;
   // Look till ';' or EOS...
   while ((*CurPos!=0) & (*CurPos!=0x3B))
      CurPos++;
   if (*CurPos==0x3B)
      *CurPos = 0;                          // Set Terminator, if ';' found

   MINSTALL_MMBaseDrive[0]  = MINSTALL_MMBase[0];
   MINSTALL_MMBaseDrive[1]  = MINSTALL_MMBase[1];
   MINSTALL_MMBaseDrive[2]  = 0;
   MINSTALL_MMBaseLetter[0] = MINSTALL_MMBaseDrive[0];
   MINSTALL_MMBaseLetter[1] = 0;

   // Build Install/DLL-Path for MINSTALL...
   if (!STRING_CombinePSZ (MINSTALL_InstallPath, MINSTMAX_PATHLENGTH, MINSTALL_MMBase, "\\INSTALL"))
      return FALSE;
   if (!STRING_CombinePSZ (MINSTALL_InstallDir, MINSTMAX_PATHLENGTH, MINSTALL_InstallPath, "\\"))
      return FALSE;
   if (!STRING_CombinePSZ (MINSTALL_DLLDir, MINSTMAX_PATHLENGTH, MINSTALL_MMBase, "\\DLL\\"))
      return FALSE;
   if (!STRING_CombinePSZ (MINSTALL_CompListINI, MINSTMAX_PATHLENGTH, MINSTALL_InstallDir, "COMPLIST.INI"))
      return FALSE;

   if ((BootDrive==0) || (BootDrive>26)) {
      if (DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &BootDrive, sizeof(BootDrive)) || (BootDrive<3))
         return FALSE;                         // We only accept C: -> Z:
    }
   MINSTALL_BootDrive[0]  = BootDrive+0x40; // 3 -> C
   MINSTALL_BootDrive[1]  = 0x3A;           // ':'
   MINSTALL_BootDrive[2]  = 0;              // NUL
   MINSTALL_BootLetter[0] = BootDrive+0x40; // 3 -> C
   MINSTALL_BootLetter[1] = 0;              // NUL

   // Find out Log-filename...
   if (LogFileName) {
      // We are supposed to use a logfilename
      if (LogFileName[0]) {
         if (!STRING_CopyPSZ (MINSTLOG_FileName, MINSTMAX_PATHLENGTH, LogFileName))
            return FALSE;
       } else {
         if (!STRING_CombinePSZ (MINSTLOG_FileName, MINSTMAX_PATHLENGTH, MINSTALL_InstallDir, "MINSTALL.LOG"))
            return FALSE;
       }
      if (MINSTALL_IsFirstInit) {           // Only on 1st MINSTALL_Init()
         // Now try to delete the logfile. If this works, we use it. Otherwise
         //  we try to set logfile to x:\OS2\INSTALL where x: is the bootdrive
         //  if this also fails, we will set logfile to NULL
         rc = DosDelete(MINSTLOG_FileName);
         if ((rc!=NO_ERROR) && (rc!=ERROR_FILE_NOT_FOUND)) {
            // Now try to put logfile into x:\OS2\INSTALL...
            if (!STRING_CombinePSZ (MINSTLOG_FileName, MINSTMAX_PATHLENGTH, MINSTALL_BootDrive, "\\OS2\\INSTALL\\MINSTALL.LOG"))
               return FALSE;
            rc = DosDelete(MINSTLOG_FileName);
            if ((rc!=NO_ERROR) && (rc!=ERROR_FILE_NOT_FOUND)) {
               MINSTLOG_FileName[0] = 0;          // Set LogFile to NULL
             }
          }
       }
    }

   // Detect replacement GENIN.DLL - if it's not found, bomb out...
// DEBUGDEBUGDEBUG
//   if (GeninDLLHandle = DLL_Load("GENIN.DLL")) {
//      if (!DLL_GetEntryPoint(GeninDLLHandle, "GeninReplacementIdentifier")) {
//         MINSTALL_TrappedError (MINSTMSG_GeninReplacementRequired); return FALSE; }
//      DLL_UnLoad (GeninDLLHandle);
//    }

   // Init file-delaying API (even if it's not actually used)
   if (!STRING_CombinePSZ (DelayedDir, MINSTMAX_PATHLENGTH, MINSTALL_InstallDir, "DELAYED"))
      return FALSE;
   if (CONFIGSYS_DelayInit (BootDrive, "MMLOCKED.LST", DelayedDir)!=CONFIGSYS_DONE)
      return FALSE;

   // Finally generate a Mutex-Semamorph, so that we can be sure that we are
   //  the only instance of MINSTALL active on the system.
   if (MINSTALL_MutexSemamorph==NULLHANDLE) {
      if (DosCreateMutexSem("\\SEM32\\MKMULTIMEDIAINSTALLER", &MINSTALL_MutexSemamorph, 0, FALSE)) {
         MINSTALL_TrappedError (MINSTMSG_AlreadyRunning); return FALSE; }
    }

   MINSTALL_IsFirstInit = FALSE;
   return TRUE;
 }

// This will load in the full installation of an MMOS/2 feature
//  It will also parse variables and check for their validity. If it returns
//  FALSE, this means that something went wrong and the installation can not
//  be done. The caller is supposed to call MINSTALL_CleanUp() afterwards to
//  deallocate any buffers. This has to be done manually.
BOOL EXPENTRY MINSTALL_InitPackage (PSZ SourcePath) {
   ULONG Temp;

   if (MINSTALL_IsFirstInit) {
      MINSTALL_TrappedError (MINSTMSG_NeedingInit); return FALSE; }
   if (MINSTALL_Done & MINSTDONE_BEGANPACKAGEINIT) {
      MINSTALL_TrappedError (MINSTMSG_NeedingCleanUp); return FALSE; }

   // We havent done anything yet...
   MINSTALL_Done = MINSTDONE_BEGANPACKAGEINIT;

   MINSTLOG_OpenFile();
   MINSTLOG_ToFile ("================================================================ MINSTALL v1.03\n");
   MINSTLOG_ToFile ("MINSTALL_InitPackage()...\n");

   if (SourcePath[0]!=0) {
      if (!STRING_CombinePSZ((PCHAR)MINSTALL_SourcePath, MINSTMAX_PATHLENGTH, SourcePath, ""))
         return FALSE;
    } else {
      // Get current Path and set it as SourcePath
      if (!FILE_GetCurrentPath(MINSTALL_SourcePath, MINSTMAX_PATHLENGTH))
         return FALSE;
    }

   if (!STRING_CombinePSZ(MINSTALL_SourceDir, MINSTMAX_PATHLENGTH, SourcePath, "\\"))
      return FALSE;

   MINSTLOG_ToFile (" SourcePath = %s, TargetPath = %s\n", MINSTALL_SourcePath, MINSTALL_MMBase);

   // Load CONTROL.scr file from SourcePath...
   if (!STRING_CombinePSZ((PCHAR)&CONTROLSCR.Name, MINSTMAX_PATHLENGTH, MINSTALL_SourceDir, "control.scr"))
      return FALSE;
   MINSTLOG_ToFile ("LoadMasterControl()...\n");
   if (!MINSTALL_LoadMasterControl())                       return FALSE;
   MINSTLOG_ToFile ("LoadMasterControl DONE\n");

   if (MCF_MUnitCount!=1) {
      MINSTALL_TrappedError (MINSTMSG_MultiDisksUnsupported); return FALSE;
    }
   if (FILELISTSCR.Name[0]==0) {
      MINSTALL_TrappedError (MINSTMSG_NoFileControlSpecified);return FALSE;
    }

   // Load in File-Control-Script...
   MINSTLOG_ToFile ("LoadFileControl()...\n");
   if (!MINSTALL_LoadFileControl())                        return FALSE;
   MINSTLOG_ToFile ("LoadFileControl DONE\n");
   if (!MINSTALL_FilterControlFileData())                  return FALSE;
   if (!MINSTALL_LoadCARDINFO())                           return FALSE;
   if (!MINSTALL_LoadCustomControl())                      return FALSE;
   // Finally create Public Group for caller
   if (!MINSTALL_GeneratePublicGroup())                    return FALSE;
   MINSTALL_ErrorMsgID = 0;
   MINSTALL_Done      |= MINSTDONE_COMPLETEDPACKAGEINIT;
   MINSTLOG_ToAll (" ...Package successfully initialized!\n");
   return TRUE;
 }

BOOL EXPENTRY MINSTALL_InstallPackage (VOID) {
   if (MINSTALL_IsFirstInit) {
      MINSTALL_TrappedError (MINSTMSG_NeedingInit); return FALSE; }
   MINSTLOG_ToFile ("MINSTALL_InstallPackage()...\n");
   if (!(MINSTALL_Done & MINSTDONE_COMPLETEDPACKAGEINIT)) {
      MINSTALL_TrappedError (MINSTMSG_PackageNotInitialized); return FALSE; }

   // Select files&directories depending on group selection done by caller
   MINSTALL_SelectFiles();

   // Generate Destination Directories, copy files and process scripts...
   if (!MINSTALL_CreateDestinDirectories())                return FALSE;
   MINSTLOG_ToFile ("MINSTALL_CopyFiles()...\n");
   if (!MINSTALL_CopyFiles())                              return FALSE;
   MINSTLOG_ToFile ("MINSTALL_CopyFiles DONE\n");
   MINSTLOG_ToFile ("MINSTALL_LinkInImports()...\n");
   if (!MINSTALL_LinkInImports())                          return FALSE;
   MINSTLOG_ToFile ("MINSTALL_LinkInImports DONE\n");
   // Hardcoded in MMI_IMPORTS.C
   MINSTALL_MigrateMMPMMMIOFile();
   MINSTLOG_ToFile ("MINSTALL_ExecuteCustomDLLs()...\n");
   if (!MINSTALL_ExecuteCustomDLLs())                      return FALSE;
   MINSTLOG_ToFile ("MINSTALL_ExecuteCustomDLLs DONE\n");
   MINSTLOG_ToFile ("MINSTALL_ProcessScripts()...\n");
   if (!MINSTALL_ProcessCARDINFO())                        return FALSE;
   if (!MINSTALL_ProcessScripts())                         return FALSE;
   MINSTLOG_ToFile ("MINSTALL_ExecuteCustomTermDLLs()...\n");
   if (!MINSTALL_ExecuteCustomTermDLLs())                  return FALSE;
   // Do special things on base installations...
   if (MINSTALL_IsBaseInstallation) {
      MINSTALL_LowerMasterVolume();
    }
   MINSTLOG_ToFile ("MINSTALL_ExecuteCustomTermDLLs DONE\n");
   if (!MINSTALL_SaveInstalledVersions())                  return FALSE;
   MINSTALL_ErrorMsgID = 0;
   MINSTALL_SystemShouldReboot = TRUE;
   MINSTLOG_ToFile ("MINSTALL_InstallPackage DONE\n");
   return TRUE;
 }

// Does all sorts of cleanup. May not fail in any way.
VOID EXPENTRY MINSTALL_CleanUp (VOID) {
   MINSTLOG_ToFile ("MINSTALL_CleanUp()...\n");
   if (MINSTALL_Done & MINSTDONE_LINKINIMPORTS)
      MINSTALL_CleanUpImports();
   if (MINSTALL_Done & MINSTDONE_PUBLICGROUP)
      MINSTALL_CleanUpPublicGroup();
   if (MINSTALL_Done & MINSTDONE_LOADCUSTOMCTRLSCR)
      MINSTALL_CleanUpCustomControl();
   if (MINSTALL_Done & MINSTDONE_LOADCARDINFO)
      MINSTALL_CleanUpCARDINFO();
   if (MINSTALL_Done & MINSTDONE_LOADFILECTRLSCR)
      MINSTALL_CleanUpFileControl();
   if (MINSTALL_Done & MINSTDONE_LOADMASTERCTRLSCR)
      MINSTALL_CleanUpMasterControl();
   MINSTLOG_ToFile ("MINSTALL_CleanUp DONE\n");

   MINSTALL_Done       = 0;
   MINSTALL_ErrorMsgID = 0;
   MINSTLOG_CloseFile();
 }

PSZ EXPENTRY MINSTALL_GetErrorMsgPtr (void) {
   if (MINSTALL_ErrorMsgID) {
      return MINSTALL_ErrorMsg;
    }
   return NULL;
 }

USHORT EXPENTRY MINSTALL_GetErrorMsgCIDCode (void) {
   switch (MINSTALL_ErrorMsgID) {
    case 0:
      if (MINSTALL_SystemShouldReboot) {
         return CIDRET_SuccessReboot;
       } else {
         return CIDRET_Success;
       }
    case MINSTMSG_CouldNotFindSourceFile:
    case MINSTMSG_CICouldNotLoadCustomDLL:
    case MINSTMSG_CouldNotAccessFile:
      return CIDRET_DataResourceNotFound;
    case MINSTMSG_GenericError:
      return CIDRET_UnexpectedCondition;
    case MINSTMSG_DiskFull:
      return CIDRET_NotEnoughDiskSpace;
    default:
      return CIDRET_SuccessErrorLogged;
    }
 }

PSZ EXPENTRY MINSTALL_GetErrorMsgCIDCodeDescription (void) {
   MINSTALL_CIDDescription[0] = 0;
   switch (MINSTALL_ErrorMsgID) {
    case 0:
      if (MINSTALL_SystemShouldReboot) {
         MSG_Get((PVOID)&MINSTALL_CIDDescription, 1024, MINSTMSG_CIDSuccessReboot); break;
       } else {
         MSG_Get((PVOID)&MINSTALL_CIDDescription, 1024, MINSTMSG_CIDSuccess); break;
       }
    case MINSTMSG_CouldNotFindSourceFile:
    case MINSTMSG_CICouldNotLoadCustomDLL:
    case MINSTMSG_CouldNotAccessFile:
      MSG_Get((PVOID)&MINSTALL_CIDDescription, 1024, MINSTMSG_CIDDataResourceNotFound); break;
    case MINSTMSG_GenericError:
      MSG_Get((PVOID)&MINSTALL_CIDDescription, 1024, MINSTMSG_CIDUnexpectedCondition); break;
    case MINSTMSG_DiskFull:
      MSG_Get((PVOID)&MINSTALL_CIDDescription, 1024, MINSTMSG_CIDNotEnoughDiskSpace); break;
    default:
      MSG_Get((PVOID)&MINSTALL_CIDDescription, 1024, MINSTMSG_CIDSuccessErrorsLogged); break;
    }
   return (PSZ)&MINSTALL_CIDDescription;
 }

PSZ EXPENTRY MINSTALL_GetSourcePathPtr (void) {
   return MINSTALL_SourcePath;
 }

PSZ EXPENTRY MINSTALL_GetTargetPathPtr (void) {
   return MINSTALL_MMBase;
 }

ULONG EXPENTRY MINSTALL_GetPublicGroupArrayPtr (PMINSTPUBGROUP *PubGroupArrayPtr) {
   if (MINSTALL_Done & MINSTDONE_COMPLETEDPACKAGEINIT) {
      *PubGroupArrayPtr = MINSTALL_PublicGroupArrayPtr;
      return MINSTALL_PublicGroupCount;
    }
   return 0;
 }

PSZ EXPENTRY MINSTALL_GetPublicGroupCustomDataPtr (ULONG GroupID) {
   PMINSTGRP      CurGroup       = MCF_GroupArrayPtr;
   ULONG          CurGroupNo     = 0;

   while (CurGroupNo<MCF_GroupCount) {
      if (CurGroup->ID==GroupID) {
         if (!(CurGroup->Flags & MINSTGRP_Flags_DontListPublic)) {
            if (CurGroup->GeninPtr) {
               // If GENIN information present, calculate realtime
               MINSTALL_FillCARDINFOCustomData (CurGroup);
             }
            return CurGroup->CustomData;
          }
         break;
       }
      CurGroupNo++; CurGroup++;
    }
   return NULL;
 }

BOOL EXPENTRY MINSTALL_SetPublicGroupCustomData (ULONG GroupID, PSZ CustomDataPtr) {
   PMINSTGRP      CurGroup       = MCF_GroupArrayPtr;
   ULONG          CurGroupNo     = 0;

   while (CurGroupNo<MCF_GroupCount) {
      if (CurGroup->ID==GroupID) {
         if (!(CurGroup->Flags & MINSTGRP_Flags_DontListPublic)) {
            if (STRING_CopyPSZ(CurGroup->CustomData, MINSTMAX_CUSTOMDATALENGTH, CustomDataPtr)) {
               if (CurGroup->GeninPtr) {
                  // If GENIN information present, automatically select
                  if (!MINSTALL_UseCARDINFOCustomData(CurGroup))
                     return FALSE;
                }
             }
            return TRUE;
          }
         break;
       }
      CurGroupNo++; CurGroup++;
    }
   return TRUE;
 }

PSZ EXPENTRY MINSTALL_GetPackageTitlePtr (void) {
   if (MINSTALL_Done & MINSTDONE_COMPLETEDPACKAGEINIT) {
      return MCF_PackageName;            // Simple, ey? ;-)
    }
   return NULL;
 }

VOID EXPENTRY MINSTALL_SelectGroup (ULONG GroupID) {
   PMINSTGRP      CurGroup       = MCF_GroupArrayPtr;
   PMINSTPUBGROUP PubGroup       = MINSTALL_PublicGroupArrayPtr;
   ULONG          CurGroupNo     = 0;

   // Cycle through all groups and match GroupID. Selecting possible only on
   //  groups that are listed in PublicGroup and SelectionForced not set.
   while (CurGroupNo<MCF_GroupCount) {
      if (CurGroup->ID==GroupID) {
         if (!(CurGroup->Flags & MINSTGRP_Flags_DontListPublic)) {
            CurGroup->Flags |= MINSTGRP_Flags_Selected;
          }
         break;
       }
      CurGroupNo++; CurGroup++;
    }

   // Update Public-Group as well...
   CurGroupNo = 0;
   while (CurGroupNo<MINSTALL_PublicGroupCount) {
      if (PubGroup->ID==GroupID) {
         if (!(PubGroup->SelectionForced)) {
            PubGroup->Selected = TRUE;
          }
       }
      CurGroupNo++; PubGroup++;
    }
 }

VOID EXPENTRY MINSTALL_DeSelectGroup (ULONG GroupID) {
   PMINSTGRP      CurGroup       = MCF_GroupArrayPtr;
   PMINSTPUBGROUP PubGroup       = MINSTALL_PublicGroupArrayPtr;
   ULONG          CurGroupNo     = 0;

   // Cycle through all groups and match GroupID. Selecting possible only on
   //  groups that are listed in PublicGroup and SelectionForced not set.
   while (CurGroupNo<MCF_GroupCount) {
      if (CurGroup->ID==GroupID) {
         if (!(CurGroup->Flags & MINSTGRP_Flags_DontListPublic)) {
            CurGroup->Flags &= !MINSTGRP_Flags_Selected;
          }
         break;
       }
      CurGroupNo++; CurGroup++;
    }

   // Update Public-Group as well...
   CurGroupNo = 0;
   while (CurGroupNo<MINSTALL_PublicGroupCount) {
      if (PubGroup->ID==GroupID) {
         if (!(PubGroup->SelectionForced)) {
            PubGroup->Selected = FALSE;
          }
       }
      CurGroupNo++; PubGroup++;
    }
 }

VOID EXPENTRY MINSTALL_SetCARDINFOCardCountForGroup (ULONG GroupID, ULONG SelectedCards) {
   PMINSTGRP      CurGroup       = MCF_GroupArrayPtr;
   PMINSTPUBGROUP PubGroup       = MINSTALL_PublicGroupArrayPtr;
   ULONG          CurGroupNo     = 0;

   // Cycle through all groups and match GroupID. Set SelectedCards only, if
   //  Genin-Information present.
   while (CurGroupNo<MCF_GroupCount) {
      if (CurGroup->ID==GroupID) {
         if (CurGroup->GeninPtr) {
            if (SelectedCards<=CurGroup->GeninPtr->MaxCardCount)
               CurGroup->GeninPtr->SelectedCards = SelectedCards;
          }
         break;
       }
      CurGroupNo++; CurGroup++;
    }
 }

ULONG EXPENTRY MINSTALL_GetCARDINFOChoiceForGroup (ULONG GroupID, ULONG CardNo, ULONG PromptNo) {
   PMINSTGRP      CurGroup       = MCF_GroupArrayPtr;
   ULONG          CurGroupNo     = 0;

   if ((!CardNo) || (!PromptNo)) return;    // Invalid input
   CardNo--; PromptNo--;

   // Cycle through all groups and match GroupID. Set Choice only, if Genin-
   //  Information present.
   while (CurGroupNo<MCF_GroupCount) {
      if (CurGroup->ID==GroupID) {
         if (CurGroup->GeninPtr) {
            // Check, if CardNo/PromptNo is a valid value
            if (CardNo>=CurGroup->GeninPtr->SelectedCards)
               return 0;
            if (PromptNo>=CurGroup->GeninPtr->PromptsCount)
               return 0;
            return (CurGroup->GeninPtr->PromptSelectedValueNo[CardNo][PromptNo])+1;
          }
         break;
       }
      CurGroupNo++; CurGroup++;
    }
   return 0;
 }

// ChoiceNo in that case is 1-based
VOID EXPENTRY MINSTALL_SetCARDINFOChoiceForGroup (ULONG GroupID, ULONG CardNo, ULONG PromptNo, ULONG ChoiceNo) {
   PMINSTGRP      CurGroup       = MCF_GroupArrayPtr;
   ULONG          CurGroupNo     = 0;
   ULONG          CurChoiceNo    = 0;
   PSZ            CurValuePtr    = 0;

   if ((!CardNo) || (!PromptNo) || (!ChoiceNo)) return; // Invalid input
   CardNo--; PromptNo--; ChoiceNo--;

   // Cycle through all groups and match GroupID. Set Choice only, if Genin-
   //  Information present.
   while (CurGroupNo<MCF_GroupCount) {
      if (CurGroup->ID==GroupID) {
         if (CurGroup->GeninPtr) {
            // We got GroupID, so cycle through all Values and set selected
            //  PSZ pointer. Also check, if ChoiceNo is a valid value
            // Check, if CardNo/PromptNo is a valid value
            if (CardNo>=CurGroup->GeninPtr->SelectedCards)
               return;
            if (PromptNo>=CurGroup->GeninPtr->PromptsCount)
               return;
            if (ChoiceNo>=CurGroup->GeninPtr->PromptChoiceCount[PromptNo])
               return;
            CurValuePtr = CurGroup->GeninPtr->PromptChoiceValues[PromptNo];
            while (CurChoiceNo<ChoiceNo) {
               CurValuePtr = STRING_SkipASCIIZ(CurValuePtr, NULL);
               CurChoiceNo++;
             }
            CurGroup->GeninPtr->PromptSelectedValue[CardNo][PromptNo]   = CurValuePtr;
            CurGroup->GeninPtr->PromptSelectedValueNo[CardNo][PromptNo] = ChoiceNo;
          }
         break;
       }
      CurGroupNo++; CurGroup++;
    }
 }
