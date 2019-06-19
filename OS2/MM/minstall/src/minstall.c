//
// MINSTALL/CLI (c) Copyright 2002-2005 Martin Kiewitz
//
// This file is part of MINSTALL/CLI for OS/2 / eComStation
//
// MINSTALL/CLI is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
// MINSTALL/CLI is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//  along with MINSTALL/CLI.  If not, see <http://www.gnu.org/licenses/>.
//

#define INCL_BASE
#define INCL_DOSMODULEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINWINDOWMGR
#include <os2.h>

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define INCLUDE_STD_MAIN
#include <global.h>
#include <cid.h>
#include <crcs.h>
#include <file.h>
#include <globstr.h>
#include <msg.h>
#include <mmi_public.h>
#include <mmi_msg.h>

CHAR Inkey (void) {
   KBDKEYINFO UserKey;

   KbdCharIn(&UserKey,0,0);
   return UserKey.chChar;
 }

// CID-Mode is set, when we are executed in CID-Mode (/R switch)
//  (CIDCompatibility is set in that case, if response file is old format)
BOOL  MINSTCLI_CIDMode                               = FALSE;
ULONG MINSTCLI_ErrorMsgID                            = 0;
// CID-Create is set, when being in create-response-file-mode (/C switch)
//  Compatibility is set for original way of creating response files
BOOL  MINSTCLI_CIDCreate                             = FALSE;
BOOL  MINSTCLI_CIDCompatibility                      = FALSE;
// Logfile is set, when log-file is specified /L switch)
CHAR  MINSTCLI_LogFileName[MINSTMAX_PATHLENGTH]      = {0};
// Response file for either CIDmode or CIDrecord
CHAR  MINSTCLI_ResponseFileName[MINSTMAX_PATHLENGTH] = {0};
ULONG MINSTCLI_ResponseGroups                        = 0;
// Source-Path containing CONTROL.scr (only path!)
CHAR  MINSTCLI_SourcePath[MINSTMAX_PATHLENGTH]       = {0};
CHAR  MINSTCLI_TargetPath[MINSTMAX_PATHLENGTH]       = {0};
ULONG MINSTCLI_TargetBootDrive                       = 0;

FILECONTROL    RESPONSEFILE;
PCHAR          MINSTCLI_ResponseFilePos;
ULONG          MINSTCLI_ResponseFileLine;

PMINSTPUBGROUP MINSTCLI_PubGroupPtr                  = 0;
ULONG          MINSTCLI_PubGroupCount                = 0;

// /M -> run from MMTEMP??
// /L:file -> logfile
// /R:file -> response file mode (CID)
// /F:file -> points to control.scr file (CLI mode)
// /C:file -> create response file (for CID)

CHAR MINSTCLI_GetSelectionChar (ULONG SelectionNo) {
   if (SelectionNo>35) return 0;

   if (SelectionNo<9) {
      SelectionNo += '1';
    } else if (SelectionNo==9) {
      SelectionNo = '0';
    } else {
      SelectionNo += 55;
    }
   return SelectionNo;
 }

LONG MINSTCLI_GetSelectionFromUser (ULONG MaxChoice) {
   CHAR CurChar;
   LONG CurSelection;

   while (1) {
      CurChar = toupper(Inkey());
      if (CurChar==0x1B) {                  // User pressed ESC
         return -1;
       } else if (CurChar==0x0D) {          // User pressed ENTER
         return -2;
       } else {
         CurSelection = -1;
         if ((CurChar>='1') && (CurChar<='9')) {
            CurSelection = CurChar-'1';
          } else if (CurChar=='0') {
            CurSelection = 9;
          } else if ((CurChar>='A') && (CurChar<='Z')) {
            CurSelection = CurChar-55;
          }
         if (CurSelection!=-1) {
            if (CurSelection<MaxChoice) return CurSelection;
          }
       }
    }
 }

