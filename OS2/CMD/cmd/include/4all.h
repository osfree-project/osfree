// 4ALL.H - Include file for all 4xxx / TCMD products
//   Copyright (c) 1988 - 1997  Rex C. Conn   All rights reserved

#include <limits.h>             // get int & long sizes

#define STDIN 0                 // standard input, output, error, aux, & prn
#define STDOUT 1
#define STDERR 2
#define STDAUX 3
#define STDPRN 4

#define HISTMIN 256             // minimum and maximum history list size
#define HISTMAX 8192
#define HISTORY_SIZE 1024

#define DIRSTACKSIZE 512        // directory stack size (for PUSHD/POPD)

#define BADQUOTES ((char *)-1)  // flag for unmatched quotes in line

#define MKFNAME_NOERROR 1
#define MKFNAME_NOCASE  4

#define DESCRIPTION_READ 1      // flags for process_descriptions()
#define DESCRIPTION_WRITE 2
#define DESCRIPTION_COPY 3      // (read | write)
#define DESCRIPTION_CREATE 4
#define DESCRIPTION_EDIT 8
#define DESCRIPTION_REMOVE 0x10
#define DESCRIPTION_PROCESS 0x20

#define FIND_BYATTS 0x400
#define FIND_DATERANGE 0x800
#define FIND_CREATE 0x1000      // tells find_file (OS2 & NT) to create a handle
#define FIND_CLOSE_GLOBAL 0x2000
#define FIND_NO_DOTNAMES 0x8000

#define BREAK_STACK_OVERFLOW 0x100
#define BREAK_CANCEL_PROCESSING 0x400
#define BREAK_ON_ERROR 0x800

// Debug flags for Debug = setting in INI file
#define INI_DB_SHOWTAIL 0x01
#define INI_DB_ERRLABEL 0x02
#define INI_DB_DIRMEM 0x04
#define INI_DB_DIRMEMTOT 0x08
#define INI_DB_FAT32FREE 0x10

// disable macro definitions of toupper() and tolower()
#ifdef toupper
#undef toupper
#undef tolower
#endif

#include <setjmp.h>


typedef struct
{
        // These variables need to be saved when doing an INT 2Eh or
        //   REXX "back door"
        int bn;                 // current batch nesting level
        int call_flag;          // 0 for batch overwrite, 1 for nesting, 2 for NT "call :label args"
        int exception_flag;     // ^C or stack overflow detected flag
        jmp_buf env;            // setjmp save state
        unsigned char verbose;          // command line ECHO ON flag
        union {
            long lFlags;
            struct {
                unsigned char _else;    // waiting for ELSE/ENDIFF
                unsigned _iff : 4;      // parsing an IFF/THEN
                unsigned _endiff : 4;   // ENDIFF nesting level
                unsigned _do : 4;       // parsing a DO/WHILE
                unsigned _do_end : 4;   // ENDDO nesting level
                unsigned _do_leave : 4;
            } flag;
        } f;
} CRITICAL_VARS;


typedef struct
{
        union {
                unsigned long DTStart;
                struct {
                        unsigned short TStart;
                        unsigned short DStart;
                } DTS;
        } DTRS;
        union {
                unsigned long DTEnd;
                struct {
                        unsigned short TEnd;
                        unsigned short DEnd;
                } DTE;
        } DTRE;
        unsigned short DateType;        // 0=last write, 1=last access; 2=created
        unsigned short TimeStart;
        unsigned short TimeEnd;
        unsigned short TimeType;        // 0=last write, 1=last access; 2=created
        unsigned long SizeMin;
        unsigned long SizeMax;
        char *pszExclude;
} RANGES;


