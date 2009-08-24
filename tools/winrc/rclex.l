%{ /* -*- C -*- */
/*************************************************************************

    rclex.l	1.32
    lex parser for resource compiler

    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "buffer.h"
#include "rcyacc.h"
#include "ifyacc.h"
#include "rc.h"

#define YY_FATAL_ERROR(msg) my_fatal_error( msg )
static void my_fatal_error( yyconst char msg[] );

int hex2dec(char *hex);
int zzparse(void);
extern void exitfunc(void);

/* Preprocessor support: includes */
#define MAX_INCLUDE_DEPTH 32

struct include_item
{
  YY_BUFFER_STATE buff;
  char *filename;
  int line;
  int column;
  int is_define;
  void *ext;
} include_stack[MAX_INCLUDE_DEPTH], *curr_include = include_stack;

int include_depth = 0;
void include_file(char *filename, int lookHere);
void include_string(char *string);
int next_line();
int add_lines(char *);
void set_position(int first_line, int last_line, int first_column, int last_column);

#define YY_USER_ACTION set_position(current_line(), current_line(), \
				    curr_include->column, curr_include->column + strlen(yytext)); \
		       curr_include->column += strlen(yytext);

/* Preprocessor support: defines */
#include "define.h"
DEFINESTRUCT *defineNew;
char *defineValue;

/* Preprocessor support: if/ifdef/ifndef/else/elif/endif */
#define MAX_IF_STACK_DEPTH 64
int ifStack[MAX_IF_STACK_DEPTH];
int ifWasTrue[MAX_IF_STACK_DEPTH];
int ifStackDepth = -1;
int ifYes = 1; /* Does the latest if say it's okay to go on? */
int more_if_stack(void);
int less_if_stack(void);
int reset_if_yes(void);
int ifResult;
int inIf = 0;

/* C/C++ typedef support */
char *last_typedef_keyword = NULL;

/* State support */
#define MAX_STATE_STACK_DEPTH	64
int stateStack[MAX_STATE_STACK_DEPTH];
int stateStackIndex = 0;				/* Indicates next position to push */

static void push_state(int new_state);
static int pop_state(void);
static void duplicate_top_of_stack(void);

#define PSTATE(p,q) (ifYes ? p : q)
#define PBEGINX(p,q) BEGIN(PSTATE(p,q))
#define PBEGIN(p) PBEGINX(p, PREPROCESSOREXCLUDE)

#define FSTATE(p,q) (inIf ? q : p)
#define FBEGINX(p,q) BEGIN(FSTATE(p,q))
#define FBEGIN(p) FBEGINX(p, KEYWORD)

#define IPSTATE(p,pp,f) FSTATE(PSTATE(p,pp),f)
#define IPBEGINX(p,pp,f) BEGIN(IPSTATE(p,pp,f))
#define IPSTANDARDSTATE(p) IPSTATE(p, PREPROCESSOREXCLUDE, KEYWORD)
#define IPBEGIN(p) IPBEGINX(p, PREPROCESSOREXCLUDE, KEYWORD)
%}

%x IGNORECFILE
%x IGNORETILBRACE
%x IGNORETILBRACEORSEMI
%x TYPEDEFMODE
%x COMMENT
%x PREPROCESSOR
%x POSTPREPROCESSOR
%x PREPROCESSOREXCLUDE
%x INCLUDE
%x DEFINE
%x DEFINEMACRO
%x DEFINEVAL
%x UNDEFINE
%x IFDEF
%x IFNDEF
%x KEYWORD
%x KWDEF
%x IFSTART
%x TYPEDEFDISPATCH
%x STRUCTDISPATCH
%x DOERROR
%s HEX

HORZWHITESPACE[ \t]
WHITESPACE [ \t\r\x1a\n]
TEXT [^ \t\r\x1a\n]
NEWLINE (\r\n)|[\x1a\n\r]
ANY [^\x1a\n\r]
NUMBER [0-9]
LETTER [A-Za-z_]
HEXDIGIT [0-9A-Fa-f]
BRACE [{}]
ANYBUTBRACE [^ \t\r\x1a\n]
BRACEORSEMI [{};]
ANYBUTBRACEORSEMI [^{}; \t\r\x1a\n]
SEMICOLON [;]
ANYBUTSEMICOLON [^; \t\r\x1a\n]

