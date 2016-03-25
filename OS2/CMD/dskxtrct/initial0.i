//ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³ XTRCDSK - Initial0, program data
//³ Author:  Alan B. Arnett
//³ Copyright 1997, 1998 by Alan B. Arnett
//³ 16 Feb 98 - split initialize into local and common sections
//³ 04 Jul 98 - add crc options to parm table
//ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//³
//³ This file is part of the source code to dskxtrct and is distributed under
//³ the terms of the GNU General Public Licence.
//³
//ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

enum
{
	OptSource,
	OptTarget,
	OptLogReplace,
	OptLogAppend,
	OptLogSkip,
	OptLog,
	OptQuietMode,
	OptMsgLevel,
	OptCrcIgnore,
	OptCrcOnly,
	OptComment,
	OptViewList,
	OptHelp,
	OptReplaceAll,
	OptReplaceNone,
	OptCount
};

enum
{
	ParmString, ParmSwitch
};

struct
{
	int Type;
	char Str[8];
}
	OptEntry[OptCount] =
	{
		{ ParmString, "s" },
		{ ParmString, "t" },
		{ ParmSwitch, "lr" },
		{ ParmSwitch, "la" },
		{ ParmSwitch, "ls" },
		{ ParmString, "l" },
		{ ParmSwitch, "q" },
		{ ParmString, "m" },
		{ ParmSwitch, "crcx" },
		{ ParmSwitch, "crc" },
		{ ParmSwitch, "c" },
		{ ParmSwitch, "v" },
		{ ParmSwitch, "?" },
		{ ParmSwitch, "ra" },
		{ ParmSwitch, "rn" },
	};
