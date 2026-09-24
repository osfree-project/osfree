%{
/****************************************************************
 * Copyright (C) Lucent Technologies 1997
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name Lucent Technologies or any of
 * its entities not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.
 *
 * LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 ****************************************************************/

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

Node    *beginloc = 0;   /*!< Node list of BEGIN actions. */
Node    *endloc = 0;     /*!< Node list of END actions. */
int     infunc  = 0;     /*!< 1 while parsing the argument list or body of a function. */
int     inloop  = 0;     /*!< 1 while inside a while, for or do loop. */
char    *curfname = 0;   /*!< Name of the function currently being parsed. */
Node    *arglist = 0;    /*!< Argument list of the function currently being parsed. */

%}

/*!
 *  @brief Semantic value union passed between the lexer and the parser.
 */
%union  {
        Node    *p;   /*!< Node pointer. */
        Cell    *cp;  /*!< Cell pointer. */
        int     i;    /*!< Integer value. */
        char    *s;   /*!< String value. */
}

%token  <i>     FIRSTTOKEN

%token  <i>     PROGRAM         /*!< PROGRAM pseudo-token produced by yyparse. */
%token  <i>     PASTAT          /*!< pattern { action } statement. */
%token  <i>     PASTAT2         /*!< pattern, pattern { action } statement. */
%token  <i>     XBEGIN          /*!< BEGIN keyword. */
%token  <i>     XEND            /*!< END keyword. */
%token  <i>     NL              /*!< Newline token. */
%token  <i>     ARRAY           /*!< Array subscript expression. */
%token  <i>     MATCH           /*!< ~ operator. */
%token  <i>     NOTMATCH        /*!< !~ operator. */
%token  <i>     MATCHOP         /*!< Common ~ / !~ operator class. */
%token  <i>     FINAL           /*!< End-of-regex position marker. */
%token  <i>     DOT             /*!< . in a regular expression. */
%token  <i>     ALL             /*!< .* in a regular expression. */
%token  <i>     CCL             /*!< Character class. */
%token  <i>     NCCL            /*!< Negated character class. */
%token  <i>     CHAR            /*!< Single character in a regular expression. */
%token  <i>     OR              /*!< | in a regular expression. */
%token  <i>     STAR            /*!< * in a regular expression. */
%token  <i>     QUEST           /*!< ? in a regular expression. */
%token  <i>     PLUS            /*!< + in a regular expression. */
%token  <i>     AND             /*!< && operator. */
%token  <i>     BOR             /*!< || operator. */
%token  <i>     APPEND          /*!< >> redirection. */
%token  <i>     EQ              /*!< == operator. */
%token  <i>     GE              /*!< >= operator. */
%token  <i>     GT              /*!< > operator. */
%token  <i>     LE              /*!< <= operator. */
%token  <i>     LT              /*!< < operator. */
%token  <i>     NE              /*!< != operator. */
%token  <i>     IN              /*!< in operator. */
%token  <i>     ARG             /*!< Function argument reference. */
%token  <i>     BLTIN           /*!< Built-in function. */
%token  <i>     BREAK           /*!< break statement. */
%token  <i>     CLOSE           /*!< close() call. */
%token  <i>     CONTINUE        /*!< continue statement. */
%token  <i>     DELETE          /*!< delete statement. */
%token  <i>     DO              /*!< do keyword. */
%token  <i>     EXIT            /*!< exit statement. */
%token  <i>     FOR             /*!< for keyword. */
%token  <i>     FUNC            /*!< function keyword. */
%token  <i>     SUB             /*!< sub() call. */
%token  <i>     GSUB            /*!< gsub() call. */
%token  <i>     IF              /*!< if keyword. */
%token  <i>     INDEX           /*!< index() call. */
%token  <i>     LSUBSTR         /*!< Left-hand side of substr(). */
%token  <i>     MATCHFCN        /*!< match() call. */
%token  <i>     NEXT            /*!< next statement. */
%token  <i>     NEXTFILE        /*!< nextfile statement. */
%token  <i>     ADD             /*!< + operator. */
%token  <i>     MINUS           /*!< - operator. */
%token  <i>     MULT            /*!< * operator. */
%token  <i>     DIVIDE          /*!< / operator. */
%token  <i>     MOD             /*!< % operator. */
%token  <i>     ASSIGN          /*!< = operator. */
%token  <i>     ASGNOP          /*!< Common assignment operator class. */
%token  <i>     ADDEQ           /*!< += operator. */
%token  <i>     SUBEQ           /*!< -= operator. */
%token  <i>     MULTEQ          /*!< *= operator. */
%token  <i>     DIVEQ           /*!< /= operator. */
%token  <i>     MODEQ           /*!< %= operator. */
%token  <i>     POWEQ           /*!< ^= operator. */
%token  <i>     PRINT           /*!< print statement. */
%token  <i>     PRINTF          /*!< printf statement. */
%token  <i>     SPRINTF         /*!< sprintf() call. */
%token  <i>     ELSE            /*!< else keyword. */
%token  <i>     INTEST          /*!< in-test operator class. */
%token  <i>     CONDEXPR        /*!< ?: operator. */
%token  <i>     POSTINCR        /*!< x++ operator. */
%token  <i>     PREINCR         /*!< ++x operator. */
%token  <i>     POSTDECR        /*!< x-- operator. */
%token  <i>     PREDECR         /*!< --x operator. */
%token  <i>     VAR             /*!< Variable name. */
%token  <i>     IVAR            /*!< Indirect variable $var. */
%token  <i>     VARNF           /*!< NF pseudo-variable. */
%token  <i>     CALL            /*!< Function call. */
%token  <i>     NUMBER          /*!< Numeric literal. */
%token  <i>     STRING          /*!< String literal. */
%token  <i>     REGEXPR         /*!< /re/ literal. */
%token  <i>     GETLINE         /*!< getline. */
%token  <i>     RETURN          /*!< return statement. */
%token  <i>     SPLIT           /*!< split() call. */
%token  <i>     SUBSTR          /*!< substr() call. */
%token  <i>     WHILE           /*!< while keyword. */
%token  <i>     CAT             /*!< String concatenation. */
%token  <i>     NOT             /*!< ! operator. */
%token  <i>     UMINUS          /*!< Unary minus. */
%token  <i>     POWER           /*!< ^ operator. */
%token  <i>     DECR            /*!< -- operator class. */
%token  <i>     INCR            /*!< ++ operator class. */
%token  <i>     INDIRECT        /*!< $(expr) operator. */
%token  <i>     LASTTOKEN       /*!< Last token of the grammar. */