// directory structure
typedef struct
{
        unsigned char dummy_pad;
        unsigned char attribute;        // file attribute
        union {
                unsigned short ufTime;
                struct {
                        unsigned seconds : 5;
                        unsigned minutes : 6;
                        unsigned hours : 5;
                } file_time;
        } ft;
        union {
                unsigned short ufDate;
                struct {
                        unsigned days : 5;
                        unsigned months : 4;
                        unsigned years : 7;
                } file_date;
        } fd;
        unsigned long file_size;
        unsigned char file_name[13];
        unsigned char ampm;             // am/pm for filetime
        unsigned short file_mark;       // SELECT file marker
        short color;
        unsigned int comp_ratio;        // compression ratio for DBLSPACE
        unsigned char *file_id;   // optional file description
        unsigned long ea_size;
        unsigned char *hpfs_name;
        unsigned char *hpfs_base;
} DIR_ENTRY;


// Disk info (free space, total space, and cluster size) (Except for NT!)
// TODO: revise double / long long
typedef struct
{
        double BytesFree;
        double BytesTotal;
        long ClusterSize;
        char szBytesFree[16];
        char szBytesTotal[16];
} QDISKINFO;


// CD / CDD flags
#define CD_CHANGEDRIVE  1
#define CD_SAVELASTDIR  2
#define CD_NOFUZZY      4
#define CD_NOERROR      8


// DIR flags

#define FAT 0
#define HPFS 1

#define DIRFLAGS_UPPER_CASE 1
#define DIRFLAGS_LOWER_CASE 2
#define DIRFLAGS_RECURSE 4
#define DIRFLAGS_JUSTIFY 8
#define DIRFLAGS_SUMMARY_ONLY 0x10
#define DIRFLAGS_VSORT 0x20
#define DIRFLAGS_NO_HEADER 0x40
#define DIRFLAGS_NO_FOOTER 0x80
#define DIRFLAGS_NAMES_ONLY 0x100
#define DIRFLAGS_HPFS 0x200
#define DIRFLAGS_HPFS_TO_FAT 0x400
#define DIRFLAGS_FULLNAME 0x800
#define DIRFLAGS_NO_COLOR 0x1000
#define DIRFLAGS_RECURSING_NOW 0x2000
#define DIRFLAGS_NO_DOTS 0x4000
#define DIRFLAGS_NT_ALT_NAME 0x8000
#define DIRFLAGS_WIDE 0x10000L
#define DIRFLAGS_COMPRESSION 0x20000L
#define DIRFLAGS_HOST_COMPRESSION 0x40000L
#define DIRFLAGS_PERCENT_COMPRESSION 0x80000L
#define DIRFLAGS_SHOW_ATTS 0x100000L
#define DIRFLAGS_TRUNCATE_DESCRIPTION 0x200000L
#define DIRFLAGS_TREE 0x400000L
#define DIRFLAGS_ALLOCATED 0x800000L


typedef struct
{
        char cname[30];
        short color;
} CDIR;


// array used for internal commands (indirect function calls)
typedef struct
{
        char *cmdname;                  // command name
        int (* func)(int, char **);     // pointer to function
        int pflag;                      // command line parse control flag
        int index;                      // command index in OSO001.MSG file
} BUILTIN;


#define CMD_EXPAND_VARS 1               // expand variables
#define CMD_EXPAND_REDIR 2              // perform redirection
#define CMD_STRIP_QUOTES 4              // remove single back quotes
#define CMD_ADD_NULLS 8                 // add terminators to each arg
#define CMD_GROUPS 0x10                 // check for command grouping
#define CMD_CLOSE_BATCH 0x20            // close batch file before executing
#define CMD_ONLY_BATCH 0x40             // command only allowed in batch files
#define CMD_DISABLED 0x80               // command enabled (0) or disabled (1)
#define CMD_RESET_DISKS 0x100           // reset disks after command
#define CMD_SET_ERRORLEVEL 0x200        // set ERRORLEVEL upon return (OS2 only)
#define CMD_BACKSLASH_OK 0x400          // kludge for trailing '\'s
#define CMD_DETACH_LINE 0x800           // pass entire line to DETACH command
#define CMD_UCASE_CMD 0x1000
#define CMD_PRESERVE_WHITESPACE 0x2000  // don't strip leading whitespace

