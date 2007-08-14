/***********************************************************************/
/* DEFAULT.C - Default settings and profile processing                 */
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

static char RCSid[] = "$Id: default.c,v 1.24 2002/08/13 09:11:47 mark Exp $";

#include <the.h>
#include <proto.h>

/*--------------------------- global data -----------------------------*/
/*#define DEBUG 1*/
 bool BIRTHDAYx=TRUE;
 bool BEEPx;
 bool CAPREXXOUTx;
 LINETYPE CAPREXXMAXx;
 bool CLEARSCREENx;
 int  CLEARERRORKEYx;
 bool CLOCKx;
 CHARTYPE CMDARROWSTABCMDx;
 CHARTYPE EOLx;
 bool HEXDISPLAYx;
 bool INSERTMODEx;
 bool LINEND_STATUSx; /* not really global, but used in command_line() */
 CHARTYPE LINEND_VALUEx; /* not really global, but used in command_line() */
 CHARTYPE NONDISPx; 
 CHARTYPE PREFIXx;/* not really global but used in set_screen_defaults() */
 bool REPROFILEx;
 CHARTYPE TABI_ONx;
 CHARTYPE TABI_Nx;
 CHARTYPE EQUIVCHARx;                         /* equivalence character */
 CHARTYPE EQUIVCHARstr[2];        /* equivalence character as a string */
 ROWTYPE STATUSLINEx='B';
 bool FILETABSx=FALSE;                 /* file tabs are off by default */
 bool TYPEAHEADx;
 bool scroll_cursor_stay;   /* determine how page scrolling is handled */
 bool MOUSEx;                                         /* mouse support */
 bool SLKx=FALSE;                            /* soft label key support */
 bool SBx=FALSE;                                  /* scrollbar support */
 bool UNTAAx=FALSE;         /* unsigned numerical targets are absolute */
 bool READONLYx=READONLY_OFF;                      /* SET READONLY OFF */
 bool PAGEWRAPx=TRUE;         /* wrap pgup/pgdn at extremities of file */
 int DEFSORTx=DIRSORT_NAME;                       /* sort on file attr */
 int DIRORDERx=DIRSORT_ASC;                          /* sort ascending */
 int popup_escape_key=-1;
 PARSER_DETAILS *first_parser=NULL,*last_parser=NULL;
 PARSER_MAPPING *first_parser_mapping=NULL,*last_parser_mapping=NULL;
 bool CTLCHARx=FALSE;
 CHARTYPE ctlchar_escape=0;
 COLOUR_ATTR ctlchar_attr[MAX_CTLCHARS];  /* colour attributes for ctlchar */
 CHARTYPE ctlchar_char[MAX_CTLCHARS];      /* escape chars for ctlchar */
 CHARTYPE INTERFACEx=INTERFACE_CLASSIC;       /* behavioural interface */
 int TARGETSAVEx=TARGET_ALL;
 int REGEXPx=0;           /* default regular expression syntax - EMACS */


/*--------------------------- regular expression syntaxes -------------*/
struct regexp_syntax _THE_FAR regexp_syntaxes[] =
{
   {"EMACS"                 ,RE_SYNTAX_EMACS                  },
   {"AWK"                   ,RE_SYNTAX_AWK                    },
   {"POSIX_AWK"             ,RE_SYNTAX_POSIX_AWK              },
   {"GREP"                  ,RE_SYNTAX_GREP                   },
   {"EGREP"                 ,RE_SYNTAX_EGREP                  },
   {"POSIX_EGREP"           ,RE_SYNTAX_POSIX_EGREP            },
   {"SED"                   ,RE_SYNTAX_SED                    },
   {"POSIX_BASIC"           ,RE_SYNTAX_POSIX_BASIC            },
   {"POSIX_MINIMAL_BASIC"   ,RE_SYNTAX_POSIX_MINIMAL_BASIC    },
   {"POSIX_EXTENDED"        ,RE_SYNTAX_POSIX_EXTENDED         },
   {"POSIX_MINIMAL_EXTENDED",RE_SYNTAX_POSIX_MINIMAL_EXTENDED },
   {NULL                    ,0                                }
};

/*--------------------------- default parsers -------------------------*/
typedef struct
{
   CHARTYPE *filename;
   CHARTYPE *name;
   CHARTYPE *contents;
} DEFAULT_PARSERS;

DEFAULT_PARSERS _THE_FAR default_parsers[] =
{
   { (CHARTYPE *)"*REXX.TLD",
     (CHARTYPE *)"REXX",
     (CHARTYPE *)"* REXX\n:case\nignore\n:option\nrexx\nfunction ( noblank\n:number\nrexx\n:identifier\n[a-zA-Z!?_@#$] [a-zA-Z0-9.!?_@#$]\n" \
         ":string\nsingle\ndouble\n:comment\npaired /* */ nest\nline -- any\n"                                           \
         ":label\ndelimiter : any\n:match\n( )\ndo,select end when,otherwise\n:header\nline #! column 1\n"               \
         ":keyword\naddress type 5\narg type 7\nby type 2\ncall type 5\ndigits type 2\ndo type 5\ndrop type 1\n"         \
         "else type 1\nend type 1\nengineering type 2\nexit type 1\nexpose type 2\nfor type 2\nforever type 2\n"         \
         "form type 2\nfuzz type 2\nif type 1\ninterpret type 1\niterate type 1\nleave type 1\nname type 2\nnop type 1\n"  \
         "novalue\nnumeric type 5\noff type 2\non type 2\noptions type 1\notherwise type 1\nparse type 5\nprocedure type 5\n"     \
         "pull type 7\npush type 1\nqueue type 1\nreturn type 1\nsay type 1\nscientific type 2\nselect type 1\n"         \
         "signal type 5\nsource type 2\nsyntax\nto type 2\nthen type 1\ntrace type 5\nuntil type 2\nupper type 2\nvalue type 2\n"  \
         "var type 2\nversion type 2\nwhen type 1\nwhile type 2\nwith type 2\nstem type 2\noutput type 2\ninput type 2\n"\
         "error type 2\nappend type 2\nreplace type 2\nnormal type 2\nstream type 2\ndefault type 1\n"                   \
         "command type 2 alt e\nsystem type 2 alt e\nos2environment type 2 alt e\nenvironment type 2 alt e\n"            \
         "cmd type 2 alt e\npath type 2 alt e\nregina type 2 alt e\nrexx type 2 alt e\n"                                 \
         ":function\n"                                                                                                   \
         "abbrev\ncenter\ncentre\nchangestr\ncompare\ncopies\ncountstr\ndatatype\ndelstr\ndelword\nbeep\ndirectory\n"            \
         "insert\nlastpos\nleft\nlength\noverlay\npos\nreverse\nright\nspace\nstrip\nsubstr\n"                           \
         "subword\ntranslate\nverify\nword\nwordindex\nwordlength\nwordpos\nwords\nxrange\n"                             \
         "abs\nformat\nmax\nmin\nsign\ntrunc\naddress\narg\ncondition\ndigits\nerrortext\nform\n"                        \
         "fuzz\nsourceline\ntrace\nb2x\nbitand\nbitor\nbitxor\nc2d\nc2x\nd2c\nd2x\nx2b\nx2c\nx2d\n"                      \
         "charin\ncharout\nchars\nlinein\nlineout\nlines\nqualify\nstream\ndate\nqueued\nrandom\n"                       \
         "symbol\ntime\nvalue\nrxfuncdrop\nrxfuncerrmsg\nrxfuncadd\nrxfuncquery\nrxqueue\n"                              \
         ":postcompare\nclass [-\\/\\+\\=\\*\\<\\>|%&,] alt w\n"
   },
   { (CHARTYPE *)"*C.TLD",
     (CHARTYPE *)"C",
     (CHARTYPE *)"* C\n:case\nrespect\n:option\npreprocessor #\n:number\nc\n:identifier\n[a-zA-Z_] [a-zA-Z0-9_]\n"       \
         ":string\nsingle backslash\ndouble backslash\n:comment\npaired /* */ nonest\nline // any\n"                     \
         ":label\ndelimiter : column 1\n:match\n( )\n{ }\n#ifdef,#if,#ifndef #endif #else,#elif,#elseif\n"               \
         ":keyword\n#define\n#elif\n#else\n#endif\n#error\n#if\n#ifdef\n#ifndef\n#include\n#line\n#pragma\n#undef\n"     \
         "auto\nbool\nbreak\ncase\nchar\nconst\ncontinue\ndefault\ndefined alt f\ndelete\ndo\ndouble\nelse\nenum\n"      \
         "extern\nfloat\nfor\ngoto\nif\nint\nlong\nnew\nregister\nreturn\nshort\nsigned\nsizeof\nstatic\nstruct\n"       \
         "switch\ntypedef\nunion\nunsigned\nvoid\nvolatile\nwhile\nclass\nprivate\npublic\n"                             \
         ":postcompare\nclass [-\\/\\+\\=\\*\\<\\>|%&!,] alt x\n"
   },
   { (CHARTYPE *)"*SH.TLD",
     (CHARTYPE *)"SH",
     (CHARTYPE *)"* SH\n:case\nrespect\n:number\nc\n"       \
         ":string\nsingle backslash\ndouble backslash\n:identifier\n[a-zA-Z_] [a-zA-Z0-9_]\n"                            \
         ":comment\nline # any\n:header\nline #! column 1\n"                                                                                        \
         ":keyword\nif\nfi\nelif\nfor\nuntil\ncase\nesac\nwhile\nthen\nelse\ntest alt 4\nshift alt 4\ndo\ndone\nin\n"          \
         "continue alt 4\nbreak alt 4\ncd alt 4\necho alt 4\neval alt 4\nexec alt 4\n" \
         "exit alt 4\nexport alt 4\ngetopts alt 4\nset alt 4\nunset alt 4\ntrap alt 4\n"
   },
   { (CHARTYPE *)"*TLD.TLD",
     (CHARTYPE *)"TLD",
     (CHARTYPE *)"* TLD\n:case\nignore\n"                                                                                \
         ":comment\nline * firstnonblank\n"                                                                                        \
         ":header\nline : column 1\n"
   },
   { (CHARTYPE *)"*HTML.TLD",
     (CHARTYPE *)"HTML",
     (CHARTYPE *)"* HTML\n" \
         ":case\nignore\n" \
         ":identifier\n[a-zA-Z] [a-zA-Z0-9]\n" \
         ":string\ndouble\n" \
         ":comment\npaired <! > nonest\n" \
         ":markup\ntag < >\nreference & ;\n"
   },
   { (CHARTYPE *)"*DIR.TLD",
     (CHARTYPE *)"DIR",
     (CHARTYPE *)"* DIR\n" \
         ":column\nexclude 1 38\n" \
         ":postcompare\ntext -> alt 1\n" \
         "class [mM]akefile.*$ alt 4\n" \
         "class .*\\.mak$ alt 4\n" \
         "class .*\\.exe$ alt 6\n" \
         "class .*\\.c$ alt 2\n" \
         "class .*\\.cpp$ alt 2\n" \
         "class .*\\.cc$ alt 2\n" \
         "class .*\\.h$ alt 2\n" \
         "class .*\\.the$ alt 1\n" \
         "class .*\\.rexx$ alt 3\n" \
         "class .*\\.rex$ alt 3\n" \
         "class .*\\.cmd$ alt 3\n" \
         "class .*\\.bak$ alt 8\n" \
         "class .*\\.zip$ alt 5\n" \
         "class .*\\.Z$ alt 5\n"   \
         "class .*\\.gz$ alt 5\n"  \
         "class .*\\.tgz$ alt 5\n"
   },
   {
     NULL, NULL
   }
};

