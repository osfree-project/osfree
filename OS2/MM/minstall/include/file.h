#ifndef _FILE_H_
#define _FILE_H_

#include <stdio.h>

typedef struct
  {
    PSZ Name;
    PCHAR BufferEndPtr;
    PCHAR BufferPtr;
    ULONG BufferSize;
  } FILECONTROL, *PFILECONTROL;

#define MAXFILELENGTH CCHMAXPATH

// Copies a file, will not replace existing file. Replies TRUE, if succeeded
APIRET FILE_Copy (PSZ SourceFileName, PSZ DestFileName);
// Copies a file, replaces existing file. Replies TRUE, if succeeded
BOOL FILE_Replace (PSZ SourceFileName, PSZ DestFileName);
// This creates a multi-level directory... Needs a FQ-directory/filename as
//  input. "C:\test\test" is not correct. Use "C:\test\test\".
BOOL FILE_CreateDirectory (PSZ DirectoryName);
// Will reset R/O, System, Hidden and Archive attributes on a given file
BOOL FILE_ResetAttributes (PSZ FileName);
// Loads in a maximum of 128k of a control file. We don't support more for
//  security, because we load the whole file at once and we dont know IF the
//  file actually IS a control file.
BOOL FILE_LoadFileControl (PFILECONTROL FileControl, ULONG MaxFileSize);
VOID FILE_UnLoadFileControl (PFILECONTROL FileControl);
// Replaces all comments and LFs with CRs in a Control-File buffer, so the
//  actual parse routines will not process this data.
VOID FILE_PreProcessControlFile (PFILECONTROL FileControl);
ULONG FILE_CountControlFileLines (PFILECONTROL FileControl);
// Checks, if FileName consists of path and filename, or filename only.
//  will return FALSE; when no path got found (by looking for '\' character.
BOOL FILE_IncludesPath (PSZ FileName);
VOID FILE_SetDefaultExtension (PSZ FileName, ULONG FileNameMaxSize, PSZ DefaultExtension);
VOID FILE_PutS (HFILE FileHandle, PSZ String);
BOOL FILE_GetCurrentPath (PSZ DestCurrentPath, ULONG PathMaxSize);

#endif