BOOL MINSTCLI_UserGroupSelection (void) {
   PSZ            PackageName   = 0;
   PMINSTPUBGROUP CurGroupPtr   = 0;
   ULONG          CurGroupNo    = 0;
   ULONG          CurPromptNo   = 0;
   PSZ            CurOptionPtr  = 0;
   ULONG          CurOptionNo   = 0;
   LONG           CurChoice     = 0;
   LONG           MaxChoice     = 0;
   ULONG          CurCardNo     = 0;
   ULONG          CardCount     = 0;
   CHAR           TempValueStr[2] = {0};

   PackageName = MINSTALL_GetPackageTitlePtr();

   if (MINSTCLI_PubGroupCount) {
      // First let user choose selection of public groups...
      while (1) {
         printf ("\x1B[2J");
         MSG_Print (MINSTMSG_CLITrailer);
         printf ("\n");
         MSG_SetInsertViaPSZ (1, PackageName);
         MSG_Print (MINSTMSG_CLIActivePackageIs);
         printf ("\n");

         // Print out all groups...
         CurGroupNo = 0; CurGroupPtr = MINSTCLI_PubGroupPtr;
         while (CurGroupNo<MINSTCLI_PubGroupCount) {
            CurChoice = MINSTCLI_GetSelectionChar(CurGroupNo);
            printf ("%c - '%s'", CurChoice, CurGroupPtr->Name);
            if (CurGroupPtr->Selected) {
               MSG_Print (MINSTMSG_CLIIsSelected);
             }
            printf ("\n");
            CurGroupNo++; CurGroupPtr++;
          }
         printf ("\n");
         MSG_Print (MINSTMSG_CLIChooseGroup);

         CurChoice = MINSTCLI_GetSelectionFromUser(MINSTCLI_PubGroupCount);
         if (CurChoice==-1) {               // User pressed ESC
            return FALSE;
          } else if (CurChoice==-2) {       // User pressed ENTER
            break;
          } else {                          // Group-Selection
            // So display group and ask, if user wants to select or deselect it
            CurGroupNo = 0; CurGroupPtr = MINSTCLI_PubGroupPtr;
            while (CurGroupNo<CurChoice) {
               CurGroupNo++; CurGroupPtr++;
             }
            if (!CurGroupPtr->SelectionForced) {
               if (CurGroupPtr->Selected) {
                  MINSTALL_DeSelectGroup (CurGroupPtr->ID);
                } else {
                  MINSTALL_SelectGroup (CurGroupPtr->ID);
                }
             } else {
               printf ("\n");
               MSG_Print (MINSTMSG_CLISelectionForced);
               Inkey();
             }
          }
       }

      // Now let him choose user options per selected group...
      CurGroupNo = 0; CurGroupPtr = MINSTCLI_PubGroupPtr;
      while (CurGroupNo<MINSTCLI_PubGroupCount) {
         if (CurGroupPtr->Selected) {
            if (CurGroupPtr->MaxCardCount>0) {
               printf ("\x1B[2J");
               MSG_Print (MINSTMSG_CLITrailer);
               printf ("\n");
               MSG_SetInsertViaPSZ (1, PackageName);
               MSG_Print (MINSTMSG_CLIActivePackageIs);
               MSG_SetInsertViaPSZ (1, CurGroupPtr->Name);
               MSG_Print (MINSTMSG_CLIActiveGroupIs);
               printf ("\n");

               ultoa (CurGroupPtr->MaxCardCount, TempValueStr, 10);
               MSG_SetInsertViaPSZ (1, TempValueStr);
               MSG_Print (MINSTMSG_CLICardSelect);
               while (1) {
                  CurChoice = Inkey();
                  if (CurChoice==0x1B) {
                     return FALSE;
                   } else if (CurChoice=='0') {
                     CardCount = 0; break;
                   } else if ((CurChoice>='1') && (CurChoice<='9')) {
                     CardCount = CurChoice-'0';
                     if (CardCount<=CurGroupPtr->MaxCardCount)
                        break;
                   }
                }
               // User chose CardCount, so set it for MINSTALL.DLL
               MINSTALL_SetCARDINFOCardCountForGroup (CurGroupPtr->ID, CardCount);

               CurCardNo = 1;
               while (CurCardNo<=CardCount) {
                  // Give user selection over user prompts per Card
                  if (CurGroupPtr->PromptsCount) {
                     // Set Insert 2 (contains Card-Number)
                     while (1) {
                        // Display Prompt-Selection...
                        printf ("\x1B[2J");
                        MSG_Print (MINSTMSG_CLITrailer);
                        printf ("\n");
                        MSG_SetInsertViaPSZ (1, PackageName);
                        MSG_Print (MINSTMSG_CLIActivePackageIs);
                        MSG_SetInsertViaPSZ (1, CurGroupPtr->Name);
                        MSG_Print (MINSTMSG_CLIActiveGroupIs);
                        ultoa (CurCardNo, TempValueStr, 10);
                        MSG_SetInsertViaPSZ (1, TempValueStr);
                        MSG_Print (MINSTMSG_CLIActiveCardNoIs);
                        printf ("\n");

                        // List all prompt-titles including current selection
                        CurPromptNo = 0;
                        while (CurPromptNo<CurGroupPtr->PromptsCount) {
                           CurChoice    = MINSTCLI_GetSelectionChar(CurPromptNo);
                           CurOptionNo  = MINSTALL_GetCARDINFOChoiceForGroup (CurGroupPtr->ID, CurCardNo, CurPromptNo+1);
                           CurOptionPtr = CurGroupPtr->PromptChoiceStrings[CurPromptNo];
                           while (CurOptionNo>1) {
                              CurOptionPtr = STRING_SkipASCIIZ(CurOptionPtr, NULL);
                              CurOptionNo--;
                            }
                           printf ("%c - %s\x1B[80D\x1B[54C%s\n", CurChoice, CurGroupPtr->PromptTitlePtr[CurPromptNo], CurOptionPtr);
                           CurPromptNo++;
                         }
                        printf ("\n");
                        MSG_Print (MINSTMSG_CLIChoosePrompt);

                        CurChoice = MINSTCLI_GetSelectionFromUser(CurGroupPtr->PromptsCount);
                        if (CurChoice==-1) {         // User pressed ESC
                           return FALSE;
                         } else if (CurChoice==-2) { // User pressed ENTER
                           break;
                         } else {                    // Prompt-Selection
                           CurPromptNo = CurChoice;

                           while (1) {
                              // So show prompt and let user choose
                              printf ("\x1B[2J");
                              MSG_Print (MINSTMSG_CLITrailer);
                              printf ("\n");
                              MSG_SetInsertViaPSZ (1, PackageName);
                              MSG_Print (MINSTMSG_CLIActivePackageIs);
                              MSG_SetInsertViaPSZ (1, CurGroupPtr->Name);
                              MSG_Print (MINSTMSG_CLIActiveGroupIs);
                              ultoa (CurCardNo, TempValueStr, 10);
                              MSG_SetInsertViaPSZ (1, TempValueStr);
                              MSG_Print (MINSTMSG_CLIActiveCardNoIs);
                              MSG_SetInsertViaPSZ (1, CurGroupPtr->PromptTitlePtr[CurPromptNo]);
                              MSG_Print (MINSTMSG_CLIActivePromptIs);
                              printf ("\n");

                              // List all prompt-selections...
                              CurOptionNo = 0; CurOptionPtr = CurGroupPtr->PromptChoiceStrings[CurPromptNo];
                              while (CurOptionNo<CurGroupPtr->PromptChoiceCount[CurPromptNo]) {
                                 CurChoice = MINSTCLI_GetSelectionChar(CurOptionNo);
                                 printf ("%c - '%s'\n", CurChoice, CurOptionPtr);
                                 CurOptionPtr = STRING_SkipASCIIZ(CurOptionPtr, NULL);
                                 CurOptionNo++;
                               }

                              printf ("\n");
                              MSG_Print (MINSTMSG_CLIChooseOption);

                              CurChoice = MINSTCLI_GetSelectionFromUser(CurGroupPtr->PromptChoiceCount[CurPromptNo]);
                              if (CurChoice==-1) {         // User pressed ESC
                                 break;
                               } else {                    // Prompt-Selection
                                 MINSTALL_SetCARDINFOChoiceForGroup (CurGroupPtr->ID, CurCardNo, CurPromptNo+1, CurChoice+1);
                                 break;
                               }
                            }
                         }
                      }
                   }
                  CurCardNo++;
                }
             }
          }
         CurGroupNo++; CurGroupPtr++;
       }
    }

   printf ("\n");
   // Dont ask user for confirmation, if we create a response file only
   if ((MINSTCLI_CIDCreate) && (!MINSTCLI_CIDCompatibility))
      return TRUE;
   // Otherwise ask just for security...
   MSG_Print (MINSTMSG_CLISureToInstallPackage);
   if (Inkey()==0x0D)
      return TRUE;
   return FALSE;
 }

