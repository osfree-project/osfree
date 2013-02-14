//ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³ 16 Feb 98 - move PrintHelp init functions to local code
//³           - add log file processing
//³           - add 'quiet' option
//ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³
//³ This file is part of the source code to dskxtrct and is distributed under
//³ the terms of the GNU General Public Licence.
//³
//ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#ifndef PRINTMSG
#define PRINTMSG

#define INCL_DOSFILEMGR			/* File Manager values */
#define INCL_DOSERRORS			/* DOS error values    */
#include <os2.h>

#ifdef __GNUC__
#define stricmp strcasecmp
#include <sys/stat.h> 
#include <errno.h>
#endif

#include "stringe.h"
#include <iostream.h>
#include <stream.h>
#include <stdlib.h>
#include <stdio.h>
/* #include <io.h> */
/* #include <fcntl.h> */
/* #include <sys\stat.h> */

#ifndef MAXSTRLEN
#define MAXSTRLEN 2048
#endif

#ifndef Exit_Fail
#define Exit_Fail 16
#endif
#ifndef Exit_Error
#define Exit_Error 8
#endif
#ifndef Exit_Warning
#define Exit_Warning 4
#endif
#ifndef Exit_Normal
#define Exit_Normal 0
#endif

enum
{
	MsgMust, MsgFail, MsgError, MsgWarning, MsgInfo, MsgStd,
	MsgFile, MsgDebug, MsgDebug2, MsgCount
};
enum
{
	MsgLvlNone, MsgLvlStd, MsgLvlFiles, MsgLvlDebug, MsgLvlDebug2, MsgLvlCount
};
struct
{
	int Level;
	char Str[8];
} MsgLvl[MsgLvlCount] =
{
	{ MsgWarning, "none" },
	{ MsgStd, "std" },
	{ MsgFile, "files" },
	{ MsgDebug, "debug" },
	{ MsgDebug2, "debug2" },
};
struct
{
	int Level;
	int Code;
} MsgRC[MsgCount] =
{
	{ MsgMust, Exit_Normal },
	{ MsgFail, Exit_Fail },
	{ MsgError, Exit_Error },
	{ MsgWarning, Exit_Warning },
	{ MsgInfo, Exit_Normal },
	{ MsgStd, Exit_Normal },
	{ MsgFile, Exit_Normal },
	{ MsgDebug, Exit_Normal },
	{ MsgDebug2, Exit_Normal },
};

void PrintExit(int rc)
{
	if (Ctl.LogFile != NULL)
		fclose(Ctl.LogFile);
	exit(rc);
}
void PrintLogClose()
{
	if (Ctl.LogFile != NULL)
		if (fclose(Ctl.LogFile) < 0)
		{
			char xMsg[MAXSTRLEN];

			sprintf(xMsg, "Error closing log file: %s", Ctl.Log);
			cerr << xMsg << endl;
		}
}
enum
{
	PrintLogNULL, PrintLogReplace, PrintLogAppend, PrintLogSkip,
	PrintLogFail
};

