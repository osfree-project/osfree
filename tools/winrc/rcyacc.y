%{
/*************************************************************************

	rcyacc.y	1.31
	yacc resource file parser
  
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

#include "windows.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rc.h"
#include "define.h"
#include "buffer.h"
#include "resfunc.h"

int yyerror(char *str);
int yylex(void);

void ignore_until_semicolon();
void ignore_until_brace();
void typedef_mode();
void initial_mode();
void ignore_rest_of_file();

int hex2dec(char *);

#define YYPRINT(file,type,val) if ((type == LONGINT) || (type == SHORTINT)) fprintf(file, " %d", val.i); else \
                               if (type == STRING) fprintf(file, " %s", val.str);

/* Some systems don't like alloca, so we have to make them work right */
#if defined(_MAC) || defined(macintosh)
#define alloca malloc
#endif

#ifdef TWIN_HASALLOCA
#include <alloca.h>
#endif
%}


%union
{
  strp str;
  buffp b;
  resp resource;
  struct maskvals mask;
  struct dimvals dim;
  struct xyvals xy;
  struct binflagvals binflags;
  struct userresbuffer *urb;
  unsigned long i;
  char c;
}

%token <str> kSTRING
%token <str> kQSTRING
%token <str> kASTRING
%token <str> kHEXCHARS
%token <i> kLONGINT kSHORTINT
%token <mask> kFLAG

%token kTABLENAME
%token kACCELERATORS
%token kBITMAP
%token kCURSOR
%token kDIALOG
%token kDIALOGEX
%token kICON
%token kMENU
%token kSTRINGTABLE
%token kVERSIONINFO
%token kRCDATA

%token kDLGINIT
%token kTOOLBAR
%token kBUTTON

%token kFILEVERSION
%token kPRODUCTVERSION
%token kFILEFLAGSMASK
%token kFILEFLAGS
%token kFILEOS
%token kFILETYPE
%token kFILESUBTYPE
%token kBLOCK
%token kVALUE

%token kPOPUP
%token kMENUITEM
%token kSEPARATOR
%token kCHECKED
%token kGRAYED
%token kHELP
%token kINACTIVE

%token kSTYLE
%token kCAPTION
%token kCLASS
%token kFONT
%token kLANGUAGE
%token kLTEXT
%token kRTEXT
%token kCTEXT
%token kEDITTEXT
%token kLISTBOX
%token kCOMBOBOX
%token kGROUPBOX
%token kPUSHBUTTON
%token kDEFPUSHBUTTON
%token kCHECKBOX
%token kAUTOCHECKBOX
%token kSTATE3
%token kAUTO3STATE
%token kRADIOBUTTON
%token kAUTORADIOBUTTON
%token kSCROLLBAR
%token kCONTROL

%token kVIRTKEY
%token kASCII
%token kSHIFT
%token kALT
%token kNOINVERT

%token kALIGN
%token kENDIAN

%token kBEGIN
%token kEND
%token kNOT

%token kTYPEDEF
%token kSTRUCT
%token kCTYPE

%token kNOTAVALIDTOKEN

%type <i> aexpr expr integer nonparenexpr
%type <i> flags menuflags menuflag
%type <str> str fileref astr
%type <b> binary binarydata binarylines binaryline binarybytes
%type <c> binarybyte
%type <dim> dims
%type <xy> coords
%type <mask> styleflags controlstyle
%type <i> accelflags accelflag
%type <i> controlid
%type <str> resourceid

%type <resource> bitmapdata cursordata icondata menudata dialogdata acceldata
%type <resource> rcdatares binaryres
%type <b> menudef menuitemlist menuitem
%type <b> dlgdef dlgbase controls control
%type <b> acceldef accelitemlist accelitem
%type <b> userres
%type <binflags> binresflags
%type <urb> userreslines userresline

%type <resource> dlginitdata
%type <b> dlginitdef
%type <b> dlginititemlist

%type <resource> toolbardata
%type <b> toolbardef
%type <i> toolbardims
%type <b> toolbaritemlist
%type <i> toolbaritem

