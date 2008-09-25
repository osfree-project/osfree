
// щ Д ДДДДНН = Д  щ  Д = ННДДДД Д щ
// і                               і
//    ЬЫЫЫЫЫЫЫЬ   ЬЫЬ  ЬЫЫЫЫЫЫЫЫЬ          ъ  ъДДДНДДНДННДДННННДНННННННННОД
// і ЫЫЫЫЯЯЯЫЫЫЫ ЫЫЫЫЫ ЫЫЫЯ   ЯЫЫЫ і             MINSTALL Front-End      є
// є ЫЫЫЫЬЬЬЫЫЫЫ ЫЫЫЫЫ ЫЫЫЬ   ЬЫЫЫ є      ъ ДДДДНДННДДННННДННННННННДНННННОД
// є ЫЫЫЫЫЫЫЫЫЫЫ ЫЫЫЫЫ ЫЫЫЫЫЫЫЫЫЯ  є       Section: MMOS/2 for eCS       є
// є ЫЫЫЫ   ЫЫЫЫ ЫЫЫЫЫ ЫЫЫЫ ЯЫЫЫЫЬ є     і Created: 28/10/02             є
// і ЯЫЫЯ   ЯЫЫЯ  ЯЫЯ  ЯЫЫЯ   ЯЫЫЯ і     і Last Modified:                і
//                  ЬЬЬ                  і Number Of Modifications: 000  і
// щ              ЬЫЫЯ             щ     і INCs required: *none*         і
//      ДДДДДДД ЬЫЫЯ                     є Written By: Martin Kiewitz    і
// і     ЪїЪїіЬЫЫЫЬЬЫЫЫЬ           і     є (c) Copyright by              і
// є     АЩіАЩЯЫЫЫЯЯЬЫЫЯ           є     є      AiR ON-Line Software '02 ъ
// є    ДДДДДДД    ЬЫЫЭ            є     є All rights reserved.
// є              ЬЫЫЫДДДДДДДДД    є    ДОНННДНННННДННННДННДДНДДНДДДъДД  ъ
// є             ЬЫЫЫЭі іЪїііД     є
// і            ЬЫЫЫЫ АДііАЩіД     і
//             ЯЫЫЫЫЭДДДДДДДДДД
// і             ЯЯ                і
// щ Дґ-=’iз йп-Liпо SйџвW’зо=-ГДД щ

#define INCL_NOPMAPI
#define INCL_BASE
#define INCL_DOSMODULEMGR
#include <os2.h>

#include <malloc.h>
#include <string.h>

#include <global.h>
#include <crcs.h>
#include <file.h>

// PCHAR FILE_CONFIGSYS           = 0;
// PCHAR FILE_DelayUtilFileName   = 0;
// PCHAR FILE_DelayListFileName   = 0;
// PCHAR FILE_DelayDirectory      = 0;

// Copies a file, will not replace existing file. Replies TRUE, if succeeded
APIRET FILE_Copy (PSZ SourceFileName, PSZ DestFileName) {
   APIRET rc;
   if (!(rc = DosCopy (SourceFileName, DestFileName, 0)))
      rc = FILE_ResetAttributes (DestFileName);
   return rc;
 }

// Copies a file, replaces existing file. Replies TRUE, if succeeded
BOOL FILE_Replace (PSZ SourceFileName, PSZ DestFileName) {
   APIRET rc;
   if (!(rc = DosCopy (SourceFileName, DestFileName, DCPY_EXISTING)))
      FILE_ResetAttributes (DestFileName);
   return rc;
 }

// This creates a multi-level directory... Needs a FQ-directory/filename as
//  input. "C:\test\test" is not correct. Use "C:\test\test\".
BOOL FILE_CreateDirectory (PSZ DirectoryName) {
   CHAR   TempDirectoryName[MAXFILELENGTH];
   PCHAR  CurPos = (PCHAR)&TempDirectoryName;
   PCHAR  EndPos = (PCHAR)((ULONG)&TempDirectoryName+strlen(DirectoryName));
   APIRET rc     = 0;

   if ((EndPos-CurPos)>=MAXFILELENGTH)
      return FALSE;                         // due Buffer-Overflow

   // Copy Directory-Name...
   strcpy (TempDirectoryName, DirectoryName);

   CurPos += 3;                             // skip over "x:\"
   while (CurPos<EndPos) {
      if (*CurPos==0x5C) {
         *CurPos = 0;                       // Set terminating NUL...
         rc = DosCreateDir (TempDirectoryName, NULL);
         if ((rc!=0) && (rc!=ERROR_ACCESS_DENIED))
            return FALSE;
         *CurPos = 0x5C;                    // Set '\' again...
       }
      CurPos++;
    }
   return TRUE;
 }

// Will reset R/O, System, Hidden and Archive attributes on a given file
BOOL FILE_ResetAttributes (PSZ FileName) {
   FILESTATUS3 CurFileInfo = {0};
   CurFileInfo.attrFile = FILE_NORMAL;
   if (DosSetPathInfo (FileName, FIL_STANDARD, &CurFileInfo, sizeof(CurFileInfo), 0))
      return FALSE;
   return TRUE;
 }