%type   <p>     pattern patternlist patternlist_nl ppattern ppatternlist
%type   <p>     pa_pat pa_stat pa_stats
%type   <p>     simple_stmt stmt stmtlist stmtlist_nl
%type   <p>     re expr term
%type   <p>     var varname
%type   <p>     prarg
%type   <p>     opt_else
%type   <p>     program
%type   <p>     funcname
%type   <p>     arglist arg
%type   <i>     opt_exp opt_pat psimple_stmt

%right  ASGNOP
%right  '?'
%right  ':'
%left   BOR
%left   AND
%left   GETLINE
%nonassoc APPEND EQ GE GT LE LT NE MATCHOP
%left   IN
%left   ARG BLTIN BREAK CALL CLOSE CONTINUE DELETE DO EXIT FOR FUNC
%left   FSPRINTF
%left   GSUB IF INDEX LSUBSTR MATCHFCN NEXT NUMBER
%left   PRINT PRINTF RETURN SPLIT SPRINTF STRING SUB SUBSTR
%left   REGEXPR VAR VARNF IVAR WHILE '('
%left   CAT
%left   '+' '-'
%left   '*' '/' '%'
%left   NOT UMINUS
%left   '^'
%left   DECR INCR
%left   INDIRECT
%token  LASTTOKEN       57346

%%

/*!
 *  @brief Top-level program: sequence of pattern-action statements.
 */
program:
          pas      { if (errorflag==0)
                        winner = (Node *)stat3(PROGRAM, beginloc, $1, endloc); }
        | error         { yyclearin; bracecheck(); SYNTAX("bailing out"); }
        ;

pas:
          pa_stats
        | pa_stats term
        ;

pa_stats:
          pa_stat
        | pa_stats term pa_stat        { $$ = linkum($1, $3); }
        ;

pa_stat:
          pattern          { $$ = stat2(PASTAT, $1, stat2(PRINT, rectonode(), NIL)); }
        | pattern '{' stmtlist '}'     { $$ = stat2(PASTAT, $1, $3); }
        | '{' stmtlist '}'             { $$ = stat2(PASTAT, NIL, $2); }
        | XBEGIN '{' stmtlist '}'      { beginloc = linkum(beginloc, $3); $$ = 0; }
        | XEND '{' stmtlist '}'        { endloc = linkum(endloc, $3); $$ = 0; }
        | FUNC funcname '(' arglist ')' '{' stmtlist '}'
                { infunc--; curfname=0; defn((Cell *)$2, $4, $7); $$ = 0; }
        ;

