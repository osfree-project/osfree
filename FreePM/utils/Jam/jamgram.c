#ifndef lint
static char const 
yyrcsid[] = "$FreeBSD: src/usr.bin/yacc/skeleton.c,v 1.28 2000/01/17 02:04:06 bde Exp $";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING() (yyerrflag!=0)
static int yygrowstack(void);
#define YYPREFIX "yy"
#line 84 "jamgram.y"
#include "jam.h"

#include "lists.h"
#include "parse.h"
#include "scan.h"
#include "compile.h"
#include "newstr.h"

# define F0 (LIST *(*)(PARSE *, LOL *))0
# define P0 (PARSE *)0
# define S0 (char *)0

# define pappend( l,r )    	parse_make( compile_append,l,r,P0,S0,S0,0 )
# define pfor( s,l,r )    	parse_make( compile_foreach,l,r,P0,s,S0,0 )
# define pif( l,r,t )	  	parse_make( compile_if,l,r,t,S0,S0,0 )
# define pincl( l )       	parse_make( compile_include,l,P0,P0,S0,S0,0 )
# define plist( s )	  	parse_make( compile_list,P0,P0,P0,s,S0,0 )
# define plocal( l,r,t )  	parse_make( compile_local,l,r,t,S0,S0,0 )
# define pnull()	  	parse_make( compile_null,P0,P0,P0,S0,S0,0 )
# define prule( s,p )     	parse_make( compile_rule,p,P0,P0,s,S0,0 )
# define prules( l,r )	  	parse_make( compile_rules,l,r,P0,S0,S0,0 )
# define pset( l,r,a ) 	  	parse_make( compile_set,l,r,P0,S0,S0,a )
# define pset1( l,r,t,a )	parse_make( compile_settings,l,r,t,S0,S0,a )
# define psetc( s,p )     	parse_make( compile_setcomp,p,P0,P0,s,S0,0 )
# define psete( s,l,s1,f ) 	parse_make( compile_setexec,l,P0,P0,s,s1,f )
# define pswitch( l,r )   	parse_make( compile_switch,l,r,P0,S0,S0,0 )

# define pnode( l,r )    	parse_make( F0,l,r,P0,S0,S0,0 )
# define pcnode( c,l,r )	parse_make( F0,l,r,P0,S0,S0,c )
# define psnode( s,l )     	parse_make( F0,l,P0,P0,s,S0,0 )

