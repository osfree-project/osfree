/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

/*
$Id: the.h,v 1.28 2002/06/09 09:26:16 mark Exp $
*/

#include <defines.h>

#include <os2.h>

#if defined(USE_XCURSES)
#  define XCURSES
#  include <xcurses.h>
#  define CURSES_H_INCLUDED
#endif

#if defined(USE_NCURSES)
#  include <ncurses.h>
#  define CURSES_H_INCLUDED
#endif

#if defined(USE_EXTCURSES)
#  include <cur00.h>
#  define A_COLOR
#  define COLOR_BLACK           0
#  define COLOR_BLUE            1
#  define COLOR_GREEN           2
#  define COLOR_CYAN            3
#  define COLOR_RED             4
#  define COLOR_MAGENTA         5
#  define COLOR_YELLOW          6
#  define COLOR_WHITE           7
   typedef char bool;
#  ifdef chtype
#    undef chtype
#  endif
#  define chtype NLSCHAR
#  define COLORS       8
#  define COLOR_PAIRS 64
   extern chtype color_pair[COLOR_PAIRS];
#  define COLOR_PAIR(n) color_pair[n]
#  ifndef HAVE_WATTRSET
#    define HAVE_WATTRSET
#    define wattrset(win,attr) xstandout(win,attr)
#    define attrset(attr) xstandout(stdscr,attr)
#  endif
#  ifndef HAVE_NOCBREAK
#    define HAVE_NOCBREAK
#    define nocbreak() nocrmode()
#  endif
#  ifndef HAVE_CBREAK
#    define HAVE_CBREAK
#    define cbreak() crmode()
#  endif
#  define CURSES_H_INCLUDED
#endif

#ifndef CURSES_H_INCLUDED
#  include <curses.h>
#endif

#ifdef HAVE_PROTO
# define Args(a) a
#else
# define Args(a) ()
#endif

/*
 * The following required because IBM AIX 4.3 defines curses colors
 * incorrectly :-(
 */
#if defined(HAVE_BROKEN_COLORS)
# undef COLOR_BLACK
# undef COLOR_BLUE
# undef COLOR_GREEN
# undef COLOR_CYAN
# undef COLOR_RED
# undef COLOR_MAGENTA
# undef COLOR_YELLOW
# undef COLOR_WHITE
# define COLOR_BLACK    0
# define COLOR_BLUE     1
# define COLOR_GREEN    2
# define COLOR_CYAN     3
# define COLOR_RED      4
# define COLOR_MAGENTA  5
# define COLOR_YELLOW   6
# define COLOR_WHITE    7
#endif

#if defined(__OS2__)
#  if defined(MSDOS) && defined(EMX)
#     undef __OS2__
#     if !defined(DOS)
#        define DOS
#     endif
#     if defined(OS2)
#        undef OS2
#     endif
#  else
#     undef MSDOS          /* in case you are using MSC 6.0 for OS/2 */
#  endif
#  include <stdlib.h>
#  include <memory.h>
#  include <string.h>
#  include <process.h>
#  include <errno.h>
#  include <ctype.h>
#  include <fcntl.h>
#  include <io.h>
#  include <sys\types.h>
#  include <sys\stat.h>
#  define ESLASH '\\'
#  define ESTR_SLASH (CHARTYPE *)"\\"
#  if defined(EMX)
#    define OSLASH '\\'
#    define OSTR_SLASH (CHARTYPE *)"\\"
#    define ISLASH '/'
#    define ISTR_SLASH (CHARTYPE *)"/"
#    define HAVE_BROKEN_TMPNAM 1
#  else
#    define OSLASH '/'
#    define OSTR_SLASH (CHARTYPE *)"/"
#    define ISLASH '\\'
#    define ISTR_SLASH (CHARTYPE *)"\\"
#  endif
#  define CURRENT_DIR (CHARTYPE *)"."
#  if defined(MSC)
/* the following 2 defines are to make MSC recognise the new names */
/* of the following OS/2 calls */
#    define DosSetDefaultDisk DosSelectDisk
#    define DosQueryCurrentDisk DosQCurDisk
#  endif
/* the following #define is to eliminate need for the getch.c/getch.h */
/* modules in the OS/2 compilation */
#  define my_getch(win)  wgetch(win)
#endif

#if defined(__MSDOS__) || defined(MSDOS)
#  include <stdlib.h>
#  include <memory.h>
#  include <string.h>
#  include <fcntl.h>
#  include <io.h>
#  if defined(GO32)
#    include <dir.h>
#  else
#    include <process.h>
#    if !defined(EMX)
#      include <direct.h>
#    endif
#  endif
#  include <errno.h>
#  include <io.h>
#  include <ctype.h>
#  include <sys\types.h>
#  include <sys\stat.h>
#  define ESLASH '\\'
#  define ESTR_SLASH (CHARTYPE *)"\\"
#  if defined(GO32) || defined(EMX)
#    define OSLASH '\\'
#    define OSTR_SLASH (CHARTYPE *)"\\"
#    define ISLASH '/'
#    define ISTR_SLASH (CHARTYPE *)"/"
#  else
#    define OSLASH '/'
#    define OSTR_SLASH (CHARTYPE *)"/"
#    define ISLASH '\\'
#    define ISTR_SLASH (CHARTYPE *)"\\"
#  endif
#  if defined(__TURBOC__) || defined(MSC)
#    define HAVE_BROKEN_TMPNAM 1
#  endif
#  define CURRENT_DIR (CHARTYPE *)"."
/* the following #define is to eliminate need for the getch.c/getch.h */
/* modules in the DOS compilation */
#  define my_getch(win)  wgetch(win)
#endif

#if defined(__NT__) || defined(WIN32)
#  include <stdlib.h>
#  include <memory.h>
#  include <string.h>
#  include <process.h>
#  include <errno.h>
#  include <ctype.h>
#  include <fcntl.h>
#  include <io.h>
#  include <sys\types.h>
#  include <sys\stat.h>
#  define ESLASH '\\'
#  define ESTR_SLASH (CHARTYPE *)"\\"
#  if defined(__CYGWIN32__)
#    if defined(UNIX)
#       undef UNIX
#    endif
#    define OSLASH '\\'
#    define OSTR_SLASH (CHARTYPE *)"\\"
#    define ISLASH '/'
#    define ISTR_SLASH (CHARTYPE *)"/"
#  else
#    define OSLASH '/'
#    define OSTR_SLASH (CHARTYPE *)"/"
#    define ISLASH '\\'
#    define ISTR_SLASH (CHARTYPE *)"\\"
#  endif
#  if defined(__WATCOMC__)
#    define HAVE_BROKEN_TMPNAM 1
#  endif
#  define CURRENT_DIR (CHARTYPE *)"."
#  define CAN_RESIZE 1
/* the following #define is to eliminate need for the getch.c/getch.h */
/* modules in the WIN32 compilation */
#  define my_getch(win)  wgetch(win)
#  if defined(__PDCURSES__)
#    define PDCURSES_MOUSE_ENABLED 1
#  endif
#endif

#if defined(AMIGA) && defined(GCC)
#  include <stdlib.h>
#  include <memory.h>
#  include <string.h>
#  include <errno.h>
#  include <ctype.h>
#  include <fcntl.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  define ESLASH '/'
#  define ESTR_SLASH (CHARTYPE *)"/"
#  define OSLASH '/'
#  define OSTR_SLASH (CHARTYPE *)"/"
#  define ISLASH '/'
#  define ISTR_SLASH (CHARTYPE *)"/"
#  define CURRENT_DIR (CHARTYPE *)"."
/* #  define CAN_RESIZE 1 */
#endif

#ifdef HAVE_CONFIG_H
# ifndef HAVE_WATTRSET
#  define wattrset(win,attr) ((attr == A_NORMAL) ? wstandend(win) : wstandout(win))
#  define attrset(attr)      wattrset(stdscr,attr)
# endif
# ifndef HAVE_KEYPAD
#  define keypad(win,bf)
# endif
# ifndef HAVE_NOTIMEOUT
#  define notimeout(win,bf)
# endif
# ifndef HAVE_RAW
#  define raw()
# endif
# ifndef HAVE_NOCBREAK
#  define nocbreak()
# endif
# ifndef HAVE_CBREAK
#  define cbreak()
# endif
# ifndef HAVE_WNOUTREFRESH
#  define wnoutrefresh(win) wrefresh(win)
# endif
# ifndef HAVE_TOUCHLINE
#  define touchline(win,start,num) touchwin(win)
# endif
# ifndef HAVE_RESET_SHELL_MODE
#  define reset_shell_mode()
# endif
# ifndef HAVE_RESET_PROG_MODE
#  define reset_prog_mode()
# endif
#endif

#ifdef M_XENIX
#  define UNIX 1
#endif

#ifdef MINIX
#  define short int
#  define UNIX 1
#endif

#ifdef UNIX
#  define ESLASH '/'
#  define ESTR_SLASH (CHARTYPE *)"/"
#  define OSLASH '\\'
#  define OSTR_SLASH (CHARTYPE *)"\\"
#  define ISLASH ESLASH
#  define ISTR_SLASH ESTR_SLASH
#  define CURRENT_DIR (CHARTYPE *)"."
#endif

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#ifdef HAVE_CTYPE_H
# include <ctype.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#ifdef HAVE_MEMORY_H
# include <memory.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# ifdef HAVE_TIME_H
#  include <time.h>
# endif
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif
#else
# ifdef HAVE_TIME_H
#   include <time.h>
# else
#  ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#  endif
# endif
#endif