int PrintLogFileReplacePrompt(char *FileName)
{
	char ReplyStr[MAXSTRLEN];

	while (Ctl.LogReplace == PrintLogNULL)
	{
		cout << "Replace log file: " << FileName << "?" << endl;
		cout << "   Select: (r)eplace, (a)ppend, (s)kip logging, (f)ail" << endl;
		cin >> ReplyStr;
		if (stricmp(ReplyStr, "r") == 0)
			Ctl.LogReplace = PrintLogReplace;
		if (stricmp(ReplyStr, "a") == 0)
			Ctl.LogReplace = PrintLogAppend;
		if (stricmp(ReplyStr, "s") == 0)
			Ctl.LogReplace = PrintLogSkip;
		if (stricmp(ReplyStr, "f") == 0)
		{
			cerr << "Processing aborted by request" << endl;
			exit(16);
		}
	}
	return (Ctl.LogReplace);
}
int PrintLogCheckFileAccess(char *FileName)
{
	if (access(FileName, 0) == -1)
		return (PrintLogReplace);
	if (Ctl.LogReplace != PrintLogNULL)
		return (Ctl.LogReplace);
	return (PrintLogFileReplacePrompt(FileName));
}
void PrintLogFileOpen()
{
	char xMsg[MAXSTRLEN];

	if (strlen(Ctl.Log) == 0)
		Ctl.LogReplace = PrintLogSkip;
	if (Ctl.LogReplace == PrintLogSkip)
		return;
	PrintLogCheckFileAccess(Ctl.Log);
	if (Ctl.LogReplace == PrintLogSkip || Ctl.LogReplace == PrintLogFail)
		return;

	char AccessType[10];

	if (Ctl.LogReplace == PrintLogReplace)
		strcpy(AccessType, "w");
	else
		strcpy(AccessType, "a");
	if (NULL == (Ctl.LogFile = fopen(Ctl.Log, AccessType)))
	{
		sprintf(xMsg, "Unable to open log file: '%s'", Ctl.Log);
		cerr << xMsg << endl;
		sprintf(xMsg, "Error: %s", strerror(errno));
		cerr << xMsg << endl;
		exit(MsgRC[MsgFail].Code);
	}
}
void PrintMsg(int MsgLevel, char *Msg)
{
	if (MsgRC[MsgLevel].Code > Ctl.RCHigh)
		Ctl.RCHigh = MsgRC[MsgLevel].Code;
	if (MsgLevel <= Ctl.MsgLevel)
	{
		char xMsg[MAXSTRLEN];

		strcpy(xMsg, Msg);
		strcat(xMsg, "\n");
		if (Ctl.LogFile != NULL)
			if (fputs(xMsg, Ctl.LogFile) == EOF)
			{
				cerr << "Error writing log file: " << Ctl.Log << endl;
				exit(MsgRC[MsgFail].Code);
			}
		if (!Ctl.QuietMode || Ctl.LogFile == NULL)
			cout << Msg << endl;
	}
	if (Ctl.RCHigh >= MsgRC[MsgFail].Code)
	{
		cout << "Exiting due to fatal error" << endl;
		PrintExit(Ctl.RCHigh);
	}
}
void ErrorExit(char *msg)
{
	char xMsg[MAXSTRLEN];

	sprintf(xMsg, "%s: %s", msg, strerror(errno));
	PrintMsg(MsgFail, xMsg);
	PrintExit(MsgRC[MsgFail].Code);
}
void ErrorExit1(char *msg, char *str)
{
	char xMsg[MAXSTRLEN];

	sprintf(xMsg, msg, str);
	ErrorExit(xMsg);
}
void PrintPgmTitle()
{
	char xMsg[MAXSTRLEN];

	sprintf(xMsg, "%s - %s", PgmTitle, PgmVer);
	PrintMsg(MsgMust, xMsg);
	PrintMsg(MsgMust, PgmCopyright);
	PrintMsg(MsgMust, "");
}
void PrintLogInitialize()
{
	char work[MAXSTRLEN];

	strcpy(work, PgmTitle);
	strcat(work, ".log");
	strtolower(work, work);
	Ctl.Log = strdup(work);
	Ctl.LogReplace = PrintLogNULL;
	Ctl.LogFile = NULL;
}
void PrintLogOptLogReplace()
{
	Ctl.LogReplace = PrintLogReplace;
	PrintMsg(MsgDebug, "Log File replace option specified");
}
void PrintLogOptLogAppend()
{
	Ctl.LogReplace = PrintLogAppend;
	PrintMsg(MsgDebug, "Log File append option specified");
}
void PrintLogOptLogSkip()
{
	Ctl.LogReplace = PrintLogSkip;
	PrintMsg(MsgDebug, "Log File skip option specified");
}

#endif
