// MSG.C - Various text messages for 4xxx / TCMD
//   (put here to simplify language conversions)
//   Copyright (c) 1988 - 1998  Rex C. Conn   All rights reserved

#include <stdlib.h>

#include "product.h"
#include "4all.h"

// Global string variables
char ON[] = "ON";
char OFF[] = "OFF";
char WILD_FILE[] = "*";
char WILD_EXT[] = ".*";
char WILD_CHARS[] = "[?*";
char QUOTES[] = "`\"";
char QUOTES_PARENS[] = "`\"(";
char BACK_QUOTE[] = "`";
char NULLSTR[] = "";
char SLASHES[] = "\\/";
char WHITESPACE[] = " \t";

char SCAN_NOCR[] = "%[^\r]";
char FMT_STR[] = "%s";
char FMT_PREC_STR[] = "%.*s";
char FMT_LEFT_STR[] = "%-*s";
char FMT_EQUAL_STR[] = "=%s";
char FMT_TWO_EQUAL_STR[] = "%s=%s";
char FMT_STR_CRLF[] = "%s\n";
char FMT_DOUBLE_STR[] = "%s%s";
char FMT_PATH_STR[] = "%s\\%s";
char FMT_STR_TO_STR[] = "%s -> %s";
char FMT_FAR_STR[] = "%Fs";
char FMT_FAR_PREC_STR[] = "%.*Fs";
char FMT_FAR_LEFT_STR[] = "%-*Fs";
char FMT_FAR_STR_CRLF[] = "%Fs\n";
char FMT_CHAR[] = "%c";
char FMT_INT[] = "%d";
char FMT_UINT[] = "%u";
char FMT_LONG[] = "%ld";
char FMT_ULONG[] = "%lu";
char FMT_UINT_LEN[] = "%u%n";
char FMT_DISK[] = "%c:";
char FMT_ROOT[] = "%c:\\";
char FMT_PATH[] = "%c:\\%s";

// executable file extensions
char *executables[] = {
        ".com",
        ".exe",
        ".btm",         // in-memory batch file
        ".cmd",
        ".bat",
        NULL
};


// video type array for %_VIDEO
char *video_type[] = {
        "mono",
        "cga",
        "ega",          // monochrome ega
        "vga",
        "",
        "",
        "",
        "8514",         // IBM 8514 (1024 x 768)
        "IA/A",         // PS/2 Image Adapter/A
        "xga"           // PS/2 XGA
};


// current process type
char *gaPType[] = {
        "FS",
        "",
        "AVIO",
        "PM",
        "DT"
};


// tables for date formatting
char *daytbl[] = {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
};


char *montbl[] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
};


char *dateformat[] = {
        "mm-dd-yy",             // USA date format
        "dd-mm-yy",             // Europe
        "yy-mm-dd"              // Japan
};


// COLORDIR attribute sequences
COLORD colorize_atts[] = {
        "dirs",0x10,
        "rdonly",0x01,
        "hidden",0x02,
        "system",0x04,
        "archive",0x20
};


// ANSI color sequences (for COLOR)
ANSI_COLORS colors[] = {
        "Bla",30,
        "Blu",34,
        "Gre",32,
        "Cya",36,
        "Red",31,
        "Mag",35,
        "Yel",33,
        "Whi",37,
        "Bri Bla",0,
        "Bri Blu",0,
        "Bri Gre",0,
        "Bri Cya",0,
        "Bri Red",0,
        "Bri Mag",0,
        "Bri Yel",0,
        "Bri Whi",0
};