#include <signal.h>

#include "regex.h"

#if defined(USE_XCURSES)
#  if defined(SIGWINCH) && defined(HAVE_RESIZE_TERM)
#    define CAN_RESIZE
#  endif
#  define PDCURSES_MOUSE_ENABLED 1
#endif

#if defined(USE_NCURSES) && defined(SIGWINCH)
#  define CAN_RESIZE
#endif

#if defined(HAVE_SLK_INIT)
# if defined(__PDCURSES__)
#  define MAX_SLK    12
#  define MAX_SLK_FORMAT 5
# elif defined(USE_NCURSES)
#  define MAX_SLK    12
#  define MAX_SLK_FORMAT 4
# else
#  define MAX_SLK     8
#  define MAX_SLK_FORMAT 2
# endif
#else
# define MAX_SLK      0
# define MAX_SLK_FORMAT 2
#endif

#ifdef VMS
#  include <string.h>
#  include <file.h>
#  include <types.h>
#  include <stat.h>
#  include <ctype.h>
#  define ISLASH ']'
#  define ISTR_SLASH (CHARTYPE *)"]"
#  define OSLASH ISLASH
#  define OSTR_SLASH ISTR_SLASH
#  define ESLASH ISLASH
#  define ESTR_SLASH ISTR_SLASH
#  define CURRENT_DIR (CHARTYPE *)"[]"
#  ifdef BSD
#    define chtype short
#    define BSDcurses 1
#  endif
#  define touchline(WIN,START,NUM)       touchwin(WIN)
/* #define isdigit(c)   (_ctype[(c) + 1] & 2)
#  define islower(c)   (_ctype[(c) + 1] & 8)
#  define isupper(c)   (_ctype[(c) + 1] & 4)*/
#  define NO_KEYPAD 1
#endif

#ifndef F_OK
#  define         F_OK          00
#endif
#ifndef W_OK
#  define         W_OK          02
#endif
#ifndef R_OK
#  define         R_OK          04
#endif
/*---------------------------------------------------------------------*/
/* End of OS-specific defines                                          */
/*---------------------------------------------------------------------*/
/*
 * This define ensures that any changes in the configure
 * cause the user to have to re-run configure.
 * This is set in defines.h or config.h
 */
#ifndef BUILD3001
# include "This release requires that you run configure again"
#endif

/*
 * This define ensures that any mismatching of version of
 * THE and PDCurses are picked up
 *
 */
#if defined(PDCURSES)
# if PDC_BUILD < 2503
# include "You need a version of PDCurses with PDC_BUILD >= 2503 defined in curses.h"
# endif
#endif

#if defined(PDCURSES) && PDC_BUILD >= 2601 && defined(WIN32)
# define THE_SINGLE_INSTANCE_ENABLED 1
#endif

#if defined(HAVE_SELECT) && defined(HAVE_MKFIFO)
# define THE_SINGLE_INSTANCE_ENABLED 1
#endif

#if defined(A_COLOR)
# define set_colour(attr) (colour_support) ? (((attr)->pair) ? COLOR_PAIR((attr)->pair) | (attr)->mod : (attr)->mod) \
                                           : ((attr)->mono)
#else
# define set_colour(attr)     ((attr)->mono)
# define COLOR_BLACK            0
# define COLOR_BLUE             0
# define COLOR_GREEN            0
# define COLOR_CYAN             0
# define COLOR_RED              0
# define COLOR_MAGENTA          0
# define COLOR_YELLOW           0
# define COLOR_WHITE            0
#endif

#ifndef A_NORMAL
/* Various video attributes */
# define A_STANDOUT      BSD_STANDOUT /* for compatability with BSD curses */
# define A_REVERSE       BSD_STANDOUT /* for compatability with BSD curses */
# define A_UNDERLINE     0
# define A_BLINK         0
# define A_DIM           0
# define A_BOLD          BSD_STANDOUT

/* The next two are subject to change so don't depend on them */
# define A_INVIS         0
# define A_PROTECT       0

# define A_NORMAL        0
# define A_CHARTEXT      0x007F
# define A_ATTRIBUTES    ~A_CHARTEXT
# define A_ALTCHARSET    0
#endif

#if THE_FOLLOWING_REMOVED_IN_22
#ifndef A_NORMAL
/* Various video attributes */
# ifdef HAVE_BSD_CURSES
#  define A_STANDOUT      BSD_STANDOUT /* for compatability with BSD curses */
#  define A_REVERSE       BSD_STANDOUT /* for compatability with BSD curses */
#  define A_UNDERLINE     0
#  define A_BLINK         0
#  define A_DIM           0
#  define A_BOLD          BSD_STANDOUT

/* The next two are subject to change so don't depend on them */
#  define A_INVIS         0
#  define A_PROTECT       0

#  define A_NORMAL        0
#  define A_CHARTEXT      0x007F
#  define A_ATTRIBUTES    ~A_CHARTEXT
#  define A_ALTCHARSET    0

# else

#  define A_STANDOUT      000000200000L
#  define A_UNDERLINE     000000400000L
#  define A_REVERSE       000001000000L
#  define A_BLINK         000002000000L
#  define A_DIM           000004000000L
#  define A_BOLD          000010000000L
#  define A_ALTCHARSET    000100000000L

/* The next two are subject to change so don't depend on them */
#  define A_INVIS         000020000000L
#  define A_PROTECT       000040000000L

#  define A_NORMAL        000000000000L
#  define A_ATTRIBUTES    037777600000L   /* 0xFFFF0000 */
#  define A_CHARTEXT      000000177777L   /* 0x0000FFFF */

# endif
#endif
#endif


#define ATTR2PAIR(fg,bg) (bg|(fg<<3))
#define FOREFROMPAIR(p)  (p>>3)
#define BACKFROMPAIR(p)  (p&0x07)

#ifndef max
# define max(a,b)        (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
# define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#ifndef getmaxy
# ifdef VMS
#  define getmaxy(win)    ((win)->_max_y)
# else
#  if defined(HAVE_UNDERSCORE_MAXY)
#   define getmaxy(win)    ((win)->_maxy)
#  endif
#  if defined(HAVE_MAXY)
#   define getmaxy(win)    ((win)->maxy)
#  endif
# endif
#endif

#ifndef getmaxx
# ifdef VMS
#  define getmaxx(win)    ((win)->_max_x)
# else
#  if defined(HAVE_UNDERSCORE_MAXY)
#   define getmaxx(win)    ((win)->_maxx)
#  endif
#  if defined(HAVE_MAXY)
#   define getmaxx(win)    ((win)->maxx)
#  endif
# endif
#endif

#ifndef getmaxyx
# define getmaxyx(win,y,x)  ((y) = getmaxy(win), (x) = getmaxx(win))
#endif

#if defined(SIGWINCH) && defined(USE_NCURSES)
# define is_termresized()  (ncurses_screen_resized)
#endif
#define QUIT          (-127)
#define SKIP          (-126)
#define QUITOK        (-125)
#define RAW_KEY       (128)

#define MAX_SCREENS         2              /* maximum number of screens*/
#define VIEW_WINDOWS        6            /* number of windows per view */
#define MAX_INT             32766          /* maximum size for integer */
#define MAX_LONG            2147483001L       /* maximum size for long */
#define WORD_DELIMS         (CHARTYPE *)" \t" /* word delimiter characters */

#define TOP_OF_FILE         (CHARTYPE *)"様様 Top of File 様様"
#define BOTTOM_OF_FILE      (CHARTYPE *)"様様 Bottom of File 様様"
#define DIRECTION_NONE      0
#define DIRECTION_FORWARD   1
#define DIRECTION_BACKWARD  (-1)

#define UNDEFINED_OPERAND (-1)

#define STATAREA_OFFSET   9

/* the first 6 windows MUST be numbered 0-5 */
#define WINDOW_FILEAREA  0
#define WINDOW_PREFIX    1
#define WINDOW_COMMAND   2
#define WINDOW_ARROW     3
#define WINDOW_IDLINE    4
#define WINDOW_GAP       5
#define WINDOW_STATAREA  6
#define WINDOW_ERROR     7
#define WINDOW_DIVIDER   8
#define WINDOW_RESERVED  9
#define WINDOW_SLK      10
#define WINDOW_FILETABS 11
#define MAX_THE_WINDOWS 12 /* one more than maximum window number */

#define MAX_PREFIX_WIDTH     20
#define DEFAULT_PREFIX_WIDTH  6
#define DEFAULT_PREFIX_GAP    0

#define MAX_CTLCHARS                64
#define MAX_DELIMITER_LENGTH        20
#define MAX_PARSER_NAME_LENGTH      30

#define CURRENT_SCREEN              screen[current_screen]
#define other_screen                ((current_screen==0)?1:0)
#define OTHER_SCREEN                screen[other_screen]

#define PREVIOUS_VIEW               (vd_current->prev)
#define NEXT_VIEW                   (vd_current->next)

