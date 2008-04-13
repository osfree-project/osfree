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
// #define INCL_MMIO_CODEC
// #define INCL_AUDIO_CODEC_ONLY
#include <os2.h>
// #include <os2me.h>
#include <malloc.h>

#include <global.h>
#include <cfgsys.h>                         // CONFIG.SYS Changing
#include <crcs.h>
#include <file.h>
#include <globstr.h>
#include <msg.h>
#include <mmi_public.h>
#include <mmi_types.h>
#include <mmi_main.h>
#include <mmi_helper.h>
#include <mmi_msg.h>
#include <mmi_inistuff.h>

BOOL MINSTALL_LoadConfigControlFile (PMINSTFILE ScriptFilePtr) {
   ULONG            ConfigEntryCount     = 0;
   PCHAR            CurPos               = 0;
   PCHAR            EndPos               = 0;
   PCHAR            LineStartPos         = 0;
   PCHAR            LineEndPos           = 0;
   CHAR             CurChar              = 0;
   ULONG            Temp                 = 0;
   PCHAR            CommandSpacePtr      = 0;
   PCONFIGSYSACTION ConfigEntryArrayPtr  = 0;
   PCONFIGSYSACTION CurConfigEntry       = 0;
   PCONFIGSYSACTSTR ConfigStringArrayPtr = 0;
   PCONFIGSYSACTSTR CurConfigString      = 0;
   PMINSTDIR        CurDirPtr            = 0;
   ULONG            CommandID1           = 0;
   ULONG            CommandID2           = 0;
   PCHAR            ValueSpacePtr        = 0;
   PCHAR            TempPtr              = 0;
   ULONG            ActionID             = 0;
   ULONG            CurLineNo            = 1;

   // Get Full-Qualified Script Name
   if (!STRING_CombinePSZ (CHANGESCR.Name, MINSTMAX_PATHLENGTH, ScriptFilePtr->SourcePtr->FQName, ScriptFilePtr->Name))
      return FALSE;
   if (!FILE_LoadFileControl(&CHANGESCR, 131767)) {
      MSG_SetInsertViaPSZ (1, CHANGESCR.Name);
      MINSTALL_TrappedError (MINSTMSG_CouldNotLoad);
      return FALSE;
    }
   FILE_PreProcessControlFile(&CHANGESCR);
   ConfigEntryCount = FILE_CountControlFileLines (&CHANGESCR);
   if (ConfigEntryCount==0) {
      FILE_UnLoadFileControl(&CHANGESCR);
      return TRUE;                          // No entries, so success
    }

   // Now allocate memory for those entries...
   ConfigEntryArrayPtr = malloc(ConfigEntryCount*CONFIGSYSACTION_Length);
   if (!ConfigEntryArrayPtr) {
      FILE_UnLoadFileControl(&CHANGESCR);
      MINSTALL_TrappedError (MINSTMSG_OutOfMemory); return FALSE; // OutOfMemory
    }
   ConfigStringArrayPtr = malloc(ConfigEntryCount*CONFIGSYSACTSTR_Length);
   if (!ConfigStringArrayPtr) {
      free (ConfigEntryArrayPtr);
      FILE_UnLoadFileControl(&CHANGESCR);
      MINSTALL_TrappedError (MINSTMSG_OutOfMemory); return FALSE; // OutOfMemory
    }

   // NUL out both arrays...
   memset (ConfigEntryArrayPtr, 0, ConfigEntryCount*sizeof(CONFIGSYSACTION));
   memset (ConfigStringArrayPtr, 0, ConfigEntryCount*sizeof(CONFIGSYSACTSTR));

   // Now extract all entries one-by-one. Known are:
   //  "MERGE"   - only used on SET lines,
   //               may include a numeric digit, which specifies the directory
   //  "REPLACE" - will replace a line, if it already got found. Otherwise
   //               the line will get added
   //               05062004 - If a path is specified, Action is 'REPLACE' and
   //                           the command is 'SET', add ';' to the end.
   //                          It's done by original minstall and needed for
   //                          compatibility.
   //  "DEVICE"  - will add that line

   ConfigEntryCount = 0;
   CurPos = CHANGESCR.BufferPtr; EndPos = CHANGESCR.BufferEndPtr;
   CurConfigEntry = ConfigEntryArrayPtr;
   CurConfigString = ConfigStringArrayPtr;
   while (CurPos<EndPos) {
      if (!(CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo)))
         break;
      LineStartPos = CurPos;
      LineEndPos   = STRING_GetEndOfLinePtr (CurPos, EndPos);

      while ((CurPos<LineEndPos) && (*CurPos!=0x20) && (*CurPos!=0x3D)) {
         *CurPos = toupper(*CurPos);     // Uppercase...
         CurPos++;                       // Search for space or '='
       }
      ActionID = CRC32_GetFromString (LineStartPos, CurPos-LineStartPos);

      if ((CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo))!='=') {
         // Read in Command/Specifier into TempBuffer...
         CurPos = STRING_GetString(CurConfigString->CommandStr, CONFIGSYSACTSTR_MAXLENGTH, CurPos, LineEndPos);
         CommandSpacePtr = (PCHAR)CurConfigString->CommandStr;
         while (*CommandSpacePtr!=0x20) {  // Find 1st space in HelpBuffer
            if (*CommandSpacePtr==0x00) {
               CommandSpacePtr = 0; break;
             }
            CommandSpacePtr++;
          }
         CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo);
       } else {
         CommandSpacePtr = 0;
       }
      CurPos++;

      if (CurPos>=LineEndPos) {
         MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfLine; break; }

      if (CurChar!='=') {
         MINSTALL_ErrorMsgID = MINSTMSG_ValueExpected; break; }

      if (ActionID==0xF9D77108) {
         /* We have to check this earlier, because we put command 'DEVICE' */
         /*  and that one is needed because we get CommandID1/2 now instead of */
         /*  later. */
         if (CurConfigString->CommandStr[0]!=0) MINSTALL_TrappedError (MINSTMSG_NoConfigCommandExpected);
         strcpy (CurConfigString->CommandStr, "DEVICE");
       }

      if (CommandSpacePtr) {
         // We got a space in the Command, so we take 2 CommandIDs
         CommandID1 = CRC32_GetFromString(CurConfigString->CommandStr,CommandSpacePtr-CurConfigString->CommandStr);
         CommandID2 = CRC32_GetFromPSZ(CommandSpacePtr+1);
       } else {
         CommandID1 = CRC32_GetFromPSZ(CurConfigString->CommandStr);
         CommandID2 = 0;
       }

      CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo);
      if (CurChar=='"') {
         // String-Delimiter, so we assume string and extract (w macros)
         if (!(CurPos = MINSTALL_GetMacrodString((PCHAR)CurConfigString->ValueStr, MINSTMAX_PATHLENGTH, CurPos, LineEndPos)))
            break;                 // Error during macro processing
       } else {
         if (!(CurPos = STRING_GetNumericValue(&Temp, CurPos, LineEndPos)))
            break;                 // Error during value extract
         CurDirPtr = MINSTALL_SearchDestinDirID (Temp);
         if (CurDirPtr) {
            strcpy (CurConfigString->ValueStr, CurDirPtr->FQName);
            Temp = strlen(CurConfigString->ValueStr);
            if (Temp>0) Temp--;
            if ((ActionID==0x33D8DA5F) && (CommandID1==0x70B36756)) {
               // If Action is 'REPLACE' and Command is 'SET'
               //  Add a ';' here for compatibility reasons (05062004)
               CurConfigString->ValueStr[Temp] = ';';
             } else {
               // cut last char (which is a '\')
               CurConfigString->ValueStr[Temp] = 0;
             }
          } else {
            MINSTALL_ErrorMsgID = MINSTMSG_UnknownDestinID; break;
          }
       }

      switch (ActionID) {
       case 0x33D8DA5F: // REPLACE, expects HelpBuffer
         if (CurConfigString->CommandStr[0]==0) MINSTALL_TrappedError (MINSTMSG_ConfigCommandExpected);
         break;
       case 0x1C3D55E9: // MERGE, expects HelpBuffer
         if (CurConfigString->CommandStr[0]==0) MINSTALL_TrappedError (MINSTMSG_ConfigCommandExpected);
         CurConfigEntry->Flags |= CONFIGSYSACTION_Flags_Merge;
         break;
       case 0xF9D77108: // DEVICE, expects NO HelpBuffer
         /* CommandStr should now be 'DEVICE' and is checked earlier for Error */
/*         if (CurConfigString->CommandStr[0]!=0) MINSTALL_TrappedError (MINSTMSG_NoConfigCommandExpected); */
/*         strcpy (CurConfigString->CommandStr, "DEVICE"); */
         CurConfigEntry->Flags |= CONFIGSYSACTION_Flags_MatchOnFilename;
         break;
       default:
         // We got something unknown...
         MINSTALL_ErrorMsgID = MINSTMSG_BadCommand; break;
       }

      switch (CommandID1) {
       case 0xC6D1E64A: // RUN
       case 0xF9D77108: // DEVICE
       case 0x2110156E: // BASEDEV
         break;
       case 0x70B36756: // SET
         if (CommandID2==0x8262959) // LIBPATH -> strip "SET"
            strcpy (CurConfigString->CommandStr, CommandSpacePtr+1);
         break;
       default:
         MINSTALL_ErrorMsgID = MINSTMSG_BadConfigCommand; break;
       }
      if (MINSTALL_ErrorMsgID) break;

      // If no error found, add this entry...
      ValueSpacePtr = CurConfigString->ValueStr;
      while (*ValueSpacePtr!=0) {
         if (*ValueSpacePtr==0x20) // If space found
            break;
         ValueSpacePtr++;
       }
      switch (CommandID1) {
       case 0xC6D1E64A: // RUN
       case 0xF9D77108: // DEVICE
       case 0x2110156E: // BASEDEV
         if (ActionID==0xF9D77108) { // DEVICE
            // Match only on filename...
            TempPtr = ValueSpacePtr;
            while (TempPtr>CurConfigString->ValueStr) {
               if (*TempPtr==0x5C) {
                  TempPtr++;
                  break;
                }
               TempPtr--;          // Search backwards for '\'
             }
            strncpy (CurConfigString->MatchStr, TempPtr, ValueSpacePtr-TempPtr);
          } else {
            // full qualified filename is match string...
            strncpy (CurConfigString->MatchStr, CurConfigString->ValueStr, ValueSpacePtr-CurConfigString->ValueStr);
          }
       }
      MINSTLOG_ToFile ("Command \"%s\", Match-Onto \"%s\" (Flags %X)\n", CurConfigString->CommandStr, CurConfigString->MatchStr, CurConfigEntry->Flags);
      MINSTLOG_ToFile (" Add/Merge in \"%s\"\n", CurConfigString->ValueStr);
      CurConfigString->MatchInLineStr[0] = 0;
      CurConfigEntry->CommandStrPtr     = CurConfigString->CommandStr;
      CurConfigEntry->MatchStrPtr       = CurConfigString->MatchStr;
      CurConfigEntry->MatchInLineStrPtr = CurConfigString->MatchInLineStr;
      CurConfigEntry->ValueStrPtr       = CurConfigString->ValueStr;
      ConfigEntryCount++; CurConfigEntry++; CurConfigString++;

      CurPos = LineEndPos+1; CurLineNo++;
    }

   // Put that Array&Count into ConfigChange-Queue...
   ScriptFilePtr->GroupPtr->ConfigChangeArray = ConfigEntryArrayPtr;
   ScriptFilePtr->GroupPtr->ConfigStringArray = ConfigStringArrayPtr;
   ScriptFilePtr->GroupPtr->ConfigChangeCount = ConfigEntryCount;

   // Remove that file from memory...
   FILE_UnLoadFileControl(&CHANGESCR);

   // We didn't find anything?
   if (ConfigEntryCount==0)
      MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile;

   if (MINSTALL_ErrorMsgID) {               // If Error-found during parsing...
      MSG_SetInsertFileLocation (1, CHANGESCR.Name, CurLineNo);
      MINSTALL_TrappedError (MINSTALL_ErrorMsgID);
      return FALSE;
    }
   return TRUE;
 }