%right '!'
%left '+' '-'
%left '*' '/' '%'
%left '&' '|'
%left '<' '>'
%right '='

%%
resourcefile: /* empty */
            | resourcefile resource
	    | resourcefile c_declaration
;

resource: resourceid kBITMAP flags bitmapdata { add_resource($1, $3, $4); }
        | resourceid kCURSOR flags cursordata { add_resource($1, $3, $4); }
        | resourceid kICON flags icondata { add_resource($1, $3, $4); }
        | resourceid kMENU flags menudata { add_resource($1, $3, $4); }
        | resourceid kDIALOG flags dialogdata { add_resource($1, $3, $4); }
	| resourceid kDIALOGEX flags dialogdata { add_resource($1, $3, $4);}
	| resourceid kDLGINIT flags dlginitdata { add_resource($1, $3, $4); }
	| resourceid kTOOLBAR flags toolbardata { add_resource($1, $3, $4); }
        | resourceid kACCELERATORS flags acceldata { add_resource($1, $3, $4); }
        | kSTRINGTABLE flags strtab { set_string_flags($2); }
        | resourceid kRCDATA flags rcdatares { add_resource($1, $3, $4); }
        | resourceid resourceid flags binaryres { type_resource($4, $2); add_resource($1, $3, $4); }
        | kTABLENAME str { VPRINT(("Table name is now %s", $2)); strcpy(tbl_name, $2); }
	| resourceid kVERSIONINFO versioninfodata { }
	| kVERSIONINFO versioninfodata { }
	| languagespec { }
;

resourceid: str
          | nonparenexpr { $$ = makeintresource($1); }
;

flags: { $$ = 0x10; /* These seem to be default flags */ }
     | flags kFLAG { $$ = ($1 & $2.and) | $2.or; }
;

menudata: maybelanguage menudef { $$ = make_menu_resource($2); }
;

menudef: kBEGIN menuitemlist kEND { $$ = set_menu_last($2); }
       | kBEGIN error kEND { CERROR(("Error processing menu definition")); }
;

menuitemlist: /* empty */ { $$ = NULL; }
            | menuitemlist menuitem { $$ = append_menu_node($1, $2); }
;

menuitem: kPOPUP str menuflags menudef { $$ = create_menu_popup($2, $4, $3); }
        | kMENUITEM kSEPARATOR { $$ = create_menu_separator(); }
        | kMENUITEM str fieldsep aexpr menuflags fieldsep 
					{ $$ = create_menu_item($2, $4, $5); }
;

menuflags: /* empty */ { $$ = 0; }
	 | menuflags fieldsep menuflag { $$ = $1 | $3; }
;

menuflag: aexpr
        | kCHECKED { $$ = menu_checked(); }
        | kGRAYED { $$ = menu_grayed(); }
        | kINACTIVE { $$ = menu_inactive(); }
        | kHELP { $$ = menu_help(); }
;

acceldata: maybelanguage acceldef { $$ = make_accelerator_resource($2); }
;

acceldef: kBEGIN accelitemlist kEND { $$ = $2; }
        | kBEGIN error kEND { CERROR(("Error processing accelerator table definition")); }
;

accelitemlist: /* empty */ { $$ = NULL; }
             | accelitemlist accelitem { $$ = append_accel($1, $2); }
;

accelitem: kQSTRING fieldsep aexpr accelflags { $$ = make_accel_s($1, $3, $4); }
         | aexpr fieldsep aexpr accelflags { $$ = make_accel_i($1, $3, $4); }
;

accelflags: /* empty */ { $$ = 0; }
          | accelflags accelflag { $$ = $1 | $2; }
;

accelflag: fieldsep kVIRTKEY { $$ = 0x01; }
         | fieldsep kASCII { $$ = 0x00; }
         | fieldsep kCONTROL { $$ = 0x08; }
         | fieldsep kSHIFT { $$ = 0x04; }
         | fieldsep kALT { $$ = 0x10; }
         | fieldsep kNOINVERT { $$ = 0x02; }
;

