/*!
 *  @file awkgram.y
 *  @brief yacc grammar for the awk language.
 *
 *  Produces ytab.c and ytab.h, which are then used by the lexer
 *  (lex.c), the parser (yyparse), the node builders (parse.c) and
 *  the generated token-handler table (proctab.c, built by maketab
 *  from ytab.h). Do not edit ytab.c / ytab.h / proctab.c by hand:
 *  edit this grammar instead.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

/****************************************************************
Copyright (C) Lucent Technologies 1997
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name Lucent Technologies or any of
its entities not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

%{
#include <stdio.h>
#include <string.h>
#include "awk.h"

/*!
 *  @brief Checks whether a name is already present in an argument list.
 *
 *  @param[in] list Argument list head.
 *  @param[in] item Cell of the name to check.
 */
void checkdup(Node *list, Cell *item);

/*!
 *  @brief Dummy end-of-input hook required by yacc parsers.
 *
 *  @return Always 1.
 */
int yywrap(void) { return(1); }

Node	*beginloc = 0;	/*!< Node list of BEGIN actions. */
Node	*endloc = 0;	/*!< Node list of END actions. */
int	infunc	= 0;	/*!< 1 while parsing the arglist or body of a function. */
int	inloop	= 0;	/*!< 1 while inside a while, for or do loop. */
char	*curfname = 0;	/*!< Name of the function currently being parsed. */
Node	*arglist = 0;	/*!< Argument list of the function currently being parsed. */
%}

/*!
 *  @brief Semantic value union passed between the lexer and the parser.
 */
%union	{
	Node	*p;   /*!< Node pointer. */
	Cell	*cp;  /*!< Cell pointer. */
	int	i;    /*!< Integer value. */
	char	*s;   /*!< String value. */
}

%token	<i>	FIRSTTOKEN
%token	<i>	PROGRAM
%token	<i>	PASTAT
%token	<i>	PASTAT2
%token	<i>	XBEGIN XEND
%token	<i>	NL
%token	<i>	ARRAY
%token	<i>	MATCH NOTMATCH MATCHOP
%token	<i>	FINAL DOT ALL CCL NCCL CHAR OR STAR QUEST PLUS
%token	<i>	AND BOR APPEND EQ GE GT LE LT NE
%token	<i>	IN
%token	<i>	ARG BLTIN BREAK CLOSE CONTINUE DELETE DO EXIT FOR FUNC
%token	<i>	SUB GSUB IF INDEX LSUBSTR MATCHFCN NEXT NEXTFILE
%token	<i>	ADD MINUS MULT DIVIDE MOD
%token	<i>	ASSIGN ASGNOP ADDEQ SUBEQ MULTEQ DIVEQ MODEQ POWEQ
%token	<i>	PRINT PRINTF SPRINTF
%token	<i>	ELSE INTEST CONDEXPR
%token	<i>	POSTINCR PREINCR POSTDECR PREDECR
%token	<cp>	VAR IVAR VARNF CALL NUMBER STRING
%token	<s>	REGEXPR
%token	<i>	GETLINE RETURN SPLIT SUBSTR WHILE
%token	<i>	CAT NOT UMINUS POWER
%token	<i>	DECR INCR INDIRECT
%token	<i>	LASTTOKEN

%type	<p>	program
%type	<p>	pas pa_stat pa_stats
%type	<p>	pattern patternlist patternlist_nl
%type	<p>	ppattern ppatternlist
%type	<p>	plist
%type	<p>	re
%type	<s>	reg_expr
%type	<p>	var varname
%type	<p>	term
%type	<p>	expr
%type	<p>	patlist
%type	<p>	stmtlist
%type	<p>	simple_stmt
%type	<p>	stmt
%type	<p>	arglist
%type	<cp>	funcname
%type	<cp>	arg
%type	<p>	prarg
%type	<p>	while for if
%type	<p>	opt_else opt_simple_stmt
%type	<i>	Print
%type	<i>	st

%right	ASGNOP
%right	'?'
%right	':'
%left	BOR
%left	AND
%left	GETLINE
%nonassoc APPEND EQ GE GT LE LT NE MATCHOP
%left	IN
%left	ARG BLTIN BREAK CALL CLOSE CONTINUE DELETE DO EXIT FOR FUNC
%left	FSPRINTF
%left	GSUB IF INDEX LSUBSTR MATCHFCN NEXT NUMBER
%left	PRINT PRINTF RETURN SPLIT SPRINTF STRING SUB SUBSTR
%left	REGEXPR VAR VARNF IVAR WHILE '('
%left	CAT
%left	'+' '-'
%left	'*' '/' '%'
%left	NOT UMINUS
%left	'^'
%left	DECR INCR
%left	INDIRECT

