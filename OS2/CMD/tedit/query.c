/***********************************************************************/
/* QUERY.C - Functions related to QUERY,STATUS and EXTRACT             */
/***********************************************************************/
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

static char RCSid[] = "$Id: query.c,v 1.33 2002/06/09 09:26:16 mark Exp $";

#include <the.h>
#include <proto.h>

#include <query.h>

#ifdef HAVE_PROTO
static short extract_point_settings(short,CHARTYPE *);
static short extract_prefix_settings(short,CHARTYPE *);
static short extract_colour_settings(short,CHARTYPE *,CHARTYPE,CHARTYPE *,bool,bool);
static short extract_autocolour_settings(short,CHARTYPE *,CHARTYPE,CHARTYPE *,bool);
static void get_etmode(CHARTYPE *,CHARTYPE *);
#else
static short extract_point_settings();
static short extract_prefix_settings();
static short extract_autocolour_settings();
static void get_etmode();
#endif

static ExtractFunction extract_after_function;
static ExtractFunction extract_alt;
static ExtractFunction extract_alt_function;
static ExtractFunction extract_altkey_function;
static ExtractFunction extract_arbchar;
static ExtractFunction extract_autocolour;
static ExtractFunction extract_autocolor;
static ExtractFunction extract_autosave;
static ExtractFunction extract_autoscroll;
static ExtractFunction extract_backup;
static ExtractFunction extract_batch_function;
static ExtractFunction extract_before_function;
static ExtractFunction extract_beep;
static ExtractFunction extract_blank_function;
static ExtractFunction extract_block;
static ExtractFunction extract_block_function;
static ExtractFunction extract_bottomedge_function;
static ExtractFunction extract_case;
static ExtractFunction extract_clearerrorkey;
static ExtractFunction extract_clearscreen;
static ExtractFunction extract_clock;
static ExtractFunction extract_command_function;
static ExtractFunction extract_cmdarrows;
static ExtractFunction extract_cmdline;
static ExtractFunction extract_color;
static ExtractFunction extract_coloring;
static ExtractFunction extract_colour;
static ExtractFunction extract_colouring;
static ExtractFunction extract_column;
static ExtractFunction extract_compat;
static ExtractFunction extract_ctrl_function;
static ExtractFunction extract_ctlchar;
static ExtractFunction extract_curline;
static ExtractFunction extract_cursor;
static ExtractFunction extract_cursorstay;
static ExtractFunction extract_current_function;
static ExtractFunction extract_define;
static ExtractFunction extract_defsort;
static ExtractFunction extract_dir_function;
static ExtractFunction extract_dirfileid;
static ExtractFunction extract_dirinclude;
static ExtractFunction extract_end_function;
static ExtractFunction extract_display;
static ExtractFunction extract_ecolor;
static ExtractFunction extract_ecolour;
static ExtractFunction extract_efileid;
static ExtractFunction extract_eof;
static ExtractFunction extract_eof_function;
static ExtractFunction extract_eolout;
static ExtractFunction extract_equivchar;
static ExtractFunction extract_etmode;
static ExtractFunction extract_field;
static ExtractFunction extract_fieldword;
static ExtractFunction extract_first_function;
static ExtractFunction extract_focuseof_function;
static ExtractFunction extract_focustof_function;
static ExtractFunction extract_filename;
static ExtractFunction extract_filestatus;
static ExtractFunction extract_fmode;
static ExtractFunction extract_fname;
static ExtractFunction extract_fpath;
static ExtractFunction extract_ftype;
static ExtractFunction extract_fullfname;
static ExtractFunction extract_getenv;
static ExtractFunction extract_hex;
static ExtractFunction extract_header;
static ExtractFunction extract_hexdisplay;
static ExtractFunction extract_hexshow;
static ExtractFunction extract_highlight;
static ExtractFunction extract_idline;
static ExtractFunction extract_impmacro;
static ExtractFunction extract_impos;
static ExtractFunction extract_inblock_function;
static ExtractFunction extract_incommand_function;
static ExtractFunction extract_initial_function;
static ExtractFunction extract_inprefix_function;
static ExtractFunction extract_inputmode;
static ExtractFunction extract_insertmode;
static ExtractFunction extract_lastkey;
static ExtractFunction extract_lastmsg;
static ExtractFunction extract_lastop;
static ExtractFunction extract_lastrc;
static ExtractFunction extract_leftedge_function;
static ExtractFunction extract_length;
static ExtractFunction extract_line;
static ExtractFunction extract_lineflag;
static ExtractFunction extract_linend;
static ExtractFunction extract_lscreen;
static ExtractFunction extract_macro;
static ExtractFunction extract_macroext;
static ExtractFunction extract_macropath;
static ExtractFunction extract_margins;
static ExtractFunction extract_modifiable_function;
static ExtractFunction extract_monitor;
static ExtractFunction extract_mouse;
static ExtractFunction extract_msgline;
static ExtractFunction extract_msgmode;
static ExtractFunction extract_nbfile;
static ExtractFunction extract_nbscope;
static ExtractFunction extract_newlines;
static ExtractFunction extract_nondisp;
static ExtractFunction extract_number;
static ExtractFunction extract_parser;
static ExtractFunction extract_pagewrap;
static ExtractFunction extract_pending;
static ExtractFunction extract_point;
static ExtractFunction extract_position;
static ExtractFunction extract_prefix;
static ExtractFunction extract_printer;
static ExtractFunction extract_reprofile;
static ExtractFunction extract_regexp;
static ExtractFunction extract_readonly;
static ExtractFunction extract_readv;
static ExtractFunction extract_reserved;
static ExtractFunction extract_rexx;
static ExtractFunction extract_rexxoutput;
static ExtractFunction extract_rightedge_function;
static ExtractFunction extract_ring;
static ExtractFunction extract_scale;
static ExtractFunction extract_scope;
static ExtractFunction extract_screen;
static ExtractFunction extract_select;
static ExtractFunction extract_shadow;
static ExtractFunction extract_shadow_function;
static ExtractFunction extract_shift_function;
static ExtractFunction extract_showkey;
static ExtractFunction extract_size;
static ExtractFunction extract_synonym;
static ExtractFunction extract_spacechar_function;
static ExtractFunction extract_statusline;
static ExtractFunction extract_stay;
static ExtractFunction extract_tabkey;
static ExtractFunction extract_tabline;
static ExtractFunction extract_tabs;
static ExtractFunction extract_tabsin;
static ExtractFunction extract_tabsout;
static ExtractFunction extract_targetsave;
static ExtractFunction extract_terminal;
static ExtractFunction extract_thighlight;
static ExtractFunction extract_timecheck;
static ExtractFunction extract_tof;
static ExtractFunction extract_tof_function;
static ExtractFunction extract_tofeof;
static ExtractFunction extract_topedge_function;
static ExtractFunction extract_trailing;
static ExtractFunction extract_typeahead;
static ExtractFunction extract_undoing;
static ExtractFunction extract_untaa;
static ExtractFunction extract_verify;
static ExtractFunction extract_vershift;
static ExtractFunction extract_verone_function;
static ExtractFunction extract_version;
static ExtractFunction extract_width;
static ExtractFunction extract_word;
static ExtractFunction extract_wordwrap;
static ExtractFunction extract_wrap;
static ExtractFunction extract_xterminal;
static ExtractFunction extract_zone;

static CHARTYPE num1[10];
static CHARTYPE num2[10];
static CHARTYPE num3[40];
static CHARTYPE num4[40];
static CHARTYPE num5[10];
static CHARTYPE num6[10];
static CHARTYPE num7[10];
static CHARTYPE num8[10];
static CHARTYPE rsrvd[MAX_FILE_NAME+1];
static _LINE *curr;


