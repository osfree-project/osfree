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
#define INCL_WINMESSAGEMGR
#define INCL_WINWINDOWMGR
#define INCL_OS2MM
#include <os2.h>
#include <os2me.h>
#include <malloc.h>

#include <global.h>
#include <cfgsys.h>
#include <crcs.h>
#include <dll.h>
#include <file.h>
#include <globstr.h>
#include <msg.h>
#include <mciini.h>                         // MCI-INI Functions
#include <mmi_public.h>
#include <mmi_types.h>
#include <mmi_main.h>
#include <mmi_helper.h>
#include <mmi_msg.h>
#include <mmi_inistuff.h>
#include <mmi_ctrlprc.h>
#include <mmi_customdll.h>


PSZ FakedConfigSysFile[] = {
   "IFS=C:\\OS2\\HPFS.IFS",
   "LIBPATH=C:\\OS2\\DLL",
   "SET PATH=C:\\OS2",
   "SET DPATH=C:\\OS2",
   "BASEDEV=IBMKBD.SYS",
   "DEVICE=C:\\OS2\\FAKED.SYS" };
#define FakedConfigSysFileMaxNo 5

PMINSTINIHEADER CustomAPI_INIChange1stEntryPtr  = 0;
PMINSTINIHEADER CustomAPI_INIChangeLastEntryPtr = 0;

PCONFIGSYSACTION CustomAPI_ConfigSysActionArrayPtr = 0;
PCONFIGSYSACTION CustomAPI_ConfigSysActionCurPtr   = 0;
PCONFIGSYSACTSTR CustomAPI_ConfigSysStringArrayPtr = 0;
PCONFIGSYSACTSTR CustomAPI_ConfigSysStringCurPtr   = 0;
ULONG            CustomAPI_ConfigSysActionCount    = 0;

// ****************************************************************************

PMINSTINIHEADER MINSTALL_CustomAPIAllocINIChange (ULONG EntryID, ULONG EntrySize) {
   PMINSTINIHEADER CurEntryPtr = malloc(EntrySize);

   if (CurEntryPtr) {
      // It worked, so set public variable or update last entry
      if (CustomAPI_INIChangeLastEntryPtr) {
         CustomAPI_INIChangeLastEntryPtr->NextPtr = CurEntryPtr;
       } else {
         CustomAPI_INIChange1stEntryPtr = CurEntryPtr;
       }
      memset (CurEntryPtr, 0, EntrySize);
      CurEntryPtr->ID   = EntryID;
      CurEntryPtr->Size = EntrySize;
      CustomAPI_INIChangeLastEntryPtr = CurEntryPtr;
    }
   return CurEntryPtr;
 }

VOID CustomAPI_AnalyseSYSINFOCalls (ULONG SysInfoID, PVOID SysInfoParms) {
   PMCI_SYSINFO_LOGDEVICE    CurSysInfoLogDev  = NULL;
   PMCI_SYSINFO_CONPARAMS    CurSysInfoConn    = NULL;
   PMCI_SYSINFO_DEVPARAMS    CurSysInfoParams  = NULL;
   USHORT                    i;
   USHORT                    j;

   MINSTLOG_ToFile ("    SYSINFO message content:\n");
   MINSTLOG_ToFile ("     - SysInfoCode = %d", SysInfoID);
   switch (SysInfoID) {
    case     1: MINSTLOG_ToFile (" (INSTALL_DRIVER)"); break;
    case     2: MINSTLOG_ToFile (" (QUERY_DRIVER)"); break;
    case     4: MINSTLOG_ToFile (" (DELETE_DRIVER)"); break;
    case     8: MINSTLOG_ToFile (" (SET_PARAMS)"); break;
    case    16: MINSTLOG_ToFile (" (QUERY_PARAMS)"); break;
    case    32: MINSTLOG_ToFile (" (SET_CONNECTORS)"); break;
    case    64: MINSTLOG_ToFile (" (QUERY_CONNECTORS)"); break;
    case   128: MINSTLOG_ToFile (" (SET_EXTENSIONS)"); break;
    case   256: MINSTLOG_ToFile (" (QUERY_EXTENSIONS)"); break;
    case   512: MINSTLOG_ToFile (" (SET_ALIAS)"); break;
    case  1024: MINSTLOG_ToFile (" (QUERY_NAMES)"); break;
    case  2048: MINSTLOG_ToFile (" (SET_DEFAULT)"); break;
    case  4096: MINSTLOG_ToFile (" (QUERY_DEFAULT)"); break;
    case  8192: MINSTLOG_ToFile (" (SET_TYPES)"); break;
    case 16384: MINSTLOG_ToFile (" (QUERY_TYPES)"); break;
    }
   MINSTLOG_ToFile ("\n");
   switch (SysInfoID) {
    case 1:
    case 2:
    case 4:
      CurSysInfoLogDev = (PMCI_SYSINFO_LOGDEVICE)SysInfoParms;
      MINSTLOG_ToFile ("     - Name %s\n", CurSysInfoLogDev->szInstallName);
      if (SysInfoID==1) {
         MINSTLOG_ToFile ("     - DeviceType %d\n", CurSysInfoLogDev->usDeviceType);
         MINSTLOG_ToFile ("     - DeviceFlag %d\n", CurSysInfoLogDev->ulDeviceFlag);
         MINSTLOG_ToFile ("     - ResourceClasses %d\n", CurSysInfoLogDev->usResourceClasses);
       }
      break;
    case 32:
      CurSysInfoConn = (PMCI_SYSINFO_CONPARAMS)SysInfoParms;
      MINSTLOG_ToFile ("     - Name %s\n", CurSysInfoConn->szInstallName);
      MINSTLOG_ToFile ("     - ConnectorCount %d\n", CurSysInfoConn->usNumConnectors);
      for (i=0; i<CurSysInfoConn->usNumConnectors; i++) {
         MINSTLOG_ToFile ("     - ConnectorType[%d] %d\n", i, CurSysInfoConn->ConnectorList[i].usConnectType);
         MINSTLOG_ToFile ("     - ConnectorTo[%d] %s\n", i, CurSysInfoConn->ConnectorList[i].szToInstallName);
         MINSTLOG_ToFile ("     - ConnectorIndex[%d] %d\n", i, CurSysInfoConn->ConnectorList[i].usToConnectIndex);
       }
      break;
    case 8:
      CurSysInfoParams = (PMCI_SYSINFO_DEVPARAMS)SysInfoParms;
      MINSTLOG_ToFile ("     - Name %s\n", CurSysInfoParams->szInstallName);
      MINSTLOG_ToFile ("     - Parameters %s\n", CurSysInfoParams->szDevParams);
      break;
    }
 }