#define EXPAND_NO_ALIASES 0x01
#define EXPAND_NO_NESTED_ALIASES 0x02
#define EXPAND_NO_VARIABLES 0x04
#define EXPAND_NO_NESTED_VARIABLES 0x08
#define EXPAND_NO_COMPOUNDS 0x10
#define EXPAND_NO_REDIR 0x20
#define EXPAND_NO_QUOTES 0x40
#define EXPAND_NO_ESCAPES 0x80


// structure used by COLOR to set screen colors via ANSI escape sequences
typedef struct
{
        unsigned char *shade;
        unsigned char ansi;
} ANSI_COLORS;


// structure used by COLORDIR to set screen colors based on attributes
typedef struct
{
        unsigned char *type;
        unsigned char attr;
} COLORD;


// start time for three timers
typedef struct
{
        int timer_flag;
        int thours;
        int tminutes;
        int tseconds;
        int thundreds;
} TIMERS;


#define MAXBATCH 10                     // maximum batch file nesting depth
#define ABORT_LINE 0x0FFF               // strange value to abort multiple cmds
#define BATCH_RETURN 0x7ABC             // strange value to abort batch nesting
#define BATCH_RETURN_RETURN 0x7ABD      // strange value to abort GOSUBs
#define IN_MEMORY_FILE 0x7FFF           // flag for .BTM file


// We define structures ("frames") for batch files; allowing us to nest batch
//   files without the overhead of calling a copy of the command processor
typedef struct
{
        unsigned char *pszBatchName;            // fully qualified filename
        unsigned char **Argv;                   // pointer to argument list
        int bfd;                                // file handle for batch file
        int Argv_Offset;                        // offset into Argv list
        long offset;                            // current file pointer position
        unsigned int uBatchLine;                // current file line (0-based)
        int gsoffset;                           // current gosub nesting level
        unsigned int echo_state;                // current batch echo state (0 = OFF)
        unsigned char *OnBreak;                 // command to execute on ^C
        unsigned char *OnError;                 // command to execute on error
        unsigned char *OnErrorMsg;              // command to execute on error message
        unsigned int OnErrorState;              // prior state of error popups
        unsigned char *pszTitle;                // window/icon title
        unsigned char *local_dir;               // saved disk and directory
        unsigned char *local_env;          // saved environment for SETLOCAL
        unsigned int local_env_size;
        unsigned char *local_alias;        // saved alias list for SETLOCAL
        unsigned int local_alias_size;
        unsigned char *in_memory_buffer;   // pointer to buffer for .BTM files
        int flags;                              // see below
        int nReturn;                            // return value
        unsigned int uChildPipeProcess;
        char cLocalParameter;
        char cLocalEscape;
        char cLocalSeparator;
        char cLocalDecimal;
        char cLocalThousands;
} BATCHFRAME;

#define BATCH_REXX 1
#define BATCH_COMPRESSED 2


// FFIND options
#define FFIND_QUIET 1
#define FFIND_HIDDEN 2
#define FFIND_LINE_NUMBERS 4
#define FFIND_SUBDIRS 8
#define FFIND_ALL 0x10
#define FFIND_TEXT 0x20
#define FFIND_SHOWALL 0x40
#define FFIND_HEX_DISPLAY 0x80
#define FFIND_HEX_SEARCH 0x100
#define FFIND_CHECK_CASE 0x200
#define FFIND_TOPSEARCH 0x400
#define FFIND_ENDSEARCH 0x800
#define FFIND_NOT 0x1000
#define FFIND_NOERROR 0x2000
#define FFIND_DIALOG 0x4000
#define FFIND_REVERSE_SEARCH 0x8000
#define FFIND_STDIN 0x10000L
#define FFIND_NOWILDCARDS 0x20000L


