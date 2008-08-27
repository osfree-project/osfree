// MESSAGE.H - Various text messages for 4xxx / TCMD
//   (put here to simplify language conversions)
//   Copyright (c) 1988 - 1998  Rex C. Conn   All rights reserved

#define SINGLE_QUOTE '`'                // quote characters
#define DOUBLE_QUOTE '"'

#define YES_CHAR 'Y'
#define NO_CHAR 'N'
#define ALL_CHAR 'A'
#define REST_CHAR 'R'

#define INI_QUIT_CHAR 'Q'
#define INI_EDIT_CHAR 'E'

#define LIST_CONTINUE_CHAR 'C'
#define LIST_PREVIOUS_CHAR 'B'
#define LIST_EXIT_CHAR 'Q'
#define LIST_FIND_CHAR 'F'
#define LIST_FIND_CHAR_REVERSE 6
#define LIST_GOTO_CHAR 'G'
#define LIST_HIBIT_CHAR 'H'
#define LIST_EDITOR_CHAR 'E'
#define LIST_SAVEFILE_CHAR 'S'
#define LIST_INFO_CHAR 'I'
#define LIST_FIND_NEXT_CHAR 'N'
#define LIST_FIND_NEXT_CHAR_REVERSE 14
#define LIST_OPEN_CHAR 'O'
#define LIST_PRINT_CHAR 'P'
#define LIST_WRAP_CHAR 'W'
#define LIST_HEX_CHAR 'X'
#define LIST_PRINT_FILE_CHAR 'F'
#define LIST_PRINT_PAGE_CHAR 'P'

#define SELECT_LIST 12

#include "shralias.h"

extern char ON[];
extern char OFF[];
extern char WILD_FILE[];
extern char WILD_EXT[];
extern char WILD_CHARS[];
extern char QUOTES[];
extern char QUOTES_PARENS[];
extern char BACK_QUOTE[];
extern char NULLSTR[];
extern char SLASHES[];
extern char WHITESPACE[];

extern char SCAN_NOCR[];
extern char FMT_STR[];
extern char FMT_PREC_STR[];
extern char FMT_LEFT_STR[];
extern char FMT_EQUAL_STR[];
extern char FMT_TWO_EQUAL_STR[];
extern char FMT_DOUBLE_STR[];
extern char FMT_PATH_STR[];
extern char FMT_STR_CRLF[];
extern char FMT_STR_TO_STR[];
extern char FMT_FAR_STR[];
extern char FMT_FAR_PREC_STR[];
extern char FMT_FAR_LEFT_STR[];
extern char FMT_FAR_STR_CRLF[];
extern char FMT_CHAR[];
extern char FMT_INT[];
extern char FMT_UINT[];
extern char FMT_LONG[];
extern char FMT_ULONG[];
extern char FMT_HEX[];
extern char FMT_UINT_LEN[];
extern char FMT_DISK[];
extern char FMT_ROOT[];
extern char FMT_PATH[];

extern char *executables[];

#define COM executables[0]
#define EXE executables[1]
#define BTM executables[2]
#define CMD executables[3]
#define BAT executables[4]

extern char *video_type[];
extern char *gaPType[];
extern char *daytbl[];
extern char *montbl[];
extern char *dateformat[];
extern COLORD colorize_atts[];
extern ANSI_COLORS colors[];
extern char *int_4dos_errors[];