/*man-start*********************************************************************


========================================================================
QUERY, EXTRACT and STATUS
========================================================================

     The following lists the valid variables that can be queried and
     also shows what values are returned. For both <QUERY> and <STATUS>,
     the values are concatenated together and displayed as a single
     line. For <EXTRACT> the REXX variables that are set are defined.
     The capitalised portion of the variable is the minimum abbreviation
     recognised.

     The bracketed text at the end of the description indicates from
     which commands a valid response will be supplied.
     (Q-Query, E-Extract, M-Modify and S-Status).

ALT
     The number of alterations to the current file since the last <SAVE>
     or automatic save via <SET AUTOSAVE>. Can be set by <SET ALT>
     (QES)

     alt.0           - 2
     alt.1           - Number of alterations since last SAVE or autosave
     alt.2           - Number of alterations since last SAVE

ARBchar
     The status of <SET ARBCHAR> and the ARBCHAR characters.
     (QEMS)

     arbchar.0       - 3
     arbchar.1       - ON|OFF
     arbchar.2       - multiple match character
     arbchar.3       - single match character

AUTOCOLOR [*|mask]
AUTOCOLOUR [*|mask]
     The status of auto colouring for the supplied file mask or
     "magic number". Set by <SET AUTOCOLOR> or <SET AUTOCOLOUR>.
     The variable name is spelt the save way that the option is specified.
     (QES)

     autocolor.0     - 3
     autocolor.1     - filemask or "magic number"
     autocolor.2     - name of parser associated or "NULL"
     autocolor.3     - MAGIC (if "magic number") or empty

     If no file mask or "magic number" is supplied as a parameter
     or '*' is passed, details of all parser mappings are set as
     follows:

     autocolor.0     - number of parser mappings
     autocolor.1     - first mapping (mask parser [MAGIC])
     autocolor.n     - nth mapping

AUtosave
     The status of <SET AUTOSAVE> and/or the frequency setting.
     (QEMS)

     autosave.0      - 1
     autosave.1      - OFF|n

AUTOSCroll
     The status of <SET AUTOSCROLL> and/or the frequency setting.
     (QEMS)

     autoscroll.0    - 1
     autoscroll.1    - OFF|HALF|n

BACKup
     Indicates if a .bak file is kept after editing.
     (QEMS)

     backup.0        - 1
     backup.1        - ON|OFF|TEMP|KEEP|INPLACE

BEEP
     Indicates if the bell is sounded on display of error messages.
     Set by <SET BEEP>
     (QEMS)

     beep.0          - 1
     beep.1          - ON|OFF

BLOCK
     Returns information about the marked <block>, if any.
     (QE)

     block.0         - 6 if a marked block exists, or 1 for NONE
     block.1         - type of marked block (LINE|BOX|NONE|WORD|COLUMN|CUA)
     block.2         - line number of start of block
     block.3         - column number of start of block
     block.4         - line number of end of block
     block.5         - column number of end of block
     block.6         - file name containing marked block

CASE
     The settings related to the CASE of characters entered, searched
     for, changed and sorted. Set by <SET CASE>
     (QEMS)

     case.0          - 4
     case.1          - MIXED|UPPER|LOWER
     case.2          - RESPECT|IGNORE
     case.3          - RESPECT|IGNORE
     case.4          - RESPECT|IGNORE

CLEARErrorkey
     Returns the key that clears the <message line>. If no specific
     key is defined, ie ALL keys perform a reset, then * is returned.
     Set by <SET CLEARERRORKEY>.
     (QEMS)

     clearerrorkey.0 - 1
     clearerrorkey.1 - keyname|*

CLEARScreen
     Indicates if the screen is cleared on exit from THE. Set by
     <SET CLEARSCREEN>.
     (QEMS)

     clearscreen.0   - 1
     clearscreen.1   - ON|OFF

CLOCK
     Indicates if the time is displayed on the status line. Set by
     <SET CLOCK>.
     (QEMS)

     clock.0         - 1
     clock.1         - ON|OFF

CMDArrows
     Returns the settings for arrow key behaviour on <command line>.
     Set by <SET CMDARROWS>.
     (QEMS)

     cmdarrows.0     - 1
     cmdarrows.1     - RETRIEVE|TAB

CMDline
     The settings for the <command line>. Set by <SET CMDLINE>.
     (QEMS)

     If CMDLINE OFF

     cmdline.0       - 1
     cmdline.1       - OFF

     If CMDLINE ON, BOTTOM or TOP

     cmdline.0       - 3
     cmdline.1       - ON|TOP|BOTTOM
     cmdline.2 *     - line number within window of command line
     cmdline.3 *     - contents of command line

     * these values are only returned with <EXTRACT>

COLOR [*|area]
COLOUR [*|area]
     Displays the current color settings for the file.
     Set by <SET COLOR> or <SET COLOUR>. The variable name is
     spelt the save way that the option is specified.

     With the ['*'] option, (or no option), returns color settings
     for all areas of the screen.
     (QE)

     color.0         - 28
     color.1         - FILEAREA    followed by its color
     color.2         - CURLINE     followed by its color
     color.3         - BLOCK       followed by its color
     color.4         - CBLOCK      followed by its color
     color.5         - CMDLINE     followed by its color
     color.6         - IDLINE      followed by its color
     color.7         - MSGLINE     followed by its color
     color.8         - ARROW       followed by its color
     color.9         - PREFIX      followed by its color
     color.10        - CPREFIX     followed by its color
     color.11        - PENDING     followed by its color
     color.12        - SCALE       followed by its color
     color.13        - TOFEOF      followed by its color
     color.14        - CTOFEOF     followed by its color
     color.15        - TABLINE     followed by its color
     color.16        - SHADOW      followed by its color
     color.17        - STATAREA    followed by its color
     color.18        - DIVIDER     followed by its color
     color.19        - RESERVED    followed by its color
     color.20        - NONDISP     followed by its color
     color.21        - HIGHLIGHT   followed by its color
     color.22        - CHIGHLIGHT  followed by its color
     color.23        - SLK         followed by its color
     color.24        - GAP         followed by its color
     color.25        - CGAP        followed by its color
     color.26        - ALERT       followed by its color
     color.27        - DIALOG      followed by its color
     color.28        - BOUNDMARKER followed by its color
     color.29        - FILETABS    followed by its color

     With the ['area'] option, returns color settings
     for the specified area of the screen.
     (QE)

     color.0         - 1
     color.1         - 'area' followed by its color

COLORING
COLOURING
     Displays the current color settings for the file. The variable
     name is spelt the same way that the option is specified.
     Set by <SET COLORING> or <SET COLOURING>.
     (QEMS)

     coloring.0      - 3
     coloring.1      - ON|OFF
     coloring.2      - AUTO|parser (if coloring.1 is ON)
     coloring.3      - parser (if coloring.1 is ON)

COLUMN
     Displays the current value of the focus column.
     (QE)

     column.0        - 1
     column.1        - Column number of focus column

COMPAT
     The settings for the compatibility mode. Set by <SET COMPAT>.
     (QEMS)

     compat.0        - 3
     compat.1        - THE|XEDIT|KEDIT|KEDITW|ISPF (compatibility LOOK)
     compat.2        - THE|XEDIT|KEDIT|KEDITW|ISPF (compatibility FEEL)
     compat.3        - THE|XEDIT|KEDIT|KEDITW|ISPF (compatibility KEYS)

CTLchar [*|char]
     The definitions of control characters that affect the display of
     <reserved line>s. Set by <SET CTLCHAR>.
     (QES)

     With the ['*'] option, (or no option), returns a list of all
     control characters that have been defined.

     ctlchar.0       - 3
     ctlchar.1       - ON|OFF
     ctlchar.2       - The character defined as the escape character.
     ctlchar.3       - List of defined control characters, if any.

     With the ['char'] option, returns color settings for the specified
     control character.

     ctlchar.0       - 1 if ctlchar.1 is OFF, otherwise 2
     ctlchar.1       - PROTECT|NOPROTECT|OFF
     ctlchar.2       - The color defined for this control character.

CURline
     The value and position of the <current line> within the current file.
     Set by <SET CURLINE>. Also provides the contents of the <focus line>
     and an indication of the status of the <focus line> for the current
     session.
     (QEMS)

     With <EXTRACT> command:

     curline.0       - 6
     curline.1       - curline setting
     curline.2       - line number within window of current line
     curline.3       - contents of <focus line>
     curline.4       - ON|OFF (ON if line has changed or been added this session)
     curline.5       - OLD|OLD CHANGED|NEW CHANGED
     curline.6       - selection level of focus line (same as select.1)

     Note: If <SET COMPAT> (feel) is set to XEDIT, curline.3 will always
     return the contents of the <current line>.

     The setting of curline.5 is as follows:
       OLD:            The line existed in its current form in the
                       file before THE began.
       OLD CHANGED:    The line was in the file before THE started,
                       but has been changed during the current editing
                       session.
       NEW CHANGED:    The line has been added to the file during
                       the current editing session.

     With <QUERY>, <MODIFY> and <STATUS> commands:

     curline.0       - 1
     curline.1       - curline setting

CURSor
     The position of the cursor within the displayed screen and file
     at the time the <EXTRACT> /CURSOR/ is issued and the position of
     the cursor at the time the <macro> was issued.
     If the cursor is not in the <filearea>, then line and column
     values will be set to -1.
     (QE)

     cursor.0        - 8
     cursor.1        - line number of cursor in screen (current)
     cursor.2        - column number of cursor in screen (current)
     cursor.3        - line number of cursor in file (current)
     cursor.4        - column number of cursor in file (current)
     cursor.5        - line number of cursor in screen (original)
     cursor.6        - column number of cursor in screen (original)
     cursor.7        - line number of cursor in file (original)
     cursor.8        - column number of cursor in file (original)

CURSORSTay
     The setting for the behaviour of the cursor after a file scroll.
     Set by <SET CURSORSTAY>.
     (QEMS)

     cursorstay.0    - 1
     cursorstay.1    - ON|OFF

DEFINE [key|mousedef IN window|*]
     Returns details about the commands associated with a keyboard key
     or mouse key.  The details returned are the same as those displayed
     by the <SHOWKEY> command.
     Set by <DEFINE>.
     (QEM)

     Three forms are available:

     With no parameter or '*', all details about all key and mouse
     definitions are returned. (Similar to <SHOWKEY> all)

     define.0        - number of all definitions
     define.1        - command assigned to the 1st key or mouse
     define.n        - command assigned to the nth key or mouse

     With a 'key' mnemonic specified, details about this one key are
     returned.

     define.0        - 1
     define.1        - command assigned to the key

     With a 'mousedef IN window' specified, details about this one mouse
     mapping for a window are returned.

     define.0        - 1
     define.1        - command assigned to the mouse for the window

DEFSORT
     Specifies the order in which files are sorted in the DIR.DIR file.
     Set by <SET DEFSORT>.
     (QEMS)

     defsort.0       - 2
     defsort.1       - DIRECTORY|NAME|SIZE|TIME|DATE|OFF
     defsort.2       - ASCENDING|DESCENDING

DIRFILEID
     The value of the path and filename of the focus line in a DIR.DIR
     file.
     (E)

     dirfileid.0     - 2
     dirfileid.1     - full path of directory
     dirfileid.2     - file name at focus line

DIRInclude
     The value of the file type masks currently in place for display
     in DIR.DIR file. Set by <SET DIRINCLUDE>.
     (QEMS)

     dirinclude.0    - 1
     dirinclude.1    - list of file types shown in DIR.DIR file

DISPlay
     Indicates the range of selection levels current. Set by
     <SET DISPLAY>.
     (QEMS)

     display.0       - 2
     display.1       - display low value
     display.2       - display high value

ECOLOR [*|category]
ECOLOUR [*|category]
     Displays the current color settings for syntax highlighting.
     Set by <SET ECOLOR> or <SET ECOLOUR>. The variable name is
     spelt the save way that the option is specified.

     With the ['*'] option, (or no option), returns color settings
     for all categories.
     (QE)

     ecolor.0        - 35
     ecolor.1        - A followed by its color
     ecolor.2        - B followed by its color
     ecolor.3        - C followed by its color
     ecolor.4        - D followed by its color
     ecolor.5        - E followed by its color
     ecolor.6        - F followed by its color
     ecolor.7        - G followed by its color
     ecolor.8        - H followed by its color
     ecolor.9        - I followed by its color
     ecolor.10       - J followed by its color
     ecolor.11       - K followed by its color
     ecolor.12       - L followed by its color
     ecolor.13       - M followed by its color
     ecolor.14       - N followed by its color
     ecolor.15       - O followed by its color
     ecolor.16       - P followed by its color
     ecolor.17       - Q followed by its color
     ecolor.18       - R followed by its color
     ecolor.19       - S followed by its color
     ecolor.20       - T followed by its color
     ecolor.21       - U followed by its color
     ecolor.22       - V followed by its color
     ecolor.23       - W followed by its color
     ecolor.24       - X followed by its color
     ecolor.25       - Y followed by its color
     ecolor.26       - Z followed by its color
     ecolor.27       - 1 followed by its color
     ecolor.28       - 2 followed by its color
     ecolor.29       - 3 followed by its color
     ecolor.30       - 4 followed by its color
     ecolor.31       - 5 followed by its color
     ecolor.32       - 6 followed by its color
     ecolor.33       - 7 followed by its color
     ecolor.34       - 8 followed by its color
     ecolor.35       - 9 followed by its color

     With the ['category'] option, returns color settings
     for the specified syntax highlighting category.
     (E)

     color.0         - 1
     color.1         - ['category'] followed by its color

EFILEId
     The original full filename of the current file. See <SET FILENAME> for a
     full description of the components of a file name. EFILEID.2 differs from
     the value returned by KEDIT.
     (QE)

     efileid.0       - 2
     efileid.1       - Original full file name.
     efileid.2       - Original file name used to edit the file.

EOF
     Indicates if the <current line> is on the <Bottom-of-File line>.
     (QES)

     eof.0           - 1
     eof.1           - ON|OFF

EOLout
     Returns the value of the end-of-line character(s).
     (QEMS)

     eolout.0        - 1
     eolout.1        - LF|CRLF|CR|NONE

EQUIVChar
     Returns the equivalence character. Set by <SET EQUIVCHAR>.
     (QEMS)

     equivchar.0     - 2
     equivchar.1     - equivalence character

ETMODE
     Indicates if extended display mode is set. Set by <SET ETMODE>.
     (QEMS)

     etmode.0        - 2
     etmode.1        - ON|OFF
     etmode.2        - character ranges if not all ON or OFF

FExt
     The extension of the current file. The characters following
     the trailing '.' character. Same as FType.
     (QEM)

     fext.0          - 1
     fext.1          - File extension.

FIELD
     Details about the current cursor field.
     (QE)

     field.0         - 4
     field.1         - contents of the cursor field
     field.2         - character under the cursor
     field.3         - column position in the cursor field
     field.4         - COMMAND|TEXT|PREFIX

FIELDWORD
     Details about the word closest to the cursor in the current cursor field.
     (E)

     fieldword.0     - 2
     fieldword.1     - word as defined by <SET WORD> ALPHANUM
     fieldword.2     - word as defined by <SET WORD> NONBLANK

FILEName
     The full filename of the current file, including any file
     extension.
     (QEM)

     filename.0      - 1
     filename.1      - Full file name.

FILESTATUS
     Details about the status of the file being edited.
     (QEM)

     filestatus.0    - 3
     filestatus.1    - sharing mode - NONE
     filestatus.2    - access type - READONLY|READWRITE
     filestatus.3    - end of line - CR/LF/CRLF/NONE

FMode
     The file mode of the current file. Under Un*x, this will
     always return an empty string. Other platforms returns the
     first character of the file's path. ie the disk drive.
     (QEM)

     fmode.0         - 1
     fmode.1         - File mode.

FName
     The fname portion of the current file. See <SET FILENAME> for a
     full description of the components of a file name.
     (QEM)

     fname.0         - 1
     fname.1         - File name.

FPath
     The path name of the current file. This includes a trailing
     directory separator.
     (QEM)

     fpath.0         - 1
     fpath.1         - File path.

FType
     The extension of the current file. The characters following
     the trailing '.' character.
     (QEM)

     ftype.0         - 1
     ftype.1         - File extension.

FULLFName
     Indicates if the fully qualified filename is displayed on the
     <idline>.
     (QEMS)

     fullfname.0     - 1
     fullfname.1     - ON|OFF

GETENV variable
     The value of the supplied environment 'variable' or ***invalid***
     if the 'variable' does not exist. On platforms other than Unix
     the supplied variable name is uppercased before obtaining the
     environment variable value.
     (E)

     getenv.0        - 1
     getenv.1        - value of variable

HEX
     Indicates if hexadecimal values in commands are interpreted
     as hexadecimal values or not. Set by <SET HEX>.
     (QEMS)

     hex.0           - 1
     hex.1           - ON|OFF

HEADer
     Returns details about which sections of a TLD file are to be
     applied to the current view. Do not rely on the position of a
     particular header in the returned stem. Set by <SET HEADER>.
     (QE)

     header.0        - 11
     header.1        - section ON|OFF
     header.2        - section ON|OFF
     header.3        - section ON|OFF
     header.4        - section ON|OFF
     header.5        - section ON|OFF
     header.6        - section ON|OFF
     header.7        - section ON|OFF
     header.8        - section ON|OFF
     header.9        - section ON|OFF
     header.10       - section ON|OFF
     header.11       - section ON|OFF

HEXDISPlay
     Indicates if the current character is displayed on the <status line>.
     Set by <SET HEXDISPLAY>.
     (QEMS)

     hexdisplay.0    - 1
     hexdisplay.1    - ON|OFF

HEXShow
     Returns details of HEXSHOW placement. Set by <SET HEXSHOW>.
     (QEMS)

     hexshow.0       - 2
     hexshow.1       - ON|OFF
     hexshow.2       - line displaying first hexshow line

HIGHlight
     Returns details of HIGHLIGHT settings. Set by <SET HIGHLIGHT>.
     (QEMS)

     highlight.0     - 1 or 3 (if highlight.1 = SELECT)
     highlight.1     - OFF|ALTERED|TAGGED|SELECT
     highlight.2     - minimum (or only) selection level for SELECT
     highlight.3     - maximum selection level for SELECT

IDline
     Indicates if the <idline> is displayed for a file. Set by
     <SET IDLINE>.
     (QEMS)

     idline.0        - 1
     idline.1        - ON|OFF

IMPMACro
     Indicates if implied <macro> processing is on or off. Set by
     <SET IMPMACRO>.
     (QEMS)

     impmacro.0      - 1
     impmacro.1      - ON|OFF

IMPOS
     Indicates if implied operating system command processing is on
     or off. Set by <SET IMPOS> or <SET IMPCMSCP>.
     (QEMS)

     impos.0         - 1
     impos.1         - ON|OFF

INPUTMode
     Indicates the inputmode for the current view.
     Set by <SET INPUTMODE>.
     (QEMS)

     inputmode.0     - 1
     inputmode.1     - OFF|FULL|LINE

INSERTmode
     Indicates if currently in insert mode or overstrike mode. Set by
     <SET INSERTMODE>.
     (QEMS)

     insertmode.1    - 1
     insertmode.1    - ON|OFF

LASTmsg
     Return the text of the last error message generated.
     (E)

     lastmsg.0       - 1
     lastmsg.1       - text of last message.

LASTKEY [n]
     Return information about the last key, or the nth last key pressed.
     (E)

     lastkey.0       - 4
     lastkey.1       - name of the key.
     lastkey.2       - character associated with the key (if applicable)
     lastkey.3       - curses mnemonic code
     lastkey.4       - shift status. see <READV> for details

LASTOP [command|*]
     Returns the last operand from selected commands.  command can be
     one of ALter, Change, CLocate COUnt, Find, Locate SCHange or TFind.
     (QE)
     EXTract /LASTOP command/ sets:

     lastop.0        - 1
     lastop.1        - command and its last operand

     If no command is supplied as a parameter or '*' is passed,
     details of all last operands are set as follows:

     lastop.0        - 8
     lastop.1        - "alter" and its last operand (not implemented)
     lastop.2        - "change" and its last operand
     lastop.3        - "clocate" and its last operand (not implemented)
     lastop.4        - "count" and its last operand (not implemented)
     lastop.5        - "find" and its last operand
     lastop.6        - "locate" and its last operand
     lastop.7        - "schange" and its last operand
     lastop.8        - "tfind" and its last operand (not implemented)

LASTRC
     Returns the return code from last command issued from <command line>.
     (QES)

     lastrc.0        - 1
     lastrc.1        - Last return code.

LENgth
     Length of the current line.
     (QES)

     length.0        - 1
     length.1        - Length of current line.

LIne
     Line number of <focus line> in current file; or <current line>
     if <SET COMPAT> (feel) is set to XEDIT.
     (QES)

     line.0          - 1
     line.1          - Line number

LINEFLAG
     Returns information about the flags set on the <focus line>, or
     <current line> if <SET COMPAT> (feel) is set to XEDIT.
     (QEMS)

     lineflag.0      - 3
     lineflag.1      - NEW|NONEW
     lineflag.2      - CHANGE|NOCHANGE
     lineflag.3      - TAG|NOTAG

LINENd
     Indicates if multiple commands allowed on command line
     and the delimiter. Set by <SET LINEND>.
     (QEMS)

     linend.0        - 2
     linend.1        - ON|OFF
     linend.2        - delimiter

LScreen
     Displays the size of the current screen and window. Also
     shows the upper left corner of the window.
     (QE)

     lscreen.0       - 6
     lscreen.1       - height of current screen
     lscreen.2       - width of current screen
     lscreen.3       - screen line of upper left corner of screen
     lscreen.4       - screen column of upper left corner of screen
     lscreen.5       - height of display
     lscreen.6       - width of display

MACRO
     Indicates if macros are executed before commands. Set by
     <SET MACRO>.
     (QEMS)

     macro.0         - 1
     macro.1         - ON|OFF

MACROExt
     The current setting for a macro's file extension. Set by
     <SET MACROEXT>.
     (QEMS)

     macroext.0      - 1
     macroext.1      - Default file extension

MACROPath
     The path that THE looks for by default for macro files. Set by
     <SET MACROPATH>.
     (QEMS)

     macropath.0     - 1
     macropath.1     - Path for macro files.

MARgins
     The settings for left and right margins and paragraph indent.
     Set by <SET MARGINS>.
     (QEMS)

     margins.0       - 3
     margins.1       - left column
     margins.2       - right column
     margins.3       - indent value (column or offset from left margin)

MONITOR
     Indicates if the combination of monitor and the curses package
     supports colour. If the curses package supports colour, then
     monitor.1 is set to COLOR and monitor.2 can be COLOR or MONO
     depending on whether the monitor supports colour. If monitor.1
     is MONO then monitor.2 will also be set to MONO.
     (QE)

     monitor.0       - 2
     monitor.1       - COLOR|MONO
     monitor.2       - COLOR|MONO

MOUSE
     Indicates if the mouse is suppoted as an input device. Set by
     <SET MOUSE>.
     (QEMS)

     mouse.0         - 1
     mouse.1         - ON|OFF

MSGLine
     Returns details of where the <message line> is displayed. Set by
     <SET MSGLINE>.
     (QEMS)

     msgline.0       - 4
     msgline.1       - ON
     msgline.2       - line position of <message line>
     msgline.3       - number of message lines available
     msgline.4       - OVERLAY (returned for compatibility reasons)

MSGMode
     Indicates if messages are suppressed. Set by <SET MSGMODE>.
     (QEMS)

     msgmode.0       - 1
     msgmode.1       - ON|OFF

NBFile
     Returns with the number of files currently in the <ring>.
     (QES)

     nbfile.0        - 1
     nbfile.1        - Number of files in ring

NBScope
     Returns with the number of lines currently in scope. If SCOPE ALL
     is in effect, this will be the same number as SIZE.
     (QE)

     nbscope.0       - 1
     nbscope.1       - Number of lines in scope
     nbscope.2       - Line number of focus line in scope

NEWLines
     Indicates if NEWLINES variable is set to LEFT or ALIGNED.
     Set by <SET NEWLINES>.
     (QEMS)

     newlines.0      - 1
     newlines.1      - ALIGNED|LEFT

NONDisp
     Returns the character that is displayed for extended characters
     that are not displayed. Set by <SET NONDISP>.
     (QEMS)

     nondisp.0       - 1
     nondisp.1       - char

NUMber
     Indicates if line numbers are displayed in the prefix area.
     Set by <SET NUMBER>.
     (QEMS)

     number.0        - 1
     number.1        - ON|OFF

PAGEWRAP
     Indicates if the scrolling the file view using the <FORWARD>
     and <BACKWARD> commands will wrap if the cursor is at the
     <Bottom-of-File line> or <Top-of-File line> respectively.
     Set by <SET PAGEWRAP>.
     (QEMS)

     pagewrap.0      - 1
     pagewrap.1      - ON|OFF

PARSER [*|parser]
     Displays the filename of the THE Language Defintion file from which
     a syntax highlighting parser was loaded. Set by <SET PARSER>.
     (QES)

     parser.0        - 2
     parser.1        - Name of parser
     parser.2        - Filename of TLD file

     If no parser is supplied as a parameter or '*' is passed,
     details of all parsers are set as follows:

     parser.0        - number of parsers currently defined
     parser.1        - name and filename of first parser
     parser.i        - name and filename of ith parser

PENDing [BLOCK] [OLDNAME] name|* [target1 [target2]
     Returns information about pending prefix commands.
     (E)

     pending.0       - 4
     pending.1       - line number in file
     pending.2       - newname - actual name entered in prefix area
     pending.3       - oldname - original name of macro after synonym resolution
     pending.4       - BLOCK or null

Point [*]
     Returns the name and line number of the <focus line>, or names and
     line numbers of all lines in a file if '*' is specified.
     If <SET COMPAT> (feel) is set to XEDIT, then the name and line number
     of the <current line> is returned, rather than the name and line
     number of the <focus line>.

     (QE)

     With no arguments:

     point.0         - 0 or 1       (0 if focus line not named)
     point.1         - line number and name of line (if line is named)

     With ['*'] argument:
     (E)

     point.0         - number of named lines in the file
     point.1         - line number and name for first named line
     point.n         - line number and name for nth named line

     Only point.0 and point.1 are available using implied extract
     functions.

POSition
     Indicates if LINE/COL is displayed on <idline>. Set by
     <SET POSITION>.
     (QMS)

     position.0      - 1
     position.1      - ON|OFF

     The current/focus line/column is also returned via <EXTRACT>.
     (E)

     position.0      - 3
     position.1      - ON|OFF
     position.2      - current or focus line
     position.3      - current or focus column

PREfix [Synonym [*]]
     Indicates if prefix is displayed for the view and if displayed
     where is is displayed. See <SET PREFIX>.
     (QEMS)

     prefix.0        - 1 or 2     (1 if prefix.1 is OFF, 2 otherwise)
     prefix.1        - ON|OFF|NULLS
     prefix.2        - LEFT|RIGHT        (if prefix.1 is ON or NULLS)
     prefix.3        - width of prefix area
     prefix.4        - width of prefix gap

     With ['Synonym'] option, the name of the macrofile (oldname)
     is returned that is associated with the synonym. If name
     is not a synonym then name is returned.
     (E)

     prefix.0        - 1
     prefix.1        - oldname

     With ['Synonym' ['*']] option, all prefix synonyms are returned.
     (E)

     prefix.0        - number of prefix synonyms
     prefix.1        - newname oldname
     prefix.n        - newname oldname

PRINTER
     Returns the value of the printer port or spooler. Set by
     <SET PRINTER>
     (QEMS)

     printer.0       - 1
     printer.1       - port or spooler name

READONLY
     Indicates if the file being edited is allowed to be alterered.
     Set by <SET READONLY>.
     (QEMS)

     readonly.0      - 1
     readonly.1      - ON|OFF|FORCE

REGEXP
     Return the type of regular expression syntax currently used
     for targets.
     Set by <SET REGEXP>.
     (QEMS)

     regexp.0        - 1
     regexp.1        - EMACS|AWK|POSIX_AWK|GREP|EGREP|POSIX_EGREP|SED|POSIX_BASIC|POSIX_MINIMAL_BASIC|POSIX_EXTENDED|POSIX_MINIMAL_EXTENDED

REPROFile
     Indicates if the specified (or implied) profile file is re-executed each
     time a file is added to the ring.
     Set by <SET REPROFILE>.
     (QEMS)

     reprofle.0      - 1
     reprofle.1      - ON|OFF

RESERved [*]
     Return with list of the screen rows that have been reserved.
     Set by <SET RESERVED>.
     (QES)

     reserved.0      - 0 if no reserved lines; 1 otherwise
     reserved.1      - list of screen row numbers reserved

     With ['*'] option, the line number, followed by the colour
     specification and <reserved line> contents are returned; one
     variable for each reserved line.
     (E)

     reserved.0      - the number of reserved lines
     reserved.1      - first reserved line
     reserved.2      - second reserved line
     reserved.n      - nth reserved line

REXX
     Returns the version details of the Rexx interpreter (if any) in
     the same format as PARSE VERSION.
     (QE)

     rexx.0          - 1
     rexx.1          - Version strings

REXXOUTput
     Indicates if <REXX> output is captured to a file or not and the
     line number limit of lines to be displayed. Set by <SET REXXOUTPUT>.
     (QEMS)

     rexxoutput.0    - 2
     rexxoutput.1    - FILE|DISPLAY
     rexxoutput.2    - line number limit

RING
     Returns details of each file being edited.
     (QE)

     With <SET COMPAT> (feel) set to XEDIT, the values set are:

     ring.0          - number of variables returned (ring.1 + 1)
     ring.1          - number of files in the <ring>
     ring.2          - IDLINE of first file in the ring
     ring.3          - IDLINE of second file in the ring
     ring.n          - IDLINE of nth file in the ring

     With <SET COMPAT> (feel) set to THE or KEDIT, the values set are:

     ring.0          - number of files in the <ring>
     ring.1          - IDLINE of first file in the ring
     ring.2          - IDLINE of second file in the ring
     ring.n          - IDLINE of nth file in the ring

     Only ring.0 and ring.1 are available using implied extract
     functions.

SCALe
     Returns details of <scale line>. Set by <SET SCALE>.
     (QEMS)

     scale.0         - 2
     scale.1         - ON|OFF
     scale.2         - line displaying scale line

SCOPE
     Returns information about whether <shadow line>s will be affected by
     commands or not. Set by <SET SCOPE>.
     (QEMS)

     scope.0         - 1
     scope.1         - ALL|DISPLAY

SCReen
     Returns the number and orientation of THE screens. Set by
     <SET SCREEN>.
     (QEMS)

     screen.0        - 2
     screen.1        - Number of screens displayed
     screen.2        - HORIZONTAL|VERTICAL

SELect
     Returns the selection level of the <focus line> and the
     maximum selection level for the file. Set by <SET SELECT> and
     <ALL> commands. If <SET COMPAT> (feel) XEDIT is set, then the
     selection level of the <current line> is returned instead of
     the selection level of the <focus line>.
     (QES)

     select.0        - 2
     select.1        - selection level of focus line
     select.2        - maximum selection level for file

SHADow
     Returns the status of the display of shadow lines. Set by
     <SET SHADOW>.
     (QEMS)

     shadow.0        - 1
     shadow.1        - ON|OFF

SHOWkey key
     Returns the commands and parameters assigned to the 'key'
     passed as an argument.  This keyvalue is returned as
     READV.3 from a call to <READV> KEY.
     (E)

     showkey.0       - the number of commands assigned
     showkey.1       - first command/parameter assigned
     showkey.n       - last command/parameter assigned

     If no 'key' supplied as a parameter:

     showkey.0       - 1
     showkey.1       - INVALID KEY

     This item is not available as an implied extract function.

SIze
     Returns the number of lines in the current file.
     (QES)

     size.0          - 1
     size.1          - Lines in current file.

STATUSLine
     Indicates if the <status line> is displayed and if so, where.
     Set by <SET STATUSLINE>.
     (QEMS)

     statusline.0    - 1
     statusline.1    - TOP|BOTTOM|OFF

STAY
     Indicates if the focus line stays where it is after a successful
     THE command or an unsuccessful <LOCATE> command.  Set by <SET STAY>.
     (QEMS)

     stay.0          - 1
     stay.1          - ON|OFF

SYNonym [*|name]
     Indicates if synonym processing is on or off. Set by <SET SYNONYM>
     command.
     (QEMS)

     synonym.0       - 1
     synonym.1       - ON|OFF

     With ['name'] option, details about the defined synonym are
     returned.
     (QE)

     synonym.0       - 4
     synonym.1       - synonym name
     synonym.2       - length of minimum abbreviation
     synonym.3       - definition
     synonym.4       - linend character (if specified)

     With ['*'] option, details about all defined synonyms are
     returned.
     (QE)

     synonym.0       - number of synonyms defined
     synonym.1       - synonym definition of first synonym
     synonym.n       - synonym definition of nth synonym

TABKey
     Returns settings about behaviour of TAB key.
     tabkey.1 indicates behaviour while not in insert mode
     tabkey.2 indicates behaviour while in insert mode
     Set by <SET TABKEY>.
     (QEMS)

     tabkey.0        - 2
     tabkey.1        - TAB|CHARACTER
     tabkey.2        - TAB|CHARACTER

TABLine
     Returns details of if and where the <tab line> is displayed.
     Set by <SET TABLINE>.
     (QEMS)

     tabline.0       - 2
     tabline.1       - ON|OFF
     tabline.2       - line displaying tab line

TABS
     Returns settings about tab columns. Set by <SET TABS>.
     (QEMS)

     tabs.0          - 1
     tabs.1          - actual tab columns or "INCR n"

TABSIn
     Indicates if TABSIN processing is on or off and the size of the
     tabs setting. Set by <SET TABSIN>.
     (QEMS)

     tabsin.0        - 2
     tabsin.1        - ON|OFF
     tabsin.2        - size of tabs

TABSOut
     Indicates if TABSOUT processing is on or off and the size of the
     tabs setting. Set by <SET TABSOUT>.
     (QEMS)

     tabsout.0       - 2
     tabsout.1       - ON|OFF
     tabsout.2       - size of tabs

TARGETSAVE
     Indicates the type of targets saved for subsequent LOCATE commands.
     Set by <SET TARGETSAVE>.
     (QEMS)

     targetsave.0    - 1
     targetsave.1    - ALL|NONE|list of target types

TERMinal
     Identifies the terminal type currently being used.
     (QES)

     terminal.0      - 1
     terminal.1      - DOS|OS2|X11|WIN32|$TERM value under Unix

THIGHlight
     Indicates if text highlighting is on. ie a found string target is
     highlighted.
     Set by <SET THIGHLIGHT>
     (QEMS)

     thighlight.0    - 1
     thighlight.1    - ON|OFF

TIMECHECK
     Indicates the status of TIMECHECK for the current file.
     (QEMS)

     timecheck.0     - 1
     timecheck.1     - ON|OFF

TOF
     Indicates if the <current line> is on the <Top-of-File line>.
     (QES)

     tof.0           - 1
     tof.1           - ON|OFF

TOFEOF
     Indicates if the <Top-of-File line> and the <Bottom-of-File line>
     are displayed.
     (QEMS)

     tofeof.0        - 1
     tofeof.1        - ON|OFF

TRAILING
     Indicates how trailing blanks on lines are handled when the file
     is saved. Set by <SET TRAILING>
     (QEMS)

     trailing.0      - 1
     trailing.1      - ON|OFF|EMPTY|SINGLE

TYPEAhead
     Indicates if THE will wait until all keyboard input has been
     processed before updating the screen display. Set by
     <SET TYPEAHEAD>.
     (QEMS)

     typeahead.0     - 1
     typeahead.1     - ON|OFF

UNDOING
     Indicates if the undo facility is available or not.
     Set by <SET UNDOING>
     (QEMS)

     undoing.0       - 1
     undoing.1       - ON|OFF

UNTAA
     Indicates if "unsigned numbers are absolute".
     Set by <SET UNTAA>
     (QEMS)

     untaa.0         - 1
     untaa.1         - ON|OFF

Verify
     Returns verify column settings. Set by <SET VERIFY>.
     (QEMS)

     verify.0        - 1
     verify.1        - Column pair of verify start and end columns.

VERShift
     Returns the value of the <vershift> internal variable.
     (QES)

     vershift.0      - 1
     vershift.1      - VERSHIFT value

VERSION
     Returns information about name of application (THE) and version
     information.
     (QES)

     version.0       - 4
     version.1       - THE
     version.2       - version string eg. 1.5
     version.3       - platform version (DOS,OS2,UNIX,X11,WIN32)
     version.4       - version status information eg. release date, beta

Width
     Returns maximum line width setting. Set by -w command line switch
     on starting THE.
     (QEMS)

     width.0         - 1
     width.1         - Maximum line width value.

WORD
     Specifies how THE defines a word. Set by <SET WORD>.
     (QEMS)

     word.0          - 1
     word.1          - ALPHANUM|NONBLANK

WORDWrap
     Indicates if WORDWRAP is on or off. Set by <SET WORDWRAP>.
     (QEMS)

     wordwrap.0      - 1
     wordwrap.1      - ON|OFF

WRap
     Indicates if WRAP is on or off. Set by <SET WRAP>.
     (QEMS)

     wrap.0          - 1
     wrap.1          - ON|OFF

XTERMinal
     Returns the current value of the X11 terminal program.
     Only applicable in X version. Set by <SET XTERMINAL>.
     (QEMS)

     xterminal.0     - 1
     xterminal.1     - X11 terminal program

Zone
     Returns zone column settings. Set by <SET ZONE>.
     (QEMS)

     zone.0          - 2
     zone.1          - Zone start column
     zone.2          - Zone end column



========================================================================
IMPLIED EXTRACT
========================================================================

     The above <REXX> variables set by the <EXTRACT> command may also
     be obtained by a REXX macro as an implied EXTRACT. Each variable
     above that may be set by an explicit EXTRACT command may also be
     eg. The REXX commands:

         'EXTRACT /SIZE/CURLINE/'
         Say size.1 curline.1

     may be substituted with:

         Say size.1() curline.1()


========================================================================
BOOLEAN FUNCTIONS
========================================================================

     THE also provides other information to the REXX interpreter via
     boolean functions. These functions return either '1' or '0'
     depending on the information queried.

after()
     Returns '1' if the cursor is currently after the last non-blank
     character on the line, or if the line is blank.

altkey()
     Returns '1' if at the time the last key was pressed, the ALT
     key was also being held down.

alt()
     Returns '1' if the file being edited has changed since the
     last SAVE. ie. if the value of alt.2 is non zero.

blank()
     Returns '1' if the line the <cursor field> is completely blank.

batch()
     Returns '1' if THE is being run in batch mode. ie THE was
     started with the -b switch.

block()
     Returns '1' if the marked <block> is within the current view.

before()
     Returns '1' if the cursor is currently before the first non-blank
     character on the line, or if the line is blank.

bottomedge()
     Returns '1' if the cursor is on the bottom edge of the <filearea>
     or <prefix area>.

command()
     Returns '1' if the <command line> is on. This is different to
     the definition in KEDIT; "Returns '1' if the cursor is on the
     command line." To get the equivalent KEDIT functionality
     use incommand();

ctrl()
     Returns '1' if at the time the last key was pressed, the CTRL
     key was also being held down.

current()
     Returns '1' if the cursor is on the <current line>.

dir()
     Returns '1' if the current file is the special DIR.DIR file.

end()
     Returns '1' if the cursor is on the last non-blank character on
     the line.

eof()
     Returns '1' if the cursor is on the <Bottom-of-File line>
     and the cursor is not on the <command line>.

first()
     Returns '1' if the cursor is in column 1 of the current window.

focuseof()
     Returns '1' if the focus line is the <Bottom-of-File line>
     whether the cursor is on it or not.

focustof()
     Returns '1' if the <focus line> is the <Top-of-File line>
     whether the cursor is on it or not.

inblock()
     Returns '1' if the cursor is in the marked <block>.

incommand()
     Returns '1' if the cursor is on the <command line>.

initial()
     Returns '1' if the function is called from the <profile>.

inprefix()
     Returns '1' if the cursor is located in the <prefix area>.

leftedge()
     Returns '1' if the cursor is on the left edge of the <filearea>.

modifiable()
     Returns '1' if the cursor is located in an area that can be changed.
     ie. not on <Top-of-File line> or <Bottom-of-File line> nor on a
     <shadow line>.

rightedge()
     Returns '1' if the cursor is on the right edge of the <filearea>.

shadow()
     Returns '1' if the cursor is on a <shadow line>.

shift()
     Returns '1' if at the time the last key was pressed, the SHIFT
     key was also being held down.

spacechar()
     Returns '1' if the cursor is on a space character.

tof()
     Returns '1' if the cursor is on the <Top-of-File line>
     and the cursor is not on the <command line>.

topedge()
     Returns '1' if the cursor is on the top edge of the <filearea>.

verone()
     Returns '1' if the column 1 of the file is being displayed in
     column 1.

========================================================================
OTHER FUNCTIONS
========================================================================

     The following functions provide features to simplify THE macros
     written in REXX.

valid_target(target[,anything])
     The first argument is the <target> to be validated. If a second,
     optional argument is supplied, the target to be validated can
     consist of a target followed by any optional characters. This can
     be useful if the arguments to a macro consist of a target followed
     by another argument. If a valid target is supplied, the remainder
     of the string passed to valid_target() is returned following the
     first line affected and the number of lines to the target.

     Returns 'ERROR' if the supplied target is invalid.
     Returns 'NOTFOUND' if the supplied target is valid, but not found.

     If a valid target, returns the first line affected by the target
     followed by the number of lines to the target, and optionally the
     remainder of the argument. eg.

     if the focus line is 12 and valid_target() is called as

         result = valid_target(":7")      ===> result = "12 -5"

     if the focus line is 12 and valid_target is called as

         result = valid_target(":7 /fred/",junk), ===> result = "12 -5 /fred/"

run_os(command[,stdin_stem[,stdout_stem[,stderr_stem]]])
     This function allows the macro writer to call an operating system
     command and have the standard streams; 'stdin', 'stdout' and 'stderr'
     redirected to or from REXX arrays.

     The first argument is the operating system command to execute.
     The command can include any command line switches appropriate
     to the command.

     All other arguments comprise a stem name (including a trailing '.')
     which refers to the REXX arrays where 'stdin', 'stdout' and 'stderr'
     are to be redirected.

     As with all REXX arrays, the value of the 0th element (stem.0)
     contains the number of elements in the array.

     The only restriction with the names of the stem variables is
     that the stem name for the 'stdin' stem cannot be the same as the
     stem for 'stdout' or 'stderr'.

     The stem name for 'stdout' and 'stderr' can be the same; the contents
     of the resulting output stems will consist of 'stdout' and 'stderr' in
     the order that the command generates this output.

     Return values:

          0    - successful
          1005 - invalid argument or syntax
          1012 - problems with system redirection of streams
          1094 - out of memory
          1099 - error interfacing to REXX interpreter

     all other numbers, return code from operating system command

     eg. to spell check the words "The Hessling Editr" with ispell

         in.0 = 3
         in.1 = "The"
         in.2 = "Hessling"
         in.3 = "Editr"
         rc = run_os("ispell -a","in.","out.")

     sets:

          out.0 --> 3
          out.1 --> "*"
          out.2 --> "#"
          out.3 --> "& edits edit editor"

**man-end**********************************************************************/