#define CURRENT_VIEW                (vd_current)
#define CURRENT_FILE                (vd_current->file_for_view)
#define CURRENT_WINDOW              (CURRENT_SCREEN.win[vd_current->current_window])
#define CURRENT_PREV_WINDOW         (CURRENT_SCREEN.win[vd_current->previous_window])
#define CURRENT_WINDOW_FILEAREA     (CURRENT_SCREEN.win[WINDOW_FILEAREA])
#define CURRENT_WINDOW_PREFIX       (CURRENT_SCREEN.win[WINDOW_PREFIX])
#define CURRENT_WINDOW_GAP          (CURRENT_SCREEN.win[WINDOW_GAP])
#define CURRENT_WINDOW_COMMAND      (CURRENT_SCREEN.win[WINDOW_COMMAND])
#define CURRENT_WINDOW_ARROW        (CURRENT_SCREEN.win[WINDOW_ARROW])
#define CURRENT_WINDOW_IDLINE       (CURRENT_SCREEN.win[WINDOW_IDLINE])

#define OTHER_VIEW                  (OTHER_SCREEN.screen_view)
#define OTHER_FILE                  (OTHER_VIEW->file_for_view)
#define OTHER_WINDOW                (OTHER_SCREEN.win[OTHER_VIEW->current_window])
#define OTHER_PREV_WINDOW           (OTHER_SCREEN.win[OTHER_VIEW->previous_window])
#define OTHER_WINDOW_FILEAREA       (OTHER_SCREEN.win[WINDOW_FILEAREA])
#define OTHER_WINDOW_PREFIX         (OTHER_SCREEN.win[WINDOW_PREFIX])
#define OTHER_WINDOW_GAP            (OTHER_SCREEN.win[WINDOW_GAP])
#define OTHER_WINDOW_COMMAND        (OTHER_SCREEN.win[WINDOW_COMMAND])
#define OTHER_WINDOW_ARROW          (OTHER_SCREEN.win[WINDOW_ARROW])
#define OTHER_WINDOW_IDLINE         (OTHER_SCREEN.win[WINDOW_IDLINE])

#define SCREEN_VIEW(scr)            (screen[(scr)].screen_view)
#define SCREEN_FILE(scr)            (screen[(scr)].screen_view->file_for_view)
#define SCREEN_WINDOW(scr)          (screen[(scr)].win[screen[(scr)].screen_view->current_window])
#define SCREEN_PREV_WINDOW(scr)     (screen[(scr)].win[screen[(scr)].screen_view->previous_window])
#define SCREEN_WINDOW_FILEAREA(scr) (screen[(scr)].win[WINDOW_FILEAREA])
#define SCREEN_WINDOW_PREFIX(scr)   (screen[(scr)].win[WINDOW_PREFIX])
#define SCREEN_WINDOW_GAP(scr)      (screen[(scr)].win[WINDOW_GAP])
#define SCREEN_WINDOW_COMMAND(scr)  (screen[(scr)].win[WINDOW_COMMAND])
#define SCREEN_WINDOW_ARROW(scr)    (screen[(scr)].win[WINDOW_ARROW])
#define SCREEN_WINDOW_IDLINE(scr)   (screen[(scr)].win[WINDOW_IDLINE])

#define MARK_VIEW (vd_mark)
#define MARK_FILE (vd_mark->file_for_view)

/*------------------------ split/join actions ------------------------*/
#define SPLTJOIN_SPLIT           1
#define SPLTJOIN_JOIN            2
#define SPLTJOIN_SPLTJOIN        3
/*----------------- adjustments for calculating rec_len --------------*/
#define ADJUST_DELETE            1
#define ADJUST_INSERT            2
#define ADJUST_OVERWRITE         3
/*------------------------ column command types ----------------------*/
#define COLUMN_CAPPEND           1
#define COLUMN_CINSERT           2
#define COLUMN_CREPLACE          3
#define COLUMN_COVERLAY          4
/*------------------------ block move command types ------------------*/
#define COMMAND_COPY             1
#define COMMAND_DELETE           2
#define COMMAND_DUPLICATE        3
#define COMMAND_MOVE_COPY_SAME   4
#define COMMAND_MOVE_COPY_DIFF   5
#define COMMAND_MOVE_DELETE_SAME 6
#define COMMAND_MOVE_DELETE_DIFF 7
#define COMMAND_OVERLAY_COPY     8
#define COMMAND_OVERLAY_DELETE   9
/*------------------------ block move source types -------------------*/
#define SOURCE_UNKNOWN      0
#define SOURCE_COMMAND      1
#define SOURCE_PREFIX       2
#define SOURCE_BLOCK        3
#define SOURCE_BLOCK_RESET  4
/*---------------------- return code constants ------------------------*/
#define RC_OK                  0
#define RC_NOT_COMMAND        -1
#define RC_INVALID_ENVIRON    -3
#define RC_TOF_EOF_REACHED     1
#define RC_TARGET_NOT_FOUND    2
#define RC_TRUNCATED           3
#define RC_NO_LINES_CHANGED    4
#define RC_INVALID_OPERAND     5
#define RC_COMMAND_NO_FILES    6
#define RC_FILE_CHANGED       12
#define RC_ACCESS_DENIED      12
#define RC_DISK_FULL          13
#define RC_BAD_FILEID         20
#define RC_BAD_DRIVE          24
#define RC_FILE_NOT_FOUND     28
#define RC_OUT_OF_MEMORY      94
#define RC_NOREXX_ERROR       98
#define RC_SYSTEM_ERROR       99
#define RC_IO_ERROR          100
#define RC_READV_TERM        101
#define RC_READV_TERM_MOUSE  102
/*---------------------- global parameters ----------------------------*/
#define EOLOUT_NONE              0
#define EOLOUT_LF                1
#define EOLOUT_CRLF              2
#define EOLOUT_CR                3
/*---------------------- extract return codes -------------------------*/
#define EXTRACT_ARG_ERROR        (-2)
#define EXTRACT_VARIABLES_SET    (-1)
/*---------------------- attribute defines ----------------------------*/
#define ATTR_FILEAREA    0
#define ATTR_CURLINE     1
#define ATTR_BLOCK       2
#define ATTR_CBLOCK      3
#define ATTR_CMDLINE     4
#define ATTR_IDLINE      5
#define ATTR_MSGLINE     6
#define ATTR_ARROW       7
#define ATTR_PREFIX      8
#define ATTR_CPREFIX     9
#define ATTR_PENDING    10
#define ATTR_SCALE      11
#define ATTR_TOFEOF     12
#define ATTR_CTOFEOF    13
#define ATTR_TABLINE    14
#define ATTR_SHADOW     15
#define ATTR_STATAREA   16
#define ATTR_DIVIDER    17
#define ATTR_RESERVED   18
#define ATTR_NONDISP    19
#define ATTR_HIGHLIGHT  20
#define ATTR_CHIGHLIGHT 21
#define ATTR_THIGHLIGHT 22
#define ATTR_SLK        23
#define ATTR_GAP        24
#define ATTR_CGAP       25
#define ATTR_ALERT      26
#define ATTR_DIALOG     27
#define ATTR_BOUNDMARK  28
#define ATTR_FILETABS   29
#define ATTR_FILETABSDIV 30
#define ATTR_MAX        31
/*--------------------- -- ecolour defines -----------------------------*/
#define ECOLOUR_COMMENTS               0
#define ECOLOUR_STRINGS                1
#define ECOLOUR_NUMBERS                2
#define ECOLOUR_KEYWORDS               3
#define ECOLOUR_LABEL                  4
#define ECOLOUR_PREDIR                 5
#define ECOLOUR_HEADER                 6
#define ECOLOUR_EXTRA_R_PAREN          7
#define ECOLOUR_LEVEL_1_PAREN          8
#define ECOLOUR_LEVEL_1_KEYWORD        9
#define ECOLOUR_LEVEL_1_PREDIR        10
#define ECOLOUR_LEVEL_2_PAREN         11
#define ECOLOUR_LEVEL_3_PAREN         12
#define ECOLOUR_LEVEL_4_PAREN         13
#define ECOLOUR_LEVEL_5_PAREN         14
#define ECOLOUR_LEVEL_6_PAREN         15
#define ECOLOUR_LEVEL_7_PAREN         16
#define ECOLOUR_LEVEL_8_PAREN         17
#define ECOLOUR_INC_STRING            18
#define ECOLOUR_HTML_TAG              19
#define ECOLOUR_HTML_CHAR             20
#define ECOLOUR_FUNCTIONS             21
#define ECOLOUR_NOTUSED2              22
#define ECOLOUR_NOTUSED3              23
#define ECOLOUR_NOTUSED4              24
#define ECOLOUR_NOTUSED5              25
#define ECOLOUR_ALT_KEYWORD_1         26
#define ECOLOUR_ALT_KEYWORD_2         27
#define ECOLOUR_ALT_KEYWORD_3         28
#define ECOLOUR_ALT_KEYWORD_4         29
#define ECOLOUR_ALT_KEYWORD_5         30
#define ECOLOUR_ALT_KEYWORD_6         31
#define ECOLOUR_ALT_KEYWORD_7         32
#define ECOLOUR_ALT_KEYWORD_8         33
#define ECOLOUR_ALT_KEYWORD_9         34
#define ECOLOUR_MAX                   35
/*---------------------- display line types --------------------------*/
#define LINE_LINE                  0
#define LINE_TABLINE               1
#define LINE_SCALE                 2
#define LINE_BOUNDS                4
#define LINE_SHADOW                8
#define LINE_TOF                  16
#define LINE_EOF                  32
#define LINE_RESERVED             64
#define LINE_OUT_OF_BOUNDS_ABOVE 128
#define LINE_OUT_OF_BOUNDS_BELOW 256
#define LINE_HEXSHOW             512
/*--------------------------- target types ---------------------------*/
#define TARGET_ERR          (-1)
#define TARGET_UNFOUND        0x0000
#define TARGET_ABSOLUTE       0x0001
#define TARGET_RELATIVE       0x0002
#define TARGET_STRING         0x0004
#define TARGET_POINT          0x0008
#define TARGET_BLANK          0x0010
#define TARGET_NORMAL         TARGET_ABSOLUTE|TARGET_RELATIVE|TARGET_STRING|TARGET_POINT|TARGET_BLANK
#define TARGET_ALL            0x0020
#define TARGET_BLOCK          0x0040
#define TARGET_BLOCK_ANY      0x0080
#define TARGET_BLOCK_CURRENT  0x0100
#define TARGET_SPARE          0x0200
#define TARGET_FIND           0x0400
#define TARGET_NFIND          0x0800
#define TARGET_FINDUP         0x1000
#define TARGET_NFINDUP        0x2000
#define TARGET_REGEXP         0x4000
/*--------------------------- compatiblility modes -------------------*/
#define COMPAT_THE            1
#define COMPAT_XEDIT          2
#define COMPAT_KEDIT          4
#define COMPAT_KEDITW         8
#define COMPAT_ISPF          16
/*--------------------------- cursor commands ------------------------*/
#define CURSOR_START          (-1)
#define CURSOR_ERROR          (-2)
#define CURSOR_HOME           0
#define CURSOR_HOME_LAST      1
#define CURSOR_SCREEN         2
#define CURSOR_ESCREEN        3
#define CURSOR_CUA            4
#define CURSOR_COLUMN         5
#define CURSOR_FILE           6
#define CURSOR_CMDLINE        7
#define CURSOR_KEDIT          8
#define CURSOR_MOUSE          9
#define CURSOR_GOTO          10
/*--------------------------- defines for tabbing to fields ----------*/
#define WHERE_WINDOW_MASK             0x0000FF00L
#define WHERE_WINDOW_FILEAREA         0x00000100L
#define WHERE_WINDOW_PREFIX_LEFT      0x00000200L
#define WHERE_WINDOW_PREFIX_RIGHT     0x00000400L
#define WHERE_WINDOW_CMDLINE_TOP      0x00000800L
#define WHERE_WINDOW_CMDLINE_BOTTOM   0x00001000L
#define WHERE_ROW_MASK                0x000000FFL
#define WHERE_SCREEN_MASK             0x000F0000L
#define WHERE_SCREEN_FIRST            0x00010000L
#define WHERE_SCREEN_LAST             0x00020000L
#define WHERE_SCREEN_ONLY             0x00040000L
/*--------------------------- defines for headers fo apply -----------*/
#define HEADER_NUMBER                 0x001
#define HEADER_COMMENT                0x002
#define HEADER_STRING                 0x004
#define HEADER_KEYWORD                0x008
#define HEADER_FUNCTION               0x010
#define HEADER_HEADER                 0x020
#define HEADER_LABEL                  0x040
#define HEADER_MATCH                  0x080
#define HEADER_COLUMN                 0x100
#define HEADER_POSTCOMPARE            0x200
#define HEADER_MARKUP                 0x300
#define HEADER_ALL                    (HEADER_NUMBER|HEADER_COMMENT|HEADER_STRING|HEADER_KEYWORD|HEADER_FUNCTION|HEADER_HEADER|HEADER_LABEL|HEADER_MATCH|HEADER_COLUMN|HEADER_POSTCOMPARE|HEADER_MARKUP)