// USAGE messages
extern char ACTIVATE_USAGE[];
extern char ALIAS_USAGE[];
extern char ASSOC_USAGE[];
extern char ATTRIB_USAGE[];
extern char BREAK_USAGE[];
extern char CALL_USAGE[];
extern char CDD_USAGE[];
extern char COLOR_USAGE[];
extern char COPY_USAGE[];
extern char DATE_USAGE[];
extern char DELETE_USAGE[];
extern char DESCRIBE_USAGE[];
extern char DETACH_USAGE[];
extern char DIR_USAGE[];
extern char DO_USAGE[];
extern char DRAWBOX_USAGE[];
extern char DRAWLINE_USAGE[];
extern char ESET_USAGE[];
extern char EXCEPT_USAGE[];
extern char FFIND_USAGE[];
extern char FOR_USAGE[];
extern char GLOBAL_USAGE[];
extern char HISTORY_USAGE[];
extern char IF_USAGE[];
extern char IFF_USAGE[];
extern char INKEY_USAGE[];
extern char INPUT_USAGE[];
extern char KEYBD_USAGE[];
extern char KEYSTACK_USAGE[];
extern char KEYS_USAGE[];
extern char LIST_USAGE[];
extern char LOG_USAGE[];
#define LOADBTM_USAGE BREAK_USAGE
extern char MD_USAGE[];
extern char MOVE_USAGE[];
extern char ON_USAGE[];
extern char POPD_USAGE[];
extern char PROCESS_USAGE[];
extern char RD_USAGE[];
extern char REBOOT_USAGE[];
extern char RENAME_USAGE[];
extern char SCREEN_USAGE[];
extern char SCRPUT_USAGE[];
extern char SELECT_USAGE[];
extern char SET_USAGE[];
#define SWAPPING_USAGE BREAK_USAGE
extern char SHRALIAS_USAGE[];
extern char TEE_USAGE[];
extern char TIME_USAGE[];
extern char TIMER_USAGE[];
extern char TITLE_USAGE[];
extern char TOUCH_USAGE[];
extern char TREE_USAGE[];
extern char TYPE_USAGE[];
extern char UNALIAS_USAGE[];
extern char UNSET_USAGE[];
extern char VER_USAGE[];
#define VERIFY_USAGE BREAK_USAGE
extern char WINDOW_USAGE[];


// BATCH.C
extern char DEBUGGER_PROMPT[];
extern char ENDTEXT[];
extern char ECHO_IS[];
extern char LOADBTM_IS[];
extern char PAUSE_PAGE_PROMPT[];
extern char PAUSE_PROMPT[];
extern char DO_DO[];
extern char DO_BY[];
extern char DO_FOREVER[];
extern char DO_LEAVE[];
extern char DO_ITERATE[];
extern char DO_END[];
extern char DO_WHILE[];
extern char DO_UNTIL[];
extern char FOR_IN[];
extern char FOR_DO[];
extern char IF_NOT[];
extern char IF_OR[];
extern char IF_XOR[];
extern char IF_AND[];
extern char IF_DEFINED[];
extern char IF_DIREXIST[];
extern char IF_EXIST[];
extern char IF_ISDIR[];
extern char IF_ISINTERNAL[];
extern char IF_ISALIAS[];
extern char IF_ISLABEL[];
extern char IF_ERRORLEVEL[];
extern char EQ[];
extern char GT[];
extern char GE[];
extern char LT[];
extern char LE[];
extern char NE[];
extern char THEN[];
extern char IFF[];
extern char ELSEIFF[];
extern char ELSE[];
extern char ENDIFF[];
extern char INKEY_COMMAND[];
extern char ON_BREAK[];
extern char ON_ERROR[];
extern char ON_ERRORMSG[];


// DIRCMDS.C
extern char ONE_FILE[];
extern char MANY_FILES[];
extern char ONE_DIR[];
extern char MANY_DIRS[];
extern char DIRECTORY_OF[];
// extern char DIRECTORY_DESCRIPTION[];
extern char DIR_FILE_SIZE[];
extern char DIR_BYTES_IN_FILES[];
extern char DIR_BYTES_ALLOCATED[];
extern char DIR_BYTES_FREE[];
extern char DIR_TOTAL[];
extern char DIR_LABEL[];
extern char HPFS_DIR_LABEL[];
extern char COLORDIR[];
extern char DESCRIBE_PROMPT[];


// xxxCALLS.C
extern char CLIP[];
extern char COMMAND_COM[];
extern char HELP_EXE[];
extern char CMD_EXE[];
extern char EXTERN_BREAK[];
extern char OPTION_EXE[];


// xxxCMDS.C
extern char KBD_CAPS_LOCK[];
extern char KBD_NUM_LOCK[];
extern char KBD_SCROLL_LOCK[];
extern char REBOOT_IT[];

extern char START_TRANSIENT_STR[];
extern char START_DOS_STR[];
extern char START_FS_STR[];
extern char START_ICON_STR[];
extern char START_INV_STR[];
extern char START_KEEP_STR[];
extern char START_MAX_STR[];
extern char START_MIN_STR[];
extern char START_NO_STR[];
extern char START_PGM_STR[];
extern char START_WIN_STR[];
extern char WIN_OS2[];
extern char START_BG_STR[];
extern char START_FG_STR[];
extern char START_PM_STR[];
extern char START_TTY[];
extern char START_WIN3_STR[];