PARSER_MAPPING _THE_FAR default_parser_mapping[] =
{

   /*
    * The following describes this table for those interested in
    * updating it manually.
    * Parameter 1 - Always NULL
    * Parameter 2 - Always NULL
    * Parameter 3 - Name of parser to associate mapping
    * Parameter 4 - Filemask - NULL if using "magic numbers"
    * Parameter 4 - Magic number string - NULL if using filemask
    * Parameter 5 - Length of "magic number" - 0 if using filemask
    * Parameter 6 - Always NULL
    */
   { NULL,NULL,(CHARTYPE*)"C",    (CHARTYPE*)"*.c"    , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"C",    (CHARTYPE*)"*.cc"   , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"C",    (CHARTYPE*)"*.cpp"  , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"C",    (CHARTYPE*)"*.h"    , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"C",    (CHARTYPE*)"*.hpp"  , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"REXX", (CHARTYPE*)"*.cmd"  , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"REXX", (CHARTYPE*)"*.rex"  , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"REXX", (CHARTYPE*)"*.rexx" , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"REXX", (CHARTYPE*)"*.the"  , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"REXX", (CHARTYPE*)".therc" , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"TLD",  (CHARTYPE*)"*.tld"  , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"HTML", (CHARTYPE*)"*.htm"  , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"HTML", (CHARTYPE*)"*.html" , NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"DIR",  (CHARTYPE*)"DIR.DIR", NULL  ,0, NULL },
   { NULL,NULL,(CHARTYPE*)"REXX", NULL    ,(CHARTYPE*)"rexx"  ,4, NULL },
   { NULL,NULL,(CHARTYPE*)"REXX", NULL    ,(CHARTYPE*)"regina",6, NULL },
   { NULL,NULL,(CHARTYPE*)"REXX", NULL    ,(CHARTYPE*)"rxx"   ,3, NULL },
   { NULL,NULL,(CHARTYPE*)"SH",   NULL    ,(CHARTYPE*)"sh"    ,2, NULL },
   { NULL,NULL,(CHARTYPE*)"SH",   NULL    ,(CHARTYPE*)"ksh"   ,3, NULL },
   { NULL,NULL,(CHARTYPE*)"SH",   NULL    ,(CHARTYPE*)"bash"  ,4, NULL },
   { NULL,NULL,(CHARTYPE*)"SH",   NULL    ,(CHARTYPE*)"zsh"   ,3, NULL },
   { NULL,NULL,NULL,              NULL    , NULL              ,0, NULL }
};

/***********************************************************************/
#ifdef HAVE_PROTO
void set_global_defaults(void)
#else
void set_global_defaults()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("default.c: set_global_defaults");
   /*
    * Set defaults for all environments first...
    */
   BEEPx = FALSE;
   CAPREXXOUTx = FALSE;
   CAPREXXMAXx = 1000L;
   INSERTMODEx = FALSE;
   CLEARERRORKEYx = -1;
   LINEND_VALUEx = '#';
   EQUIVCHARx = '=';
   EQUIVCHARstr[0] = '=';
   EQUIVCHARstr[1] = '\0';
   REPROFILEx = FALSE;
   TABI_ONx = FALSE;
   TABI_Nx = 8;
   UNTAAx = FALSE;
   READONLYx = READONLY_OFF;
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
   MOUSEx = TRUE;
#else
   MOUSEx = FALSE;
