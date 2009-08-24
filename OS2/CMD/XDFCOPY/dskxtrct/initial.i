//ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³ XTRCDSK - Initial, program initialization
//³ Author:  Alan B. Arnett
//³ Copyright 1997, 1998 by Alan B. Arnett
//³ 16 Feb 98 - split initialize into local and common sections
//³  4 Jul 98 - add crc parms to help msg
//³ 28 Aug 00 -
//³ 02 Apr 03 - Lars Erdmann: DSKXTRCT now handles files without leader (savedskf /D)
//ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³
//³ This file is part of the source code to dskxtrct and is distributed under
//³ the terms of the GNU General Public Licence.
//³
//ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

/* ------------------ Initialize  ------------------- */

void PrintHelp()
{
   PrintMsg(MsgMust, "syntax: DSKXTRCT /s:<file> /t:<file> [/ra|/rn]");
   PrintMsg(MsgMust, "        DSKXTRCT /s:<file> [/v]");
   PrintMsg(MsgMust, "  /s:<mask> - source file mask of dsk files");
   PrintMsg(MsgMust, "  /t:<sdir> - target directory for file creation");
   PrintMsg(MsgMust, "  /l:<file> - logfile filename");
   PrintMsg(MsgMust, "  /m:<mode> - message mode [STD, NONE, FILES]");
   PrintMsg(MsgMust, "  /r(a|n) - duplicate files - replace all | none");
   PrintMsg(MsgMust, "  /c - display dsk file comments");
   PrintMsg(MsgMust, "  /v - only list file names and sizes");
   PrintMsg(MsgMust, "  /l(r|a|s) - duplicate log file - replace | append | skip");
   PrintMsg(MsgMust, "  /q - write messages only to the log file");
   PrintMsg(MsgMust, "  /crc - only perform CRC checking");
   PrintMsg(MsgMust, "  /crcx - ignore CRC errors");
   PrintMsg(MsgMust, "  /? - display this help message");
   PrintMsg(MsgMust, "   ");
}

void initialize(int argc, char *argv[])
{
   Ctl.CreateCount = 0;
   Ctl.ReplaceCount = 0;
   Ctl.SkipCount = 0;
   initialize2(argc, &argv[0]);
   if (strlen(Ctl.Source) == 0 && !Ctl.Quit)
   {
      PrintMsg(MsgError, "Source filename(s) or pattern is required");
      Ctl.Fail = TRUE;
   }
   if (strlen(Ctl.Target) == 0 && !Ctl.Quit && !Ctl.ViewList && !Ctl.CrcOnly)
   {
      PrintMsg(MsgError, "Target directory is required");
      Ctl.Fail = TRUE;
   }
}