/***********************************************************************/
/* Keep the following items in alphabetic order of name.               */
/***********************************************************************/
QUERY_ITEM _THE_FAR query_item[] =
{
   {(CHARTYPE *)"alt",             3, 3,ITEM_ALT,           2, 2,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_alt                    },
   {(CHARTYPE *)"arbchar",         7, 3,ITEM_ARBCHAR,       3, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_arbchar                },
   {(CHARTYPE *)"autocolor",       9, 9,ITEM_AUTOCOLOR,     3, 0,          LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_autocolor              },
   {(CHARTYPE *)"autocolour",     10,10,ITEM_AUTOCOLOUR,    3, 0,          LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_autocolour             },
   {(CHARTYPE *)"autosave",        8, 2,ITEM_AUTOSAVE,      1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_autosave               },
   {(CHARTYPE *)"autoscroll",     10, 6,ITEM_AUTOSCROLL,    1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_autoscroll             },
   {(CHARTYPE *)"backup",          6, 4,ITEM_BACKUP,        1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_backup                 },
   {(CHARTYPE *)"beep",            4, 4,ITEM_BEEP,          1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_beep                   },
   {(CHARTYPE *)"block",           5, 5,ITEM_BLOCK,         0, 6,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_block                  },
   {(CHARTYPE *)"case",            4, 4,ITEM_CASE,          4, 4,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_case                   },
   {(CHARTYPE *)"clearerrorkey",  13, 6,ITEM_CLEARERRORKEY, 1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_clearerrorkey          },
   {(CHARTYPE *)"clearscreen",    11, 6,ITEM_CLEARSCREEN,   1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_clearscreen            },
   {(CHARTYPE *)"clock",           5, 5,ITEM_CLOCK,         1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_clock                  },
   {(CHARTYPE *)"cmdarrows",       9, 4,ITEM_CMDARROWS,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_cmdarrows              },
   {(CHARTYPE *)"cmdline",         7, 3,ITEM_CMDLINE,       2, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_cmdline                },
   {(CHARTYPE *)"color",           5, 5,ITEM_COLOR,         1, ATTR_MAX,   LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_color                  },
   {(CHARTYPE *)"coloring",        8, 8,ITEM_COLORING,      3, 3,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_coloring               },
   {(CHARTYPE *)"colour",          6, 5,ITEM_COLOUR,        1, ATTR_MAX,   LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_colour                 },
   {(CHARTYPE *)"colouring",       9, 9,ITEM_COLOURING,     3, 3,          LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT|QUERY_MODIFY,extract_colouring              },
   {(CHARTYPE *)"column",          6, 3,ITEM_COLUMN,        1, 1,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_column                 },
   {(CHARTYPE *)"compat",          6, 6,ITEM_COMPAT,        3, 3,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_compat                 },
   {(CHARTYPE *)"ctlchar",         7, 7,ITEM_CTLCHAR,       3, 3,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_ctlchar                },
   {(CHARTYPE *)"curline",         7, 3,ITEM_CURLINE,       6, 6,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_curline                },
   {(CHARTYPE *)"cursor",          6, 4,ITEM_CURSOR,        8, 8,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_cursor                 },
   {(CHARTYPE *)"cursorstay",     10, 8,ITEM_CURSORSTAY,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_cursorstay             },
   {(CHARTYPE *)"define",          6, 6,ITEM_DEFINE,        1, 0,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT|QUERY_MODIFY,extract_define                 },
   {(CHARTYPE *)"defsort",         7, 7,ITEM_DEFSORT,       2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_defsort                },
   {(CHARTYPE *)"dirfileid",       9, 9,ITEM_DIRFILEID,     2, 2,          LVL_FILE,                         QUERY_EXTRACT             ,extract_dirfileid              },
   {(CHARTYPE *)"dirinclude",     10, 4,ITEM_DIRINCLUDE,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_dirinclude             },
   {(CHARTYPE *)"display",         7, 4,ITEM_DISPLAY,       2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_display                },
   {(CHARTYPE *)"ecolor",          6, 6,ITEM_ECOLOR,        2, ECOLOUR_MAX,LVL_VIEW,QUERY_QUERY|             QUERY_EXTRACT             ,extract_ecolor                 },
   {(CHARTYPE *)"ecolour",         7, 7,ITEM_ECOLOUR,       2, ECOLOUR_MAX,LVL_VIEW,QUERY_QUERY|             QUERY_EXTRACT             ,extract_ecolour                },
   {(CHARTYPE *)"efileid",         7, 6,ITEM_EFILEID,       2, 2,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT             ,extract_efileid                },
   {(CHARTYPE *)"eof",             3, 3,ITEM_EOF,           1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_eof                    },
   {(CHARTYPE *)"eolout",          6, 3,ITEM_EOLOUT,        1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_eolout                 },
   {(CHARTYPE *)"equivchar",       9, 9,ITEM_EQUIVCHAR,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_equivchar              },
   {(CHARTYPE *)"etmode",          6, 6,ITEM_ETMODE,        2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_etmode                 },
   {(CHARTYPE *)"fext",            4, 2,ITEM_FEXT,          1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_ftype                  },
   {(CHARTYPE *)"field",           5, 5,ITEM_FIELD,         4, 4,          LVL_VIEW,QUERY_QUERY|             QUERY_EXTRACT             ,extract_field                  },
   {(CHARTYPE *)"fieldword",       9, 9,ITEM_FIELDWORD,     2, 2,          LVL_VIEW,                         QUERY_EXTRACT             ,extract_fieldword              },
   {(CHARTYPE *)"filename",        8, 5,ITEM_FILENAME,      1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_filename               },
   {(CHARTYPE *)"filestatus",     10,10,ITEM_FILESTATUS,    3, 3,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_filestatus             },
   {(CHARTYPE *)"fmode",           5, 2,ITEM_FMODE,         1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_fmode                  },
   {(CHARTYPE *)"fname",           5, 2,ITEM_FNAME,         1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_fname                  },
   {(CHARTYPE *)"fpath",           5, 2,ITEM_FPATH,         1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_fpath                  },
   {(CHARTYPE *)"ftype",           5, 2,ITEM_FTYPE,         1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_ftype                  },
   {(CHARTYPE *)"fullfname",       9, 6,ITEM_FULLFNAME,     1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_fullfname              },
   {(CHARTYPE *)"getenv",          6, 6,ITEM_GETENV,        1, 1,          LVL_GLOB,                         QUERY_EXTRACT             ,extract_getenv                 },
   {(CHARTYPE *)"header",          4, 3,ITEM_HEADER,       11,11,          LVL_GLOB,QUERY_QUERY|             QUERY_EXTRACT             ,extract_header                 },
   {(CHARTYPE *)"hex",             3, 3,ITEM_HEX,           1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_hex                    },
   {(CHARTYPE *)"hexdisplay",     10, 7,ITEM_HEXDISPLAY,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_hexdisplay             },
   {(CHARTYPE *)"hexshow",         7, 4,ITEM_HEXSHOW,       2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_hexshow                },
   {(CHARTYPE *)"highlight",       9, 4,ITEM_HIGHLIGHT,     1, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_highlight              },
   {(CHARTYPE *)"idline",          6, 2,ITEM_IDLINE,        1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_idline                 },
   {(CHARTYPE *)"impmacro",        8, 6,ITEM_IMPMACRO,      1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_impmacro               },
   {(CHARTYPE *)"impos",           5, 5,ITEM_IMPOS,         1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_impos                  },
   {(CHARTYPE *)"inputmode",       9, 6,ITEM_INPUTMODE,     1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_inputmode              },
   {(CHARTYPE *)"insertmode",     10, 6,ITEM_INSERTMODE,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_insertmode             },
   {(CHARTYPE *)"lastkey",         7, 7,ITEM_LASTKEY,       4, 4,          LVL_GLOB,                         QUERY_EXTRACT             ,extract_lastkey                },
   {(CHARTYPE *)"lastmsg",         7, 4,ITEM_LASTMSG,       1, 1,          LVL_GLOB,                         QUERY_EXTRACT             ,extract_lastmsg                },
   {(CHARTYPE *)"lastop",          6, 6,ITEM_LASTOP,        8, 8,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_lastop                 },
   {(CHARTYPE *)"lastrc",          6, 6,ITEM_LASTRC,        1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_lastrc                 },
   {(CHARTYPE *)"length",          6, 3,ITEM_LENGTH,        1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_length                 },
   {(CHARTYPE *)"line",            4, 2,ITEM_LINE,          1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_line                   },
   {(CHARTYPE *)"lineflag",        8, 8,ITEM_LINEFLAG,      3, 3,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_lineflag               },
   {(CHARTYPE *)"linend",          6, 5,ITEM_LINEND,        2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_linend                 },
   {(CHARTYPE *)"lscreen",         7, 2,ITEM_LSCREEN,       6, 6,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_lscreen                },
   {(CHARTYPE *)"macro",           5, 5,ITEM_MACRO,         1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_macro                  },
   {(CHARTYPE *)"macroext",        8, 6,ITEM_MACROEXT,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_macroext               },
   {(CHARTYPE *)"macropath",       9, 6,ITEM_MACROPATH,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_macropath              },
   {(CHARTYPE *)"margins",         7, 3,ITEM_MARGINS,       3, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_margins                },
   {(CHARTYPE *)"monitor",         7, 7,ITEM_MONITOR,       2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_monitor                },
   {(CHARTYPE *)"mouse",           5, 5,ITEM_MOUSE,         1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_mouse                  },
   {(CHARTYPE *)"msgline",         7, 4,ITEM_MSGLINE,       4, 4,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_msgline                },
   {(CHARTYPE *)"msgmode",         7, 4,ITEM_MSGMODE,       1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_msgmode                },
   {(CHARTYPE *)"nbfile",          6, 3,ITEM_NBFILE,        1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_nbfile                 },
   {(CHARTYPE *)"nbscope",         7, 3,ITEM_NBSCOPE,       2, 2,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_nbscope                },
   {(CHARTYPE *)"newlines",        8, 4,ITEM_NEWLINES,      1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_newlines               },
   {(CHARTYPE *)"nondisp",         7, 4,ITEM_NONDISP,       1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_nondisp                },
   {(CHARTYPE *)"number",          6, 3,ITEM_NUMBER,        1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_number                 },
   {(CHARTYPE *)"pagewrap",        8, 8,ITEM_PAGEWRAP,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_pagewrap               },
   {(CHARTYPE *)"parser",          6, 6,ITEM_PARSER,        5, 0,          LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_parser                 },
   {(CHARTYPE *)"pending",         7, 4,ITEM_PENDING,       4, 1,          LVL_VIEW,                         QUERY_EXTRACT             ,extract_pending                },
   {(CHARTYPE *)"point",           5, 1,ITEM_POINT,         1, 1,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_point                  },
   {(CHARTYPE *)"position",        8, 3,ITEM_POSITION,      3, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_position               },
   {(CHARTYPE *)"prefix",          6, 3,ITEM_PREFIX,        0, 4,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_prefix                 },
   {(CHARTYPE *)"printer",         7, 7,ITEM_PRINTER,       1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_printer                },
   {(CHARTYPE *)"readonly",        8, 8,ITEM_READONLY,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_readonly               },
   {(CHARTYPE *)"readv",           5, 5,ITEM_READV,         4, 1,          LVL_GLOB,QUERY_READV                                        ,extract_readv                  },
   {(CHARTYPE *)"regexp",          6, 6,ITEM_REGEXP,        1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_regexp                 },
   {(CHARTYPE *)"reprofile",       9, 6,ITEM_REPROFILE,     1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_reprofile              },
   {(CHARTYPE *)"reserved",        8, 5,ITEM_RESERVED,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_reserved               },
   {(CHARTYPE *)"rexx",            4, 4,ITEM_REXX,          1, 1,          LVL_GLOB,QUERY_QUERY                                        ,extract_rexx                   },
   {(CHARTYPE *)"rexxoutput",     10, 7,ITEM_REXXOUTPUT,    2, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_rexxoutput             },
   {(CHARTYPE *)"ring",            4, 4,ITEM_RING,          1, 0,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_ring                   },
   {(CHARTYPE *)"scale",           5, 4,ITEM_SCALE,         2, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_scale                  },
   {(CHARTYPE *)"scope",           5, 5,ITEM_SCOPE,         1, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_scope                  },
   {(CHARTYPE *)"screen",          6, 3,ITEM_SCREEN,        2, 2,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_screen                 },
   {(CHARTYPE *)"select",          6, 3,ITEM_SELECT,        2, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_select                 },
   {(CHARTYPE *)"shadow",          6, 4,ITEM_SHADOW,        1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_shadow                 },
   {(CHARTYPE *)"showkey",         7, 4,ITEM_SHOWKEY,       1, 1,          LVL_FILE,                         QUERY_EXTRACT             ,extract_showkey                },
   {(CHARTYPE *)"size",            4, 2,ITEM_SIZE,          1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_size                   },
   {(CHARTYPE *)"statusline",     10, 7,ITEM_STATUSLINE,    1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_statusline             },
   {(CHARTYPE *)"stay",            4, 4,ITEM_STAY,          1, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_stay                   },
   {(CHARTYPE *)"synonym",         7, 3,ITEM_SYNONYM,       1, 0,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_synonym                },
   {(CHARTYPE *)"tabkey",          6, 4,ITEM_TABKEY,        2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabkey                 },
   {(CHARTYPE *)"tabline",         7, 4,ITEM_TABLINE,       2, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabline                },
   {(CHARTYPE *)"tabs",            4, 4,ITEM_TABS,          1, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabs                   },
   {(CHARTYPE *)"tabsin",          6, 5,ITEM_TABSIN,        2, 2,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabsin                 },
   {(CHARTYPE *)"tabsout",         7, 5,ITEM_TABSOUT,       2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabsout                },
   {(CHARTYPE *)"targetsave",     10,10,ITEM_TARGETSAVE,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_targetsave             },
   {(CHARTYPE *)"terminal",        8, 4,ITEM_TERMINAL,      1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_terminal               },
   {(CHARTYPE *)"thighlight",      5, 5,ITEM_THIGHLIGHT,    1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_thighlight             },
   {(CHARTYPE *)"timecheck",       9, 9,ITEM_TIMECHECK,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_timecheck              },
   {(CHARTYPE *)"tof",             3, 3,ITEM_TOF,           1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_tof                    },
   {(CHARTYPE *)"tofeof",          6, 6,ITEM_TOFEOF,        1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tofeof                 },
   {(CHARTYPE *)"trailing",        8, 8,ITEM_TRAILING,      1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_trailing               },
   {(CHARTYPE *)"typeahead",       9, 5,ITEM_TYPEAHEAD,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_typeahead              },
   {(CHARTYPE *)"undoing",         7, 7,ITEM_UNDOING,       1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_undoing                },
   {(CHARTYPE *)"untaa",           5, 5,ITEM_UNTAA,         1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_untaa                  },
   {(CHARTYPE *)"verify",          6, 1,ITEM_VERIFY,        1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_verify                 },
   {(CHARTYPE *)"vershift",        8, 4,ITEM_VERSHIFT,      1, 1,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_vershift               },
   {(CHARTYPE *)"version",         7, 7,ITEM_VERSION,       4, 4,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_version                },
   {(CHARTYPE *)"width",           5, 1,ITEM_WIDTH,         1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_width                  },
   {(CHARTYPE *)"word",            4, 4,ITEM_WORD,          1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_word                   },
   {(CHARTYPE *)"wordwrap",        8, 5,ITEM_WORDWRAP,      1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_wordwrap               },
   {(CHARTYPE *)"wrap",            4, 2,ITEM_WRAP,          1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_wrap                   },
   {(CHARTYPE *)"xterminal",       9, 5,ITEM_XTERMINAL,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_xterminal              },
   {(CHARTYPE *)"zone",            4, 1,ITEM_ZONE,          2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_zone                   },
};
#define NUMBER_QUERY_ITEM ( sizeof( query_item ) / sizeof( QUERY_ITEM ) )

QUERY_ITEM _THE_FAR function_item[] =
{
   {(CHARTYPE *)"after",           5, 5,ITEM_AFTER_FUNCTION,         0, 1,LVL_VIEW,0                                                  ,extract_after_function         },
   {(CHARTYPE *)"alt",             3, 3,ITEM_ALT_FUNCTION,           0, 1,LVL_FILE,0                                                  ,extract_alt_function           },
   {(CHARTYPE *)"altkey",          6, 6,ITEM_ALTKEY_FUNCTION,        0, 1,LVL_GLOB,0                                                  ,extract_altkey_function        },
   {(CHARTYPE *)"batch",           5, 5,ITEM_BATCH_FUNCTION,         0, 1,LVL_GLOB,0                                                  ,extract_batch_function         },
   {(CHARTYPE *)"before",          6, 6,ITEM_BEFORE_FUNCTION,        0, 1,LVL_VIEW,0                                                  ,extract_before_function        },
   {(CHARTYPE *)"blank",           5, 5,ITEM_BLANK_FUNCTION,         0, 1,LVL_VIEW,0                                                  ,extract_blank_function         },
   {(CHARTYPE *)"block",           5, 5,ITEM_BLOCK_FUNCTION,         0, 1,LVL_VIEW,0                                                  ,extract_block_function         },
   {(CHARTYPE *)"bottomedge",     10,10,ITEM_BOTTOMEDGE_FUNCTION,    0, 1,LVL_VIEW,0                                                  ,extract_bottomedge_function    },
   {(CHARTYPE *)"command",         7, 7,ITEM_COMMAND_FUNCTION,       0, 1,LVL_VIEW,0                                                  ,extract_command_function       },
   {(CHARTYPE *)"ctrl",            4, 4,ITEM_CTRL_FUNCTION,          0, 1,LVL_GLOB,0                                                  ,extract_ctrl_function          },
   {(CHARTYPE *)"current",         7, 7,ITEM_CURRENT_FUNCTION,       0, 1,LVL_VIEW,0                                                  ,extract_current_function       },
   {(CHARTYPE *)"dir",             3, 3,ITEM_DIR_FUNCTION,           0, 1,LVL_FILE,0                                                  ,extract_dir_function           },
   {(CHARTYPE *)"end",             3, 3,ITEM_END_FUNCTION,           0, 1,LVL_VIEW,0                                                  ,extract_end_function           },
   {(CHARTYPE *)"eof",             3, 3,ITEM_EOF_FUNCTION,           0, 1,LVL_VIEW,0                                                  ,extract_eof_function           },
   {(CHARTYPE *)"first",           5, 5,ITEM_FIRST_FUNCTION,         0, 1,LVL_VIEW,0                                                  ,extract_first_function         },
   {(CHARTYPE *)"focuseof",        8, 8,ITEM_FOCUSEOF_FUNCTION,      0, 1,LVL_VIEW,0                                                  ,extract_focuseof_function      },
   {(CHARTYPE *)"focustof",        8, 8,ITEM_FOCUSTOF_FUNCTION,      0, 1,LVL_VIEW,0                                                  ,extract_focustof_function      },
   {(CHARTYPE *)"inblock",         7, 7,ITEM_INBLOCK_FUNCTION,       0, 1,LVL_VIEW,0                                                  ,extract_inblock_function       },
   {(CHARTYPE *)"incommand",       9, 9,ITEM_INCOMMAND_FUNCTION,     0, 1,LVL_VIEW,0                                                  ,extract_incommand_function     },
   {(CHARTYPE *)"initial",         7, 7,ITEM_INITIAL_FUNCTION,       0, 1,LVL_GLOB,0                                                  ,extract_initial_function       },
   {(CHARTYPE *)"inprefix",        8, 8,ITEM_INPREFIX_FUNCTION,      0, 1,LVL_VIEW,0                                                  ,extract_inprefix_function      },
   {(CHARTYPE *)"leftedge",        8, 8,ITEM_LEFTEDGE_FUNCTION,      0, 1,LVL_VIEW,0                                                  ,extract_leftedge_function      },
   {(CHARTYPE *)"modifiable",     10,10,ITEM_MODIFIABLE_FUNCTION,    0, 1,LVL_VIEW,0                                                  ,extract_modifiable_function    },
   {(CHARTYPE *)"rightedge",       9, 9,ITEM_RIGHTEDGE_FUNCTION,     0, 1,LVL_VIEW,0                                                  ,extract_rightedge_function     },
   {(CHARTYPE *)"run_os",          6, 6,ITEM_RUN_OS_FUNCTION,        0, 1,LVL_GLOB,0                                                  ,NULL                           },
   {(CHARTYPE *)"shadow",          6, 6,ITEM_SHADOW_FUNCTION,        0, 1,LVL_VIEW,0                                                  ,extract_shadow_function        },
   {(CHARTYPE *)"shift",           5, 5,ITEM_SHIFT_FUNCTION,         0, 1,LVL_GLOB,0                                                  ,extract_shift_function         },
   {(CHARTYPE *)"spacechar",       9, 9,ITEM_SPACECHAR_FUNCTION,     0, 1,LVL_VIEW,0                                                  ,extract_spacechar_function     },
   {(CHARTYPE *)"tof",             3, 3,ITEM_TOF_FUNCTION,           0, 1,LVL_VIEW,0                                                  ,extract_tof_function           },
   {(CHARTYPE *)"topedge",         7, 7,ITEM_TOPEDGE_FUNCTION,       0, 1,LVL_VIEW,0                                                  ,extract_topedge_function       },
   {(CHARTYPE *)"valid_target",   12,12,ITEM_VALID_TARGET_FUNCTION,  0, 1,LVL_VIEW,0                                                  ,NULL                           },
   {(CHARTYPE *)"verone",          6, 6,ITEM_VERONE_FUNCTION,        0, 1,LVL_VIEW,0                                                  ,extract_verone_function        },
};
#define NUMBER_FUNCTION_ITEM ( sizeof( function_item ) / sizeof( QUERY_ITEM ) )

CHARTYPE _THE_FAR *block_name[] = {
                          (CHARTYPE *)"",
                          (CHARTYPE *)"LINE",
                          (CHARTYPE *)"BOX",
                          (CHARTYPE *)"STREAM",
                          (CHARTYPE *)"COLUMN",
                          (CHARTYPE *)"WORD",
                          (CHARTYPE *)"CUA"
                         };

VALUE item_values[MAX_VARIABLES_RETURNED];