// 4xxx error messages
char *int_4dos_errors[] =
{
        "Syntax error",
        "Unknown command",
        "Command line too long",
        "No closing quote",
        "Can't open",
        "Can't create",
        "Can't delete",
        "Error reading",
        "Error writing",
        "Can't COPY or MOVE file to itself",
        "Insufficient disk space for",          // 10
        "Contents lost before copy",
        "Infinite COPY or MOVE loop",
        "Not an alias",
        "No aliases defined",
        "Alias loop",
        "Variable loop",
        "UNKNOWN_CMD loop",
        "Invalid date",
        "Invalid time",
        "Directory stack empty",                // 20
        "Can't get directory",
        "Label not found",
        "Out of environment space",
        "Out of alias space",
        "Not in environment",
        "4OS2 internal stack overflow",
        "Command only valid in batch file",
        "Batch file missing",
        "Exceeded batch nesting limit",
        "Missing ENDTEXT",                      // 30
        "Missing GOSUB",
        "Environment already saved",
        "Missing SETLOCAL",
        "Unbalanced parentheses",
        "No expression",                        // 35
        "Overflow",
        "File is empty",
        "Not a directory",
        "Clipboard is in use by another program",
        "Clipboard is empty or not text format",        // 40
        "Already excluded files",
        "Error in command-line directive",
        "Window title not found",
        "Not a windowed session",
        "Unknown process",
        "Listbox is full",
        "File association not found for extension",
        "JPOS2DLL.DLL not loaded",
        "Invalid key",                  // SENDKEYS error messages
        "Missing close paren",          // 50
        "Invalid count",
        "String too long",
        "Can't install hook",
};


// USAGE messages
char ACTIVATE_USAGE[] = "\"title\" [CLOSE | MIN | MAX | RESTORE]";
char ALIAS_USAGE[] = "[/PR ?] name=value";
char BREAK_USAGE[] = "[ON | OFF]";
char CALL_USAGE[] = "[/Q] ?";
char CDD_USAGE[] = "[/AS] ~";
char COLOR_USAGE[] = "# [BORDER bc]";
char COPY_USAGE[] = "[/A:-rhsda /CEHKMNPQRSTUVWXZ] ?...[/AB] ?[/AB]";
char DATE_USAGE[] = "[/T] [date]";
char DELETE_USAGE[] = "[/A:-rhsda /EFNPQSTWXYZ] ?...";
char DESCRIBE_USAGE[] = "?... [/D]";
char DETACH_USAGE[] = "command";
char DIR_USAGE[] = "[/A:-rhsda /O:-deginrsu /T:acw /124BDEFGHIJKLMNPRSTUVWZ] ?...";
char DO_USAGE[] = "[repetitor] [WHILE][UNTIL] ...] ... ENDDO";
char DRAWBOX_USAGE[] = "top left bottom right style # [FILL bg] [SHA] [ZOOM]";
char DRAWLINE_USAGE[] = "row col len style #";
char ESET_USAGE[] = "[/A] name...";
char EXCEPT_USAGE[] = "(?...) ...";
char FFIND_USAGE[] = "[/A:-rhsda /O:-acdeginrsu /D[a-z] /BCEIKLMNPRSV] [/T|X\"text\"] ?...";
char FOR_USAGE[] = "[/A[:-rhsda] /DFHLR] %var IN (set) DO ... [args]";
char GLOBAL_USAGE[] = "[/HIPQ] ...";
char HISTORY_USAGE[] = "[/AFP /R ?]";
char IF_USAGE[] = "[NOT] condition ...";
char IFF_USAGE[] = "[NOT] condition THEN & ... [ELSE[IFF] & ...] ENDIFF";
char INKEY_USAGE[] = "[/CDPX /K\"mask\" /Wn] [text] %%var";
char INPUT_USAGE[] = "[/CDENPX /Ln /Wn] [text] %%var";
char KEYBD_USAGE[] = "[/Cn /Nn /Sn]";
char KEYSTACK_USAGE[] = "[\"text\"] [n] [/Wn] [!]";
char KEYS_USAGE[] = "[ON | OFF | LIST]";
char LIST_USAGE[] = "[/A:-rhsda /T\"text\" /HIRSWX] ?...";
char LOG_USAGE[] = "[/H][ON | OFF | /W ?][text]";
char MD_USAGE[] = "[/NS] ~...";
char MOVE_USAGE[] = "[/A:-rhsda /CDEFHMNPQRSTUV] ?[... ?]";
char ON_USAGE[] = "[BREAK | ERROR | ERRORMSG] ...";
char POPD_USAGE[] = "[*]";
char PROCESS_USAGE[] = "[/K[pid][hwnd][title]]";
char RD_USAGE[] = "~...";
char REBOOT_USAGE[] = "[/SV]";
char RENAME_USAGE[] = "[/A:-rhsda /ENPQST] ?... ?";
char SCREEN_USAGE[] = "row col [text]";
char SCRPUT_USAGE[] = "row col # text";
char SELECT_USAGE[] = "[/A:-rhsda /DEHIJLTXZ /O:-deginrsu /T:acw] ... (?) ...";
char SET_USAGE[] = "[/APR ?] name=value";
char SHRALIAS_USAGE[] = "[/U]";
char TEE_USAGE[] = "[/A] ?...";
char TIME_USAGE[] = "[/T] [hh:mm:ss]";
char TIMER_USAGE[] = "[/123S]";
char TITLE_USAGE[] = "text";
char TOUCH_USAGE[] = "[/CEFQ /D[acw]date /T[acw]time] ?...";
char TREE_USAGE[] = "[/ABFHPS /T[acw]] dir...";
char TYPE_USAGE[] = "[/A:-rhsda /LP] ?...";
char UNALIAS_USAGE[] = "[/QR] name...";
char UNSET_USAGE[] = "[/QR] name...";
char WINDOW_USAGE[] = "MIN | MAX | POS=lx,ly,lcx,lcy | RESTORE | \"title\"";


