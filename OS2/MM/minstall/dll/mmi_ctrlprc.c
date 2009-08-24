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
#define INCL_WINWORKPLACE                   // for WPS functions
// #define INCL_OS2MM                          // Include MMIO functions
// #define INCL_MMIO_CODEC                     //  ...
// #define INCL_AUDIO_CODEC_ONLY
#include <os2.h>
#include <pmdef.h>
#include <pmshl.h>
#include <pmwp.h>
#define INCL_MCIOS2
#include <os2me.h>

#include <malloc.h>
#include <string.h>

#include <global.h>
#include <dll.h>
#include <cfgsys.h>                         // CONFIG.SYS Changing
#include <crcs.h>
#include <file.h>
#include <globstr.h>
#include <msg.h>
#include <mciini.h>                         // My own MCI-SYSINFO routines...
#include <mmi_public.h>
#include <mmi_types.h>
#include <mmi_main.h>
#include <mmi_helper.h>
#include <mmi_msg.h>
#include <mmi_inistuff.h>
#include <mmi_imports.h>

BOOL MINSTALL_ProcessConfigControl (ULONG ChangeCount, PVOID ChangeArrayPtr) {
   LONG  CONFIGSYSrc = 0;

   CONFIGSYSrc = CONFIGSYS_Process (0, ChangeCount, (PCONFIGSYSACTION)ChangeArrayPtr, "\nREM *** MMOS/2 ***\n");
   switch (CONFIGSYSrc) {
    case CONFIGSYS_DONE:
      MINSTLOG_ToFile ("Internal: CONFIG.SYS processed, did not get modified\n");
      return TRUE;
    case CONFIGSYS_DONE_BackUp:
      MINSTLOG_ToFile ("Internal: CONFIG.SYS updated, old one backupped\n");
      return TRUE;
    case CONFIGSYS_DONE_Changed:
      MINSTLOG_ToFile ("Internal: CONFIG.SYS updated, w/o backup\n");
      return TRUE;
    case CONFIGSYS_ERR_IsReadOnly:
      MINSTALL_TrappedError (MINSTMSG_CONFIGSYSReadOnly); return FALSE;
    case CONFIGSYS_ERR_FailedBackUp:
      MINSTALL_TrappedError (MINSTMSG_CONFIGSYSFailedBackUp); return FALSE;
    default:
      MINSTALL_TrappedError (MINSTMSG_CONFIGSYSGenericProblem); return FALSE;
    }
 }

BOOL EA_JoinTheseEAs (PSZ JoinToPathName, PFEA2LIST EAListPtr) {
   EAOP2       EASetStruct;
   APIRET      rc;

   EASetStruct.fpGEA2List = NULL;
   EASetStruct.fpFEA2List = EAListPtr;
   EASetStruct.oError     = 0;
   if ((rc = DosSetPathInfo(JoinToPathName, FIL_QUERYEASIZE, &EASetStruct, sizeof(EASetStruct), DSPI_WRTTHRU))!=0) {
      MINSTLOG_ToFile ("rc = %d...", rc); return FALSE; }
   return TRUE;
 }

typedef struct _EA_SETLONGNAMEFEA {
   ULONG    cbList;
   ULONG    oNextEntryOffset;
   BYTE     fEA;
   BYTE     cbName;
   USHORT   cbValue;
   CHAR     szLONGNAME[10];
   USHORT   LongNameVal;
   USHORT   LongNameLen;
   CHAR     LongName[MINSTMAX_PATHLENGTH];
 } EA_SETLONGNAMEFEA;

BOOL EA_SetLongnameEA (PSZ TargetPathName, PSZ LongName) {
   ULONG             LongNameLen = strlen(LongName);
   EA_SETLONGNAMEFEA MyFEA;

   MyFEA.cbList           = sizeof(MyFEA);
   MyFEA.oNextEntryOffset = sizeof(MyFEA);
   MyFEA.fEA              = 0;
   MyFEA.cbName           = 9;
   MyFEA.cbValue          = 0x10;
   strcpy (MyFEA.szLONGNAME, ".LONGNAME");
   MyFEA.LongNameVal      = 0xFFFD;
   MyFEA.LongNameLen      = LongNameLen;
   strcpy (MyFEA.LongName, LongName);
   return EA_JoinTheseEAs (TargetPathName, (PFEA2LIST)&MyFEA);
 }