// ****************************************************************************

//
ULONG MINSTALL_GetNextINIFuncID (PCHAR *CurPosPtr, PCHAR EndPos, PULONG CurLineNoPtr) {
   PCHAR  CurPos     = *CurPosPtr;
   PCHAR  StartPos   = 0;
   ULONG  FunctionID = 0;
   CHAR   CurChar;

   if (!(CurChar = STRING_GetValidChar(&CurPos, EndPos, CurLineNoPtr))) {
      *CurPosPtr = CurPos; return 0;        // No more valid chars...
    }

   StartPos = CurPos;
   while (CurPos<EndPos) {
      CurChar = *CurPos; *CurPos = toupper(CurChar);
      if ((CurChar==0x0D) || (CurChar==' ') || (CurChar=='[') || (CurChar=='=')) {
         FunctionID = CRC32_GetFromString(StartPos, CurPos-StartPos);
         *CurPosPtr = CurPos;
         // Now check that we got "=" and "(" afterwards...
         if (STRING_GetValidChar (CurPosPtr, EndPos, CurLineNoPtr)!='=') {
            MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; return 0;
          }
         *CurPosPtr += 1;
         if (STRING_GetValidChar (CurPosPtr, EndPos, CurLineNoPtr)!='(') {
            MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; return 0;
          }
         *CurPosPtr += 1;
         // CHECK Function for validity...
         if (ICF_CheckFuncList) {
            // Lookup Function-Name in FuncList, issue error if not found
            ICF_CurFuncEntry = ICF_CheckFuncList;
            while (FunctionID!=ICF_CurFuncEntry->ID) {
               if (!ICF_CurFuncEntry->ID) {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICUnknownFunction; return 0;
                }
               ICF_CurFuncEntry++;
             }
            ICF_CheckParmList = (PMINSTINI_DEFENTRY)ICF_CurFuncEntry->ParmListPtr;
            ICF_FilledParms   = 0;          // No parms currently filled out...
          }
         return FunctionID;
       }
      CurPos++;
    }
   *CurPosPtr = CurPos;                     // Experienced End-Of-Buffer
   return 0;
 }