// Loads in a maximum of 128k of a control file. We don't support more for
//  security, because we load the whole file at once and we dont know IF the
//  file actually IS a control file.
BOOL FILE_LoadFileControl (PFILECONTROL FileControl, ULONG MaxFileSize) {
   FILE *FileHandle = 0;

   FileControl->BufferPtr  = 0;
   FileControl->BufferSize = 0;

   FileHandle = fopen(FileControl->Name, "rb");
   if (FileHandle==NULL)
      return FALSE;

   // Get File-Length of INI-File, check for maximum buffer-length
   fseek (FileHandle, 0, SEEK_END);
   FileControl->BufferSize = ftell(FileHandle);
   if (FileControl->BufferSize>=MaxFileSize) FileControl->BufferSize = MaxFileSize;

   FileControl->BufferPtr = malloc(FileControl->BufferSize+1);
   if (FileControl->BufferPtr==NULL) {
      fclose (FileHandle);
      return FALSE;
    }

   FileControl->BufferEndPtr = (PCHAR)((ULONG)FileControl->BufferPtr+FileControl->BufferSize);

   // Read in CONTROL.SCR-File (maximum MaxFileSize)
   fseek (FileHandle, 0, SEEK_SET); // Seek back to start
   if (FileControl->BufferSize != fread(FileControl->BufferPtr, 1, FileControl->BufferSize, FileHandle)) {
      fclose (FileHandle);
      FILE_UnLoadFileControl (FileControl);
      return FALSE;
    }

   // Put ending NUL for safety
   *FileControl->BufferEndPtr = 0;

   // And close INI-File afterwards...
   fclose (FileHandle);
   return TRUE;
 }

VOID FILE_UnLoadFileControl (PFILECONTROL FileControl) {
   if (FileControl->BufferPtr) {
      free (FileControl->BufferPtr);
      FileControl->BufferPtr = 0;
    }
 }

// Replaces all comments and LFs with CRs in a Control-File buffer, so the
//  actual parse routines will not process this data.
VOID FILE_PreProcessControlFile (PFILECONTROL FileControl) {
   PCHAR CurPos   = FileControl->BufferPtr;
   PCHAR StartPos = CurPos;
   CHAR  CurChar  = 0;

   while (CurPos<FileControl->BufferEndPtr) {
      if (*(PUSHORT)CurPos==0x2A2F) {
         *(PUSHORT)CurPos = 0x2020; StartPos = CurPos; CurPos += 2;
         while (CurPos<FileControl->BufferEndPtr) {
            if (*(PUSHORT)CurPos==0x2F2A) { // Got End-Of-Comment
               *(PUSHORT)CurPos = 0x2020; CurPos++;
               break;
             } else if (*CurPos!=0x0D)
               *CurPos = 0x20;              // Fill in spaces...
            CurPos++;
          }
       } else {
         CurChar = *CurPos;
         if ((CurChar==0x0A) || (CurChar==0x1A) || (CurChar==0x09))
            *CurPos = 0x20; // Replace LF, EOF and TAB with space
       }
      CurPos++;
    }
 }

ULONG FILE_CountControlFileLines (PFILECONTROL FileControl) {
   PCHAR CurPos     = FileControl->BufferPtr;
   PCHAR StartPos   = CurPos;
   ULONG TotalLines = 0;

   while (CurPos<FileControl->BufferEndPtr) {
      if (*CurPos!=0x0D) {
         StartPos = CurPos;
         while ((CurPos<FileControl->BufferEndPtr) & (*CurPos!=0x0D)) {
            if ((*CurPos!=0x20)) {          // Count line, if not empty...
               TotalLines++;
               while ((CurPos<FileControl->BufferEndPtr) & (*CurPos!=0x0D))
                  CurPos++;
               break;
             } else CurPos++;
          }
       }
      CurPos++;
    }
   return TotalLines;
 }

// Checks, if FileName consists of path and filename, or filename only.
//  will return FALSE; when no path got found (by looking for '\' character.
BOOL FILE_IncludesPath (PSZ FileName) {
   PCHAR CurPos = FileName;

   while (*CurPos!=0) {
      if (*CurPos==0x5C)
         return TRUE;
      CurPos++;
    }
   return FALSE;
 }

VOID FILE_SetDefaultExtension (PSZ FileName, ULONG FileNameMaxSize, PSZ DefaultExtension) {
   ULONG FileNameLen = strlen(FileName);
   PCHAR EndPos      = (PCHAR)((ULONG)FileName+FileNameLen);
   PCHAR CurPos      = EndPos;
   ULONG DefaultLen;

   // Don't modify NUL filenames
   if (!FileNameLen) return;

   while (CurPos>FileName) {
      if (*CurPos==0x5C)                    // If '\' is found, add extension
         break;
      if (*CurPos=='.')                     // Got '.' so has extension, exit
         return;
      CurPos--;
    }
   // Check, if we have enough place left
   DefaultLen = strlen(DefaultExtension);
   if ((FileNameMaxSize-FileNameLen)>DefaultLen) {
      // Copy extension over including terminating NUL
      memcpy (EndPos, DefaultExtension, DefaultLen+1);
    }
 }

VOID FILE_PutS (HFILE FileHandle, PSZ String) {
   ULONG StringLen = strlen(String);
   ULONG Written;

   DosWrite (FileHandle, String, StringLen, &Written);
 }

BOOL FILE_GetCurrentPath (PSZ DestCurrentPath, ULONG PathMaxSize) {
   ULONG  CurrentDisk       = 0;
   ULONG  CurrentLogDisks   = 0;
   CHAR   CurrentDir[MAXFILELENGTH];
   ULONG  CurrentDirMaxSize = MAXFILELENGTH;

   // Find out path, where we currently are...
   if (DosQueryCurrentDisk (&CurrentDisk, &CurrentLogDisks))
      return FALSE;
   if (DosQueryCurrentDir (0, CurrentDir, &CurrentDirMaxSize))
      return FALSE;
   if (strlen(CurrentDir)+3>=PathMaxSize)
      return FALSE;                         // due Buffer-overflow

   sprintf (DestCurrentPath, "%c:\\%s", CurrentDisk+64, CurrentDir);
   return TRUE;
 }