%%

  /* Comments: Discard both C-style and C++-style comments */

<*>\/\* { push_state(COMMENT); }
<COMMENT>{ANY}*\*\/ BEGIN(pop_state());
<COMMENT>{ANY}
<COMMENT>{NEWLINE} next_line();

<KEYWORD,KWDEF,INITIAL,PREPROCESSOREXCLUDE>;{ANY}*{NEWLINE} yyless(yyleng - 1);

<*>\/\/{ANY}*{NEWLINE} yyless(yyleng - 1);

  /* Preprocessor */
<*>^[ \t]*#[ \t]* {
        if (YYSTATE != COMMENT)
	    push_state(PREPROCESSOR);	/* preprocessor excursion */
  }

<PREPROCESSOR>include[ \t]* {
    PBEGINX(INCLUDE,POSTPREPROCESSOR);
  }

<PREPROCESSOR>define[ \t]* {
    PBEGINX(DEFINE,POSTPREPROCESSOR);
  }

<PREPROCESSOR>undef[ \t]* {
    PBEGINX(UNDEFINE,POSTPREPROCESSOR);
  }

<PREPROCESSOR>ifdef[ \t]* {
    BEGIN(IFDEF);
  }

<PREPROCESSOR>ifndef[ \t]* {
    BEGIN(IFNDEF);
  }

<PREPROCESSOR>endif[ \t]* {
    pop_state();			/* discard state saved when '#' found */
    LOG(("endif"));
    less_if_stack();
    reset_if_yes();
    BEGIN(POSTPREPROCESSOR);
  }

<PREPROCESSOR>else[ \t]* {
    pop_state();			/* discard state saved when '#' found */
    ifStackDepth--;
    reset_if_yes();
    ifStackDepth++;
    if (!ifYes)
      {
	reset_if_yes();
	LOG(("else: ignored"));
	duplicate_top_of_stack();
	BEGIN(POSTPREPROCESSOR);
      }
    else
      {
	ifStack[ifStackDepth] = !ifWasTrue[ifStackDepth];
	LOG(("else: %d", ifStack[ifStackDepth]));
	ifWasTrue[ifStackDepth] = 1;
	reset_if_yes();
	if (ifYes)
	{
	  duplicate_top_of_stack();
	  BEGIN(POSTPREPROCESSOR);
	}
	else
	  BEGIN(PREPROCESSOREXCLUDE);
      }
  }

<PREPROCESSOR>if/[ \t]* {
    if (!ifYes)
      {
	LOG(("if: ignored"));
	duplicate_top_of_stack();
	BEGIN(POSTPREPROCESSOR);
	ifStack[more_if_stack()] = 0;
      }
    else
      {
	LOG(("if: ?"));
	ifResult = -17;
	BEGIN(IFSTART);
	zzparse();
	LOG(("if: %d", ifResult));
	ifStack[more_if_stack()] = ifResult;
	ifWasTrue[ifStackDepth] = ifResult;
	reset_if_yes();
	if (ifYes)
	{
	  duplicate_top_of_stack();
	  BEGIN(pop_state());
	}
	else
	  BEGIN(PREPROCESSOREXCLUDE);
      }
  }

<PREPROCESSOR>elif/[ \t]* {
    pop_state();		/* discard state saved when '#' found */
    ifStackDepth--;
    reset_if_yes();
    ifStackDepth++;
    if (!ifYes)
      {
  	reset_if_yes();
	LOG(("elif: ignored"));
	duplicate_top_of_stack();
	BEGIN(POSTPREPROCESSOR);
      }
    else
      {
	reset_if_yes();
	if (!ifWasTrue[ifStackDepth])
	  {
	    LOG(("elif: ?"));
	    ifResult = -17;
	    BEGIN(IFSTART);
	    zzparse();
	    ifStack[ifStackDepth] = ifResult;
	    ifWasTrue[ifStackDepth] = ifResult;
	    LOG(("elif: %d", ifResult));
	    reset_if_yes();
	    if (ifYes)
	    {
	      duplicate_top_of_stack();
	      BEGIN(pop_state());
	    }
	    else
	      BEGIN(PREPROCESSOREXCLUDE);
	  }
	else
	  {
	    LOG(("elif: was true"));
	    ifStack[ifStackDepth] = 0;
	    reset_if_yes();
	    BEGIN(PREPROCESSOREXCLUDE);
	  }
      }
  }

