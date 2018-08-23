
%{
/*************************************************************************

    ifyacc.y	1.9
    yacc parser for #if statements

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
#ifdef __OS2__
#include <malloc.h>
#endif
#include "rc.h"
#include "define.h"
#include "buffer.h"

int zzerror(char *str);
int zzlex(void);

extern int ifResult;

/* Some systems don't like alloca, so we have to make them work right */
#if defined(_MAC) || defined(macintosh)
#define alloca malloc
#endif

#if defined(TWIN_HAVEALLOCA)
#include <alloca.h>
#endif
%}

%union
{
  int i;
  char *str;
}

%token <i> ifINT
%token <str> ifSTRING
%token ifDEFINED
%token ifnDEFINED
%token ifPREPROCESSORIF

%type <i> expr

%right '!'
%left '+' '-'
%left '*' '/' '%'
%left '&' '|'
%left '<' '>'
%right '='

%%

preprocessorifclause: ifPREPROCESSORIF expr {ifResult = $2; }

expr: ifINT
| ifSTRING { LOG(("Undefined constant %s", $1)); $$ = 0; }
| expr '+' expr { $$ = $1 + $3; }
| expr '-' expr { $$ = $1 - $3; }
| expr '*' expr { $$ = $1 * $3; }
| expr '/' expr { $$ = $1 / $3; }
| expr '%' expr { $$ = $1 % $3; }
| expr '|' expr { $$ = $1 | $3; }
| expr '&' expr { $$ = $1 & $3; }
| expr '|' '|' expr { $$ = $1 || $4; }
| expr '&' '&' expr {$$ = $1 && $4; }
| '(' expr ')' { $$ = $2; }
| '!' expr {$$ = !$2; }
| expr '>' expr { $$ = $1 > $3; }
| expr '<' expr { $$ = $1 < $3; }
| expr '>' '=' expr { $$ = ($1 >= $4); }
| expr '<' '=' expr { $$ = ($1 <= $4); }
| expr '=' '=' expr { $$ = ($1 == $4); }
| expr '!' '=' expr { $$ = ($1 != $4); }
| ifDEFINED '(' ifSTRING ')' { LOG(("%s is%s defined", $3, match_define($3) ? "" : " not")); $$ = match_define($3) ? 1 : 0; str_free($3); }
| ifDEFINED ifSTRING { LOG(("%s is%s defined", $2, match_define($2) ? "" : " not")); $$ = match_define($2) ? 1 : 0; str_free($2); }
| ifnDEFINED '(' ifSTRING ')' { LOG(("%s is%s defined", $3, match_define($3) ? "" : " not")); $$ = !match_define($3) ? 1 : 0; str_free($3); }
| ifnDEFINED ifSTRING { LOG(("%s is%s defined", $2, match_define($2) ? "" : " not")); $$ = !match_define($2) ? 1 : 0; str_free($2); }

%%

int yyerror(char *str)
{
  CERROR(("%s", str));
  return 0;
}