VOID MINSTCID_CreateResponseFile (void) {
   FILE          *ResponseFile        = 0;
   PMINSTPUBGROUP CurGroupPtr         = MINSTCLI_PubGroupPtr;
   ULONG          CurGroupNo          = 0;
   ULONG          TotalSelectedGroups = 0;
   PSZ            SourcePathPtr       = 0;
   CHAR           SourcePath[MINSTMAX_PATHLENGTH] = {0};
   PSZ            TargetPathPtr       = 0;
   CHAR           TargetPath[MINSTMAX_PATHLENGTH] = {0};
   PSZ            CustomDataPtr       = 0;
   CHAR           CustomData[MINSTMAX_CUSTOMDATALENGTH];
   ULONG          MaxLength           = 0;

   ResponseFile = fopen(MINSTCLI_ResponseFileName, "w+");
   if (ResponseFile) {
      fprintf (ResponseFile, "/* Response file generated by Multimedia Installer CLI/CID */\n\n");
      fprintf (ResponseFile, "MMINSTALL = (\n");

      // Get current Source and Target-Path from MINSTALL...
      SourcePathPtr = MINSTALL_GetSourcePathPtr();
      TargetPathPtr = MINSTALL_GetTargetPathPtr();

      MaxLength = MINSTMAX_PATHLENGTH;
      if (STRING_BuildEscaped(SourcePath, &MaxLength, SourcePathPtr))
         fprintf (ResponseFile, "   SOURCE = \"%s\"\n", SourcePath);
      MaxLength = MINSTMAX_PATHLENGTH;
      if (STRING_BuildEscaped(TargetPath, &MaxLength, TargetPathPtr))
         fprintf (ResponseFile, "   TARGET = \"%s\"\n", TargetPath);
      fprintf (ResponseFile, "   TARGETBOOT = \"\"\n\n");

      while (CurGroupNo<MINSTCLI_PubGroupCount) {
         if (CurGroupPtr->Selected)
            TotalSelectedGroups++;
         CurGroupNo++; CurGroupPtr++;
       }
      fprintf (ResponseFile, "   GROUPS[%d] = (\n", TotalSelectedGroups);

      CurGroupNo = 0; CurGroupPtr = MINSTCLI_PubGroupPtr;
      while (CurGroupNo<MINSTCLI_PubGroupCount) {
         if (CurGroupPtr->Selected) {
            fprintf (ResponseFile, "      ( GROUPID    = %d\n", CurGroupPtr->ID);

            MaxLength = MINSTMAX_PATHLENGTH;
            CustomDataPtr = MINSTALL_GetPublicGroupCustomDataPtr(CurGroupPtr->ID);
            STRING_BuildEscaped(CustomData, &MaxLength, CustomDataPtr);
            fprintf (ResponseFile, "        CUSTOMDATA = \"%s\"\n", CustomData);
            fprintf (ResponseFile, "      )\n");
          }
         CurGroupNo++; CurGroupPtr++;
       }

      fprintf (ResponseFile, "    )\n");
      fprintf (ResponseFile, " )\n");
      fclose (ResponseFile);
      ResponseFile = 0;

      MSG_Print (MINSTMSG_CLIResponseFileCreated);
    }
 }

