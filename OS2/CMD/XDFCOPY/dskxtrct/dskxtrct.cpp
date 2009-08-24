//ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³ DSKXTRCT - main program/processing
//³ 16 Feb 98 - split initialize code into common and local
//³           - added replace file logic, with added counters
//³           - add file replace control and prompting
//³           - add PrintHelp if no options
//³ 04 Jul 98 - add crc checking
//ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³
//³ This file is part of the source code to dskxtrct and is distributed under
//³ the terms of the GNU General Public Licence.
//³
//ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

char
   *PgmTitle = "DSKXTRCT",
   *PgmVer = "1.3 (2 Apr 03)",
   *PgmCopyright = "Copyright 1997, 1998 by Alan B. Arnett\n"
                   "Updated 2003 by Lars Erdmann\n"
      "[Free distribution permitted under the terms of the GNU GPL]";

#define INCL_DOSERRORS                         /* DOS error values    */
#include <os2.h>
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

#define BOOL int
#define MAXSTRLEN 2048
#define Exit_Fail 16
#define Exit_Error 8
#define Exit_Warning 4
#define Exit_Normal 0

struct
{
   char *Source;
   char *Target;
   char *Log;
   FILE *LogFile;
   int LogReplace;
   int QuietMode;
   int DskCount;
   int CreateCount;
   int ReplaceCount;
   int SkipCount;
   int ErrorCount;
   int MsgLevel;
   int Replace;
   int ReplaceNone;
   int Fail;
   int Quit;                                   // quit if help was printed
   int CommentDisplay;
   int ViewList;
   int RCHigh;
   int CrcCount;
   int CrcOnly;
   int CrcIgnore;
   int HasLeader;
}   Ctl;


#include "printmsg.i"
#include "initial0.i"
#include "initial2.i"
#include "initial.i"
#include "dsklogic.i"

void BuildTargetDirectory(char *subdname)
{
   char xMsg[MAXSTRLEN], dname[MAXSTRLEN];

   if (Ctl.ViewList || Ctl.CrcOnly)
      return;
   DskBuildName(dname, Ctl.Target, subdname);
   if (access(dname, 0) != 0)
      if (mkdir(dname) != 0)
         ErrorExit1("Unable to create directory: '%s'", dname);
      else
      {
         sprintf(xMsg, "Directory created: '%s'", dname);
         PrintMsg(MsgDebug, xMsg);
      }
}

enum
{
   CheckFileCreate, CheckFileReplace, CheckFileSkip, CheckFileFail
};
int ReplaceFilePrompt(char *FileName)
{
   char ReplyStr[MAXSTRLEN];
   int Reply = -1;

   while (Reply < 0)
   {
      cout << "Replace file: " << FileName << "?" << endl;
      cout << "   Select: (r)eplace, (s)kip, replace (a)ll, replace (n)one, or (f)ail" << endl;
      cin >> ReplyStr;
      if (stricmp(ReplyStr, "r") == 0)
         Reply = CheckFileReplace;
      if (stricmp(ReplyStr, "s") == 0)
         Reply = CheckFileSkip;
      if (stricmp(ReplyStr, "a") == 0)
      {
         Ctl.Replace = TRUE;
         Reply = CheckFileReplace;
      }
      if (stricmp(ReplyStr, "n") == 0)
      {
         Ctl.ReplaceNone = TRUE;
         Reply = CheckFileSkip;
      }
      if (stricmp(ReplyStr, "f") == 0)
      {
         PrintMsg(MsgFail, "Processing aborted by request");
         exit(Exit_Fail);
      }
   }
   return (Reply);
}
int CheckFileAccess(char *FileName)
{
   if (access(FileName, 0) == -1)
      return (CheckFileCreate);
   if (Ctl.Replace)
      return (CheckFileReplace);
   if (Ctl.ReplaceNone)
      return (CheckFileSkip);
   return (ReplaceFilePrompt(FileName));
}
void BuildFilePrintMsg(int MsgLevel, DskFileCtl * FileCB, char *MsgPrefix)
{
   char xMsg[MAXSTRLEN], xMsgW[MAXSTRLEN];

   sprintf(xMsg, "%sile: %s  size: %d", MsgPrefix, FileCB->pFullName,
         FileCB->Size);
   if (Ctl.MsgLevel >= MsgDebug)
   {
      strcpy(xMsgW, xMsg);
      sprintf(xMsg, "%s  cluster: '%d'  mem: '%x'", xMsgW,
            FileCB->Cluster, FileCB->pData);
   }
   PrintMsg(MsgLevel, xMsg);
}
void BuildFilePrintMsgByType(int Reply, DskFileCtl * FileCB)
{
   char xMsg[MAXSTRLEN];

   switch (Reply)
   {
   case CheckFileSkip:
      strcpy(xMsg, "Skipping f");
      break;
   case CheckFileReplace:
      strcpy(xMsg, "Replacing f");
      break;
   case CheckFileCreate:
      strcpy(xMsg, "Creating f");
      break;
   }
   BuildFilePrintMsg(Ctl.MsgLevel, FileCB, xMsg);
}