// LIST file info structure
typedef struct {
        char  szName[260];
        int   hHandle;
        int   fEoL;             // line end character (CR or LF)
        union {
                unsigned short ufTime;
                struct {
                        unsigned seconds : 5;
                        unsigned minutes : 6;
                        unsigned hours : 5;
                } file_time;
        } ft;
        union {
                unsigned short ufDate;
                struct {
                        unsigned days : 5;
                        unsigned months : 4;
                        unsigned years : 7;
                } file_date;
        } fd;
        long  lSize;
        long  lCurrentLine;

        long lViewPtr;                  // pointer to top line
        long lFileOffset;               // offset of block

        char *lpBufferEnd;         // end of file buffer
        char *lpEOF;               // pointer to EOF in buffer
        char *lpCurrent;           // current char in get buffer
        char *lpBufferStart;       // beginning of file buffer

        unsigned int uTotalSize;        // size of entire buffer
        unsigned int uBufferSize;       // size of each buffer block
        int nListHorizOffset;           // horizontal scroll offset
        int nSearchLen;
        int fDisplaySearch;
} LISTFILESTRUCT;


#define LIST_BY_ATTRIBUTES 1            // /a:-rhsda
#define LIST_HIBIT 2                    // strip high bit
#define LIST_NOWILDCARDS 4
#define LIST_REVERSE 8
#define LIST_STDIN 0x10                 // get input from STDIN
#define LIST_WRAP 0x20                  // wrap lines at right margin
#define LIST_HEX 0x40                   // display in hex
#define LIST_SEARCH 0x80

// Include INI file data structures
#include "inistruc.h"
extern INIFILE *iniptr;


// popup text window control block
typedef struct
{
        int top;                // upper left (inside) corner of window
        int left;
        int bottom;
        int right;
        int attrib;             // attribute to be used in window
        int inverse;            // inverse of "attrib"
        int c_row;              // current cursor offset in window
        int c_col;
        int old_row;            // cursor position when window was
        int old_col;            //   opened (used for screen restore)
        int hoffset;            // horizontal scroll offset
        int fShadow;            // if != 0, draw shadow
        int fPopupFlags;        // flags for window
        char *screen_save; // pointer to screen save buffer
} POPWINDOW, *POPWINDOWPTR;


#define EDIT_COMMAND 1          // constants for egets()
#define EDIT_DATA 2
#define EDIT_DIALOG 4
#define EDIT_ECHO 8             // and getkey()
#define EDIT_NO_ECHO 0x10
#define EDIT_BIOS_KEY 0x20
#define EDIT_ECHO_CRLF 0x40
#define EDIT_NO_CRLF 0x80
#define EDIT_KB_FLUSH 0x100
#define EDIT_UC_SHIFT 0x200
#define EDIT_PASSWORD 0x400
#define EDIT_NO_INPUTCOLOR 0x800
#define EDIT_DIGITS 0x1000
#define EDIT_SWAP_SCROLL 0x2000

// Key mapping context flag values for calling cvtkey()
#define EXTKEY 1
#define MAP_GEN 2
#define MAP_EDIT 4
#define MAP_HWIN 8
#define MAP_LIST 0x10
#define MAP_NORM_KEY 0x80

#define FIND_FIRST 0x4E
#define FIND_NEXT 0x4F


// 4xxx-specific error messages

#define OFFSET_4DOS_MSG                 0x2000
#define OFFSET_SENDKEYS_MSG             48