<PREPROCESSOR>pragma[ \t]*	{
				    LOG(("#pragma directive ignored")); 
				    BEGIN(POSTPREPROCESSOR);
				}

<PREPROCESSOR>error[ \t]* PBEGINX(DOERROR,POSTPREPROCESSOR);

<PREPROCESSOR>{TEXT}*	{
			    CERROR(("unknown preprocessor directive %s", 
				    yytext)); 
			    BEGIN(POSTPREPROCESSOR);
			}

<POSTPREPROCESSOR>{ANY}*\\{NEWLINE} next_line();

<POSTPREPROCESSOR>{ANY}*{NEWLINE} {
				      next_line(); 
				      BEGIN(pop_state()); /* end excursion */
				  }

<PREPROCESSOREXCLUDE>{NEWLINE} next_line();
<PREPROCESSOREXCLUDE>{ANY}
<DOERROR>{ANY}* FATAL(("error directive: %s", yytext));
<DOERROR>{NEWLINE} FATAL(("error directive"));

  /* Include file support */
<INCLUDE>\"[^\"]*\" include_file(str_dup(yytext+1), 1);
<INCLUDE>\<[^\>]*\> include_file(str_dup(yytext+1), 0);
<INCLUDE>{ANY} CERROR(("Malformed #include statement")); BEGIN(POSTPREPROCESSOR);

<<EOF>> {
  int wasDefine;
  if (include_depth <= 0)
    {
      yyterminate();
    }
  else
    {
      if ((wasDefine = curr_include->is_define))
	LOG(("End of macro expansion"));
      else
	{
	  LOG(("End of file %s", current_file()));
	  at_end_of_file();
	}
      str_free(curr_include->filename);
      yy_delete_buffer(YY_CURRENT_BUFFER);
      yy_switch_to_buffer(curr_include->buff);
      curr_include--;
      include_depth--;
      LOG((" now in %s (%d)", current_file(), include_depth));

      /* Loop through all levels except for the main level.  The main level
       * is neither an include nor a define.  So, it doesn't make sense to
       * check it.
       */
      update_info();

      pop_state();	/* end excursion, do not change to old state */
      if (YYSTATE == IGNORECFILE)
	BEGIN(INITIAL);	
    }
}

  /* Define support -- see also define.c */
<DEFINE>[A-Za-z_][A-Za-z0-9_]* {
    LOG(("define %s", yytext));
    defineNew = create_define(yytext);
    BEGIN(DEFINEVAL);
  }