/*
 * struct for LASTOP fields
 */
struct lastop_struct
{
   CHARTYPE value[MAX_COMMAND_LENGTH];
   CHARTYPE *command;
   int min_len;
};
typedef struct lastop_struct LASTOP;
#define LASTOP_ALTER                  0
#define LASTOP_CHANGE                 1
#define LASTOP_CLOCATE                2
#define LASTOP_COUNT                  3
#define LASTOP_FIND                   4
#define LASTOP_LOCATE                 5
#define LASTOP_SCHANGE                6
#define LASTOP_TFIND                  7
#define LASTOP_MAX                    8

struct pending_prefix_command
{
   struct pending_prefix_command *next;
   struct pending_prefix_command *prev;
   CHARTYPE ppc_command[MAX_PREFIX_WIDTH+1];          /* prefix command */
   LINETYPE ppc_line_number;                     /* line number in file */
   LINETYPE ppc_cmd_param;                  /* prefix command parameter */
   short ppc_cmd_idx;                           /* prefix command index */
   bool ppc_block_command;                     /* is it a BLOCK command */
   bool ppc_shadow_line;        /* was command entered on SHADOW line ? */
};
typedef struct pending_prefix_command THE_PPC;

typedef struct
{
   unsigned int new_flag;
   unsigned int changed_flag;
   unsigned int tag_flag;
   unsigned int save_tag_flag;
   unsigned int unused1 ;
   unsigned int unused2 ;
   unsigned int unused3 ;
   unsigned int unused4 ;
} lineflags ;

struct line
{
   struct line *prev;                       /* pointer to previous line */
   struct line *next;                           /* pointer to next line */
   CHARTYPE *name;            /* pointer to name of line (for SET POINT)*/
   CHARTYPE *line;                       /* pointer to contents of line */
   LENGTHTYPE length;                   /* number of characters in line */
#ifdef FILENAME_LENGTH
   LENGTHTYPE filename_length;    /* length of filename in DIR.DIR file */
#endif
   THE_PPC *pre;
   SELECTTYPE select;                     /* select level for each line */
   SELECTTYPE save_select;          /* saved select level (used by ALL) */
   lineflags flags;
};
typedef struct line _LINE;

struct colour_attr
{
   int pair;                                  /* pair number for colour */
   chtype mod;                                       /* colour modifier */
   chtype mono;                                      /* mono attributes */
};
typedef struct colour_attr COLOUR_ATTR;

struct reserved
{
   struct reserved *prev;          /* pointer to previous reserved line */
   struct reserved *next;              /* pointer to next reserved line */
   CHARTYPE *line;                       /* pointer to contents of line */
   CHARTYPE *disp; /* pointer to contents of line after CTLCHAR applied */
   CHARTYPE *spec;                        /* row position specification */
   short length;                             /* length of reserved line */
   short disp_length;  /* length of reserved line after CTLCHAR applied */
   short base;                                              /* row base */
   short off;                                   /* row offset from base */
   chtype highlighting[260];       /* array of colours for highlighting */
   COLOUR_ATTR *attr;                              /* colour attributes */
};
typedef struct reserved RESERVED;

struct prefix_commands
{
   CHARTYPE *cmd;                                     /* prefix command */
   LENGTHTYPE cmd_len;                      /* length of prefix command */
   bool action_prefix;              /* is command an action or a target */
   bool multiples_allowed;                     /* are multiples allowed */
   bool full_target_allowed;            /* full target multiple allowed */
   bool block_prefix_command;             /* is command a block command */
   bool target_required;                 /* does command require target */
   bool valid_on_tof;         /* is command allowed on Top of File line */
   bool valid_on_bof;      /* is command allowed on Bottom of File line */
   bool valid_in_readonly;    /* TRUE if command valid in readonly mode */
   short (*function) Args((THE_PPC *,short,LINETYPE));
   LINETYPE default_target;/* number of lines to process if not specified */
   bool ignore_scope;/* TRUE if scope to be ignored when finding target */
   bool use_last_not_in_scope;/* TRUE if starting at end of shadow lines*/
   int  priority;                         /* priority of prefix command */
   short (*post_function) Args((THE_PPC *,short,LINETYPE));
};
typedef struct prefix_commands PREFIX_COMMAND;
#define PC_IS_ACTION      TRUE
#define PC_NOT_ACTION     FALSE
#define PC_MULTIPLES      TRUE
#define PC_NO_MULTIPLES     FALSE
#define PC_FULL_TARGET      TRUE
#define PC_NO_FULL_TARGET     FALSE
#define PC_IS_BLOCK      TRUE
#define PC_NOT_BLOCK     FALSE
#define PC_TARGET_REQD     TRUE
#define PC_TARGET_NOT_REQD     FALSE
#define PC_VALID_TOF      TRUE
#define PC_INVALID_TOF         FALSE
#define PC_VALID_BOF       TRUE
#define PC_INVALID_BOF     FALSE
#define PC_VALID_RO     TRUE
#define PC_INVALID_RO     FALSE
#define PC_IGNORE_SCOPE     TRUE
#define PC_RESPECT_SCOPE     FALSE
#define PC_USE_LAST_IN_SCOPE     TRUE
#define PC_NO_USE_LAST_IN_SCOPE     FALSE

struct parse_comments
{
   struct parse_comments *prev;
   struct parse_comments *next;
   bool line_comment;                /* TRUE if this is a LINE comment */
   LENGTHTYPE column;      /* 0-ANY MAX_INT-FIRSTNONBLANK other-column */
   CHARTYPE start_delim[MAX_DELIMITER_LENGTH+1];  /* delimiter string for start of comment */
   CHARTYPE end_delim[MAX_DELIMITER_LENGTH+1];    /* delimiter string for end of comment */
   short len_start_delim;
   short len_end_delim;
   bool nested;                       /* can paired comments be nested */
   bool single_line; /* is this comment only allowed on a single line ?*/
};
typedef struct parse_comments PARSE_COMMENTS;