/***********************************************************************/
#ifdef HAVE_PROTO
static THE_PPC *in_range( THE_PPC *found_ppc, THE_PPC *curr_ppc, LINETYPE first_in_range, LINETYPE last_in_range )
#else
static THE_PPC *in_range( found_ppc, curr_ppc, first_in_range, last_in_range )
THE_PPC *found_ppc, *curr_ppc;
LINETYPE first_in_range, last_in_range;
#endif
/***********************************************************************/
{
   if ( found_ppc == NULL )
   {
      if ( curr_ppc->ppc_line_number >= first_in_range
      &&   curr_ppc->ppc_line_number <= last_in_range )
         found_ppc = curr_ppc;
   }
   else
   {
      if ( curr_ppc->ppc_line_number < found_ppc->ppc_line_number
      &&   curr_ppc->ppc_line_number >= first_in_range
      &&   curr_ppc->ppc_line_number <= last_in_range )
         found_ppc = curr_ppc;
   }
   return found_ppc;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static void set_key_values(int key, bool mouse_key)
#else
static void set_key_values(key, mouse_key)
int key;
bool mouse_key;
#endif
/***********************************************************************/
{
 CHARTYPE *keyname=NULL;
 int shift=0;
 if (mouse_key)
 {
    item_values[1].value = mouse_key_number_to_name(key,rsrvd);
    item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
 }
 else
 {
    keyname = get_key_name(key,&shift);
    if (keyname == NULL)
    {
       item_values[1].value = (CHARTYPE *)"";
       item_values[1].len = 0;
    }
    else
    {
       item_values[1].value = keyname;
       item_values[1].len = strlen((DEFCHAR *)keyname);
    }
 }
 if (key < 256
 &&  key >= 0)
 {
    sprintf((DEFCHAR *)num1,"%d",key);
    item_values[2].value = num1;
    item_values[2].len = strlen((DEFCHAR *)num1);
 }
 else
 {
    item_values[2].value = (CHARTYPE *)"";
    item_values[2].len = 0;
 }
 sprintf((DEFCHAR *)num2,"%d",key);
 item_values[3].value = num2;
 item_values[3].len = strlen((DEFCHAR *)num2);
 memset((DEFCHAR *)num3,'0',8);
 num3[8] = '\0';
 if (key != -1)
 {
    if (INSERTMODEx)
       num3[0] = '1';
    if (shift & SHIFT_ALT)
       num3[4] = '1';
    if (shift & SHIFT_CTRL)
       num3[5] = '1';
    if (shift & SHIFT_SHIFT)
       num3[6] = num3[7] = '1';
 }
 item_values[4].value = num3;
 item_values[4].len = 8;
 return;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short set_boolean_value(bool flag, short num)
#else
static short set_boolean_value(flag, num)
bool flag;
short num;
#endif
/***********************************************************************/
{
 if (flag)
 {
    item_values[num].value = (CHARTYPE *)"1";
    item_values[num].len = 1;
 }
 else
 {
    item_values[num].value = (CHARTYPE *)"0";
    item_values[num].len = 1;
 }
 return 1;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short set_on_off_value(bool flag, short num)
#else
static short set_on_off_value(flag, num)
bool flag;
short num;
#endif
/***********************************************************************/
{
 if (flag)
 {
    item_values[num].value = (CHARTYPE *)"ON";
    item_values[num].len = 2;
 }
 else
 {
    item_values[num].value = (CHARTYPE *)"OFF";
    item_values[num].len = 3;
 }
 return 1;
}

/***********************************************************************/
#ifdef HAVE_PROTO
short find_query_item(CHARTYPE *item_name,int len,CHARTYPE *query_type)
#else
short find_query_item(item_name,len,query_type)
CHARTYPE *item_name;
int len;
CHARTYPE *query_type;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#if 0
 register short i=0;
 short itemno = (-1);
#else
 int rc=0;
#endif
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("query.c   :find_item");
/*---------------------------------------------------------------------*/
/* Using the supplied abbreviation for an item, find the entry in the  */
/* query_item[] array. It should return with the index pointer (for    */
/* QUERY_EXTRACT) or an index to the first item in the function_item[] */
/* rexx.c for other query types.                                       */
/*---------------------------------------------------------------------*/
#if 0
 for (i=0; i<NUMBER_QUERY_ITEM; i++)
    {
     if (equal(query_item[i].name,item_name,query_item[i].min_len)
     && (query_item[i].query & query_type))
       {
        if (query_type == QUERY_EXTRACT)
           itemno = i;
        else
           itemno = query_item[i].item_number;
        break;
       }
    }
#else
 set_compare_exact( FALSE );
 rc = search_query_item_array(query_item,
             number_query_item(),
             sizeof( QUERY_ITEM ),
             (DEFCHAR *)item_name,
             len);
 if (rc != (-1))
    *query_type = query_item[rc].query;
#endif
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short show_status(void)
#else
short show_status()
#endif
/***********************************************************************/
{
#define STATUS_COLS 6
   register short i=0,j=0,k=0;
   short lineno=0,colno=0;
   short number_variables=0;
   short item_width=0,column=0,column_width=0,col[STATUS_COLS];

   TRACE_FUNCTION("query.c   :show_status");
   /*
    * For each item that is displayable, display it...
    */
   column = 0;
   column_width = COLS / STATUS_COLS;
   col[0] = 0;
   for (i=1;i<STATUS_COLS;i++)
      col[i] = col[i-1]+column_width+1;
   wclear(stdscr);
   for (i=0; i<NUMBER_QUERY_ITEM; i++)
   {
      /*
       * Get only those settings that are queryable...
       */
      if (query_item[i].query & QUERY_STATUS)
      {
         number_variables = get_item_values(1,query_item[i].item_number,(CHARTYPE *)"",QUERY_STATUS,0L,NULL,0L);
         item_width = 0;
         /*
          * Obtain the total length of the setting values...
          */
         for (j=0;j<number_variables+1;j++)
            item_width += item_values[j].len+1;
         item_width--;     /* reduce by 1 for last value's blank at end */
         /*
          * If the length of the variables is > the screen width, go to next
          * line.
          */
         if (item_width+col[column] > COLS)
         {
            column = colno = 0;
            lineno++;
         }
         else
            colno = col[column];
         /*
          * Display the variables. For the first value, display in BOLD.
          */
         for (j=0;j<number_variables+1;j++)
         {
            if (j == 0)
               attrset(A_BOLD);
            mvaddstr(lineno,colno,(DEFCHAR *)item_values[j].value);
            attrset(A_NORMAL);
            colno += item_values[j].len+1;
         }
         colno--;
         /*
          * Now have to determine where to display the next set of variables...
          * If the just-displayed values go past the last column, or we are
          * already in the last column, go to the next line...
          */
         if (colno >= col[STATUS_COLS-1] || column == STATUS_COLS-1)
         {
            lineno++;
            column = 0;
         }
         else
         {
            /*
             * ...else if the the just-displayed values go past the 2nd column...
             * If the just-displayed values go past the last column, go on to the
             * next line...
             */
            for (k=0;k<STATUS_COLS;k++)
            {
               if (colno < col[k])
               {
                  column = k;
                  break;
               }
            }
         }
      }
   }
   mvaddstr(terminal_lines-2,0,HIT_ANY_KEY);
   refresh();

   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short save_status(CHARTYPE *filename)
#else
short save_status(filename)
CHARTYPE *filename;
#endif
/***********************************************************************/
{
   register short i=0,j=0;
   short number_variables=0,rc=RC_OK;
   FILE *fp=NULL;

   TRACE_FUNCTION("query.c   :save_status");
   /*
    * Get the fully qualified filename from the supplied filename.
    */
   strrmdup(strtrans(filename,OSLASH,ISLASH),ISLASH,TRUE);
   if ((rc = splitpath(filename)) != RC_OK)
   {
      display_error(10,filename,FALSE);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * splitpath() has set up sp_path  and sp_fname. Append the filename.
    */
   strcat((DEFCHAR *)sp_path,(DEFCHAR *)sp_fname);
   /*
    * If the file already exists, display an error.
    */
   if (file_exists(sp_path))
   {
      display_error(8,filename,FALSE);
      TRACE_RETURN();
      return(rc);
   }
   fp = fopen((DEFCHAR *)sp_path,"w");
   /*
    * For each item that is modifiable, get its value...
    */
   fprintf(fp,"/* This file generated by STATUS command of THE %s %s*/\n",the_version,the_release);
   for (i=0; i<NUMBER_QUERY_ITEM ;i++)
   {
      /*
       * Get only those settings that are modifiable...
       */
      if (query_item[i].query & QUERY_MODIFY)
      {
         number_variables = get_item_values(1,query_item[i].item_number,(CHARTYPE *)"",QUERY_MODIFY,0L,NULL,0L);
         fputs("'set",fp);
         /*
          * Write the variables to the file...
          */
         for (j=0;j<number_variables+1;j++)
         {
            fputc(' ',fp);
            fputs((DEFCHAR *)item_values[j].value,fp);
         }
         fprintf(fp,"'\n");
      }
   }
   fflush(fp);
   fclose(fp);
   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short set_extract_variables(short itemno)
#else
short set_extract_variables(itemno)
short itemno;
#endif
/***********************************************************************/
{
   register short i=0;
   short rc=RC_OK,number_values=0;

   TRACE_FUNCTION("query.c:   set_extract_variables");
   number_values = atoi((DEFCHAR *)item_values[0].value);
   for (i=0;i<number_values+1;i++)
   {
      if ((rc = set_rexx_variable(query_item[itemno].name,item_values[i].value,item_values[i].len,i)) != RC_OK)
         break;
   }
   return(rc);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short get_number_dynamic_items( int qitem )
#else
short get_number_dynamic_items( qitem )
int qitem;
#endif
/***********************************************************************/
{
   int number_variables=0;
   PARSER_MAPPING *mapping=NULL;
   PARSER_DETAILS *details=NULL;
   DEFINE *synonym=NULL;

   switch(qitem)
   {
      case ITEM_RING:
         number_variables = number_of_files;
         break;
      case ITEM_PARSER:
         for(details=first_parser;details!=NULL;details=details->next,number_variables++);
         break;
      case ITEM_AUTOCOLOR:
      case ITEM_AUTOCOLOUR:
         for(mapping=first_parser_mapping;mapping!=NULL;mapping=mapping->next,number_variables++);
         break;
      case ITEM_SYNONYM:
         for (synonym=first_synonym;synonym!=NULL;synonym=synonym->next,number_variables++);
      default:
         break;
   }
   return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
short get_item_values(int qitem,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
short get_item_values(qitem,itemno,itemargs,query_type,argc,arg,arglen)
int qitem;
short itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   short number_variables = 1;
   static CHARTYPE num0[3]; /* DO NOT USE THIS FOR DATA !! */
   CHARTYPE *itemname=NULL;

   TRACE_FUNCTION("query.c:   get_item_values");
   /*
    * Only use itemno to reference query_item[] array if it is NOT a
    * boolean function...
    */
   if (qitem)
   {
      itemname = query_item[itemno].name;
      number_variables = (query_item[itemno].ext_func)(
          query_item[itemno].number_values,
          itemno,
          itemargs,
          query_type,
          argc,
          arg,
          arglen);
   }
   else
   {
      itemname = function_item[itemno].name;
      number_variables = (function_item[itemno].ext_func)(
          1,
          itemno,
          itemargs,
          query_type,
          argc,
          arg,
          arglen);
   }
   /*
    * If an "error" condition exists, do not set any values.
    */
   if (number_variables >= 0)
   {
      if (number_variables == 0)
      {
         /*
          * The number of variables needs to be set dynamically for the
          * following items. Any item that has a value of 0 for item_values
          * column of query_item, needs to be catered for here.
          */
         number_variables = get_number_dynamic_items( query_item[itemno].item_number );
      }
      switch(query_type)
      {
         case QUERY_EXTRACT:
         case QUERY_FUNCTION:
         case QUERY_READV:
            sprintf((DEFCHAR *)num0,"%d",number_variables);
            item_values[0].value = num0;
            item_values[0].len = strlen((DEFCHAR *)num0);
            break;
         case QUERY_STATUS:
         case QUERY_QUERY:
         case QUERY_MODIFY:
            item_values[0].value = query_item[itemno].name;
            item_values[0].len = strlen((DEFCHAR *)query_item[itemno].name);
            break;
         default:
            break;
      }
   }
   TRACE_RETURN();
   return(number_variables);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_point_settings(short itemno,CHARTYPE *params)
#else
static short extract_point_settings(itemno,params)
short itemno;
CHARTYPE *params;
#endif
/***********************************************************************/
{
   register short i=0;
   short number_variables = query_item[itemno].number_values;
   static CHARTYPE num4[15];
   _LINE *curr=NULL;
   LINETYPE true_line=0L;
   short rc=RC_OK;

   if (strcmp((DEFCHAR *)params,"") == 0) /* get name for focus line only */
   {
      true_line = (compatible_feel==COMPAT_XEDIT)?CURRENT_VIEW->current_line:get_true_line(TRUE);
      curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
      if (curr->name == NULL)  /* line not named */
         number_variables = 0;
      else
      {
         sprintf((DEFCHAR *)num4,"%ld %s",true_line,curr->name);
         item_values[1].value = num4;
         item_values[1].len = strlen((DEFCHAR *)num4);
         number_variables = 1;
      }
   }
   else
   {
      if (strcmp((DEFCHAR *)params,"*") != 0)     /* argument must be "*" */
      {
         display_error(1,(CHARTYPE *)params,FALSE);
         number_variables = EXTRACT_ARG_ERROR;
      }
      else
      {
         curr = CURRENT_FILE->first_line;
         for(true_line=0,i=0;curr != NULL;true_line++)
         {
            if (curr->name != NULL)  /* line is named */
            {
               sprintf((DEFCHAR *)num4,"%ld %s",true_line,curr->name);
               rc = set_rexx_variable(query_item[itemno].name,num4,strlen((DEFCHAR *)num4),++i);
               if (rc == RC_SYSTEM_ERROR)
               {
                  display_error(54,(CHARTYPE *)"",FALSE);
                  return(EXTRACT_ARG_ERROR);
               }
            }
            curr = curr->next;
         }
         sprintf((DEFCHAR *)num4,"%d",i);
         rc = set_rexx_variable(query_item[itemno].name,num4,strlen((DEFCHAR *)num4),0);
         if (rc == RC_SYSTEM_ERROR)
         {
            display_error(54,(CHARTYPE *)"",FALSE);
            number_variables = EXTRACT_ARG_ERROR;
         }
         else
            number_variables = EXTRACT_VARIABLES_SET;
      }
   }
   return(number_variables);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_prefix_settings(short itemno,CHARTYPE *params)
#else
static short extract_prefix_settings(itemno,params)
short itemno;
CHARTYPE *params;
#endif
/***********************************************************************/
{
   register short i=0;
#define PRE_PARAMS  3
   CHARTYPE *word[PRE_PARAMS+1];
   CHARTYPE strip[PRE_PARAMS];
   unsigned short num_params=0;
   short number_variables = query_item[itemno].number_values;
   static CHARTYPE num4[15];
   _LINE *curr=NULL;
   short rc=RC_OK;
   CHARTYPE *tmpbuf=NULL;

   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   num_params = param_split(params,word,PRE_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   switch(num_params)
   {
      /*
       * None or 1 parameters, error.
       */
      case 0:
      case 1:
         display_error(3,(CHARTYPE *)"",FALSE);
         number_variables = EXTRACT_ARG_ERROR;
         break;
      /*
       * 2  parameters, Synonym and name.
       */
      case 2:
         if (!equal((CHARTYPE *)"synonym",word[0],1))
         {
            display_error(13,(CHARTYPE *)"",FALSE);
            number_variables = EXTRACT_ARG_ERROR;
            break;
         }
         break;
    default:
         /*
          * Too many parameters, error.
          */
         display_error(2,(CHARTYPE *)"",FALSE);
         number_variables = EXTRACT_ARG_ERROR;
         break;
   }
   /*
    * If the validation of parameters is successful...
    */
   if (number_variables >= 0)
   {
      if (strcmp((DEFCHAR *)word[1],"*") == 0)
      {
         /*
          * Get oldname for all synonyms...
          */
         curr = first_prefix_synonym;
         i = 0;
         while(curr != NULL)
         {
            tmpbuf = (CHARTYPE *)(*the_malloc)(sizeof(CHARTYPE)*(strlen((DEFCHAR *)curr->name)+strlen((DEFCHAR *)curr->line)+1));
            if (tmpbuf == (CHARTYPE *)NULL)
            {
               display_error(30,(CHARTYPE *)"",FALSE);
               return(EXTRACT_ARG_ERROR);
            }
            strcpy((DEFCHAR *)tmpbuf,(DEFCHAR *)curr->name);
            strcat((DEFCHAR *)tmpbuf," ");
            strcat((DEFCHAR *)tmpbuf,(DEFCHAR *)curr->line);
            rc = set_rexx_variable(query_item[itemno].name,tmpbuf,strlen((DEFCHAR *)tmpbuf),++i);
            (*the_free)(tmpbuf);
            if (rc == RC_SYSTEM_ERROR)
            {
               display_error(54,(CHARTYPE *)"",FALSE);
               return(EXTRACT_ARG_ERROR);
            }
            curr = curr->next;
         }
         sprintf((DEFCHAR *)num4,"%d",i);
         rc = set_rexx_variable(query_item[itemno].name,num4,strlen((DEFCHAR *)num4),0);
         if (rc == RC_SYSTEM_ERROR)
         {
            display_error(54,(CHARTYPE *)"",FALSE);
            number_variables = EXTRACT_ARG_ERROR;
         }
         else
            number_variables = EXTRACT_VARIABLES_SET;
      }
      else
      {
         /*
          * Get oldname for named synonym...
          */
         item_values[1].value = find_prefix_synonym(word[1]);
         item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
         number_variables = 1;
      }
   }
   return(number_variables);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static void get_etmode(CHARTYPE *onoff,CHARTYPE *list)
#else
static void get_etmode(onoff,list)
CHARTYPE *onoff,*list;
#endif
/***********************************************************************/
{
   bool on_flag=FALSE;
   bool off_flag=FALSE;
   bool last_state=FALSE;
   char tmp[15];
   register int i=0;

   TRACE_FUNCTION("query.c:   get_etmode");
   strcpy((DEFCHAR *)list,"");
   last_state = TRUE;
   for (i=0;i<256;i++)
   {
      if (etmode_flag[i] == TRUE)
         off_flag = TRUE;
      else
         on_flag = TRUE;
      if (last_state != etmode_flag[i])
      {
         if (last_state == FALSE)
         {
            sprintf(tmp,"%d",i-1);
            strcat((DEFCHAR *)list,tmp);
         }
         else
         {
            sprintf(tmp," %d-",i);
            strcat((DEFCHAR *)list,tmp);
         }
         last_state = etmode_flag[i];
      }
   }
   if (*(list+strlen((DEFCHAR *)list)-1) == '-')
   {
      sprintf(tmp,"%d",255);
      strcat((DEFCHAR *)list,tmp);
   }
   if (on_flag)
   {
      strcpy((DEFCHAR *)onoff,"ON");
      if (!off_flag)
         strcpy((DEFCHAR *)list,"");
   }
   else
   {
      strcpy((DEFCHAR *)onoff,"OFF");
      strcpy((DEFCHAR *)list,"");
   }
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_colour_settings(short itemno,CHARTYPE *buffer,CHARTYPE query_type,CHARTYPE *params,bool us,bool isecolour)
#else
static short extract_colour_settings(itemno,buffer,query_type,params,us,isecolour)
short itemno;
CHARTYPE *buffer;
CHARTYPE query_type;
CHARTYPE *params;
bool us,isecolour;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   register int i=0,maxnum;
   int start=0,end=0,number_variables=0,off=0;
   bool found=FALSE;
   CHARTYPE *attr_string=NULL,tmparea[2];
   CHARTYPE save_msgline_base = CURRENT_VIEW->msgline_base;
   short save_msgline_off = CURRENT_VIEW->msgline_off;
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
   CHARTYPE *ptr,*area,*colour=(CHARTYPE*)"colour ",*color=(CHARTYPE*)"color ",*ecolour=(CHARTYPE*)"ecolour ",*ecolor=(CHARTYPE*)"ecolor ";
   COLOUR_ATTR *attr;

   TRACE_FUNCTION("query.c:   extract_colour_settings");
   tmparea[1] = '\0';
   maxnum = (isecolour) ? ECOLOUR_MAX : ATTR_MAX;
   attr = (isecolour) ? CURRENT_FILE->ecolour : CURRENT_FILE->attr;
   if (blank_field(params)
   ||  strcmp((DEFCHAR*)params,"*") == 0)
   {
     start = 0;
     end = maxnum;
   }
   else
   {
      if (isecolour)
      {
         if (strlen((DEFCHAR *)params) != 1)
         {
            TRACE_RETURN();
            return(EXTRACT_ARG_ERROR);
         }
         if (*params >= 'A' && *params <= 'Z')
            i = *params - 'A';
         else if (*params >= 'a' && *params <= 'z')
            i = *params - 'a';
         else if (*params >= '1' && *params <= '9')
            i = *params - '1' + 26;
         else
         {
            TRACE_RETURN();
            return(EXTRACT_ARG_ERROR);
         }
         found = TRUE;
      }
      else
      {
         for (i=0;i<maxnum;i++)
         {
            if (equal(valid_areas[i].area,params,valid_areas[i].area_min_len))
            {
               found = TRUE;
               break;
            }
         }
      }
      if (!found)
      {
         display_error(1,params,FALSE);
         TRACE_RETURN();
         return(EXTRACT_ARG_ERROR);
      }
      start = i;
      end = i+1;
   }
   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = POSITION_TOP;
      CURRENT_VIEW->msgline_off    = 1;
      CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,end-start);
      CURRENT_VIEW->msgmode_status = TRUE;
   }

   for (i=start;i<end;i++)
   {
      attr_string = get_colour_strings(attr+i);
      if (attr_string == (CHARTYPE *)NULL)
      {
         TRACE_RETURN();
         return(EXTRACT_ARG_ERROR);
      }
      if (us)
      {
         if (isecolour)
            ptr = ecolor;
         else
            ptr = color;
      }
      else
      {
         if (isecolour)
            ptr = ecolour;
         else
            ptr = colour;
      }
      if (isecolour)
      {
        if ( i > 25 )
           tmparea[0] = (CHARTYPE)(i-25)+'0';
        else
           tmparea[0] = (CHARTYPE)(i+'A');
         area = tmparea;
      }
      else
         area = valid_areas[i].area;
      sprintf((DEFCHAR *)rsrvd,"%s%s %s",
            (query_type == QUERY_QUERY) ? (DEFCHAR *)ptr : "",
            area,
            attr_string);
      (*the_free)(attr_string);

      if (query_type == QUERY_QUERY)
         display_error(0,rsrvd,TRUE);
      else
      {
         number_variables++;
         item_values[number_variables].len = strlen((DEFCHAR *)rsrvd);
         memcpy((DEFCHAR*)trec+off,(DEFCHAR*)rsrvd,(item_values[number_variables].len)+1);
         item_values[number_variables].value = trec+off;
         off += (item_values[number_variables].len)+1;
      }
   }
   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = save_msgline_base;
      CURRENT_VIEW->msgline_off    = save_msgline_off;
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      rc = EXTRACT_VARIABLES_SET;
   }
   else
      rc = number_variables;
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_autocolour_settings(short itemno,CHARTYPE *buffer,CHARTYPE query_type,CHARTYPE *params,bool us)
#else
static short extract_autocolour_settings(itemno,buffer,query_type,params,us)
short itemno;
CHARTYPE *buffer;
CHARTYPE query_type;
CHARTYPE *params;
bool us;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   register int i=0;
   int number_variables=0,off=0;
   bool found=FALSE;
   CHARTYPE save_msgline_base = CURRENT_VIEW->msgline_base;
   short save_msgline_off = CURRENT_VIEW->msgline_off;
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
   CHARTYPE *ptr,*ptr_mask=NULL,*ptr_magic=NULL,*ptr_parser=NULL;
   PARSER_MAPPING *curr;

   TRACE_FUNCTION("query.c:   extract_autocolour_settings");
   ptr = (us) ? (CHARTYPE *)"autocolor " : (CHARTYPE *)"autocolour ";
   if (blank_field(params)
   ||  strcmp((DEFCHAR*)params,"*") == 0)
   {
      if (query_type == QUERY_QUERY)
      {
         for (i=0,curr=first_parser_mapping;curr!=NULL;curr=curr->next,i++);
         CURRENT_VIEW->msgline_base   = POSITION_TOP;
         CURRENT_VIEW->msgline_off    = 1;
         CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,i);
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      for (curr=first_parser_mapping;curr!=NULL;curr=curr->next)
      {
         sprintf((DEFCHAR *)rsrvd,"%s%s%s %s%s",
           (query_type == QUERY_QUERY) ? (DEFCHAR *)ptr : "",
           (curr->filemask) ? (DEFCHAR *)curr->filemask : "",
           (curr->magic_number) ? (DEFCHAR *)curr->magic_number : "",
            (DEFCHAR *)curr->parser->parser_name,
           (curr->magic_number) ? " MAGIC" : "");

         if (query_type == QUERY_QUERY)
            display_error(0,rsrvd,TRUE);
         else
         {
            number_variables++;
            item_values[number_variables].len = strlen((DEFCHAR *)rsrvd);
            memcpy((DEFCHAR*)trec+off,(DEFCHAR*)rsrvd,(item_values[number_variables].len)+1);
            item_values[number_variables].value = trec+off;
            off += (item_values[number_variables].len)+1;
         }
      }
   }
   else
   {
      if (query_type == QUERY_QUERY)
      {
         CURRENT_VIEW->msgline_base   = POSITION_TOP;
         CURRENT_VIEW->msgline_off    = 1;
         CURRENT_VIEW->msgline_rows   = 1;
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      /*
       * Find a match for the supplied mask or magic number
       */
      for (curr=first_parser_mapping;curr!=NULL;curr=curr->next)
      {
#ifdef UNIX
         if (curr->filemask
         &&  strcmp((DEFCHAR *)params,(DEFCHAR *)curr->filemask) == 0)
         {
            ptr_mask = curr->filemask;
            ptr_magic = (CHARTYPE *)"";
            found = TRUE;
            break;
         }
         if (curr->magic_number
         &&  strcmp((DEFCHAR *)params,(DEFCHAR *)curr->magic_number) == 0)
         {
            ptr_mask = curr->magic_number;
            ptr_magic = (CHARTYPE *)"MAGIC";
            found = TRUE;
            break;
         }
#else
         if (curr->filemask
         &&  my_stricmp(params,curr->filemask) == 0)
         {
            ptr_mask = curr->filemask;
            ptr_magic = (CHARTYPE *)"";
            found = TRUE;
            break;
         }
         if (curr->magic_number
         &&  my_stricmp(params,curr->magic_number) == 0)
         {
            ptr_mask = curr->magic_number;
            ptr_magic = (CHARTYPE *)"MAGIC";
            found = TRUE;
            break;
         }
#endif
      }
      if (found)
         ptr_parser = (CHARTYPE *)curr->parser->parser_name;
      else
      {
         ptr_mask = params;
         ptr_magic = (CHARTYPE *)"";
         ptr_parser = (CHARTYPE *)"NULL";
      }
      if (query_type == QUERY_QUERY)
      {
         sprintf((DEFCHAR *)rsrvd,"%s%s %s%s",
            (query_type == QUERY_QUERY) ? (DEFCHAR *)ptr : "",
            ptr_mask,
            ptr_parser,
            ptr_magic);
         display_error(0,rsrvd,TRUE);
      }
      else
      {
         item_values[1].value = ptr_mask;
         item_values[1].len = strlen((DEFCHAR *)ptr_mask);
         item_values[2].value = ptr_parser;
         item_values[2].len = strlen((DEFCHAR *)ptr_parser);
         item_values[3].value = ptr_magic;
         item_values[3].len = strlen((DEFCHAR *)ptr_magic);
         number_variables = 3;
      }
   }

   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = save_msgline_base;
      CURRENT_VIEW->msgline_off    = save_msgline_off;
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      rc = EXTRACT_VARIABLES_SET;
   }
   else
      rc = number_variables;

   TRACE_RETURN();
   return(rc);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_after_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_after_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   short y=0,x=0;
   bool bool_flag=FALSE;

   if (batch_only)
   {
      item_values[1].value = (CHARTYPE *)"0";
      item_values[1].len = 1;
   }
   else
   {
      getyx(CURRENT_WINDOW,y,x);
      bool_flag = FALSE;
      switch(CURRENT_VIEW->current_window)
      {
         case WINDOW_FILEAREA:
            if ((x+CURRENT_VIEW->verify_col-1) >= rec_len)
               bool_flag = TRUE;
            break;
         case WINDOW_COMMAND:
            if (x >= cmd_rec_len)
               bool_flag = TRUE;
            break;
         case WINDOW_PREFIX:
            if (x >= pre_rec_len)
               bool_flag = TRUE;
            break;
      }
      set_boolean_value(bool_flag,1);
   }
   return 1; /* number of values set */
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_alt(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_alt(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   sprintf((DEFCHAR *)num1,"%d",CURRENT_FILE->autosave_alt);
   sprintf((DEFCHAR *)num2,"%d",CURRENT_FILE->save_alt);
   item_values[1].value = num1;
   item_values[2].value = num2;
   item_values[1].len = strlen((DEFCHAR *)num1);
   item_values[2].len = strlen((DEFCHAR *)num2);
   return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_alt_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_alt_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   return set_boolean_value((bool)(CURRENT_FILE->save_alt != 0),(short)1);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_altkey_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_altkey_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   int shift=0;

   get_key_name(lastkeys[current_key],&shift);
   return set_boolean_value((bool)(shift & SHIFT_ALT),(short)1);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_batch_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_batch_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   return set_boolean_value((bool)batch_only,(short)1);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_arbchar(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_arbchar(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   set_on_off_value(CURRENT_VIEW->arbchar_status,1);
   num1[0] = CURRENT_VIEW->arbchar_multiple;
   num1[1] ='\0';
   item_values[2].value = num1;
   item_values[2].len = 1;
   num2[0] = CURRENT_VIEW->arbchar_single;
   num2[1] ='\0';
   item_values[3].value = num2;
   item_values[3].len = 1;
   return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_autosave(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_autosave(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   if (CURRENT_FILE->autosave == 0)
   {
      item_values[1].value = (CHARTYPE *)"OFF";
      item_values[1].len = 3;
   }
   else
   {
      sprintf((DEFCHAR *)num1,"%d",CURRENT_FILE->autosave);
      item_values[1].value = num1;
      item_values[1].len = strlen((DEFCHAR *)num1);
   }
   return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_autocolor(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_autocolor(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   return extract_autocolour_settings(itemno,rsrvd,query_type,itemargs,TRUE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_autocolour(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_autocolour(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   return extract_autocolour_settings(itemno,rsrvd,query_type,itemargs,FALSE);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_autoscroll(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_autoscroll(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   if (CURRENT_VIEW->autoscroll == 0)
   {
      item_values[1].value = (CHARTYPE *)"OFF";
      item_values[1].len = 3;
   }
   else if (CURRENT_VIEW->autoscroll == (-1))
   {
      item_values[1].value = (CHARTYPE *)"HALF";
      item_values[1].len = 4;
   }
   else
   {
      sprintf((DEFCHAR *)num1,"%ld",CURRENT_VIEW->autoscroll);
      item_values[1].value = num1;
      item_values[1].len = strlen((DEFCHAR *)num1);
   }
   return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_backup(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_backup(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   switch(CURRENT_FILE->backup)
   {
      case BACKUP_OFF:
         item_values[1].value = (CHARTYPE *)"OFF";
         break;
      case BACKUP_TEMP:
         item_values[1].value = (CHARTYPE *)"TEMP";
         break;
      case BACKUP_KEEP:
         item_values[1].value = (CHARTYPE *)"KEEP";
         break;
      case BACKUP_INPLACE:
         item_values[1].value = (CHARTYPE *)"INPLACE";
         break;
   }
   item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
   return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_beep(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_beep(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   return set_on_off_value(BEEPx,1);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_before_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_before_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;
 bool bool_flag=FALSE;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
 }
 else
 {
    getyx(CURRENT_WINDOW,y,x);
    bool_flag = FALSE;
    switch(CURRENT_VIEW->current_window)
    {
       case WINDOW_FILEAREA:
          if ((x+CURRENT_VIEW->verify_col-1) < memne(rec,' ',rec_len))
             bool_flag = TRUE;
          break;
       case WINDOW_COMMAND:
          if (x < memne(cmd_rec,' ',cmd_rec_len))
             bool_flag = TRUE;
          break;
       case WINDOW_PREFIX:/* cursor can't go before 1st non-blank */
          break;
    }
    set_boolean_value((bool)bool_flag,(short)1);
 }
 return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_blank_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_blank_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = (CHARTYPE *)"0"; /* FALSE by default */
 switch(CURRENT_VIEW->current_window)
 {
    case WINDOW_FILEAREA:
       if (rec_len == 0)
          item_values[1].value = (CHARTYPE *)"1";
       break;
    case WINDOW_PREFIX:
       if (pre_rec_len == 0)
          item_values[1].value = (CHARTYPE *)"1";
       break;
    case WINDOW_COMMAND:
       if (cmd_rec_len == 0)
          item_values[1].value = (CHARTYPE *)"1";
       break;
 }
 item_values[1].len = 1;
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_block(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_block(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (MARK_VIEW == NULL)
 {
    item_values[1].value = (CHARTYPE *)"NONE";
    item_values[1].len = 4;
    number_variables = 1;
 }
 else
 {
    number_variables = 6;
    item_values[1].value = block_name[MARK_VIEW->mark_type];
    item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
    sprintf((DEFCHAR *)num1,"%ld",MARK_VIEW->mark_start_line);
    item_values[2].value = num1;
    item_values[2].len = strlen((DEFCHAR *)num1);
    sprintf((DEFCHAR *)num2,"%d",MARK_VIEW->mark_start_col);
    item_values[3].value = num2;
    item_values[3].len = strlen((DEFCHAR *)num2);
    sprintf((DEFCHAR *)num3,"%ld",MARK_VIEW->mark_end_line);
    item_values[4].value = num3;
    item_values[4].len = strlen((DEFCHAR *)num3);
    sprintf((DEFCHAR *)num4,"%d",MARK_VIEW->mark_end_col);
    item_values[5].value = num4;
    item_values[5].len = strlen((DEFCHAR *)num4);
    strcpy((DEFCHAR *)trec,(DEFCHAR *)MARK_FILE->fpath);
    strcat((DEFCHAR *)trec,(DEFCHAR *)MARK_FILE->fname);
    item_values[6].value = (CHARTYPE *)trec;
    item_values[6].len = strlen((DEFCHAR *)trec);
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_block_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_block_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(CURRENT_VIEW == MARK_VIEW),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_bottomedge_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_bottomedge_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return 1;
 }
 getyx(CURRENT_WINDOW,y,x);
 return set_boolean_value((bool)(CURRENT_VIEW->current_window == WINDOW_FILEAREA && y == CURRENT_SCREEN.rows[WINDOW_FILEAREA]-1),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_case(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_case(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 switch(CURRENT_VIEW->case_enter)
 {
    case CASE_MIXED:
       item_values[1].value = (CHARTYPE *)"MIXED";
       break;
    case CASE_UPPER:
       item_values[1].value = (CHARTYPE *)"UPPER";
       break;
    case CASE_LOWER:
       item_values[1].value = (CHARTYPE *)"LOWER";
       break;
    default:
       break;
 }
 item_values[1].len = 5;
 switch(CURRENT_VIEW->case_locate)
 {
    case CASE_IGNORE:
       item_values[2].value = (CHARTYPE *)"IGNORE";
       item_values[2].len = 6;
       break;
    case CASE_RESPECT:
       item_values[2].value = (CHARTYPE *)"RESPECT";
       item_values[2].len = 7;
       break;
    default:
       break;
 }
 switch(CURRENT_VIEW->case_change)
 {
    case CASE_IGNORE:
       item_values[3].value = (CHARTYPE *)"IGNORE";
       item_values[3].len = 6;
       break;
    case CASE_RESPECT:
       item_values[3].value = (CHARTYPE *)"RESPECT";
       item_values[3].len = 7;
       break;
    default:
       break;
 }
 switch(CURRENT_VIEW->case_sort)
 {
    case CASE_IGNORE:
       item_values[4].value = (CHARTYPE *)"IGNORE";
       item_values[4].len = 6;
       break;
    case CASE_RESPECT:
       item_values[4].value = (CHARTYPE *)"RESPECT";
       item_values[4].len = 7;
       break;
    default:
       break;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_clearerrorkey(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_clearerrorkey(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 int dummy=0;

 if (CLEARERRORKEYx == -1)
 {
    item_values[1].value = (CHARTYPE *)"*";
    item_values[1].len = 1;
 }
 else
 {
    item_values[1].value = get_key_name(CLEARERRORKEYx,&dummy);
    item_values[1].len = strlen((DEFCHAR*)item_values[1].value);
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_clearscreen(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_clearscreen(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CLEARSCREENx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_clock(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_clock(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CLOCKx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_command_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_command_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(CURRENT_WINDOW_COMMAND != NULL),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_cmdarrows(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_cmdarrows(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (CMDARROWSTABCMDx)
 {
    item_values[1].value = (CHARTYPE *)"TAB";
    item_values[1].len = 3;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)"RETRIEVE";
    item_values[1].len = 8;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_cmdline(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_cmdline(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 switch(CURRENT_VIEW->cmd_line)
 {
    case 'B':
       item_values[1].value = (CHARTYPE *)"BOTTOM";
       item_values[1].len = 6;
       break;
    case 'T':
       item_values[1].value = (CHARTYPE *)"TOP";
       item_values[1].len = 3;
       break;
    case 'O':
       item_values[1].value = (CHARTYPE *)"OFF";
       item_values[1].len = 3;
       number_variables = 1;
       break;
 }
 if (CURRENT_VIEW->cmd_line == 'O')
    return number_variables;

 if (query_type == QUERY_EXTRACT
 ||  query_type == QUERY_FUNCTION)
 {
    sprintf((DEFCHAR *)num1,"%d",CURRENT_SCREEN.start_row[WINDOW_COMMAND]+1);
    item_values[2].value = num1;
    item_values[2].len = strlen((DEFCHAR *)num1);
    item_values[3].value = cmd_rec;
    item_values[3].len = cmd_rec_len;
    number_variables = 3;
 }
 else
    number_variables = 1;
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_color(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_color(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return extract_colour_settings(itemno,rsrvd,query_type,itemargs,TRUE,FALSE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_colour(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_colour(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return extract_colour_settings(itemno,rsrvd,query_type,itemargs,FALSE,FALSE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_coloring(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_coloring(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   if (CURRENT_FILE->colouring)
   {
      item_values[1].value = (CHARTYPE *)"ON";
      item_values[1].len = 2;
      if (CURRENT_FILE->parser)
      {
         item_values[3].value = CURRENT_FILE->parser->parser_name;
         item_values[3].len = strlen((DEFCHAR *)item_values[3].value);
      }
      else
      {
         item_values[3].value = (CHARTYPE *)"NULL";
         item_values[3].len = 4;
      }
      if (CURRENT_FILE->autocolour)
      {
         item_values[2].value = (CHARTYPE *)"AUTO";
         item_values[2].len = 4;
      }
      else
      {
         item_values[2].value = CURRENT_FILE->parser->parser_name;
         item_values[2].len = strlen((DEFCHAR *)item_values[2].value);
      }
   }
   else
   {
      item_values[1].value = (CHARTYPE *)"OFF";
      item_values[1].len = 3;
      number_variables = 1;
   }
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_colouring(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_colouring(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   if (CURRENT_FILE->colouring)
   {
      item_values[1].value = (CHARTYPE *)"ON";
      item_values[1].len = 2;
      if (CURRENT_FILE->parser)
      {
         item_values[3].value = CURRENT_FILE->parser->parser_name;
         item_values[3].len = strlen((DEFCHAR *)item_values[3].value);
      }
      else
      {
         item_values[3].value = (CHARTYPE *)"NULL";
         item_values[3].len = 4;
      }
      if (CURRENT_FILE->autocolour)
      {
         item_values[2].value = (CHARTYPE *)"AUTO";
         item_values[2].len = 4;
      }
      else
      {
         item_values[2].value = CURRENT_FILE->parser->parser_name;
         item_values[2].len = strlen((DEFCHAR *)item_values[2].value);
      }
   }
   else
   {
      item_values[1].value = (CHARTYPE *)"OFF";
      item_values[1].len = 3;
      number_variables = 1;
   }
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_column(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_column(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;

 if (batch_only
 ||  CURRENT_VIEW->current_window != WINDOW_FILEAREA)
    sprintf((DEFCHAR *)num1,"%d",CURRENT_VIEW->current_column);
 else
 {
    getyx(CURRENT_WINDOW,y,x);
    sprintf((DEFCHAR *)num1,"%d",x+CURRENT_VIEW->verify_col);
 }
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_compat(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_compat(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 switch(compatible_look)
 {
    case COMPAT_THE:
       item_values[1].value = (CHARTYPE *)"THE";
       item_values[1].len = 3;
       break;
    case COMPAT_XEDIT:
       item_values[1].value = (CHARTYPE *)"XEDIT";
       item_values[1].len = 5;
       break;
    case COMPAT_ISPF:
       item_values[1].value = (CHARTYPE *)"ISPF";
       item_values[1].len = 4;
       break;
    case COMPAT_KEDIT:
       item_values[1].value = (CHARTYPE *)"KEDIT";
       item_values[1].len = 5;
       break;
    case COMPAT_KEDITW:
       item_values[1].value = (CHARTYPE *)"KEDITW";
       item_values[1].len = 6;
       break;
 }
 switch(compatible_feel)
 {
    case COMPAT_THE:
       item_values[2].value = (CHARTYPE *)"THE";
       item_values[2].len = 3;
       break;
    case COMPAT_XEDIT:
       item_values[2].value = (CHARTYPE *)"XEDIT";
       item_values[2].len = 5;
       break;
    case COMPAT_ISPF:
       item_values[2].value = (CHARTYPE *)"ISPF";
       item_values[2].len = 4;
       break;
    case COMPAT_KEDIT:
       item_values[2].value = (CHARTYPE *)"KEDIT";
       item_values[2].len = 5;
       break;
    case COMPAT_KEDITW:
       item_values[2].value = (CHARTYPE *)"KEDITW";
       item_values[2].len = 6;
       break;
 }
 switch(compatible_keys)
 {
    case COMPAT_THE:
       item_values[3].value = (CHARTYPE *)"THE";
       item_values[3].len = 3;
       break;
    case COMPAT_XEDIT:
       item_values[3].value = (CHARTYPE *)"XEDIT";
       item_values[3].len = 5;
       break;
    case COMPAT_ISPF:
       item_values[3].value = (CHARTYPE *)"ISPF";
       item_values[3].len = 4;
       break;
    case COMPAT_KEDIT:
       item_values[3].value = (CHARTYPE *)"KEDIT";
       item_values[3].len = 5;
       break;
    case COMPAT_KEDITW:
       item_values[3].value = (CHARTYPE *)"KEDITW";
       item_values[3].len = 6;
       break;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_ctlchar(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_ctlchar(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   int i,j;

   if (arg == NULL
   ||  blank_field(arg)
   ||  strcmp((DEFCHAR*)arg,"*") == 0)
   {
      if (CTLCHARx)
      {
         item_values[1].value = (CHARTYPE *)"ON";
         item_values[1].len = 2;
         num2[0] = ctlchar_escape;
         num2[1] = '\0';
         item_values[2].value = (CHARTYPE *)num2;
         item_values[2].len = 1;
         memset(rsrvd,' ',MAX_CTLCHARS*2);
         for (i=0,j=0;i<MAX_CTLCHARS;i++)
         {
            if (ctlchar_char[i] != 0)
            {
               rsrvd[j*2] = ctlchar_char[i];
               j++;
            }
         }
         rsrvd[(j*2)-1] = '\0';
         item_values[3].value = rsrvd;
         item_values[3].len = strlen((DEFCHAR *)rsrvd);
         number_variables = 3;
      }
      else
      {
         item_values[1].value = (CHARTYPE *)"OFF";
         item_values[1].len = 3;
         number_variables = 1;
      }
   }
   else
   {
   }
   return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_ctrl_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_ctrl_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 int shift=0;

 get_key_name(lastkeys[current_key],&shift);
 return set_boolean_value((bool)(shift & SHIFT_CTRL),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_curline(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_curline(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (CURRENT_VIEW->current_base == POSITION_MIDDLE)
 {
    if (CURRENT_VIEW->current_off == 0)
       strcpy((DEFCHAR *)rsrvd,"M");
    else
       sprintf((DEFCHAR *)rsrvd,"M%+d",CURRENT_VIEW->current_off);
 }
 else
    sprintf((DEFCHAR *)rsrvd,"%d",CURRENT_VIEW->current_off);
 item_values[1].value = rsrvd;
 item_values[1].len = strlen((DEFCHAR *)rsrvd);
 if (query_type == QUERY_EXTRACT
 ||  query_type == QUERY_FUNCTION)
 {
    sprintf((DEFCHAR *)num1,"%d",CURRENT_VIEW->current_row+1);
    item_values[2].value = num1;
    item_values[2].len = strlen((DEFCHAR *)num1);
    curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,
                   (compatible_feel==COMPAT_XEDIT)?CURRENT_VIEW->current_line:get_true_line(TRUE),
                   CURRENT_FILE->number_lines);
    item_values[3].value = (CHARTYPE *)curr->line;
    item_values[3].len = curr->length;
    item_values[4].value = (curr->flags.new_flag||curr->flags.changed_flag)?(CHARTYPE *)"ON":(CHARTYPE *)"OFF";
    item_values[4].len = strlen((DEFCHAR *)item_values[4].value);
    if (curr->flags.new_flag)
       item_values[5].value = (CHARTYPE *)"NEW CHANGED";
    else
       if (curr->flags.changed_flag)
          item_values[5].value = (CHARTYPE *)"OLD CHANGED";
       else
          item_values[5].value = (CHARTYPE *)"OLD";
    item_values[5].len = strlen((DEFCHAR *)item_values[5].value);
    sprintf((DEFCHAR *)num2,"%d",curr->select);
    item_values[6].value = num2;
    item_values[6].len = strlen((DEFCHAR *)num2);
 }
 else
    number_variables = 1;
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_cursor(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_cursor(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 LINETYPE current_screen_line=(-1L);
 LINETYPE current_screen_column=(-1L);
 LINETYPE current_file_line=(-1L);
 LINETYPE current_file_column=(-1L);

 get_cursor_position(&current_screen_line,&current_screen_column,
                     &current_file_line,&current_file_column);
 sprintf((DEFCHAR *)num1,"%ld",current_screen_line);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 sprintf((DEFCHAR *)num2,"%ld",current_screen_column);
 item_values[2].value = num2;
 item_values[2].len = strlen((DEFCHAR *)num2);
 sprintf((DEFCHAR *)num3,"%ld",current_file_line);
 item_values[3].value = num3;
 item_values[3].len = strlen((DEFCHAR *)num3);
 sprintf((DEFCHAR *)num4,"%ld",current_file_column);
 item_values[4].value = num4;
 item_values[4].len = strlen((DEFCHAR *)num4);
 sprintf((DEFCHAR *)num5,"%ld",original_screen_line);
 item_values[5].value = num5;
 item_values[5].len = strlen((DEFCHAR *)num5);
 sprintf((DEFCHAR *)num6,"%ld",original_screen_column);
 item_values[6].value = num6;
 item_values[6].len = strlen((DEFCHAR *)num6);
 sprintf((DEFCHAR *)num7,"%ld",original_file_line);
 item_values[7].value = num7;
 item_values[7].len = strlen((DEFCHAR *)num7);
 sprintf((DEFCHAR *)num8,"%ld",original_file_column);
 item_values[8].value = num8;
 item_values[8].len = strlen((DEFCHAR *)num8);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_cursorstay(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_cursorstay(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(scroll_cursor_stay,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_current_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_current_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(CURRENT_VIEW->current_window != WINDOW_COMMAND && CURRENT_VIEW->focus_line == CURRENT_VIEW->current_line),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_define(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_define(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = (CHARTYPE *)"JUNK";
 item_values[1].len = strlen((DEFCHAR*)item_values[1].value);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_defsort(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_defsort(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 switch(DEFSORTx)
 {
    case DIRSORT_DIR:
       item_values[1].value = (CHARTYPE *)"DIRECTORY";
       break;
    case DIRSORT_NAME:
       item_values[1].value = (CHARTYPE *)"NAME";
       break;
    case DIRSORT_SIZE:
       item_values[1].value = (CHARTYPE *)"SIZE";
       break;
    case DIRSORT_TIME:
       item_values[1].value = (CHARTYPE *)"TIME";
       break;
    case DIRSORT_DATE:
       item_values[1].value = (CHARTYPE *)"DATE";
       break;
    case DIRSORT_NONE:
       item_values[1].value = (CHARTYPE *)"OFF";
       break;
 }
 item_values[1].len = strlen((DEFCHAR*)item_values[1].value);
 if (DIRORDERx == DIRSORT_ASC)
 {
    item_values[2].value = (CHARTYPE *)"ASCENDING";
    item_values[2].len = 9;
 }
 else
 {
    item_values[2].value = (CHARTYPE *)"DESCENDING";
    item_values[2].len = 10;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_dir_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_dir_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(CURRENT_FILE->pseudo_file == PSEUDO_DIR),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_dirfileid(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_dirfileid(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 LINETYPE true_line = (-1L);

 if (CURRENT_FILE->pseudo_file == PSEUDO_DIR)
 {
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
    {
       if (CURRENT_TOF || CURRENT_BOF)
          true_line = (-1L);
       else
          true_line = CURRENT_VIEW->current_line;
    }
    else
    {
       if (FOCUS_TOF || FOCUS_BOF)
          true_line = (-1L);
       else
          true_line = CURRENT_VIEW->focus_line;
    }
 }
 if (true_line == (-1L))
 {
    item_values[1].value = (CHARTYPE *)"";
    item_values[1].len = 0;
    item_values[2].value = (CHARTYPE *)"";
    item_values[2].len = 0;
 }
 else
 {
    curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
    item_values[1].value = (CHARTYPE *)dir_path;
    item_values[1].len = strlen((DEFCHAR *)dir_path);
    if (curr->length < file_start)
    {
       item_values[2].value = (CHARTYPE *)"";
       item_values[2].len = 0;
    }
    else
    {
       item_values[2].value = (CHARTYPE *)curr->line+file_start;
       item_values[2].len = strlen((DEFCHAR *)curr->line+file_start);
    }
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_dirinclude(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_dirinclude(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = get_dirtype(rsrvd);
 item_values[1].len = strlen((DEFCHAR*)item_values[1].value);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_ecolor(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_ecolor(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return extract_colour_settings(itemno,rsrvd,query_type,itemargs,TRUE,TRUE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_ecolour(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_ecolour(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return extract_colour_settings(itemno,rsrvd,query_type,itemargs,FALSE,TRUE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_end_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_end_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return 1;
 }
 item_values[1].value = (CHARTYPE *)"0"; /* set FALSE by default */
 getyx(CURRENT_WINDOW,y,x);
 switch(CURRENT_VIEW->current_window)
 {
    case WINDOW_FILEAREA:
       if (x+CURRENT_VIEW->verify_col == rec_len)
          item_values[1].value = (CHARTYPE *)"1";
       break;
    case WINDOW_PREFIX:
       if (pre_rec_len > 0
       &&  pre_rec_len-1 == x)
          item_values[1].value = (CHARTYPE *)"1";
       break;
    case WINDOW_COMMAND:
       if (cmd_rec_len > 0
       &&  cmd_rec_len-1 == x)
          item_values[1].value = (CHARTYPE *)"1";
       break;
 }
 item_values[1].len = 1;
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_display(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_display(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%d",CURRENT_VIEW->display_low);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 sprintf((DEFCHAR *)num2,"%d",CURRENT_VIEW->display_high);
 item_values[2].value = num2;
 item_values[2].len = strlen((DEFCHAR *)num2);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_eof(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_eof(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_BOF,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_eof_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_eof_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(FOCUS_BOF && CURRENT_VIEW->current_window != WINDOW_COMMAND),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_eolout(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_eolout(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 switch(CURRENT_FILE->eolout)
 {
    case EOLOUT_LF:
       item_values[1].value = (CHARTYPE *)"LF";
       item_values[1].len = 2;
       break;
    case EOLOUT_CR:
       item_values[1].value = (CHARTYPE *)"CR";
       item_values[1].len = 2;
       break;
    case EOLOUT_CRLF:
       item_values[1].value = (CHARTYPE *)"CRLF";
       item_values[1].len = 4;
       break;
    case EOLOUT_NONE:
       item_values[1].value = (CHARTYPE *)"NONE";
       item_values[1].len = 4;
       break;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_equivchar(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_equivchar(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = EQUIVCHARstr;
 item_values[1].len = strlen((DEFCHAR *)EQUIVCHARstr);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_etmode(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_etmode(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 get_etmode(num1,rsrvd);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 item_values[2].value = rsrvd;
 item_values[2].len = strlen((DEFCHAR *)rsrvd);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_field(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_field(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 int i=0;
 short y=0,x=0;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return 1;
 }
 getyx(CURRENT_WINDOW,y,x);
 switch(CURRENT_VIEW->current_window)
 {
    case WINDOW_FILEAREA:
       item_values[1].value = rec;
       item_values[1].len = rec_len;
       i = x+CURRENT_VIEW->verify_col;
       num1[0] = rec[i-1];
       item_values[4].value = (CHARTYPE*)"TEXT";
       item_values[4].len = 4;
       break;
    case WINDOW_PREFIX:
       item_values[1].value = pre_rec;
       item_values[1].len = pre_rec_len;
       i = x+1;
       num1[0] = pre_rec[i-1];
       item_values[4].value = (CHARTYPE*)"PREFIX";
       item_values[4].len = 6;
       break;
    case WINDOW_COMMAND:
       item_values[1].value = cmd_rec;
       item_values[1].len = cmd_rec_len;
       i = x+1;
       num1[0] = cmd_rec[i-1];
       item_values[4].value = (CHARTYPE*)"COMMAND";
       item_values[4].len = 7;
       break;
 }
 num1[1] = '\0';
 item_values[2].value = num1;
 item_values[2].len = 1;
 sprintf((DEFCHAR*)num2,"%d",i);
 item_values[3].value = num2;
 item_values[3].len = strlen((DEFCHAR*)num2);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_fieldword(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_fieldword(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   short y=0,x=0,rc;
   unsigned short real_col=0;
   LENGTHTYPE first_col,last_col,len=0;
   CHARTYPE *ptr=NULL,save_word;
   int word_len;
   CHARTYPE *tmpbuf;

   getyx(CURRENT_WINDOW,y,x);
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
       ptr = rec;
       len = rec_len;
       real_col = x+CURRENT_VIEW->verify_col;
       break;
      case WINDOW_PREFIX:
       ptr = pre_rec;
       len = pre_rec_len;
       real_col = x+1;
       break;
      case WINDOW_COMMAND:
       ptr = cmd_rec;
       len = cmd_rec_len;
       real_col = x+1;
       break;
   }
   save_word = CURRENT_VIEW->word;
   /*
    * Get the word based on an alphanumeric word setting...
    */
   CURRENT_VIEW->word = 'A';
   if ( len == 0
   ||   get_word( ptr, len, real_col-1,&first_col,&last_col) == 0)
   {
      rc = set_rexx_variable( query_item[itemno].name, (CHARTYPE *)"", 0, 1 );
   }
   else
   {
      word_len = (last_col - first_col) + 1;
      tmpbuf = (CHARTYPE *)(*the_malloc)( sizeof(CHARTYPE)*(word_len+1) );
      if ( tmpbuf == (CHARTYPE *)NULL )
      {
         display_error( 30, (CHARTYPE *)"", FALSE );
         CURRENT_VIEW->word = save_word;
         return( EXTRACT_ARG_ERROR );
      }
      memcpy( (DEFCHAR *)tmpbuf, (DEFCHAR *)ptr+first_col, word_len );
      tmpbuf[word_len] = '\0';
      rc = set_rexx_variable( query_item[itemno].name, tmpbuf, word_len, 1 );
      (*the_free)(tmpbuf);
   }
   if ( rc == RC_SYSTEM_ERROR )
   {
      display_error( 54, (CHARTYPE *)"", FALSE );
      CURRENT_VIEW->word = save_word;
      return( EXTRACT_ARG_ERROR );
   }
   /*
    * Get the word based on a non-blank word setting...
    */
   CURRENT_VIEW->word = 'N';
   if ( len == 0
   ||   get_word( ptr, len, real_col-1,&first_col,&last_col) == 0)
   {
      rc = set_rexx_variable( query_item[itemno].name, (CHARTYPE *)"", 0, 2 );
   }
   else
   {
      word_len = (last_col - first_col) + 1;
      tmpbuf = (CHARTYPE *)(*the_malloc)( sizeof(CHARTYPE)*(word_len+1) );
      if ( tmpbuf == (CHARTYPE *)NULL )
      {
         display_error( 30, (CHARTYPE *)"", FALSE );
         CURRENT_VIEW->word = save_word;
         return( EXTRACT_ARG_ERROR );
      }
      memcpy( (DEFCHAR *)tmpbuf, (DEFCHAR *)ptr+first_col, word_len );
      tmpbuf[word_len] = '\0';
      rc = set_rexx_variable( query_item[itemno].name, tmpbuf, word_len, 2 );
      (*the_free)(tmpbuf);
   }
   if ( rc == RC_SYSTEM_ERROR )
   {
      display_error( 54, (CHARTYPE *)"", FALSE );
      CURRENT_VIEW->word = save_word;
      return( EXTRACT_ARG_ERROR );
   }
   rc = set_rexx_variable( query_item[itemno].name, (CHARTYPE *)"2", 1, 0 );
   if ( rc == RC_SYSTEM_ERROR )
   {
      display_error( 54, (CHARTYPE *)"", FALSE );
      CURRENT_VIEW->word = save_word;
      return( EXTRACT_ARG_ERROR );
   }
   CURRENT_VIEW->word = save_word;

 return EXTRACT_VARIABLES_SET;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_first_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_first_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return 1;
 }
 getyx(CURRENT_WINDOW,y,x);
 return set_boolean_value((bool)(x == 0 && CURRENT_VIEW->verify_col == 1),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_focuseof_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_focuseof_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   bool bool_flag;
   if ( CURRENT_VIEW->current_window == WINDOW_COMMAND )
      bool_flag = CURRENT_BOF;
   else
      bool_flag = FOCUS_BOF;
   return set_boolean_value( (bool)(bool_flag), (short)1 );
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_focustof_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_focustof_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   bool bool_flag;
   if ( CURRENT_VIEW->current_window == WINDOW_COMMAND )
      bool_flag = CURRENT_TOF;
   else
      bool_flag = FOCUS_TOF;
   return set_boolean_value( (bool)(bool_flag), (short)1 );
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_filename(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_filename(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = (CHARTYPE *)CURRENT_FILE->fname;
 item_values[1].len = strlen((DEFCHAR *)CURRENT_FILE->fname);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_filestatus(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_filestatus(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = (CHARTYPE *)"NONE";
 item_values[1].len = 4;
 if (CURRENT_FILE->disposition == FILE_READONLY)
 {
    item_values[2].value = (CHARTYPE *)"READONLY";
    item_values[2].len = 8;
 }
 else
 {
    item_values[2].value = (CHARTYPE *)"READWRITE";
    item_values[2].len = 9;
 }
 switch(CURRENT_FILE->eolfirst)
 {
    case EOLOUT_LF:
       item_values[3].value = (CHARTYPE *)"LF";
       item_values[3].len = 2;
       break;
    case EOLOUT_CR:
       item_values[3].value = (CHARTYPE *)"CR";
       item_values[3].len = 2;
       break;
    case EOLOUT_CRLF:
       item_values[3].value = (CHARTYPE *)"CRLF";
       item_values[3].len = 4;
       break;
    case EOLOUT_NONE:
       item_values[3].value = (CHARTYPE *)"NONE";
       item_values[3].len = 4;
       break;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_fmode(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_fmode(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
#if defined(UNIX)
 item_values[1].value = (CHARTYPE *)"";
 item_values[1].len = 0;
#else
 num1[0] = (CHARTYPE)*(CURRENT_FILE->fpath);
 num1[1] = '\0';
 item_values[1].value = num1;
 item_values[1].len = 1;
#endif
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_fname(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_fname(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short x=0;

 strcpy((DEFCHAR*)rsrvd,(DEFCHAR*)CURRENT_FILE->fname);
 x = strzreveq(rsrvd,'.');
 if (x != (-1))
    rsrvd[x] = '\0';
 item_values[1].value = rsrvd;
 item_values[1].len = strlen((DEFCHAR *)rsrvd);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_efileid(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_efileid(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   item_values[1].value = (CHARTYPE *)CURRENT_FILE->efileid;
   item_values[1].len = strlen( (DEFCHAR *)CURRENT_FILE->efileid );
   item_values[2].value = (CHARTYPE *)CURRENT_FILE->actualfname;
   item_values[2].len = strlen( (DEFCHAR *)CURRENT_FILE->actualfname );
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_fpath(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_fpath(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = (CHARTYPE *)CURRENT_FILE->fpath;
 item_values[1].len = strlen((DEFCHAR *)CURRENT_FILE->fpath);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_ftype(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_ftype(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short x=0;

 x = strzreveq(CURRENT_FILE->fname,'.');
 if (x == (-1))
 {
    item_values[1].value = (CHARTYPE *)"";
    item_values[1].len = 0;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)CURRENT_FILE->fname+x+1;
    item_values[1].len = strlen((DEFCHAR *)CURRENT_FILE->fname+x+1);
 }
 return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_fullfname(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_fullfname(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_FILE->display_actual_filename,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_getenv(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_getenv(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 CHARTYPE *tmpbuf=NULL;

 if (itemargs == NULL
 ||  strlen((DEFCHAR *)itemargs) == 0)
 {
    item_values[1].value = (CHARTYPE *)"***invalid***";
    item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
    return number_variables;
 }
#if defined(UNIX)
 if (query_type == QUERY_FUNCTION)
    tmpbuf = (CHARTYPE *)getenv((DEFCHAR *)arg);
 else
    tmpbuf = (CHARTYPE *)getenv((DEFCHAR *)itemargs);
#else
 if (query_type == QUERY_FUNCTION)
    tmpbuf = (CHARTYPE *)getenv((DEFCHAR *)make_upper(arg));
 else
    tmpbuf = (CHARTYPE *)getenv((DEFCHAR *)make_upper(itemargs));
#endif
 if (tmpbuf == NULL)
    item_values[1].value = (CHARTYPE *)"***invalid***";
 else
    item_values[1].value = tmpbuf;
 item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_header(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_header(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   register int i=0,off=0;
   CHARTYPE save_msgline_base = CURRENT_VIEW->msgline_base;
   short save_msgline_off = CURRENT_VIEW->msgline_off;
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("query.c:   extract_header");
   if (query_type == QUERY_QUERY)
   {
      for ( i = 0; thm[i].the_header != HEADER_ALL; i++ )
      {
         ;
      }
      CURRENT_VIEW->msgline_base   = POSITION_TOP;
      CURRENT_VIEW->msgline_off    = 1;
      CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,i);
      CURRENT_VIEW->msgmode_status = TRUE;
   }

   for ( i = 0; thm[i].the_header != HEADER_ALL; i++ )
   {
      sprintf((DEFCHAR *)rsrvd,"%s %s",
        thm[i].the_header_name,
        ( CURRENT_VIEW->syntax_headers & thm[i].the_header ) ? "ON" : "OFF" );

      if (query_type == QUERY_QUERY)
         display_error(0,rsrvd,TRUE);
      else
      {
         number_variables++;
         item_values[number_variables].len = strlen((DEFCHAR *)rsrvd);
         memcpy((DEFCHAR*)trec+off,(DEFCHAR*)rsrvd,(item_values[number_variables].len)+1);
         item_values[number_variables].value = trec+off;
         off += (item_values[number_variables].len)+1;
      }
   }

   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = save_msgline_base;
      CURRENT_VIEW->msgline_off    = save_msgline_off;
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      rc = EXTRACT_VARIABLES_SET;
   }
   else
      rc = number_variables;

   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_hex(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_hex(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->hex,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_hexdisplay(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_hexdisplay(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(HEXDISPLAYx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_hexshow(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_hexshow(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 set_on_off_value(CURRENT_VIEW->hexshow_on,1);
 if (CURRENT_VIEW->hexshow_base == POSITION_MIDDLE)
    sprintf((DEFCHAR *)rsrvd,"M%+d",CURRENT_VIEW->hexshow_off);
 else
    sprintf((DEFCHAR *)rsrvd,"%d",CURRENT_VIEW->hexshow_off);
 item_values[2].value = rsrvd;
 item_values[2].len = strlen((DEFCHAR *)rsrvd);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_highlight(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_highlight(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 switch (CURRENT_VIEW->highlight)
 {
    case HIGHLIGHT_NONE:
       item_values[1].value = (CHARTYPE *)"OFF";
       break;
    case HIGHLIGHT_ALT:
       item_values[1].value = (CHARTYPE *)"ALTERED";
       break;
    case HIGHLIGHT_TAG:
       item_values[1].value = (CHARTYPE *)"TAGGED";
       break;
    case HIGHLIGHT_SELECT:
       item_values[1].value = (CHARTYPE *)"SELECT";
       sprintf((DEFCHAR *)num1,"%d",CURRENT_VIEW->highlight_low);
       item_values[2].value = num1;
       item_values[2].len = strlen((DEFCHAR *)num1);
       sprintf((DEFCHAR *)num2,"%d",CURRENT_VIEW->highlight_high);
       item_values[3].value = num2;
       item_values[3].len = strlen((DEFCHAR *)num2);
       number_variables = 3;
       break;
    default:
       break;
 }
 item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_idline(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_idline(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->id_line,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_impmacro(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_impmacro(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->imp_macro,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_impos(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_impos(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->imp_os,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_inblock_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_inblock_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;
 bool bool_flag=FALSE;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return 1;
 }
 bool_flag = FALSE;
 if (CURRENT_VIEW == MARK_VIEW
 &&  CURRENT_VIEW->current_window == WINDOW_FILEAREA)
 {
    getyx(CURRENT_WINDOW_FILEAREA,y,x);
    switch(MARK_VIEW->mark_type)
    {
       case M_LINE:
          if ((CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line)
          &&  (CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line))
             bool_flag = TRUE;
          break;
       case M_BOX:
       case M_WORD:
       case M_COLUMN:
          if ((CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line)
          &&  (CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line)
          &&  (x + CURRENT_VIEW->verify_start >= MARK_VIEW->mark_start_col)
          &&  (x + CURRENT_VIEW->verify_start <= MARK_VIEW->mark_end_col))
             bool_flag = TRUE;
          break;
       case M_STREAM:
          if ((CURRENT_VIEW->focus_line < MARK_VIEW->mark_start_line)
          ||  (CURRENT_VIEW->focus_line > MARK_VIEW->mark_end_line))
          {
             bool_flag = FALSE;
             break;
          }
          if (MARK_VIEW->mark_start_line == MARK_VIEW->mark_end_line)
          {
             /*
              * Single-line block
              */
             if (x + CURRENT_VIEW->verify_start >= MARK_VIEW->mark_start_col
             &&  x + CURRENT_VIEW->verify_start <= MARK_VIEW->mark_end_col)
             {
                bool_flag = TRUE;
                break;
             }
             else
             {
                bool_flag = FALSE;
                break;
             }
          }
          /*
           * Multi-line stream block
           */
          if (CURRENT_VIEW->focus_line == MARK_VIEW->mark_start_line
          &&  x + CURRENT_VIEW->verify_start >= MARK_VIEW->mark_start_col)
          {
             bool_flag = TRUE;
             break;
          }
          if (CURRENT_VIEW->focus_line == MARK_VIEW->mark_end_line
          &&  x + CURRENT_VIEW->verify_start <= MARK_VIEW->mark_end_col)
          {
             bool_flag = TRUE;
             break;
          }
          if (CURRENT_VIEW->focus_line != MARK_VIEW->mark_start_line
          &&  CURRENT_VIEW->focus_line != MARK_VIEW->mark_end_line)
          {
             bool_flag = TRUE;
             break;
          }
          bool_flag = FALSE;
          break;
       default:
          break;
    }
 }
 return set_boolean_value((bool)bool_flag,(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_incommand_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_incommand_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(CURRENT_VIEW->current_window == WINDOW_COMMAND),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_initial_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_initial_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)in_profile,(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_inprefix_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_inprefix_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(CURRENT_VIEW->current_window == WINDOW_PREFIX),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_inputmode(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_inputmode(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (CURRENT_VIEW->inputmode == INPUTMODE_OFF)
 {
    item_values[1].value = (CHARTYPE *)"OFF";
    item_values[1].len = 3;
 }
 if (CURRENT_VIEW->inputmode == INPUTMODE_FULL)
 {
    item_values[1].value = (CHARTYPE *)"FULL";
    item_values[1].len = 4;
 }
 if (CURRENT_VIEW->inputmode == INPUTMODE_LINE)
 {
    item_values[1].value = (CHARTYPE *)"LINE";
    item_values[1].len = 4;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_insertmode(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_insertmode(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(INSERTMODEx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_lastkey(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_lastkey(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 int keynum=0;

 if (argc == 0)
    set_key_values((current_key==-1)?-1:lastkeys[current_key],FALSE);
 else
 {
    if (valid_positive_integer(itemargs))
    {
       keynum = atoi((DEFCHAR *)itemargs);
       if (keynum > 8)
          return(EXTRACT_ARG_ERROR);
       if (current_key == -1)
          set_key_values(-1,FALSE);
       else
       {
          keynum--;  /* 0 base the number */
          if (keynum <= current_key)
             keynum = current_key - keynum;
          else
             keynum = 8 - (keynum - current_key);
          set_key_values(keynum,FALSE);
       }
    }
    else
       return(EXTRACT_ARG_ERROR);
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_lastmsg(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_lastmsg(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (last_message == NULL)
 {
    item_values[1].value = (CHARTYPE *)"";
    item_values[1].len = 0;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)last_message;
    item_values[1].len = strlen((DEFCHAR *)last_message);
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_lastop(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_lastop(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   register int i=0;
   bool found=FALSE;
   CHARTYPE save_msgline_base = CURRENT_VIEW->msgline_base;
   short save_msgline_off = CURRENT_VIEW->msgline_off;
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
   CHARTYPE *ptr_lastop=NULL;
   int off=0;

   if (itemargs == NULL
   ||  blank_field(itemargs)
   ||  strcmp((DEFCHAR*)itemargs,"*") == 0)
   {
      if (query_type == QUERY_QUERY)
      {
         CURRENT_VIEW->msgline_base   = POSITION_TOP;
         CURRENT_VIEW->msgline_off    = 1;
         CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,LASTOP_MAX);
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      else
         number_variables = 0;
      for ( i = 0; i < LASTOP_MAX; i++ )
      {
         sprintf((DEFCHAR *)rsrvd,"%s%s %s",
           (query_type == QUERY_QUERY) ? (DEFCHAR *)"lastop " : "",
           lastop[i].command,
           lastop[i].value );

         if ( query_type == QUERY_QUERY )
            display_error( 0, rsrvd, TRUE);
         else
         {
            number_variables++;
            item_values[number_variables].len = strlen( (DEFCHAR *)rsrvd );
            memcpy( (DEFCHAR*)trec+off, (DEFCHAR*)rsrvd, (item_values[number_variables].len)+1 );
            item_values[number_variables].value = trec+off;
            off += (item_values[number_variables].len)+1;
         }
      }
   }
   else
   {
      if (query_type == QUERY_QUERY)
      {
         CURRENT_VIEW->msgline_base   = POSITION_TOP;
         CURRENT_VIEW->msgline_off    = 1;
         CURRENT_VIEW->msgline_rows   = 1;
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      /*
       * Find a match for the supplied mask or magic number
       */
      for ( i = 0; i < LASTOP_MAX; i++ )
      {
         if ( equal( lastop[i].command, (CHARTYPE *)itemargs, lastop[i].min_len ) )
         {
            ptr_lastop = lastop[i].value;
            found = TRUE;
            break;
         }
      }
      if (query_type == QUERY_QUERY)
      {
         if (!found)
         {
            display_error( 1, itemargs, TRUE );
            TRACE_RETURN();
            return EXTRACT_ARG_ERROR;
         }
         sprintf((DEFCHAR *)rsrvd,"%s%s %s",
            (query_type == QUERY_QUERY) ? (DEFCHAR *)"lastop " : "",
            lastop[i].command,
            ptr_lastop);
         display_error( 0, rsrvd, TRUE );
      }
      else
      {
         if (!found)
         {
            return EXTRACT_ARG_ERROR;
            TRACE_RETURN();
         }
         item_values[1].value = lastop[i].command;
         item_values[1].len = strlen((DEFCHAR *)itemargs);
         item_values[2].value = ptr_lastop;
         item_values[2].len = strlen((DEFCHAR *)ptr_lastop);
         number_variables = 2;
      }
   }

   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = save_msgline_base;
      CURRENT_VIEW->msgline_off    = save_msgline_off;
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      rc = EXTRACT_VARIABLES_SET;
   }
   else
      rc = number_variables;

   TRACE_RETURN();
   return rc;

}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_lastrc(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_lastrc(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%d",lastrc);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_leftedge_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_leftedge_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return 1;
 }
 getyx(CURRENT_WINDOW,y,x);
 return set_boolean_value((bool)(CURRENT_VIEW->current_window == WINDOW_FILEAREA && x == 0),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_length(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_length(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,CURRENT_VIEW->current_line,CURRENT_FILE->number_lines);
 sprintf((DEFCHAR *)num1,"%d",curr->length);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_line(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_line(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%ld",
    (compatible_feel==COMPAT_XEDIT)?CURRENT_VIEW->current_line:get_true_line(TRUE));
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_lineflag(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_lineflag(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,
                 (compatible_feel==COMPAT_XEDIT)?CURRENT_VIEW->current_line:get_true_line(TRUE),
                 CURRENT_FILE->number_lines);
 if (curr->flags.new_flag)
    item_values[1].value = (CHARTYPE *)"NEW";
 else
    item_values[1].value = (CHARTYPE *)"NONEW";
 item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
 if (curr->flags.changed_flag)
    item_values[2].value = (CHARTYPE *)"CHANGE";
 else
    item_values[2].value = (CHARTYPE *)"NOCHANGE";
 item_values[2].len = strlen((DEFCHAR *)item_values[2].value);
 if (curr->flags.tag_flag)
    item_values[3].value = (CHARTYPE *)"TAG";
 else
    item_values[3].value = (CHARTYPE *)"NOTAG";
 item_values[3].len = strlen((DEFCHAR *)item_values[3].value);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_linend(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_linend(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 set_on_off_value(CURRENT_VIEW->linend_status,1);
 num1[0] = CURRENT_VIEW->linend_value;
 num1[1] = '\0';
 item_values[2].value = num1;
 item_values[2].len = 1;
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_lscreen(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_lscreen(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%d",CURRENT_SCREEN.screen_rows);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 sprintf((DEFCHAR *)num2,"%d",CURRENT_SCREEN.screen_cols);
 item_values[2].value = num2;
 item_values[2].len = strlen((DEFCHAR *)num2);
 sprintf((DEFCHAR *)num3,"%d",CURRENT_SCREEN.screen_start_row+1);
 item_values[3].value = num3;
 item_values[3].len = strlen((DEFCHAR *)num3);
 sprintf((DEFCHAR *)num4,"%d",CURRENT_SCREEN.screen_start_col+1);
 item_values[4].value = num4;
 item_values[4].len = strlen((DEFCHAR *)num4);
 sprintf((DEFCHAR *)num5,"%d",terminal_lines);
 item_values[5].value = num5;
 item_values[5].len = strlen((DEFCHAR *)num5);
 sprintf((DEFCHAR *)num6,"%d",COLS);
 item_values[6].value = num6;
 item_values[6].len = strlen((DEFCHAR *)num6);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_macro(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_macro(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->macro,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_macroext(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_macroext(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (strlen((DEFCHAR *)macro_suffix) == 0)
    item_values[1].value = (CHARTYPE *)macro_suffix;
 else
    item_values[1].value = (CHARTYPE *)macro_suffix+1;
 item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_macropath(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_macropath(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = (CHARTYPE *)the_macro_path;
 item_values[1].len = strlen((DEFCHAR *)the_macro_path);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_margins(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_margins(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%d",CURRENT_VIEW->margin_left);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 sprintf((DEFCHAR *)num2,"%d",CURRENT_VIEW->margin_right);
 item_values[2].value = num2;
 item_values[2].len = strlen((DEFCHAR *)num2);
 if (CURRENT_VIEW->margin_indent_offset_status)
    sprintf((DEFCHAR *)num3,"%+d",CURRENT_VIEW->margin_indent);
 else
    sprintf((DEFCHAR *)num3,"%d",CURRENT_VIEW->margin_indent);
 item_values[3].value = num3;
 item_values[3].len = strlen((DEFCHAR *)num3);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_modifiable_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_modifiable_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 bool bool_flag=FALSE;
 short y=0,x=0;

 switch(CURRENT_VIEW->current_window)
 {
    case WINDOW_FILEAREA:
       if (batch_only)
       {
           bool_flag = FALSE;
           break;
       }
       getyx(CURRENT_WINDOW,y,x);
       if (FOCUS_TOF
       ||  FOCUS_BOF
       ||  CURRENT_SCREEN.sl[y].line_type == LINE_SHADOW)
           bool_flag = FALSE;
       else
           bool_flag = TRUE;
       break;
    default:
       bool_flag = TRUE;
       break;
 }
 return set_boolean_value((bool)bool_flag,(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_monitor(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_monitor(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
#ifdef A_COLOR
 if (colour_support)
 {
    item_values[1].value = (CHARTYPE *)"COLOR";
    item_values[1].len = 5;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)"MONO";
    item_values[1].len = 4;
 }
 item_values[2].value = (CHARTYPE *)"COLOR";
 item_values[2].len = 5;
#else
 item_values[1].value = (CHARTYPE *)"MONO";
 item_values[1].len = 4;
 item_values[2].value = (CHARTYPE *)"MONO";
 item_values[2].len = 4;
#endif
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_mouse(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_mouse(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(MOUSEx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_msgline(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_msgline(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   item_values[1].value = (CHARTYPE *)"ON";
   item_values[1].len = 2;
   if (CURRENT_VIEW->msgline_base == POSITION_MIDDLE)
      sprintf((DEFCHAR *)rsrvd,"M%+d",CURRENT_VIEW->msgline_off);
   else
      sprintf((DEFCHAR *)rsrvd,"%d",CURRENT_VIEW->msgline_off);
   item_values[2].value = rsrvd;
   item_values[2].len = strlen((DEFCHAR *)rsrvd);
   if ( CURRENT_VIEW->msgline_rows )
   {
      sprintf((DEFCHAR *)num1,"%d",CURRENT_VIEW->msgline_rows);
      item_values[3].value = num1;
   }
   else
      item_values[3].value = (CHARTYPE *)"*";
   item_values[3].len = strlen((DEFCHAR *)num1);
   item_values[4].value = (CHARTYPE *)"OVERLAY";
   item_values[4].len = 7;
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_msgmode(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_msgmode(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->msgmode_status,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_nbfile(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_nbfile(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%ld",number_of_files);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_nbscope(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_nbscope(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   LINETYPE number_lines = 0;

   if ( CURRENT_VIEW->scope_all )
   {
      sprintf((DEFCHAR *)num1,"%ld",CURRENT_FILE->number_lines);
   }
   else
   {
      for( number_lines=0L, curr=CURRENT_FILE->first_line; curr!=NULL; curr=curr->next )
      {
        if ( curr->prev == NULL
        ||   curr->next == NULL )
           continue;
         if ( IN_SCOPE( CURRENT_VIEW, curr) )
            number_lines++;
      }
      sprintf( (DEFCHAR *)num1, "%ld", number_lines );
   }

   sprintf( (DEFCHAR *)num2, "%ld", get_true_line( TRUE ) );
   item_values[1].value = num1;
   item_values[1].len = strlen( (DEFCHAR *)num1 );
   item_values[2].value = num2;
   item_values[2].len = strlen( (DEFCHAR *)num2 );
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_newlines(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_newlines(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (CURRENT_VIEW->newline_aligned)
 {
    item_values[1].value = (CHARTYPE *)"ALIGNED";
    item_values[1].len = 7;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)"LEFT";
    item_values[1].len = 4;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_nondisp(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_nondisp(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 num1[0] = NONDISPx;
 num1[1] ='\0';
 item_values[1].value = num1;
 item_values[1].len = 1;
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_number(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_number(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->number,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_pagewrap(short pagewrap_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_pagewrap(pagewrap_variables,itemno,itemargs,query_type,argc,arg,arglen)
short pagewrap_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(PAGEWRAPx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_parser(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_parser(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   register int i=0;
   int off=0;
   bool found=FALSE;
   CHARTYPE save_msgline_base = CURRENT_VIEW->msgline_base;
   short save_msgline_off = CURRENT_VIEW->msgline_off;
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
   CHARTYPE *ptr_filename=NULL;
   PARSER_DETAILS *curr;

   if (itemargs == NULL
   ||  blank_field(itemargs)
   ||  strcmp((DEFCHAR*)itemargs,"*") == 0)
   {
      if (query_type == QUERY_QUERY)
      {
         for (i=0,curr=first_parser;curr!=NULL;curr=curr->next,i++);
         CURRENT_VIEW->msgline_base   = POSITION_TOP;
         CURRENT_VIEW->msgline_off    = 1;
         CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,i);
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      else
         number_variables = 0;
      for (curr=first_parser;curr!=NULL;curr=curr->next)
      {
         sprintf((DEFCHAR *)rsrvd,"%s%s %s",
           (query_type == QUERY_QUERY) ? (DEFCHAR *)"parser " : "",
           curr->parser_name,
           curr->filename);

         if (query_type == QUERY_QUERY)
            display_error(0,rsrvd,TRUE);
         else
         {
            number_variables++;
            item_values[number_variables].len = strlen((DEFCHAR *)rsrvd);
            memcpy((DEFCHAR*)trec+off,(DEFCHAR*)rsrvd,(item_values[number_variables].len)+1);
            item_values[number_variables].value = trec+off;
            off += (item_values[number_variables].len)+1;
         }
      }
   }
   else
   {
      if (query_type == QUERY_QUERY)
      {
         CURRENT_VIEW->msgline_base   = POSITION_TOP;
         CURRENT_VIEW->msgline_off    = 1;
         CURRENT_VIEW->msgline_rows   = 1;
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      /*
       * Find a match for the supplied mask or magic number
       */
      for (curr=first_parser;curr!=NULL;curr=curr->next)
      {
         if (my_stricmp((DEFCHAR *)itemargs,(DEFCHAR *)curr->parser_name) == 0)
         {
            ptr_filename = curr->filename;
            found = TRUE;
            break;
         }
      }
      if (!found)
      {
         ptr_filename = (CHARTYPE *)"";
      }
      if (query_type == QUERY_QUERY)
      {
         sprintf((DEFCHAR *)rsrvd,"%s%s %s",
            (query_type == QUERY_QUERY) ? (DEFCHAR *)"parser " : "",
            itemargs,
            ptr_filename);
         display_error(0,rsrvd,TRUE);
      }
      else
      {
         item_values[1].value = itemargs;
         item_values[1].len = strlen((DEFCHAR *)itemargs);
         item_values[2].value = ptr_filename;
         item_values[2].len = strlen((DEFCHAR *)ptr_filename);
         number_variables = 2;
      }
   }

   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = save_msgline_base;
      CURRENT_VIEW->msgline_off    = save_msgline_off;
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      rc = EXTRACT_VARIABLES_SET;
   }
   else
      rc = number_variables;

   TRACE_RETURN();
   return rc;
}

/*
                                 +-- * ---+
<---+---------+--+-----------+---+- name -+---+---------------------------------+--->
    +- BLOCK -+  +- OLDNAME -+                |   +--- :1 ---+   +--- * ----+   |
                                              +---+- target -+---+- target -+---+
*/
/***********************************************************************/

#ifdef HAVE_PROTO
static short extract_pending(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_pending(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
#define PEN_PARAMS  6
#define STATE_START    0
#define STATE_OLDNAME  1
#define STATE_NAME     2
#define STATE_TARGET1  3
#define STATE_TARGET2  4
   CHARTYPE *word[PEN_PARAMS+1];
   CHARTYPE strip[PEN_PARAMS];
   unsigned short num_params=0;
   bool find_block=FALSE;
   bool find_oldname=FALSE;
   CHARTYPE *name=NULL;
   static CHARTYPE num1[7];
   THE_PPC *curr_ppc=NULL;
   LINETYPE first_in_range=1L;
   LINETYPE last_in_range=CURRENT_FILE->number_lines;
   THE_PPC *found_ppc=NULL;
   short target_type=TARGET_ABSOLUTE|TARGET_RELATIVE;
   TARGET target;
   short rc;
   short state=STATE_START;
   int i;

   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   strip[4]=STRIP_BOTH;
   strip[5]=STRIP_BOTH;
   num_params = param_split(itemargs,word,PEN_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if ( num_params == 0 )
   {
      /*
       * No parameters, error.
       */
      display_error(3,(CHARTYPE *)"",FALSE);
      return EXTRACT_ARG_ERROR;
   }

   i = 0;
   for ( ; ; )
   {
      if ( i == num_params )
         break;
      switch( state )
      {
         case STATE_START:
            if ( equal( (CHARTYPE *)"block", word[i], 5 ) )
            {
               find_block = TRUE;
               state = STATE_OLDNAME;
               i++;
               break;
            }
            if ( equal( (CHARTYPE *)"oldname", word[i], 7 ) )
            {
               find_oldname = TRUE;
               state = STATE_NAME;
               i++;
               break;
            }
            state = STATE_NAME;
            break;
         case STATE_OLDNAME:
            if ( equal( (CHARTYPE *)"oldname", word[i], 7 ) )
            {
               find_oldname = TRUE;
               state = STATE_NAME;
               i++;
               break;
            }
            state = STATE_NAME;
            break;
         case STATE_NAME:
            name = word[i];
            state = STATE_TARGET1;
            i++;
            break;
         case STATE_TARGET1:
            initialise_target(&target);
            rc = validate_target(word[i],&target,target_type,0L,FALSE,FALSE);
            if (rc == RC_OK)
            {
               first_in_range = target.rt[0].numeric_target;
               i++;
               state = STATE_TARGET2;
            }
            else
            {
               /* error */
               number_variables = EXTRACT_ARG_ERROR;
            }
            free_target(&target);
            break;
         case STATE_TARGET2:
            initialise_target(&target);
            rc = validate_target(word[i],&target,target_type,first_in_range,FALSE,FALSE);
            if (rc == RC_OK)
            {
               last_in_range = target.rt[0].numeric_target;
               i++;
               state = STATE_TARGET2;
            }
            else
            {
               /* error */
               number_variables = EXTRACT_ARG_ERROR;
            }
            free_target(&target);
            break;
      }
      if ( number_variables == EXTRACT_ARG_ERROR )
         break;
   }
   /*
    * If the validation of parameters is successful...
    */
   if (number_variables >= 0)
   {
      /*
       * No pending prefix commands, return 0.
       */
      if (CURRENT_FILE->first_ppc == NULL)
         number_variables = 0;
      else
      {
         /*
          * If we are to look for OLDNAME, find a synonym for it if one exists..
          */
         if (find_oldname)
            name = find_prefix_oldname(name);
         /*
          * For each pending prefix command...
          */
         curr_ppc = CURRENT_FILE->first_ppc;
         for ( ; ; )
         {
            if (curr_ppc == NULL)
               break;
            /*
             * If we want to match on any name...
             */
            if (strcmp((DEFCHAR *)name,"*") == 0)
            {
               /*
                * Are we matching on any BLOCK command...
                */
               if (find_block)
               {
                  if (curr_ppc->ppc_block_command)
                  {
                     /*
                      * We have found the first BLOCK command with any name.
                      */
                     if ( found_ppc == NULL )
                        found_ppc = curr_ppc;
                     else
                     {
                        if ( curr_ppc->ppc_line_number < found_ppc->ppc_line_number )
                           found_ppc = curr_ppc;
                     }
                  }
                  /*
                   * Go back and look for another either because we didn't
                   * find a block command, or because we did, but it may not
                   * be the one with the smallest line number.
                   */
                  curr_ppc = curr_ppc->next;
                  continue;
               }
               else
               {
                  /*
                   * We have found the first command with any name.
                   */
                  found_ppc = in_range( found_ppc, curr_ppc, first_in_range, last_in_range );
                  /*
                   * Go back and look for another because it may not
                   * be the one with the smallest line number.
                   */
                  curr_ppc = curr_ppc->next;
                  continue;
               }
            }
            /*
             * We want to find a specific command...
             */
            if (memcmpi(curr_ppc->ppc_command,name,strlen((DEFCHAR *)name)) == 0)
            {
               /*
                * Are we looking for a specific BLOCK command...
                */
               if (find_block)
               {
                  if (curr_ppc->ppc_block_command)
                  {
                     /*
                      * We have found the first specific BLOCK command.
                      */
                     found_ppc = in_range( found_ppc, curr_ppc, first_in_range, last_in_range );
                  }
               }
               else
               {
                  /*
                   * We have found the first specific command.
                   */
                  found_ppc = in_range( found_ppc, curr_ppc, first_in_range, last_in_range );
               }
               /*
                * Go back and look for another because it may not
                * be the one with the smallest line number.
                */
               curr_ppc = curr_ppc->next;
               continue;
            }
            curr_ppc = curr_ppc->next;
         }
         /*
          * Did we find a matching pending prefix command ?
          */
         if (found_ppc == NULL)
            number_variables = 0;
         else
         {
            /*
             * Yes we did. Set all of the REXX variables to the correct values...
             */
            sprintf((DEFCHAR *)num1,"%ld",found_ppc->ppc_line_number);
            item_values[1].value = num1;
            item_values[1].len = strlen((DEFCHAR *)num1);
            item_values[2].value = found_ppc->ppc_command;
            item_values[2].len = strlen((DEFCHAR *)item_values[2].value);
            item_values[3].value = find_prefix_synonym(found_ppc->ppc_command);
            item_values[3].len = strlen((DEFCHAR *)item_values[2].value);
            if (found_ppc->ppc_block_command)
               item_values[4].value = (CHARTYPE *)"BLOCK";
            else
               item_values[4].value = (CHARTYPE *)"";
            item_values[4].len = strlen((DEFCHAR *)item_values[4].value);
         }
      }
   }
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_point(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_point(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 LINETYPE true_line=0;

 switch(query_type)
 {
    case QUERY_QUERY:
    case QUERY_MODIFY:
       true_line = (compatible_feel==COMPAT_XEDIT)?CURRENT_VIEW->current_line:get_true_line(TRUE);
       curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
       if (curr->name == NULL)  /* line not named */
       {
          item_values[1].value = (CHARTYPE *)"";
          item_values[1].len = 0;
       }
       else
       {
          item_values[1].value = curr->name;
          item_values[1].len = strlen((DEFCHAR *)curr->name);
       }
       break;
    default:
       number_variables = extract_point_settings(itemno,itemargs);
       break;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_position(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_position(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 LINETYPE true_line=0;
 LENGTHTYPE col=0;

 set_on_off_value(CURRENT_VIEW->position_status,1);
 if ((query_type == QUERY_EXTRACT
   ||  query_type == QUERY_FUNCTION)
 &&  !batch_only)
 {
    get_current_position(current_screen,&true_line,&col);
    sprintf((DEFCHAR *)num1,"%ld",true_line);
    item_values[2].value = num1;
    item_values[2].len = strlen((DEFCHAR *)num1);
    sprintf((DEFCHAR *)num2,"%d",col);
    item_values[3].value = num2;
    item_values[3].len = strlen((DEFCHAR *)num2);
 }
 else
    number_variables = 1;

 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_prefix(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_prefix(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (rexx_support)
 {
    if (strcmp((DEFCHAR *)itemargs,"") != 0)
       return extract_prefix_settings(itemno,itemargs);
 }
 else
 {
    if (strcmp((DEFCHAR *)itemargs,"") != 0)
    {
       display_error(2,(CHARTYPE *)itemargs,FALSE);
       return EXTRACT_ARG_ERROR;
    }
 }
 if (CURRENT_VIEW->prefix == PREFIX_OFF)
 {
    item_values[1].value = (CHARTYPE *)"OFF";
    item_values[1].len = 3;
    item_values[2].value = (CHARTYPE *)"";  /* this set to empty deliberately */
    item_values[2].len = 0;
    return 1;
 }
 if ((CURRENT_VIEW->prefix&PREFIX_STATUS_MASK) == PREFIX_ON)
 {
    item_values[1].value = (CHARTYPE *)"ON";
    item_values[1].len = 2;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)"NULLS";
    item_values[1].len = 5;
 }
 if ((CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) == PREFIX_LEFT)
 {
    item_values[2].value = (CHARTYPE *)"LEFT";
    item_values[2].len = 4;
 }
 else
 {
    item_values[2].value = (CHARTYPE *)"RIGHT";
    item_values[2].len = 5;
 }
 sprintf((DEFCHAR *)num1,"%d",CURRENT_VIEW->prefix_width);
 item_values[3].value = num1;
 item_values[3].len = strlen((DEFCHAR *)num1);
 sprintf((DEFCHAR *)num2,"%d",CURRENT_VIEW->prefix_gap);
 item_values[4].value = num2;
 item_values[4].len = strlen((DEFCHAR *)num2);
 return 4;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_printer(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_printer(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = (CHARTYPE *)spooler_name;
 item_values[1].len = strlen((DEFCHAR *)spooler_name);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_reprofile(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_reprofile(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(REPROFILEx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_readonly(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_readonly(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   if ( READONLYx == READONLY_FORCE )
   {
      item_values[1].value = (CHARTYPE *)"FORCE";
      item_values[1].len = 5;
   }
   else if ( ISREADONLY(CURRENT_FILE) )
   {
      item_values[1].value = (CHARTYPE *)"ON";
      item_values[1].len = 2;
   }
   else
   {
      item_values[1].value = (CHARTYPE *)"OFF";
      item_values[1].len = 3;
   }
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_regexp(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_regexp(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   int i;
   item_values[1].value = (CHARTYPE *)"";
   for ( i = 0; regexp_syntaxes[i].name != NULL; i++ )
   {
      if ( regexp_syntaxes[i].value == REGEXPx )
      {
         item_values[1].value = (CHARTYPE *)regexp_syntaxes[i].name;
         break;
      }
   }
   item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_readv(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_readv(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   int key=0;
   bool mouse_key=FALSE;

   if (batch_only)
   {
      item_values[1].value = (CHARTYPE *)"0";
      item_values[1].len = 1;
      return 1;
   }
   while(1)
   {
#ifdef CAN_RESIZE
      if (is_termresized())
      {
         (void)THE_Resize(0,0);
         (void)THERefresh((CHARTYPE *)"");
      }
#endif
      key = my_getch(stdscr);
#ifdef CAN_RESIZE
      if (is_termresized())
         continue;
#endif
#if defined (PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
      if (key == KEY_MOUSE)
      {
         int b,ba,bm,w;
         CHARTYPE scrn;
         if (get_mouse_info(&b,&ba,&bm) != RC_OK)
            continue;
         which_window_is_mouse_in(&scrn,&w);
         mouse_key = TRUE;
         key = mouse_info_to_key(w,b,ba,bm);
      }
      else
         mouse_key = FALSE;
#endif
      break;
   }
   if (current_key == -1)
      current_key = 0;
   else
      current_key = current_key == 8 ? 0 : current_key++;
   lastkeys[current_key] = key;
   set_key_values(key,mouse_key);
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_reserved(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_reserved(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   bool line_numbers_only=TRUE;
   static RESERVED *curr_rsrvd;
   CHARTYPE *tmpbuf=NULL,*attr_string=NULL;
   short rc=RC_OK;
   static CHARTYPE rsrvd[100];
   short y=0,x=0;

   if (query_type == QUERY_EXTRACT
   &&  strcmp((DEFCHAR *)itemargs,"*") == 0)
      line_numbers_only = FALSE;

   number_variables = 0;
   curr_rsrvd = CURRENT_FILE->first_reserved;
   strcpy((DEFCHAR *)rsrvd,"");
   while(curr_rsrvd != NULL)
   {
      if (line_numbers_only)
      {
         y = strlen((DEFCHAR *)curr_rsrvd->spec) + 1;
         if ((x + y) > 80)
            break;
         strcat((DEFCHAR *)rsrvd,(DEFCHAR *)curr_rsrvd->spec);
         strcat((DEFCHAR *)rsrvd," ");
         x += y;
      }
      else
      {
         attr_string = get_colour_strings(curr_rsrvd->attr);
         if (attr_string == (CHARTYPE *)NULL)
         {
            return(EXTRACT_ARG_ERROR);
         }
         tmpbuf = (CHARTYPE *)(*the_malloc)(sizeof(CHARTYPE)*(strlen((DEFCHAR *)attr_string)+strlen((DEFCHAR *)curr_rsrvd->line)+strlen((DEFCHAR *)curr_rsrvd->spec)+3));
         if (tmpbuf == (CHARTYPE *)NULL)
         {
            display_error(30,(CHARTYPE *)"",FALSE);
            return(EXTRACT_ARG_ERROR);
         }
         strcpy((DEFCHAR *)tmpbuf,(DEFCHAR *)curr_rsrvd->spec);
         strcat((DEFCHAR *)tmpbuf," ");
         strcat((DEFCHAR *)tmpbuf,(DEFCHAR *)attr_string);
         (*the_free)(attr_string);
         strcat((DEFCHAR *)tmpbuf,(DEFCHAR *)curr_rsrvd->line);
         rc = set_rexx_variable(query_item[itemno].name,tmpbuf,strlen((DEFCHAR *)tmpbuf),++number_variables);
         (*the_free)(tmpbuf);
         if (rc == RC_SYSTEM_ERROR)
         {
            display_error(54,(CHARTYPE *)"",FALSE);
            return(EXTRACT_ARG_ERROR);
         }
      }
      curr_rsrvd = curr_rsrvd->next;
   }
   if (line_numbers_only)
   {
      if (x == 0)
         number_variables = 0;
      else
      {
         number_variables = 1;
         item_values[1].value = rsrvd;
         item_values[1].len = strlen((DEFCHAR *)rsrvd);
      }
   }
   else
   {
      sprintf((DEFCHAR *)rsrvd,"%d",number_variables);
      rc = set_rexx_variable(query_item[itemno].name,rsrvd,strlen((DEFCHAR *)rsrvd),0);
      if (rc == RC_SYSTEM_ERROR)
      {
         display_error(54,(CHARTYPE *)"",FALSE);
         number_variables = EXTRACT_ARG_ERROR;
      }
      else
         number_variables = EXTRACT_VARIABLES_SET;
   }
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_rexx(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_rexx(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = get_rexx_interpreter_version(rsrvd);
 item_values[1].len = strlen((DEFCHAR *)rsrvd);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_rexxoutput(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_rexxoutput(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (CAPREXXOUTx)
 {
    item_values[1].value = (CHARTYPE *)"FILE";
    item_values[1].len = 4;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)"DISPLAY";
    item_values[1].len = 7;
 }
 sprintf((DEFCHAR *)num1,"%ld",CAPREXXMAXx);
 item_values[2].value = num1;
 item_values[2].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_rightedge_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_rightedge_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return number_variables;
 }
 getyx(CURRENT_WINDOW,y,x);
 return set_boolean_value((bool)(CURRENT_VIEW->current_window == WINDOW_FILEAREA && x == getmaxx(CURRENT_WINDOW)-1),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_ring(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_ring(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   VIEW_DETAILS *curr;
   FILE_DETAILS *first_view_file=NULL;
   bool process_view=FALSE;
   LINETYPE lineno=0L;
   LENGTHTYPE col=0;
   register int i=0,j=0;
   int offset=0,off=0;
   bool view_being_displayed=FALSE;
   CHARTYPE save_msgline_base = CURRENT_VIEW->msgline_base;
   short save_msgline_off = CURRENT_VIEW->msgline_off;
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;

   if (compatible_feel == COMPAT_XEDIT)
      offset = 1;

   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = POSITION_TOP;
      CURRENT_VIEW->msgline_off    = 1;
      CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,number_of_files);
      CURRENT_VIEW->msgmode_status = TRUE;
   }
   else
      number_variables = offset;

   curr = vd_current;
   for (j=0;j<number_of_files;)
   {
      process_view = TRUE;
      if (curr->file_for_view->file_views > 1)
      {
         if (first_view_file == curr->file_for_view)
            process_view = FALSE;
         else
            first_view_file = curr->file_for_view;
      }
      if (process_view)
      {
         j++;
         view_being_displayed=FALSE;
         for (i=0;i<display_screens;i++)
         {
            if (SCREEN_VIEW(i) == curr)
            {
               view_being_displayed = TRUE;
               get_current_position(i,&lineno,&col);
            }
         }
         if (!view_being_displayed)
         {
            lineno = (curr->current_window==WINDOW_COMMAND)?curr->current_line:curr->focus_line;
            col = curr->current_column;
         }
         if (compatible_look == COMPAT_XEDIT)
            sprintf((DEFCHAR *)rsrvd,"%s%s Size=%ld Line=%ld Col=%d Alt=%d,%d",
                  curr->file_for_view->fpath,
                  curr->file_for_view->fname,
                  curr->file_for_view->number_lines,
                  lineno,col,
                  curr->file_for_view->autosave_alt,
                  curr->file_for_view->save_alt);
         else
            sprintf((DEFCHAR *)rsrvd,"%s%s Line=%ld Col=%d Size=%ld Alt=%d,%d",
                  curr->file_for_view->fpath,
                  curr->file_for_view->fname,
                  lineno,col,
                  curr->file_for_view->number_lines,
                  curr->file_for_view->autosave_alt,
                  curr->file_for_view->save_alt);
         if (query_type == QUERY_QUERY)
         {
            display_error(0,rsrvd,TRUE);
         }
         else
         {
            number_variables++;
            item_values[number_variables].len = strlen((DEFCHAR *)rsrvd);
            memcpy((DEFCHAR*)trec+off,(DEFCHAR*)rsrvd,(item_values[number_variables].len)+1);
            item_values[number_variables].value = trec+off;
            off += (item_values[number_variables].len)+1;
         }
      }
      curr = curr->next;
      if (curr == NULL)
         curr = vd_first;
   }

   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = save_msgline_base;
      CURRENT_VIEW->msgline_off    = save_msgline_off;
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      number_variables = EXTRACT_VARIABLES_SET;
   }
   else
   {
      if ( offset )
      {
         sprintf( (DEFCHAR *)num1, "%d", number_variables - 1 );
         item_values[1].value = num1;
         item_values[1].len = strlen( (DEFCHAR *)num1 );
      }
   }

   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_scale(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_scale(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 set_on_off_value(CURRENT_VIEW->scale_on,1);
 if (CURRENT_VIEW->scale_base == POSITION_MIDDLE)
    sprintf((DEFCHAR *)rsrvd,"M%+d",CURRENT_VIEW->scale_off);
 else
    sprintf((DEFCHAR *)rsrvd,"%d",CURRENT_VIEW->scale_off);
 item_values[2].value = rsrvd;
 item_values[2].len = strlen((DEFCHAR *)rsrvd);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_scope(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_scope(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (CURRENT_VIEW->scope_all)
 {
    item_values[1].value = (CHARTYPE *)"ALL";
    item_values[1].len = 3;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)"DISPLAY";
    item_values[1].len = 7;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_screen(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_screen(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%d",display_screens);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 if (horizontal)
 {
    item_values[2].value = (CHARTYPE *)"HORIZONTAL";
    item_values[2].len = 10;
 }
 else
 {
    item_values[2].value = (CHARTYPE *)"VERTICAL";
    item_values[2].len = 8;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_select(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_select(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 _LINE *curr=NULL;
 short x=0;

 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,
             (compatible_feel==COMPAT_XEDIT)?CURRENT_VIEW->current_line:get_true_line(TRUE),
             CURRENT_FILE->number_lines);
 sprintf((DEFCHAR *)num1,"%d",curr->select);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 x = 0;
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,1L,CURRENT_FILE->number_lines);
 while(curr->next != NULL)
 {
    if (curr->select > x)
       x = curr->select;
    curr = curr->next;
 }
 sprintf((DEFCHAR *)num2,"%d",x);
 item_values[2].value = num2;
 item_values[2].len = strlen((DEFCHAR *)num2);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_shadow(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_shadow(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->shadow,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_shadow_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_shadow_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 bool bool_flag=FALSE;
 short y=0,x=0;

 switch(CURRENT_VIEW->current_window)
 {
    case WINDOW_FILEAREA:
       if (batch_only)
       {
          bool_flag = FALSE;
          break;
       }
       getyx(CURRENT_WINDOW,y,x);
       if (CURRENT_SCREEN.sl[y].line_type == LINE_SHADOW)
          bool_flag = TRUE;
       else
          bool_flag = FALSE;
       break;
    default:
       bool_flag = FALSE;
       break;
 }
 return set_boolean_value((bool)bool_flag,(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_shift_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_shift_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 int shift=0;

 get_key_name(lastkeys[current_key],&shift);
 return set_boolean_value((bool)(shift & SHIFT_SHIFT),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_showkey(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_showkey(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 int key=0;

 if (strcmp((DEFCHAR *)itemargs,"") == 0
 || ((key = find_key_value(itemargs)) == -1))
 {
    item_values[1].value = (CHARTYPE *)"INVALID KEY";
    item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
 }
 else
 {
    function_key(key,OPTION_EXTRACT,FALSE);
    number_variables = EXTRACT_VARIABLES_SET;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_size(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_size(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%ld",CURRENT_FILE->number_lines);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_spacechar_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_spacechar_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 CHARTYPE cursor_char=0;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return 1;
 }
 cursor_char = (CHARTYPE)(winch(CURRENT_WINDOW) & A_CHARTEXT);
 return set_boolean_value((bool)(cursor_char == ' '),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_statusline(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_statusline(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 switch(STATUSLINEx)
 {
    case 'B':
       item_values[1].value = (CHARTYPE *)"BOTTOM";
       item_values[1].len = 6;
       break;
    case 'T':
       item_values[1].value = (CHARTYPE *)"TOP";
       item_values[1].len = 3;
       break;
    case 'O':
       item_values[1].value = (CHARTYPE *)"OFF";
       item_values[1].len = 3;
       break;
    case 'G':
       item_values[1].value = (CHARTYPE *)"GUI";
       item_values[1].len = 3;
       break;
    }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_stay(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_stay(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->stay,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_synonym( short number_variables, short itemno, CHARTYPE *itemargs, CHARTYPE query_type, LINETYPE argc, CHARTYPE *arg, LINETYPE arglen )
#else
static short extract_synonym( number_variables, itemno, itemargs, query_type, argc, arg, arglen )
short number_variables;
short itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
   bool found=FALSE;
   CHARTYPE save_msgline_base = CURRENT_VIEW->msgline_base;
   short save_msgline_off = CURRENT_VIEW->msgline_off;
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
   CHARTYPE *ptr;
   DEFINE *curr;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("query.c:   extract_synonym");
   if ( blank_field( itemargs ) )
   {
      if ( CURRENT_VIEW->synonym )
      {
         item_values[1].value = (CHARTYPE *)"ON";
         item_values[1].len = 2;
         number_variables = 1;
      }
      else
      {
         item_values[1].value = (CHARTYPE *)"OFF";
         item_values[1].len = 3;
         number_variables = 1;
      }
      TRACE_RETURN();
      return( number_variables );
   }
#if 0
   else if ( strcmp( (DEFCHAR*)itemargs, "*" ) == 0 )
   {
      if (query_type == QUERY_QUERY)
      {
         for (i=0,curr=first_synonym;curr!=NULL;curr=curr->next,i++);
         CURRENT_VIEW->msgline_base   = POSITION_TOP;
         CURRENT_VIEW->msgline_off    = 1;
         CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,i);
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      for ( curr = first_synonym; curr != NULL; curr = curr->next )
      {
         sprintf((DEFCHAR *)rsrvd,"%s%s %1.*s %s%s %s",
           (query_type == QUERY_QUERY) ? (DEFCHAR *)"synonym " : "",
           curr->synonym,
           curr->def_funkey,
           curr->synonym,
           (curr->linend) ? (char*)"LINEND" : "",
           (curr->linend) ? (char*)"?" : "",
           curr->def_params );

         if (query_type == QUERY_QUERY)
            display_error(0,rsrvd,TRUE);
         else
         {
            number_variables++;
            item_values[number_variables].len = strlen((DEFCHAR *)rsrvd);
            memcpy((DEFCHAR*)trec+off,(DEFCHAR*)rsrvd,(item_values[number_variables].len)+1);
            item_values[number_variables].value = trec+off;
            off += (item_values[number_variables].len)+1;
         }
      }
   }
#endif
   else
   {
      if (query_type == QUERY_QUERY)
      {
         CURRENT_VIEW->msgline_base   = POSITION_TOP;
         CURRENT_VIEW->msgline_off    = 1;
         CURRENT_VIEW->msgline_rows   = 1;
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      /*
       * Find a match for the supplied mask or magic number
       */
      for ( curr = first_synonym; curr != NULL; curr = curr->next )
      {
         if ( equal( curr->synonym, itemargs, curr->def_funkey ) )
         {
            found = TRUE;
            break;
         }
      }
      if (found)
      {
         strcpy( (DEFCHAR *)rsrvd, "" );
         ptr = build_synonym_definition( curr->synonym, rsrvd, curr );
      }
      else
      {
         ptr = NULL;
      }
      if (query_type == QUERY_QUERY)
      {
         if ( found)
         {
            sprintf((DEFCHAR *)trec,"%s%s %d %s %c",
               (query_type == QUERY_QUERY) ? "synonym " : "",
               itemargs,
               curr->def_funkey,
               ptr,
               curr->linend );
         }
         else
         {
            sprintf((DEFCHAR *)trec,"%s%s %d %s",
               (query_type == QUERY_QUERY) ? "synonym " : "",
               itemargs,
               (int)strlen( (DEFCHAR *)itemargs),
               itemargs );
         }
         display_error(0,trec,TRUE);
      }
      else
      {
         if ( found )
         {
            item_values[1].value = curr->synonym;
            item_values[1].len = strlen( (DEFCHAR *)curr->synonym );
            sprintf( (DEFCHAR *)num1,"%d", curr->def_funkey );
            item_values[2].value = num1;
            item_values[2].len = strlen((DEFCHAR *)num1);
            item_values[3].value = ptr;
            item_values[3].len = strlen( (DEFCHAR *)ptr );
            sprintf( (DEFCHAR *)num2,"%c", curr->linend );
            item_values[4].value = num2;
            item_values[4].len = strlen( (DEFCHAR *)num2 );
            number_variables = 4;
         }
         else
         {
            item_values[1].value = itemargs;
            item_values[1].len = strlen( (DEFCHAR *)itemargs );
            sprintf( (DEFCHAR *)num1,"%d", item_values[1].len );
            item_values[2].value = num1;
            item_values[3].value = itemargs;
            item_values[3].len = strlen( (DEFCHAR *)itemargs );
            number_variables = 3;
         }
      }
   }

   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_base   = save_msgline_base;
      CURRENT_VIEW->msgline_off    = save_msgline_off;
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      rc = EXTRACT_VARIABLES_SET;
   }
   else
      rc = number_variables;


   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_tabkey(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_tabkey(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (tabkey_overwrite == 'T')
 {
    item_values[1].value = (CHARTYPE *)"TAB";
    item_values[1].len = 3;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)"CHARACTER";
    item_values[1].len = 9;
 }
 if (tabkey_insert == 'T')
 {
    item_values[2].value = (CHARTYPE *)"TAB";
    item_values[2].len = 3;
 }
 else
 {
    item_values[2].value = (CHARTYPE *)"CHARACTER";
    item_values[2].len = 9;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_tabline(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_tabline(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 set_on_off_value(CURRENT_VIEW->tab_on,1);
 if (CURRENT_VIEW->tab_base == POSITION_MIDDLE)
    sprintf((DEFCHAR *)rsrvd,"M%+d",CURRENT_VIEW->tab_off);
 else
    sprintf((DEFCHAR *)rsrvd,"%d",CURRENT_VIEW->tab_off);
 item_values[2].value = rsrvd;
 item_values[2].len = strlen((DEFCHAR *)rsrvd);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_tabs(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_tabs(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 register int i=0;

 strcpy((DEFCHAR *)rsrvd,"");
 if (CURRENT_VIEW->tabsinc)
   {
    sprintf((DEFCHAR *)rsrvd,"INCR %d",CURRENT_VIEW->tabsinc);
   }
 else
   {
    for (i=0;i<CURRENT_VIEW->numtabs;i++)
       {
        sprintf((DEFCHAR *)num1,"%d ",CURRENT_VIEW->tabs[i]);
        strcat((DEFCHAR *)rsrvd,(DEFCHAR *)num1);
       }
    if (query_type == QUERY_QUERY
    ||  query_type == QUERY_STATUS)
       rsrvd[COLS-7] = '\0';
   }
 item_values[1].value = rsrvd;
 item_values[1].len = strlen((DEFCHAR *)rsrvd);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_tabsin(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_tabsin(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 set_on_off_value(TABI_ONx,1);
 sprintf((DEFCHAR *)num1,"%d",TABI_Nx);
 item_values[2].value = num1;
 item_values[2].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_tabsout(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_tabsout(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 set_on_off_value(CURRENT_FILE->tabsout_on,1);
 sprintf((DEFCHAR *)num1,"%d",CURRENT_FILE->tabsout_num);
 item_values[2].value = num1;
 item_values[2].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_targetsave(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_targetsave(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   int len=0;
   switch( TARGETSAVEx )
   {
      case TARGET_ALL:
         item_values[1].value = (CHARTYPE *)"ALL";
         item_values[1].len = 3;
         break;
      case TARGET_UNFOUND:
         item_values[1].value = (CHARTYPE *)"NONE";
         item_values[1].len = 4;
         break;
      default:
         strcpy( (DEFCHAR *)rsrvd, "" );
         if ( TARGETSAVEx & TARGET_STRING )
            strcat( (DEFCHAR *)rsrvd, "STRING " );
         if ( TARGETSAVEx & TARGET_REGEXP )
            strcat( (DEFCHAR *)rsrvd, "REGEXP " );
         if ( TARGETSAVEx & TARGET_ABSOLUTE )
            strcat( (DEFCHAR *)rsrvd, "ABSOLUTE " );
         if ( TARGETSAVEx & TARGET_RELATIVE )
            strcat( (DEFCHAR *)rsrvd, "RELATIVE " );
         if ( TARGETSAVEx & TARGET_POINT )
            strcat( (DEFCHAR *)rsrvd, "POINT " );
         if ( TARGETSAVEx & TARGET_BLANK )
            strcat( (DEFCHAR *)rsrvd, "BLANK " );
         len = strlen( (DEFCHAR *)rsrvd );
         if ( rsrvd[len-1] == ' ' )
         {
            rsrvd[len-1] = '\0';
            len--;
         }
         item_values[1].value = rsrvd;
         item_values[1].len = len;
         break;
   }
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_terminal(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_terminal(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = term_name;
 item_values[1].len = strlen((DEFCHAR *)term_name);
 return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_thighlight(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_thighlight(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->thighlight_on,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_timecheck(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_timecheck(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_FILE->timecheck,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_tof(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_tof(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_TOF,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_tofeof(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_tofeof(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->tofeof,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_tof_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_tof_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(FOCUS_TOF && CURRENT_VIEW->current_window != WINDOW_COMMAND),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_topedge_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_topedge_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 short y=0,x=0;

 if (batch_only)
 {
    item_values[1].value = (CHARTYPE *)"0";
    item_values[1].len = 1;
    return 1;
 }
 getyx(CURRENT_WINDOW,y,x);
 return set_boolean_value((bool)(CURRENT_VIEW->current_window == WINDOW_FILEAREA && y == 0),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_trailing(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_trailing(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
   switch( CURRENT_FILE->trailing )
   {
      case TRAILING_ON:
         item_values[1].value = (CHARTYPE *)"ON";
         item_values[1].len = 2;
         break;
      case TRAILING_OFF:
         item_values[1].value = (CHARTYPE *)"OFF";
         item_values[1].len = 3;
         break;
      case TRAILING_SINGLE:
         item_values[1].value = (CHARTYPE *)"SINGLE";
         item_values[1].len = 6;
         break;
      case TRAILING_EMPTY:
         item_values[1].value = (CHARTYPE *)"EMPTY";
         item_values[1].len = 5;
         break;
      default:
         break;
   }
   return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_typeahead(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_typeahead(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(TYPEAHEADx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_undoing(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_undoing(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_FILE->undoing,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_untaa(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_untaa(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(UNTAAx,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_verify(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_verify(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num3,"%d %d",CURRENT_VIEW->verify_start,CURRENT_VIEW->verify_end);
 item_values[1].value = num3;
 item_values[1].len = strlen((DEFCHAR *)num3);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_vershift(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_vershift(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)rsrvd,"%ld",(LINETYPE)CURRENT_VIEW->verify_col - (LINETYPE)CURRENT_VIEW->verify_start);
 item_values[1].value = rsrvd;
 item_values[1].len = strlen((DEFCHAR *)rsrvd);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_verone_function(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_verone_function(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_boolean_value((bool)(CURRENT_VIEW->verify_col == 1),(short)1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_version(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_version(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = (CHARTYPE *)"THE";
 item_values[1].len = 3;
 item_values[2].value = (CHARTYPE *)the_version;
 item_values[2].len = strlen((DEFCHAR *)the_version);
 item_values[3].value = (CHARTYPE *)"???";
#if defined(DOS)
# if defined(EMX)
 if (_osmode == DOS_MODE)
    item_values[3].value = (CHARTYPE *)"DOS";
 else
    item_values[3].value = (CHARTYPE *)"OS2";
#else
 item_values[3].value = (CHARTYPE *)"DOS";
# endif
#endif
#if defined(OS2)
# if defined(EMX)
 if (_osmode == DOS_MODE)
    item_values[3].value = (CHARTYPE *)"DOS";
 else
    item_values[3].value = (CHARTYPE *)"OS2";
#else
 item_values[3].value = (CHARTYPE *)"OS2";
# endif
#endif
#if defined(UNIX)
# if defined(__QNX__)
 item_values[3].value = (CHARTYPE *)"QNX";
# else
 item_values[3].value = (CHARTYPE *)"UNIX";
# endif
#endif
#if defined(XCURSES)
 item_values[3].value = (CHARTYPE *)"X11";
#endif
#if defined(MSWIN)
 item_values[3].value = (CHARTYPE *)"MS-WINDOWS";
#endif
#if defined(WIN32)
 item_values[3].value = (CHARTYPE *)"WIN32";
#endif
#if defined(AMIGA)
 item_values[3].value = (CHARTYPE *)"AMIGA";
#endif
#if defined(__BEOS__)
 item_values[3].value = (CHARTYPE *)"BEOS";
#endif
 item_values[3].len = strlen((DEFCHAR *)item_values[3].value);
 item_values[4].value = (CHARTYPE *)the_release;
 item_values[4].len = strlen((DEFCHAR *)item_values[4].value);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_width(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_width(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%d",max_line_length);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_word(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_word(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 if (CURRENT_VIEW->word == 'A')
 {
    item_values[1].value = (CHARTYPE *)"ALPHANUM";
    item_values[1].len = 8;
 }
 else
 {
    item_values[1].value = (CHARTYPE *)"NONBLANK";
    item_values[1].len = 8;
 }
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_wordwrap(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_wordwrap(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->wordwrap,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_wrap(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_wrap(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 return set_on_off_value(CURRENT_VIEW->wrap,1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_xterminal(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_xterminal(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 item_values[1].value = xterm_program;
 item_values[1].len = strlen((DEFCHAR *)xterm_program);
 return number_variables;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short extract_zone(short number_variables,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
#else
static short extract_zone(number_variables,itemno,itemargs,query_type,argc,arg,arglen)
short number_variables,itemno;
CHARTYPE *itemargs;
CHARTYPE query_type;
LINETYPE argc;
CHARTYPE *arg;
LINETYPE arglen;
#endif
/***********************************************************************/
{
 sprintf((DEFCHAR *)num1,"%d",CURRENT_VIEW->zone_start);
 item_values[1].value = num1;
 item_values[1].len = strlen((DEFCHAR *)num1);
 sprintf((DEFCHAR *)num2,"%d",CURRENT_VIEW->zone_end);
 item_values[2].value = num2;
 item_values[2].len = strlen((DEFCHAR *)num2);
 return number_variables;
}

/***********************************************************************/
#ifdef HAVE_PROTO
int number_function_item( void )
#else
int number_function_item()
#endif
/***********************************************************************/
{
 return NUMBER_FUNCTION_ITEM;
}

/***********************************************************************/
#ifdef HAVE_PROTO
int number_query_item( void )
#else
int number_query_item()
#endif
/***********************************************************************/
{
 return NUMBER_QUERY_ITEM;
}

/***********************************************************************/
#ifdef HAVE_PROTO
void format_options( CHARTYPE *buf )
#else
void format_options( buf )
CHARTYPE *buf;
#endif
/***********************************************************************/
{
 _LINE *curr=first_option;
 int tail,col,itemno,linelen,valuelen,number_variables,max_col=0,off;

 TRACE_FUNCTION("query.c:   format_options");

 while(curr != NULL)
 {
    itemno = curr->length;
    tail = curr->flags.new_flag;
    number_variables = get_item_values(1,query_item[itemno].item_number,(CHARTYPE *)"",QUERY_EXTRACT,0L,NULL,0L);
    if (number_variables >= tail)
    {
       col = curr->select;
       linelen = curr->line?strlen((DEFCHAR *)curr->line):0;
       valuelen = (curr->save_select)?min(curr->save_select,item_values[tail].len):item_values[tail].len;
       if (curr->line)
       {
          memcpy(buf+col,(DEFCHAR *)curr->line,linelen);
       }
       memcpy(buf+col+linelen,(DEFCHAR *)item_values[tail].value,valuelen);
       off = col+linelen+valuelen;
       if (off > max_col)
          max_col = off;
    }
    curr = curr->next;
 }
 buf[max_col] = '\0';
 TRACE_RETURN();
 return;
}