%%

program:
	  pas	{ if (errorflag==0)
			winner = (Node *)stat3(PROGRAM, beginloc, $1, endloc); }
	| error	{ yyclearin; bracecheck(); SYNTAX("bailing out"); }
	;

pas:
	  pa_stat	{ $$ = $1; }
	| pa_stats	{ $$ = $1; }
	;

pa_stats:
	  pa_stat	{ $$ = $1; }
	| pa_stats pa_stat	{ $$ = linkum($1, $2); }
	;

pa_stat:
	  pattern		{ $$ = stat2(PASTAT, $1, stat2(PRINT, rectonode(), NIL)); }
	| pattern lbrace stmtlist '}'
				{ $$ = stat2(PASTAT, $1, $3); }
	| lbrace stmtlist '}'
				{ $$ = stat2(PASTAT, NIL, $2); }
	| XBEGIN lbrace stmtlist '}'
				{ beginloc = linkum(beginloc, $3); $$ = 0; }
	| XEND lbrace stmtlist '}'
				{ endloc = linkum(endloc, $3); $$ = 0; }
	| FUNC funcname '(' arglist ')' lbrace stmtlist '}'
				{ infunc--; curfname=0; defn($2, $4, $7); $$ = 0; }
	;

funcname:
	  VAR			{ $$ = $1; }
	;

arglist:
	  /* empty */		{ arglist = $$ = 0; }
	| arg			{ arglist = $$ = celltonode($1, CVAR); }
	| arglist ',' arg	{
					checkdup($1, $3);
					arglist = $$ = linkum($1, celltonode($3, CVAR)); }
	;

arg:
	  VAR			{ $$ = $1; }
	| VARNF			{ $$ = $1; }
	;

pattern:
	  patternlist_nl	{ $$ = $1; }
	;

patternlist:
	  patternlist ',' ppattern	{ $$ = op2(BOR, notnull($1), notnull($3)); }
	| ppatternlist			{ $$ = $1; }
	;

patternlist_nl:
	  ppatternlist
	| patternlist_nl NL ppatternlist	{ $$ = op2(BOR, notnull($1), notnull($3)); }
	;

ppatternlist:
	  ppattern
	| ppatternlist ',' ppattern	{ $$ = op2(BOR, notnull($1), notnull($3)); }
	;

ppattern:
	  re			{ $$ = $1; }
	| ppattern MATCHOP reg_expr	{ $$ = op3($2, NIL, $1, (Node*)makedfa($3, 0)); }
	| ppattern MATCHOP re	{ if (constnode($3))
					$$ = op3($2, NIL, $1, (Node*)makedfa(strnode($3), 0));
				  else
					$$ = op3($2, (Node *)1, $1, $3); }
	| ppattern IN varname	{ $$ = op2(INTEST, $1, makearr($3)); }
	| '(' plist ')' IN varname	{ $$ = op2(INTEST, $2, makearr($5)); }
	| ppattern AND ppattern		{ $$ = op2(AND, notnull($1), notnull($3)); }
	| ppattern BOR ppattern		{ $$ = op2(BOR, notnull($1), notnull($3)); }
	| NOT ppattern		{ $$ = op1(NOT, notnull($2)); }
	;

plist:
	  patternlist_nl	{ $$ = $1; }
	| plist NL patternlist_nl { $$ = linkum($1, $3); }
	;

re:
	  reg_expr		{ $$ = op3(MATCH, NIL, rectonode(), (Node*)makedfa($1, 0)); }
	| '!' re		{ $$ = op1(NOT, notnull($2)); }
	| re MATCHOP reg_expr	{ $$ = op3($2, NIL, $1, (Node*)makedfa($3, 0)); }
	| re MATCHOP re		{ if (constnode($3))
					$$ = op3($2, NIL, $1, (Node*)makedfa(strnode($3), 0));
				  else
					$$ = op3($2, (Node *)1, $1, $3); }
	| term			{ $$ = $1; }
	| re re %prec CAT	{ $$ = op2(CAT, $1, $2); }
	;

