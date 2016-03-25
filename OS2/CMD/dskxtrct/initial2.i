//ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³ XTRCDSK - Initial2, program initialization, shared section
//³ Author:  Alan B. Arnett
//³ Copyright 1997, 1998 by Alan B. Arnett
//³ 16 Feb 98 - split initialize into local and common sections
//³ 24 May 98 - add crc parms to parm checking
//ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³
//³ This file is part of the source code to dskxtrct and is distributed under
//³ the terms of the GNU General Public Licence.
//³
//ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#ifndef MAXSTRLEN
#define MAXSTRLEN 2048
#endif

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void PrintHelp();

#ifdef __GNUC__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define strcmpi strcasecmp
#include <sys/stat.h> 
#include <errno.h>
#endif

/* ------------------ Initialize  ------------------- */

void ProcessParmsFound(int argc, char *argv[], int argi, int optj)
{
       char xMsg[MAXSTRLEN];
       int lvli;

       switch (optj)
       {
       case OptSource:
               sprintf(xMsg, "Processing Source: '%s'", argv[argi]);
               PrintMsg(MsgDebug, xMsg);
               if (Ctl.Source != NULL)
                       free(Ctl.Source);
               Ctl.Source = strdup(&argv[argi][3]);
               break;
       case OptTarget:
               sprintf(xMsg, "Processing Target: '%s'", argv[argi]);
               PrintMsg(MsgDebug, xMsg);
               if (Ctl.Target != NULL)
                       free(Ctl.Target);
               Ctl.Target = strdup(&argv[argi][3]);
               break;
       case OptLog:
               sprintf(xMsg, "Processing Message Log: '%s'", argv[argi]);
               PrintMsg(MsgDebug, xMsg);
               if (Ctl.Log != NULL)
                       free(Ctl.Log);
               Ctl.Log = strdup(&argv[argi][3]);
               break;
       case OptLogReplace:
               PrintLogOptLogReplace();
               break;
       case OptLogAppend:
               PrintLogOptLogAppend();
               break;
       case OptLogSkip:
               PrintLogOptLogSkip();
               break;
       case OptMsgLevel:
               sprintf(xMsg, "Processing Message Level: '%s'", argv[argi]);
               PrintMsg(MsgDebug, xMsg);
               for (lvli = 0; lvli < MsgLvlCount &&
                        strcmpi(MsgLvl[lvli].Str, &argv[argi][3]) != 0;
                        lvli++);
               if (lvli < MsgLvlCount)
               {
                       Ctl.MsgLevel = MsgLvl[lvli].Level;
                       sprintf(xMsg, "Found message level str: '%s', level: '%d'",
                                       MsgLvl[lvli].Str, MsgLvl[lvli].Level);
                       PrintMsg(MsgDebug, xMsg);
               }
               else
               {
                       sprintf(xMsg, "Message level '%s' is not valid, ignored",
                                       &argv[argi][3]);
                       PrintMsg(MsgError, xMsg);
               }
               break;
       case OptReplaceAll:
               PrintMsg(MsgDebug, "File replace ALL option specified");
               Ctl.Replace = TRUE;
               break;
       case OptReplaceNone:
               PrintMsg(MsgDebug, "File replace NONE option specified");
               Ctl.ReplaceNone = TRUE;
               break;
       case OptComment:
               PrintMsg(MsgDebug, "Comment display option specified");
               Ctl.CommentDisplay = TRUE;
               break;
       case OptViewList:
               PrintMsg(MsgDebug, "View list option specified");
               Ctl.ViewList = TRUE;
               break;
       case OptCrcOnly:
               PrintMsg(MsgDebug, "CRC-only option specified");
               Ctl.CrcOnly = TRUE;
               break;
       case OptCrcIgnore:
               PrintMsg(MsgDebug, "CRC-ignore option specified");
               Ctl.CrcIgnore = TRUE;
               break;
       case OptQuietMode:
               PrintMsg(MsgDebug, "Quiet Mode option specified");
               Ctl.QuietMode = TRUE;
               break;
       case OptHelp:
               PrintHelp();
               Ctl.Quit = TRUE;
               break;
       }
}

static void ProcessParms(int argc, char *argv[])
{
       char xMsg[MAXSTRLEN];
       int argi, optj;

       if ((Ctl.MsgLevel & MsgDebug) > 0)
       {
               sprintf(xMsg, "ProcessParms Argc: %d", argc);
               PrintMsg(MsgDebug, xMsg);
               for (argi = 0; argi < argc; argi++)
               {
                       sprintf(xMsg, "ProcessParms Argv[%d]: '%s'", argi, argv[argi]);
                       PrintMsg(MsgDebug, xMsg);
               }
       }
       argi = 0;
       while (argi < argc)
       {
               if (argv[argi][0] != '/' || strlen(argv[argi]) < 2)
               {
                       sprintf(xMsg, "Invalid parameter: '%s'\n", argv[argi]);
                       PrintMsg(MsgError, xMsg);
                       Ctl.Fail = TRUE;
               }
               else
               {
                       for (optj = 0; optj < OptCount; optj++)
                       {
                               if (strnicmp(OptEntry[optj].Str, &argv[argi][1],
                                                        strlen(OptEntry[optj].Str)) == 0)
                                       break;
                       }
                       if (optj == OptCount)
                       {
                               sprintf(xMsg, "Unrecognized parameter: '%s'\n", argv[argi]);
                               PrintMsg(MsgError, xMsg);
                               Ctl.Fail = TRUE;
                       }
                       else if (OptEntry[optj].Type == ParmString && strlen(argv[argi]) < 3)
                       {
                               sprintf(xMsg, "Invalid parameter format: '%s'\n", argv[argi]);
                               PrintMsg(MsgError, xMsg);
                               Ctl.Fail = TRUE;
                       }
                       else if (OptEntry[optj].Type == ParmString && argv[argi][2] != ':')
                       {
                               sprintf(xMsg, "Invalid parameter format: '%s'\n", argv[argi]);
                               PrintMsg(MsgError, xMsg);
                               Ctl.Fail = TRUE;
                       }
                       else
                       {
                               sprintf(xMsg, "Processing Option[%d], Argv[%d]: '%s'", optj, argi, argv[argi]);
                               PrintMsg(MsgDebug, xMsg);
                               ProcessParmsFound(argc, &argv[0], argi, optj);
                       }
               }
               argi++;
       }
}

void initialize2(int argc, char *argv[])
{
       char xMsg[MAXSTRLEN];

       Ctl.Source = strdup("");
       Ctl.Target = strdup("");
       Ctl.MsgLevel = MsgStd;
       Ctl.Replace = FALSE;
       Ctl.ReplaceNone = FALSE;
       Ctl.QuietMode = FALSE;
       Ctl.Fail = FALSE;
       Ctl.Quit = FALSE;
       Ctl.CommentDisplay = FALSE;
       Ctl.ViewList = FALSE;
       Ctl.RCHigh = 0;
       Ctl.DskCount = 0;
       Ctl.ErrorCount = 0;
       Ctl.CrcOnly = FALSE;
       Ctl.CrcIgnore = FALSE;
       Ctl.CrcCount = 0;
       Ctl.HasLeader = TRUE;
       PrintLogInitialize();

       ProcessParms(argc - 1, &argv[1]);
}