ULONG MINSTALL_GetNextINIParmID (PCHAR *CurPosPtr, PCHAR EndPos, PULONG CurLineNoPtr, PLONG ArrayCountPtr) {
   PCHAR  CurPos   = *CurPosPtr;
   PCHAR  StartPos = 0;
   ULONG  ParamID  = 0;
   CHAR   CurChar;
   ULONG  ParamBit = 0;

   if (!(CurChar = STRING_GetValidChar(&CurPos, EndPos, CurLineNoPtr))) {
      *CurPosPtr = CurPos;                  // No more valid chars...
      MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; return 0;
    }

   if (CurChar==')') {
      *CurPosPtr = CurPos+1;
      STRING_GetValidChar (CurPosPtr, EndPos, CurLineNoPtr);
      return 0;                             // Signals End-Of-Param-List
    }

   StartPos = CurPos;
   while (CurPos<EndPos) {
      *CurPos = toupper(CurChar);
      if ((CurChar==0x0D) || (CurChar==' ') || (CurChar=='[') || (CurChar=='=')) {
         ParamID = CRC32_GetFromString(StartPos, CurPos-StartPos);
         *CurPosPtr = CurPos;
         if (!(CurChar = STRING_GetValidChar(CurPosPtr, EndPos, CurLineNoPtr))) {
            MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; return 0;
          }
         if (CurChar=='[') {
            // We got an array here...
            *CurPosPtr += 1;
            if (!(*CurPosPtr = STRING_GetNumericValue ((PULONG)ArrayCountPtr, *CurPosPtr, EndPos))) {
               MINSTALL_ErrorMsgID = MINSTMSG_ICBadArray; return 0;
             }
            if (**CurPosPtr!=']') {
               MINSTALL_ErrorMsgID = MINSTMSG_ICBadArray; return 0;
             }
            *CurPosPtr += 1;
            if (STRING_GetValidChar(CurPosPtr, EndPos, CurLineNoPtr)!='=') {
               MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; return 0;
             }
            *CurPosPtr += 1;
            if (STRING_GetValidChar(CurPosPtr, EndPos, CurLineNoPtr)!='(') {
               MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; return 0;
             }
          } else if (CurChar!='=') {
            MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; return 0;
          } else {
            *ArrayCountPtr = -1;
          }
         // CHECK Parameter for validity...
         if (ICF_CheckParmList) {
            // Lookup Parameter-Name in ParmList, issue error if not found
            ICF_CurParmEntry = ICF_CheckParmList; ParamBit = 1;
            while (ParamID!=ICF_CurParmEntry->ID) {
               if (!ICF_CurParmEntry->ID) {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICUnknownParameter; return 0;
                }
               ICF_CurParmEntry++; ParamBit <<= 1;
             }
            if (ParamBit & ICF_FilledParms) {
               MSG_SetInsertViaPSZ (2, ICF_CurParmEntry->Name);
               MINSTALL_ErrorMsgID = MINSTMSG_ICDuplicateParameter; return 0;
             }
            ICF_FilledParms |= ParamBit;    // Got that parameter...
            // CHECK Array descriptor...
            if (*ArrayCountPtr==-1) {       // <-- Array got not specified
               if (ICF_CurParmEntry->MaxSize!=0) {
                  MSG_SetInsertViaPSZ (2, ICF_CurParmEntry->Name);
                  MINSTALL_ErrorMsgID = MINSTMSG_ICParameterIsArray; return 0;
                }
             } else {                       // <-- Array got specified
               if (ICF_CurParmEntry->MaxSize==0) {
                  MSG_SetInsertViaPSZ (2, ICF_CurParmEntry->Name);
                  MINSTALL_ErrorMsgID = MINSTMSG_ICParameterNotArray; return 0;
                }
               if (ICF_CurParmEntry->MaxSize<*ArrayCountPtr) {
                  MSG_SetInsertViaPSZ (2, ICF_CurParmEntry->Name);
                  MINSTALL_ErrorMsgID = MINSTMSG_ICArrayTooBig; return 0;
                }
             }
          }
         *CurPosPtr += 1;
         // Now seek to next valid char...
         if (!STRING_GetValidChar(CurPosPtr, EndPos, CurLineNoPtr)) {
            MINSTALL_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; return 0; }
         return ParamID;
       }
      CurPos++; CurChar = *CurPos;
    }
   *CurPosPtr = CurPos;                     // Experienced End-Of-Buffer
   MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure;
   return 0;
 }

// CurLineNo is transferred for Warning-logging
BOOL MINSTALL_CheckForMissingINIParms (ULONG CurLineNo) {
   ULONG  ParamBit = 1;

   if (ICF_CheckParmList) {
      // Check, if all parameters were set...
      ICF_CurParmEntry = ICF_CheckParmList;
      while (ICF_CurParmEntry->ID) {
         if (!(ParamBit & ICF_FilledParms)) {
            MSG_SetInsertViaPSZ (2, ICF_CurParmEntry->Name);
            if (ICF_CurParmEntry->Mandatory==TRUE) {
               MINSTALL_ErrorMsgID = MINSTMSG_ICMissingParameter; return TRUE;
             } else {
               MSG_SetInsertFileLocation (1, CHANGESCR.Name, CurLineNo);
               MINSTALL_TrappedWarning (MINSTMSG_ICMissingParameterWarning);
             }
          }
         ICF_CurParmEntry++; ParamBit <<= 1;
       }
    }
   return FALSE;
 }