extern char START_LOCAL_STR[];
extern char START_LA_STR[];
extern char START_LD_STR[];
extern char START_LH_STR[];
extern char START_POS_STR[];

extern char START_CAVEMAN[];
extern char START_NOINHERIT_STR[];
extern char START_WAIT_STR[];

extern char PROCESS_ID_MSG[];
extern char NO_DPATH[];

extern char SHUTDOWN_COMPLETE[];
extern char DOS_SYS[];
extern char SHUTDOWN_OS2_QUERY[];
extern char TOTAL_OS2_PHYSICAL_RAM[];
extern char TOTAL_OS2_RESIDENT_RAM[];
extern char OS2_BYTES_FREE[];
extern char OS2_SWAPNAME[];
extern char OS2_SWAPFILE_SIZE[];
// extern char OS2_SPOOLNAME[];
// extern char OS2_SPOOLFILE_SIZE[];
extern char TOTAL_ENVIRONMENT[];
extern char TOTAL_ALIAS[];
extern char TOTAL_HISTORY[];
extern char TOTAL_DISK_USED[];


// xxxINIT.C
extern char NAME_HEADER[];
extern char COPYRIGHT[];
extern char COPYRIGHT2[];
extern char COMSPEC[];
extern char TAILIS[];

extern char PROGRAM[];
extern char SET_PROGRAM[];
extern char *SHORT_NAME;
extern char INI_SECTION_NAME[];

extern char SHAREMEM_NAME[];
extern char SHAREMEM_PIPE_ENV[];
extern char szIniName[];
// extern char NPIPE_NAME[];
extern char *OS2_NAME;
extern char OS2_INI[];
extern char OS2_FS[];
extern char OS2_WIN[];
extern char COMSPEC_OS2[];
extern char NO_DLL[];
extern char OS2_TITLE[];


// ENV.C
extern char SET_COMMAND[];
extern char UNSET_COMMAND[];
extern char SEMAPHORE_NAME[];
extern char BEGINLIBPATH[];
extern char ENDLIBPATH[];


// ERROR.C
extern char USAGE_MSG[];
extern char FILE_SPEC[];
extern char PATH_SPEC[];
extern char COLOR_SPEC[];


// EXPAND.C
extern char PATH_VAR[];
extern char PATHEXT[];
extern char MONO_MONITOR[];
extern char COLOR_MONITOR[];
extern char END_OF_FILE_STR[];
extern char OPEN_READ[];
extern char OPEN_WRITE[];
extern char OPEN_APPEND[];

extern char *ACList[];
extern char *BatteryList[];

extern char KEYS_LIST[];

// defines for internal variable array
#define VAR_4VER 0
#define VAR_IERRORLEVEL 1
#define VAR_ALIAS 2
#define VAR_ANSI 3
#define VAR_APMAC 4
#define VAR_APMBATT 5
#define VAR_APMLIFE 6
#define VAR_BATCH 7
#define VAR_BATCHLINE 8
#define VAR_BATCHNAME 9
#define VAR_BG_COLOR 10
#define VAR_BOOT 11
#define VAR_CI 12
#define VAR_CO 13
#define VAR_CODEPAGE 14
#define VAR_COLUMN 15
#define VAR_COLUMNS 16
#define VAR_COUNTRY 17
#define VAR_CPU 18
#define VAR_CWD 19
#define VAR_CWDS 20
#define VAR_CWP 21
#define VAR_CWPS 22
#define VAR_DATE 23
#define VAR_DAY 24
#define VAR_DISK 25
#define VAR_DNAME 26
#define VAR_DOS 27
#define VAR_DOSVER 28
#define VAR_DOW 29
#define VAR_DOWI 30
#define VAR_DOY 31
#define VAR_DPMI 32
#define VAR_DV 33
#define VAR_ENVIRONMENT 34
#define VAR_FG_COLOR 35
#define VAR_HLOGFILE 36
#define VAR_HOUR 37
#define VAR_KBHIT 38
#define VAR_KSTACK 39
#define VAR_LASTDISK 40
#define VAR_LOGFILE 41
#define VAR_MINUTE 42
#define VAR_MONITOR 43
#define VAR_MONTH 44
#define VAR_MOUSE 45
#define VAR_NDP 46
#define VAR_PID 47              // process ID
#define VAR_PIPE 48
#define VAR_ROW 49
#define VAR_ROWS 50
#define VAR_SECOND 51
#define VAR_SELECTED 52
#define VAR_SHELL 53
#define VAR_SWAPPING 54
#define VAR_SYSERR 55
#define VAR_TIME 56
#define VAR_TRANSIENT 57
#define VAR_VIDEO 58
#define VAR_WIN 59
#define VAR_WINTITLE 60
#define VAR_YEAR 61
#define VAR_CMDPROC 62
#define VAR_XPIXELS 63
#define VAR_YPIXELS 64
#define VAR_OS2_PPID 65         // parent process ID
#define VAR_OS2_SID 66          // session ID
#define VAR_OS2_PTYPE 67        // process type