reg_expr:
	  REGEXPR		{ $$ = $1; }
	;

var:
	  varname		{ $$ = $1; }
	| ARG			{ $$ = op1(ARG, itonp($1)); }
	| VARNF			{ $$ = op1(VARNF, (Node *) $1); }
	| var '[' patlist ']'	{ $$ = op2(ARRAY, makearr($1), $3); }
	| var '(' patlist ')'	{ $$ = op2(CALL, $1, $3); }
	| var '[' ']'		{ $$ = op2(ARRAY, makearr($1), NIL); }
	| var '(' ')'		{ $$ = op2(CALL, $1, NIL); }
	;

varname:
	  VAR			{ $$ = celltonode($1, CVAR); }
	| varname '[' patlist ']'	{ $$ = op2(ARRAY, makearr($1), $3); }
	| varname '(' patlist ')'	{ $$ = op2(CALL, $1, $3); }
	| varname '[' ']'	{ $$ = op2(ARRAY, makearr($1), NIL); }
	| varname '(' ')'	{ $$ = op2(CALL, $1, NIL); }
	;

term:
	  re			{ $$ = $1; }
	| term '+' term		{ $$ = op2(ADD, $1, $3); }
	| term '-' term		{ $$ = op2(MINUS, $1, $3); }
	| term '*' term		{ $$ = op2(MULT, $1, $3); }
	| term '/' term		{ $$ = op2(DIVIDE, $1, $3); }
	| term '%' term		{ $$ = op2(MOD, $1, $3); }
	| term POWER term	{ $$ = op2(POWER, $1, $3); }
	| '-' term %prec UMINUS	{ $$ = op1(UMINUS, $2); }
	| '+' term %prec UMINUS	{ $$ = $2; }
	| NOT term		{ $$ = op1(NOT, notnull($2)); }
	| BLTIN '(' ')'		{ $$ = op2(BLTIN, itonp($1), rectonode()); }
	| BLTIN '(' patlist ')'	{ $$ = op2(BLTIN, itonp($1), $3); }
	| BLTIN			{ $$ = op2(BLTIN, itonp($1), rectonode()); }
	| CALL '(' ')'		{ $$ = op2(CALL, celltonode($1,CVAR), NIL); }
	| CALL '(' patlist ')'	{ $$ = op2(CALL, celltonode($1,CVAR), $3); }
	| CLOSE term		{ $$ = op1(CLOSE, $2); }
	| DECR var		{ $$ = op1(PREDECR, $2); }
	| INCR var		{ $$ = op1(PREINCR, $2); }
	| var DECR		{ $$ = op1(POSTDECR, $1); }
	| var INCR		{ $$ = op1(POSTINCR, $1); }
	| GETLINE var LT term	{ $$ = op3(GETLINE, $2, itonp($3), $4); }
	| GETLINE LT term	{ $$ = op3(GETLINE, NIL, itonp($2), $3); }
	| GETLINE var		{ $$ = op3(GETLINE, $2, NIL, NIL); }
	| GETLINE		{ $$ = op3(GETLINE, NIL, NIL, NIL); }
	| INDEX '(' patternlist_nl ',' patternlist_nl ')'
				{ $$ = op2(INDEX, $3, $5); }
	| INDEX '(' patternlist_nl ',' reg_expr ')'
				{ SYNTAX("index() doesn't permit regular expressions");
				  $$ = op2(INDEX, $3, (Node*)$5); }
	| '(' patternlist_nl ')'	{ $$ = $2; }
	| MATCHFCN '(' patternlist_nl ',' reg_expr ')'
				{ $$ = op3(MATCHFCN, NIL, $3, (Node*)makedfa($5, 1)); }
	| MATCHFCN '(' patternlist_nl ',' re ')'
				{ if (constnode($5))
					$$ = op3(MATCHFCN, NIL, $3, (Node*)makedfa(strnode($5), 1));
				  else
					$$ = op3(MATCHFCN, (Node *)1, $3, $5); }
	| NUMBER		{ $$ = celltonode($1, CCON); }
	| SPLIT '(' patternlist_nl ',' varname ',' reg_expr ')'
				{ $$ = op4(SPLIT, $3, makearr($5), (Node*)makedfa($7, 1), (Node *)REGEXPR); }
	| SPLIT '(' patternlist_nl ',' varname ',' STRING ')'
				{ $$ = op4(SPLIT, $3, makearr($5), (Node*)$7, (Node*)STRING); }
	| SPLIT '(' patternlist_nl ',' varname ')'
				{ $$ = op4(SPLIT, $3, makearr($5), NIL, (Node*)STRING); }
	| SPRINTF '(' patlist ')'	{ $$ = op1($1, $3); }
	| STRING		{ $$ = celltonode($1, CCON); }
	| SUBSTR '(' patternlist_nl ',' patternlist_nl ',' patternlist_nl ')'
				{ $$ = op3(SUBSTR, $3, $5, $7); }
	| SUBSTR '(' patternlist_nl ',' patternlist_nl ')'
				{ $$ = op3(SUBSTR, $3, $5, NIL); }
	| var			{ $$ = $1; }
	;