// Gets .EAs from a file and adds them to a specified file/path
BOOL EA_JoinFileEA (PSZ JoinToPathName, PSZ EAFileName) {
   FILECONTROL TEMPFILE;
   CHAR        ProcessFileName[MINSTMAX_PATHLENGTH];
   BOOL        ErrorOccured = FALSE;
   FILE        *FileHandle  = 0;
   ULONG       FileSize     = 0;
   PVOID       EAFileBuffer = 0;

   FileHandle = fopen(EAFileName, "rb");
   if (FileHandle==NULL) return FALSE;

   // Get File-Length of INI-File, check for maximum buffer-length
   fseek (FileHandle, 0, SEEK_END);
   FileSize = ftell(FileHandle);
   if (FileSize<524288) {                    // We dont take files larger 512k
      EAFileBuffer = malloc(FileSize+4);
      if (EAFileBuffer) {
         // Read in EA-data...
         fseek (FileHandle, 0, SEEK_SET); // Seek back to start
         if (FileSize != fread((PVOID)(((ULONG)EAFileBuffer)+4), 1, FileSize, FileHandle))
            ErrorOccured = TRUE;
       } else ErrorOccured = TRUE;
    } else ErrorOccured = TRUE;
   fclose (FileHandle);

   if (!ErrorOccured) {
      // If no error occured, add those .EAs to file/path...
      (*(PULONG)EAFileBuffer) = FileSize;
      if (!EA_JoinTheseEAs(JoinToPathName, (PFEA2LIST)EAFileBuffer))
         ErrorOccured = TRUE;
    }

   free (EAFileBuffer);
   if (!ErrorOccured) return TRUE;
   return FALSE;
 }

BOOL PRF_SetProfileData (PPRF_PROFILEDATA CustomPtr) {
   HMODULE DLLHandle    = 0;
   PVOID   ResourcePtr  = 0;
   ULONG   ResourceSize = 0;
   HINI    INIHandle    = 0;
   BOOL    ErrorOccured = FALSE;
   BOOL    NeedToClose  = FALSE;

   if ((DLLHandle = DLL_Load(CustomPtr->Dll))!=0) {
      if (DLL_GetDataResource (DLLHandle, CustomPtr->Id, &ResourcePtr, &ResourceSize)) {
         // Uppercase filename...
         strupr (CustomPtr->Ini);

         // Check for hardcoded SYSTEM/USER...
         if (strcmp(CustomPtr->Ini, "HINI_SYSTEM")==0) {
            INIHandle = HINI_SYSTEMPROFILE;
          } else if (strcmp(CustomPtr->Ini, "HINI_USER")==0) {
            INIHandle = HINI_USERPROFILE;
          } else {
            // We assume that the string is an INI-Filename...
            if (!(INIHandle   = PrfOpenProfile(MINSTALL_PMHandle, CustomPtr->Ini)))
               ErrorOccured = TRUE;
            NeedToClose = TRUE;
          }
       } else ErrorOccured = TRUE;
    } else ErrorOccured = TRUE;

   if (INIHandle) {
      // Got valid INI-Handle, so write the string...
      ErrorOccured = !PrfWriteProfileData(INIHandle, CustomPtr->AppName, CustomPtr->KeyName, ResourcePtr, ResourceSize);
    } else ErrorOccured = TRUE;

   if (NeedToClose) PrfCloseProfile(INIHandle);
   if (DLLHandle)   DLL_UnLoad(DLLHandle);
   return !ErrorOccured;
 }