#line 49 "y.tab.c"
#define YYERRCODE 256
#define _BANG 257
#define _BANG_EQUALS 258
#define _AMPERAMPER 259
#define _LPAREN 260
#define _RPAREN 261
#define _PLUS_EQUALS 262
#define _COLON 263
#define _SEMIC 264
#define _LANGLE 265
#define _LANGLE_EQUALS 266
#define _EQUALS 267
#define _RANGLE 268
#define _RANGLE_EQUALS 269
#define _QUESTION_EQUALS 270
#define _LBRACKET 271
#define _RBRACKET 272
#define ACTIONS 273
#define BIND 274
#define CASE 275
#define DEFAULT 276
#define ELSE 277
#define EXISTING 278
#define FOR 279
#define IF 280
#define IGNORE 281
#define IN 282
#define INCLUDE 283
#define LOCAL 284
#define ON 285
#define PIECEMEAL 286
#define QUIETLY 287
#define RETURN 288
#define RULE 289
#define SWITCH 290
#define TOGETHER 291
#define UPDATED 292
#define _LBRACE 293
#define _BARBAR 294
#define _RBRACE 295
#define ARG 296
#define STRING 297
const short yylhs[] = {                                        -1,
    0,    0,    2,    2,    1,    1,    1,    1,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    3,   12,
   13,    3,    7,    7,    7,    7,    9,    9,    9,    9,
    9,    9,    9,    9,    9,    9,    9,    9,    8,    8,
   14,    5,    5,    4,   15,   15,    6,    6,   10,   10,
   16,   16,   16,   16,   16,   16,   11,   11,
};
const short yylen[] = {                                         2,
    0,    1,    0,    1,    1,    2,    4,    6,    3,    3,
    3,    4,    6,    3,    7,    5,    5,    7,    3,    0,
    0,    9,    1,    1,    1,    2,    1,    3,    3,    3,
    3,    3,    3,    3,    2,    3,    3,    3,    0,    2,
    4,    1,    3,    1,    0,    2,    1,    4,    0,    2,
    1,    1,    1,    1,    1,    1,    0,    2,
};
const short yydefred[] = {                                      0,
    0,   49,    0,    0,   45,   45,   45,    0,   45,    0,
    0,    0,    2,    0,    0,   45,    0,    0,    0,    0,
   47,    0,    0,    0,    0,    0,    0,    0,    0,    4,
    0,    0,    0,    6,   24,   23,   25,    0,   45,   45,
    0,   56,   53,   55,   54,   52,   51,    0,   50,   45,
   35,    0,    0,    0,    0,    0,    0,    0,   45,    0,
    0,    0,   10,   46,    0,   45,   14,   19,    0,    9,
   45,   11,   26,    0,    0,   48,   45,    0,    0,   38,
   29,   30,   31,   28,   32,   33,   34,   36,    0,    0,
    7,    0,    0,    0,    0,   43,   45,   12,   58,   20,
    0,    0,    0,    0,   16,   40,    0,    0,    0,    0,
    8,    0,   13,   21,   15,   18,   41,    0,   22,
};
const short yydgoto[] = {                                      12,
   30,   31,   14,   32,   33,   15,   40,   94,   23,   17,
   78,  108,  118,   95,   25,   49,
};
const short yysindex[] = {                                   -160,
 -273,    0, -271, -236,    0,    0,    0, -270,    0, -160,
    0,    0,    0, -160, -158,    0, -114, -255, -236, -236,
    0, -106, -254, -226, -265, -216, -221, -139, -244,    0,
 -245, -209, -205,    0,    0,    0,    0, -212,    0,    0,
 -202,    0,    0,    0,    0,    0,    0, -194,    0,    0,
    0, -241, -265, -265, -265, -265, -265, -265,    0, -236,
 -160, -236,    0,    0, -160,    0,    0,    0, -190,    0,
    0,    0,    0, -145, -173,    0,    0, -200, -199,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -198, -161,
    0, -164, -193, -189, -190,    0,    0,    0,    0,    0,
 -160, -172, -160, -149,    0,    0, -162, -182, -179, -139,
    0, -160,    0,    0,    0,    0,    0, -169,    0,
};
const short yyrindex[] = {                                    135,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -157,
 -175,    0,    0,    3,    0,    0,    0,    0,    0,    0,
    0, -249,    0,    0, -186,    0,    0,    0,    0,    0,
    0, -242,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -156,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -157,    0,    0,    0,    4,    0,    0,    0, -153,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -247,
    0,    0,    0,    0, -153,    0,    0,    0,    0,    0,
 -157,    1,    4,    0,    0,    0,    0,    0,    0,    0,
    0, -258,    0,    0,    0,    0,    0,    0,    0,
};
const short yygindex[] = {                                      0,
   19,  -29,  -28,    2,  -14,    9,   65,   48,   -4,    0,
    0,    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 299
const short yytable[] = {                                      68,
   17,   41,    5,    3,   60,    1,   24,   26,   27,   27,
   29,   27,   22,   37,   51,   52,    3,   60,   13,   80,
   19,   42,   16,   20,   18,   28,   50,   22,   22,   42,
   21,   89,   34,   64,    1,   91,    3,   63,   61,   62,
   74,   75,   67,   27,   27,   37,   37,   65,   69,   70,
   66,   79,   62,   71,   73,   88,   96,   90,   72,   21,
   87,   81,   82,   83,   84,   85,   86,   92,   22,   76,
   22,  109,   44,  111,   44,   44,   44,   44,   99,   77,
   44,  116,  117,   44,   93,   44,   47,   45,   45,   44,
   98,   47,  100,  101,   47,   45,  102,   60,  107,  103,
   47,  113,  104,   35,  110,  105,   44,   44,   36,   47,
    1,   37,    2,  112,  114,  115,   35,   38,    3,    4,
   45,   36,    5,    6,   37,  119,   39,    7,    8,    9,
   38,    1,   10,    2,    1,   11,   57,    3,   97,    3,
    4,   39,  106,    5,    0,    0,    0,    0,    7,    8,
    9,   53,    0,   10,    0,    0,   11,    0,   54,   55,
   56,   57,   58,   42,    0,    0,   43,    0,    0,    0,
    0,   44,   45,    0,    0,   59,   46,   47,    0,    0,
    0,   48,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   17,    0,   17,    0,   17,    0,    5,    3,   17,
   17,    0,    0,   17,   17,    0,    0,    0,   17,   17,
   17,    0,    0,   17,    0,   17,   17,    5,    3,
};
const short yycheck[] = {                                      28,
    0,   16,    0,    0,  259,  271,    5,    6,    7,  259,
    9,  261,    4,  261,   19,   20,  275,  259,    0,  261,
  257,  264,  296,  260,  296,  296,  282,   19,   20,  272,
  296,   61,   14,   25,  271,   65,  295,  264,  293,  294,
   39,   40,  264,  293,  294,  293,  294,  264,  293,  295,
  267,   50,  294,  263,  267,   60,   71,   62,  264,  296,
   59,   53,   54,   55,   56,   57,   58,   66,   60,  272,
   62,  101,  259,  103,  261,  262,  263,  264,   77,  274,
  267,  110,  112,  270,  275,  272,  262,  263,  264,  276,
  264,  267,  293,  293,  270,  271,  295,  259,   97,  264,
  276,  264,  296,  262,  277,  295,  293,  294,  267,  285,
  271,  270,  273,  263,  297,  295,  262,  276,  279,  280,
  296,  267,  283,  284,  270,  295,  285,  288,  289,  290,
  276,  271,  293,  273,    0,  296,  293,  295,   74,  279,
  280,  295,   95,  283,   -1,   -1,   -1,   -1,  288,  289,
  290,  258,   -1,  293,   -1,   -1,  296,   -1,  265,  266,
  267,  268,  269,  278,   -1,   -1,  281,   -1,   -1,   -1,
   -1,  286,  287,   -1,   -1,  282,  291,  292,   -1,   -1,
   -1,  296,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  271,   -1,  273,   -1,  275,   -1,  275,  275,  279,
  280,   -1,   -1,  283,  284,   -1,   -1,   -1,  288,  289,
  290,   -1,   -1,  293,   -1,  295,  296,  295,  295,
};
#define YYFINAL 12
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 297
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"_BANG","_BANG_EQUALS",
"_AMPERAMPER","_LPAREN","_RPAREN","_PLUS_EQUALS","_COLON","_SEMIC","_LANGLE",
"_LANGLE_EQUALS","_EQUALS","_RANGLE","_RANGLE_EQUALS","_QUESTION_EQUALS",
"_LBRACKET","_RBRACKET","ACTIONS","BIND","CASE","DEFAULT","ELSE","EXISTING",
"FOR","IF","IGNORE","IN","INCLUDE","LOCAL","ON","PIECEMEAL","QUIETLY","RETURN",
"RULE","SWITCH","TOGETHER","UPDATED","_LBRACE","_BARBAR","_RBRACE","ARG",
"STRING",
};
const char * const yyrule[] = {
"$accept : run",
"run :",
"run : rules",
"block :",
"block : rules",
"rules : rule",
"rules : rule rules",
"rules : LOCAL list _SEMIC block",
"rules : LOCAL list _EQUALS list _SEMIC block",
"rule : _LBRACE block _RBRACE",
"rule : INCLUDE list _SEMIC",
"rule : ARG lol _SEMIC",
"rule : arg assign list _SEMIC",
"rule : arg ON list assign list _SEMIC",
"rule : RETURN list _SEMIC",
"rule : FOR ARG IN list _LBRACE block _RBRACE",
"rule : SWITCH list _LBRACE cases _RBRACE",
"rule : IF cond _LBRACE block _RBRACE",
"rule : IF cond _LBRACE block _RBRACE ELSE rule",
"rule : RULE ARG rule",
"$$1 :",
"$$2 :",
"rule : ACTIONS eflags ARG bindlist _LBRACE $$1 STRING $$2 _RBRACE",
"assign : _EQUALS",
"assign : _PLUS_EQUALS",
"assign : _QUESTION_EQUALS",
"assign : DEFAULT _EQUALS",
"cond : arg",
"cond : arg _EQUALS arg",
"cond : arg _BANG_EQUALS arg",
"cond : arg _LANGLE arg",
"cond : arg _LANGLE_EQUALS arg",
"cond : arg _RANGLE arg",
"cond : arg _RANGLE_EQUALS arg",
"cond : arg IN list",
"cond : _BANG cond",
"cond : cond _AMPERAMPER cond",
"cond : cond _BARBAR cond",
"cond : _LPAREN cond _RPAREN",
"cases :",
"cases : case cases",
"case : CASE ARG _COLON block",
"lol : list",
"lol : list _COLON lol",
"list : listp",
"listp :",
"listp : listp arg",
"arg : ARG",
"arg : _LBRACKET ARG lol _RBRACKET",
"eflags :",
"eflags : eflags eflag",
"eflag : UPDATED",
"eflag : TOGETHER",
"eflag : IGNORE",
"eflag : QUIETLY",
"eflag : PIECEMEAL",
"eflag : EXISTING",
"bindlist :",
"bindlist : BIND list",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#if YYDEBUG
#include <stdio.h>
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    newss = yyss ? (short *)realloc(yyss, newsize * sizeof *newss) :
      (short *)malloc(newsize * sizeof *newss);
    if (newss == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    newvs = yyvs ? (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs) :
      (YYSTYPE *)malloc(newsize * sizeof *newvs);
    if (newvs == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#ifndef YYPARSE_PARAM
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG void
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif	/* ANSI-C/C++ */
#else	/* YYPARSE_PARAM */
#ifndef YYPARSE_PARAM_TYPE
#define YYPARSE_PARAM_TYPE void *
#endif
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG YYPARSE_PARAM_TYPE YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL YYPARSE_PARAM_TYPE YYPARSE_PARAM;
#endif	/* ANSI-C/C++ */
#endif	/* ! YYPARSE_PARAM */

int
yyparse (YYPARSE_PARAM_ARG)
    YYPARSE_PARAM_DECL
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 2:
#line 122 "jamgram.y"
{ parse_save( yyvsp[0].parse ); }
break;
case 3:
#line 133 "jamgram.y"
{ yyval.parse = pnull(); }
break;
case 4:
#line 135 "jamgram.y"
{ yyval.parse = yyvsp[0].parse; }
break;
case 5:
#line 139 "jamgram.y"
{ yyval.parse = yyvsp[0].parse; }
break;
case 6:
#line 141 "jamgram.y"
{ yyval.parse = prules( yyvsp[-1].parse, yyvsp[0].parse ); }
break;
case 7:
#line 143 "jamgram.y"
{ yyval.parse = plocal( yyvsp[-2].parse, pnull(), yyvsp[0].parse ); }
break;
case 8:
#line 145 "jamgram.y"
{ yyval.parse = plocal( yyvsp[-4].parse, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 9:
#line 149 "jamgram.y"
{ yyval.parse = yyvsp[-1].parse; }
break;
case 10:
#line 151 "jamgram.y"
{ yyval.parse = pincl( yyvsp[-1].parse ); }
break;
case 11:
#line 153 "jamgram.y"
{ yyval.parse = prule( yyvsp[-2].string, yyvsp[-1].parse ); }
break;
case 12:
#line 155 "jamgram.y"
{ yyval.parse = pset( yyvsp[-3].parse, yyvsp[-1].parse, yyvsp[-2].number ); }
break;
case 13:
#line 157 "jamgram.y"
{ yyval.parse = pset1( yyvsp[-5].parse, yyvsp[-3].parse, yyvsp[-1].parse, yyvsp[-2].number ); }
break;
case 14:
#line 159 "jamgram.y"
{ yyval.parse = yyvsp[-1].parse; }
break;
case 15:
#line 161 "jamgram.y"
{ yyval.parse = pfor( yyvsp[-5].string, yyvsp[-3].parse, yyvsp[-1].parse ); }
break;
case 16:
#line 163 "jamgram.y"
{ yyval.parse = pswitch( yyvsp[-3].parse, yyvsp[-1].parse ); }
break;
case 17:
#line 165 "jamgram.y"
{ yyval.parse = pif( yyvsp[-3].parse, yyvsp[-1].parse, pnull() ); }
break;
case 18:
#line 167 "jamgram.y"
{ yyval.parse = pif( yyvsp[-5].parse, yyvsp[-3].parse, yyvsp[0].parse ); }
break;
case 19:
#line 169 "jamgram.y"
{ yyval.parse = psetc( yyvsp[-1].string, yyvsp[0].parse ); }
break;
case 20:
#line 171 "jamgram.y"
{ yymode( SCAN_STRING ); }
break;
case 21:
#line 173 "jamgram.y"
{ yymode( SCAN_NORMAL ); }
break;
case 22:
#line 175 "jamgram.y"
{ yyval.parse = psete( yyvsp[-6].string,yyvsp[-5].parse,yyvsp[-2].string,yyvsp[-7].number ); }
break;
case 23:
#line 183 "jamgram.y"
{ yyval.number = ASSIGN_SET; }
break;
case 24:
#line 185 "jamgram.y"
{ yyval.number = ASSIGN_APPEND; }
break;
case 25:
#line 187 "jamgram.y"
{ yyval.number = ASSIGN_DEFAULT; }
break;
case 26:
#line 189 "jamgram.y"
{ yyval.number = ASSIGN_DEFAULT; }
break;
case 27:
#line 197 "jamgram.y"
{ yyval.parse = pcnode( COND_EXISTS, yyvsp[0].parse, pnull() ); }
break;
case 28:
#line 199 "jamgram.y"
{ yyval.parse = pcnode( COND_EQUALS, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 29:
#line 201 "jamgram.y"
{ yyval.parse = pcnode( COND_NOTEQ, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 30:
#line 203 "jamgram.y"
{ yyval.parse = pcnode( COND_LESS, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 31:
#line 205 "jamgram.y"
{ yyval.parse = pcnode( COND_LESSEQ, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 32:
#line 207 "jamgram.y"
{ yyval.parse = pcnode( COND_MORE, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 33:
#line 209 "jamgram.y"
{ yyval.parse = pcnode( COND_MOREEQ, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 34:
#line 211 "jamgram.y"
{ yyval.parse = pcnode( COND_IN, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 35:
#line 213 "jamgram.y"
{ yyval.parse = pcnode( COND_NOT, yyvsp[0].parse, P0 ); }
break;
case 36:
#line 215 "jamgram.y"
{ yyval.parse = pcnode( COND_AND, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 37:
#line 217 "jamgram.y"
{ yyval.parse = pcnode( COND_OR, yyvsp[-2].parse, yyvsp[0].parse ); }
break;
case 38:
#line 219 "jamgram.y"
{ yyval.parse = yyvsp[-1].parse; }
break;
case 39:
#line 229 "jamgram.y"
{ yyval.parse = P0; }
break;
case 40:
#line 231 "jamgram.y"
{ yyval.parse = pnode( yyvsp[-1].parse, yyvsp[0].parse ); }
break;
case 41:
#line 235 "jamgram.y"
{ yyval.parse = psnode( yyvsp[-2].string, yyvsp[0].parse ); }
break;
case 42:
#line 244 "jamgram.y"
{ yyval.parse = pnode( P0, yyvsp[0].parse ); }
break;
case 43:
#line 246 "jamgram.y"
{ yyval.parse = pnode( yyvsp[0].parse, yyvsp[-2].parse ); }
break;
case 44:
#line 256 "jamgram.y"
{ yyval.parse = yyvsp[0].parse; yymode( SCAN_NORMAL ); }
break;
case 45:
#line 260 "jamgram.y"
{ yyval.parse = pnull(); yymode( SCAN_PUNCT ); }
break;
case 46:
#line 262 "jamgram.y"
{ yyval.parse = pappend( yyvsp[-1].parse, yyvsp[0].parse ); }
break;
case 47:
#line 266 "jamgram.y"
{ yyval.parse = plist( yyvsp[0].string ); }
break;
case 48:
#line 268 "jamgram.y"
{ yyval.parse = prule( yyvsp[-2].string, yyvsp[-1].parse ); }
break;
case 49:
#line 278 "jamgram.y"
{ yyval.number = 0; }
break;
case 50:
#line 280 "jamgram.y"
{ yyval.number = yyvsp[-1].number | yyvsp[0].number; }
break;
case 51:
#line 284 "jamgram.y"
{ yyval.number = EXEC_UPDATED; }
break;
case 52:
#line 286 "jamgram.y"
{ yyval.number = EXEC_TOGETHER; }
break;
case 53:
#line 288 "jamgram.y"
{ yyval.number = EXEC_IGNORE; }
break;
case 54:
#line 290 "jamgram.y"
{ yyval.number = EXEC_QUIETLY; }
break;
case 55:
#line 292 "jamgram.y"
{ yyval.number = EXEC_PIECEMEAL; }
break;
case 56:
#line 294 "jamgram.y"
{ yyval.number = EXEC_EXISTING; }
break;
case 57:
#line 303 "jamgram.y"
{ yyval.parse = pnull(); }
break;
case 58:
#line 305 "jamgram.y"
{ yyval.parse = yyvsp[0].parse; }
break;
#line 758 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