patlist:
	  patternlist		{ $$ = $1; }
	| patlist ',' patternlist	{ $$ = linkum($1, $3); }
	| patlist NL patternlist	{ $$ = linkum($1, $3); }
	;

stmtlist:
	  /* empty */		{ $$ = 0; }
	| stmtlist stmt		{ $$ = linkum($1, $2); }
	;

simple_stmt:
	  Print prarg '|' term	{
				if (safe) SYNTAX("print | is unsafe");
				else $$ = stat3($1, $2, itonp($<i>3), $4); }
	| Print prarg APPEND term	{
				if (safe) SYNTAX("print >> is unsafe");
				else $$ = stat3($1, $2, itonp($3), $4); }
	| Print prarg GT term	{
				if (safe) SYNTAX("print > is unsafe");
				else $$ = stat3($1, $2, itonp($3), $4); }
	| Print prarg		{ $$ = stat3($1, $2, NIL, NIL); }
	| DELETE varname '[' patlist ']' { $$ = stat2(DELETE, makearr($2), $4); }
	| DELETE varname	{ $$ = stat2(DELETE, makearr($2), 0); }
	| expr			{ $$ = exptostat($1); }
	| error			{ yyclearin; SYNTAX("illegal statement"); }
	;

st:
	  NL		{ $$ = 0; }
	| ';'		{ $$ = 0; }
	;

stmt:
	  BREAK		{ if (!inloop) SYNTAX("break illegal outside of loops");
			  $$ = stat1(BREAK, NIL); }
	| CONTINUE	{ if (!inloop) SYNTAX("continue illegal outside of loops");
			  $$ = stat1(CONTINUE, NIL); }
	| do stmt WHILE '(' patternlist_nl ')' st
			{ $$ = stat2(DO, $2, notnull($5)); }
	| EXIT patternlist_nl st	{ $$ = stat1(EXIT, $2); }
	| EXIT st	{ $$ = stat1(EXIT, NIL); }
	| for st	{ $$ = $1; }
	| if stmt opt_else	{ $$ = stat3(IF, $1, $2, $3); }
	| lbrace stmtlist '}'	{ $$ = $2; }
	| NEXT st	{ if (infunc)
				SYNTAX("next is illegal inside a function");
			  $$ = stat1(NEXT, NIL); }
	| NEXTFILE st	{ if (infunc)
				SYNTAX("nextfile is illegal inside a function");
			  $$ = stat1(NEXTFILE, NIL); }
	| RETURN patternlist_nl st	{ $$ = stat1(RETURN, $2); }
	| RETURN st	{ $$ = stat1(RETURN, NIL); }
	| simple_stmt st	{ $$ = $1; }
	| while st	{ $$ = $1; }
	| ';' NL	{ $$ = 0; }
	;

lbrace:
	  '{'		{ }
	;

opt_else:
	  /* empty */	{ $$ = 0; }
	| ELSE stmt	{ $$ = $2; }
	;

while:
	  WHILE '(' patternlist_nl ')' stmt {
				--inloop; $$ = stat2(WHILE, $3, $5); }
	;

do:
	  DO	{ inloop++; }
	;

for:
	  FOR '(' opt_simple_stmt ';' patternlist ';' opt_simple_stmt ')' stmt {
				--inloop; $$ = stat4(FOR, $3, notnull($5), $7, $9); }
	| FOR '(' opt_simple_stmt ';' ';' opt_simple_stmt ')' stmt {
				--inloop; $$ = stat4(FOR, $3, NIL, $6, $8); }
	| FOR '(' varname IN varname ')' stmt {
				--inloop; $$ = stat3(IN, $3, makearr($5), $7); }
	;