<DEFINE>[A-Za-z_][A-Za-z0-9_]*/\( {
    defineNew = create_define(yytext);
    BEGIN(DEFINEMACRO);
  }

<DEFINE>{ANY} {
    CERROR(("Defining something other than a keyword, starts with %c", yytext[0]));
    BEGIN(POSTPREPROCESSOR);
  }

<DEFINEMACRO>\(([A-Za-z, _]*(\\{NEWLINE})*)*\) {
    BEGIN(DEFINEVAL);
  }

<DEFINEMACRO>{ANY} {
    CERROR(("Invalid macro parameter list"));
    BEGIN(POSTPREPROCESSOR);
  }

<DEFINEVAL>[ \t]+

<DEFINEVAL>{NEWLINE} {
    defineNew->value = str_dup("");
    yyless(0);
    BEGIN(POSTPREPROCESSOR);
  }

<DEFINEVAL>({ANY}*(\\{NEWLINE})*)* {
    char *p;

    defineNew->value = str_dup(yytext);
    p = strstr(defineNew->value, "//");
    if (p)
        *p = '\0';
    add_lines(yytext);
    BEGIN(POSTPREPROCESSOR);
  }

<UNDEFINE>[A-Za-z_][A-Za-z0-9_]*/{WHITESPACE} {
    undefine_define(yytext);
    BEGIN(POSTPREPROCESSOR);
  }

<UNDEFINE>{ANY} {
    CERROR(("Undefining something other than a keyword"));
    BEGIN(POSTPREPROCESSOR);
  }

  /* Ifdef support */
<IFDEF>[A-Za-z_][A-Za-z0-9_]*/{WHITESPACE} {
    if (!ifYes)
      {
	LOG(("ifdef (ignored)"));
	more_if_stack();
	duplicate_top_of_stack();
	BEGIN(POSTPREPROCESSOR);
      }
    else
      {
	ifStack[more_if_stack()] = (match_define(yytext)) ? 1 : 0;
	ifWasTrue[ifStackDepth] = ifStack[ifStackDepth];
	LOG(("ifdef %s (%d)", yytext, ifStack[ifStackDepth]));
	reset_if_yes();
	if (ifYes)
	{
	  duplicate_top_of_stack();
	  BEGIN(POSTPREPROCESSOR);
	}
	else
	  BEGIN(PREPROCESSOREXCLUDE);
      }
  }

<IFNDEF>[A-Za-z_][A-Za-z0-9_]*/{WHITESPACE} {
    if (!ifYes)
      {
	LOG(("ifndef (ignored)"));
	more_if_stack();
	duplicate_top_of_stack();
	BEGIN(POSTPREPROCESSOR);
      }
    else
      {
	ifStack[more_if_stack()] = (match_define(yytext)) ? 0 : 1;
	ifWasTrue[ifStackDepth] = ifStack[ifStackDepth];
	LOG(("ifndef %s (%d)", yytext, ifStack[ifStackDepth]));
	reset_if_yes();
	if (ifYes)
	{
	  duplicate_top_of_stack();
	  BEGIN(POSTPREPROCESSOR);
	}
	else
	  BEGIN(PREPROCESSOREXCLUDE);
      }
  }

<IFDEF,IFNDEF>{HORZWHITESPACE}+

<IFDEF,IFNDEF>{ANY} {
    if (!ifYes)
      {
	LOG(("ifdef (ignored)"));
	more_if_stack();
	duplicate_top_of_stack();
	BEGIN(POSTPREPROCESSOR);
      }
    else
      CERROR(("#ifdef/#ifndef test on something other than a keyword"));
  }

<IFDEF,IFNDEF>{NEWLINE} {
    if (!ifYes)
      {
	LOG(("ifdef (ignored)"));
	more_if_stack();
      }
    else
      {
        CERROR(("#ifdef/#ifndef test on nothing"));
      }
    next_line();
    duplicate_top_of_stack();
    BEGIN(pop_state());		/* end excursion */
  }

  /* Just plain #if support */
<IFSTART>[ \t]* {
    inIf = 1;
    BEGIN(KEYWORD);
    return ifPREPROCESSORIF;
  }

<KEYWORD>[ \t]+
<KEYWORD>\\{NEWLINE} next_line();

<KEYWORD>-?{NUMBER}+ {
    zzlval.i = atoi(yytext);
    LOG(("IF: INT: %d", zzlval.i));
    return ifINT;
  }

<KEYWORD>0x{HEXDIGIT}+L? {
    zzlval.i = hex2dec(yytext+2);
    LOG(("IF: INT: 0x%X", zzlval.i));
    return ifINT;
  }

<KEYWORD>defined {
    BEGIN(KWDEF);
    LOG(("IF: DEFINED"));
    return ifDEFINED;
  }

<KEYWORD>!defined {
    BEGIN(KWDEF);
    LOG(("IF: !DEFINED"));
    return ifnDEFINED;
} 

<KWDEF>[ \t]

<KWDEF>[A-Za-z_][A-Za-z0-9_]* {
    zzlval.str = str_dup(yytext);
    LOG(("IF: KEYWORD: %s", yytext));
    BEGIN(KEYWORD);
    return ifSTRING;
  }

<KWDEF>{ANY} LOG(("IF: CHAR '%c'", yytext[0])); return yytext[0];

<KEYWORD>{ANY} LOG(("IF: CHAR '%c'", yytext[0])); return yytext[0];

<KEYWORD>{NEWLINE} LOG(("IF: EOL")); inIf = 0; next_line(); BEGIN(INITIAL); return 0;

    /*
     * C/C++ keywords that indicate the beginning of a block.
     */
<INITIAL>struct		|
<INITIAL>union		|
<INITIAL>class		|
<INITIAL>template	return kSTRUCT;

    /*
     * C/C++ keywords that indicate a type
     */
<INITIAL>HANDLE		|
<INITIAL>UINT		|
<INITIAL>WORD		|
<INITIAL>DWORD		|
<INITIAL>LONG		|
<INITIAL>BYTE		|
<INITIAL>HWND		|
<INITIAL>HGLOBAL	|
<INITIAL>ULONG		|
<INITIAL>LPSTR		|
<INITIAL>inline		|
<INITIAL>_inline	|
<INITIAL>__inline	|
<INITIAL>pascal		|
<INITIAL>_pascal	|
<INITIAL>__pascal	|
<INITIAL>fortran	|
<INITIAL>_fortran	|
<INITIAL>__fortran	|
<INITIAL>cdecl		|
<INITIAL>_cdecl		|
<INITIAL>__cdecl	|
<INITIAL>far		|
<INITIAL>_far		|
<INITIAL>__far		|
<INITIAL>near		|
<INITIAL>_near		|
<INITIAL>__near		|
<INITIAL>huge		|
<INITIAL>_huge		|
<INITIAL>__huge		|

<INITIAL>enum		|
<INITIAL>auto		|
<INITIAL>static		|
<INITIAL>const		|
<INITIAL>volatile	|
<INITIAL>extern		|
<INITIAL>char		|
<INITIAL>short		|
<INITIAL>int		|
<INITIAL>long		|
<INITIAL>signed		|
<INITIAL>unsigned	|
<INITIAL>double		|
<INITIAL>float		|
<INITIAL>void		|
<INITIAL>overload	return kCTYPE;

    /*
     * Other C/C++ keywords of interest
     */
<INITIAL>typedef	return kTYPEDEF;

    /* 
     * Replace defined values, if necessary 
     */
<INITIAL,KEYWORD>[A-Za-z_][A-Za-z0-9_]* {
  if ((defineValue = match_define(yytext))) 
	{ 
	  LOG(("Expanding %s as %s", yytext, defineValue)); 
	  push_state(YYSTATE);
	  include_string(defineValue); 
	} 
  else 
	REJECT;
}

    /* 
     * Discard all whitespace and newlines 
     */
<INITIAL,TYPEDEFMODE,IGNORETILBRACE,IGNORETILBRACEORSEMI>[ \t]+
<INITIAL,TYPEDEFMODE,IGNORETILBRACE,IGNORETILBRACEORSEMI>{NEWLINE} next_line();

    /*
     * Eat typedefs except for the name of the new type.
     */
<TYPEDEFMODE>{SEMICOLON} { 
			    if (last_typedef_keyword == NULL)
			        return kNOTAVALIDTOKEN;

			    yylval.str = last_typedef_keyword;
			    last_typedef_keyword = NULL;
			    return kASTRING; 
			 }

<TYPEDEFMODE>{ANYBUTSEMICOLON}+ {
			    if (last_typedef_keyword)
			        free(last_typedef_keyword);

			    last_typedef_keyword = str_dup(yytext);
			 }

    /*
     * Eat C/C++ code until we hit the special characters that we are
     * interested in.
     */
<IGNORECFILE>{ANY}
<IGNORECFILE>{NEWLINE} next_line();

<IGNORETILBRACE,IGNORETILBRACEORSEMI>\{		return kBEGIN;

<IGNORETILBRACE,IGNORETILBRACEORSEMI>\}		return kEND;

<IGNORETILBRACEORSEMI>{SEMICOLON}		return ';';

<IGNORETILBRACE>{ANYBUTBRACE}
<IGNORETILBRACEORSEMI>{ANYBUTBRACEORSEMI}

<KEYWORD>[A-Za-z_][A-Za-z0-9_]* zzlval.str = yytext; return ifSTRING;

  /* Resource flags */
<INITIAL>PRELOAD yylval.mask.and = ~(0x0040); yylval.mask.or = 0x0040; return kFLAG;
<INITIAL>LOADONCALL yylval.mask.and = ~(0x0040); yylval.mask.or = 0; return kFLAG;
<INITIAL>MOVEABLE yylval.mask.and = ~(0x0010); yylval.mask.or = 0x0010; return kFLAG;
<INITIAL>FIXED yylval.mask.and = ~(0x0010); yylval.mask.or = 0; return kFLAG;
<INITIAL>DISCARDABLE yylval.mask.and = -1; yylval.mask.or = 0; return kFLAG;
<INITIAL>PURE yylval.mask.and = -1; yylval.mask.or = 0; return kFLAG;
<INITIAL>IMPURE yylval.mask.and = -1; yylval.mask.or = 0; return kFLAG;

  /* Resource types */
<INITIAL>TABLENAME return kTABLENAME;
<INITIAL>ACCELERATORS return kACCELERATORS;
<INITIAL>BITMAP return kBITMAP;
<INITIAL>CURSOR return kCURSOR;
<INITIAL>DIALOG return kDIALOG;
<INITIAL>DIALOGEX return kDIALOGEX;
<INITIAL>ICON return kICON;
<INITIAL>MENU return kMENU;
<INITIAL>STRINGTABLE return kSTRINGTABLE;
<INITIAL>VERSIONINFO return kVERSIONINFO;

  /* Resource types (MFC 4.x) */
<INITIAL>DLGINIT return kDLGINIT;
<INITIAL>TOOLBAR return kTOOLBAR;
<INITIAL>BUTTON return kBUTTON;

  /* Version info keywords */
<INITIAL>FILEVERSION return kFILEVERSION;
<INITIAL>PRODUCTVERSION return kPRODUCTVERSION;
<INITIAL>FILEFLAGSMASK return kFILEFLAGSMASK;
<INITIAL>FILEFLAGS return kFILEFLAGS;
<INITIAL>FILEOS return kFILEOS;
<INITIAL>FILETYPE return kFILETYPE;
<INITIAL>FILESUBTYPE return kFILESUBTYPE;
<INITIAL>BLOCK return kBLOCK;
<INITIAL>VALUE return kVALUE;

  /* Menu keywords */
<INITIAL>POPUP return kPOPUP;
<INITIAL>MENUITEM return kMENUITEM;
<INITIAL>SEPARATOR return kSEPARATOR;
<INITIAL>CHECKED return kCHECKED;
<INITIAL>GRAYED return kGRAYED;
<INITIAL>HELP return kHELP;
<INITIAL>INACTIVE return kINACTIVE;

  /* Dialog keywords */
<INITIAL>STYLE return kSTYLE;
<INITIAL>CAPTION return kCAPTION;
<INITIAL>CLASS return kCLASS;
<INITIAL>FONT return kFONT;
<INITIAL>LANGUAGE return kLANGUAGE;
<INITIAL>LTEXT return kLTEXT;
<INITIAL>RTEXT return kRTEXT;
<INITIAL>CTEXT return kCTEXT;
<INITIAL>EDITTEXT return kEDITTEXT;
<INITIAL>LISTBOX return kLISTBOX;
<INITIAL>COMBOBOX return kCOMBOBOX;
<INITIAL>GROUPBOX return kGROUPBOX;
<INITIAL>PUSHBUTTON return kPUSHBUTTON;
<INITIAL>DEFPUSHBUTTON return kDEFPUSHBUTTON;
<INITIAL>CHECKBOX return kCHECKBOX;
<INITIAL>AUTOCHECKBOX return kAUTOCHECKBOX;
<INITIAL>STATE3 return kSTATE3;
<INITIAL>AUTO3STATE return kAUTO3STATE;
<INITIAL>RADIOBUTTON return kRADIOBUTTON;
<INITIAL>AUTORADIOBUTTON return kAUTORADIOBUTTON;
<INITIAL>SCROLLBAR return kSCROLLBAR;
<INITIAL>CONTROL return kCONTROL;

<INITIAL>VIRTKEY return kVIRTKEY;
<INITIAL>ASCII return kASCII;
<INITIAL>SHIFT return kSHIFT;
<INITIAL>ALT return kALT;
<INITIAL>NOINVERT return kNOINVERT;

<INITIAL>ALIGN return kALIGN;
<INITIAL>ENDIAN return kENDIAN;

<INITIAL>BEGIN return kBEGIN;
<INITIAL>END return kEND;
<INITIAL>\{ return kBEGIN;
<INITIAL>\} return kEND;
<INITIAL>NOT return kNOT;

<HEX>' BEGIN(pop_state()); return *yytext;
<HEX>{HEXDIGIT}{2} yylval.str = str_dup(yytext); return kHEXCHARS;
<HEX>[ \t]+
<INITIAL>' push_state(HEX); return *yytext;

<INITIAL>-?{NUMBER}+ {
    yylval.i = atoi(yytext);
    LOG(("INT: %d", yylval.i));
    return (yylval.i > 0xffff) ? kLONGINT : kSHORTINT;
  }

<INITIAL>-?{NUMBER}+[Ll] {
    yylval.i = atoi(yytext);
    LOG(("INT: %d", yylval.i));
    return kLONGINT;
  }

<INITIAL>0x{HEXDIGIT}+ {
    yylval.i = hex2dec(yytext+2);
    LOG(("INT: 0x%X", yylval.i));
    return (yylval.i > 0xffff) ? kLONGINT : kSHORTINT;
  }

<INITIAL>0x{HEXDIGIT}+[Ll] {
     yylval.i = hex2dec(yytext+2);
     LOG(("INT: 0x%X", yylval.i));
     return kLONGINT;
  }

<INITIAL>[A-Za-z0-9\.\_\/\\]+ {
     yylval.str = str_dup(yytext);
     LOG(("ASTRING: %s", yylval.str));
     return kASTRING;
  }

<INITIAL>\"[^\"]*\" {
     yylval.str = str_dup(yytext+1);
     yylval.str[strlen(yylval.str)-1] = '\0';
     LOG(("QSTRING: %s", yylval.str));
     return kQSTRING;
  }