#endif
   if (display_length)          /* if display_length has been specified) */
      EOLx = EOLOUT_NONE;
   else
   {
#if defined(UNIX) || defined(AMIGA)
      EOLx = EOLOUT_LF;
#endif
#if defined(DOS) || defined(OS2) || defined(WIN32)
      EOLx = EOLOUT_CRLF;
#endif
   }
   /*
    * Set CTLCHAR defaults...
    */
   for (i=0;i<MAX_CTLCHARS;i++)
   {
      memset((CHARTYPE *)&ctlchar_attr[i],0,sizeof(COLOUR_ATTR));
      ctlchar_char[i] = 0;
   }
   /*
    * If STATUSLINE is OFF before we come here, leave it OFF.
    */
   if (STATUSLINEx != 'O')
      STATUSLINEx = 'B';
   TYPEAHEADx = FALSE;
   scroll_cursor_stay = TRUE;
   prefix_width = DEFAULT_PREFIX_WIDTH;
   /*
    * Set defaults for individual environments next...
    */
   set_global_look_defaults();
   set_global_feel_defaults();
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void set_global_look_defaults(void)
#else
void set_global_look_defaults()
#endif
/***********************************************************************/
{

   TRACE_FUNCTION("default.c: set_global_look_defaults");
   switch(compatible_look)
   {
      case COMPAT_THE:
         CLEARSCREENx = FALSE;
         CLOCKx = TRUE;
         HEXDISPLAYx = TRUE;
         PREFIXx = PREFIX_ON|PREFIX_LEFT;
         prefix_gap = 0;
         NONDISPx = '#';
         break;
      case COMPAT_KEDITW:
      case COMPAT_KEDIT:
         CLEARSCREENx = TRUE;
         CLOCKx = TRUE;
         HEXDISPLAYx = FALSE;
         NONDISPx = '#';
         PREFIXx = PREFIX_OFF;
         prefix_gap = 1;
         break;
      case COMPAT_XEDIT:
         CLEARSCREENx = TRUE;
         CLOCKx = FALSE;
         HEXDISPLAYx = FALSE;
         NONDISPx = '"';
         PREFIXx = PREFIX_ON|PREFIX_LEFT;
         prefix_gap = 1;
         break;
      case COMPAT_ISPF:
         CLEARSCREENx = TRUE;
         CLOCKx = FALSE;
         HEXDISPLAYx = FALSE;
         NONDISPx = '"';
         PREFIXx = PREFIX_ON|PREFIX_LEFT;
         prefix_gap = 1;
         break;
   }
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void set_global_feel_defaults(void)
#else
void set_global_feel_defaults()
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("default.c: set_global_feel_defaults");
   switch(compatible_feel)
   {
      case COMPAT_THE:
         CMDARROWSTABCMDx = FALSE;
         LINEND_STATUSx = FALSE;
         TARGETSAVEx = TARGET_STRING;
         break;
      case COMPAT_KEDIT:
      case COMPAT_KEDITW:
         CMDARROWSTABCMDx = TRUE;
         LINEND_STATUSx = FALSE;
         TARGETSAVEx = TARGET_ALL;
         break;
      case COMPAT_XEDIT:
         CMDARROWSTABCMDx = TRUE;
         LINEND_STATUSx = TRUE;
         TARGETSAVEx = TARGET_ALL;
         break;
      case COMPAT_ISPF:
         CMDARROWSTABCMDx = TRUE;
         LINEND_STATUSx = TRUE;
         TARGETSAVEx = TARGET_ALL;
         break;
   }
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void set_file_defaults(FILE_DETAILS *filep)
#else
void set_file_defaults(filep)
FILE_DETAILS *filep;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: set_file_defaults");
/*---------------------------------------------------------------------*/
/* Set defaults for all environments first...                          */
/*---------------------------------------------------------------------*/
 filep->autosave = 0;
 filep->autosave_alt = 0;
 filep->save_alt = 0;
 filep->tabsout_on = FALSE;
 filep->display_actual_filename = TRUE;
 filep->tabsout_num =  8;
 filep->eolout = EOLx;
 filep->timecheck = TRUE;
 filep->undoing = TRUE;
 filep->autocolour = TRUE;
 set_up_default_colours(filep,(COLOUR_ATTR *)NULL,ATTR_MAX);
 set_up_default_ecolours(filep);
 filep->trailing = TRAILING_ON;
/*---------------------------------------------------------------------*/
/* Set defaults for individual environments next...                    */
/*---------------------------------------------------------------------*/
 switch(compatible_feel)
   {
    case COMPAT_THE:
         filep->colouring = TRUE;
         filep->backup = BACKUP_KEEP;
         break;
    case COMPAT_XEDIT:
         filep->colouring = TRUE;
         filep->backup = BACKUP_OFF;
         break;
    case COMPAT_ISPF:
         filep->colouring = TRUE;
         filep->backup = BACKUP_OFF;
         break;
    case COMPAT_KEDITW:
         filep->colouring = TRUE;
         filep->backup = BACKUP_OFF;
         break;
    case COMPAT_KEDIT:
         filep->colouring = FALSE;
         filep->backup = BACKUP_OFF;
         break;
   }
 TRACE_RETURN();
 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void set_view_defaults(VIEW_DETAILS *viewp)
#else
void set_view_defaults(viewp)
VIEW_DETAILS *viewp;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register int i=0;
   short tabinc=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("default.c: set_view_defaults");
/*---------------------------------------------------------------------*/
/* Set defaults for all environments first...                          */
/*---------------------------------------------------------------------*/
   viewp->arbchar_status =        FALSE;
   viewp->arbchar_single =        '?';
   viewp->arbchar_multiple =      '$';
   viewp->arrow_on = TRUE;
   viewp->case_locate = CASE_IGNORE;
   viewp->case_change = CASE_RESPECT;
   viewp->case_sort = CASE_RESPECT;
   viewp->cmd_line = 'B';
   viewp->cmdline_col = 0;
   viewp->id_line = TRUE;
   viewp->in_ring = FALSE;
   viewp->current_row = 0;   /* this is set once we know the screen size */
   viewp->current_base = POSITION_MIDDLE;
   viewp->current_off = 0;
   viewp->display_high = 0;
   viewp->display_low = 0;
   viewp->hex = FALSE;
   viewp->highlight = HIGHLIGHT_NONE;
   viewp->highlight_high = 0;
   viewp->highlight_low = 0;
   viewp->inputmode = INPUTMODE_LINE;
   viewp->macro = FALSE;
   viewp->margin_left = 1;
   viewp->margin_right = 72;
   viewp->margin_indent = 0;
   viewp->margin_indent_offset_status = TRUE;
   viewp->msgmode_status = TRUE;
   viewp->position_status = TRUE;
   viewp->newline_aligned = TRUE;
   viewp->scale_base = POSITION_MIDDLE;
   viewp->scale_off = 1;
   viewp->scope_all = FALSE;
   viewp->shadow = TRUE;
   viewp->tab_base = POSITION_BOTTOM;
   viewp->tab_on = FALSE;
   viewp->numtabs = 32;
   viewp->tabsinc = 0;
   viewp->tofeof = TRUE;
   viewp->verify_start = 1;
   viewp->verify_col = 1;
   viewp->verify_end = max_line_length;
   viewp->hexshow_on = FALSE;
   viewp->hexshow_base = POSITION_TOP;
   viewp->hexshow_off = 7;
   viewp->word = 'N';
   viewp->wordwrap = FALSE;
   viewp->wrap = FALSE;
   viewp->zone_start = 1;
   viewp->synonym = TRUE;
   viewp->zone_end = max_line_length;
   viewp->autoscroll = (-1);  /* scroll half */
   viewp->boundmark = BOUNDMARK_OFF; /* normal default is BOUNDMARK_ZONE */
   viewp->syntax_headers = HEADER_ALL; /* ALL headers applied */
   viewp->thighlight_active = FALSE;
   initialise_target( &viewp->thighlight_target );
  
   if (viewp->cmd_line == 'O')
   {
      viewp->current_window =     WINDOW_FILEAREA;
      viewp->previous_window =    WINDOW_FILEAREA;
   }
   else
   {
      viewp->current_window =        WINDOW_COMMAND;
      viewp->previous_window =       WINDOW_FILEAREA;
   }
   /*
    * Set defaults for individual environments next...
    */
   switch(compatible_look)
   {
      case COMPAT_THE:
         viewp->number = TRUE;
         viewp->prefix = PREFIX_ON|PREFIX_LEFT;
         viewp->prefix_width = 6;
         viewp->prefix_gap = 0;
         viewp->scale_on = FALSE;
         viewp->tab_off = (-3);
         viewp->msgline_base = POSITION_TOP;
         viewp->msgline_off = 2;
         viewp->msgline_rows = 5;
         break;
      case COMPAT_XEDIT:
         viewp->number = FALSE;
         viewp->prefix = PREFIX_ON|PREFIX_LEFT;
         viewp->prefix_width = 6;
         viewp->prefix_gap = 1;
         viewp->scale_on = TRUE;
         viewp->tab_off = (-3);
         viewp->msgline_base = POSITION_TOP;
         viewp->msgline_off = 2;
         viewp->msgline_rows = 2;
         break;
      case COMPAT_ISPF:
         viewp->number = FALSE;
         viewp->prefix = PREFIX_ON|PREFIX_LEFT;
         viewp->prefix_width = 7;
         viewp->prefix_gap = 1;
         viewp->scale_on = TRUE;
         viewp->tab_off = (-3);
         viewp->msgline_base = POSITION_TOP;
         viewp->msgline_off = 2;
         viewp->msgline_rows = 2;
         viewp->cmd_line = 'T';
         break;
      case COMPAT_KEDITW:
      case COMPAT_KEDIT:
         viewp->number = FALSE;
         viewp->prefix = PREFIX_OFF;
         viewp->prefix_width = 6;
         viewp->prefix_gap = 1;
         viewp->scale_on = FALSE;
         viewp->tab_off = (-2);
         viewp->msgline_base = POSITION_TOP;
         viewp->msgline_off = 2;
         viewp->msgline_rows = 5;
         break;
   }
   switch(compatible_feel)
   {
      case COMPAT_THE:
         viewp->case_enter = CASE_MIXED;
         viewp->imp_macro = TRUE;
#if defined(USE_XCURSES)
         viewp->imp_os = FALSE;
#else
         viewp->imp_os = TRUE;
#endif
         viewp->linend_status = FALSE;
         viewp->linend_value = '#';
         viewp->stay = TRUE;
         viewp->tab_off = (-3);
         viewp->thighlight_on = TRUE;
         tabinc = 8;
         break;
      case COMPAT_XEDIT:
         viewp->case_enter = CASE_UPPER;
         viewp->imp_macro = TRUE;
#if defined(USE_XCURSES)
         viewp->imp_os = FALSE;
#else
         viewp->imp_os = TRUE;
#endif
         viewp->linend_status = TRUE;
         viewp->linend_value = '#';
         viewp->stay = FALSE;
         viewp->tab_off = (-3);
         viewp->thighlight_on = FALSE;
         tabinc = 3;
         break;
      case COMPAT_ISPF:
         viewp->case_enter = CASE_UPPER;
         viewp->imp_macro = TRUE;
#if defined(USE_XCURSES)
         viewp->imp_os = FALSE;
#else
         viewp->imp_os = TRUE;
#endif
         viewp->linend_status = TRUE;
         viewp->linend_value = '#';
         viewp->stay = FALSE;
         viewp->tab_off = (-3);
         viewp->thighlight_on = FALSE;
         tabinc = 3;
         break;
      case COMPAT_KEDITW:
      case COMPAT_KEDIT:
         viewp->case_enter = CASE_MIXED;
         viewp->imp_macro = FALSE;
         viewp->imp_os = FALSE;
         viewp->linend_status = FALSE;
         viewp->linend_value = '#';
         viewp->stay = TRUE;
         viewp->tab_off = (-2);
         viewp->thighlight_on = TRUE;
         tabinc = 8;
         break;
   }
   for (i=0;i<MAX_NUMTABS;i++)
      viewp->tabs[i] = 1 + (i*tabinc);
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short get_profile(CHARTYPE *prf_file,CHARTYPE *prf_arg)
#else
short get_profile(prf_file,prf_arg)
CHARTYPE *prf_file,*prf_arg;
#endif
/***********************************************************************/
{
   FILE *fp=NULL;
   short rc=RC_OK;
   bool save_in_macro=in_macro;
   short macrorc=0;

   TRACE_FUNCTION("default.c: get_profile");
   in_macro = TRUE;
   /*
    * If REXX is supported, process the macro as a REXX macro...          
    */
   if (rexx_support)
   {
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
      rc = execute_macro_file(prf_file,prf_arg,&macrorc,FALSE);
      if (rc != RC_OK)
      {
         display_error(54,(CHARTYPE *)"",FALSE);
         rc = RC_SYSTEM_ERROR;
      }
   }
   else
   {
      /*
       * ...otherwise, process the file as a non-REXX macro file...          
       */
      if ((fp = fopen((DEFCHAR *)prf_file,"r")) == NULL)
      {
         rc = RC_ACCESS_DENIED;
         display_error(8,prf_file,FALSE);
      }
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
      if (rc == RC_OK)
      {
         rc = execute_command_file(fp);
         fclose(fp);
      }
      if (rc == RC_SYSTEM_ERROR)
         display_error(53,(CHARTYPE *)"",FALSE);
      if (rc == RC_NOREXX_ERROR)
         display_error(52,(CHARTYPE *)"",FALSE);
   }
   in_macro = save_in_macro;
   TRACE_RETURN();
/* return(rc);*/
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short defaults_for_first_file(void)
#else
short defaults_for_first_file()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register int i=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("default.c: defaults_for_first_file");
   /*
    * Add to view linked list.
    */
   if ((CURRENT_VIEW = vll_add(vd_first,vd_current,sizeof(VIEW_DETAILS)))
                   == (VIEW_DETAILS *)NULL)
   {
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   if (vd_first == (VIEW_DETAILS *)NULL)
      vd_first = vd_last = CURRENT_VIEW;

   set_view_defaults(CURRENT_VIEW);
   /*
    * We now have CURRENT_VIEW and real screen sizes set, we can
    * calculate the CURLINE value.
    */
   CURRENT_VIEW->current_row = calculate_actual_row(CURRENT_VIEW->current_base,
                                                  CURRENT_VIEW->current_off,
                                                  CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE);

   CURRENT_VIEW->mark_type = M_NONE;
   CURRENT_VIEW->marked_line = FALSE;
   CURRENT_VIEW->marked_col = FALSE;
   CURRENT_VIEW->mark_start_line = 0;
   CURRENT_VIEW->mark_end_line = 0;
   CURRENT_VIEW->mark_start_col = 0;
   CURRENT_VIEW->mark_end_col = 0;
   CURRENT_VIEW->focus_line = 0L;
   CURRENT_VIEW->current_line = 0L;
   CURRENT_VIEW->current_column = 1;
   for (i=0;i<VIEW_WINDOWS;i++)
      CURRENT_VIEW->x[i] = CURRENT_VIEW->y[i] = 0;

   CURRENT_FILE = (FILE_DETAILS *)NULL;
   
   CURRENT_SCREEN.screen_view = CURRENT_VIEW;

   number_of_views++;
   if (CURRENT_VIEW->cmd_line == 'O')
   {
      CURRENT_VIEW->current_window =     WINDOW_FILEAREA;
      CURRENT_VIEW->previous_window =    WINDOW_FILEAREA;
   }
   else
   {
      CURRENT_VIEW->current_window =        WINDOW_COMMAND;
      CURRENT_VIEW->previous_window =       WINDOW_FILEAREA;
   }
   CURRENT_VIEW->preserved_view_details = (PRESERVED_VIEW_DETAILS *)NULL;
   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short defaults_for_other_files(VIEW_DETAILS *base_view)
#else
short defaults_for_other_files(base_view)
VIEW_DETAILS *base_view;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register int i=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("default.c: defaults_for_other_files");
   if ((CURRENT_VIEW = vll_add(vd_first,vd_current,sizeof(VIEW_DETAILS)))
                   == (VIEW_DETAILS *)NULL)
   {
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   if (CURRENT_VIEW->next == (VIEW_DETAILS *)NULL)
      vd_last = CURRENT_VIEW;
   if (vd_first == (VIEW_DETAILS *)NULL)
      vd_first = vd_last = CURRENT_VIEW;

   if (base_view)
   {
      CURRENT_VIEW->current_line =    base_view->current_line;
      CURRENT_VIEW->current_column =  base_view->current_column;
      CURRENT_VIEW->display_high =    base_view->display_high;
      CURRENT_VIEW->display_low =     base_view->display_low;
      CURRENT_VIEW->hex =             base_view->hex;
      CURRENT_VIEW->highlight =       base_view->highlight;
      CURRENT_VIEW->highlight_high =  base_view->highlight_high;
      CURRENT_VIEW->highlight_low =   base_view->highlight_low;
      CURRENT_VIEW->newline_aligned = base_view->newline_aligned;
      CURRENT_VIEW->scope_all =       base_view->scope_all;
      CURRENT_VIEW->shadow =          base_view->shadow;
      CURRENT_VIEW->stay =            base_view->stay;
      CURRENT_VIEW->hexshow_on =      base_view->hexshow_on;
      CURRENT_VIEW->hexshow_base =    base_view->hexshow_base;
      CURRENT_VIEW->hexshow_off =     base_view->hexshow_off;
      CURRENT_VIEW->verify_start =    base_view->verify_start;
      CURRENT_VIEW->verify_col =      base_view->verify_col;
      CURRENT_VIEW->verify_end =      base_view->verify_end;
      CURRENT_VIEW->zone_start =      base_view->zone_start;
      CURRENT_VIEW->zone_end =        base_view->zone_end;
      CURRENT_VIEW->autoscroll =      base_view->autoscroll;
      CURRENT_VIEW->boundmark =       base_view->boundmark;
      CURRENT_VIEW->syntax_headers =  base_view->syntax_headers;
    
      for (i=0;i<VIEW_WINDOWS;i++)
      {
         CURRENT_VIEW->y[i] = base_view->y[i];
         CURRENT_VIEW->x[i] = base_view->x[i];
      }

      CURRENT_VIEW->inputmode  =      base_view->inputmode;
      CURRENT_VIEW->focus_line =      base_view->focus_line;
      CURRENT_VIEW->mark_type =       base_view->mark_type;
      CURRENT_VIEW->marked_line =     base_view->marked_line;
      CURRENT_VIEW->marked_col =      base_view->marked_col;
      CURRENT_VIEW->mark_start_line = base_view->mark_start_line;
      CURRENT_VIEW->mark_end_line =   base_view->mark_end_line;
      CURRENT_VIEW->mark_start_col =  base_view->mark_start_col;
      CURRENT_VIEW->mark_end_col =    base_view->mark_end_col;
      CURRENT_VIEW->thighlight_on =   base_view->thighlight_on;
      CURRENT_VIEW->thighlight_active = FALSE;
      CURRENT_VIEW->arbchar_status =  base_view->arbchar_status;
      CURRENT_VIEW->arbchar_single =  base_view->arbchar_single;
      CURRENT_VIEW->arbchar_multiple= base_view->arbchar_multiple;
      CURRENT_VIEW->arrow_on =        base_view->arrow_on;
      CURRENT_VIEW->case_enter =      base_view->case_enter;
      CURRENT_VIEW->case_locate =     base_view->case_locate;
      CURRENT_VIEW->case_change =     base_view->case_change;
      CURRENT_VIEW->case_sort =       base_view->case_sort;
      CURRENT_VIEW->cmd_line =        base_view->cmd_line;
      CURRENT_VIEW->id_line =         base_view->id_line;
      CURRENT_VIEW->current_base =    base_view->current_base;
      CURRENT_VIEW->current_off =     base_view->current_off;
      CURRENT_VIEW->current_row =     base_view->current_row;
      CURRENT_VIEW->imp_macro =       base_view->imp_macro;
      CURRENT_VIEW->imp_os =          base_view->imp_os;
      CURRENT_VIEW->linend_status =   base_view->linend_status;
      CURRENT_VIEW->linend_value =    base_view->linend_value;
      CURRENT_VIEW->macro =           base_view->macro;
      CURRENT_VIEW->margin_left =     base_view->margin_left;
      CURRENT_VIEW->margin_right =    base_view->margin_right;
      CURRENT_VIEW->margin_indent =   base_view->margin_indent;
      CURRENT_VIEW->margin_indent_offset_status = base_view->margin_indent_offset_status;
      CURRENT_VIEW->msgmode_status =  TRUE;
      CURRENT_VIEW->position_status = base_view->position_status;
      CURRENT_VIEW->msgline_base =    base_view->msgline_base;
      CURRENT_VIEW->msgline_off =     base_view->msgline_off;
      CURRENT_VIEW->msgline_rows =    base_view->msgline_rows;
      CURRENT_VIEW->number =          base_view->number;
      CURRENT_VIEW->prefix =          base_view->prefix;
      CURRENT_VIEW->prefix_width =    base_view->prefix_width;
      CURRENT_VIEW->prefix_gap =      base_view->prefix_gap;
      CURRENT_VIEW->scale_base =      base_view->scale_base;
      CURRENT_VIEW->scale_off =       base_view->scale_off;
      CURRENT_VIEW->scale_on =        base_view->scale_on;
      CURRENT_VIEW->tab_base =        base_view->tab_base;
      CURRENT_VIEW->tab_off =         base_view->tab_off;
      CURRENT_VIEW->tab_on =          base_view->tab_on;
      CURRENT_VIEW->tofeof=           base_view->tofeof;
      CURRENT_VIEW->word =            base_view->word;
      CURRENT_VIEW->wordwrap =        base_view->wordwrap;
      CURRENT_VIEW->wrap =            base_view->wrap;
      CURRENT_VIEW->numtabs =         base_view->numtabs;
      CURRENT_VIEW->tabsinc =         base_view->tabsinc;
      memcpy(CURRENT_VIEW->tabs,base_view->tabs,MAX_NUMTABS*sizeof(LENGTHTYPE));
   }
   else
   {
      set_view_defaults(CURRENT_VIEW);
      CURRENT_VIEW->focus_line = 0L;
      CURRENT_VIEW->current_line = 0L;
      CURRENT_VIEW->current_column = 1;
      CURRENT_VIEW->mark_type = M_NONE;
      CURRENT_VIEW->marked_line = FALSE;
      CURRENT_VIEW->marked_col = FALSE;
      CURRENT_VIEW->mark_start_line = 0;
      CURRENT_VIEW->mark_end_line = 0;
      CURRENT_VIEW->mark_start_col = 0;
      CURRENT_VIEW->mark_end_col = 0;
      for (i=0;i<VIEW_WINDOWS;i++)
      {
         CURRENT_VIEW->y[i] = CURRENT_VIEW->x[i] = 0;
      }
      CURRENT_VIEW->arbchar_status =  PREVIOUS_VIEW->arbchar_status;
      CURRENT_VIEW->arbchar_single =  PREVIOUS_VIEW->arbchar_single;
      CURRENT_VIEW->arbchar_multiple= PREVIOUS_VIEW->arbchar_multiple;
      CURRENT_VIEW->arrow_on =        PREVIOUS_VIEW->arrow_on;
      CURRENT_VIEW->case_enter =      PREVIOUS_VIEW->case_enter;
      CURRENT_VIEW->case_locate =     PREVIOUS_VIEW->case_locate;
      CURRENT_VIEW->case_change =     PREVIOUS_VIEW->case_change;
      CURRENT_VIEW->case_sort =       PREVIOUS_VIEW->case_sort;
      CURRENT_VIEW->cmd_line =        PREVIOUS_VIEW->cmd_line;
      CURRENT_VIEW->id_line =         PREVIOUS_VIEW->id_line;
      CURRENT_VIEW->current_base =    PREVIOUS_VIEW->current_base;
      CURRENT_VIEW->current_off =     PREVIOUS_VIEW->current_off;
      CURRENT_VIEW->current_row =     PREVIOUS_VIEW->current_row;
      CURRENT_VIEW->imp_macro =       PREVIOUS_VIEW->imp_macro;
      CURRENT_VIEW->imp_os =          PREVIOUS_VIEW->imp_os;
      CURRENT_VIEW->inputmode  =      PREVIOUS_VIEW->inputmode;
      CURRENT_VIEW->highlight =       PREVIOUS_VIEW->highlight;
      CURRENT_VIEW->highlight_high =  PREVIOUS_VIEW->highlight_high;
      CURRENT_VIEW->highlight_low =   PREVIOUS_VIEW->highlight_low;
      CURRENT_VIEW->newline_aligned = PREVIOUS_VIEW->newline_aligned;
      CURRENT_VIEW->linend_status =   PREVIOUS_VIEW->linend_status;
      CURRENT_VIEW->linend_value =    PREVIOUS_VIEW->linend_value;
      CURRENT_VIEW->macro =           PREVIOUS_VIEW->macro;
      CURRENT_VIEW->margin_left =     PREVIOUS_VIEW->margin_left;
      CURRENT_VIEW->margin_right =    PREVIOUS_VIEW->margin_right;
      CURRENT_VIEW->margin_indent =   PREVIOUS_VIEW->margin_indent;
      CURRENT_VIEW->margin_indent_offset_status = PREVIOUS_VIEW->margin_indent_offset_status;
      CURRENT_VIEW->msgmode_status =  TRUE;
      CURRENT_VIEW->position_status = PREVIOUS_VIEW->position_status;
      CURRENT_VIEW->msgline_base =    PREVIOUS_VIEW->msgline_base;
      CURRENT_VIEW->msgline_off =     PREVIOUS_VIEW->msgline_off;
      CURRENT_VIEW->msgline_rows =    PREVIOUS_VIEW->msgline_rows;
      CURRENT_VIEW->number =          PREVIOUS_VIEW->number;
      CURRENT_VIEW->prefix =          PREVIOUS_VIEW->prefix;
      CURRENT_VIEW->prefix_width =    PREVIOUS_VIEW->prefix_width;
      CURRENT_VIEW->prefix_gap =      PREVIOUS_VIEW->prefix_gap;
      CURRENT_VIEW->scale_base =      PREVIOUS_VIEW->scale_base;
      CURRENT_VIEW->scale_off =       PREVIOUS_VIEW->scale_off;
      CURRENT_VIEW->scale_on =        PREVIOUS_VIEW->scale_on;
      CURRENT_VIEW->tab_base =        PREVIOUS_VIEW->tab_base;
      CURRENT_VIEW->tab_off =         PREVIOUS_VIEW->tab_off;
      CURRENT_VIEW->tab_on =          PREVIOUS_VIEW->tab_on;
      CURRENT_VIEW->tofeof =          PREVIOUS_VIEW->tofeof;
      CURRENT_VIEW->word =            PREVIOUS_VIEW->word;
      CURRENT_VIEW->wordwrap =        PREVIOUS_VIEW->wordwrap;
      CURRENT_VIEW->wrap =            PREVIOUS_VIEW->wrap;
      CURRENT_VIEW->numtabs =         PREVIOUS_VIEW->numtabs;
      CURRENT_VIEW->tabsinc =         PREVIOUS_VIEW->tabsinc;
      memcpy(CURRENT_VIEW->tabs,PREVIOUS_VIEW->tabs,MAX_NUMTABS*sizeof(LENGTHTYPE));
   }

   CURRENT_FILE = (FILE_DETAILS *)NULL;

   CURRENT_SCREEN.screen_view = CURRENT_VIEW;

   number_of_views++;
   if (CURRENT_VIEW->cmd_line == 'O')
   {
      CURRENT_VIEW->current_window =     WINDOW_FILEAREA;
      CURRENT_VIEW->previous_window =    WINDOW_FILEAREA;
   }
   else
   {
      CURRENT_VIEW->current_window =        WINDOW_COMMAND;
      CURRENT_VIEW->previous_window =       WINDOW_FILEAREA;
   }
   CURRENT_VIEW->preserved_view_details = (PRESERVED_VIEW_DETAILS *)NULL;
   initialise_target( &CURRENT_VIEW->thighlight_target );
   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short default_file_attributes(FILE_DETAILS *fd)
#else
short default_file_attributes(fd)
FILE_DETAILS *fd;
#endif
/***********************************************************************/
{
   RESERVED *curr=NULL;
   short rc=RC_OK;

   TRACE_FUNCTION("default.c: default_file_attributes");
  
   set_file_defaults(CURRENT_FILE);
  
   CURRENT_FILE->fname =           (CHARTYPE *)NULL;
   CURRENT_FILE->fpath =           (CHARTYPE *)NULL;
   CURRENT_FILE->autosave_fname =  (CHARTYPE *)NULL;
   CURRENT_FILE->file_views =      1;
   CURRENT_FILE->first_line =      (LINE *)NULL;
   CURRENT_FILE->last_line =       (LINE *)NULL;
   CURRENT_FILE->editv =           (LINE *)NULL;
   CURRENT_FILE->first_reserved  = (RESERVED *)NULL;
   CURRENT_FILE->fmode =           0;
   CURRENT_FILE->modtime =         0;
   CURRENT_FILE->pseudo_file =     PSEUDO_REAL;
   CURRENT_FILE->first_ppc = CURRENT_FILE->last_ppc = NULL;
   CURRENT_FILE->preserved_file_details = NULL;
   CURRENT_FILE->fp =              NULL;
   CURRENT_FILE->parser =          NULL;
   /*
    * Set defaults for the current file based on the settings for the
    * previous file. The defaults to copy are:
    * - colours, reserved lines.
    */
   if (fd != NULL)
   {
      CURRENT_FILE->trailing = fd->trailing;
      CURRENT_FILE->display_actual_filename = fd->display_actual_filename;
      memcpy(CURRENT_FILE->attr,fd->attr,ATTR_MAX*sizeof(COLOUR_ATTR));
      /*
       * If in XEDIT compatibility mode, don't copy reserved lines to the
       * new file.
       */
      if (compatible_feel != COMPAT_XEDIT)
      {
         curr = fd->first_reserved;
         while(curr!=NULL)
         {
            if (add_reserved_line(curr->spec,curr->line,curr->base,curr->off, curr->attr) == NULL)
            {
               rc = RC_OUT_OF_MEMORY;
               break;
            }
            curr = curr->next;
         }
      }
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void set_screen_defaults(void)
#else
void set_screen_defaults()
#endif
/***********************************************************************/
{
   register int i=0;
   register int j=0;
   ROWTYPE cmdline=0;
   CHARTYPE prefix=0;
   ROWTYPE start_row=0,number_rows_less=0,idline_rows=0,filetabs_row=0;
   short my_prefix_width=0,my_prefix_gap=0;
   bool my_arrow=TRUE;

   TRACE_FUNCTION( "default.c: set_screen_defaults" );
   /*
    * Set values that affect the placement of each screen depending on
    * the position of the status line...
    */
   switch( STATUSLINEx )
   {
      case 'B':
         start_row = 0;
         number_rows_less = 1;
         break;
      case 'T':
         start_row = 1;
         number_rows_less = 1;
         break;
      case 'O':
      case 'G':
         start_row = 0;
         number_rows_less = 0;
         break;
   }
   /*
    * If FILETABS is ON, then set its row to the first or second
    * line of the screen depending if STATUSLINE is on top or not.
    */
   if ( FILETABSx )
   {
      filetabs_row = start_row;
      start_row++;
      number_rows_less++;
   }
   /*
    * Determine size of overall screen dimensions...
    */
   if ( horizontal )
   {
      if ( screen_rows[0] == 0 )
         screen[0].screen_rows = (terminal_lines - number_rows_less) / display_screens;
      else
         screen[0].screen_rows = screen_rows[0];
      screen[0].screen_cols = terminal_cols;
      screen[0].screen_start_row = start_row;
      screen[0].screen_start_col = 0;
      if ( display_screens == 1 )
      {
         screen[1].screen_rows = 0;
         screen[1].screen_cols = 0;
         screen[1].screen_start_row = 0;
         screen[1].screen_start_col = 0;
      }
      else
      {
         screen[1].screen_rows = (terminal_lines - number_rows_less) - screen[0].screen_rows;
         screen[1].screen_cols = terminal_cols;
         screen[1].screen_start_row = screen[0].screen_rows + start_row;
         screen[1].screen_start_col = 0;
      }
   }
   else
   {
      screen[0].screen_rows = terminal_lines - number_rows_less;
      screen[0].screen_cols = (terminal_cols / display_screens)-((display_screens == 1) ? 0 : 1);
      screen[0].screen_start_row = start_row;
      screen[0].screen_start_col = 0;
      if (display_screens == 1)
      {
         screen[1].screen_rows = 0;
         screen[1].screen_cols = 0;
         screen[1].screen_start_row = 0;
         screen[1].screen_start_col = 0;
      }
      else
      {
         screen[1].screen_rows = terminal_lines - number_rows_less;
         screen[1].screen_cols = terminal_cols - screen[0].screen_cols - 2;
         screen[1].screen_start_row = start_row;
         screen[1].screen_start_col = screen[0].screen_cols + 2;
      }
   }
   /*
    * Reset window sizes to zero...
    */
   for ( i = 0; i < MAX_SCREENS; i++ )
   {
      for ( j = 0; j < VIEW_WINDOWS; j++ )
      {
         screen[i].start_row[j] = 0;
         screen[i].start_col[j] = 0;
         screen[i].rows[j] = 0;
         screen[i].cols[j] = 0;
      }
   }
   /*
    * Determine size of each window for each screen...
    */
   for ( i = 0; i < display_screens; i++ )
   {
      if ( screen[i].screen_view == (VIEW_DETAILS *)NULL )
      {
         cmdline = 'B';
         idline_rows = 1;
         prefix = PREFIXx;
         my_prefix_width = DEFAULT_PREFIX_WIDTH;
         my_prefix_gap = DEFAULT_PREFIX_GAP;
         my_arrow = TRUE;
      }
      else
      {
         cmdline = screen[i].screen_view->cmd_line;
         idline_rows = (screen[i].screen_view->id_line) ? 1 : 0;
         prefix = screen[i].screen_view->prefix;
         my_prefix_width = screen[i].screen_view->prefix_width;
         my_prefix_gap = screen[i].screen_view->prefix_gap;
         my_arrow = screen[i].screen_view->arrow_on;
      }
      switch( cmdline )
      {
         case 'T':                                   /* command line on top */
            screen[i].start_row[WINDOW_FILEAREA] = screen[i].screen_start_row + 1 + idline_rows;
            screen[i].rows[WINDOW_FILEAREA] = screen[i].screen_rows - 1 - idline_rows;
            screen[i].start_row[WINDOW_COMMAND] = screen[i].screen_start_row + idline_rows;
            screen[i].rows[WINDOW_COMMAND] = 1;
            screen[i].cols[WINDOW_COMMAND] = screen[i].screen_cols-((my_arrow)?my_prefix_width:0);
            screen[i].start_row[WINDOW_ARROW] = screen[i].start_row[WINDOW_COMMAND];
            screen[i].rows[WINDOW_ARROW] = ((my_arrow)?1:0);
            screen[i].cols[WINDOW_ARROW] = ((my_arrow)?my_prefix_width:0);
            break;
         case 'B':                                /* command line on bottom */
            screen[i].start_row[WINDOW_FILEAREA] = screen[i].screen_start_row + idline_rows;
            screen[i].rows[WINDOW_FILEAREA] = screen[i].screen_rows - 1 - idline_rows;
            screen[i].start_row[WINDOW_COMMAND] = screen[i].screen_start_row + screen[i].screen_rows - 1;
            screen[i].rows[WINDOW_COMMAND] = 1;
            screen[i].cols[WINDOW_COMMAND] = screen[i].screen_cols-((my_arrow)?my_prefix_width:0);
            screen[i].start_row[WINDOW_ARROW] = screen[i].start_row[WINDOW_COMMAND];
            screen[i].rows[WINDOW_ARROW] = ((my_arrow)?1:0);
            screen[i].cols[WINDOW_ARROW] = ((my_arrow)?my_prefix_width:0);
            break;
         case 'O':                                      /* command line off */
            screen[i].start_row[WINDOW_FILEAREA] = screen[i].screen_start_row + idline_rows;
            screen[i].rows[WINDOW_FILEAREA] = screen[i].screen_rows - idline_rows;
            screen[i].start_row[WINDOW_COMMAND] = 0;
            screen[i].rows[WINDOW_COMMAND] = 0;
            screen[i].cols[WINDOW_COMMAND] = 0;
            screen[i].start_row[WINDOW_ARROW] = 0;
            screen[i].rows[WINDOW_ARROW] = 0;
            screen[i].cols[WINDOW_ARROW] = 0;
            break;
      }
      screen[i].start_col[WINDOW_ARROW] = screen[i].screen_start_col;
      screen[i].start_col[WINDOW_COMMAND] = screen[i].screen_start_col+((my_arrow)?my_prefix_width:0);
      if ( idline_rows == 1 )
      {
         screen[i].start_row[WINDOW_IDLINE] = screen[i].screen_start_row;
         screen[i].start_col[WINDOW_IDLINE] = screen[i].screen_start_col;
         screen[i].rows[WINDOW_IDLINE] = 1;
         screen[i].cols[WINDOW_IDLINE] = screen[i].screen_cols;
      }
      if ( prefix )
      {
         screen[i].start_row[WINDOW_PREFIX] = screen[i].start_row[WINDOW_FILEAREA];
         screen[i].start_row[WINDOW_GAP] = screen[i].start_row[WINDOW_FILEAREA];
         screen[i].rows[WINDOW_PREFIX] = screen[i].rows[WINDOW_FILEAREA];
         screen[i].rows[WINDOW_GAP] = screen[i].rows[WINDOW_FILEAREA];
         screen[i].cols[WINDOW_PREFIX] = my_prefix_width-my_prefix_gap;
         screen[i].cols[WINDOW_GAP] = my_prefix_gap;
         screen[i].cols[WINDOW_FILEAREA] = screen[i].screen_cols-my_prefix_width;
         if ( (prefix&PREFIX_LOCATION_MASK) == PREFIX_LEFT )
         {
            screen[i].start_col[WINDOW_PREFIX] = screen[i].screen_start_col;
            screen[i].start_col[WINDOW_GAP] = screen[i].screen_start_col+(my_prefix_width-my_prefix_gap);
            screen[i].start_col[WINDOW_FILEAREA] = screen[i].screen_start_col+my_prefix_width;
         }
         else
         {
            screen[i].start_col[WINDOW_FILEAREA] = screen[i].screen_start_col;
            screen[i].start_col[WINDOW_PREFIX] = ((screen[i].screen_start_col+screen[i].screen_cols+my_prefix_gap)-my_prefix_width);
            screen[i].start_col[WINDOW_GAP] = (screen[i].screen_start_col+screen[i].screen_cols)-my_prefix_width;
         }
      }
      else
      {
         screen[i].cols[WINDOW_FILEAREA] = screen[i].screen_cols;
         screen[i].start_col[WINDOW_FILEAREA] = screen[i].screen_start_col;
         screen[i].rows[WINDOW_PREFIX] = 0;
         screen[i].cols[WINDOW_PREFIX] = 0;
         screen[i].rows[WINDOW_GAP] = 0;
         screen[i].cols[WINDOW_GAP] = 0;
      }
   }
   /*
    * We now have the size of each screen, so we can allocate the display
    * line arrays.
    */
   if ( screen[0].sl != NULL )
   {
      (*the_free)( screen[0].sl );
      screen[0].sl = NULL;
   }
   if ( screen[1].sl != NULL )
   {
      (*the_free)( screen[1].sl );
      screen[1].sl = NULL;
   }
   if ( ( screen[0].sl = (SHOW_LINE *)(*the_malloc)( screen[0].rows[WINDOW_FILEAREA]*sizeof(SHOW_LINE) ) ) == NULL )
   {
      cleanup();
      display_error( 30, (CHARTYPE *)"", FALSE );
      exit(1);
   }
   if ( display_screens > 1 )
   {
      if ( ( screen[1].sl = (SHOW_LINE *)(*the_malloc)( screen[1].rows[WINDOW_FILEAREA]*sizeof(SHOW_LINE) ) ) == NULL )
      {
         cleanup();
         display_error( 30, (CHARTYPE *)"", FALSE );
         exit(1);
      }
   }
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void set_defaults(void)
#else
void set_defaults()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: set_defaults");
/*---------------------------------------------------------------------*/
/* Set up screen default sizes.                                        */
/*---------------------------------------------------------------------*/
 set_screen_defaults();
/*---------------------------------------------------------------------*/
/* Allocate memory to cmd_rec and set it to blanks.                    */
/*---------------------------------------------------------------------*/
 if (cmd_rec != NULL)
    (*the_free)(cmd_rec);
 if ((cmd_rec = (CHARTYPE *)(*the_malloc)((COLS+1)*sizeof(CHARTYPE))) == NULL)
   {
    cleanup();
    display_error(30,(CHARTYPE *)"",FALSE);
    exit(1);
   }
 memset(cmd_rec,' ',max_line_length);
 cmd_rec_len = 0;

 TRACE_RETURN();
 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short set_THE_key_defaults(int prey,int prex)
#else
short set_THE_key_defaults(prey,prex)
int prey,prex;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: set_THE_key_defaults");
/*---------------------------------------------------------------------*/
/* This function is for resetting all default values for THE           */
/* compatibility mode.                                                 */
/*---------------------------------------------------------------------*/
 Define((CHARTYPE *)"F2");
 Define((CHARTYPE *)"F3");
 Define((CHARTYPE *)"F4");
 Define((CHARTYPE *)"F6");
 Define((CHARTYPE *)"F7");
 Define((CHARTYPE *)"F8");
 Define((CHARTYPE *)"F9");
 Define((CHARTYPE *)"F11");
 Define((CHARTYPE *)"CURD");
 Define((CHARTYPE *)"CURL");
 Define((CHARTYPE *)"CURR");
 Define((CHARTYPE *)"CURU");
#if defined(UNIX) && !defined(USE_EXTCURSES) && !defined(USE_XCURSES)
 Define((CHARTYPE *)"C-M");
 Define((CHARTYPE *)"F16");
# if defined(KEY_TAB)
 Define((CHARTYPE *)"KEY_TAB");
# endif
# if defined(KEY_BTAB)
 Define((CHARTYPE *)"S-TAB");
# endif
 Define((CHARTYPE *)"C-I");
#else
 Define((CHARTYPE *)"TAB");
 Define((CHARTYPE *)"S-TAB");
 Define((CHARTYPE *)"ENTER");
#endif
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short set_XEDIT_key_defaults(int prey,int prex)
#else
short set_XEDIT_key_defaults(prey,prex)
int prey,prex;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: set_XEDIT_key_defaults");
/*---------------------------------------------------------------------*/
/* This function is for resetting all default values for XEDIT         */
/* compatibility mode.                                                 */
/*---------------------------------------------------------------------*/
 Define((CHARTYPE *)"F2 sos lineadd");
 Define((CHARTYPE *)"F3 quit");
 Define((CHARTYPE *)"F4 sos tabf");
 Define((CHARTYPE *)"F6 ?");
 Define((CHARTYPE *)"F7 backward");
 Define((CHARTYPE *)"F8 forward");
 Define((CHARTYPE *)"F9 =");
 Define((CHARTYPE *)"F11 spltjoin");
 Define((CHARTYPE *)"CURD cursor screen down");
 Define((CHARTYPE *)"CURL cursor screen left");
 Define((CHARTYPE *)"CURR cursor screen right");
 Define((CHARTYPE *)"CURU cursor screen up");
#if defined(UNIX) && !defined(USE_EXTCURSES) && !defined(USE_XCURSES)
 Define((CHARTYPE *)"C-M sos doprefix execute");
# if defined(KEY_BTAB)
 Define((CHARTYPE *)"S-TAB sos tabfieldb");
# endif
# if defined(KEY_TAB)
 Define((CHARTYPE *)"KEY_TAB sos tabfieldf");
# endif
 Define((CHARTYPE *)"C-I sos tabfieldf");
#else
 Define((CHARTYPE *)"ENTER sos doprefix execute");
 Define((CHARTYPE *)"S-TAB sos tabfieldb");
 Define((CHARTYPE *)"TAB sos tabfieldf");
#endif
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short set_ISPF_key_defaults(int prey,int prex)
#else
short set_ISPF_key_defaults(prey,prex)
int prey,prex;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: set_XEDIT_key_defaults");
/*---------------------------------------------------------------------*/
/* This function is for resetting all default values for XEDIT         */
/* compatibility mode.                                                 */
/*---------------------------------------------------------------------*/
 Define((CHARTYPE *)"F2 sos lineadd");
 Define((CHARTYPE *)"F3 quit");
 Define((CHARTYPE *)"F4 sos tabf");
 Define((CHARTYPE *)"F6 ?");
 Define((CHARTYPE *)"F7 up");
 Define((CHARTYPE *)"F8 down");
 Define((CHARTYPE *)"F9 =");
 Define((CHARTYPE *)"F11 spltjoin");
 Define((CHARTYPE *)"CURD cursor screen down");
 Define((CHARTYPE *)"CURL cursor screen left");
 Define((CHARTYPE *)"CURR cursor screen right");
 Define((CHARTYPE *)"CURU cursor screen up");
#if defined(UNIX) && !defined(USE_EXTCURSES) && !defined(USE_XCURSES)
 Define((CHARTYPE *)"C-M sos doprefix execute");
# if defined(KEY_BTAB)
 Define((CHARTYPE *)"S-TAB sos tabfieldb");
# endif
# if defined(KEY_TAB)
 Define((CHARTYPE *)"KEY_TAB sos tabfieldf");
# endif
 Define((CHARTYPE *)"C-I sos tabfieldf");
#else
 Define((CHARTYPE *)"ENTER sos doprefix execute");
 Define((CHARTYPE *)"S-TAB sos tabfieldb");
 Define((CHARTYPE *)"TAB sos tabfieldf");
#endif
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short set_KEDIT_key_defaults(int prey,int prex)
#else
short set_KEDIT_key_defaults(prey,prex)
int prey,prex;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: set_KEDIT_key_defaults");
/*---------------------------------------------------------------------*/
/* This function is for resetting all default values for KEDIT         */
/* compatibility mode.                                                 */
/*---------------------------------------------------------------------*/
 Define((CHARTYPE *)"F2 sos lineadd");
 Define((CHARTYPE *)"F3 quit");
 Define((CHARTYPE *)"F4 sos tab");
 Define((CHARTYPE *)"F5 sos makecurr");
 Define((CHARTYPE *)"F6 ?");
 Define((CHARTYPE *)"F7 ");
 Define((CHARTYPE *)"F8 dup");
 Define((CHARTYPE *)"F9 =");
 Define((CHARTYPE *)"F11 spltjoin");
 Define((CHARTYPE *)"F12 cursor home");
 Define((CHARTYPE *)"CURD");
 Define((CHARTYPE *)"CURL cursor kedit left");
 Define((CHARTYPE *)"CURR cursor kedit right");
 Define((CHARTYPE *)"CURU");
#if defined(UNIX) && !defined(USE_EXTCURSES) && !defined(USE_XCURSES)
 Define((CHARTYPE *)"C-M");
# if defined(KEY_TAB)
 Define((CHARTYPE *)"KEY_TAB");
# endif
# if defined(KEY_BTAB)
 Define((CHARTYPE *)"S-TAB");
# endif
 Define((CHARTYPE *)"C-I");
#else
 Define((CHARTYPE *)"ENTER");
 Define((CHARTYPE *)"TAB");
 Define((CHARTYPE *)"S-TAB");
#endif

 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short construct_default_parsers(void)
#else
short construct_default_parsers()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
   PARSER_DETAILS *curr;
   int i;
   CHARTYPE tmp[20];
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("default.c: construct_default_parsers");
/*---------------------------------------------------------------------*/
   for (i=0;;i++)
   {
      if (default_parsers[i].contents == NULL)
         break;
      rc = construct_parser(default_parsers[i].contents,
                            strlen((DEFCHAR *)default_parsers[i].contents),
                            &curr,
                            default_parsers[i].name,
                            default_parsers[i].filename);
      if (rc != RC_OK)
         break;
      if (rexx_support)
      {
         /*
          * If we have a Rexx interpreter, register an implied extract
          * function for the number of parsers we now have.
          */
         sprintf((DEFCHAR *)tmp,"parser.%d",i+1);
         MyRexxRegisterFunctionExe(tmp);
      }
   }

   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short destroy_all_parsers(void)
#else
short destroy_all_parsers()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 PARSER_DETAILS *curr=first_parser;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: destroy_all_parsers");
 while( curr->next != NULL )
 {
    destroy_parser(curr);
    curr = curr->next;
 }
 first_parser = last_parser = parserll_free(first_parser);
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short construct_default_parser_mapping(void)
#else
short construct_default_parser_mapping()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 PARSER_MAPPING *curr;
 PARSER_DETAILS *currp;
 int i;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: construct_default_parser_mapping");
/*---------------------------------------------------------------------*/
 for (i=0;;i++)
 {
    if (default_parser_mapping[i].parser_name == NULL)
       break;
    last_parser_mapping = curr = mappingll_add(first_parser_mapping,last_parser_mapping,sizeof(PARSER_MAPPING));
    if (first_parser_mapping == NULL)
       first_parser_mapping = curr;
    if (default_parser_mapping[i].filemask)
    {
       curr->filemask = (CHARTYPE *)(*the_malloc)(1+strlen((DEFCHAR *)default_parser_mapping[i].filemask)*sizeof(CHARTYPE));
       if (curr->filemask == NULL)
       {
          display_error(30,(CHARTYPE *)"",FALSE);
          TRACE_RETURN();
          return(RC_OUT_OF_MEMORY);
       }
       strcpy((DEFCHAR *)curr->filemask,(DEFCHAR *)default_parser_mapping[i].filemask);
    }
    if (default_parser_mapping[i].magic_number)
    {
       curr->magic_number = (CHARTYPE *)(*the_malloc)((1+default_parser_mapping[i].magic_number_length)*sizeof(CHARTYPE));
       if (curr->magic_number == NULL)
       {
          display_error(30,(CHARTYPE *)"",FALSE);
          TRACE_RETURN();
          return(RC_OUT_OF_MEMORY);
       }
       strcpy((DEFCHAR *)curr->magic_number,(DEFCHAR *)default_parser_mapping[i].magic_number);
       curr->magic_number_length = default_parser_mapping[i].magic_number_length;
    }
    curr->parser = NULL;
    for(currp=first_parser;currp!=NULL;currp=currp->next)
    {
       if (my_stricmp((DEFCHAR *)currp->parser_name,(DEFCHAR *)default_parser_mapping[i].parser_name) == 0)
       {
          curr->parser = currp;
          break;
       }
    }
   if (rexx_support)
   {
      /*
       * If we have a Rexx interpreter, register an implied extract
       * function for the number of parsers we now have.
       */
      CHARTYPE tmp[20];
      sprintf((DEFCHAR *)tmp,"autocolor.%d",i+1);
      MyRexxRegisterFunctionExe(tmp);
      sprintf((DEFCHAR *)tmp,"autocolour.%d",i+1);
      MyRexxRegisterFunctionExe(tmp);
   }
 }
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *find_default_parser(CHARTYPE *name,CHARTYPE *tldname)
#else
CHARTYPE *find_default_parser(name,tldname)
CHARTYPE *name;
CHARTYPE *tldname;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 int i;
 CHARTYPE *contents=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("default.c: find_default_parser");
/*---------------------------------------------------------------------*/
 tldname = NULL;
 for (i=0;;i++)
 {
    if (default_parsers[i].filename == NULL)
       break;
    if (my_stricmp((DEFCHAR *)default_parsers[i].filename,(DEFCHAR *)name) == 0)
    {
       contents = default_parsers[i].contents;
       tldname = default_parsers[i].filename;
       break;
    }
 }
 TRACE_RETURN();
 return(contents);
}