opt_simple_stmt:
	  /* empty */	{ $$ = 0; }
	| simple_stmt	{ $$ = $1; }
	;

if:
	  IF '(' patternlist_nl ')' { $$ = notnull($3); }
	;

expr:
	  re			{ $$ = $1; }
	| term			{ $$ = $1; }
	| expr '<' expr		{ $$ = op2(LT, $1, $3); }
	| expr LE expr		{ $$ = op2(LE, $1, $3); }
	| expr NE expr		{ $$ = op2(NE, $1, $3); }
	| expr EQ expr		{ $$ = op2(EQ, $1, $3); }
	| expr GE expr		{ $$ = op2(GE, $1, $3); }
	| expr GT expr		{ $$ = op2(GT, $1, $3); }
	| expr AND expr		{ $$ = op2(AND, notnull($1), notnull($3)); }
	| expr BOR expr		{ $$ = op2(BOR, notnull($1), notnull($3)); }
	| expr '?' expr ':' expr	{ $$ = op3(CONDEXPR, notnull($1), $3, $5); }
	| expr IN varname	{ $$ = op2(INTEST, $1, makearr($3)); }
	| '(' plist ')' IN varname	{ $$ = op2(INTEST, $2, makearr($5)); }
	| term ASSIGN expr	{ $$ = op2(ASSIGN, $1, $3); }
	| term POWEQ expr	{ $$ = op2(ASSIGN, $1, op2(POWER, $1, $3)); }
	| term MODEQ expr	{ $$ = op2(ASSIGN, $1, op2(MOD, $1, $3)); }
	| term MULTEQ expr	{ $$ = op2(ASSIGN, $1, op2(MULT, $1, $3)); }
	| term DIVEQ expr	{ $$ = op2(ASSIGN, $1, op2(DIVIDE, $1, $3)); }
	| term ADDEQ expr	{ $$ = op2(ASSIGN, $1, op2(ADD, $1, $3)); }
	| term SUBEQ expr	{ $$ = op2(ASSIGN, $1, op2(MINUS, $1, $3)); }
	;

prarg:
	  /* empty */	{ $$ = 0; }
	| patternlist	{ $$ = $1; }
	;

Print:
	  PRINT		{ $$ = $1; }
	| PRINTF	{ $$ = $1; }
	;

%%

/*!
 *  @brief Records the name of the function currently being defined.
 *
 *  @param[in] p Cell of the function name.
 */
void setfname(Cell *p)
{
	if (isarr(p))
		SYNTAX("%s is an array, not a function", p->nval);
	else if (isfcn(p))
		SYNTAX("you can't define function %s more than once", p->nval);
	curfname = p->nval;
}

/*!
 *  @brief Tests whether a node is a constant value.
 *
 *  @param[in] p Node to test.
 *
 *  @return Non-zero if the node is a constant.
 */
int constnode(Node *p)
{
	return isvalue(p) && ((Cell *) (p->narg[0]))->csub == CCON;
}

/*!
 *  @brief Returns the string of a string-literal node.
 *
 *  @param[in] p Node to inspect.
 *
 *  @return Pointer to the string value.
 */
char *strnode(Node *p)
{
	return ((Cell *)(p->narg[0]))->sval;
}

/*!
 *  @brief Wraps a node so that it is guaranteed non-null.
 *
 *  @param[in] n Node to wrap.
 *
 *  @return Node suitable for a boolean context.
 */
Node *notnull(Node *n)
{
	switch (n->nobj) {
	case LE: case LT: case EQ: case NE: case GT: case GE:
	case BOR: case AND: case NOT:
		return n;
	default:
		return op2(NE, n, nullnode);
	}
}

/*!
 *  @brief Checks whether a name is already present in an argument list.
 *
 *  @param[in] vl Argument list head.
 *  @param[in] cp Cell of the name to check.
 */
void checkdup(Node *vl, Cell *cp)
{
	char *s = cp->nval;
	for ( ; vl; vl = vl->nnext) {
		if (strcmp(s, ((Cell *)(vl->narg[0]))->nval) == 0) {
			SYNTAX("duplicate argument %s", s);
			break;
		}
	}
}