// defines for variable function array
#define FUNC_ALIAS 0
#define FUNC_ALTNAME 1
#define FUNC_ASCII 2
#define FUNC_ATTRIB 3
#define FUNC_CDROM 4
#define FUNC_CHAR 5
#define FUNC_CLIP 6
#define FUNC_COMMA 7
#define FUNC_CONVERT 8
#define FUNC_DATE 9
#define FUNC_DAY 10
#define FUNC_DEC 11
#define FUNC_DESCRIPT 12
#define FUNC_DEVICE 13
#define FUNC_DISKFREE 14
#define FUNC_DISKTOTAL 15
#define FUNC_DISKUSED 16
#define FUNC_DOSMEM 17
#define FUNC_DOW 18
#define FUNC_DOWI 19
#define FUNC_DOY 20
#define FUNC_EMS 21
#define FUNC_EVAL 22
#define FUNC_EXECUTE 23
#define FUNC_EXECSTR 24
#define FUNC_EXPAND 25
#define FUNC_EXTENSION 26
#define FUNC_EXTENDED 27
#define FUNC_FILEAGE 28
#define FUNC_FILECLOSE 29
#define FUNC_FILEDATE 30
#define FUNC_FILENAME 31
#define FUNC_FILEOPEN 32
#define FUNC_FILEREAD 33
#define FUNC_FILES 34
#define FUNC_FILESEEK 35
#define FUNC_FILESEEKL 36
#define FUNC_FILESIZE 37
#define FUNC_FILETIME 38
#define FUNC_FILEWRITE 39
#define FUNC_FILEWRITEB 40
#define FUNC_FINDCLOSE 41
#define FUNC_FINDFIRST 42
#define FUNC_FINDNEXT 43
#define FUNC_FORMAT 44
#define FUNC_FULLNAME 45
#define FUNC_GETDIR 46
#define FUNC_GETFILE 47
#define FUNC_IF 48
#define FUNC_INC 49
#define FUNC_INDEX 50
#define FUNC_INIREAD 51
#define FUNC_INIWRITE 52
#define FUNC_INSERT 53
#define FUNC_INSTR 54
#define FUNC_INTEGER 55
#define FUNC_LABEL 56
#define FUNC_LEFT 57
#define FUNC_LENGTH 58
#define FUNC_LFN 59
#define FUNC_LINE 60
#define FUNC_LINES 61
#define FUNC_LOWER 62
#define FUNC_LPT 63
#define FUNC_MAKEAGE 64
#define FUNC_MAKEDATE 65
#define FUNC_MAKETIME 66
#define FUNC_MASTER 67
#define FUNC_MONTH 68
#define FUNC_NAME 69
#define FUNC_NUMERIC 70
#define FUNC_PATH 71
#define FUNC_RANDOM 72
#define FUNC_READSCR 73
#define FUNC_READY 74
#define FUNC_REMOTE 75
#define FUNC_REMOVABLE 76
#define FUNC_REPEAT 77
#define FUNC_REPLACE 78
#define FUNC_REXX 79
#define FUNC_RIGHT 80
#define FUNC_SEARCH 81
#define FUNC_SELECT 82
#define FUNC_SFN 83
#define FUNC_STRIP 84
#define FUNC_SUBSTR 85
#define FUNC_TIME 86
#define FUNC_TIMER 87
#define FUNC_TRIM 88
#define FUNC_TRUENAME 89
#define FUNC_UNIQUE 90
#define FUNC_UPPER 91
#define FUNC_WILD 92
#define FUNC_WORD 93
#define FUNC_WORDS 94
#define FUNC_XMS 95
#define FUNC_YEAR 96
#define FUNC_EXETYPE 97
#define FUNC_FSTYPE 98
#define FUNC_EAREAD 99
#define FUNC_EAWRITE 100