// BATCH.C
char DEBUGGER_PROMPT[] =  "T(race) S(tep) J(ump) X(pand)  L(ist) V(ars) A(liases)  O(ff) Q(uit)";
char ENDTEXT[] = "ENDTEXT";
char ECHO_IS[] = "ECHO is %s\n";
char LOADBTM_IS[] = "LOADBTM is %s\n";
char PAUSE_PAGE_PROMPT[] = "Press ESC to quit or another key to continue...";
char PAUSE_PROMPT[] = "Press any key when ready...";
char DO_DO[] = "do";
char DO_BY[] = "by";
char DO_FOREVER[] = "forever";
char DO_LEAVE[] = "leave";
char DO_ITERATE[] = "iterate";
char DO_END[] = "enddo";
char DO_WHILE[] = "while";
char DO_UNTIL[] = "until";
char FOR_IN[] = "in";
char FOR_DO[] = "do";
char IF_NOT[] = "not";
char IF_OR[] = ".OR.";
char IF_XOR[] = ".XOR.";
char IF_AND[] = ".AND.";
char IF_DEFINED[] = "defined";
char IF_DIREXIST[] = "direxist";
char IF_EXIST[] = "exist";
char IF_ISDIR[] = "isdir";
char IF_ISINTERNAL[] = "isinternal";
char IF_ISALIAS[] = "isalias";
char IF_ISLABEL[] = "islabel";
char IF_ERRORLEVEL[] = "errorlevel";
char EQ[] = "EQ";
char GT[] = "GT";
char GE[] = "GE";
char LT[] = "LT";
char LE[] = "LE";
char NE[] = "NE";
char THEN[] = "then";
char IFF[] = "iff";
char ELSEIFF[] = "elseiff";
char ELSE[] = "else";
char ENDIFF[] = "endiff";
char INKEY_COMMAND[] = "INKEY";
char ON_BREAK[] = "break";
char ON_ERROR[] = "error";
char ON_ERRORMSG[] = "errormsg";


// DIRCMDS.C
char ONE_FILE[] = "file";
char MANY_FILES[] = "files";
char ONE_DIR[] = "dir";
char MANY_DIRS[] = "dirs";
char DIRECTORY_OF[] = " Directory of  %s";
char DIR_FILE_SIZE[] = "KMG";
char DIR_BYTES_IN_FILES[] = "%15Lq bytes in %Lu %s and %Lu %s";
char DIR_BYTES_ALLOCATED[] = "    %Lq bytes allocated";
char DIR_BYTES_FREE[] = "%15Lq bytes free";
char DIR_TOTAL[] = "    Total for:  %s";
char DIR_LABEL[] = " <DIR>   ";
char HPFS_DIR_LABEL[] = "        <DIR>  ";
char COLORDIR[] = "colordir";
char DESCRIBE_PROMPT[] = "Describe \"%s\" : ";


// xxxCALLS.C