<INITIAL>{ANY} LOG(("CHAR: '%c'", *yytext)); return *yytext;

%%

int zzlex(void)
{
  return yylex();
}

int yywrap(void)
{
  return 1;
}

int more_if_stack(void)
{
  if (++ifStackDepth >= MAX_IF_STACK_DEPTH)
    FATAL(("Too many #ifs or #ifdefs"));
  ifWasTrue[ifStackDepth] = 0;
  return ifStackDepth;
}

int less_if_stack(void)
{
  if (ifStackDepth < 0)
    FATAL(("Too many #endifs"));
  ifStackDepth--;
  reset_if_yes();
  return ifStackDepth;
}

int reset_if_yes(void)
{
  int i;
  ifYes = 1;
  for (i = 0; i <= ifStackDepth; i++)
    ifYes &= ifStack[i];
  return ifYes;
}

void add_include(void)
{
  if (++include_depth >= MAX_INCLUDE_DEPTH)
    FATAL(("Includes/macros nested too deeply"));

  curr_include++;
  curr_include->buff = YY_CURRENT_BUFFER;
  LOG(("add_include (%d)", include_depth));
}

void include_string(char *string)
{
  add_include();
  yy_scan_string(string);
  curr_include->filename = str_dup((curr_include - 1)->filename);
  curr_include->line = (curr_include - 1)->line;
  curr_include->column = (curr_include - 1)->column;
  curr_include->is_define = 1;
  at_start_of_macro();
}

