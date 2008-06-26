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
$Id: command.h,v 1.14 2002/08/13 09:11:47 mark Exp $
*/
/*
 * Definitions of various CUA behaviours
 * CUA_NONE:
 * - no affect. ie no automatic processing is done, but the
 *   individual command can handle CUA behaviour itself. eg
 *   CURSOR command would need to handle CUA behaviour.
 *
 * CUA_DELETE_BLOCK
 * - save first line/column position (not anchor position)
 * - delete the marked CUA block (ignores other blocks )
 * - move cursor to first line/column (not anchor position)
 *
 * CUA_RESET_BLOCK
 * - reset CUA block
 */
/*
 * Definitions of various THIGHLIGHT behaviours
 * THIGHLIGHT_NONE:
 * - no affect. ie no automatic resetting of the THIGHLIGHT
 *   area.
 *
 * THIGHLIGHT_RESET_ALL
 * - reset the THIGHLIGHT area in all circumstances
 *
 * THIGHLIGHT_RESET_FOCUS
 * - reset CUA THIGHLIGHT area if this command executed on
 *   the focus line and this line contains the THIGHLIGHT area
 */

struct commands
{
   CHARTYPE *text;
   unsigned short min_len;
   int funkey;
#ifdef HAVE_PROTO
   short (*function)(CHARTYPE *);
#else
   short (*function)();
#endif
   bool valid_batch_command;
   bool set_command;
   bool sos_command;
   bool valid_in_readonly;
   bool edit_command;
   bool strip_param;
   bool valid_readv;
   bool valid_input_full;
   bool valid_for_reprofile;
   short cua_behaviour;
   short thighlight_behaviour;
   CHARTYPE *params;
};
typedef struct commands COMMAND;

#include "proto.h"