// This procedure needs MINSTALL_LinkInImports()!
MRESULT EXPENTRY MINSTALL_CustomAPIProcedure (HWND WindowHandle, ULONG MsgID, MPARAM mp1, MPARAM mp2) {
   MRESULT                   WResult;
   ULONG                     APIResult         = 0;
   PMINSTOLD_CONFIGDATA      ConfigData;
   ULONG                     TmpLen;
   CHAR                      TempBuffer[MAXFILELENGTH];
   ULONG                     CRC32;
   PMINSTFILE                CurFilePtr        = NULL;
   PMINSTOLD_MCI_SENDCOMMAND CurSendCommand    = NULL;
   PMINSTINIHEADER           INIChangeEntryPtr = NULL;
   PCHAR                     CurPos, EndPos, WritePos;
   ULONG                     WriteLeft;
   PMCI_SYSINFO_PARMS        CurSysInfoParms   = NULL;
   PMCI_NETWORK_DEFAULT_CONNECTION_PARMS CurNetDefConn     = NULL;
   USHORT                    i;
   BOOL                      IsBannedOp = FALSE;

//   printf("MsgID=%d\n", MsgID);

   switch (MsgID) {
    case MINSTOLD_LOG_ERROR_MSGID:
      MINSTLOG_ToFile ("Log: %s\n", (PSZ)mp1);
      break;
    case MINSTOLD_QUERYPATH_MSGID:
      if (!STRING_CombinePSZ (TempBuffer, MAXFILELENGTH, (PSZ)mp1, ""))
         break;
      strlwr (TempBuffer);                  // Filename needs to be low-cased
      FILE_SetDefaultExtension (TempBuffer, MAXFILELENGTH, ".dll");
      CRC32 = CRC32_GetFromPSZ(TempBuffer);
      CurFilePtr = MINSTALL_SearchFileCRC32(CRC32);
      MINSTLOG_ToFile ("Querying path for %s...", TempBuffer);
      if (CurFilePtr) {
         STRING_CombinePSZ((PSZ)mp2, CCHMAXPATH, CurFilePtr->SourcePtr->FQName, TempBuffer);
         MINSTLOG_ToFile ("found\n");
       } else {
         memset ((PSZ)mp2, 0, CCHMAXPATH);
         MINSTLOG_ToFile ("not found\n");
       }
      break;
    case MINSTOLD_MCI_SYSINFO_MSGID:
      // Cheap wrapper to MCI - Extended SYSINFO
      // mp1 got FunctionID, mp2 got SysInfoParm
      MINSTLOG_ToFile ("Doing MCI-SYSINFO...\n");
      strcpy(PVOIDFROMMP(mp2), "IBMSEQSB01");
      CustomAPI_AnalyseSYSINFOCalls ((ULONG)LONGFROMMP(mp1), PVOIDFROMMP(mp2));
      APIResult = MCIINI_SendSysInfoExtCommand ((ULONG)LONGFROMMP(mp1), PVOIDFROMMP(mp2));
      break;
    case MINSTOLD_MCI_SENDCOMMAND_MSGID:
      // Wrapper to send an MCI command
      // mp2 got MciSendCommand
      MINSTLOG_ToFile ("Doing MCI-SendCommand...\n");
      CurSendCommand = (PMINSTOLD_MCI_SENDCOMMAND)mp2;
      MINSTLOG_ToFile (" - wDeviceID = %d\n", CurSendCommand->wDeviceID);
      MINSTLOG_ToFile (" - wMessage  = %d\n", CurSendCommand->wMessage);
      MINSTLOG_ToFile (" - dwParam1  = %d\n", CurSendCommand->dwParam1);
      MINSTLOG_ToFile (" - dwParam2  = %d\n", CurSendCommand->dwParam2);
      MINSTLOG_ToFile (" - wUserParm = %d\n", CurSendCommand->wUserParm);
      switch (CurSendCommand->wMessage) {
       case 15:
         CurSysInfoParms = (PMCI_SYSINFO_PARMS)CurSendCommand->dwParam2;
         CustomAPI_AnalyseSYSINFOCalls (CurSysInfoParms->ulItem, (PVOID)CurSysInfoParms->pSysInfoParm);
         break;
       case 35:
         CurNetDefConn = (PMCI_NETWORK_DEFAULT_CONNECTION_PARMS)CurSendCommand->dwParam2;
         MINSTLOG_ToFile ("    NETWORK_DEFAULT_CONNECTION message content:\n");
         MINSTLOG_ToFile ("     - Name %s\n", CurNetDefConn->szInstallName);
         MINSTLOG_ToFile ("     - ulNumDevices %d\n", CurNetDefConn->ulNumDevices);
         MINSTLOG_ToFile ("     - ulNumPlayConnections %d\n", CurNetDefConn->ulNumPlayConnections);
         MINSTLOG_ToFile ("     - ulNumRecordConnections %d\n", CurNetDefConn->ulNumRecordConnections);
         for (i=0; i<MAX_DEVICE_NAME; i++) {
            MINSTLOG_ToFile ("     - Devices[%d] %s\n", i, CurNetDefConn->pDevices[i]);
          }
         MINSTLOG_ToFile ("     - PLAYBACK\n");
         MINSTLOG_ToFile ("     - ulFromDevice = %d\n", CurNetDefConn->pPlayConnections->ulFromDevice);
         MINSTLOG_ToFile ("     - ulSrcConnType = %d\n", CurNetDefConn->pPlayConnections->ulSrcConnType);
         MINSTLOG_ToFile ("     - ulSrcConnType = %d\n", CurNetDefConn->pPlayConnections->ulSrcConnType);
         MINSTLOG_ToFile ("     - ulSrcConnNum = %d\n", CurNetDefConn->pPlayConnections->ulSrcConnNum);
         MINSTLOG_ToFile ("     - ulToDevice = %d\n", CurNetDefConn->pPlayConnections->ulToDevice);
         MINSTLOG_ToFile ("     - ulTgtConnType = %d\n", CurNetDefConn->pPlayConnections->ulTgtConnType);
         MINSTLOG_ToFile ("     - ulTgtConnNum = %d\n", CurNetDefConn->pPlayConnections->ulTgtConnNum);
         MINSTLOG_ToFile ("     - RECORD\n");
         MINSTLOG_ToFile ("     - ulFromDevice = %d\n", CurNetDefConn->pRecordConnections->ulFromDevice);
         MINSTLOG_ToFile ("     - ulSrcConnType = %d\n", CurNetDefConn->pRecordConnections->ulSrcConnType);
         MINSTLOG_ToFile ("     - ulSrcConnType = %d\n", CurNetDefConn->pRecordConnections->ulSrcConnType);
         MINSTLOG_ToFile ("     - ulSrcConnNum = %d\n", CurNetDefConn->pRecordConnections->ulSrcConnNum);
         MINSTLOG_ToFile ("     - ulToDevice = %d\n", CurNetDefConn->pRecordConnections->ulToDevice);
         MINSTLOG_ToFile ("     - ulTgtConnType = %d\n", CurNetDefConn->pRecordConnections->ulTgtConnType);
         MINSTLOG_ToFile ("     - ulTgtConnNum = %d\n", CurNetDefConn->pRecordConnections->ulTgtConnNum);
       }
      APIResult = (*MCIINI_MciSendCommandFunc) (CurSendCommand->wDeviceID, CurSendCommand->wMessage,
                   CurSendCommand->dwParam1, CurSendCommand->dwParam2, CurSendCommand->wUserParm);
      break;
    case MINSTOLD_CONFIG_ENUMERATE_MSGID:
    case MINSTOLD_CONFIG_UPDATE_MSGID:
    case MINSTOLD_CONFIG_MERGE_MSGID:
    case MINSTOLD_CONFIG_REPLACE_MSGID:
    case MINSTOLD_CONFIG_NEW_MSGID:
    case MINSTOLD_CONFIG_DELETE_MSGID:
    case MINSTOLD_CONFIG_QUERYCHANGED_MSGID:
      ConfigData = (PMINSTOLD_CONFIGDATA)mp1;
      switch (ConfigData->lLine) {
       case MINSTOLD_CONFIG_TOP:    CustomAPI_ConfigSysLine = 0; break;
       case MINSTOLD_CONFIG_BOTTOM: CustomAPI_ConfigSysLine = FakedConfigSysFileMaxNo; break;
       case MINSTOLD_CONFIG_NEXT:
         CustomAPI_ConfigSysLine++;
         if (CustomAPI_ConfigSysLine>=FakedConfigSysFileMaxNo) {
            CustomAPI_ConfigSysLine = FakedConfigSysFileMaxNo;
            APIResult = MINSTOLD_RETATBOTTOM;
          }
         break;
       case MINSTOLD_CONFIG_PREV:
         CustomAPI_ConfigSysLine--;
         if (CustomAPI_ConfigSysLine>=FakedConfigSysFileMaxNo) {
            CustomAPI_ConfigSysLine = 0;
            APIResult = MINSTOLD_RETATTOP;
          }
         break;
       case MINSTOLD_CONFIG_CURRENT:
         break;
       default:
         CustomAPI_ConfigSysLine = ConfigData->lLine;
       }
      if (CustomAPI_ConfigSysLine>FakedConfigSysFileMaxNo) {
         APIResult = MINSTOLD_RETLINENOTFOUND;
       } else {
         switch (MsgID) {
          case MINSTOLD_CONFIG_ENUMERATE_MSGID:
            // Get that specific line and copy it into destination buffer
            TmpLen = strlen(FakedConfigSysFile[CustomAPI_ConfigSysLine]);
            if (TmpLen<ConfigData->lBufferLen) {
               strcpy (ConfigData->pszBuffer, FakedConfigSysFile[CustomAPI_ConfigSysLine]);
             } else {
               if (!APIResult) APIResult = MINSTOLD_RETBUFFEROVERFLOW;
             }
            break;
          case MINSTOLD_CONFIG_UPDATE_MSGID:
          case MINSTOLD_CONFIG_NEW_MSGID:
          case MINSTOLD_CONFIG_MERGE_MSGID:
            switch (MsgID) {
              case MINSTOLD_CONFIG_UPDATE_MSGID:
               MINSTLOG_ToFile ("CONFIG.SYS-Update: %s\n", ConfigData->pszBuffer);
               MINSTLOG_ToFile (" -> Converted to insert line command\n");
               break;
              case MINSTOLD_CONFIG_NEW_MSGID:
               MINSTLOG_ToFile ("CONFIG.SYS-Add: %s\n", ConfigData->pszBuffer);
               break;
              case MINSTOLD_CONFIG_MERGE_MSGID:
               MINSTLOG_ToFile ("Merge: %s\n", ConfigData->pszBuffer);
               MINSTLOG_ToFile (" -> Converted to merge line command\n");
               break;
             }
            // Allocate memory for CFGSYS-API, if not already done
            if ((!CustomAPI_ConfigSysActionArrayPtr) || (!CustomAPI_ConfigSysStringArrayPtr)) {
               CustomAPI_ConfigSysActionArrayPtr = malloc(128*sizeof(CONFIGSYSACTION));
               if (!CustomAPI_ConfigSysActionArrayPtr) {
                  APIResult = MINSTOLD_RETOUTOFMEMORY; break; }
               CustomAPI_ConfigSysStringArrayPtr = malloc(128*sizeof(CONFIGSYSACTSTR));
               if (!CustomAPI_ConfigSysStringArrayPtr) {
                  APIResult = MINSTOLD_RETOUTOFMEMORY; break; }

               memset (CustomAPI_ConfigSysActionArrayPtr, 0, 128*sizeof(CONFIGSYSACTION));
               memset (CustomAPI_ConfigSysStringArrayPtr, 0, 128*sizeof(CONFIGSYSACTSTR));
               CustomAPI_ConfigSysActionCurPtr = CustomAPI_ConfigSysActionArrayPtr;
               CustomAPI_ConfigSysStringCurPtr = CustomAPI_ConfigSysStringArrayPtr;
               CustomAPI_ConfigSysActionCount  = 0;
             }
            // Don't allow more than 128 changes to CONFIG.SYS
            if (CustomAPI_ConfigSysActionCount==128) {
               APIResult = MINSTOLD_RETOUTOFMEMORY; break; }

            // Don't allow lines that are bigger than 1024 bytes (our limit)
            CurPos    = ConfigData->pszBuffer;
            if (strlen(CurPos)>=1024) {
               APIResult = MINSTOLD_RETOUTOFMEMORY; break; }
            EndPos    = (PCHAR)(((ULONG)CurPos)+strlen(CurPos));

            // Now those changes are converted into CFGSYS-API commands...
            CustomAPI_ConfigSysActionCurPtr->Flags = 0;
            CustomAPI_ConfigSysActionCurPtr->CommandStrPtr     = CustomAPI_ConfigSysStringCurPtr->CommandStr;
            CustomAPI_ConfigSysActionCurPtr->MatchStrPtr       = CustomAPI_ConfigSysStringCurPtr->MatchStr;
            CustomAPI_ConfigSysActionCurPtr->MatchInLineStrPtr = CustomAPI_ConfigSysStringCurPtr->MatchInLineStr;
            CustomAPI_ConfigSysActionCurPtr->ValueStrPtr       = CustomAPI_ConfigSysStringCurPtr->ValueStr;

            WritePos  = CustomAPI_ConfigSysStringCurPtr->CommandStr;
            WriteLeft = CONFIGSYSACTSTR_MAXLENGTH-1;
            while (CurPos<EndPos) {
               if ((*CurPos=='=') || (*CurPos==' ')) {
                  CurPos++; break; }
               if (WriteLeft>0) {
                  *WritePos = *CurPos; WritePos++; WriteLeft--; }
               CurPos++;
             }

            WritePos  = CustomAPI_ConfigSysStringCurPtr->ValueStr;
            WriteLeft = CONFIGSYSACTSTR_MAXLENGTH-1;
            while (CurPos<EndPos) {
               if (WriteLeft>0) {
                  *WritePos = *CurPos; WritePos++; WriteLeft--; }
               CurPos++;
             }

            switch (MsgID) {
              case MINSTOLD_CONFIG_UPDATE_MSGID:
              case MINSTOLD_CONFIG_NEW_MSGID:
               // Get filename on BASEDEV/DEVICE and RUN operations and compare
               //  it to our blacklist - ignore if its found...
               strcpy(TempBuffer, CustomAPI_ConfigSysStringCurPtr->ValueStr);
               strlwr (TempBuffer); // operation needs to be low-cased
               // Find out where actual filename starts
               CurPos = (PCHAR)&TempBuffer;
               EndPos = (PCHAR)(((ULONG)&TempBuffer)+strlen(TempBuffer));
               while ((CurPos<EndPos) && (*CurPos!=' '))
                  CurPos++;
               // First space occurance -> put NUL there
               if (*CurPos==' ') *CurPos = 0;
               CurPos = EndPos;
               while ((CurPos>(PCHAR)&TempBuffer) && (*CurPos!='\\'))
                  CurPos--;
               // Set CurPos to start of filename
               if (*CurPos=='\\') {
                  CurPos++;
                } else {
                  CurPos = (PCHAR)&TempBuffer;
                }

               CRC32 = CRC32_GetFromPSZ(CurPos);
               switch (CRC32) {
                case 0xB8E85432: // timer0.sys (Timing PDD)
                case 0x84F52EBF: // clock01.sys (Timing PDD)
                case 0xC355546F: // clock02.sys (Timing PDD)
                case 0x6DDDA492: // resource.sys (Resource Manager)
                  IsBannedOp = TRUE;
                  break;
                }
               MINSTLOG_ToFile (" -> Converted INSERT '%s' - '%s'\n", CustomAPI_ConfigSysStringCurPtr->CommandStr, CustomAPI_ConfigSysStringCurPtr->ValueStr);
               break;
              case MINSTOLD_CONFIG_MERGE_MSGID:
               CustomAPI_ConfigSysActionCurPtr->Flags |= CONFIGSYSACTION_Flags_Merge;
               MINSTLOG_ToFile (" -> Converted MERGE '%s' - '%s'\n", CustomAPI_ConfigSysStringCurPtr->CommandStr, CustomAPI_ConfigSysStringCurPtr->ValueStr);
               break;
             }
            if (IsBannedOp) {
               MINSTLOG_ToFile ("     (!BANNED OPERATION, GOT IGNORED!)\n");
             } else {
               // Copy ValueStr to MatchStr, so duplicates will get overwritten
               strcpy (CustomAPI_ConfigSysStringCurPtr->MatchStr, CustomAPI_ConfigSysStringCurPtr->ValueStr);
               CustomAPI_ConfigSysActionCurPtr++; CustomAPI_ConfigSysStringCurPtr++;
               CustomAPI_ConfigSysActionCount++;
             }
            break;
          case MINSTOLD_CONFIG_QUERYCHANGED_MSGID:
            APIResult = TRUE;               // We reply TRUE at any time
            break;
          // Unsupported functions...
          case MINSTOLD_CONFIG_REPLACE_MSGID:
            MINSTLOG_ToFile ("Tried to replace CONFIG.SYS, not supported!\n");
            break;
          case MINSTOLD_CONFIG_DELETE_MSGID:
            MINSTLOG_ToFile ("Tried to delete line, not supported!\n");
            break;
          }
       }
      break;
    case MINSTOLD_EA_JOIN_MSGID:
      // Joins EAs from a file onto a file/path
      MINSTLOG_ToFile ("Delaying EA_JOINEA...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(EA_JOINEA_ID, sizeof(EA_JOINEA)))!=0) {
         strcpy (((PEA_JOINEA)INIChangeEntryPtr)->JoinFileName, ((PMINSTOLD_EA_JOIN)mp2)->achFileName);
         strcpy (((PEA_JOINEA)INIChangeEntryPtr)->JoinEAFileName, ((PMINSTOLD_EA_JOIN)mp2)->achEAFileName);
       }
      break;
    case MINSTOLD_EA_LONGNAMEJOIN_MSGID:
      // Joins EAs from a file onto a file/path, also sets long-name
      MINSTLOG_ToFile ("Delaying EA_LONGNAMEJOIN...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(EA_JOINLONGNAMEEA_ID, sizeof(EA_JOINLONGNAMEEA)))!=0) {
         strcpy (((PEA_JOINLONGNAMEEA)INIChangeEntryPtr)->JoinLongName, ((PMINSTOLD_EA_LONGNAMEJOIN)mp2)->achLongName);
         strcpy (((PEA_JOINLONGNAMEEA)INIChangeEntryPtr)->JoinLongFileName, ((PMINSTOLD_EA_LONGNAMEJOIN)mp2)->achLongFileName);
         strcpy (((PEA_JOINLONGNAMEEA)INIChangeEntryPtr)->JoinEALongFileName, ((PMINSTOLD_EA_LONGNAMEJOIN)mp2)->achEALongFileName);
       }
      break;
    case MINSTOLD_MMIO_INSTALL_MSGID:
      // Installs an IO-Proc (using obscure MINSTALL format)
      MINSTLOG_ToFile ("Delaying MMIO_INSTALL...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(MMIO_MMIOINSTALL_ID, sizeof(MMIO_MMIOINSTALL)))!=0) {
         ((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->fccIOProc    = (ULONG)((PMINSTOLD_MMIO_INSTALL)mp2)->fccIOProc;
         strcpy (((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->szDLLName, ((PMINSTOLD_MMIO_INSTALL)mp2)->szDLLName);
         strcpy (((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->szProcName, ((PMINSTOLD_MMIO_INSTALL)mp2)->szProcName);
         ((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->dwFlags      = ((PMINSTOLD_MMIO_INSTALL)mp2)->ulFlags;
         ((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->dwExtendLen  = ((PMINSTOLD_MMIO_INSTALL)mp2)->ulExtendLen;
         ((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->dwMediaType  = ((PMINSTOLD_MMIO_INSTALL)mp2)->ulMediaType;
         ((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->dwIOProcType = ((PMINSTOLD_MMIO_INSTALL)mp2)->ulIOProcType;
         strcpy (((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->szDefExt, ((PMINSTOLD_MMIO_INSTALL)mp2)->szDefExt);
       }
      break;
    case MINSTOLD_MMIO_CODEC1INSTALL_MSGID:
    case MINSTOLD_MMIO_CODEC2INSTALL_MSGID:
      // Installs an IO-Codec (using ulCodecCompType or fccCodecCompType)
      MINSTLOG_ToFile ("Delaying MMIO_CODECxINSTALL...\n");
      // We get an MMIOCODEC1INSTALL-Block, in fact it doesnt matter because
      //  process routine handles both the same way (due same buffer format)
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(MMIO_MMIOCODEC1INSTALL_ID, sizeof(MMIO_MMIOCODEC)))!=0) {
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulStructLen        = ((PMINSTOLD_MMIO_CODEC)mp2)->ulStructLen;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fcc                = (ULONG)((PMINSTOLD_MMIO_CODEC)mp2)->fcc;
         strcpy (((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szDLLName, ((PMINSTOLD_MMIO_CODEC)mp2)->szDLLName);
         strcpy (((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szProcName, ((PMINSTOLD_MMIO_CODEC)mp2)->szProcName);
         if (MsgID==MINSTOLD_MMIO_CODEC1INSTALL_MSGID) {
            ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressType  = ((PMINSTOLD_MMIO_CODEC)mp2)->x.ulCodecCompType;
          } else {
            ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressType  = (ULONG)((PMINSTOLD_MMIO_CODEC)mp2)->x.fccCodecCompType;
          }
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressSubType  = ((PMINSTOLD_MMIO_CODEC)mp2)->ulCompressSubType;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMediaType        = ((PMINSTOLD_MMIO_CODEC)mp2)->ulMediaType;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCapsFlags        = ((PMINSTOLD_MMIO_CODEC)mp2)->ulCapsFlags;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulFlags            = ((PMINSTOLD_MMIO_CODEC)mp2)->ulFlags;
         strcpy (((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szHWID, ((PMINSTOLD_MMIO_CODEC)mp2)->szHWID);
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMaxSrcBufLen     = ((PMINSTOLD_MMIO_CODEC)mp2)->ulMaxSrcBufLen;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulSyncMethod       = ((PMINSTOLD_MMIO_CODEC)mp2)->ulSyncMethod;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fccPreferredFormat = ((PMINSTOLD_MMIO_CODEC)mp2)->fccPreferredFormat;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulXalignment       = ((PMINSTOLD_MMIO_CODEC)mp2)->ulXalignment;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulYalignment       = ((PMINSTOLD_MMIO_CODEC)mp2)->ulYalignment;
         strcpy (((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szSpecInfo, ((PMINSTOLD_MMIO_CODEC)mp2)->szSpecInfo);
       }
      break;
    case MINSTOLD_MMIO_CODECDELETE_MSGID:
      // Deletes an IO-Codec
      MINSTLOG_ToFile ("Delaying MMIO_CODECDELETE...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(MMIO_MMIOCODECDELETE_ID, sizeof(MMIO_MMIOCODEC)))!=0) {
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulStructLen        = ((PMINSTOLD_MMIO_CODEC)mp2)->ulStructLen;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fcc                = (ULONG)((PMINSTOLD_MMIO_CODEC)mp2)->fcc;
         strcpy (((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szDLLName, ((PMINSTOLD_MMIO_CODEC)mp2)->szDLLName);
         strcpy (((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szProcName, ((PMINSTOLD_MMIO_CODEC)mp2)->szProcName);
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressType  = ((PMINSTOLD_MMIO_CODEC)mp2)->x.ulCodecCompType;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressSubType  = ((PMINSTOLD_MMIO_CODEC)mp2)->ulCompressSubType;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMediaType        = ((PMINSTOLD_MMIO_CODEC)mp2)->ulMediaType;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCapsFlags        = ((PMINSTOLD_MMIO_CODEC)mp2)->ulCapsFlags;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulFlags            = ((PMINSTOLD_MMIO_CODEC)mp2)->ulFlags;
         strcpy (((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szHWID, ((PMINSTOLD_MMIO_CODEC)mp2)->szHWID);
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMaxSrcBufLen     = ((PMINSTOLD_MMIO_CODEC)mp2)->ulMaxSrcBufLen;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulSyncMethod       = ((PMINSTOLD_MMIO_CODEC)mp2)->ulSyncMethod;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fccPreferredFormat = ((PMINSTOLD_MMIO_CODEC)mp2)->fccPreferredFormat;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulXalignment       = ((PMINSTOLD_MMIO_CODEC)mp2)->ulXalignment;
         ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulYalignment       = ((PMINSTOLD_MMIO_CODEC)mp2)->ulYalignment;
         strcpy (((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szSpecInfo, ((PMINSTOLD_MMIO_CODEC)mp2)->szSpecInfo);
       }
      break;
    case MINSTOLD_PRF_STRINGDATA_MSGID:
      // Adds a profile string to an INI file
      MINSTLOG_ToFile ("Delaying PRF_STRINGDATA...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(PRF_PROFILESTRING_ID, sizeof(PRF_PROFILESTRING)))!=0) {
         strcpy (((PPRF_PROFILESTRING)INIChangeEntryPtr)->Inis, ((PMINSTOLD_PRF_STRINGDATA)mp2)->achInisName);
         strcpy (((PPRF_PROFILESTRING)INIChangeEntryPtr)->AppNames, ((PMINSTOLD_PRF_STRINGDATA)mp2)->achAppsName);
         strcpy (((PPRF_PROFILESTRING)INIChangeEntryPtr)->KeyNames, ((PMINSTOLD_PRF_STRINGDATA)mp2)->achKeysName);
         strcpy (((PPRF_PROFILESTRING)INIChangeEntryPtr)->Datas, ((PMINSTOLD_PRF_STRINGDATA)mp2)->achDatasName);
       }
      break;
    case MINSTOLD_PRF_APPENDDATA_MSGID:
      // Appends a string onto an INI file
      MINSTLOG_ToFile ("Delaying PRF_APPENDDATA (UNSUPPORTED!)...\n");
      break;
    case MINSTOLD_SPI_INSTALL_MSGID:
      // Just an SpiInstall (primitive forward)
      MINSTLOG_ToFile ("Delaying SPI_INSTALL...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(SPI_SPIINSTALL_ID, sizeof(SPI_SPIINSTALL)))!=0) {
         strcpy (((PSPI_SPIINSTALL)INIChangeEntryPtr)->SpiDllName, (PSZ)mp2);
       }
      break;
    case MINSTOLD_WPS_CREATEOBJECT_MSGID:
      // Generates a WPS-object
      MINSTLOG_ToFile ("Delaying WPS_CREATEOBJECT...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(WPS_CREATEOBJECT_ID, sizeof(WPS_CREATEOBJECT)))!=0) {
         strcpy (((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPClassName, ((PMINSTOLD_WPS_CREATEOBJECT)mp2)->achClassName);
         strcpy (((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPTitle, ((PMINSTOLD_WPS_CREATEOBJECT)mp2)->achTitle);
         strcpy (((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPSetupString, ((PMINSTOLD_WPS_CREATEOBJECT)mp2)->achSetupString);
         strcpy (((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPLocation, ((PMINSTOLD_WPS_CREATEOBJECT)mp2)->achLocation);
         ((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPFlags = ((PMINSTOLD_WPS_CREATEOBJECT)mp2)->ulFlags;
       }
      break;
    case MINSTOLD_WPS_DESTROYOBJECT_MSGID:
      // Removes a WPS-object
      MINSTLOG_ToFile ("Delaying WPS_DESTROYOBJECT...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(WPS_DESTROYOBJECT_ID, sizeof(WPS_DESTROYOBJECT)))!=0) {
         strcpy (((PWPS_DESTROYOBJECT)INIChangeEntryPtr)->WPDestroyObjectID, (PSZ)mp2);
       }
      break;
    case MINSTOLD_WPS_WPCLASS_MSGID:
      // Registers a WPS-class and is able to replace another class with it
      MINSTLOG_ToFile ("Delaying WPS_WPCLASS...\n");
      if ((INIChangeEntryPtr = MINSTALL_CustomAPIAllocINIChange(WPS_WPCLASS_ID, sizeof(WPS_WPCLASS)))!=0) {
         strcpy (((PWPS_WPCLASS)INIChangeEntryPtr)->WPClassNameNew, ((PMINSTOLD_WPS_WPCLASS)mp2)->achClassNewName);
         strcpy (((PWPS_WPCLASS)INIChangeEntryPtr)->WPDllName, ((PMINSTOLD_WPS_WPCLASS)mp2)->achDllName);
         strcpy (((PWPS_WPCLASS)INIChangeEntryPtr)->WPReplaceClass, ((PMINSTOLD_WPS_WPCLASS)mp2)->achReplaceClass);
       }
      break;
    case MINSTOLD_MIDIMAP_INSTALL_MSGID:
      // Installs a MIDIMAP
      MINSTLOG_ToFile ("Delaying MIDIMAP_INSTALL (UNSUPPORTED!)...\n");
      break;
    default:
      MINSTLOG_ToFile ("Unsupported MsgID: MsgID %d\n", MsgID);
      WResult = WinDefWindowProc(WindowHandle, MsgID, mp1, mp2);
      return WResult;
    }
   WResult = (PVOID)APIResult;
   return WResult;
 }

VOID MINSTALL_CustomAPIThread (PVOID pvoid) {
   QMSG  qmsg;
   ERRORID ErrorID;

   do {
      // We need our own PM and message queue handle in here...
      if (!(CustomAPI_PMHandle = WinInitialize(0)))
         break;

      if (!(CustomAPI_MSGQHandle = WinCreateMsgQueue(CustomAPI_PMHandle, 0)))
         break;

      if (!WinRegisterClass(CustomAPI_PMHandle, "OBJECTWINDOW", (PFNWP)MINSTALL_CustomAPIProcedure, 0, 0))
         break;

      if (!(CustomAPI_WindowHandle = WinCreateWindow(HWND_OBJECT, "OBJECTWINDOW",
          NULL, 0, 0, 0, 0, 0, NULLHANDLE, HWND_TOP, 1, NULL, NULL)))
         break;

      CustomAPI_ThreadCreated = TRUE;
      DosPostEventSem (CustomAPI_InitEventHandle);
                                                    /* Message loop */
      while(WinGetMsg(CustomAPI_MSGQHandle, &qmsg, NULLHANDLE, 0UL, 0UL))
         WinDispatchMsg (CustomAPI_MSGQHandle, &qmsg);
   } while (0);

   if (WinIsWindow(CustomAPI_PMHandle, CustomAPI_WindowHandle))
      WinDestroyWindow (CustomAPI_WindowHandle);
   CustomAPI_WindowHandle = 0;

   /* Release CONFIG-SYS API related memory */
   if (CustomAPI_ConfigSysStringArrayPtr) free (CustomAPI_ConfigSysStringArrayPtr);
   if (CustomAPI_ConfigSysActionArrayPtr) free (CustomAPI_ConfigSysActionArrayPtr);

   if (CustomAPI_MSGQHandle)   WinDestroyMsgQueue (CustomAPI_MSGQHandle);
   if (CustomAPI_WindowHandle) WinTerminate (CustomAPI_WindowHandle);

   if (!CustomAPI_ThreadCreated)
      DosPostEventSem (CustomAPI_InitEventHandle);
   return;
 }

BOOL MINSTALL_CreateCustomAPIThread (void) {
   APIRET rc;

   CustomAPI_ThreadCreated = FALSE;
   if (!(DosCreateEventSem(NULL, &CustomAPI_InitEventHandle, DC_SEM_SHARED, FALSE))) {
      if ((CustomAPI_ThreadID = _beginthread ((THREADFUNC)&MINSTALL_CustomAPIThread, NULL, 65536, NULL))!=-1) {
         DosWaitEventSem (CustomAPI_InitEventHandle, -1);
       }
      DosCloseEventSem (CustomAPI_InitEventHandle);
    }
   return CustomAPI_ThreadCreated;
 }

VOID MINSTALL_RemoveCustomAPIThread (void) {
   if (!WinPostMsg(CustomAPI_WindowHandle, WM_QUIT, NULL, NULL)) {
      DosWaitThread (&CustomAPI_ThreadID, DCWW_WAIT);
    }
 }

// 10.07.2005 - compatibility code for bad custom DLLs
//               This code executes the custom entrypoints via their own
//               threads, so if the custom DLL closes things like PM handle
//               we won't be affected

VOID MINSTALL_CallCustomEntryPointThread (PVOID pvoid) {
   HAB  ThreadPMHandle = 0;
   HMQ  ThreadMSGQHandle = 0;

   do {
      // We initiate PM in this thread as well
      if (!(ThreadPMHandle = WinInitialize(0)))
         break;

      if (!(ThreadMSGQHandle = WinCreateMsgQueue(ThreadPMHandle, 0)))
         break;

      ((PCUSTOMDLL_ENTRYFUNC)CustomDLL_EntryPoint) (0, MINSTALL_SourcePath, MINSTALL_MMBaseDrive,
        CustomDLL_EntryParms, CustomAPI_WindowHandle, CustomDLL_CustomData);
   } while (0);

   if (ThreadMSGQHandle) WinDestroyMsgQueue (ThreadMSGQHandle);
   if (ThreadPMHandle)   WinTerminate (ThreadPMHandle);
   return;
 }

// Creates another thread, that calls the entrypoint and waits for this thread
//  to finish.
VOID MINSTALL_CallCustomEntryPoint (PCUSTOMDLL_ENTRYFUNC EntryPoint, PSZ EntryParms, PSZ CustomData) {
   TID CustomEntryPointThread = 0;
   // Set public variables, so that the thread knows what to do...
   CustomDLL_EntryPoint = (PVOID)EntryPoint;
   CustomDLL_EntryParms = EntryParms;
   CustomDLL_CustomData = CustomData;
   if ((CustomEntryPointThread = _beginthread ((THREADFUNC)&MINSTALL_CallCustomEntryPointThread, NULL, 131072, NULL))!=-1) {
      DosWaitThread(&CustomEntryPointThread, DCWW_WAIT);
    }
 }

BOOL MINSTALL_ExecuteCustomDLLs (void) {
   PMINSTGRP            CurGroupPtr = MCF_GroupArrayPtr;
   PMINSTFILE           CurFilePtr  = 0;
   USHORT               CurNo       = 0;
   USHORT               CurChangeNo = 0;
   CHAR                 DLLFileName[MINSTMAX_PATHLENGTH];
   HMODULE              DLLHandle   = 0;
   PCUSTOMDLL_ENTRYFUNC CustomDLLEntryPoint = 0;
   BOOL                 GotDLL = FALSE;

   while (CurNo<MCF_GroupCount) {
      if (CurGroupPtr->Flags & MINSTGRP_Flags_Selected) {
         if (CurGroupPtr->DLLFilePtr) {
            // CustomDLL got defined...
            if (!GotDLL) {
               // Initiate Custom-API Thread
               if (!MINSTALL_CreateCustomAPIThread()) {
               MINSTLOG_ToFile ("Custom-API: Thread init failed\n"); return FALSE; }
               MINSTLOG_ToAll ("Custom-API: Thread opened\n");
               GotDLL = TRUE;
               /* 26.06.2005 - implemented compatibility code */
               /*               saves current drive and directory and sets */
               /*               current directory to be in source path */
               /*               Some badly custom DLLs need this */
               MINSTALL_SaveCurrentDirectory();
               MINSTALL_SetCurrentDirectoryToSource();
             }
            CurFilePtr = CurGroupPtr->DLLFilePtr;
            if (STRING_CombinePSZ(DLLFileName, MINSTMAX_PATHLENGTH, CurFilePtr->SourcePtr->FQName, CurFilePtr->Name)) {
               if ((DLLHandle = DLL_Load(DLLFileName))!=0) {
                  if (!(CustomDLLEntryPoint = (PCUSTOMDLL_ENTRYFUNC)DLL_GetEntryPoint (DLLHandle, CurGroupPtr->DLLEntry))) {
                     MINSTLOG_ToAll ("Custom-API: Entrypoint not found\n");
                   } else {
                     MINSTLOG_ToAll (" - Calling custom entrypoint\n");
                     CustomAPI_ConfigSysLine = 0;
                     // 10.07.2005 - compatibility code for bad custom DLLs
                     MINSTALL_CallCustomEntryPoint(CustomDLLEntryPoint, CurGroupPtr->DLLParms, CurGroupPtr->CustomData);
//                     (CustomDLLEntryPoint) (0, MINSTALL_SourcePath, MINSTALL_MMBaseDrive,
//                      CurGroupPtr->DLLParms, CustomAPI_WindowHandle, CurGroupPtr->CustomData);
                   }
                  DLL_UnLoad (DLLHandle);
                }
             }
          }
       }
      CurGroupPtr++; CurNo++;
    }
   if (GotDLL) {
      MINSTLOG_ToFile ("Total Custom-API-CONFIG.SYS Actions: %d\n", CustomAPI_ConfigSysActionCount);
      if (CustomAPI_ConfigSysActionCount) {
         MINSTALL_ProcessConfigControl (CustomAPI_ConfigSysActionCount, CustomAPI_ConfigSysActionArrayPtr);
       }
      MINSTALL_RemoveCustomAPIThread();
      MINSTLOG_ToAll ("Custom-API: Thread closed\n");
      /* 26.06.2005 - implemented compatibility code */
      /*               restores current directory in case */
      MINSTALL_RestoreCurrentDirectory();
    }
   return TRUE;
 }

BOOL MINSTALL_ExecuteCustomTermDLLs (void) {
   PMINSTGRP            CurGroupPtr = MCF_GroupArrayPtr;
   PMINSTFILE           CurFilePtr  = 0;
   USHORT               CurNo       = 0;
   USHORT               CurChangeNo = 0;
   CHAR                 DLLFileName[MINSTMAX_PATHLENGTH];
   HMODULE              DLLHandle   = 0;
   PCUSTOMDLL_ENTRYFUNC CustomDLLEntryPoint = 0;
   CHAR                 CustomData[MINSTMAX_CUSTOMDATALENGTH];
   BOOL                 GotDLL = FALSE;

   while (CurNo<MCF_GroupCount) {
      if (CurGroupPtr->Flags & MINSTGRP_Flags_Selected) {
         if (CurGroupPtr->TermDLLFilePtr) {
            // CustomDLL got defined...
            if (!GotDLL) {
               // Initiate Custom-API Thread
               if (!MINSTALL_CreateCustomAPIThread()) {
               MINSTLOG_ToFile ("Custom-Terminate-API: Thread init failed\n"); return FALSE; }
               MINSTLOG_ToAll ("Custom-Terminate-API: Thread opened\n");
               GotDLL = TRUE;
               /* 26.06.2005 - implemented compatibility code */
               /*               saves current drive and directory and sets */
               /*               current directory to be in source path */
               /*               Some badly custom DLLs need this */
               MINSTALL_SaveCurrentDirectory();
               MINSTALL_SetCurrentDirectoryToSource();
             }
            CurFilePtr = CurGroupPtr->TermDLLFilePtr;
            if (STRING_CombinePSZ(DLLFileName, MINSTMAX_PATHLENGTH, CurFilePtr->SourcePtr->FQName, CurFilePtr->Name)) {
               if ((DLLHandle = DLL_Load(DLLFileName))!=0) {
                  if (!(CustomDLLEntryPoint = (PCUSTOMDLL_ENTRYFUNC)DLL_GetEntryPoint (DLLHandle, CurGroupPtr->TermDLLEntry))) {
                     MINSTLOG_ToFile ("Custom-Terminate-API: Entrypoint not found\n");
                   } else {
                     MINSTLOG_ToAll (" - Calling custom terminate-entrypoint\n");
                     memset (CustomData, 0, sizeof(CustomData));
                     CustomAPI_ConfigSysLine = 0;
                     // 10.07.2005 - compatibility code for bad custom DLLs
                     MINSTALL_CallCustomEntryPoint(CustomDLLEntryPoint, CurGroupPtr->DLLParms, CurGroupPtr->CustomData);
//                     (CustomDLLEntryPoint) (0, MINSTALL_SourcePath, MINSTALL_MMBaseDrive,
//                      "", CustomAPI_WindowHandle, CustomData);
                   }
                  DLL_UnLoad (DLLHandle);
                }
             }
          }
       }
      CurGroupPtr++; CurNo++;
    }
   if (GotDLL) {
      MINSTLOG_ToFile ("Total Custom-API-CONFIG.SYS Actions: %d\n", CustomAPI_ConfigSysActionCount);
      if (CustomAPI_ConfigSysActionCount) {
         MINSTALL_ProcessConfigControl (CustomAPI_ConfigSysActionCount, CustomAPI_ConfigSysActionArrayPtr);
       }
      MINSTALL_RemoveCustomAPIThread();
      MINSTLOG_ToAll ("Custom-Terminate-API: Thread closed\n");
      /* 26.06.2005 - implemented compatibility code */
      /*               restores current directory in case */
      MINSTALL_RestoreCurrentDirectory();
    }
   return TRUE;
 }