char CLIP[] = "clip:";
char COMMAND_COM[] = "command.com";
// OS2CALLS.C
char CMD_EXE[] = "cmd.exe";
char EXTERN_BREAK[] = "External process cancelled by a Ctrl+Break or another process\r\n";
char HELP_EXE[] = "view.exe";
char OPTION_EXE[] = "option2.exe";

// xxxCMDS.C
char KBD_CAPS_LOCK[] = "Caps=%s\n";
char KBD_NUM_LOCK[] = "Num=%s\n";
char KBD_SCROLL_LOCK[] = "Scroll=%s\n";
char REBOOT_IT[] = "Confirm system reboot";
char TOTAL_ENVIRONMENT[] = "\n%15Lu bytes total environment\n";
char TOTAL_ALIAS[] = "\n%15Lu bytes total alias\n";
char TOTAL_HISTORY[] = "\n%15Lu bytes total history\n";
char TOTAL_DISK_USED[] = "%15Lq bytes total disk space\n%15Lq bytes used\n";

char START_TRANSIENT_STR[] = "c";
char START_DOS_STR[] = "dos";
char START_FS_STR[] = "fs";
char START_ICON_STR[] = "icon=";
char START_INV_STR[] = "inv";
char START_KEEP_STR[] = "k";
char START_MAX_STR[] = "max";
char START_MIN_STR[] = "min";
char START_NO_STR[] = "n";
char START_NOINHERIT_STR[] = "i";
char START_PGM_STR[] = "pgm";
char START_WAIT_STR[] = "wait";
char START_WIN_STR[] = "win";
char WIN_OS2[] = "WINOS2";
char START_BG_STR[] = "bg";
char START_FG_STR[] = "fg";
char START_PM_STR[] = "pm";
char START_WIN3_STR[] = "win3";

char START_LOCAL_STR[] = "l";
char START_LA_STR[] = "la";
char START_LD_STR[] = "ld";
char START_LH_STR[] = "lh";
char START_POS_STR[] = "pos=";

char PROCESS_ID_MSG[] = "The process ID is %u\n";
char NO_DPATH[] = "No DPATH";

char SHUTDOWN_COMPLETE[] = "Shutdown complete; turn the system off or press Ctrl-Alt-Del to reboot.\n";
char DOS_SYS[] = "DOS.SYS";
char TOTAL_OS2_PHYSICAL_RAM[] = "\n%15Lu bytes total physical RAM\n";
char TOTAL_OS2_RESIDENT_RAM[] = "%15Lu bytes total resident RAM\n";
char OS2_BYTES_FREE[] = "\n%15Lu bytes largest free block\n";
char OS2_SWAPNAME[64] = "C:\\OS2\\SYSTEM\\SWAPPER.DAT";
char OS2_SWAPFILE_SIZE[] = "\n%15Lu bytes total swap file\n";


// xxxINIT.C
// Embedded copyright notice
char COPYRIGHT2[] = "Built with Open Watcom C/C++ (http://www.openwatcom.org)\n";
char COMSPEC[] = "COMSPEC";
char TAILIS[] = "%s tail is:  [%s]\nPress any key to continue ...";

char COMSPEC_OS2[] = "%s=%Fs";
char szIniName[MAXFILENAME];

char PROGRAM[32];
char SHAREMEM_NAME[] = "\\SHAREMEM\\4OS2%03x%c";
char SHAREMEM_PIPE_ENV[] = "4OS2PIPE=";
//char INI_SECTION_NAME[] = "4OS2";
char NAME_HEADER[] = "4OS2";
char SET_PROGRAM[] = "4OS2 %u%c%02u%s";
char *OS2_NAME = "4OS2.EXE";
char *SHORT_NAME = "4OS2";
char OS2_INI[] = "4OS2.INI";
char OS2_FS[] = "OS/2 Full Screen";
char OS2_WIN[] = "OS/2 Window";
char NO_DLL[] = "Warning:  Can't load JPOS2DLL";


// ENV.C
char SET_COMMAND[] = "SET";
char UNSET_COMMAND[] = "UNSET";
char BEGINLIBPATH[] = "BeginLIBPATH";
char ENDLIBPATH[] = "EndLIBPATH";
char SEMAPHORE_NAME[] = "\\SEM32\\4OS2\\ALIAS.SEM";