struct parse_keywords
{
   struct parse_keywords *prev;
   struct parse_keywords *next;
   CHARTYPE *keyword;
   short keyword_length;
   CHARTYPE alternate;
   CHARTYPE type;
};
typedef struct parse_keywords PARSE_KEYWORDS;

struct parse_functions
{
   struct parse_functions *prev;
   struct parse_functions *next;
   CHARTYPE *function;
   short function_length;
   CHARTYPE alternate;
};
typedef struct parse_functions PARSE_FUNCTIONS;

struct parse_headers
{
   struct parse_headers *prev;
   struct parse_headers *next;
   CHARTYPE header_delim[MAX_DELIMITER_LENGTH+1];
   short len_header_delim;
   LENGTHTYPE header_column;      /* 0-ANY MAX_INT-FIRSTNONBLANK other-column */
};
typedef struct parse_headers PARSE_HEADERS;

struct parse_postcompare
{
   struct parse_postcompare *prev;
   struct parse_postcompare *next;
   bool is_class_type;
   struct re_pattern_buffer pattern_buffer;
   CHARTYPE *string;
   short string_length;
   CHARTYPE alternate;
};
typedef struct parse_postcompare PARSE_POSTCOMPARE;

struct parser_details
{
   struct parser_details *prev;
   struct parser_details *next;
   /*
    * Parser-level features
    */
   CHARTYPE parser_name[MAX_PARSER_NAME_LENGTH+1];
   CHARTYPE filename[MAX_FILE_NAME+1];
   struct re_pattern_buffer body_pattern_buffer;
   bool have_body_pattern_buffer;
   /*
    * string features
    */
   bool have_string;
   bool check_single_quote;
   bool backslash_single_quote;
   bool check_double_quote;
   bool backslash_double_quote;
   CHARTYPE string_delimiter;
   bool backslash_delimiter;
   /*
    * comments features
    */
   PARSE_COMMENTS *first_comments;
   PARSE_COMMENTS *current_comments;
   bool have_paired_comments;
   /*
    * keyword features
    */
   PARSE_KEYWORDS *first_keyword;
   PARSE_KEYWORDS *current_keyword;
   short min_keyword_length;
   /*
    * function features
    */
   PARSE_FUNCTIONS *first_function;
   PARSE_FUNCTIONS *current_function;
   short min_function_length;
   struct re_pattern_buffer function_pattern_buffer;
   bool have_function_pattern_buffer;
   bool have_function_option_alternate;
   CHARTYPE function_option_alternate;
   /*
    * case features
    */
   bool case_sensitive;
   /*
    * option
    */
   bool rexx_option;
   bool preprocessor_option;
   bool function_option;
   CHARTYPE preprocessor_char;
   CHARTYPE function_char;
   bool function_blank;
   /*
    * match features - minimal at the moment
    */
   bool have_match;
   /*
    * header features
    */
   PARSE_HEADERS *first_header;
   PARSE_HEADERS *current_header;
   bool have_headers;
   /*
    * label features
    */
   CHARTYPE label_delim[11];
   short len_label_delim;
   LENGTHTYPE label_column;      /* 0-ANY MAX_INT-FIRSTNONBLANK other-column */
   /*
    * markup features
    */
   bool have_markup_tag;
   CHARTYPE markup_tag_start_delim[MAX_DELIMITER_LENGTH+1];
   short len_markup_tag_start_delim;
   CHARTYPE markup_tag_end_delim[MAX_DELIMITER_LENGTH+1];
   short len_markup_tag_end_delim;
   bool have_markup_reference;
   CHARTYPE markup_reference_start_delim[MAX_DELIMITER_LENGTH+1];
   short len_markup_reference_start_delim;
   CHARTYPE markup_reference_end_delim[MAX_DELIMITER_LENGTH+1];
   short len_markup_reference_end_delim;
   /*
    * postcompare features
    */
   bool have_postcompare;
   PARSE_POSTCOMPARE *first_postcompare;
   PARSE_POSTCOMPARE *current_postcompare;
   /*
    * column features
    */
#define MAX_PARSER_COLUMNS 20
   bool have_columns;
   LENGTHTYPE first_column[MAX_PARSER_COLUMNS];
   LENGTHTYPE last_column[MAX_PARSER_COLUMNS];
   short number_columns;
   CHARTYPE column_alternate[MAX_PARSER_COLUMNS];
   /*
    * number features
    */
   struct re_pattern_buffer number_pattern_buffer;
   bool have_number_pattern_buffer;
};
typedef struct parser_details PARSER_DETAILS;

 struct parser_mapping
{
   struct parser_mapping *prev;
   struct parser_mapping *next;
   CHARTYPE *parser_name;
   CHARTYPE *filemask;
   CHARTYPE *magic_number;
   int magic_number_length;
   PARSER_DETAILS *parser;
} ;
typedef struct parser_mapping PARSER_MAPPING;

/* structure for repeating targets */
struct rtarget
{
   CHARTYPE *string;                               /* pointer to target */
   unsigned short length;         /* length of specified target: string */
   unsigned short found_length;            /* length of matching string */
   unsigned short start;      /* starting column of found string target */
   CHARTYPE boolean;                                /* boolean operator */
   bool not;                                      /* TRUE if NOT target */
   LINETYPE numeric_target;                     /* numeric target value */
   short  target_type;                                /* type of target */
   bool negative;                         /* TRUE if direction backward */
   bool found;               /* TRUE if this repeating target was found */
   bool have_compiled_re;              /* TRUE if we have a compiled RE */
   struct re_pattern_buffer pattern_buffer;   /* compiled RE for REGEXP */
};
typedef struct rtarget RTARGET;

/* structure for targets */
struct target
{
   CHARTYPE *string;                      /* pointer to original target */
   LINETYPE num_lines;                     /* number of lines to target */
   LINETYPE true_line;                     /* line number to start with */
   LINETYPE last_line;            /* line number of last line in target */
   RTARGET *rt;                         /* pointer to repeating targets */
   short num_targets;                        /* number of targets found */
   short spare;             /* index to which repeating target is spare */
   bool ignore_scope;/* TRUE if scope to be ignored when finding target */
};
typedef struct target TARGET;

typedef struct
{
   CHARTYPE autosave;
   short backup;
   COLOUR_ATTR *attr;
   COLOUR_ATTR *ecolour;             /* array of ECOLOURS for this file */
   CHARTYPE eolout;
   bool tabsout_on;
   bool display_actual_filename;
   bool undoing;
   bool timecheck;
   CHARTYPE tabsout_num;
} PRESERVED_FILE_DETAILS;

typedef struct
{
   CHARTYPE autosave;        /* number of alterations before autosaving */
   short backup;           /* indicates type of backup file to be saved */
   COLOUR_ATTR *attr;                              /* colour attributes */
   COLOUR_ATTR *ecolour;             /* array of ECOLOURS for this file */
   CHARTYPE eolout;     /* indicates how lines are terminated on output */
   bool tabsout_on;      /* indicates if tabs to replace spaces on file */
   bool display_actual_filename;
   bool undoing;
   bool timecheck;                          /* file time stamp checking */
   CHARTYPE tabsout_num;                 /* length of tab stops on file */
   /*
    * All settings above this line are saveable.
    * Ensure that PRESERVED_FILE_DETAILS structure reflects this.
    */
   PRESERVED_FILE_DETAILS *preserved_file_details;
   ROWTYPE status_row;         /* row on which status line is displayed */
   CHARTYPE pseudo_file;   /* indicates if file is a "pseudo" file and if so, what sort */
   CHARTYPE disposition;        /* indicates if file is new or existing */
   unsigned short autosave_alt;  /* number of alterations since last autosave */
   unsigned short save_alt;    /* number of alterations since last save */
   CHARTYPE *autosave_fname;             /* file name for AUTOSAVE file */
   FILE *fp;                               /* file handle for this file */
   CHARTYPE *fname;                                        /* file name */
   CHARTYPE *fpath;                                        /* file path */
   CHARTYPE *actualfname;                         /* filename specified */
   CHARTYPE *efileid;                         /* original full filename */
   unsigned short fmode;                           /* file mode of file */
#if defined(HAVE_CHOWN)
   uid_t uid;                                         /* userid of file */
   gid_t gid;                                        /* groupid of file */
#endif
   long modtime;                      /* timestamp of file modification */
   _LINE *first_line;                           /* pointer to first line */
   _LINE *last_line;                             /* pointer to last line */
   _LINE *editv;                          /* pointer for EDITV variables */
   LINETYPE  number_lines;            /* number of actual lines in file */
   LINETYPE  max_line_length;            /* Maximum line length in file */
   CHARTYPE file_views;              /* number of views of current file */
   RESERVED *first_reserved;          /* pointer to first reserved line */
   THE_PPC *first_ppc;                      /* first pending prefix command */
   THE_PPC *last_ppc;                        /* last pending prefix command */
   CHARTYPE eolfirst;       /* indicates termination of first line read */
   bool colouring;            /* specifies if syntax highlighting is on */
   bool autocolour;                    /* specifies if AUTOCOLOUR is on */
   PARSER_DETAILS *parser;     /* parser to use for syntax highlighting */
   short trailing;       /* how to handle trailing spaces on file write */
} FILE_DETAILS;