void include_file(char *filename, int lookHere)
{
  char *pFile = str_dup(filename);
  char *pString;

  pFile[strlen(pFile)-1] = '\0';
  pString = load_file(str_dup(pFile), lookHere);
  if (!pString)
    {
      str_free(pFile);
      return;
    }
  add_include();
  yy_scan_string(pString);
  str_free(pString);
  curr_include->filename = pFile;
  curr_include->line = 1;
  curr_include->column = 1;
  curr_include->is_define = 0;
  at_start_of_file();

  LOG(("Now in file %s", pFile));

  IPBEGIN(INITIAL);
}

int hex2dec(char *hex)
{
  int result = 0, len, i;
	
  len = strlen(hex);
  
  for (i=0; i < len; i++)
    if (isxdigit(hex[i]))
      {
	result <<= 4;
	if ((hex[i] >= '0') && (hex[i] <= '9'))
	  result += hex[i] - '0';
	else if ((hex[i] >= 'A') && (hex[i] <= 'F'))
	  result += hex[i] - 'A' + 10;
	else if ((hex[i] >= 'a') && (hex[i] <= 'f'))
	  result += hex[i] - 'a' + 10;
	else
	  FATAL(("isxdigit() returned true on '%c', not a hex digit", hex[i]));
      }
  return result;
}