dialogdata: dlgdef { $$ = make_dialog_resource($1); }
;

dlgdef: dlgbase kBEGIN controls kEND { $$ = set_dialog_controls($1, $3); }
      | dlgbase kBEGIN error kEND { CERROR(("Error processing dialog box definition")); }
;

dlgbase: dims { $$ = make_dialog($1); }
| dlgbase kSTYLE expr { $$ = set_dialog_style($1, $3); }
| dlgbase kSTYLE error { CERROR(("Expected integer after STYLE")); }
| dlgbase kCAPTION str { $$ = set_dialog_caption($1, $3); }
| dlgbase kCLASS str { $$ = set_dialog_class($1, $3); }
| dlgbase kMENU resourceid { $$ = set_dialog_menu($1, $3); }
| dlgbase kFONT aexpr fieldsep str { $$ = set_dialog_font($1, $5, $3); }
| dlgbase kFONT error fieldsep str { CERROR(("Need integer point size for FONT")); }
| dlgbase languagespec
;

dims: coords fieldsep coords { $$.corner = $1; $$.extent = $3; }
;

coords: aexpr fieldsep aexpr { $$.x = $1; $$.y = $3; }
      | error fieldsep aexpr { CERROR(("Coordinates must be integers")); }

      | aexpr fieldsep error { CERROR(("Coordinates must be integers")); }
;

controls: /* empty */ { $$ = NULL; }
        | controls control { $$ = append_control($1, $2); }
;

controlstyle: /* empty */ { $$.or = 0; $$.and = -1;}
            | styleflags { $$ = $1; }
;

styleflags: integer			{ $$.or = $1; $$.and = -1; }
          | kNOT integer			{ $$.or = 0; $$.and = ~$2; }
	  | styleflags '|' styleflags	{ 
					  $$.or = $1.or | $3.or; 
					  $$.and = $1.and & $3.and; 
					}
	  | '(' styleflags ')'		{ $$.or = $2.or; $$.and = $2.and; }
;

control: kLTEXT fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_ltext_control($3, $5, $7, $9); }
       | kRTEXT fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_rtext_control($3, $5, $7, $9); }
       | kCTEXT fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_ctext_control($3, $5, $7, $9); }
       | kICON fieldsep resourceid fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_icon_control($3, $5, $7, $9); }
       | kEDITTEXT fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_edit_control($3, $5, $7); }
       | kLISTBOX fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_listbox_control($3, $5, $7); }
       | kCOMBOBOX fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_combobox_control($3, $5, $7); }
       | kGROUPBOX fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_groupbox_control($3, $5, $7, $9); }
       | kPUSHBUTTON fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_pushbutton_control($3, $5, $7, $9); }
       | kDEFPUSHBUTTON fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_defpushbutton_control($3, $5, $7, $9); }
       | kRADIOBUTTON fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_radiobutton_control($3, $5, $7, $9); }
       | kAUTORADIOBUTTON fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle
	{
	$9.or |= BS_AUTORADIOBUTTON;
	$$ = create_autoradiobutton_control($3,$5,$7,$9);
	}    
       | kCHECKBOX fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_checkbox_control($3, $5, $7, $9); }
       | kAUTOCHECKBOX fieldsep str fieldsep controlid fieldsep dims fieldsep controlstyle 
	{
	$9.or |= BS_AUTOCHECKBOX;
   	$$ = create_checkbox_control($3,$5,$7,$9);
	}	
       | kSTATE3 fieldsep str fieldsep controlid fieldsep str fieldsep styleflags fieldsep dims 
	{
	$9.or |= BS_3STATE;
        $$ = create_generic_control($3,$5,$7,$9, $11);
	}
       | kAUTO3STATE fieldsep str fieldsep controlid fieldsep str fieldsep styleflags
 fieldsep dims
        {
        $9.or |= BS_AUTO3STATE;
        $$ = create_generic_control($3,$5,$7,$9, $11);
        }
       | kSCROLLBAR fieldsep controlid fieldsep dims fieldsep controlstyle { $$ = create_scrollbar_control($3, $5, $7); }
       | kCONTROL fieldsep str fieldsep controlid fieldsep str fieldsep styleflags fieldsep dims Woptions {$$ = create_generic_control($3, $5, $7, $9, $11);}