typedef struct
{
   struct view_details *prev;               /* pointer to previous view */
   struct view_details *next;                   /* pointer to next view */
   bool arbchar_status;                   /* indicates if arbchar is on */
   CHARTYPE arbchar_single;         /* single arbitrary character value */
   CHARTYPE arbchar_multiple;     /* multiple arbitrary character value */
   bool arrow_on;                    /* indicates if arrow is displayed */
   CHARTYPE case_enter;               /* indicates case of data entered */
   CHARTYPE case_locate;              /* indicates case of data located */
   CHARTYPE case_change;              /* indicates case of data changed */
   CHARTYPE case_sort;       /* indicates case significance for sorting */
   ROWTYPE cmd_line;                        /* position of command line */
   ROWTYPE current_row;                     /* row which is current row */
   CHARTYPE current_base;/* indicates relative position of current line */
   short current_off;       /* offset from current_base for current_row */
   SELECTTYPE display_low;                /* low range of display level */
   SELECTTYPE display_high;              /* high range of display level */
   bool hex;       /* TRUE if hex conversion is done on string operands */
   bool hexshow_on;                                /* status of hexshow */
   CHARTYPE hexshow_base;  /* base position for starting row of hexshow */
   short hexshow_off;           /* offset from base of start of hexshow */
   CHARTYPE highlight;                    /* lines to highlight, if any */
   SELECTTYPE highlight_high; /* high select level of highlighted lines */
   SELECTTYPE highlight_low;   /* low select level of highlighted lines */
   bool id_line;                            /* TRUE if IDLINE displayed */
   bool imp_macro;       /* indicates if implied macro processing is on */
   bool imp_os;             /* indicates if implied os processing is on */
   CHARTYPE inputmode;            /* indicates type of input processing */
   bool linend_status; /* indicates if multiple commands allowed on command line */
   CHARTYPE linend_value;                /* specifies command delimiter */
   bool macro;      /* indicates if macros are executed before commands */
   LENGTHTYPE margin_left;                /* left margin column 1 based */
   LENGTHTYPE margin_right;              /* right margin column 1 based */
   short margin_indent;                        /* paragraph indentation */
   bool margin_indent_offset_status; /* TRUE if paragraph indentation is an offset from left margin */
   CHARTYPE msgline_base;     /* indicates relative position of msgline */
   short msgline_off;           /* offset from msgline_base for msgline */
   ROWTYPE msgline_rows;                   /* number of rows in msgline */
   bool msgmode_status;    /* indicates if messages are to be displayed */
   bool newline_aligned; /* TRUE if adding a new line results in cursor appearing under 1st non-blank of previous line */
   bool number;   /* indicates if numbers in prefix are to be displayed */
   bool position_status;     /* TRUE if LINE/COL is displayed on IDLINE */
   CHARTYPE prefix;       /* indicates if and where prefix is displayed */
   short prefix_width;                       /* overall width of prefix */
   short prefix_gap;        /* width of gap between prefix and filearea */
   bool scale_on;                    /* indicates if scale is displayed */
   CHARTYPE scale_base;/* base position on which scale row is displayed */
   short  scale_off; /* offset from base position on which scale row is displayed */
   bool scope_all; /* indicates if commands act on All lines or only those Displayed */
   bool shadow;        /* indicates if shadow lines are to be displayed */
   bool stay;                               /* indicates status of STAY */
   bool synonym;       /* indicates if synonym processing is in effect */
   bool tab_on;             /* indicates if tab line is to be displayed */
   CHARTYPE tab_base;   /* base position on which tab line is displayed */
   short  tab_off;  /* offset from base position on which tab line is displayed */
   bool tabsinc;                    /* tab increment or 0 if fixed tabs */
   COLTYPE numtabs;                      /* number of tab stops defined */
   LENGTHTYPE tabs[MAX_NUMTABS];      /* tab settings for each tab stop */
   LENGTHTYPE verify_col;                /* left col for current verify */
   LENGTHTYPE verify_start;                   /* col of start of verify */
   LENGTHTYPE verify_end;                       /* col of end of verify */
   CHARTYPE word;                                       /* word setting */
   bool wordwrap;                                   /* wordwrap setting */
   bool wrap;                                           /* wrap setting */
   bool tofeof;                /* true if want to display TOF/EOF lines */
   LENGTHTYPE zone_start;                       /* col of start of zone */
   LENGTHTYPE zone_end;                           /* col of end of zone */
   LINETYPE autoscroll;     /* 0 - no autoscroll, -1 half, other number */
   CHARTYPE boundmark;                             /* type of boundmark */
   LINETYPE syntax_headers;       /* which syntax headers to be applied */
   bool thighlight_on;                 /* indicates if THIGHLIGHT is on */
   bool thighlight_active;         /* indicates if THIGHLIGHT is active */
   TARGET thighlight_target;          /* details of target to highlight */
} PRESERVED_VIEW_DETAILS;

struct view_details
{
   struct view_details *prev;               /* pointer to previous view */
   struct view_details *next;                   /* pointer to next view */
   bool arbchar_status;                   /* indicates if arbchar is on */
   CHARTYPE arbchar_single;         /* single arbitrary character value */
   CHARTYPE arbchar_multiple;     /* multiple arbitrary character value */
   bool arrow_on;                    /* indicates if arrow is displayed */
   CHARTYPE case_enter;               /* indicates case of data entered */
   CHARTYPE case_locate;              /* indicates case of data located */
   CHARTYPE case_change;              /* indicates case of data changed */
   CHARTYPE case_sort;       /* indicates case significance for sorting */
   ROWTYPE cmd_line;                        /* position of command line */
   ROWTYPE current_row;                     /* row which is current row */
   CHARTYPE current_base;/* indicates relative position of current line */
   short current_off;       /* offset from current_base for current_row */
   SELECTTYPE display_low;                /* low range of display level */
   SELECTTYPE display_high;              /* high range of display level */
   bool hex;       /* TRUE if hex conversion is done on string operands */
   bool hexshow_on;                                /* status of hexshow */
   CHARTYPE hexshow_base;  /* base position for starting row of hexshow */
   short hexshow_off;           /* offset from base of start of hexshow */
   CHARTYPE highlight;                    /* lines to highlight, if any */
   SELECTTYPE highlight_high; /* high select level of highlighted lines */
   SELECTTYPE highlight_low;   /* low select level of highlighted lines */
   bool id_line;                            /* TRUE if IDLINE displayed */
   bool imp_macro;       /* indicates if implied macro processing is on */
   bool imp_os;             /* indicates if implied os processing is on */
   CHARTYPE inputmode;            /* indicates type of input processing */
   bool linend_status; /* indicates if multiple commands allowed on command line */
   CHARTYPE linend_value;                /* specifies command delimiter */
   bool macro;      /* indicates if macros are executed before commands */
   LENGTHTYPE margin_left;                /* left margin column 1 based */
   LENGTHTYPE margin_right;              /* right margin column 1 based */
   short margin_indent;                        /* paragraph indentation */
   bool margin_indent_offset_status; /* TRUE if paragraph indentation is an offset from left margin */
   CHARTYPE msgline_base;     /* indicates relative position of msgline */
   short msgline_off;           /* offset from msgline_base for msgline */
   ROWTYPE msgline_rows;                   /* number of rows in msgline */
   bool msgmode_status;    /* indicates if messages are to be displayed */
   bool newline_aligned; /* TRUE if adding a new line results in cursor appearing under 1st non-blank of previous line */
   bool number;   /* indicates if numbers in prefix are to be displayed */
   bool position_status;     /* TRUE if LINE/COL is displayed on IDLINE */
   CHARTYPE prefix;       /* indicates if and where prefix is displayed */
   short prefix_width;                       /* overall width of prefix */
   short prefix_gap;        /* width of gap between prefix and filearea */
   bool scale_on;                    /* indicates if scale is displayed */
   CHARTYPE scale_base;/* base position on which scale row is displayed */
   short  scale_off; /* offset from base position on which scale row is displayed */
   bool scope_all; /* indicates if commands act on All lines or only those Displayed */
   bool shadow;        /* indicates if shadow lines are to be displayed */
   bool stay;                               /* indicates status of STAY */
   bool synonym;       /* indicates if synonym processing is in effect */
   bool tab_on;             /* indicates if tab line is to be displayed */
   CHARTYPE tab_base;   /* base position on which tab line is displayed */
   short  tab_off;  /* offset from base position on which tab line is displayed */
   bool tabsinc;                    /* tab increment or 0 if fixed tabs */
   COLTYPE numtabs;                      /* number of tab stops defined */
   LENGTHTYPE tabs[MAX_NUMTABS];      /* tab settings for each tab stop */
   LENGTHTYPE verify_col;                /* left col for current verify */
   LENGTHTYPE verify_start;                   /* col of start of verify */
   LENGTHTYPE verify_end;                       /* col of end of verify */
   CHARTYPE word;                                       /* word setting */
   bool wordwrap;                                   /* wordwrap setting */
   bool wrap;                                           /* wrap setting */
   bool tofeof;                /* true if want to display TOF/EOF lines */
   LENGTHTYPE zone_start;                       /* col of start of zone */
   LENGTHTYPE zone_end;                           /* col of end of zone */
   LINETYPE autoscroll;     /* 0 - no autoscroll, -1 half, other number */
   CHARTYPE boundmark;                             /* type of boundmark */
   LINETYPE syntax_headers;       /* which syntax headers to be applied */
   bool thighlight_on;                 /* indicates if THIGHLIGHT is on */
   bool thighlight_active;         /* indicates if THIGHLIGHT is active */
   TARGET thighlight_target;          /* details of target to highlight */
   /*
    * All settings above this line are saveable.
    * Ensure that PRESERVED_VIEW_DETAILS structure reflects this.
    */
   PRESERVED_VIEW_DETAILS *preserved_view_details;
   LINETYPE current_line;      /* line in file displayed on current row */
   LENGTHTYPE current_column;   /* column in line of last column target */
   short y[VIEW_WINDOWS];               /* y coordinate for each window */
   short x[VIEW_WINDOWS];               /* x coordinate for each window */
   LINETYPE focus_line;                 /* line in file where cursor is */
   short mark_type;                             /* type of marked block */
   LINETYPE mark_start_line;                 /* first line to be marked */
   LINETYPE mark_end_line;                    /* last line to be marked */
   bool marked_line;                             /* TRUE if line marked */
   bool in_ring;                   /* TRUE if file already in edit ring */
   LENGTHTYPE mark_start_col;                    /* first column marked */
   LENGTHTYPE mark_end_col;                       /* last column marked */
   bool marked_col;                            /* TRUE if column marked */
   int cmdline_col;                     /* column to display in cmdline */
   CHARTYPE current_window;        /* current window for current screen */
   CHARTYPE previous_window;      /* previous window for current screen */
   FILE_DETAILS *file_for_view;            /* pointer to file structure */
};
typedef struct view_details VIEW_DETAILS;