BOOL PRF_SetProfileString (PPRF_PROFILESTRING CustomPtr) {
   HINI INIHandle    = 0;
   BOOL ErrorOccured = FALSE;
   BOOL NeedToClose  = FALSE;

   // Uppercase filename...
   strupr (CustomPtr->Inis);

   // Check for hardcoded SYSTEM/USER...
   if (strcmp(CustomPtr->Inis, "HINI_SYSTEM")==0) {
      INIHandle = HINI_SYSTEMPROFILE;
    } else if (strcmp(CustomPtr->Inis, "HINI_USER")==0) {
      INIHandle = HINI_USERPROFILE;
    } else {
      // We assume that the string is an INI-Filename...
      if (!(INIHandle   = PrfOpenProfile(MINSTALL_PMHandle, CustomPtr->Inis)))
         ErrorOccured = TRUE;
      NeedToClose = TRUE;
    }

   if (INIHandle) {
      // Got valid INI-Handle, so write the string...
      ErrorOccured = !PrfWriteProfileString(INIHandle, CustomPtr->AppNames, CustomPtr->KeyNames, CustomPtr->Datas);
    } else ErrorOccured = TRUE;
   if (NeedToClose) PrfCloseProfile(INIHandle);
   return !ErrorOccured;
 }

BOOL WPS_CreateObject (PWPS_CREATEOBJECT CustomPtr) {
   if (!WinCreateObject(CustomPtr->WPClassName, CustomPtr->WPTitle,
       CustomPtr->WPSetupString, CustomPtr->WPLocation, CustomPtr->WPFlags))
      return FALSE;
   return TRUE;
 }

BOOL WPS_DestroyObject (PWPS_DESTROYOBJECT CustomPtr) {
   HOBJECT ObjectHandle = 0;

   if (!(ObjectHandle = WinQueryObject(CustomPtr->WPDestroyObjectID)))
      return FALSE;
   return WinDestroyObject(ObjectHandle);
 }

BOOL WPS_WPClass (PWPS_WPCLASS CustomPtr) {
   CHAR       DLLName[MAXFILELENGTH];
   ULONG      CRC32;
   PMINSTFILE CurFilePtr;

   if (FILE_IncludesPath(CustomPtr->WPDllName)) {
      strcpy (DLLName, CustomPtr->WPDllName);
    } else {
      // If DLL-Name doesnt contain path already, we search for the file in
      //  filelist and add the corresponding path.
      strlwr (CustomPtr->WPDllName);
      FILE_SetDefaultExtension (CustomPtr->WPDllName, MINSTMAX_PATHLENGTH, ".dll");
      CRC32 = CRC32_GetFromPSZ(CustomPtr->WPDllName);
      if ((CurFilePtr = MINSTALL_SearchFileCRC32(CRC32))!=0) {
         if (!STRING_CombinePSZ (DLLName, MAXFILELENGTH, CurFilePtr->DestinPtr->FQName, CustomPtr->WPDllName))
            return FALSE;                   // buffer-overflow
       } else {
         strcpy (DLLName, CustomPtr->WPDllName);
       }
    }
   // Register the class at any time...
   if (!WinRegisterObjectClass (CustomPtr->WPClassNameNew, DLLName))
      return FALSE;
   if (strlen(CustomPtr->WPReplaceClass)>0) {
      // If Replace-Class set, replace this class with the new one...
      if (!WinReplaceObjectClass (CustomPtr->WPReplaceClass, CustomPtr->WPClassNameNew, TRUE))
         return FALSE;
    }
   return TRUE;
 }