/*---------------------- *** IMPORTANT *** ----------------------------*/
/* Make sure that there is no chance two or more commands can be       */
/* matched based on number of significant charcters.                   */
/*---------------------- *** IMPORTANT *** ----------------------------*/
/*                                                                  Batch/ Set / SOS / RO   /edit/strip     /readv/input/reprof/CUA             /THIGHLIGHT  */
COMMAND command[] =
{
 {(CHARTYPE *)"birthday",    8,  (-1),            Birthday,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"cursor",      0,  KEY_CURR,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"right"},  /* comm1.c */
 {(CHARTYPE *)"cursor",      0,  KEY_CURL,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"left"},  /* comm1.c */
 {(CHARTYPE *)"cursor",      0,  KEY_CURD,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"down"},  /* comm1.c */
 {(CHARTYPE *)"cursor",      0,  KEY_CURU,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"up"},  /* comm1.c */
#else
 {(CHARTYPE *)"cursor",      0,  KEY_RIGHT,       Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"right"},  /* comm1.c */
 {(CHARTYPE *)"cursor",      0,  KEY_LEFT,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"left"},  /* comm1.c */
 {(CHARTYPE *)"cursor",      0,  KEY_DOWN,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"down"},  /* comm1.c */
 {(CHARTYPE *)"cursor",      0,  KEY_UP,          Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"up"},  /* comm1.c */
#endif
#if defined(HAVE_SB_INIT)
 {(CHARTYPE *)"scrollbar",   0,  KEY_SF,          ScrollbarVert,    FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* mouse.c */
 {(CHARTYPE *)"scrollbar",   0,  KEY_SR,          ScrollbarHorz,    FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* mouse.c */
#endif
 {(CHARTYPE *)"add",         1,  (-1),            Add,              TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""}, /* comm1.c */
 {(CHARTYPE *)"alert",       5,  (-1),            Alert,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"all",         3,  (-1),            All,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"arbchar",     3,  (-1),            Arbchar,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"alt",         3,  (-1),            Alt,              TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"autocolour", 10,  (-1),            Autocolour,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"autocolor",   9,  (-1),            Autocolour,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"autosave",    2,  (-1),            Autosave,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"autoscroll",  6,  (-1),            Autoscroll,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"backward",    2,  KEY_PGUP,        Backward,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm1.c */
#else
 {(CHARTYPE *)"backward",    2,  KEY_PPAGE,       Backward,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm1.c */
 {(CHARTYPE *)"backward",    2,  KEY_PrevScreen,  Backward,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm1.c */
#endif
 {(CHARTYPE *)"backup",      4,  (-1),            Backup,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"beep",        4,  (-1),            BeepSound,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"bottom",      1,  (-1),            Bottom,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"boundmark",   9,  (-1),            Boundmark,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"cancel",      3,  (-1),            Cancel,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"cappend",     2,  (-1),            Cappend,          TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"case",        4,  (-1),            Case,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"ccancel",     2,  (-1),            Ccancel,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"cdelete",     2,  (-1),            Cdelete,          TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"cfirst",      2,  (-1),            Cfirst,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"change",      1,  (-1),            Change,           TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"cinsert",     2,  (-1),            Cinsert,          TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"clast",       3,  (-1),            Clast,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"clearerrorkey", 6,(-1),            Clearerrorkey,    TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"clearscreen", 6,  (-1),            Clearscreen,      TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"clocate",     2,  (-1),            Clocate,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"clock",       5,  (-1),            Clock,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"cmatch",      6,  KEY_F7,          Cmatch,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
#else
 {(CHARTYPE *)"cmatch",      6,  KEY_F(7),        Cmatch,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
#endif
 {(CHARTYPE *)"cmdarrows",   4,  (-1),            Cmdarrows,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"cmdline",     3,  (-1),            Cmdline,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"cmsg",        4,  (-1),            Cmsg,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"command",     7,  (-1),            THECommand,       TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"colour",      6,  (-1),            Colour,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"color",       5,  (-1),            Colour,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"colouring",   9,  (-1),            Colouring,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"coloring",    8,  (-1),            Colouring,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"compat",      6,  (-1),            Compat,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"compress",    4,  (-1),            Compress,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"copy",        4,  (-1),            Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm1.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"controlchar", 8,  KEY_F11,         ControlChar,      FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
#if  defined(USE_XCURSES)
 {(CHARTYPE *)"copy",        0,  KEY_C_c,         Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block reset"},  /* comm1.c */
 {(CHARTYPE *)"copy",        0,  KEY_C_k,         Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block"},  /* comm1.c */
#  else
 {(CHARTYPE *)"copy",        0,  ALT_C,           Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block reset"},  /* comm1.c */
 {(CHARTYPE *)"copy",        0,  ALT_K,           Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block"},  /* comm1.c */
#  endif
 {(CHARTYPE *)"cursor",      3,  KEY_HOME,        Cursor,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"home save"},  /* comm1.c */
#else
 {(CHARTYPE *)"controlchar" ,8,  KEY_F(11),       ControlChar,      FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"copy",        0,  KEY_C_c,         Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block reset"},  /* comm1.c */
 {(CHARTYPE *)"copy",        0,  KEY_C_k,         Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block"},  /* comm1.c */
 {(CHARTYPE *)"cursor",      3,  KEY_HOME,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"home save"},  /* comm1.c */
 {(CHARTYPE *)"cursor",      0,  KEY_F16,         Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"home save"},  /* comm1.c */
#endif
 {(CHARTYPE *)"ctlchar",     3,  (-1),            Ctlchar,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"coverlay",    3,  (-1),            Coverlay,         TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"creplace",    2,  (-1),            Creplace,         TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm1.c */
 {(CHARTYPE *)"curline",     4,  (-1),            Curline,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"cursorstay",  8,  (-1),            CursorStay,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"define",      3,  (-1),            Define,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"delete",      3,  (-1),            DeleteLine,       TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""}, /* comm2.c */
#if defined(DOS) || defined(OS2) || defined(WIN32)
 {(CHARTYPE *)"delete",      0,  ALT_G,           DeleteLine,       FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block"}, /* comm2.c */
#else
 {(CHARTYPE *)"delete",      0,  KEY_C_g,         DeleteLine,       FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block"}, /* comm2.c */
#endif
 {(CHARTYPE *)"dialog",      6,  (-1),            Dialog,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"directory",   3,  (-1),            Directory,        FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"dirinclude",  4,  (-1),            Dirinclude,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"*"}, /* commset1.c */
 {(CHARTYPE *)"ls",          2,  (-1),            Directory,        FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"defsort",     7,  (-1),            Defsort,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"display",     4,  (-1),            Display,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"down",        1,  (-1),            THENext,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm3.c */
 {(CHARTYPE *)"duplicate",   3,  (-1),            Duplicate,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
#if defined(DOS) || defined(OS2) || defined(WIN32)
 {(CHARTYPE *)"duplicate",   0,  ALT_D,           Duplicate,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"1 BLOCK"},  /* comm2.c */
#else
 {(CHARTYPE *)"duplicate",   0,  KEY_C_d,         Duplicate,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"1 BLOCK"},  /* comm2.c */
 {(CHARTYPE *)"enter",       0,  KEY_C_m,         Enter,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"enter",       0,  KEY_ENTER,       Enter,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
#endif
#if defined(USE_EXTCURSES)
 {(CHARTYPE *)"enter",       0,  KEY_NEWL,        Enter,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
#else
 {(CHARTYPE *)"enter",       0,  KEY_RETURN,      Enter,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
#endif
 {(CHARTYPE *)"ecolor",      6,  (-1),            Ecolour,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"ecolour",     7,  (-1),            Ecolour,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"editv",       5,  (-1),            THEEditv,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"emsg",        4,  (-1),            Emsg,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"eolout",      4,  (-1),            Eolout,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"equivchar",   6,  (-1),            Equivchar,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"etmode",      6,  (-1),            Etmode,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"expand",      3,  (-1),            Expand,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"extract",     3,  (-1),            Extract,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"ffile",       2,  (-1),            Ffile,            TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
#if defined(DOS) || defined(OS2) || defined(WIN32)
 {(CHARTYPE *)"file",        4,  KEY_F3,          File,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"fillbox",     4,  ALT_F,           Fillbox,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
#else
#  if defined(USE_XCURSES)
 {(CHARTYPE *)"file",        4,  KEY_F3,          File,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
#  else
 {(CHARTYPE *)"file",        4,  KEY_F(3),        File,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"file",        0,  KEY_PF3,         File,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
#  endif
 {(CHARTYPE *)"fillbox",     4,  KEY_C_f,         Fillbox,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
#endif
 {(CHARTYPE *)"filetabs",    8,  (-1),            Filetabs,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"find",        1,  (-1),            Find,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"findup",      5,  (-1),            Findup,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"fup",         2,  (-1),            Findup,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"fext",        2,  (-1),            Fext,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"filename",    5,  (-1),            Filename,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"fmode",       2,  (-1),            Fmode,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"fname",       2,  (-1),            Fname,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"fpath",       2,  (-1),            Fpath,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"ftype",       2,  (-1),            Fext,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"forward",     2,  KEY_PGDN,        Forward,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm2.c */
 {(CHARTYPE *)"forward",     2,  CTL_PGDN,        Forward,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"*"}, /* comm2.c */
#else
 {(CHARTYPE *)"forward",     2,  KEY_NPAGE,       Forward,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm2.c */
 {(CHARTYPE *)"forward",     2,  KEY_NextScreen,  Forward,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm2.c */
#endif
 {(CHARTYPE *)"fullfname",   6,  (-1),            Fullfname,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"get",         3,  (-1),            Get,              TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"header",      4,  (-1),            THEHeader,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"help",        4,  KEY_F1,          Help,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
#else
 {(CHARTYPE *)"help",        4,  KEY_F(1),        Help,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"help",        0,  KEY_PF1,         Help,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
#endif
 {(CHARTYPE *)"hex",         3,  (-1),            Hex,              TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"hexdisplay",  7,  (-1),            Hexdisplay,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"hexshow",     4,  (-1),            Hexshow,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"highlight",   4,  (-1),            Highlight,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"hit",         3,  (-1),            Hit,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"idline",      2,  (-1),            Idline,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"impmacro",    5,  (-1),            Impmacro,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"impos",       5,  (-1),            Impos,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"impcmscp",    3,  (-1),            Impos,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"input",       1,  (-1),            Input,            TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm2.c */
 {(CHARTYPE *)"inputmode",   6,  (-1),            Inputmode,        TRUE, TRUE, FALSE,FALSE,FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"interface",   3,  (-1),            THEInterface,     TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"insertmode",  7,  KEY_INS,         Insertmode,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"toggle"},  /* commset1.c */
 {(CHARTYPE *)"locate",      1,  KEY_S_F1,        Locate,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
#else
 {(CHARTYPE *)"insertmode",  7,  KEY_IC,          Insertmode,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"toggle"},  /* commset1.c */
 {(CHARTYPE *)"locate",      1,  KEY_F(11),       Locate,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
#endif
 {(CHARTYPE *)"left",        2,  (-1),            Left,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"lineflag",    8,  (-1),            Lineflag,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"linend",      5,  (-1),            Linend,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"lowercase",   3,  (-1),            Lowercase,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"macro",       5,  (-1),            Macro,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"macro",       5,  (-1),            SetMacro,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"macroext",    6,  (-1),            Macroext,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"macropath",   6,  (-1),            Macropath,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"margins",     3,  (-1),            Margins,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"mark",        4,  (-1),            Mark,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"modify",      3,  (-1),            Modify,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"mouse",       5,  (-1),            Mouse,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
 {(CHARTYPE *)"",            0,  KEY_MOUSE,       THEMouse,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
#endif
#if defined(DOS) || defined(OS2) || defined(WIN32)
 {(CHARTYPE *)"mark",        0,  ALT_L,           Mark,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"line"},  /* comm3.c */
 {(CHARTYPE *)"mark",        0,  ALT_B,           Mark,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"box"},  /* comm3.c */
 {(CHARTYPE *)"move",        4,  ALT_M,           THEMove,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block reset"},  /* comm3.c */
 {(CHARTYPE *)"nextwindow",  5,  KEY_F2,          Nextwindow,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm3.c */
 {(CHARTYPE *)"overlaybox",  8,  ALT_O,           Overlaybox,       FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm3.c */
#else
 {(CHARTYPE *)"mark",        0,  KEY_C_l,         Mark,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"line"},  /* comm3.c */
 {(CHARTYPE *)"mark",        0,  KEY_C_b,         Mark,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"box"},  /* comm3.c */
 {(CHARTYPE *)"move",        4,  KEY_C_v,         THEMove,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"block reset"},  /* comm3.c */
#  if defined(USE_XCURSES)
 {(CHARTYPE *)"nextwindow",  5,  KEY_F2,          Nextwindow,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
#  else
 {(CHARTYPE *)"nextwindow",  5,  KEY_F(2),        Nextwindow,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"nextwindow",  5,  KEY_PF2,         Nextwindow,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
#  endif
 {(CHARTYPE *)"overlaybox",  8,  KEY_C_o,         Overlaybox,       FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm3.c */
#endif
 {(CHARTYPE *)"nfind",       2,  (-1),            Nfind,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"nfindup",     6,  (-1),            Nfindup,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"nfup",        3,  (-1),            Nfindup,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"msg",         3,  (-1),            Msg,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"msgline",     4,  (-1),            Msgline,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"msgmode",     4,  (-1),            Msgmode,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"newlines",    4,  (-1),            Newlines,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"next",        1,  (-1),            THENext,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}, /* comm3.c */
 {(CHARTYPE *)"nomsg",       5,  (-1),            Nomsg,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"nondisp",     4,  (-1),            Nondisp,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset1.c */
 {(CHARTYPE *)"nop",         3,  (-1),            Nop,              FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"number",      3,  (-1),            Number,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"osnowait",    3,  (-1),            Osnowait,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"osquiet",     3,  (-1),            Osquiet,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"osredir",     3,  (-1),            Osredir,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"os",          2,  (-1),            Os,               FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"dosnowait",   4,  (-1),            Osnowait,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"dosquiet",    4,  (-1),            Osquiet,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"dos",         3,  (-1),            Os,               FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"!",           1,  (-1),            Os,               FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm3.c */
 {(CHARTYPE *)"pagewrap",    8,  (-1),            Pagewrap,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"parser",      6,  (-1),            Parser,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"pending",     4,  (-1),            Pending,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"point",       1,  (-1),            Point,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"popup",       5,  (-1),            Popup,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"point",       0,  KEY_C_F11,       Point,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)".a"},/* commset2.c */
 {(CHARTYPE *)"locate",      0,  KEY_C_F12,       Locate,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)".a"},/* commset2.c */
#else
 {(CHARTYPE *)"point",       0,  KEY_F(31),       Point,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)".a"},/* commset2.c */
 {(CHARTYPE *)"locate",      0,  KEY_F(32),       Locate,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)".a"},/* commset2.c */
#endif
 {(CHARTYPE *)"position",    3,  (-1),            Position,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"prefix",      3,  (-1),            Prefix,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"preserve",    4,  (-1),            Preserve,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"prevwindow",  5,  (-1),            Prevwindow,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"printer",     7,  (-1),            THEPrinter,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"print",       2,  (-1),            Print,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"pscreen",     4,  (-1),            Pscreen,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"put",         3,  (-1),            Put,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"putd",        4,  (-1),            Putd,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm4.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"qquit",       2,  KEY_S_F3,        Qquit,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
#else
 {(CHARTYPE *)"qquit",       2,  KEY_F(13),       Qquit,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
#endif
 {(CHARTYPE *)"quit",        4,  (-1),            Quit,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"query",       1,  (-1),            Query,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"recover",     3,  KEY_F8,          Recover,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm4.c */
#else
 {(CHARTYPE *)"recover",     3,  KEY_F(8),        Recover,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm4.c */
#endif
 {(CHARTYPE *)"readv",       5,  (-1),            Readv,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"readonly",    8,  (-1),            THEReadonly,      TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"redit",       5,  (-1),            Redit,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"redraw",      6,  KEY_C_r,         Redraw,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"refresh",     7,  (-1),            THERefresh,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"repeat",      4,  (-1),            Repeat,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"replace",     1,  (-1),            Replace,          TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"regexp",      6,  (-1),            Regexp,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"reprofile",   6,  (-1),            Reprofile,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"reserved",    5,  (-1),            Reserved,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"reset",       3,  (-1),            Reset,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
#if defined(DOS) || defined(OS2) || defined(WIN32)
 {(CHARTYPE *)"reset",       0,  ALT_U,           Reset,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"block"},  /* comm4.c */
#else
 {(CHARTYPE *)"reset",       0,  KEY_C_u,         Reset,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"block"},  /* comm4.c */
#endif
 {(CHARTYPE *)"restore",     3,  (-1),            Restore,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"rexxoutput",  7,  (-1),            Rexxoutput,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"rgtleft",     7,  (-1),            Rgtleft,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"rexx",        4,  (-1),            THERexx,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"right",       2,  (-1),            Right,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"=",           1,  (-1),            Reexecute,        TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
#if defined(DOS) || defined(OS2) || defined(XCURSES) || defined(WIN32)
 {(CHARTYPE *)"?",           1,  KEY_F6,          Retrieve,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
#else
 {(CHARTYPE *)"?",           1,  KEY_F(6),        Retrieve,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
#endif
 {(CHARTYPE *)"save",        4,  (-1),            Save,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"scale",       4,  (-1),            Scale,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"schange",     3,  (-1),            Schange,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"screen",      3,  (-1),            THEScreen,        FALSE,TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"scope",       5,  (-1),            Scope,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"select",      3,  (-1),            Select,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"set",         3,  (-1),            Set,              TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"shadow",      4,  (-1),            Shadow,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"shift",       2,  (-1),            Shift,            TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"showkey",     4,  (-1),            ShowKey,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"slk",         3,  (-1),            Slk,              TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"sort",        4,  (-1),            Sort,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"sos",         3,  (-1),            Sos,              FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"span",        4,  (-1),            Span,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"spill",       5,  (-1),            Spill,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"bottomedge",  7,  (-1),            Sos_bottomedge,   FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"current",     4,  (-1),            Sos_current,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"endchar",     4,  (-1),            Sos_endchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"execute",     2,  (-1),            Sos_execute,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"firstchar",   7,  (-1),            Sos_firstchar,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"firstcol",    7,  (-1),            Sos_firstcol,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"instab",      6,  (-1),            Sos_instab,       FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"lastcol",     6,  (-1),            Sos_lastcol,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"leftedge",    5,  (-1),            Sos_leftedge,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"marginl",     7,  (-1),            Sos_marginl,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"marginr",     7,  (-1),            Sos_marginr,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"pastecmdline",6,  KEY_C_p,         Sos_pastecmdline, FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"parindent",   6,  (-1),            Sos_parindent,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"prefix",      3,  (-1),            Sos_prefix,       FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"qcmnd",       2,  (-1),            Sos_qcmnd,        FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"rightedge",   6,  (-1),            Sos_rightedge,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"tabfieldb",   9,  (-1),            Sos_tabfieldb,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"tabfieldf",   8,  (-1),            Sos_tabfieldf,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"topedge",     4,  (-1),            Sos_topedge,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"blockstart",  6,  (-1),            Sos_blockstart,   FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"blockend",    6,  (-1),            Sos_blockend,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"cursoradj",   7,  KEY_C_a,         Sos_cursoradj,    FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"cursorshift",11,  (-1),            Sos_cursorshift,  FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"cuadelback",  8,  (-1),            Sos_cuadelback,   FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"cuadelchar",  7,  (-1),            Sos_cuadelchar,   FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"addline",     3,  KEY_F4,          Sos_addline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delback",     5,  KEY_BKSP,        Sos_delback,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delchar",     4,  KEY_DEL,         Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delend",      4,  CTL_END,         Sos_delend,       FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delline",     4,  KEY_F9,          Sos_delline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"doprefix",    5,  KEY_NUMENTER,    Sos_doprefix,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"settab",      6,  (-1),            Sos_settab,       FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"startendchar",9,  KEY_END,         Sos_startendchar, FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"makecurr",    8,  KEY_F5,          Sos_makecurr,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"tabb",        4,  KEY_S_TAB,       Sos_tabb,         FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"tabf",        3,  KEY_TAB,         Sos_tabf,         FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"tabwordb",    8,  CTL_LEFT,        Sos_tabwordb,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"tabwordf",    7,  CTL_RIGHT,       Sos_tabwordf,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"undo",        4,  KEY_ESC,         Sos_undo,         FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"spltjoin",    8,  KEY_F10,         Spltjoin,         FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm4.c */
#  if defined(USE_XCURSES)
 {(CHARTYPE *)"delword",     4,  KEY_C_w,         Sos_delword,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"edit",        4,  KEY_C_x,         Sos_edit,         FALSE,FALSE,TRUE, TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"split",       2,  KEY_C_s,         Split,            FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"aligned cursor"},  /* comm4.c */
 {(CHARTYPE *)"join",        1,  KEY_C_j,         Join,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"aligned cursor"},  /* comm2.c */
#  else
 {(CHARTYPE *)"delword",     4,  ALT_W,           Sos_delword,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"edit",        4,  ALT_X,           Sos_edit,         FALSE,FALSE,TRUE, TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"split",       2,  ALT_S,           Split,            FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"aligned cursor"},  /* comm4.c */
 {(CHARTYPE *)"join",        1,  ALT_J,           Join,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"aligned cursor"},  /* comm2.c */
#  endif
#else
 {(CHARTYPE *)"addline",     3,  KEY_F(4),        Sos_addline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"addline",     3,  KEY_PF4,         Sos_addline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"tabb",        4,  (-1),            Sos_tabb,         FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
#ifdef OLD_DELBACK_DELCHAR
 {(CHARTYPE *)"delback",     4,  127,             Sos_delback,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delchar",     4,  8,               Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delchar",     4,  KEY_DC,          Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
#else
 {(CHARTYPE *)"delback",     4,  KEY_C_h,         Sos_delback,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delback",     4,  KEY_BACKSPACE,   Sos_delback,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delchar",     4,  KEY_Remove,      Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delchar",     4,  KEY_DELETE,      Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delchar",     4,  KEY_DC,          Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
#endif
 {(CHARTYPE *)"delend",      4,  KEY_C_e,         Sos_delend,       FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delline",     4,  KEY_F(9),        Sos_delline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"delword",     4,  KEY_C_w,         Sos_delword,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"doprefix",    5,  KEY_C_p,         Sos_doprefix,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"edit",        4,  KEY_C_x,         Sos_edit,         FALSE,FALSE,TRUE, TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"startendchar",9,  KEY_F(20),       Sos_startendchar, FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"makecurr",    8,  KEY_F(5),        Sos_makecurr,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
#if defined(USE_EXTCURSES)
 {(CHARTYPE *)"tabf",        3,  KEY_TAB,         Sos_tabf,         FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
#else
 {(CHARTYPE *)"tabf",        3,  KEY_C_i,         Sos_tabf,         FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
#endif
 {(CHARTYPE *)"tabwordb",    8,  KEY_F(18),       Sos_tabwordb,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"tabwordf",    7,  KEY_F(19),       Sos_tabwordf,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"undo",        4,  KEY_C_q,         Sos_undo,         FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"split",       2,  KEY_C_s,         Split,            FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)"aligned cursor"},  /* comm4.c */
 {(CHARTYPE *)"join",        1,  KEY_C_j,         Join,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)"aligned cursor"},  /* comm2.c */
 {(CHARTYPE *)"spltjoin",    8,  KEY_F(10),       Spltjoin,         FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm4.c */
#endif
 {(CHARTYPE *)"lineadd",     5,  (-1),            Sos_addline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"linedel",     5,  (-1),            Sos_delline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* commsos.c */
 {(CHARTYPE *)"ssave",       2,  (-1),            Ssave,            TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"status",      4,  (-1),            Status,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
 {(CHARTYPE *)"statusline",  7,  (-1),            Statusline,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"statopt",     7,  (-1),            Statopt,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"stay",        4,  (-1),            Stay,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"synonym",     3,  (-1),            Synonym,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"suspend",     4,  KEY_C_z,         Suspend,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm4.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"tabpre",      0,  PADPLUS,         Tabpre,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"tabpre",      0,  KEY_F12,         Tabpre,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
#else
 {(CHARTYPE *)"tabpre",      0,  KEY_F(12),       Tabpre,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
#endif
 {(CHARTYPE *)"tabfile",     7,  (-1),            Tabfile,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"tabline",     4,  (-1),            Tabline,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"tabkey",      4,  (-1),            Tabkey,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"tabs",        4,  (-1),            Tabs,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"tabsin",      5,  (-1),            Tabsin,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"tabsout",     5,  (-1),            Tabsout,          TRUE, TRUE, FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"tag",         3,  (-1),            Tag,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"targetsave", 10,  (-1),            Targetsave,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"text",        4,  (-1),            Text,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_NONE,TRUE, FALSE,TRUE ,CUA_DELETE_BLOCK ,THIGHLIGHT_RESET_FOCUS  ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"thighlight",  5,  (-1),            THighlight,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"timecheck",   9,  (-1),            Timecheck,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"toascii",     7,  (-1),            Toascii,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"tofeof",      6,  (-1),            Tofeof,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
#if defined(DOS) || defined(OS2) || defined(USE_XCURSES) || defined(WIN32)
 {(CHARTYPE *)"top",         3,  CTL_PGUP,        Top,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
#else
 {(CHARTYPE *)"top",         3,  (-1),            Top,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
#endif
 {(CHARTYPE *)"trailing",    8,  (-1),            Trailing,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"trunc",       5,  (-1),            Trunc,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"typeahead",   5,  (-1),            THETypeahead,     TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"undoing",     7,  (-1),            Undoing,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"up",          1,  (-1),            Up,               TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"uppercase",   3,  (-1),            Uppercase,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"verify",      1,  (-1),            Verify,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"width",       5,  (-1),            Width,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"word",        4,  (-1),            _Word,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"wrap",        2,  (-1),            Wrap,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"wordwrap",    5,  (-1),            Wordwrap,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"xterminal",   5,  (-1),            Xterminal,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"edit",        1,  (-1),            Xedit,            TRUE, FALSE,FALSE,TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"xedit",       1,  (-1),            Xedit,            TRUE, FALSE,FALSE,TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"kedit",       1,  (-1),            Xedit,            TRUE, FALSE,FALSE,TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"the",         3,  (-1),            Xedit,            TRUE, FALSE,FALSE,TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* comm5.c */
 {(CHARTYPE *)"untaa",       5,  (-1),            Untaa,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {(CHARTYPE *)"zone",        1,  (-1),            Zone,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,(CHARTYPE *)""},  /* commset2.c */
 {NULL,                      0,  (-1),            NULL,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_NONE         ,(CHARTYPE *)""}
};