#define ERROR_4DOS_BAD_SYNTAX           0+OFFSET_4DOS_MSG
#define ERROR_4DOS_UNKNOWN_COMMAND      1+OFFSET_4DOS_MSG
#define ERROR_4DOS_COMMAND_TOO_LONG     2+OFFSET_4DOS_MSG
#define ERROR_4DOS_NO_CLOSE_QUOTE       3+OFFSET_4DOS_MSG
#define ERROR_4DOS_CANT_OPEN            4+OFFSET_4DOS_MSG
#define ERROR_4DOS_CANT_CREATE          5+OFFSET_4DOS_MSG
#define ERROR_4DOS_CANT_DELETE          6+OFFSET_4DOS_MSG
#define ERROR_4DOS_READ_ERROR           7+OFFSET_4DOS_MSG
#define ERROR_4DOS_WRITE_ERROR          8+OFFSET_4DOS_MSG
#define ERROR_4DOS_DUP_COPY             9+OFFSET_4DOS_MSG
#define ERROR_4DOS_DISK_FULL            10+OFFSET_4DOS_MSG
#define ERROR_4DOS_CONTENTS_LOST        11+OFFSET_4DOS_MSG
#define ERROR_4DOS_INFINITE_COPY        12+OFFSET_4DOS_MSG
#define ERROR_4DOS_INFINITE_MOVE        12+OFFSET_4DOS_MSG
#define ERROR_4DOS_NOT_ALIAS            13+OFFSET_4DOS_MSG
#define ERROR_4DOS_NO_ALIASES           14+OFFSET_4DOS_MSG
#define ERROR_4DOS_ALIAS_LOOP           15+OFFSET_4DOS_MSG
#define ERROR_4DOS_VARIABLE_LOOP        16+OFFSET_4DOS_MSG
#define ERROR_4DOS_UNKNOWN_CMD_LOOP     17+OFFSET_4DOS_MSG
#define ERROR_4DOS_INVALID_DATE         18+OFFSET_4DOS_MSG
#define ERROR_4DOS_INVALID_TIME         19+OFFSET_4DOS_MSG
#define ERROR_4DOS_DIR_STACK_EMPTY      20+OFFSET_4DOS_MSG
#define ERROR_4DOS_CANT_GET_DIR         21+OFFSET_4DOS_MSG
#define ERROR_4DOS_LABEL_NOT_FOUND      22+OFFSET_4DOS_MSG
#define ERROR_4DOS_OUT_OF_ENVIRONMENT   23+OFFSET_4DOS_MSG
#define ERROR_4DOS_OUT_OF_ALIAS         24+OFFSET_4DOS_MSG
#define ERROR_4DOS_NOT_IN_ENVIRONMENT   25+OFFSET_4DOS_MSG
#define ERROR_4DOS_STACK_OVERFLOW       26+OFFSET_4DOS_MSG
#define ERROR_4DOS_ONLY_BATCH           27+OFFSET_4DOS_MSG
#define ERROR_4DOS_MISSING_BATCH        28+OFFSET_4DOS_MSG
#define ERROR_4DOS_EXCEEDED_NEST        29+OFFSET_4DOS_MSG
#define ERROR_4DOS_MISSING_ENDTEXT      30+OFFSET_4DOS_MSG
#define ERROR_4DOS_BAD_RETURN           31+OFFSET_4DOS_MSG
#define ERROR_4DOS_ENV_SAVED            32+OFFSET_4DOS_MSG
#define ERROR_4DOS_ENV_NOT_SAVED        33+OFFSET_4DOS_MSG
#define ERROR_4DOS_UNBALANCED_PARENS    34+OFFSET_4DOS_MSG
#define ERROR_4DOS_NO_EXPRESSION        35+OFFSET_4DOS_MSG
#define ERROR_4DOS_OVERFLOW             36+OFFSET_4DOS_MSG
#define ERROR_4DOS_FILE_EMPTY           37+OFFSET_4DOS_MSG
#define ERROR_4DOS_NOT_A_DIRECTORY      38+OFFSET_4DOS_MSG
#define ERROR_4DOS_CLIPBOARD_INUSE      39+OFFSET_4DOS_MSG
#define ERROR_4DOS_CLIPBOARD_NOT_TEXT   40+OFFSET_4DOS_MSG
#define ERROR_4DOS_ALREADYEXCLUDED      41+OFFSET_4DOS_MSG
#define ERROR_4DOS_BAD_DIRECTIVE        42+OFFSET_4DOS_MSG
#define ERROR_4DOS_INVALID_WINDOW_TITLE 43+OFFSET_4DOS_MSG
#define ERROR_4DOS_NOT_WINDOWED         44+OFFSET_4DOS_MSG
#define ERROR_4DOS_UNKNOWN_PROCESS      45+OFFSET_4DOS_MSG
#define ERROR_4DOS_LISTBOX_FULL         46+OFFSET_4DOS_MSG
#define ERROR_4DOS_ASSOCIATION_NOT_FOUND 47+OFFSET_4DOS_MSG
#define ERROR_4DOS_KEYSTACK_NOT_LOADED  48+OFFSET_4DOS_MSG
#define ERROR_4DOS_NO_FTYPE             48+OFFSET_4DOS_MSG