BOOL MINSTALL_ProcessINIControl (ULONG ChangeCount, PVOID Change1stEntryPtr) {
   ULONG           CurChangeNo = 0;
   PMINSTINIHEADER CurEntryPtr = Change1stEntryPtr;
   PVOID           CustomPtr   = 0;
   APIRET          rc          = 0;
   BOOL            Failed      = FALSE;
   ERRORID         ErrorID;

   while (CurChangeNo<ChangeCount) {
      // CustomPtr points at actual INIChange specific data
      CustomPtr = (PVOID)(((ULONG)CurEntryPtr)+sizeof(MINSTINIHEADER));
      switch (CurEntryPtr->ID) {
       case EA_JOINEA_ID:                   // ==================== EA - JoinEA
         MINSTLOG_ToFile (" - JoinEA...");
         if (!EA_JoinFileEA(((PEA_JOINEA)CurEntryPtr)->JoinFileName, ((PEA_JOINEA)CurEntryPtr)->JoinEAFileName))
            Failed = TRUE;
         break;
       case EA_JOINLONGNAMEEA_ID:           // ============ EA - JoinLongNameEA
         MINSTLOG_ToFile (" - JoinLongNameEA...");
         if (strlen(((PEA_JOINLONGNAMEEA)CurEntryPtr)->JoinEALongFileName)) {
            // If file specified, add that one first...
            if (!EA_JoinFileEA(((PEA_JOINLONGNAMEEA)CurEntryPtr)->JoinLongFileName, ((PEA_JOINLONGNAMEEA)CurEntryPtr)->JoinEALongFileName))
               Failed = TRUE;
          }
         // Now set .LONGNAME-EA manually...
         if (!EA_SetLongnameEA (((PEA_JOINLONGNAMEEA)CurEntryPtr)->JoinLongFileName, ((PEA_JOINLONGNAMEEA)CurEntryPtr)->JoinLongName))
            Failed = TRUE;
         break;
       case MCI_MCIINSTALLDRV_ID:           // ============ MCI - MciInstallDrv
         MINSTLOG_ToFile (" - MciInstallDrv...");
         if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_INSTALL_DRIVER, (PMCI_SYSINFO_LOGDEVICE)CustomPtr))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case MCI_MCIINSTALLCONN_ID:          // =========== MCI - MciInstallConn
         MINSTLOG_ToFile (" - MciInstallConn...");
         if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_CONNECTORS, (PMCI_SYSINFO_CONPARAMS)CustomPtr))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case MCI_MCIINSTALLPARM_ID:          // =========== MCI - MciInstallParm
         MINSTLOG_ToFile (" - MciInstallParm...");
         if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_PARAMS, (PMCI_SYSINFO_DEVPARAMS)CustomPtr))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case MCI_MCIINSTALLALIAS_ID:         // ========== MCI - MciInstallAlias
         MINSTLOG_ToFile (" - MciInstallAlias...");
         if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_ALIAS, (PMCI_SYSINFO_ALIAS)CustomPtr))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case MCI_MCIINSTALLEXT_ID:           // ============ MCI - MciInstallExt
         MINSTLOG_ToFile (" - MciInstallExt...");
         if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_EXTENSIONS, (PMCI_SYSINFO_EXTENSION)CustomPtr))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case MCI_MCIINSTALLTYPES_ID:         // ========== MCI - MciInstallTypes
         MINSTLOG_ToFile (" - MciInstallTypes...");
         if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_TYPES, (PMCI_SYSINFO_TYPES)CustomPtr))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case MMIO_MMIOINSTALL_ID:            // ============= MMIO - mmioInstall
         MINSTLOG_ToFile (" - mmioInstall...");
         if ((rc = (*CODE_mmioIniFileHandlerFunc) ((PMMIO_MMIOINSTALL)CustomPtr, MMIO_INSTALLPROC|MMIO_EXTENDED_STRUCT))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case MMIO_MMIOCODECDELETE_ID:        // ========= MMIO - mmioCodecDelete
         MINSTLOG_ToFile (" - mmioCodecDelete...");
         // We set StructLen by ourselves, just for security
         ((PMMIO_MMIOCODEC)CurEntryPtr)->ulStructLen = sizeof(MMIO_MMIOCODEC)-sizeof(MINSTINIHEADER);
         if ((rc = (*CODE_mmioIniFileCODECFunc) ((PMMIO_MMIOCODEC)CustomPtr, MMIO_REMOVEPROC|MMIO_MATCHFOURCC|MMIO_MATCHDLL|MMIO_MATCHPROCEDURENAME|MMIO_MATCHCOMPRESSTYPE))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case MMIO_MMIOCODEC1INSTALL_ID:      // ======= MMIO - mmioCodec1Install
       case MMIO_MMIOCODEC2INSTALL_ID:      // ======= MMIO - mmioCodec2Install
         // We can process both because buffer is exactly the same...
         MINSTLOG_ToFile (" - mmioCodecInstall...");
         // We set StructLen by ourselves, just for security
         ((PMMIO_MMIOCODEC)CurEntryPtr)->ulStructLen = sizeof(MMIO_MMIOCODEC)-sizeof(MINSTINIHEADER);
         if ((rc = (*CODE_mmioIniFileCODECFunc) ((PMMIO_MMIOCODEC)CustomPtr, MMIO_INSTALLPROC|MMIO_MATCHFOURCC|MMIO_MATCHDLL|MMIO_MATCHPROCEDURENAME|MMIO_MATCHCOMPRESSTYPE))!=0) {
            MINSTLOG_ToFile ("rc = %Xh...", rc); Failed = TRUE; }
         break;
       case PRF_PROFILEDATA_ID:             // ============== PRF - ProfileData
         MINSTLOG_ToFile (" - ProfileData...");
         if (!PRF_SetProfileData((PPRF_PROFILEDATA)CurEntryPtr))
            Failed = TRUE;
         break;
       case PRF_PROFILESTRING_ID:           // ============ PRF - ProfileString
         MINSTLOG_ToFile (" - ProfileString...");
         if (!PRF_SetProfileString((PPRF_PROFILESTRING)CurEntryPtr))
            Failed = TRUE;
         break;
       case SPI_SPIINSTALL_ID:              // =============== SPI - SpiInstall
         MINSTLOG_ToFile (" - SpiInstall...");
         if ((*CODE_SpiInstallFunc) (((PSPI_SPIINSTALL)CurEntryPtr)->SpiDllName))
            Failed = TRUE;
         break;
       case WPS_CREATEOBJECT_ID:            // ================= WPS - WPObject
         MINSTLOG_ToFile (" - WPS-CreateObject...");
         if (!WPS_CreateObject((PWPS_CREATEOBJECT)CurEntryPtr)) {
            ErrorID = WinGetLastError (MINSTALL_PMHandle);
            MINSTLOG_ToFile ("ErrorID = %Xh...", ErrorID); Failed = TRUE; }
         break;
       case WPS_DESTROYOBJECT_ID:           // ========== WPS - WPDestroyObject
         MINSTLOG_ToFile (" - WPS-DestroyObject...");
         if (!WPS_DestroyObject((PWPS_DESTROYOBJECT)CurEntryPtr)) {
            ErrorID = WinGetLastError (MINSTALL_PMHandle);
            MINSTLOG_ToFile ("ErrorID = %Xh...", ErrorID); Failed = TRUE; }
         break;
       case WPS_WPCLASS_ID:                 // ================== WPS - WPClass
         MINSTLOG_ToFile (" - WPClass...");
         if (!WPS_WPClass((PWPS_WPCLASS)CurEntryPtr)) {
            ErrorID = WinGetLastError (MINSTALL_PMHandle);
            MINSTLOG_ToFile ("ErrorID = %Xh...", ErrorID); Failed = TRUE; }
         break;
       }

      if (Failed) {
         MINSTLOG_ToFile ("failed\n");
         Failed = FALSE;
        } else {
         MINSTLOG_ToFile ("success\n");
       }

      // Go to next INI-Change...
      CurEntryPtr = CurEntryPtr->NextPtr;
      CurChangeNo++;
    }

   return TRUE;
 }


BOOL MINSTALL_ProcessScripts (VOID) {
   PMINSTGRP  CurGroupPtr = MCF_GroupArrayPtr;
   USHORT     CurNo       = 0;
   USHORT     CurChangeNo = 0;

   while (CurNo<MCF_GroupCount) {
      if (CurGroupPtr->Flags & MINSTGRP_Flags_Selected) {
         MINSTLOG_ToFile (" (Group %d)...\n", CurGroupPtr->ID);
         if (CurGroupPtr->ConfigChangeCount) {
            if (!MINSTALL_ProcessConfigControl(CurGroupPtr->ConfigChangeCount, CurGroupPtr->ConfigChangeArray))
               return FALSE;
          }
         if (CurGroupPtr->INIChangeCount) {
            if (!MINSTALL_ProcessINIControl(CurGroupPtr->INIChangeCount, CurGroupPtr->INIChange1stEntry))
               return FALSE;
          }
       }
      CurGroupPtr++; CurNo++;
    }
   return TRUE;
 }