ULONG MINSTCLI_GetNextRespParmID (PCHAR *CurPosPtr, PCHAR EndPos, PULONG CurLineNoPtr, PLONG ArrayCountPtr) {
   PCHAR StartPos = *CurPosPtr;
   PCHAR CurPos   = StartPos;
   CHAR  CurChar  = *CurPos;
   ULONG ParamID;

   // Isolate till space or '='...
   StartPos = CurPos;
   while (CurPos<EndPos) {
      *CurPos = toupper(CurChar);
      if ((CurChar==0x0D) || (CurChar==' ') || (CurChar=='[') || (CurChar=='=')) {
         ParamID = CRC32_GetFromString(StartPos, CurPos-StartPos);
         *CurPosPtr = CurPos;
         if (!(CurChar = STRING_GetValidChar(CurPosPtr, EndPos, CurLineNoPtr))) {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; return 0; }
         if (CurChar=='[') {
            // We got an array here...
            *CurPosPtr += 1;
            if (!(*CurPosPtr = STRING_GetNumericValue ((PULONG)ArrayCountPtr, *CurPosPtr, EndPos))) {
               MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; return 0; }
            if (**CurPosPtr!=']') {
               MSG_Print (MINSTMSG_CLIResponseFileIsInvalid); return 0; }
            *CurPosPtr += 1;
            if (STRING_GetValidChar(CurPosPtr, EndPos, CurLineNoPtr)!='=') {
               MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; return 0; }
          } else if (CurChar!='=') {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; return 0;
          } else {
            *ArrayCountPtr = -1;
          }
         *CurPosPtr += 1;
         // Now seek to next valid char...
         if (!STRING_GetValidChar(CurPosPtr, EndPos, CurLineNoPtr)) {
            MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; return 0; }
         return ParamID;
       }
      CurPos++; CurChar = *CurPos;
    }
   // Experienced End-Of-Buffer...
   *CurPosPtr = CurPos; MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile;
   return 0;
 }