BOOL MINSTALL_LoadINIControlFile (PMINSTFILE ScriptFilePtr) {
   ULONG              ConfigEntryCount      = 0;
   PCHAR              CurPos                = 0;
   PCHAR              StartPos              = 0;
   PCHAR              EndPos                = 0;
   PCHAR              LineStartPos          = 0;
   PCHAR              LineEndPos            = 0;
   CHAR               CurChar               = 0;
   ULONG              Temp                  = 0;
   ULONG              FunctionID            = 0;
   ULONG              ParamID               = 0;
   ULONG              CurLineNo             = 1;
   ULONG              RequiredSpace         = 0;
   LONG               ArrayCount            = -1;
   ULONG              CurArray              = 0;
   ULONG              ArrayTotal            = 0;

   PVOID              INIChange1stEntryPtr  = 0;
   ULONG              INIChangeCount        = 0;
   PMINSTINIHEADER    INIChangeEntryPtr     = 0; // <-- Current Entry
   PMINSTINIHEADER    INIChangeLastEntryPtr = 0;

   ULONG              MainFilledParms       = 0; // <-- helper for array processing
   PMINSTINI_DEFENTRY MainCheckParmList     = NULL;

   CHAR               TempBuffer[MINSTMAX_PATHLENGTH]; // Temporary buffer
   ULONG              TempULong;
   ULONG              TmpNo;

   // Get Full-Qualified Script Name
   if (!STRING_CombinePSZ (CHANGESCR.Name, MINSTMAX_PATHLENGTH, ScriptFilePtr->SourcePtr->FQName, ScriptFilePtr->Name))
      return FALSE;
   if (!FILE_LoadFileControl(&CHANGESCR, 131767)) {
      MSG_SetInsertViaPSZ (1, CHANGESCR.Name);
      MINSTALL_TrappedError (MINSTMSG_CouldNotLoad);
      return FALSE;
    }
   FILE_PreProcessControlFile(&CHANGESCR);

   // Switch on Function-Name checking...
   ICF_CheckFuncList = MINSTINI_FuncList;

   // Now we are doing the REAL read-in...
   //  This is MUCH MUCH code, but the damn IBM Linker didnt want to compile my
   //  structure-based stuff, so I needed to do it the hardcoded way :(

   CurLineNo = 1; INIChangeCount = 0;
   CurPos = CHANGESCR.BufferPtr; EndPos = CHANGESCR.BufferEndPtr;
   while (CurPos<EndPos) {
      if (!(FunctionID = MINSTALL_GetNextINIFuncID (&CurPos, EndPos, &CurLineNo)))
         break;                             // End reached

      // We got a function, so allocate a memory-block for it...
      if (!(INIChangeEntryPtr = malloc(ICF_CurFuncEntry->MaxSize))) {
         MINSTALL_TrappedError (MINSTMSG_OutOfMemory); break; }
      if (!INIChangeLastEntryPtr) {
         INIChange1stEntryPtr           = INIChangeEntryPtr;
       } else {
         INIChangeLastEntryPtr->NextPtr = INIChangeEntryPtr;
       }
      INIChangeLastEntryPtr = INIChangeEntryPtr;

      // Reset structure and set INI Header
      memset (INIChangeEntryPtr, 0, ICF_CurFuncEntry->MaxSize);
      INIChangeEntryPtr->ID   = FunctionID;
      INIChangeEntryPtr->Size = ICF_CurFuncEntry->MaxSize;

      // We have now switched the parm-list, process Parameters now...
      switch (FunctionID) {
       case EA_JOINEA_ID:                   // ==================== EA - JoinEA
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // JoinFileName/JoinEAFileName
            switch (ParamID) {
             case 0x0FD42A68: CurPos = MINSTALL_GetMacrodString(((PEA_JOINEA)INIChangeEntryPtr)->JoinFileName, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             case 0xDF5CDE5B: CurPos = MINSTALL_GetMacrodString(((PEA_JOINEA)INIChangeEntryPtr)->JoinEAFileName, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case EA_JOINLONGNAMEEA_ID:           // ============ EA - JoinLongNameEA
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // JoinLongName/JoinLongFileName/JoinEALongFileName
            switch (ParamID) {
             case 0xCACCF1F1: CurPos = MINSTALL_GetMacrodString(((PEA_JOINLONGNAMEEA)INIChangeEntryPtr)->JoinLongName, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             case 0xD5C2CE0C: CurPos = MINSTALL_GetMacrodString(((PEA_JOINLONGNAMEEA)INIChangeEntryPtr)->JoinLongFileName, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             case 0xF01882D8: CurPos = MINSTALL_GetMacrodString(((PEA_JOINLONGNAMEEA)INIChangeEntryPtr)->JoinEALongFileName, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case MCI_MCIINSTALLDRV_ID:           // ============ MCI - MciInstallDrv
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // DrvInstallName/DrvDeviceType/DrvDeviceFlag/DrvVersionNumber/
            //  DrvProductInfo/DrvMCDDriver/DrvVSDDriver/DrvPDDName/DrvMCDTable
            //  DrvVSDTable/DrvShareType/DrvResourceName/DrvClassArray
            switch (ParamID) {
             case 0xCAECA131: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szInstallName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0xF1558F48:
               if (!(CurPos = MINSTALL_GetNumericValue(&TempULong, CurPos, EndPos)))
                  break;
               ((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->usDeviceType = TempULong;
               break;
             case 0xAC7F33FB: CurPos = MINSTALL_GetNumericValue(&((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->ulDeviceFlag, CurPos, EndPos); break;
             case 0xDD6E470A: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szVersionNumber, MCIMAX_VERSIONLENGTH, CurPos, EndPos); break;
             case 0xB377234D: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szProductInfo, MCIMAX_PRODLENGTH, CurPos, EndPos); break;
             case 0xB4BFB2C2: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szMCDDriver, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0xE6915F45: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szVSDDriver, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0xE8AB4FF7: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szPDDName, MCIMAX_PDDNAMELENGTH, CurPos, EndPos); break;
             case 0x2552FF41: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szMCDTable, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0x635D4EF2: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szVSDTable, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0xC5E91968:
               if (!(CurPos = MINSTALL_GetNumericValue(&TempULong, CurPos, EndPos)))
                  break;
               ((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->usShareType = TempULong;
               break;
             case 0x25A89CAF: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->szResourceName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0x7DAFC42F:
               if (!(CurPos = MINSTALL_GetNumericValue(&TempULong, CurPos, EndPos)))
                  break;
               ((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->usResourceUnits = TempULong;
               break;
             case 0x924AB2E1: // DrvClassArray is ARRAY
               // Safe current Main-ParmList and FilledParms...
               MainFilledParms   = ICF_FilledParms; MainCheckParmList = ICF_CheckParmList;
               // Set to Array-specific ParmList...
               ICF_CheckParmList = ICF_CurParmEntry->ParmListPtr;

               ((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->usResourceClasses = ArrayCount;
               ArrayTotal = ArrayCount; CurArray = 0;
               while ((CurArray<ArrayTotal) && (!MINSTALL_ErrorMsgID)) {
                  if (*CurPos!='(') {
                     MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; break; }
                  CurPos++;
                  ICF_FilledParms = 0;      // We dont have any Parms yet
                  while (CurPos<EndPos) {
                     if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
                        break;
                     // DrvClassNumber (ARRAY!)
                     switch (ParamID) {
                      case 0x2FC58D01:
                        if (!(CurPos = MINSTALL_GetNumericValue(&TempULong, CurPos, EndPos)))
                           break;
                        ((PMCI_MCIINSTALLDRV)INIChangeEntryPtr)->ausClassArray[CurArray] = TempULong;
                        break;
                      }
                     if (MINSTALL_ErrorMsgID) break; // If any error occured...
                   }
                  if (MINSTALL_ErrorMsgID) break;
                  // Check, if array element had all parameters...
                  if (MINSTALL_CheckForMissingINIParms(CurLineNo)) break;
                  CurArray++;
                }
               if (MINSTALL_ErrorMsgID) break;
               if (*CurPos!=')') {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; break; }
               CurPos++;
               // Now restore Main-ParmList and FilledParms...
               ICF_FilledParms   = MainFilledParms; ICF_CheckParmList = MainCheckParmList;
             }
          }
         break;
       case MCI_MCIINSTALLCONN_ID:          // =========== MCI - MciInstallConn
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // ConnInstallName/ConnArray
            switch (ParamID) {
             case 0x7C9375DB: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLCONN)INIChangeEntryPtr)->szInstallName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0x40B7450F: // ConnArray is ARRAY
               // Safe current Main-ParmList and FilledParms...
               MainFilledParms   = ICF_FilledParms; MainCheckParmList = ICF_CheckParmList;
               // Set to Array-specific ParmList...
               ICF_CheckParmList = ICF_CurParmEntry->ParmListPtr;

               ((PMCI_MCIINSTALLCONN)INIChangeEntryPtr)->usNumConnectors = ArrayCount;
               ArrayTotal = ArrayCount; CurArray = 0;
               while ((CurArray<ArrayTotal) && (!MINSTALL_ErrorMsgID)) {
                  if (*CurPos!='(') {
                     MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; break; }
                  CurPos++;
                  ICF_FilledParms = 0;      // We dont have any Parms yet
                  while (CurPos<EndPos) {
                     if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
                        break;
                     // ConnType/ConnInstallTo/ConnIndexTo (ARRAY!)
                     switch (ParamID) {
                      case 0x39EDF742:
                        if (!(CurPos = MINSTALL_GetNumericValue(&TempULong, CurPos, EndPos)))
                           break;
                        ((PMCI_MCIINSTALLCONN)INIChangeEntryPtr)->ConnectorList[CurArray].usConnectType = TempULong;
                        break;
                      case 0x77E86C5D: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLCONN)INIChangeEntryPtr)->ConnectorList[CurArray].szToInstallName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
                      case 0xDEB82C0C:
                        if (!(CurPos = MINSTALL_GetNumericValue(&TempULong, CurPos, EndPos)))
                           break;
                        ((PMCI_MCIINSTALLCONN)INIChangeEntryPtr)->ConnectorList[CurArray].usToConnectIndex = TempULong;
                        break;
                      }
                     if (MINSTALL_ErrorMsgID) break; // If any error occured...
                   }
                  if (MINSTALL_ErrorMsgID) break;
                  // Check, if array element had all parameters...
                  if (MINSTALL_CheckForMissingINIParms(CurLineNo)) break;
                  CurArray++;
                }
               if (MINSTALL_ErrorMsgID) break;
               if (*CurPos!=')') {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; break; }
               CurPos++;
               // Now restore Main-ParmList and FilledParms...
               ICF_FilledParms   = MainFilledParms; ICF_CheckParmList = MainCheckParmList;
             }
          }
         break;
       case MCI_MCIINSTALLPARM_ID:          // =========== MCI - MciInstallParm
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // ParmInstallName/ParmString
            switch (ParamID) {
             case 0xBB3418BC: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLPARM)INIChangeEntryPtr)->szInstallName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0x7E2CCDF4: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLPARM)INIChangeEntryPtr)->szDevParams, MCIMAX_DEVPARAMSLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case MCI_MCIINSTALLALIAS_ID:         // ========== MCI - MciInstallAlias
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // AliasInstallName/AliasString
            switch (ParamID) {
             case 0xDE60EB0E: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLALIAS)INIChangeEntryPtr)->szInstallName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0xA278C064: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLALIAS)INIChangeEntryPtr)->szAliasName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case MCI_MCIINSTALLEXT_ID:           // ============ MCI - MciInstallExt
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // ExtInstallName/ExtArray
            switch (ParamID) {
             case 0x8187B8FE: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLEXT)INIChangeEntryPtr)->szInstallName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0x130025CB: // ExtArray is ARRAY
               // Safe current Main-ParmList and FilledParms...
               MainFilledParms   = ICF_FilledParms; MainCheckParmList = ICF_CheckParmList;
               // Set to Array-specific ParmList...
               ICF_CheckParmList = ICF_CurParmEntry->ParmListPtr;

               ((PMCI_MCIINSTALLEXT)INIChangeEntryPtr)->usNumExtensions = ArrayCount;
               ArrayTotal = ArrayCount; CurArray = 0;
               while ((CurArray<ArrayTotal) && (!MINSTALL_ErrorMsgID)) {
                  if (*CurPos!='(') {
                     MINSTALL_TrappedError (MINSTMSG_ICBadStructure); break; }
                  CurPos++;
                  ICF_FilledParms = 0;      // We dont have any Parms yet
                  while (CurPos<EndPos) {
                     if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
                        break;
                     // ExtString (ARRAY!)
                     switch (ParamID) {
                      case 0xFCD1A3BD: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLEXT)INIChangeEntryPtr)->szExtension[CurArray], MCIMAX_EXTENSIONNAMELENGTH, CurPos, EndPos); break;
                      }
                     if (MINSTALL_ErrorMsgID) break; // If any error occured...
                   }
                  if (MINSTALL_ErrorMsgID) break;
                  // Check, if array element had all parameters...
                  if (MINSTALL_CheckForMissingINIParms(CurLineNo)) break;
                  CurArray++;
                }
               if (MINSTALL_ErrorMsgID) break;
               if (*CurPos!=')') {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICBadStructure; break; }
               CurPos++;
               // Now restore Main-ParmList and FilledParms...
               ICF_FilledParms   = MainFilledParms; ICF_CheckParmList = MainCheckParmList;
             }
          }
         break;
       case MCI_MCIINSTALLTYPES_ID:         // ========== MCI - MciInstallTypes
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // TypesInstallName/TypesTypeList
            switch (ParamID) {
             case 0x3DA680F8: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLTYPES)INIChangeEntryPtr)->szInstallName, MCIMAX_DEVICENAMELENGTH, CurPos, EndPos); break;
             case 0x84C253AD: CurPos = MINSTALL_GetMacrodString(((PMCI_MCIINSTALLTYPES)INIChangeEntryPtr)->szTypes, MCIMAX_TYPELISTLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case MMIO_MMIOINSTALL_ID:            // ============= MMIO - mmioInstall
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // FourCC/DllName/DllEntryPoint/Flags/ExtendLen/MediaType/IOProcType/DefExt
            switch (ParamID) {
             case 0x874AF32B: // mmioFourCC must be 4 chars wide
               if (!(CurPos = MINSTALL_GetMacrodString(TempBuffer, MINSTMAX_PATHLENGTH, CurPos, EndPos)))
                  break;
               if (strlen(TempBuffer)!=4) {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICBadFourCC; break; }
               ((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->fccIOProc = *((PULONG)&TempBuffer);
               break;
             case 0x83BC6E92: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->szDLLName, MMIOMAX_DLLLENGTH, CurPos, EndPos); break;
             case 0x383FA8EC: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->szProcName, MMIOMAX_PROCNAMELENGTH, CurPos, EndPos); break;
             case 0x6EA2DBC6: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->dwFlags, CurPos, EndPos); break;
             case 0x953B087F: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->dwExtendLen, CurPos, EndPos); break;
             case 0xB1824C61: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->dwMediaType, CurPos, EndPos); break;
             case 0xCD997BB5: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->dwIOProcType, CurPos, EndPos); break;
             case 0xF1124958: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOINSTALL)INIChangeEntryPtr)->szDefExt, MMIOMAX_EXTENSIONNAMELENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case MMIO_MMIOCODECDELETE_ID:        // ========= MMIO - mmioCodecDelete
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // Length/FourCC/DllName/DllEntryPoint/CompTypeFcc/CompSubType/
            //  MediaType/Flags/CapsFlags/HWName/MaxSrcBuf/SyncMethod/Reserved1
            //  XAlign/YAlign/SpecInfo
            switch (ParamID) {
             case 0xB8420316: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulStructLen, CurPos, EndPos); break;
             case 0x5CCDD4CA: // mmioFourCC must be 4 chars wide
               if (!(CurPos = MINSTALL_GetMacrodString(TempBuffer, MINSTMAX_PATHLENGTH, CurPos, EndPos)))
                  break;
               if (strlen(TempBuffer)!=4) {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICBadFourCC; break; }
               ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fcc = *((PULONG)&TempBuffer);
               break;
             case 0x546A3B5B: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szDLLName, MMIOMAX_DLLLENGTH, CurPos, EndPos); break;
             case 0x75D262DB: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szProcName, MMIOMAX_PROCNAMELENGTH, CurPos, EndPos); break;
             case 0xB3D96C53: // mmioCompressType (FCC) must be 4 chars wide
               if (!(CurPos = MINSTALL_GetMacrodString(TempBuffer, MINSTMAX_PATHLENGTH, CurPos, EndPos)))
                  break;
               if (strlen(TempBuffer)!=4) {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICBadFourCC; break; }
               ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressType = *((PULONG)&TempBuffer);
               break;
             case 0x6CCF6654: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressSubType, CurPos, EndPos); break;
             case 0xB0BB52CB: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMediaType, CurPos, EndPos); break;
             case 0x524CECEC: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulFlags, CurPos, EndPos); break;
             case 0x09007EEC: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCapsFlags, CurPos, EndPos); break;
             case 0xD8CF2CF5: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szHWID, MMIOMAX_HWNAMELENGTH, CurPos, EndPos); break;
             case 0x2841E06C: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMaxSrcBufLen, CurPos, EndPos); break;
             case 0xD9643500: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulSyncMethod, CurPos, EndPos); break;
             case 0xF7CAA0ED: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fccPreferredFormat, CurPos, EndPos); break;
             case 0xFD9EE47D: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulXalignment, CurPos, EndPos); break;
             case 0x36C237D8: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulYalignment, CurPos, EndPos); break;
             case 0x2DCCCF4F: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szSpecInfo, MMIOMAX_SPECINFOLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case MMIO_MMIOCODEC1INSTALL_ID:      // ======= MMIO - mmioCodec1Install
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // Length/FourCC/DllName/DllEntryPoint/CompTypeFcc/CompSubType/
            //  MediaType/Flags/CapsFlags/HWName/MaxSrcBuf/SyncMethod/Reserved1
            //  XAlign/YAlign/SpecInfo
            switch (ParamID) {
             case 0xD1C05A78: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulStructLen, CurPos, EndPos); break;
             case 0x354F8DA4: // mmioFourCC must be 4 chars wide
               if (!(CurPos = MINSTALL_GetMacrodString(TempBuffer, MINSTMAX_PATHLENGTH, CurPos, EndPos)))
                  break;
               if (strlen(TempBuffer)!=4) {
                  MINSTALL_ErrorMsgID = MINSTMSG_ICBadFourCC; break; }
               ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fcc = *((PULONG)&TempBuffer);
               break;
             case 0xFE09F55D: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szDLLName, MMIOMAX_DLLLENGTH, CurPos, EndPos); break;
             case 0x08D25727: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szProcName, MMIOMAX_PROCNAMELENGTH, CurPos, EndPos); break;
             // Codec1Install -> Change is here: ULONG instead of FCC
             case 0x487146E0: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressType, CurPos, EndPos); break;
             case 0xAA46F050: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressSubType, CurPos, EndPos); break;
             case 0x9A3DB099: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMediaType, CurPos, EndPos); break;
             case 0xD6D77C7F: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulFlags, CurPos, EndPos); break;
             case 0x23869CBE: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCapsFlags, CurPos, EndPos); break;
             case 0xB14D759B: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szHWID, MMIOMAX_HWNAMELENGTH, CurPos, EndPos); break;
             case 0x02C7023E: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMaxSrcBufLen, CurPos, EndPos); break;
             case 0x5C2B833A: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulSyncMethod, CurPos, EndPos); break;
             case 0xDD4C42BF: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fccPreferredFormat, CurPos, EndPos); break;
             case 0x941CBD13: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulXalignment, CurPos, EndPos); break;
             case 0x5F406EB6: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulYalignment, CurPos, EndPos); break;
             case 0xC40509B4:  CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szSpecInfo, MMIOMAX_SPECINFOLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case MMIO_MMIOCODEC2INSTALL_ID:      // ======= MMIO - mmioCodec2Install
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // Length/FourCC/DllName/DllEntryPoint/CompTypeFcc/CompSubType/
            //  MediaType/Flags/CapsFlags/HWName/MaxSrcBuf/SyncMethod/Reserved1
            //  XAlign/YAlign/SpecInfo
            switch (ParamID) {
             case 0xE02840E5: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulStructLen, CurPos, EndPos); break;
             case 0x04A79739: // mmioFourCC must be 4 chars wide
               if (!(CurPos = MINSTALL_GetMacrodString(TempBuffer, MINSTMAX_PATHLENGTH, CurPos, EndPos)))
                  break;
               if (strlen(TempBuffer)!=4) {
                  MINSTALL_TrappedError (MINSTMSG_ICBadFourCC); break; }
               ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fcc = *((PULONG)&TempBuffer);
               break;
             case 0x7086F2BE: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szDLLName, MMIOMAX_DLLLENGTH, CurPos, EndPos); break;
             case 0x74B372FC: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szProcName, MMIOMAX_PROCNAMELENGTH, CurPos, EndPos); break;
             // Codec2Install -> Change is here: FCC instead of ULONG
             case 0x02CE28A7: // mmioCompressType (FCC) must be 4 chars wide
               if (!(CurPos = MINSTALL_GetMacrodString(TempBuffer, MINSTMAX_PATHLENGTH, CurPos, EndPos)))
                  break;
               if (strlen(TempBuffer)!=4) {
                  MINSTALL_TrappedError (MINSTMSG_ICBadFourCC); break; }
               ((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressType = *((PULONG)&TempBuffer);
               break;
             case 0xDDD822A0: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCompressSubType, CurPos, EndPos); break;
             case 0x710A0B9A: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMediaType, CurPos, EndPos); break;
             case 0x50430ED1: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulFlags, CurPos, EndPos); break;
             case 0xC8B127BD: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulCapsFlags, CurPos, EndPos); break;
             case 0x80A56F06: CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szHWID, MMIOMAX_HWNAMELENGTH, CurPos, EndPos); break;
             case 0xE9F0B93D: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulMaxSrcBufLen, CurPos, EndPos); break;
             case 0xC5C9E53B: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulSyncMethod, CurPos, EndPos); break;
             case 0x367BF9BC: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->fccPreferredFormat, CurPos, EndPos); break;
             case 0xA5F4A78E: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulXalignment, CurPos, EndPos); break;
             case 0x6EA8742B: CurPos = MINSTALL_GetNumericValue(&((PMMIO_MMIOCODEC)INIChangeEntryPtr)->ulYalignment, CurPos, EndPos); break;
             case 0xFD883571:  CurPos = MINSTALL_GetMacrodString(((PMMIO_MMIOCODEC)INIChangeEntryPtr)->szSpecInfo, MMIOMAX_SPECINFOLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case PRF_PROFILEDATA_ID:             // ============== PRF - ProfileData
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // Ini/AppName/KeyName/Dll/Id
            switch (ParamID) {
             case 0xE1F0F4E2: CurPos = MINSTALL_GetMacrodString(((PPRF_PROFILEDATA)INIChangeEntryPtr)->Ini, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             case 0x027F2A21: CurPos = MINSTALL_GetMacrodString(((PPRF_PROFILEDATA)INIChangeEntryPtr)->AppName, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0xF8048436: CurPos = MINSTALL_GetMacrodString(((PPRF_PROFILEDATA)INIChangeEntryPtr)->KeyName, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0xAB74F1BC: CurPos = MINSTALL_GetMacrodString(((PPRF_PROFILEDATA)INIChangeEntryPtr)->Dll, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             case 0x11D3633A: CurPos = MINSTALL_GetNumericValue(&((PPRF_PROFILEDATA)INIChangeEntryPtr)->Id, CurPos, EndPos); break;
             }
          }
         break;
       case PRF_PROFILESTRING_ID:           // ============ PRF - ProfileString
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // Inis/AppNames/KeyNames/Datas
            switch (ParamID) {
             case 0x6E859C63: CurPos = MINSTALL_GetMacrodString(((PPRF_PROFILESTRING)INIChangeEntryPtr)->Inis, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             case 0x6C0B80B7: CurPos = MINSTALL_GetMacrodString(((PPRF_PROFILESTRING)INIChangeEntryPtr)->AppNames, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0xEF227EDE: CurPos = MINSTALL_GetMacrodString(((PPRF_PROFILESTRING)INIChangeEntryPtr)->KeyNames, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0x384CCEAA: CurPos = MINSTALL_GetMacrodString(((PPRF_PROFILESTRING)INIChangeEntryPtr)->Datas, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case SPI_SPIINSTALL_ID:              // =============== SPI - SpiInstall
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // SpiDllName
            switch (ParamID) {
             case 0xE906AF16: CurPos = MINSTALL_GetMacrodString(((PSPI_SPIINSTALL)INIChangeEntryPtr)->SpiDllName, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case WPS_CREATEOBJECT_ID:            // ================= WPS - WPObject
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // WPClassName/WPTitle/WPSetupString/WPLocation/WPFlags
            switch (ParamID) {
             case 0x441CEA42: CurPos = MINSTALL_GetMacrodString(((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPClassName, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0x6DD04D76: CurPos = MINSTALL_GetMacrodString(((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPTitle, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0x10108A85: CurPos = MINSTALL_GetMacrodString(((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPSetupString, MINSTMAX_PATHLENGTH, CurPos, EndPos); break;
             case 0x379F7EDF: CurPos = MINSTALL_GetMacrodString(((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPLocation, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0x4DE374A7: CurPos = MINSTALL_GetNumericValue(&((PWPS_CREATEOBJECT)INIChangeEntryPtr)->WPFlags, CurPos, EndPos); break;
             }
          }
         break;
       case WPS_DESTROYOBJECT_ID:     // ================ WPS - WPDestroyObject
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // WPDestroyObjectID
            switch (ParamID) {
             case 0x5BE681E8: CurPos = MINSTALL_GetMacrodString(((PWPS_DESTROYOBJECT)INIChangeEntryPtr)->WPDestroyObjectID, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       case WPS_WPCLASS_ID:                  // ================= WPS - WPClass
         while ((CurPos<EndPos) && (!MINSTALL_ErrorMsgID)) {
            if (!(ParamID = MINSTALL_GetNextINIParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount)))
               break;
            // WPClassNameNew/WPDllName/WPReplaceClass
            switch (ParamID) {
             case 0x3ECA180C: CurPos = MINSTALL_GetMacrodString(((PWPS_WPCLASS)INIChangeEntryPtr)->WPClassNameNew, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0xB933A94C: CurPos = MINSTALL_GetMacrodString(((PWPS_WPCLASS)INIChangeEntryPtr)->WPDllName, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             case 0x7F6632F4: CurPos = MINSTALL_GetMacrodString(((PWPS_WPCLASS)INIChangeEntryPtr)->WPReplaceClass, MINSTMAX_STRLENGTH, CurPos, EndPos); break;
             }
          }
         break;
       default:
         MINSTLOG_ToFile ("Inconsistent MINSTINI_CheckFuncTable\n");
         return FALSE;                      // Internal error
       }
      if (MINSTALL_ErrorMsgID) break;       // If any error occured, get out...

      // Last but not least check if any parameter(s) are missing...
      if (MINSTALL_CheckForMissingINIParms(CurLineNo)) break;

//      // Now update current INIChangeArray-Pointers to point behind this entry
//      INIChangeCustomPtr = (PVOID)(((ULONG)INIChangeEntryPtr)+INIChangeHeaderPtr->Size);
//      INIChangeHeaderPtr = (PVOID)(((ULONG)INIChangeHeaderPtr)+INIChangeHeaderPtr->Size);
      INIChangeCount++;
    }

   // Put that Array&Count into INIChange-Queue...
   ScriptFilePtr->GroupPtr->INIChange1stEntry = INIChange1stEntryPtr;
   ScriptFilePtr->GroupPtr->INIChangeCount    = INIChangeCount;

   // Remove that file from memory...
   FILE_UnLoadFileControl(&CHANGESCR);

//  For debugging the memory structure...
//   FileHandle = fopen ("minstall.out", "wb");
//   fwrite(INIChangeArrayPtr, 1, RequiredSpace, FileHandle);
//   fclose (FileHandle);

   if (MINSTALL_ErrorMsgID) {               // If Error-found during parsing...
      MSG_SetInsertFileLocation (1, CHANGESCR.Name, CurLineNo);
      MINSTALL_TrappedError (MINSTALL_ErrorMsgID);
      return FALSE;
    }
   return TRUE;
 }

// ****************************************************************************

BOOL MINSTALL_LoadCustomControl (VOID) {
   PMINSTFILE CurFilePtr      = FCF_FileArrayPtr;
   USHORT     CurNo           = 0;

   // Remember this action for cleanup...
   MINSTALL_Done |= MINSTDONE_LOADCUSTOMCTRLSCR;

   while (CurNo<FCF_FileCount) {
      if (CurFilePtr->Flags & MINSTFILE_Flags_CFGCF) {
         // Is an CONFIG-Control-File, so process it...
         MINSTLOG_ToFile ("LoadConfigControlFile \"%s\"...\n", CurFilePtr->Name);
         if (!MINSTALL_LoadConfigControlFile (CurFilePtr))
            return FALSE;
         MINSTLOG_ToFile ("LoadConfigControlFile DONE\n");
       } else if (CurFilePtr->Flags & MINSTFILE_Flags_INICF) {
         // Is an INI-Control-File, so process it...
         MINSTLOG_ToFile ("LoadINIControlFile \"%s\"...\n", CurFilePtr->Name);
         if (!(MINSTALL_LoadINIControlFile (CurFilePtr)))
            return FALSE;
         MINSTLOG_ToFile ("LoadINIControlFile DONE\n");
       }
      CurFilePtr++; CurNo++;
    }
   return TRUE;
 }

VOID MINSTALL_CleanUpCustomControl (VOID) {
   BOOL            CurNo       = 0;
   PMINSTGRP       CurGroupPtr = MCF_GroupArrayPtr;
   PMINSTINIHEADER CurINIChangePtr  = 0;
   PMINSTINIHEADER NextINIChangePtr = 0;

   // Release CustomControl-memory-blocks...
   while (CurNo<MCF_GroupCount) {
      // Release Config-Control-Script blocks...
      if (CurGroupPtr->ConfigChangeArray) {
         free (CurGroupPtr->ConfigChangeArray);
         CurGroupPtr->ConfigChangeArray = 0; }
      if (CurGroupPtr->ConfigStringArray) {
         free (CurGroupPtr->ConfigStringArray);
         CurGroupPtr->ConfigChangeArray = 0; }
      CurGroupPtr->ConfigChangeCount    = 0;

      // Release INI-Control-Script blocks...
      CurINIChangePtr = CurGroupPtr->INIChange1stEntry;
      while (CurINIChangePtr) {
         NextINIChangePtr = CurINIChangePtr->NextPtr;
         free (CurINIChangePtr);
         CurINIChangePtr  = NextINIChangePtr;
       }
      CurGroupPtr->INIChangeCount       = 0;
      CurGroupPtr++; CurNo++;
    }

   // Remove this action from cleanup...
   MINSTALL_Done &= !MINSTDONE_LOADCUSTOMCTRLSCR;
 }