void BuildFile(DskFileCtl * FileCB)
{
   char xMsg[MAXSTRLEN], fname[MAXSTRLEN];
   int OFile = 0, Create = FALSE;
   struct stat StatBuf;

   if (Ctl.CrcOnly)
      return;
   if (Ctl.ViewList)
   {
      BuildFilePrintMsg(MsgMust, FileCB, "F");
      Ctl.CreateCount++;
      return;
   }

   DskBuildName(fname, Ctl.Target, FileCB->pFullName);
   int FileAccessReply = CheckFileAccess(fname);

   if (Ctl.MsgLevel >= MsgFile)
      BuildFilePrintMsgByType(FileAccessReply, FileCB);
   if (FileAccessReply == CheckFileSkip)
   {
      Ctl.SkipCount++;
      return;
   }

   if (-1 == (OFile = open(fname, O_CREAT | O_WRONLY | O_BINARY, S_IWRITE)))
   {
      sprintf(xMsg, "Unable to open output file: '%s'", fname);
      PrintMsg(MsgError, xMsg);
   }
   else
   {
      Create = TRUE;
      while (FileCB->DataSize > 0 && Create == TRUE)
      {
         if (write(OFile, FileCB->pData, FileCB->DataSize) != FileCB->DataSize)
         {
            sprintf(xMsg, "Error writing output file: %s", fname);
            PrintMsg(MsgError, xMsg);
            Create = FALSE;
         }
         else
            DskReadNext(FileCB);
      }
   }
   if (OFile != 0)
   {
      if (_chsize(OFile, FileCB->Size) < 0)
      {
         sprintf(xMsg, "Error setting size of target file: %s", fname);
         PrintMsg(MsgError, xMsg);
         Create = FALSE;
      }
      if (close(OFile) < 0)
      {
         sprintf(xMsg, "Error closing input file: %s", fname);
         PrintMsg(MsgError, xMsg);
         Create = FALSE;
      }
      struct utimbuf ubuf;

      if (utime(fname, DskBuildDateTime(&ubuf, FileCB)) < 0)
      {
         sprintf(xMsg, "Error setting date/time of target file: %s", fname);
         PrintMsg(MsgError, xMsg);
         Create = FALSE;
      }
   }
   if (!Create)
      Ctl.ErrorCount++;
   else if (FileAccessReply == CheckFileCreate)
      Ctl.CreateCount++;
   else
      Ctl.ReplaceCount++;
}

void ProcessDskFile(char *sfile)
{
   char xMsg[MAXSTRLEN];
   DskFileCtl *pFileCB = 0;

   sprintf(xMsg, "Processing input file: '%s'", sfile);
   PrintMsg(MsgStd, xMsg);
   if (access(sfile, 0) != 0)
      ErrorExit1("Unable to access source dsk file: %s", sfile);
   int rc = DskFindFirst(sfile, &pFileCB);

   if (rc != Dsk_no_more_files && Ctl.CommentDisplay == TRUE)
      PrintMsg(MsgInfo, pFileCB->pDskCB->pBuf + 40); // print file comments
   while (rc != Dsk_no_more_files)
   {
      if (pFileCB->Attributes & DskDirAttrSubDir)
         BuildTargetDirectory(pFileCB->pFullName);
      else
         BuildFile(pFileCB);
      rc = DskFileNext(pFileCB);
   }
   DskClose(pFileCB);
}