BOOL MINSTCLI_ReadResponseHeader (void) {
   PCHAR StartPos;
   PCHAR EndPos     = RESPONSEFILE.BufferEndPtr;
   PCHAR CurPos     = MINSTCLI_ResponseFilePos;
   ULONG CurLineNo  = MINSTCLI_ResponseFileLine;
   LONG  ArrayCount = -1;
   CHAR  CurChar;
   ULONG CRC32;
   ULONG GotParms   = 0;
   BOOL  HeaderDone = FALSE;
   CHAR  TargetBootPath[MINSTMAX_PATHLENGTH] = {0};
   ULONG TargetBootPathLen                   = 0;

   if (!STRING_GetValidChar (&CurPos, EndPos, &CurLineNo))
      return FALSE;                         // No more valid chars, so done...

   // Get first Parameter-ID...
   CRC32 = MINSTCLI_GetNextRespParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount);

   if (CRC32==0xF6AFBBDF) {                 // Is 'MMINSTALL'?
      // So, we found new response file format...
      MINSTCLI_CIDCompatibility = FALSE;

      if (*CurPos!='(') {
         MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid;
       } else {
         CurPos++;

         while ((CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo))!=0) {
            if (CurChar==')') {
               MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
            CRC32 = MINSTCLI_GetNextRespParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount);
            switch (CRC32) {
             case 0xAF72B8F5: // SOURCE - defines source path
               if (ArrayCount!=-1) {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
               if (!(GotParms & 0x01)) {
                  CurPos = STRING_GetString(MINSTCLI_SourcePath, MINSTMAX_PATHLENGTH, CurPos, EndPos);
                  if (!CurPos) {
                     MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; }
                  GotParms |= 0x01;
                } else {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
               break;
             case 0xB697E87A: // TARGET - defines target path
               if (ArrayCount!=-1) {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
               if (!(GotParms & 0x02)) {
                  CurPos = STRING_GetString(MINSTCLI_TargetPath, MINSTMAX_PATHLENGTH, CurPos, EndPos);
                  if (!CurPos) {
                     MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; }
                  GotParms |= 0x02;
                } else {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
               break;
             case 0xCF0988F4: // TARGETBOOT - defines target boot-drive
               if (ArrayCount!=-1) {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
               if (!(GotParms & 0x04)) {
                  CurPos = STRING_GetString(TargetBootPath, MINSTMAX_PATHLENGTH, CurPos, EndPos);
                  if (!CurPos) {
                     MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; }
                  TargetBootPathLen = strlen(TargetBootPath);
                  if (TargetBootPathLen==2) {
                     // If 2 chars specified, 2nd one has to be ':' and 1st one
                     //  is supposed to be the drive letter
                     if (TargetBootPath[1]!=':') {
                        MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
                     // We only set TargetBootDrive once. If multiple packages
                     //  are installed, they *will* use the same TargetBootDrive
                     //  anyway.
                     if (!MINSTCLI_TargetBootDrive) {
                        MINSTCLI_TargetBootDrive = TargetBootPath[0]-0x40;
                        if ((MINSTCLI_TargetBootDrive<1) || (MINSTCLI_TargetBootDrive>26)) {
                           MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
                      }
                   } else if (TargetBootPathLen) {
                     // If not 2 chars or no chars -> error!
                     MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
                  GotParms |= 0x04;
                } else {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
               break;
             case 0x0095FEF6: // GROUPS...
               if (ArrayCount==-1) {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
               GotParms  |= 0x08;
               if (*CurPos!='(') {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
               CurPos++;
               // We got the info we needed...
               MINSTCLI_ResponseGroups = ArrayCount; HeaderDone = TRUE;
               break;
             default:
               MINSTCLI_ErrorMsgID = MINSTMSG_BadCommand;
             }

            if ((MINSTCLI_ErrorMsgID) || (HeaderDone)) break;
          }

         // Check, if we got SOURCE, TARGET, TARGETBOOT and GROUPS...
         if (GotParms!=0x0F) {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
       }

    } else {
      // Oh, Oh, we possibly have to old ugly IBM format...
      MINSTCLI_CIDCompatibility = TRUE;
      while (CRC32) {
         if (ArrayCount!=-1) {              // Someone has defined array? NoWay
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
         switch (CRC32) {
          case 0xAEB9F0B1: // MMINSTSOURCE - defines source path
            if (!(GotParms & 0x01)) {
               CurPos = STRING_GetString(MINSTCLI_SourcePath, MINSTMAX_PATHLENGTH, CurPos, EndPos);
               if (!CurPos) {
                  MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; }
               GotParms |= 0x01;
             } else {
               MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
            break;
          case 0xB75CA03E: // MMINSTTARGET - defines target *drive*
            if (!(GotParms & 0x02)) {
               CurPos = STRING_GetString(MINSTCLI_TargetPath, MINSTMAX_PATHLENGTH, CurPos, EndPos);
               if (!CurPos) {
                  MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; }
               // Add ':\MMOS2' to the Target-Path
               strcpy (MINSTCLI_TargetPath+1, ":\\MMOS2");
               GotParms |= 0x02;
             } else {
               MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
            break;
          case 0x975037BD: // CHANGECONFIG - compatibility accept only...
            CurPos = STRING_SkipString (CurPos, EndPos);
            if (!CurPos) {
               MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; }
            break;
          case 0x015EB6B2: // MMINSTGROUPS - Lists groups & custom data
            // Check, if Source and Target are defined, then exit
            GotParms  |= 0x04;
            CurChar    = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo);
            if (CurChar!='(') {
               MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
            CurPos++; HeaderDone = TRUE;    // We got the info we needed...
            break;
          default:
            MINSTCLI_ErrorMsgID = MINSTMSG_BadCommand;
          }
         if ((MINSTCLI_ErrorMsgID) || (HeaderDone)) break;

         // No more chars? -> so break!
         if (!STRING_GetValidChar (&CurPos, EndPos, &CurLineNo)) {
            MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; break; }

         // Get next Parameter-ID...
         CRC32 = MINSTCLI_GetNextRespParmID (&CurPos, EndPos, &CurLineNo, &ArrayCount);
       }

      // Check, if we got SOURCE, TARGET and GROUPS...
      if (GotParms!=0x07) {
         MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
    }

   if (MINSTCLI_ErrorMsgID) {
      MSG_SetInsertFileLocation (1, RESPONSEFILE.Name, CurLineNo);
      MSG_Print (MINSTCLI_ErrorMsgID);
      return FALSE;
    }

   // Remember those variables for later...
   MINSTCLI_ResponseFilePos  = CurPos;
   MINSTCLI_ResponseFileLine = CurLineNo;
   return TRUE;
 }

BOOL MINSTCLI_ReadResponseData (void) {
   PCHAR StartPos;
   PCHAR EndPos     = RESPONSEFILE.BufferEndPtr;
   PCHAR CurPos     = MINSTCLI_ResponseFilePos;
   ULONG CurLineNo  = MINSTCLI_ResponseFileLine;
   LONG  ArrayCount = -1;
   CHAR  CurChar;
   ULONG CRC32;
   ULONG GroupID    = 0;
   ULONG GroupNo    = 0;
   ULONG GotParms   = 0;
   CHAR  CustomData[MINSTMAX_CUSTOMDATALENGTH];

   if (!MINSTCLI_CIDCompatibility) {
      // ============================================== We use new format...
      for (GroupNo=0; GroupNo<MINSTCLI_ResponseGroups; GroupNo++) {
         CurChar    = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo);
         if (CurChar!='(') {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
         CurPos++;

         GotParms = 0;
         while (1) {
            if (!(CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo))) {
               MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; break; }
            if (CurChar==')') {
               CurPos++; break; }           // End-Of-Parm-List
            CRC32 = MINSTCLI_GetNextRespParmID(&CurPos, EndPos, &CurLineNo, &ArrayCount);
            switch (CRC32) {
             case 0x47405208: // GROUPID
               if (ArrayCount!=-1) {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
               if (!(GotParms & 0x01)) {
                  if (!(CurPos = STRING_GetNumericValue(&GroupID, CurPos, EndPos))) {
                     MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
                  GotParms |= 0x01;
                }
               break;
             case 0x46BE0CE7: // CUSTOMDATA
               if (ArrayCount!=-1) {
                  MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
               if (!(GotParms & 0x02)) {
                  if (!(CurPos = STRING_GetString(CustomData, MINSTMAX_CUSTOMDATALENGTH, CurPos, EndPos))) {
                     MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
                  GotParms |= 0x02;
                }
               break;
             default:
               MINSTCLI_ErrorMsgID = MINSTMSG_BadCommand;
             }
          }
         if (MINSTCLI_ErrorMsgID) break;

         if (GotParms!=0x03) {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }

         // Select that group...
         MINSTALL_SelectGroup (GroupID);
         // And set CustomData...
         if (!MINSTALL_SetPublicGroupCustomData(GroupID, CustomData)) {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
       }
      if (!MINSTCLI_ErrorMsgID) {
         CurChar    = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo);
         if (CurChar==')') {
            CurPos++; CurChar = STRING_GetValidChar(&CurPos, EndPos, &CurLineNo);
            if (CurChar==')') {
               CurPos++;
             } else {
               MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
          } else {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }
       }

    } else {
      // ============================================== We use old format...
      while (1) {
         if (!(CurChar = STRING_GetValidChar (&CurPos, EndPos, &CurLineNo))) {
            MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; break; }
         if (CurChar==')') {
            CurPos++; break; }              // End-Of-Parm-List encountered

         StartPos = CurPos;
         if (!MINSTCLI_GetNextRespParmID(&CurPos, EndPos, &CurLineNo, &ArrayCount)) {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; }

         if ((CurPos-StartPos<6) || (strncmp(StartPos, "GROUP.", 6)!=0)) {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
         StartPos += 6;
         if (!STRING_GetNumericValue(&GroupID, StartPos, CurPos)) {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }

         CurPos = STRING_GetString(CustomData, MINSTMAX_CUSTOMDATALENGTH, CurPos, EndPos);
         if (!CurPos) {
            MINSTCLI_ErrorMsgID = MINSTMSG_UnexpectedEndOfFile; }

         // Select that group...
         MINSTALL_SelectGroup (GroupID);
         // And set CustomData...
         if (!MINSTALL_SetPublicGroupCustomData(GroupID, CustomData)) {
            MINSTCLI_ErrorMsgID = MINSTMSG_CLIResponseFileIsInvalid; break; }
       }
    }

   if (MINSTCLI_ErrorMsgID) {
      MSG_SetInsertFileLocation (1, RESPONSEFILE.Name, CurLineNo);
      MSG_Print (MINSTCLI_ErrorMsgID);
      return FALSE;
    }

   // Remember those variables for later...
   MINSTCLI_ResponseFilePos  = CurPos;
   MINSTCLI_ResponseFileLine = CurLineNo;
   return TRUE;
 }

USHORT main (int argc, char *argv[]) {
   HAB       OurPMHandle   = 0;
   HMQ       OurMSGQHandle = 0;
   PSZ       ErrorMsgPtr   = 0;

   PPIB      pib;
   PTIB      tib;

   STARTDATA StartData;
   ULONG     PIDOfSession  = 0;
   PID       PIDOfProcess  = 0;

   ULONG     CurArgument   = 0;
   ULONG     ArgumentLen   = 0;
   PCHAR     StartPos      = 0;
   PCHAR     CurPos        = 0;
   CHAR      CurChar;

   BOOL      BadParm       = FALSE;
   BOOL      BadUsage      = FALSE;
   BOOL      BadCombo      = FALSE;
   BOOL      DisplaySyntax = FALSE;

   USHORT    ReturnCode;
   PSZ       ReturnCodeDescription;

   printf("1\n");
   // Make ourself an PM application...
   DosGetInfoBlocks (&tib, &pib);
   pib->pib_ultype = 3;
   printf("1\n");

   // Initialize with message-file...
   if (!MSG_Init ("minstall.msg"))
      return FALSE;
   printf("1\n");

   MSG_Print (MINSTMSG_CLITrailer);

   CurArgument = 1;
   while (CurArgument<argc) {
      StartPos = argv[CurArgument];
      ArgumentLen = strlen(StartPos);

      if ((*StartPos=='/') && (ArgumentLen>1)) {
         StartPos++; ArgumentLen--;
         CurChar = toupper(*StartPos++); ArgumentLen--;

         switch (CurChar) {
          case 'C':                         // /C: record to response file
            if ((MINSTCLI_CIDMode) || (MINSTCLI_CIDCreate))
               BadCombo = TRUE;
            if ((ArgumentLen>1) && (*StartPos==':')) {
               MINSTCLI_CIDCreate = TRUE; MINSTCLI_CIDCompatibility = TRUE;
               if (!STRING_CopyPSZ(MINSTCLI_ResponseFileName, MINSTMAX_PATHLENGTH, StartPos+1))
                  BadUsage = TRUE;
             } else if ((ArgumentLen>2) && (*StartPos++=='2') && (*StartPos==':')) {
               ArgumentLen--;
               MINSTCLI_CIDCreate = TRUE;
               if (!STRING_CopyPSZ(MINSTCLI_ResponseFileName, MINSTMAX_PATHLENGTH, StartPos+1))
                  BadUsage = TRUE;
             } else BadUsage = TRUE;
            break;
          case 'F':                         // /F: specifies control.scr
            if ((ArgumentLen>1) && (*StartPos==':')) {
               StartPos++; ArgumentLen--;
               strupr (StartPos);
               if (ArgumentLen>=12) {
                  CurPos = StartPos+ArgumentLen-12;
                  if (strcmp(CurPos, "\\CONTROL.SCR")==0) {
                     ArgumentLen -= 12;
                   }
                }
               if (!STRING_Copy(MINSTCLI_SourcePath, MINSTMAX_PATHLENGTH, StartPos, StartPos+ArgumentLen))
                  BadUsage = TRUE;
             } else BadUsage = TRUE;
            break;
          case 'L':                         // /L: specifies logfile
            if ((ArgumentLen>1) && (*StartPos==':')) {
               if (!STRING_CopyPSZ(MINSTCLI_LogFileName, MINSTMAX_PATHLENGTH, StartPos+1))
                  BadUsage = TRUE;
             } else BadUsage = TRUE;
            break;
          case 'R':                         // /R: specifies response file (CID)
            if ((MINSTCLI_CIDMode) || (MINSTCLI_CIDCreate))
               BadCombo = TRUE;
            if ((ArgumentLen>1) && (*StartPos==':')) {
               MINSTCLI_CIDMode = TRUE;
               if (!STRING_CopyPSZ(MINSTCLI_ResponseFileName, MINSTMAX_PATHLENGTH, StartPos+1))
                  BadUsage = TRUE;
             } else BadUsage = TRUE;
            break;
          case 'M':                         // MMTEMP?!?!?!
            // Accepted only for compatibility
            break;
          case '?':
            DisplaySyntax = TRUE;
            break;
          case '*':                         // Makes MINSTALL use CLI interface
            break;
          default:
            BadParm = TRUE;
          }
       } else BadParm = TRUE;

      if (BadParm) {
         MSG_SetInsertViaPSZ (1, argv[CurArgument]);
         MSG_Print (MINSTMSG_CLIUnknownParameter);
         return CIDRET_IncorrectProgramInvocation;
       }
      if (BadUsage) {
         MSG_SetInsertViaString (1, argv[CurArgument], 2);
         MSG_Print (MINSTMSG_CLIInvalidUsageOfParm);
         return CIDRET_IncorrectProgramInvocation;
       }
      if (BadCombo) {
         MSG_Print (MINSTMSG_CLICantUseInCombinationParm);
         return CIDRET_IncorrectProgramInvocation;
       }

      CurArgument++;
    }

   // ======================================================= Display Syntax...
   if (DisplaySyntax) {
      MSG_Print (MINSTMSG_CLISyntaxExplanation); return 0; }

   if (argc<2) {
      APIRET rc;
      // User did not specify any command-line options, so try to execute
      //  MINSTPM.EXE, if possible. If this works, exit directly. Otherwise
      //  continue with normal CLI processing.

      memset(&StartData, 0, sizeof(StartData));
      StartData.Length      = sizeof(StartData);
      StartData.PgmName     = "minstpm.exe";
      StartData.SessionType = SSF_TYPE_PM;
      rc = DosStartSession (&StartData, &PIDOfSession, &PIDOfProcess);
      if (rc==0) {
         MSG_Print (MINSTMSG_CLIGaveControlToGUI); return 0; }
    }

   if (MINSTCLI_SourcePath[0]==0) {
      // Set default SourcePath, if not already set...
      if (!FILE_GetCurrentPath(MINSTCLI_SourcePath, MINSTMAX_PATHLENGTH))
         return CIDRET_UnexpectedCondition;
    }

   // We need to initialize with PM and get a Message Queue for MINSTALL.DLL...
   if (!(OurPMHandle = WinInitialize(0))) {
      MSG_Print (MINSTMSG_CLICouldNotInitPM);
      return CIDRET_UnexpectedCondition; }
   if (!(OurMSGQHandle = WinCreateMsgQueue(OurPMHandle, 0))) {
      MSG_Print (MINSTMSG_CLICouldNotInitMsgQueue);
      return CIDRET_UnexpectedCondition; }

   // Are we in CID-Mode?
   if (MINSTCLI_CIDMode) { // ======================================== CID-Mode
      MSG_Print (MINSTMSG_CLIExecutingInCIDMode);
      // Open response file and process it...
      strcpy (RESPONSEFILE.Name, MINSTCLI_ResponseFileName);
      if (!FILE_LoadFileControl(&RESPONSEFILE, 131767)) {
         MSG_SetInsertViaPSZ (1, MINSTCLI_ResponseFileName);
         MSG_Print (MINSTMSG_CouldNotLoad);
         return CIDRET_DataResourceNotFound; }
      // Remove remarks, TAB-Stops and other crap from file...
      FILE_PreProcessControlFile(&RESPONSEFILE);
      MINSTCLI_ResponseFilePos = RESPONSEFILE.BufferPtr;

      // Now read in header (or exit, if EOF)
      while (MINSTCLI_ReadResponseHeader()) {
         // We init per package, because options may change (TargetPath/etc.)
         if (!MINSTALL_Init (MINSTCLI_TargetBootDrive, OurPMHandle, OurMSGQHandle, MINSTCLI_TargetPath, MINSTCLI_LogFileName))
            break;
         if (!MINSTALL_InitPackage(MINSTCLI_SourcePath))
            break;
         // Get pointer to Public-Group Array
         MINSTCLI_PubGroupCount = MINSTALL_GetPublicGroupArrayPtr (&MINSTCLI_PubGroupPtr);
         // Now read in response file data and configure groups automatically
         if (!MINSTCLI_ReadResponseData())
            break;
         // Finally install this package...
         if (!MINSTALL_InstallPackage())
            break;
         // We need to CleanUp at anytime...
         MINSTALL_CleanUp();
         // MINSTALL.DLL will only delete log-file on first Init(). Afterwards
         //  data will just get appended, so this here will work.
       }
      FILE_UnLoadFileControl(&RESPONSEFILE);

    } else { // ====================================================== CLI-Mode
      // Initialize MINSTALL...
      if (MINSTALL_Init (0, OurPMHandle, OurMSGQHandle, "", MINSTCLI_LogFileName)) {
         // Now initialize the package...
         if (MINSTALL_InitPackage(MINSTCLI_SourcePath)) {
            // Get pointer to Public-Group Array
            MINSTCLI_PubGroupCount = MINSTALL_GetPublicGroupArrayPtr (&MINSTCLI_PubGroupPtr);

            // Let user select...
            if (MINSTCLI_UserGroupSelection()) {
               if ((MINSTCLI_CIDCreate) && (!MINSTCLI_CIDCompatibility)) {
                  // Generate a CID file w/o installation (no compatibility)
                  MINSTCID_CreateResponseFile();
                } else {
                  if (MINSTALL_InstallPackage()) {
                     // So, if CID file requested (using compatibility), we
                     //  install the package and create it now...
                     if ((MINSTCLI_CIDCreate) && (MINSTCLI_CIDCompatibility)) {
                        MINSTCID_CreateResponseFile();
                      }
                   }
                }
             }
          }
       }
    }
   // If there was an error, print it out...
   if ((ErrorMsgPtr = MINSTALL_GetErrorMsgPtr())!=0) {
      printf (ErrorMsgPtr); }

   if (MINSTCLI_ErrorMsgID) {
      ReturnCode = CIDRET_IncorrectProgramInvocation;
      ReturnCodeDescription = "CID: Incorrect program invocation";
    } else {
      // Get CID-Returncode from MINSTALL.DLL
      ReturnCode = MINSTALL_GetErrorMsgCIDCode();
      ReturnCodeDescription = MINSTALL_GetErrorMsgCIDCodeDescription();
    }

   // We need to CleanUp at anytime... (wouldnt be required on CID)
   MINSTALL_CleanUp();

   // Clean Up Message Queue and PM...
   WinDestroyMsgQueue (OurMSGQHandle);
   WinTerminate (OurPMHandle);
   printf ("Return-Code %X (%s)\n", ReturnCode, ReturnCodeDescription);
   return ReturnCode;
 }