/* structure for each line to be displayed */
struct show_line
{
   CHARTYPE *contents;                   /* pointer to contents of line */
   RESERVED *rsrvd; /* pointer to reserved line struct if a reserved line */
   LINETYPE number_lines_excluded;          /* number of lines excluded */
   LINETYPE line_number;                     /* line number within file */
   _LINE *current;                            /* pointer to current line */
   short line_type;                                     /* type of line */
   bool prefix_enterable;            /* TRUE if prefix can be tabbed to */
   bool main_enterable;            /* TRUE if filearea can be tabbed to */
   bool highlight;                       /* TRUE if line is highlighted */

   /* NOTE: The following entries are only for displaying. They are NOT
    * updated by a call to build_screen. Therefore you can't access them.
    */

   LENGTHTYPE length;                   /* number of characters in line */
   chtype normal_colour;                      /* normal colour for line */
   chtype other_colour;                        /* other colour for line */
   LENGTHTYPE other_start_col; /* start column of other colour from col 0 */
   LENGTHTYPE other_end_col;   /* end column of other colour from col 0 */
   chtype prefix_colour;                            /* colour of prefix */
   chtype gap_colour;                           /* colour of prefix gap */
   CHARTYPE prefix[MAX_PREFIX_WIDTH+1];      /* contents of prefix area */
   chtype prefix_highlighting[MAX_PREFIX_WIDTH+1];    /* array of colours for syntax highlighting */
   CHARTYPE gap[MAX_PREFIX_WIDTH+1];          /* contents of prefix gap */
   chtype gap_highlighting[MAX_PREFIX_WIDTH+1];    /* array of colours for syntax highlighting */
   chtype highlighting[260];    /* array of colours for syntax highlighting */
   bool is_highlighting;  /* TRUE if this line contains syntax highlighting */
   bool is_current_line;       /* TRUE if this line is the current line */
};
typedef struct show_line SHOW_LINE;

/* structure for each screen */
typedef struct
{
   ROWTYPE screen_start_row;                     /* start row of screen */
   COLTYPE screen_start_col;                     /* start col of screen */
   ROWTYPE screen_rows;                                /* physical rows */
   COLTYPE screen_cols;                                /* physical cols */
   ROWTYPE rows[VIEW_WINDOWS];                        /* rows in window */
   COLTYPE cols[VIEW_WINDOWS];                        /* cols in window */
   ROWTYPE start_row[VIEW_WINDOWS];              /* start row of window */
   COLTYPE start_col[VIEW_WINDOWS];              /* start col of window */
   WINDOW *win[VIEW_WINDOWS];  /* curses windows for the screen display */
   VIEW_DETAILS *screen_view;    /* view being displayed in this screen */
   SHOW_LINE *sl;       /* pointer to SHOW_DETAILS structure for screen */
} SCREEN_DETAILS;

/* structure for colour definitions */
typedef struct
{
   chtype fore;
   chtype back;
   chtype mod;
   chtype mono;
} COLOUR_DEF;

/* structure for regular expression syntaxes */
struct regexp_syntax
{
   char *name;
   int value;
};

#define STATUS_ROW         (screen_rows-1)
#define COMMAND_ROW        (screen_rows-2)
#define ERROR_ROW          1
#define TAB_ROW            6
#define TAB_ON             NO
#define SCALE_ROW          6
#define SCALE_ON           NO
#define CURSOR_ROW         COMMAND_ROW
#define CURSOR_COL         5
#define CURRENT_ROW_POS    0
#define CURRENT_ROW        0
#define ZONE_START         1
#define ZONE_END           MAX_INT

/* defines for base value for relative row positions */
#define POSITION_TOP       0
#define POSITION_MIDDLE    1
#define POSITION_BOTTOM    2

/* defines for function_key() function calling */
#define OPTION_NORMAL      0
#define OPTION_EXTRACT     1
#define OPTION_READV       2

/* defines for pseudo file types */
#define PSEUDO_REAL        0
#define PSEUDO_DIR         1
#define PSEUDO_REXX        2
#define PSEUDO_KEY         3
#define PSEUDO_REMOTE      4

/* defines for prefix settings */
#define PREFIX_OFF           0x00
#define PREFIX_ON            0x10
#define PREFIX_NULLS         0x20
#define PREFIX_LEFT          0x01
#define PREFIX_RIGHT         0x02
#define PREFIX_LOCATION_MASK 0x0F
#define PREFIX_STATUS_MASK   0xF0

/* defines for query types */
#define QUERY_NONE         0
#define QUERY_QUERY        1
#define QUERY_STATUS       2
#define QUERY_EXTRACT      4
#define QUERY_FUNCTION     8
#define QUERY_MODIFY      16
#define QUERY_READV       32

/* defines for case settings */
#define CASE_MIXED         (CHARTYPE)'M'
#define CASE_UPPER         (CHARTYPE)'U'
#define CASE_LOWER         (CHARTYPE)'L'
#define CASE_IGNORE        (CHARTYPE)'I'
#define CASE_RESPECT       (CHARTYPE)'R'

/* type of marked blocks - do not change these values!! SET BLOCK needs them */
#define M_NONE             0
#define M_LINE             1
#define M_BOX              2
#define M_STREAM           3
#define M_COLUMN           4
#define M_WORD             5
#define M_CUA              6

/* defines for temporary space allocation */
#define TEMP_PARAM         1
#define TEMP_MACRO         2
#define TEMP_TMP_CMD       3
#define TEMP_TEMP_CMD      4

/* defines for [SET] BACKUP */
#define BACKUP_OFF         1
#define BACKUP_TEMP        2
#define BACKUP_KEEP        3
#define BACKUP_ON          BACKUP_KEEP
#define BACKUP_INPLACE     4

/* defines for [SET] DIRSORT */
#define DIRSORT_NONE       0
#define DIRSORT_DIR        1
#define DIRSORT_SIZE       2
#define DIRSORT_NAME       3
#define DIRSORT_DATE       4
#define DIRSORT_TIME       5
#define DIRSORT_ASC        0
#define DIRSORT_DESC       1

/* box opertaions */
#define BOX_C           1
#define BOX_M           2
#define BOX_D           3
#define BOX_F           4

/* defines for highlighting */
#define HIGHLIGHT_NONE     0
#define HIGHLIGHT_TAG      1
#define HIGHLIGHT_ALT      2
#define HIGHLIGHT_SELECT   3

/* defines for INPUTMODE */
#define INPUTMODE_OFF      0
#define INPUTMODE_FULL     1
#define INPUTMODE_LINE     2

/* defines for EDITV - suprise! */
#define EDITV_GET    1
#define EDITV_PUT    2
#define EDITV_SET    3
#define EDITV_SETL   4
#define EDITV_LIST   5

#ifndef FILE_NORMAL
#define FILE_NORMAL        0
#endif

#ifndef FILE_READONLY
#define FILE_READONLY      1
#endif

#ifndef FILE_NEW
#define FILE_NEW          99
#endif

#define COMMAND_ONLY_TRUE    TRUE
#define COMMAND_ONLY_FALSE   FALSE

#define ADDCHAR            0
#define INSCHAR            1

#define LVL_GLOB  1
#define LVL_FILE  2
#define LVL_VIEW  3

#define CHAR_OTHER     0
#define CHAR_ALPHANUM  1
#define CHAR_SPACE     2

#define SHIFT_ALT            1
#define SHIFT_CTRL           2
#define SHIFT_SHIFT          4
#define SHIFT_MODIFIER_ONLY  8