;

controlid: aexpr
         | error { CERROR(("Control identifiers must be integers")); }
;

dlginitdata
	: dlginitdef
		{ $$ = make_dlginit_resource($1); }
	;

dlginitdef
	: kBEGIN dlginititemlist kEND
		{ $$ = $2; }
	| kBEGIN error kEND
		{ CERROR(("Error processing dialog init definition")); }
	;

dlginititemlist
	: /* empty */
		{ $$ = make_dlginit(); }
	| dlginititemlist kSHORTINT
		{
		  $$ = add_dlginit_item(add_dlginit_item($1,
			(BYTE)((WORD)$2) & 0xff),
			(BYTE)(((WORD)$2) >> 8) & 0xff);
		}
	| dlginititemlist kQSTRING	/* ASSUMPTION: all strings are "\000" */
		{ $$ = add_dlginit_item($1, '\0'); }
	| dlginititemlist ','
		{ $$ = $1; }
	;

toolbardata
	: toolbardef
		{ $$ = make_toolbar_resource($1); }
	;

toolbardef
	: toolbardims kBEGIN toolbaritemlist kEND
		{ $$ = set_toolbar_dims($3, LOWORD($1), HIWORD($1)); }
	| toolbardims kBEGIN error kEND
		{ CERROR(("Error processing tool bar definition")); }
	;

toolbardims
	: kSHORTINT ',' kSHORTINT
		{ $$ = MAKELONG($1, $3); }
	;

toolbaritemlist
	: /* empty */
		{ $$ = make_toolbar(1, 16, 15); }
	| toolbaritemlist toolbaritem
		{ $$ = add_toolbar_item($1, (WORD)$2); }
	;

toolbaritem
	: kBUTTON kSHORTINT
		{ $$ = $2; }
	| kSEPARATOR
		{ $$ = 0; }
	;

strtab: maybelanguage kBEGIN strtablines kEND
      | maybelanguage kBEGIN error kEND { CERROR(("Error processing string table")); }
;

strtablines: /* empty */
           | strtablines strtabline
;

strtabline: aexpr fieldsep str { add_table_string($1, $3); }
;

bitmapdata: binary { $$ = make_bitmap_resource($1); }
;

cursordata: binary { $$ = make_cursor_resource($1); }
;

icondata: binary { $$ = make_icon_resource($1); }
;

rcdatares: binaryres { $$ = type_resource_rcdata($1); }
;

binaryres: fileref { $$ = make_binary_resource(buff_load($1, 1)); }
         | userres { $$ = make_binary_resource($1); }
;

userres: binresflags kBEGIN userreslines kEND { $$ = pack_rcdata($1, $3); }
       | binresflags kBEGIN error kEND { CERROR(("Error in user-defined resource definition")); }
;

userreslines: userresline { $$ = $1; }
            | userreslines fieldsep userresline { $$ = urb_cat($1, $3); }
;
            
userresline: str { $$ = urb_string($1); }
           | kSHORTINT { $$ = urb_short($1); }
           | kLONGINT { $$ = urb_long($1); }
	   | binaryline { $$ = urb_binary($1); }
;

binresflags: /* empty */ { $$ = defbinflags; }
           | binresflags kALIGN aexpr { $1.align = $3; $$ = $1; }
           | binresflags kALIGN str { $1.align = alignval($3); $$ = $1; }
           | binresflags kENDIAN str { if (($3[0] == 'b') || ($3[0] == 'B')) $1.endian = big;
                                      else if (($3[0] == 'l') || ($3[0] == 'L')) $1.endian = little;
                                      else CERROR(("Invalid endian specification %s", $3));
                                      $$ = $1; }
           | binresflags languagespec

/* Version Info resource */
versioninfodata: fixedinfo kBEGIN versioninfoblock kEND 

fixedinfo: /* empty */
	 | fixedinfo fistmnt