funcname:
          VAR           { $$ = celltonode($1, CVAR); }
        ;

arglist:
          /* empty */   { arglist = $$ = 0; }
        | arg           { arglist = $$ = $1; }
        | arglist ',' arg       {
                        checkdup($1, $3);
                        arglist = $$ = linkum($1, celltonode($3,CVAR)); }
        ;

arg:
          VAR           { $$ = celltonode($1, CVAR); }
        | ARG           { $$ = op1(ARG, itonp($1)); }
        | VARNF         { $$ = op1(VARNF, (Node *) $1); }
        ;

pattern:
          ppatternlist
        ;

patternlist:
          ppattern
        | patternlist ',' ppattern     { $$ = op2(BOR, notnull($1), notnull($3)); }
        ;

patternlist_nl:
          ppatternlist
        | patternlist_nl NL ppatternlist { $$ = op2(BOR, notnull($1), notnull($3)); }
        ;

ppatternlist:
          ppattern
        | ppatternlist ',' ppattern    { $$ = op2(BOR, notnull($1), notnull($3)); }
        ;

ppattern:
          re
        | ppattern MATCHOP re          { if (constnode($3))
                                            $$ = op3($2, NIL, $1, (Node*)makedfa(strnode($3), 0));
                                          else
                                            $$ = op3($2, (Node *)1, $1, $3); }
        | ppattern IN varname          { $$ = op2(INTEST, $1, makearr($3)); }
        | '(' plist ')' IN varname     { $$ = op2(INTEST, $2, makearr($5)); }
        | ppattern AND ppattern        { $$ = op2(AND, notnull($1), notnull($3)); }
        | ppattern BOR ppattern        { $$ = op2(BOR, notnull($1), notnull($3)); }
        | NOT ppattern                 { $$ = op1(NOT, notnull($2)); }
        ;

plist:
          patternlist_nl
        | plist NL patternlist_nl      { $$ = linkum($1, $3); }
        ;

re:
          reg_expr
        | '!' re                       { $$ = op1(NOT, notnull($2)); }
        | re MATCHOP reg_expr          { if (constnode($3))
                                            $$ = op3($2, NIL, $1, (Node*)makedfa(strnode($3), 0));
                                          else
                                            $$ = op3($2, (Node *)1, $1, $3); }
        | term
        | re re %prec CAT              { $$ = op2(CAT, $1, $2); }
        ;

reg_expr:
          REGEXPR       { $$ = op3(MATCH, NIL, rectonode(), (Node*)makedfa($1, 0)); }
        ;

var:
          varname
        | ARG           { $$ = op1(ARG, itonp($1)); }
        | VARNF         { $$ = op1(VARNF, (Node *) $1); }
        | var '[' patlist ']'  { $$ = op2(ARRAY, makearr($1), $3); }
        | var '(' patlist ')'  { $$ = op2(CALL, celltonode($1,CVAR), $3); }
        | var '[' ']'          { $$ = op2(ARRAY, makearr($1), NIL); }
        | var '(' ')'          { $$ = op2(CALL, celltonode($1,CVAR), NIL); }
        ;

varname:
          VAR           { $$ = celltonode($1, CVAR); }
        | varname '[' patlist ']' { $$ = op2(ARRAY, makearr($1), $3); }
        | varname '(' patlist ')' { $$ = op2(CALL, celltonode($1,CVAR), $3); }
        | varname '[' ']'      { $$ = op2(ARRAY, makearr($1), NIL); }
        | varname '(' ')'      { $$ = op2(CALL, celltonode($1,CVAR), NIL); }
        ;