#define INTERFACE_CLASSIC    1
#define INTERFACE_CUA        2
/*
 * #defines for SET TRAILING
 */
#define TRAILING_OFF       0
#define TRAILING_ON        1
#define TRAILING_SINGLE    2
#define TRAILING_EMPTY     3
/*
 * #defines for behaviour of commands when a CUA block is current
 */
#define CUA_NONE           0
#define CUA_DELETE_BLOCK   1
#define CUA_RESET_BLOCK    2
/*
 * #defines for behaviour of commands with a THIGHLIGHT area
 */
#define THIGHLIGHT_NONE        0
#define THIGHLIGHT_RESET_ALL   1
#define THIGHLIGHT_RESET_FOCUS 2

/*
 * #defines for BOUNDMARK
 */
#define BOUNDMARK_OFF       0
#define BOUNDMARK_ZONE      1
#define BOUNDMARK_TRUNC     2
#define BOUNDMARK_MARGINS   3
#define BOUNDMARK_TABS      4
#define BOUNDMARK_VERIFY    5

/*
 * following #defines for MyStrip() function
 */
#define STRIP_NONE         0
#define STRIP_LEADING      1
#define STRIP_TRAILING     2
#define STRIP_BOTH         (STRIP_LEADING|STRIP_TRAILING)
#define STRIP_ALL          (STRIP_LEADING|STRIP_TRAILING|4)
/*
 * following #defines for button types for DIALOG command
 */
#define BUTTON_OK          0
#define BUTTON_OKCANCEL    1
#define BUTTON_YESNO       2
#define BUTTON_YESNOCANCEL 3
/*
 * following #defines for icon types for DIALOG command
 */
#define ICON_NONE          0
#define ICON_EXCLAMATION   1
#define ICON_INFORMATION   2
#define ICON_QUESTION      3
#define ICON_STOP          4
/*
 * following #defines for SET READONLY
 */
#define READONLY_OFF       0
#define READONLY_ON        1
#define READONLY_FORCE     2
/*
 * Following are used for determining the button action of the mouse
 */
#if !defined(BUTTON_RELEASED)
# define BUTTON_RELEASED 0
#endif
#if !defined(BUTTON_PRESSED)
# define BUTTON_PRESSED 1
#endif
#if !defined(BUTTON_CLICKED)
# define BUTTON_CLICKED 2
#endif
#if !defined(BUTTON_DOUBLE_CLICKED)
# define BUTTON_DOUBLE_CLICKED 3
#endif
#if !defined(BUTTON_MOVED)
# define BUTTON_MOVED 5
#endif

#ifndef getbegyx
# if defined(HAVE_BEGY)
#  define getbegyx(win,y,x)       (y = (win)->begy, x = (win)->begx)
# endif
# if defined(HAVE_UNDERSCORE_BEGY)
#  define getbegyx(win,y,x)       (y = (win)->_begy, x = (win)->_begx)
# endif
#endif

#define HIT_ANY_KEY "Hit any key to continue..."

/*---------------------- useful macros --------------------------------*/
#define     TOF(line)           ((line == 0L) ? TRUE : FALSE)
#define     BOF(line)           ((line == CURRENT_FILE->number_lines+1L) ? TRUE : FALSE)
#define     VIEW_TOF(view,line) ((line == 0L) ? TRUE : FALSE)
#define     VIEW_BOF(view,line) ((line == view->file_for_view->number_lines+1L) ? TRUE : FALSE)
#define     CURRENT_TOF         ((CURRENT_VIEW->current_line == 0L) ? TRUE : FALSE)
#define     CURRENT_BOF         ((CURRENT_VIEW->current_line == CURRENT_FILE->number_lines+1L) ? TRUE : FALSE)
#define     FOCUS_TOF           ((CURRENT_VIEW->focus_line == 0L) ? TRUE : FALSE)
#define     FOCUS_BOF           ((CURRENT_VIEW->focus_line == CURRENT_FILE->number_lines+1L) ? TRUE : FALSE)
#define     IN_VIEW(view,line)   ((line >= (view->current_line - (LINETYPE)view->current_row)) && (line <= (view->current_line + ((LINETYPE)CURRENT_SCREEN.rows[WINDOW_FILEAREA] - (LINETYPE)view->current_row))))
#define     IN_SCOPE(view,line) ((line)->select >= (view)->display_low && (line)->select <= (view)->display_high)
/*---------------------- system specific redefines --------------------*/
#ifdef VAX
#define     wattrset     wsetattr
#define     A_REVERSE    _REVERSE
#define     A_BOLD       _BOLD
#endif

#define ISREADONLY(x)  (readonly || (READONLYx==READONLY_FORCE) || (READONLYx==READONLY_ON && x->disposition == FILE_READONLY))
#define STATUSLINEON() ((STATUSLINEx == 'T') || (STATUSLINEx == 'B'))

extern VIEW_DETAILS *vd_current;
extern CHARTYPE current_screen;
extern SCREEN_DETAILS screen[MAX_SCREENS];

#ifdef MAIN
LENGTHTYPE max_line_length=MAX_LENGTH_OF_LINE;
#else
extern LENGTHTYPE max_line_length;
#endif

#define PARACOL (CURRENT_VIEW->margin_indent_offset_status?CURRENT_VIEW->margin_left+CURRENT_VIEW->margin_indent:CURRENT_VIEW->margin_indent)

/* structure for passing queryable values parameters */
struct query_values
{
   CHARTYPE *value;                                    /* value of item */
   short len;               /* length of string representation of value */
};
typedef struct query_values VALUE;

/* structure for function key redefinition */
struct defines
{
   struct defines *prev;
   struct defines *next;
   int def_funkey;
   short def_command;
   CHARTYPE *def_params;
   CHARTYPE *synonym;
   CHARTYPE *pcode;
   int pcode_len;
   CHARTYPE linend;
};
typedef struct defines DEFINE;

/* structure for window areas */
struct window_areas
{
   CHARTYPE *area;            /* window area - used for COLOUR command */
   short area_min_len;                     /* min abbrev for area name */
   short area_window;                          /* window where area is */
   bool  actual_window;                    /* TRUE if area is a window */
};
typedef struct window_areas AREAS;

typedef short (ExtractFunction) Args(( short, short, CHARTYPE *, CHARTYPE ,LINETYPE ,CHARTYPE *, LINETYPE ));

/* structure for query and implied extract */
struct query_item
{
   CHARTYPE *name;                                      /* name of item */
   short name_length;                        /* length of function name */
   short min_len;                     /* minimum length of abbreviation */
   short item_number;                         /* unique number for item */
   short number_values;                    /* number of values returned (from query/extract)*/
   short item_values;     /* number of values this item can have (implied extract) (excludes 0th value ) */
   short level;                    /* level of item; global, file, view */
   CHARTYPE query;                            /* valid query response ? */
   ExtractFunction *ext_func;  /* function that generates extract details */
};
typedef struct query_item QUERY_ITEM;

/* maximum number of variables that can be returned via EXTRACT */
/* this MUST be max of ECOLOUR_MAX and ATTR_MAX */
#define MAX_VARIABLES_RETURNED               ECOLOUR_MAX

/* structure for list of TLD headers */
typedef struct
{
   char *the_header_name;
   int the_header_name_len;
   LINETYPE the_header;
} the_header_mapping;

#if defined(HAVE_STRICMP)
# define my_stricmp stricmp
#elif defined(HAVE_STRCMPI)
# define my_stricmp strcmpi
#elif defined(HAVE_STRCASECMP)
# define my_stricmp strcasecmp
#else
#endif

#if defined(MAIN)
#  ifdef MSWIN
#    define _THE_FAR __far
void far * (*the_malloc)(unsigned long);             /* ptr to some malloc(size) */
void far * (*the_calloc)();                          /* ptr to some ecalloc(num,size)*/
void  (*the_free)();                            /* ptr to some free(ptr) */
void far * (*the_realloc)(void far *,unsigned long); /* ptr to some realloc(ptr,size) */
#  else
#    define _THE_FAR
void* (*the_malloc)();  /* ptr to some malloc(size) */
void* (*the_calloc)();  /* ptr to some calloc(num,size)*/
void  (*the_free)();    /* ptr to some free(ptr) */
void* (*the_realloc)(); /* ptr to some realloc(ptr,size) */
#  endif
#else
#  ifdef MSWIN
#    define _THE_FAR __far
extern void far * (*the_malloc)(unsigned long);
extern void far * (*the_calloc)();
extern void  (*the_free)();
extern void far * (*the_realloc)(void far *,unsigned long);
#  else
#    define _THE_FAR
extern void* (*the_malloc)();
extern void* (*the_calloc)();
extern void  (*the_free)();
extern void* (*the_realloc)();
#  endif
#endif

#if defined(THE_TRACE)
void trace_initialise Args((void));
void trace_function Args((char *));
void trace_return Args((void));
void trace_string Args((char *,...));
void trace_constant Args((char *));
# define TRACE_RETURN()     trace_return()
# define TRACE_FUNCTION(x)  trace_function(x)
# define TRACE_INITIALISE() trace_initialise()
# define TRACE_CONSTANT(x) trace_constant(x)
#else
# define TRACE_RETURN()
# define TRACE_FUNCTION(x)
# define TRACE_INITIALISE()
# define TRACE_CONSTANT(x)
#endif
#include "vars.h"