// ERROR.C
char USAGE_MSG[] = "Usage : %s ";
char FILE_SPEC[] = "[d:][path]name";
char PATH_SPEC[] = "[d:]pathname";
char COLOR_SPEC[] = "[BRI][BLI] fg ON [BRI] bg";


// EXPAND.C
char KEYS_LIST[] = "list";
char PATH_VAR[] = "PATH";
char PATHEXT[] = "PATHEXT";
char MONO_MONITOR[] = "mono";
char COLOR_MONITOR[] = "color";
char END_OF_FILE_STR[] = "**EOF**";
char OPEN_READ[] = "read";
char OPEN_WRITE[] = "write";
char OPEN_APPEND[] = "append";

char *ACList[] = {
        "off-line",
        "on-line",
        "unknown"
};

char *BatteryList[] = {
        "high",
        "low",
        "critical",
        "charging",
        "unknown"
};

char *VAR_ARRAY[] = {
        "4ver",                 // 4DOS / 4OS2 version
        "?",                    // return code of previous internal command
        "alias",                // free alias space
        "ansi",                 // ANSI driver loaded
        "apmac",                // APM AC line status
        "apmbatt",              // APM battery status
        "apmlife",              // APM remaining battery life
        "batch",                // batch nesting level
        "batchline",            // line # in current batch file
        "batchname",            // name of current batch file
        "bg",                   // background color at cursor
        "boot",                 // boot drive
        "ci",                   // insert cursor shape
        "co",                   // overstrike cursor shape
        "codepage",             // active codepage
        "column",               // current column position
        "columns",              // # of columns on active display
        "country",              // country code
        "cpu",                  // cpu type
        "cwd",                  // current working directory
        "cwds",                 // current working directory with trailing '\'
        "cwp",                  // current working path
        "cwps",                 // current working path with trailing '\'
        "date",                 // current date
        "day",                  // current day
        "disk",                 // current disk
        "dname",                // description file name
        "dos",                  // DOS type (DOS or OS2)
        "dosver",               // DOS version #
        "dow",                  // day of week
        "dowi",
        "doy",                  // day of year (1 - 366)
        "dpmi",                 // DPMI version
        "dv",                   // DESQview loaded flag
        "env",                  // free environment space
        "fg",                   // foreground color at cursor
        "hlogfile",             // name of current history log file
        "hour",                 // current hour
        "kbhit",                // != 0 if key is waiting
        "kstack",               // != 0 if KSTACK is loaded
        "lastdisk",             // last disk in use
        "logfile",              // name of current log file
        "minute",               // current minute
        "monitor",              // monitor type (mono or color)
        "month",                // current month
        "mouse",                // mouse loaded (0 or 1)
        "ndp",                  // math coprocessor type
        "pid",                  // process ID (NT & OS2)
        "pipe",                 // currently in a pipe (0 or 1)
        "row",                  // current row
        "rows",                 // # of rows on active display
        "second",               // current second
        "selected",             // selected text
        "shell",                // shell level (0 - 99)
        "swapping",             // current 4DOS swapping mode
        "syserr",               // last system error #
        "time",                 // current time
        "transient",            // transient or resident shell
        "video",                // video type (cga, mono, ega, vga)
        "win",                  // Windows loaded flag
        "wintitle",             // window title
        "year",                 // current year

        "cmdproc",              // executable name
        "xpixels",              // x screen size
        "ypixels",              // y screen size
        "ppid",                 // parent process ID
        "sid",                  // session ID
        "ptype",                // process type
        NULL
};