// 4DOS error return codes
#define USAGE_ERR 1
#define ERROR_EXIT 2
// #define CTRLC 3


#define EOS '\0'                // standard end of string
#define FALSE 0
#define TRUE 1

#define SOH 1
#define CTRLC 3
#define EOT 4
#define ACK 6
#define BELL 7
#define BACKSPACE 8
#define BS 8
#define TAB 9
#define LF 10
#define LINEFEED 10
#define FORMFEED 12
#define FF 12
#define CR 13
#define XON 17
#define DC2 18
#define XOFF 19
#define DC4 20
#define NAK 21
#define SYN 22
#define ETB 23
#define CRC 'C'
#define CAN 24
#define EM 25
#define EoF 26
#define ESC 27
#define ESCAPE 27
#define SPACE 32
#define CTL_BS 127


#define FBIT 256

#define SHIFT_TAB       15+FBIT
#define ALT_TAB         165+FBIT
#define CTL_TAB         148+FBIT

#define F1              59+FBIT         // function keys
#define F2              60+FBIT
#define F3              61+FBIT
#define F4              62+FBIT
#define F5              63+FBIT
#define F6              64+FBIT
#define F7              65+FBIT
#define F8              66+FBIT
#define F9              67+FBIT
#define F10             68+FBIT
#define F11             0x85+FBIT
#define F12             0x86+FBIT
#define SHFT_F1         84+FBIT
#define SHFT_F2         85+FBIT
#define SHFT_F3         86+FBIT
#define SHFT_F4         87+FBIT
#define SHFT_F5         88+FBIT
#define SHFT_F6         89+FBIT
#define SHFT_F7         90+FBIT
#define SHFT_F8         91+FBIT
#define SHFT_F9         92+FBIT
#define SHFT_F10        93+FBIT
#define SHFT_F11        0x87+FBIT
#define SHFT_F12        0x88+FBIT
#define CTL_F1          94+FBIT
#define CTL_F2          95+FBIT
#define CTL_F3          96+FBIT
#define CTL_F4          97+FBIT
#define CTL_F5          98+FBIT
#define CTL_F6          99+FBIT
#define CTL_F7          100+FBIT
#define CTL_F8          101+FBIT
#define CTL_F9          102+FBIT
#define CTL_F10         103+FBIT
#define CTL_F11         0x89+FBIT
#define CTL_F12         0x8A+FBIT
#define ALT_F1          104+FBIT
#define ALT_F2          105+FBIT
#define ALT_F3          106+FBIT
#define ALT_F4          107+FBIT
#define ALT_F5          108+FBIT
#define ALT_F6          109+FBIT
#define ALT_F7          110+FBIT
#define ALT_F8          111+FBIT
#define ALT_F9          112+FBIT
#define ALT_F10         113+FBIT
#define ALT_F11         0x8B+FBIT
#define ALT_F12         0x8C+FBIT
#define HOME            71+FBIT
#define CUR_UP          72+FBIT
#define PgUp            73+FBIT
#define CUR_LEFT        75+FBIT
#define CUR_RIGHT       77+FBIT
#define END             79+FBIT
#define CUR_DOWN        80+FBIT
#define PgDn            81+FBIT
#define INS             82+FBIT
#define DEL             83+FBIT
#define CTL_LEFT        115+FBIT
#define CTL_RIGHT       116+FBIT
#define CTL_END         117+FBIT
#define CTL_PgDn        118+FBIT
#define CTL_HOME        119+FBIT
#define CTL_PgUp        132+FBIT
#define CTL_UP          141+FBIT
#define CTL_DOWN        145+FBIT

#include "version.h"

#define INCL_BASE
#define INCL_DOSNLS
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#define INCL_WIN
#define INCL_PM
#include <os2.h>
#include <cmd_shared.h>         // Include file for cmd tools
#include "4os2.h"

#include "globals.h"            // global variables
#include "proto.h"              // function prototypes

#include "message.h"            // English message definitions for 4xxx