char *BuildSDir(char *sdir, char *spath)
{
   int found = FALSE;

   strcpy(sdir, spath);
   for (int i = strlen(sdir); i > 0; i--)
   {
      if (sdir[i] == '\\')
      {
         sdir[i] = '\0';
         found = TRUE;
         break;
      }
   }
   if (found == FALSE)
      strcpy(sdir, "");
   return (sdir);
}

void ProcessInputMask()
{
   HDIR SDirHandle = HDIR_SYSTEM;
   FILEFINDBUF3 FindBuffer = {0};
   ULONG ResultBufLen = sizeof(FILEFINDBUF3);
   ULONG FindCount = 1;
   APIRET rc = NO_ERROR;
   char Sdir[MAXSTRLEN];
   char fname[MAXSTRLEN];

   BuildSDir(Sdir, Ctl.Source);
   rc = DosFindFirst(Ctl.Source, &SDirHandle, FILE_NORMAL, &FindBuffer, ResultBufLen,
                 &FindCount, FIL_STANDARD);
   if (rc != NO_ERROR)
      ErrorExit1("Search for input files failed: %s", Ctl.Source);
   while (rc != ERROR_NO_MORE_FILES)
   {
      Ctl.DskCount++;
      DskBuildName(fname, Sdir, FindBuffer.achName);
      ProcessDskFile(fname);
      FindCount = 1;
      rc = DosFindNext(SDirHandle, &FindBuffer, ResultBufLen, &FindCount);
      if (rc != NO_ERROR && rc != ERROR_NO_MORE_FILES)
         ErrorExit1("Search for additional input files failed: %s", Ctl.Source);
   }
   rc = DosFindClose(SDirHandle);
}

int main(int argc, char *argv[])
{
   char xMsg[MAXSTRLEN];

   memset(&Ctl, '\0', sizeof(Ctl));
   PrintPgmTitle();
   if (argc == 1)
   {
      PrintHelp();
      exit(Exit_Warning);
   }
   initialize(argc, &argv[0]);
   if (Ctl.Fail)
   {
      PrintMsg(MsgError, "Terminating due to above initialization error(s)");
      PrintHelp();
      exit(Exit_Error);
   }
   if (Ctl.Quit)
      exit(Exit_Warning);
   PrintLogFileOpen();
   if (!Ctl.ViewList && !Ctl.CrcOnly)
      BuildTargetDirectory("");
   ProcessInputMask();
   sprintf(xMsg, "DSK files processed: %d", Ctl.DskCount);
   PrintMsg(MsgInfo, xMsg);
   if (Ctl.ViewList)
   {
      sprintf(xMsg, "Files listed:       %d", Ctl.CreateCount);
      PrintMsg(MsgInfo, xMsg);
   }
   if (Ctl.CrcOnly)
   {
      sprintf(xMsg, "CRC errors found:    %d", Ctl.CrcCount);
      PrintMsg(MsgInfo, xMsg);
   }
   if (!Ctl.ViewList && !Ctl.CrcOnly)
   {
      sprintf(xMsg, "Files created:       %d", Ctl.CreateCount);
      PrintMsg(MsgInfo, xMsg);
      sprintf(xMsg, "Files replaced:      %d", Ctl.ReplaceCount);
      PrintMsg(MsgInfo, xMsg);
      sprintf(xMsg, "Files skipped:       %d", Ctl.SkipCount);
      PrintMsg(MsgInfo, xMsg);
      sprintf(xMsg, "Error count:         %d", Ctl.ErrorCount);
      PrintMsg(MsgInfo, xMsg);
      sprintf(xMsg, "CRC errors found:    %d", Ctl.CrcCount);
      PrintMsg(MsgInfo, xMsg);
      if (Ctl.CrcCount > 0)
      {
         sprintf(xMsg, "*** WARNING: CRC ERRORS WERE FOUND");
         PrintMsg(MsgInfo, xMsg);
         sprintf(xMsg, "*** CHECK THE LOG AND SEE THE DOCUMENTATION ABOUT CRC ERRORS");
         PrintMsg(MsgInfo, xMsg);
      }
   }
   sprintf(xMsg, "Ending with return code: %d", Ctl.RCHigh);
   PrintMsg(MsgInfo, xMsg);
   PrintLogClose();
   return (Ctl.RCHigh);
}