char *FUNC_ARRAY[] = {
        "alias",
        "altname",
        "ascii",
        "attrib",
        "cdrom",
        "char",
        "clip",                 // clipboard paste
        "comma",
        "convert",
        "date",
        "day",
        "dec",
        "descript",
        "device",
        "diskfree",
        "disktotal",
        "diskused",
        "dosmem",
        "dow",
        "dowi",
        "doy",                  // day of year (1-366)
        "ems",
        "eval",
        "exec",
        "execstr",
        "expand",
        "ext",
        "extended",
        "fileage",
        "fileclose",
        "filedate",
        "filename",
        "fileopen",
        "fileread",
        "files",
        "fileseek",
        "fileseekl",
        "filesize",
        "filetime",
        "filewrite",
        "filewriteb",
        "findclose",
        "findfirst",
        "findnext",
        "format",
        "full",
        "getdir",
        "getfile",
        "if",
        "inc",
        "index",
        "iniread",
        "iniwrite",
        "insert",
        "instr",
        "int",
        "label",
        "left",
        "len",
        "lfn",
        "line",
        "lines",
        "lower",
        "lpt",
        "makeage",
        "makedate",
        "maketime",
        "master",
        "month",
        "name",
        "numeric",
        "path",
        "random",
        "readscr",
        "ready",
        "remote",
        "removable",
        "repeat",
        "replace",
        "rexx",
        "right",
        "search",
        "select",
        "sfn",
        "strip",
        "substr",
        "time",
        "timer",
        "trim",
        "truename",
        "unique",
        "upper",
        "wild",
        "word",
        "words",
        "xms",
        "year",

        "exetype",
        "fstype",
        "earead",               // read an EA (ASCII)
        "eawrite",              // write an EA (ASCII)
        NULL
};


// FILECMDS.C
char REPLACE[] = " (Replace)";
char FILES_COPIED[] = "%6Lu %s copied\n";
char MOVE_CREATE_DIR[] = "Create \"%s\"";
char FILES_MOVED[] = "%6Lu %s moved";
char FILES_RENAMED[] = "%6Lu %s renamed\n";
char FILES_DELETED[] = "%6Lu %s deleted";
char FILES_BYTES_FREED[] = " %15Lq bytes freed";
char ARE_YOU_SURE[] = "Are you sure";
char DELETE_QUERY[] = "Delete ";
char DELETING_FILE[] = "Deleting %s\n";

// INIPARSE.C
// char INI_QUERY[] = "  (Y/N/Q/R/E) ? ";
char INI_ERROR[] = "Error on line %d of %s:\n  %s \"%s\"\n";
char INI_QUERY[] = "  (Y/N/Q/R/E) ? ";


// LINES.C
char BOX_FILL[] = "fil";
char BOX_SHADOW[] = "sha";
char BOX_ZOOM[] = "zoo";


// LIST.C
char FFIND_OFFSET[] = "Offset: %Lu  (%lxh)";
char FFIND_TEXT_FOUND[] = "  %Lu %s in";
char FFIND_ONE_LINE[] = "line";
char FFIND_MANY_LINES[] = "lines";
char FFIND_FOUND[] = " %6Lu %s";
char FFIND_RESULT[] = "Found:";
char LIST_STDIN_MSG[] = "STDIN";
char LIST_GOTO[] = "Line: ";
char LIST_GOTO_OFFSET[] = "Hex Offset: ";
char LIST_INFO_PIPE[] = "LIST is displaying the output of a pipe.";

char LIST_INFO_FAT[] = "File:  %s\nDesc:  %.60s\nSize:  %Ld bytes\nDate:  %s\nTime:  %02u%c%02u\n";
char LIST_INFO_HPFS[] = "File:         %s\nDescription:  %.50s\nSize:         %Ld bytes\nLast Write:   %-8s  %02u%c%02u\nLast Access:  %-8s  %02u%c%02u\nCreated:      %-8s  %02u%c%02u";

char LIST_HEADER[] = " %-12.12s %c F1 Help %c Commands: BFGHINPWX %c";
char LIST_LINE[] = "Col %-3d  Line %-9Lu%3d%%";
char LIST_WAIT[] = "WAIT";
char LIST_GOTO_TITLE[] = " Goto Line / Offset ";
char LIST_FIND[] = "Find: ";
char LIST_FIND_WAIT[] = "Finding \"%.64s\"";
char LIST_FIND_TITLE[] = " Find Text ";
char LIST_FIND_TITLE_REVERSE[] = " Find Text (Reverse) ";
char LIST_FIND_HEX[] = "Hex search (Y/N)? ";
char LIST_NOT_FOUND[] = "Not found--press a key to continue ";
char LIST_PRINT_TITLE[] = " Print ";
char LIST_SAVE_TITLE[] = " Save ";
char LIST_QUERY_PRINT[] = "Print File or Page (%c/%c)? ";
char LIST_QUERY_SAVE[] = "Save File to: ";
char LIST_PRINTING[] = "Printing--press ESC to quit ";