term:
          re
        | term '+' term        { $$ = op2(ADD, $1, $3); }
        | term '-' term        { $$ = op2(MINUS, $1, $3); }
        | term '*' term        { $$ = op2(MULT, $1, $3); }
        | term '/' term        { $$ = op2(DIVIDE, $1, $3); }
        | term '%' term        { $$ = op2(MOD, $1, $3); }
        | term POWER term      { $$ = op2(POWER, $1, $3); }
        | '-' term %prec UMINUS { $$ = op1(UMINUS, $2); }
        | '+' term %prec UMINUS { $$ = $2; }
        | NOT term %prec UNARY { $$ = op1(NOT, notnull($2)); }
        | BLTIN '(' ')'        { $$ = op2(BLTIN, itonp($1), rectonode()); }
        | BLTIN '(' patlist ')' { $$ = op2(BLTIN, itonp($1), $3); }
        | BLTIN                { $$ = op2(BLTIN, itonp($1), rectonode()); }
        | CALL '(' ')'         { $$ = op2(CALL, celltonode($1,CVAR), NIL); }
        | CALL '(' patlist ')' { $$ = op2(CALL, celltonode($1,CVAR), $3); }
        | CLOSE term           { $$ = op1(CLOSE, $2); }
        | DECR var             { $$ = op1(PREDECR, $2); }
        | INCR var             { $$ = op1(PREINCR, $2); }
        | var DECR             { $$ = op1(POSTDECR, $1); }
        | var INCR             { $$ = op1(POSTINCR, $1); }
        | GETLINE var LT term  { $$ = op3(GETLINE, $2, itonp($3), $4); }
        | GETLINE LT term      { $$ = op3(GETLINE, NIL, itonp($2), $3); }
        | GETLINE var          { $$ = op3(GETLINE, $2, NIL, NIL); }
        | GETLINE              { $$ = op3(GETLINE, NIL, NIL, NIL); }
        | INDEX '(' patternlist_nl ',' patternlist_nl ')' {
                                $$ = op2(INDEX, $3, $5); }
        | INDEX '(' patternlist_nl ',' reg_expr ')' {
                                SYNTAX("index() doesn't permit regular expressions");
                                $$ = op2(INDEX, $3, (Node*)$5); }
        | '(' patternlist_nl ')' { $$ = $2; }
        | MATCHFCN '(' patternlist_nl ',' reg_expr ')' {
                                $$ = op3(MATCHFCN, NIL, $3, (Node*)makedfa($5, 1)); }
        | MATCHFCN '(' patternlist_nl ',' re ')' {
                                if (constnode($5))
                                        $$ = op3(MATCHFCN, NIL, $3, (Node*)makedfa(strnode($5), 1));
                                else
                                        $$ = op3(MATCHFCN, (Node *)1, $3, $5); }
        | NUMBER               { $$ = celltonode($1, CCON); }
        | SPLIT '(' patternlist_nl ',' varname ',' reg_expr ')' {
                                $$ = op4(SPLIT, $3, makearr($5), (Node*)makedfa($7, 1), (Node *)REGEXPR); }
        | SPLIT '(' patternlist_nl ',' varname ',' STRING ')' {
                                $$ = op4(SPLIT, $3, makearr($5), $7, (Node*)STRING); }
        | SPLIT '(' patternlist_nl ',' varname ')' {
                                $$ = op4(SPLIT, $3, makearr($5), NIL, (Node*)STRING); }
        | SPRINTF '(' patlist ')' { $$ = op1($1, $3); }
        | STRING               { $$ = celltonode($1, CCON); }
        | SUBSTR '(' patternlist_nl ',' patternlist_nl ',' patternlist_nl ')' {
                                $$ = op3(SUBSTR, $3, $5, $7); }
        | SUBSTR '(' patternlist_nl ',' patternlist_nl ')' {
                                $$ = op3(SUBSTR, $3, $5, NIL); }
        | var
        ;

patlist:
          patternlist
        | patlist ',' patternlist { $$ = linkum($1, $3); }
        | patlist NL patternlist { $$ = linkum($1, $3); }
        ;

stmtlist:
          /* empty */          { $$ = 0; }
        | stmtlist term
        | stmtlist stmt        { $$ = linkum($1, $2); }
        ;

stmtlist_nl:
          /* empty */          { $$ = 0; }
        | stmtlist_nl NL
        | stmtlist_nl stmt     { $$ = linkum($1, $2); }
        ;

simple_stmt:
          Print prarg '|' term {
                        if (safe) SYNTAX("print | is unsafe");
                        else $$ = stat3($1, $2, itonp($3), $4); }
        | Print prarg APPEND term {
                        if (safe) SYNTAX("print >> is unsafe");
                        else $$ = stat3($1, $2, itonp($3), $4); }
        | Print prarg GT term {
                        if (safe) SYNTAX("print > is unsafe");
                        else $$ = stat3($1, $2, itonp($3), $4); }
        | Print prarg          { $$ = stat3($1, $2, NIL, NIL); }
        | DELETE varname '[' patlist ']' { $$ = stat2(DELETE, makearr($2), $4); }
        | DELETE varname       { $$ = stat2(DELETE, makearr($2), 0); }
        | expr                 { $$ = exptostat($1); }
        | error                { yyclearin; SYNTAX("illegal statement"); }
        ;