fistmnt: kFILEVERSION aexpr fieldsep aexpr fieldsep aexpr fieldsep aexpr
       | kPRODUCTVERSION aexpr fieldsep aexpr fieldsep aexpr fieldsep aexpr
       | kFILEFLAGSMASK aexpr
       | kFILEFLAGS aexpr
       | kFILEOS aexpr
       | kFILETYPE aexpr
       | kFILESUBTYPE aexpr

versioninfoblock: /* empty */
		| versioninfoblock viblocks

viblocks: kBLOCK kQSTRING kBEGIN viblockstmnts kEND
        | kBLOCK kQSTRING '=' aexpr kBEGIN viblockstmnts kEND


viblockstmnts: /* empty */
	     | viblockstmnts viblockstmnt

valuestmnt: kVALUE kQSTRING fieldsep aexpr fieldsep aexpr
          | kVALUE kQSTRING fieldsep manyqstrings


viblockstmnt: kBLOCK kQSTRING kBEGIN viblockstmnts kEND
	    | kBLOCK kQSTRING '=' aexpr kBEGIN viblockstmnts kEND
            | valuestmnt

manyqstrings: kQSTRING { }
	    | manyqstrings kQSTRING { }

fileref: astr
;

binary: binarydata
      | fileref { $$ = buff_load($1, 1); }
;

binarydata: kBEGIN binarylines kEND { $$ = $2; }
          | kBEGIN error kEND { CERROR(("Error in binary data lines")); $$ = NULL; }
;

binarylines: /* empty */ { $$ = NULL; }
           | binarylines binaryline { $$ = buff_cat($1, $2); }
;

binaryline: '\'' binarybytes '\'' { $$ = $2; }
          | '\'' error '\'' { CERROR(("Error in binary data")); $$ = '\0'; }
;

binarybytes: /* empty */ { $$ = NULL; }
           | binarybytes binarybyte { $$ = buff_append_byte($1, $2); }
;

binarybyte: kHEXCHARS { $$ = hex2dec($1); }
;

maybelanguage: /* empty */
	     | maybelanguage languagespec

languagespec: kLANGUAGE aexpr fieldsep aexpr fieldsep

/* Arithmetic-only expression, no logical operators allowed */
aexpr: integer
    | aexpr '+' aexpr { $$ = $1 + $3; }
    | aexpr '-' aexpr { $$ = $1 - $3; }
    | aexpr '*' aexpr { $$ = $1 * $3; }
    | aexpr '/' aexpr { $$ = $1 / $3; }
    | aexpr '%' aexpr { $$ = $1 % $3; }
    | aexpr '|' aexpr { $$ = $1 | $3; }
    | aexpr '&' aexpr { $$ = $1 & $3; }
    | '(' aexpr ')' { $$ = $2; }

/* 
 * Arithmetic-only expression, no logical operators allowed, 
 * can't start with an open paren 
 */
nonparenexpr: integer
    | integer '+' aexpr { $$ = $1 + $3; }
    | integer '-' aexpr { $$ = $1 - $3; }
    | integer '*' aexpr { $$ = $1 * $3; }
    | integer '/' aexpr { $$ = $1 / $3; }
    | integer '%' aexpr { $$ = $1 % $3; }
    | integer '|' aexpr { $$ = $1 | $3; }
    | integer '&' aexpr { $$ = $1 & $3; }

/* Arithmetic and/or logical expression */
expr: integer
    | expr '+' expr { $$ = $1 + $3; }
    | expr '-' expr { $$ = $1 - $3; }
    | expr '*' expr { $$ = $1 * $3; }
    | expr '/' expr { $$ = $1 / $3; }
    | expr '%' expr { $$ = $1 % $3; }
    | expr '|' expr { $$ = $1 | $3; }
    | expr '&' expr { $$ = $1 & $3; }
    | expr '|' '|' expr { $$ = $1 || $4; }
    | expr '&' '&' expr { $$ = $1 && $4; }
    | '(' expr ')' { $$ = $2; }
    | '!' expr { $$ = !$2; }
    | expr '>' expr { $$ = $1 > $3; }
    | expr '<' expr { $$ = $1 < $3; }
    | expr '>' '=' expr { $$ = ($1 >= $4); }
    | expr '<' '=' expr { $$ = ($1 <= $4); }
    | expr '=' '=' expr { $$ = ($1 == $4); }
    | expr '!' '=' expr { $$ = ($1 != $4); }