// FILECMDS.C
extern char REPLACE[];
extern char FILES_COPIED[];
extern char MOVE_CREATE_DIR[];
extern char FILES_MOVED[];
extern char FILES_RENAMED[];
extern char FILES_DELETED[];
extern char FILES_BYTES_FREED[];
extern char ARE_YOU_SURE[];
extern char DELETE_QUERY[];
extern char DELETING_FILE[];


// INIPARSE.C
extern char INI_ERROR[];
extern char INI_QUERY[];


// LINES.C
extern char BOX_FILL[];
extern char BOX_SHADOW[];
extern char BOX_ZOOM[];


// LIST.C
extern char FFIND_TEXT_FOUND[];
extern char FFIND_ONE_LINE[];
extern char FFIND_MANY_LINES[];
extern char FFIND_FOUND_DLG[];
extern char FFIND_FOUND[];
extern char FFIND_RESULT[];
extern char FFIND_DIALOG_OFFSET[];
extern char FFIND_OFFSET[];
extern char LIST_STDIN_MSG[];
extern char LIST_INFO_FAT[];
extern char LIST_INFO_HPFS[];
extern char LIST_INFO_PIPE[];
extern char LIST_LINE[];
extern char LIST_HEADER[];
extern char LIST_WAIT[];
extern char LIST_GOTO[];
extern char LIST_GOTO_OFFSET[];
extern char LIST_GOTO_TITLE[];
extern char LIST_FIND[];
extern char LIST_FIND_WAIT[];
extern char LIST_FIND_TITLE[];
extern char LIST_FIND_TITLE_REVERSE[];
extern char LIST_FIND_HEX[];
extern char LIST_NOT_FOUND[];
extern char LIST_PRINT_TITLE[];
extern char LIST_SAVE_TITLE[];
extern char LIST_QUERY_PRINT[];
extern char LIST_QUERY_SAVE[];
extern char LIST_PRINTING[];


// MISC.C
extern char DESCRIPTION_FILE[];
extern char DESCRIPTION_SCAN[];
extern char SUBJECT_EA[];
extern char LONGNAME_EA[];
extern char HTTP[];
extern char CONSOLE[];
extern char DEV_CONSOLE[];
extern char YES_NO[];
extern char YES_NO_REST[];
extern char BRIGHT[];
extern char BLINK[];
extern char BORDER[];


// PARSER.C
extern char CANCEL_BATCH_JOB[];
extern char INSERT_DISK[];
extern char UNKNOWN_COMMAND[];
extern char COMMAND_GROUP_MORE[];
extern char CMDLINE_VAR[];
extern char PROMPT_NAME[];
extern char OS2_PROMPT[];


// SCREENIO.C
extern char HISTORY_TITLE[];
extern char DIRECTORY_TITLE[];
extern char FILENAMES_TITLE[];


// SELECT.C
extern char MARKED_FILES[];
extern char SELECT_HEADER[];
extern char SELECT_PAGE_COUNT[];


// SYSCMDS.C
extern char BUILDING_INDEX[];
extern char BUILDING_UNC_INDEX[];
extern char CDPATH[];
extern char UNLABELED[];
extern char VOLUME_LABEL[];
extern char VOLUME_SERIAL[];
extern char NO_PATH[];
extern char BYTES_FREE[];
extern char LBYTES_FREE[];
extern char LOG_FILENAME[];
extern char HLOG_FILENAME[];
extern char BREAK_IS[];
extern char LOG_IS[];
#define VERIFY_IS BREAK_IS
#define KEYS_IS BREAK_IS
extern char SETDOS_IS[];
extern char GLOBAL_DIR[];
extern char TIMER_NUMBER[];
extern char TIMER_ON[];
extern char TIMER_OFF[];
extern char TIMER_SPLIT[];
extern char TIMER_ELAPSED[];
extern char TIME_FMT[];
extern char DATE_FMT[];
extern char NEW_DATE[];
extern char NEW_TIME[];
extern char CODE_PAGE[];
extern char PREPARED_CODE_PAGES[];