st:
          NL
        | ';'
        ;

stmt:
          BREAK                { if (!inloop) SYNTAX("break illegal outside of loops");
                                $$ = stat1(BREAK, NIL); }
        | CONTINUE             { if (!inloop) SYNTAX("continue illegal outside of loops");
                                $$ = stat1(CONTINUE, NIL); }
        | do stmt WHILE '(' patternlist_nl ')' st
                               { $$ = stat2(DO, $2, notnull($5)); }
        | EXIT patternlist_nl st { $$ = stat1(EXIT, $2); }
        | EXIT st              { $$ = stat1(EXIT, NIL); }
        | for st               { $$ = $1; }
        | if stmt opt_else     { $$ = stat3(IF, $1, $2, $3); }
        | lbrace stmtlist '}'  { $$ = $2; }
        | NEXT st              { if (infunc)
                                        SYNTAX("next is illegal inside a function");
                                $$ = stat1(NEXT, NIL); }
        | NEXTFILE st          { if (infunc)
                                        SYNTAX("nextfile is illegal inside a function");
                                $$ = stat1(NEXTFILE, NIL); }
        | RETURN patternlist_nl st { $$ = stat1(RETURN, $2); }
        | RETURN st            { $$ = stat1(RETURN, NIL); }
        | simple_stmt st       { $$ = $1; }
        | while st             { $$ = $1; }
        | ';' NL               { $$ = 0; }
        ;

lbrace:
          '{'  { }
        ;

opt_else:
          /* empty */          { $$ = 0; }
        | ELSE stmt            { $$ = $2; }
        ;

while:
          WHILE '(' patternlist_nl ')' stmt {
                        --inloop; $$ = stat2(WHILE, $3, $5); }
        ;

do:
          DO  { inloop++; }
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
          /* empty */          { $$ = 0; }
        | simple_stmt          { $$ = $1; }
        ;

if:
          IF '(' patternlist_nl ')' { $$ = notnull($3); }
        ;

expr:
          re
        | term
        | expr '<' expr        { $$ = op2(LT, $1, $3); }
        | expr LE expr         { $$ = op2(LE, $1, $3); }
        | expr NE expr         { $$ = op2(NE, $1, $3); }
        | expr EQ expr         { $$ = op2(EQ, $1, $3); }
        | expr GE expr         { $$ = op2(GE, $1, $3); }
        | expr GT expr         { $$ = op2(GT, $1, $3); }
        | expr AND expr        { $$ = op2(AND, notnull($1), notnull($3)); }
        | expr BOR expr        { $$ = op2(BOR, notnull($1), notnull($3)); }
        | expr '?' expr ':' expr { $$ = op3(CONDEXPR, notnull($1), $3, $5); }
        | expr IN varname      { $$ = op2(INTEST, $1, makearr($3)); }
        | '(' plist ')' IN varname { $$ = op2(INTEST, $2, makearr($5)); }
        | term ASSIGN expr     { $$ = op2(ASSIGN, $1, $3); }
        | term POWEQ expr      { $$ = op2(ASSIGN, $1, op2(POWER, $1, $3)); }
        | term MODEQ expr      { $$ = op2(ASSIGN, $1, op2(MOD, $1, $3)); }
        | term MULTEQ expr     { $$ = op2(ASSIGN, $1, op2(MULT, $1, $3)); }
        | term DIVEQ expr      { $$ = op2(ASSIGN, $1, op2(DIVIDE, $1, $3)); }
        | term ADDEQ expr      { $$ = op2(ASSIGN, $1, op2(ADD, $1, $3)); }
        | term SUBEQ expr      { $$ = op2(ASSIGN, $1, op2(MINUS, $1, $3)); }
        ;

prarg:
          /* empty */          { $$ = 0; }
        | patternlist
        ;

Print:
          PRINT
        | PRINTF
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
void checkdup(Node *vl, Cell *cp)       /* check if name already in list */
{
        char *s = cp->nval;
        for ( ; vl; vl = vl->nnext) {
                if (strcmp(s, ((Cell *)(vl->narg[0]))->nval) == 0) {
                        SYNTAX("duplicate argument %s", s);
                        break;
                }
        }
}