extern char in_name[];

void include_init(void)
{
  curr_include->filename = in_name;
  curr_include->line = 1;
  curr_include->column = 1;
  curr_include->is_define = 0;
}

void include_init_string(char *str)
{
  yy_scan_string(str);
  at_start_of_file();
}

char *current_file(void)
{
  return curr_include->filename;
}

int current_line(void)
{
  return curr_include->line;
}

int next_line(void)
{
  curr_include->column = 1;
  if (!(++(curr_include->line) % 50))
    update_info();
  yy_set_bol(1);
  return curr_include->line;
}

int add_lines(char *s)
{
  while (*s)
    if ((*s++ == '\n') || (*s == '\r'))
	next_line();
    else
      yy_set_bol(0);
  return current_line();
}

int extra_lines(void)
{
  int i, c;
  for (c = i = 0; i <= include_depth; i++)
    if (!include_stack[i].is_define)
      c += include_stack[i].line;
  return c;
}

void *get_ext(void)
{
  return curr_include->ext;
}

void *set_ext(void *v)
{
  return curr_include->ext = v;
}

void ignore_until_brace()
{
    LOG(("STATE: IGNORETILBRACE"));
    BEGIN(IGNORETILBRACE);
}

void ignore_until_semicolon()
{
    LOG(("STATE: IGNORETILBRACEORSEMI"));
    BEGIN(IGNORETILBRACEORSEMI);
}