// MISC.C
char DESCRIPTION_FILE[80] = "DESCRIPT.ION";
char DESCRIPTION_SCAN[] = "%*[ ,\t]%511[^\r\n\004\032]";
char SUBJECT_EA[] = ".SUBJECT";
char LONGNAME_EA[] = ".LONGNAME";
char CONSOLE[] = "CON";
char DEV_CONSOLE[] = "\\DEV\\CON";
char YES_NO[] = "Y/N";
char YES_NO_REST[] = "Y/N/R";
char BRIGHT[] = "Bri ";
char BLINK[] = "Bli ";
char BORDER[] = "Bor";


// PARSER.C
char CANCEL_BATCH_JOB[] = "\nCancel batch job %s ? (Y/N/A) : ";
char INSERT_DISK[] = "\nInsert disk with \"%s\"\nPress any key when ready...";
char UNKNOWN_COMMAND[] = "UNKNOWN_CMD";
char COMMAND_GROUP_MORE[] = "More? ";
char CMDLINE_VAR[] = "CMDLINE=";
char PROMPT_NAME[] = "PROMPT";

char OS2_PROMPT[] = "  OS/2     Ctrl+Esc = Window List                             Type HELP = help  ";


// SCREENIO.C
char HISTORY_TITLE[] = " History ";
char DIRECTORY_TITLE[] = " Directories ";
char FILENAMES_TITLE[] = " Filenames ";


// SELECT.C
char SELECT_HEADER[] = "     chars %c Cursor keys select %c +Mark  -Clear %c ENTER to run %c";
char MARKED_FILES[] = "  Marked: %4u files %6LuK";
char SELECT_PAGE_COUNT[] = "Page %2u of %2u";


// SYSCMDS.C
char BUILDING_INDEX[] = "Indexing drive %c:\n";
char BUILDING_UNC_INDEX[] = "Indexing %s\n";
char CDPATH[] = "_cdpath";
char UNLABELED[] = "unlabeled";
char VOLUME_LABEL[] = " Volume in drive %c is %-12s";
char VOLUME_SERIAL[] = "   Serial number is %04lx:%04lx";
char NO_PATH[] = "No PATH";
char BYTES_FREE[] = "%15Lq bytes free\n";
char LBYTES_FREE[] = "%15Lu bytes free\n";
char CODE_PAGE[] = "Active code page: %u\n";
char LOG_IS[] = "LOG (%s) is %s\n";
char BREAK_IS[] = "%s is %s\n";
char PREPARED_CODE_PAGES[] = "Prepared code pages: ";

char SETDOS_IS[] = "BRIGHTBG=%d\nCOMPOUND=%c\nDESCRIPTIONS=%u  (%s)\nESCAPE=%c\nEVAL=%d%c%d\nEXPANSION=%s\nINPUT=%u\nMODE=%u\nNOCLOBBER=%u\nPARAMETERS=%c\nROWS=%u\nCURSOR OVERSTRIKE=%u\nCURSOR INSERT=%u\nUPPER CASE=%u\nVERBOSE=%u\nSINGLESTEP=%u\n";
char LOG_FILENAME[] = "4OS2LOG";
char HLOG_FILENAME[] = "4OS2HLOG";

char GLOBAL_DIR[] = "\nGLOBAL: %s";
char TIMER_NUMBER[] = "Timer %d ";
char TIMER_ON[] = "on: %s\n";
char TIMER_OFF[] = "off: %s ";
char TIMER_SPLIT[] = "%u%c%02u%c%02u%c%02u";
char TIMER_ELAPSED[] = " Elapsed: %s\n";
char TIME_FMT[] = "%2u%c%02u%c%02u%c";
char DATE_FMT[] = "%u%*1s%u%*1s%u";
char NEW_DATE[] = "\nNew date (%s): ";
char NEW_TIME[] = "\nNew time (hh:mm:ss): ";