;

str: kSTRING
   | kQSTRING
   | kASTRING
   | kHEXCHARS
;

astr: kQSTRING
    | kASTRING
;

integer: kLONGINT
       | kSHORTINT
;

/*
 * Attempt to eat C/C++ code.  This grammar looks a little wierd because it
 * is not a full grammar for C/C++.  The idea was to recognize just enough to
 * semi-intelligently throw out code.
 *
 * 96/10/18 - Two problems have crept up that have resulted in taking a
 *	      shortcut for the moment.
 *
 *	      BUG 1 - Parameterized macros are not correctly handled
 *	      BUG 2 - The typedef grammar does not support:
 *
 *		      typedef int (pascal *func)(int parm1, int parm2);
 *
 *	      To avoid dealing with these things, we are assuming that
 *	      once we run across a valid C/C++ construct, the rest of the
 *	      file will not contain any resource definitions.  The following
 *	      element controls this behavior:
 *
 *			{ ignore_rest_of_file(); }
 */
c_declaration: struct_union_class
	     | typedef
	     | prototype

/*
 * This rule eats everything within a set of curly braces.  It references
 * itself to keep track of matching braces.
 */
begin_block_end: kBEGIN { ignore_until_brace(); } begin_block_end_or_null kEND

begin_block_end_or_null: /* empty */
		       | begin_block_end begin_block_end_or_null

/*
 * STRUCT is any of struct, union, class or template
 */
struct_union_class: kSTRUCT { ignore_until_semicolon(); } 
		    begin_block_end_or_null { ignore_until_semicolon(); }
		    ';' /* { initial_mode(); } */
		    { ignore_rest_of_file(); }

/*
 * We don't need to expand typedefs to their actual value, instead we treat
 * them as a macro that expands to a reserved word that we will recognize as
 * a C/C++ type.
 */
typedef: kTYPEDEF { typedef_mode(); } 
         begin_block_end_or_null { typedef_mode(); }
	 kASTRING {
		     DEFINESTRUCT *defineNew = create_define($5);
		     defineNew->value = str_dup("extern");
		     initial_mode();
		     { ignore_rest_of_file(); }
		 }
	 /* The semicolon is eaten by lex */

/*
 * This should eat function prototypes and extern declarations of global
 * variables.
 */
prototype: kCTYPE { ignore_until_semicolon(); } 
	   begin_block_end_or_null { ignore_until_semicolon(); }
	   ';' /* { initial_mode(); } */
	   { ignore_rest_of_file(); }
	 | kASTRING '(' { ignore_until_semicolon(); } 
	   ';' /* { initial_mode(); } */
	   { ignore_rest_of_file(); }

/*
 * We used to require commas between fields, but it turns out that Microsoft's
 * rc doesn't require the commas.  Every place that we used to require a comma
 * has now been replaced by the keyword fieldsep.  If you require the strict
 * behavior, then change the definition below to be:
 *
 *	fieldsep: ','
 */
fieldsep: /* empty */
	| ',' fieldsep

/*
 * Option
 */
Woptions: /* empty */
        | fieldsep styleflags

%%

int yyerror(char *str)
{
  CERROR(("%s", str));
  return 0;
}

void set_position(int first_line, int last_line, int first_column, int last_column)
{
#ifdef NEED_BISON_POSITION_INFO
  yylloc.first_line = first_line;
  yylloc.last_line = last_line;
  yylloc.first_column = first_column;
  yylloc.last_column = last_column;
#endif
}

void rcyacc_debug_level(int level)
{
    extern int yydebug;
    
#ifdef YYDEBUG
    yydebug = level;
#endif
}