void typedef_mode()
{
    LOG(("STATE: TYPEDEF"));
    BEGIN(TYPEDEFMODE);
}

void initial_mode()
{
    LOG(("STATE: INITIAL"));
    BEGIN(INITIAL);
}

void ignore_rest_of_file()
{
    LOG(("STATE: IGNORECFILE"));
    BEGIN(IGNORECFILE);
}

static void push_state(int new_state)
{
    if (stateStackIndex == MAX_STATE_STACK_DEPTH)
	FATAL(("Max state stack depth exceeded"));

    LOG(("(%d) PUSH from %d to %d", stateStackIndex, YYSTATE, new_state));

    stateStack[stateStackIndex++] = YYSTATE;
    BEGIN(new_state);
}

static int pop_state(void)
{
    if (--stateStackIndex < 0)
    	FATAL(("Stack underflow"));

    LOG(("(%d) POP from %d to %d", 
	 stateStackIndex, YYSTATE, stateStack[stateStackIndex]));

    return(stateStack[stateStackIndex]);
}

static void duplicate_top_of_stack(void)
{
    int top_value;

    if (stateStackIndex == MAX_STATE_STACK_DEPTH)
	FATAL(("Max state stack depth exceeded"));

    top_value = stateStack[stateStackIndex-1];

    LOG(("(%d) PUSH from %d to %d", stateStackIndex, top_value, top_value));

    stateStack[stateStackIndex++] = top_value;
}    

static void my_fatal_error( yyconst char msg[] )
{
	(void) fprintf( stderr, "%s\n", msg );
	exitfunc();
	exit( YY_EXIT_FAILURE );
}

