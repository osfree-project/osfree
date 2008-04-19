
/*  A Bison parser, made from parse.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define CHAR    258
#define NUMBER  259
#define SECTEND 260
#define SCDECL  261
#define XSCDECL 262
#define NAME    263
#define PREVCCL 264
#define EOF_OP  265
#define OPTION_OP       266
#define OPT_OUTFILE     267
#define OPT_PREFIX      268
#define OPT_YYCLASS     269
#define CCE_ALNUM       270
#define CCE_ALPHA       271
#define CCE_BLANK       272
#define CCE_CNTRL       273
#define CCE_DIGIT       274
#define CCE_GRAPH       275
#define CCE_LOWER       276
#define CCE_PRINT       277
#define CCE_PUNCT       278
#define CCE_SPACE       279
#define CCE_UPPER       280
#define CCE_XDIGIT      281

#line 9 "parse.y"

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Vern Paxson.
 *
 * The United States Government has rights in this work pursuant
 * to contract no. DE-AC03-76SF00098 between the United States
 * Department of Energy and the University of California.
 *
 * Redistribution and use in source and binary forms with or without
 * modification are permitted provided that: (1) source distributions retain
 * this entire copyright notice and comment, and (2) distributions including
 * binaries display the following acknowledgement:  ``This product includes
 * software developed by the University of California, Berkeley and its
 * contributors'' in the documentation or other materials provided with the
 * distribution and in all advertising materials mentioning features or use
 * of this software.  Neither the name of the University nor the names of
 * its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* $Header: /home/daffy/u0/vern/flex/RCS/parse.y,v 2.28 95/04/21 11:51:51 vern Exp $ */


/* Some versions of bison are broken in that they use alloca() but don't
 * declare it properly.  The following is the patented (just kidding!)
 * #ifdef chud to fix the problem, courtesy of Francois Pinard.
 */
#ifdef YYBISON
/* AIX requires this to be the first thing in the file.  What a piece.  */
# ifdef _AIX
 #pragma alloca
# endif
#endif

#include "flexdef.h"

/* The remainder of the alloca() cruft has to come after including flexdef.h,
 * so HAVE_ALLOCA_H is (possibly) defined.
 */
#ifdef YYBISON
# ifdef __GNUC__
#  ifndef alloca
#   define alloca __builtin_alloca
#  endif
# else
#  if HAVE_ALLOCA_H
#   include <alloca.h>
#  else
#   ifdef __hpux
void *alloca ();
#   else
#    ifdef __TURBOC__
#     include <malloc.h>
#    else
#     ifdef __WATCOM__
#      include <malloc.h>
#     else
char *alloca ();
#     endif
#    endif
#   endif
#  endif
# endif
#endif

/* Bletch, ^^^^ that was ugly! */


int pat, scnum, eps, headcnt, trailcnt, anyccl, lastchar, i, rulelen;
int trlcontxt, xcluflg, currccl, cclsorted, varlength, variable_trail_rule;

int *scon_stk;
int scon_stk_ptr;

static int madeany = false;  /* whether we've made the '.' character class */
int previous_continued_action;  /* whether the previous rule's action was '|' */

/* Expand a POSIX character class expression. */
#define CCL_EXPR(func) \
        { \
        int c; \
        for ( c = 0; c < csize; ++c ) \
                if ( isascii(c) && func(c) ) \
                        ccladd( currccl, c ); \
        }

/* While POSIX defines isblank(), it's not ANSI C. */
#define IS_BLANK(c) ((c) == ' ' || (c) == '\t')

/* On some over-ambitious machines, such as DEC Alpha's, the default
 * token type is "long" instead of "int"; this leads to problems with
 * declaring yylval in flexdef.h.  But so far, all the yacc's I've seen
 * wrap their definitions of YYSTYPE with "#ifndef YYSTYPE"'s, so the
 * following should ensure that the default token type is "int".
 */
#define YYSTYPE int

#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define YYFINAL         107
#define YYFLAG          -32768
#define YYNTBASE        48

#define YYTRANSLATE(x) ((unsigned)(x) <= 281 ? yytranslate[x] : 73)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    28,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    42,     2,    36,     2,     2,     2,    43,
    44,    34,    39,    35,    47,    41,    38,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    32,
    27,    33,    40,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    45,     2,    46,    31,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    29,    37,    30,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     6,     7,    11,    14,    15,    17,    19,    21,    23,
    26,    28,    30,    33,    36,    37,    41,    45,    49,    55,
    61,    62,    63,    66,    68,    70,    72,    73,    78,    82,
    83,    87,    89,    91,    93,    96,   100,   103,   105,   109,
   111,   114,   117,   119,   122,   125,   128,   135,   141,   146,
   148,   150,   152,   156,   160,   162,   166,   171,   176,   179,
   182,   183,   185,   187,   189,   191,   193,   195,   197,   199,
   201,   203,   205,   207,   210
};

static const short yyrhs[] = {    49,
    50,    51,    57,    58,     0,     0,    50,    52,    53,     0,
    50,    54,     0,     0,     1,     0,     5,     0,     6,     0,
     7,     0,    53,     8,     0,     8,     0,     1,     0,    11,
    55,     0,    55,    56,     0,     0,    12,    27,     8,     0,
    13,    27,     8,     0,    14,    27,     8,     0,    57,    61,
    58,    59,    28,     0,    57,    61,    29,    57,    30,     0,
     0,     0,    31,    64,     0,    64,     0,    10,     0,     1,
     0,     0,    32,    60,    62,    33,     0,    32,    34,    33,
     0,     0,    62,    35,    63,     0,    63,     0,     1,     0,
     8,     0,    66,    65,     0,    66,    65,    36,     0,    65,
    36,     0,    65,     0,    65,    37,    67,     0,    67,     0,
    65,    38,     0,    67,    68,     0,    68,     0,    68,    34,
     0,    68,    39,     0,    68,    40,     0,    68,    29,     4,
    35,     4,    30,     0,    68,    29,     4,    35,    30,     0,
    68,    29,     4,    30,     0,    41,     0,    69,     0,     9,
     0,    42,    72,    42,     0,    43,    65,    44,     0,     3,
     0,    45,    70,    46,     0,    45,    31,    70,    46,     0,
    70,     3,    47,     3,     0,    70,     3,     0,    70,    71,
     0,     0,    15,     0,    16,     0,    17,     0,    18,     0,
    19,     0,    20,     0,    21,     0,    22,     0,    23,     0,
    24,     0,    25,     0,    26,     0,    72,     3,     0,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   112,   141,   149,   150,   151,   152,   156,   164,   167,   171,
   174,   177,   181,   184,   185,   188,   193,   195,   199,   201,
   203,   206,   219,   255,   279,   302,   306,   310,   313,   330,
   334,   336,   338,   342,   365,   420,   423,   466,   484,   490,
   495,   522,   530,   534,   541,   547,   553,   581,   595,   614,
   636,   654,   661,   664,   667,   678,   681,   688,   716,   727,
   734,   742,   743,   744,   745,   746,   747,   748,   749,   750,
   751,   752,   758,   761,   771
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","CHAR","NUMBER",
"SECTEND","SCDECL","XSCDECL","NAME","PREVCCL","EOF_OP","OPTION_OP","OPT_OUTFILE",
"OPT_PREFIX","OPT_YYCLASS","CCE_ALNUM","CCE_ALPHA","CCE_BLANK","CCE_CNTRL","CCE_DIGIT",
"CCE_GRAPH","CCE_LOWER","CCE_PRINT","CCE_PUNCT","CCE_SPACE","CCE_UPPER","CCE_XDIGIT",
"'='","'\\n'","'{'","'}'","'^'","'<'","'>'","'*'","','","'$'","'|'","'/'","'+'",
"'?'","'.'","'\"'","'('","')'","'['","']'","'-'","goal","initlex","sect1","sect1end",
"startconddecl","namelist1","options","optionlist","option","sect2","initforrule",
"flexrule","scon_stk_ptr","scon","namelist2","sconname","rule","re","re2","series",
"singleton","fullccl","ccl","ccl_expr","string", NULL
};
#endif

static const short yyr1[] = {     0,
    48,    49,    50,    50,    50,    50,    51,    52,    52,    53,
    53,    53,    54,    55,    55,    56,    56,    56,    57,    57,
    57,    58,    59,    59,    59,    59,    60,    61,    61,    61,
    62,    62,    62,    63,    64,    64,    64,    64,    65,    65,
    66,    67,    67,    68,    68,    68,    68,    68,    68,    68,
    68,    68,    68,    68,    68,    69,    69,    70,    70,    70,
    70,    71,    71,    71,    71,    71,    71,    71,    71,    71,
    71,    71,    71,    72,    72
};

static const short yyr2[] = {     0,
     5,     0,     3,     2,     0,     1,     1,     1,     1,     2,
     1,     1,     2,     2,     0,     3,     3,     3,     5,     5,
     0,     0,     2,     1,     1,     1,     0,     4,     3,     0,
     3,     1,     1,     1,     2,     3,     2,     1,     3,     1,
     2,     2,     1,     2,     2,     2,     6,     5,     4,     1,
     1,     1,     3,     3,     1,     3,     4,     4,     2,     2,
     0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     2,     0
};

static const short yydefact[] = {     2,
     0,     6,     0,     7,     8,     9,    15,    21,     0,     4,
    13,    30,    12,    11,     3,     0,     0,     0,    14,    27,
     1,    22,    10,     0,     0,     0,     0,     0,    21,     0,
    16,    17,    18,    29,    33,    34,     0,    32,    30,    26,
    55,    52,    25,     0,    50,    75,     0,    61,     0,    24,
    38,     0,    40,    43,    51,    28,     0,    20,    23,     0,
     0,    61,     0,    19,    37,     0,    41,    35,    42,     0,
    44,    45,    46,    31,    74,    53,    54,     0,    59,    62,
    63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    56,    60,    39,    36,     0,    57,     0,    49,     0,
    58,     0,    48,    47,     0,     0,     0
};

static const short yydefgoto[] = {   105,
     1,     3,     8,     9,    15,    10,    11,    19,    12,    21,
    49,    28,    22,    37,    38,    50,    51,    52,    53,    54,
    55,    63,    93,    60
};

static const short yypact[] = {-32768,
    78,-32768,    81,-32768,-32768,-32768,-32768,-32768,     6,-32768,
    -2,     5,-32768,-32768,    -7,   -14,     1,     9,-32768,    28,
-32768,    14,-32768,    55,    65,    74,    57,    30,-32768,    -1,
-32768,-32768,-32768,-32768,-32768,-32768,    58,-32768,    44,-32768,
-32768,-32768,-32768,    24,-32768,-32768,    24,    63,    69,-32768,
    23,    24,    24,    41,-32768,-32768,    90,-32768,-32768,    26,
    27,-32768,     0,-32768,-32768,    24,-32768,    59,    41,    95,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    32,    53,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    24,-32768,     4,-32768,    98,-32768,     2,
-32768,    72,-32768,-32768,   103,   104,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    76,    84,
-32768,-32768,-32768,-32768,    50,    64,    25,-32768,    43,   -49,
-32768,    48,-32768,-32768
};


#define YYLAST          110


static const short yytable[] = {    40,
    23,    41,    79,    69,   -22,   102,    13,    42,    43,    16,
    17,    18,    24,    14,    80,    81,    82,    83,    84,    85,
    86,    87,    88,    89,    90,    91,    41,    25,    75,    44,
    35,   103,    42,    99,    79,    26,    20,    36,   100,    45,
    46,    47,    29,    48,    69,    92,    80,    81,    82,    83,
    84,    85,    86,    87,    88,    89,    90,    91,    65,    66,
    67,    27,    31,    66,    45,    46,    47,    76,    48,    70,
    77,    61,    32,    58,    71,    20,    68,    97,     2,    72,
    73,    33,    -5,    -5,    -5,     4,     5,     6,    -5,    34,
    56,     7,    57,    62,    95,    66,    64,    36,    96,    98,
   101,   104,   106,   107,    39,    30,    74,    59,    94,    78
};

static const short yycheck[] = {     1,
     8,     3,     3,    53,     0,     4,     1,     9,    10,    12,
    13,    14,    27,     8,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,     3,    27,     3,    31,
     1,    30,     9,    30,     3,    27,    32,     8,    35,    41,
    42,    43,    29,    45,    94,    46,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    36,    37,
    38,    34,     8,    37,    41,    42,    43,    42,    45,    29,
    44,    47,     8,    30,    34,    32,    52,    46,     1,    39,
    40,     8,     5,     6,     7,     5,     6,     7,    11,    33,
    33,    11,    35,    31,    36,    37,    28,     8,     4,    47,
     3,    30,     0,     0,    29,    22,    57,    44,    66,    62
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        return(0)
#define YYABORT         return(1)
#define YYERROR         goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (yychar == YYEMPTY && yylen == 1)                          \
    { yychar = (token), yylval = (value);                       \
      yychar1 = YYTRANSLATE (yychar);                           \
      YYPOPSTACK;                                               \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    { yyerror ("syntax error: cannot back up"); YYERROR; }      \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YYPURE
#define YYLEX           yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX           yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX           yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX           yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX           yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int     yychar;                 /*  the lookahead symbol                */
YYSTYPE yylval;                 /*  the semantic value of the           */
                                /*  lookahead symbol                    */

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;                 /*  location data for the lookahead     */
                                /*  symbol                              */
#endif

int yynerrs;                    /*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)      __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;              /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YYSTYPE yyvsa[YYINITDEPTH];   /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;        /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];   /*  the location stack                  */
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;                /*  the variable used to return         */
                                /*  semantic values from the action     */
                                /*  routines                            */

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;             /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
         the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
         but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
                 &yyss1, size * sizeof (*yyssp),
                 &yyvs1, size * sizeof (*yyvsp),
                 &yyls1, size * sizeof (*yylsp),
                 &yystacksize);
#else
      yyoverflow("parser stack overflow",
                 &yyss1, size * sizeof (*yyssp),
                 &yyvs1, size * sizeof (*yyvsp),
                 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
        {
          yyerror("parser stack overflow");
          return 2;
        }
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
        yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
        fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
        YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
        fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)              /* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;           /* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
        fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
        {
          fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
          /* Give the individual parser a way to print the precise meaning
             of a token, for further debugging info.  */
#ifdef YYPRINT
          YYPRINT (stderr, yychar, yylval);
#endif
          fprintf (stderr, ")\n");
        }
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
               yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
        fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 113 "parse.y"
{ /* add default rule */
                        int def_rule;

                        pat = cclinit();
                        cclnegate( pat );

                        def_rule = mkstate( -pat );

                        /* Remember the number of the default rule so we
                         * don't generate "can't match" warnings for it.
                         */
                        default_rule = num_rules;

                        finish_rule( def_rule, false, 0, 0 );

                        for ( i = 1; i <= lastsc; ++i )
                                scset[i] = mkbranch( scset[i], def_rule );

                        if ( spprdflt )
                                add_action(
                                "YY_FATAL_ERROR( \"flex scanner jammed\" )" );
                        else
                                add_action( "ECHO" );

                        add_action( ";\n\tYY_BREAK\n" );
                        ;
    break;}
case 2:
#line 142 "parse.y"
{ /* initialize for processing rules */

                        /* Create default DFA start condition. */
                        scinstal( "INITIAL", false );
                        ;
    break;}
case 6:
#line 153 "parse.y"
{ synerr( "unknown error processing section 1" ); ;
    break;}
case 7:
#line 157 "parse.y"
{
                        check_options();
                        scon_stk = allocate_integer_array( lastsc + 1 );
                        scon_stk_ptr = 0;
                        ;
    break;}
case 8:
#line 165 "parse.y"
{ xcluflg = false; ;
    break;}
case 9:
#line 168 "parse.y"
{ xcluflg = true; ;
    break;}
case 10:
#line 172 "parse.y"
{ scinstal( nmstr, xcluflg ); ;
    break;}
case 11:
#line 175 "parse.y"
{ scinstal( nmstr, xcluflg ); ;
    break;}
case 12:
#line 178 "parse.y"
{ synerr( "bad start condition list" ); ;
    break;}
case 16:
#line 189 "parse.y"
{
                        outfilename = copy_string( nmstr );
                        did_outfilename = 1;
                        ;
    break;}
case 17:
#line 194 "parse.y"
{ prefix = copy_string( nmstr ); ;
    break;}
case 18:
#line 196 "parse.y"
{ yyclass = copy_string( nmstr ); ;
    break;}
case 19:
#line 200 "parse.y"
{ scon_stk_ptr = yyvsp[-3]; ;
    break;}
case 20:
#line 202 "parse.y"
{ scon_stk_ptr = yyvsp[-3]; ;
    break;}
case 22:
#line 207 "parse.y"
{
                        /* Initialize for a parse of one rule. */
                        trlcontxt = variable_trail_rule = varlength = false;
                        trailcnt = headcnt = rulelen = 0;
                        current_state_type = STATE_NORMAL;
                        previous_continued_action = continued_action;
                        in_rule = true;

                        new_rule();
                        ;
    break;}
case 23:
#line 220 "parse.y"
{
                        pat = yyvsp[0];
                        finish_rule( pat, variable_trail_rule,
                                headcnt, trailcnt );

                        if ( scon_stk_ptr > 0 )
                                {
                                for ( i = 1; i <= scon_stk_ptr; ++i )
                                        scbol[scon_stk[i]] =
                                                mkbranch( scbol[scon_stk[i]],
                                                                pat );
                                }

                        else
                                {
                                /* Add to all non-exclusive start conditions,
                                 * including the default (0) start condition.
                                 */

                                for ( i = 1; i <= lastsc; ++i )
                                        if ( ! scxclu[i] )
                                                scbol[i] = mkbranch( scbol[i],
                                                                        pat );
                                }

                        if ( ! bol_needed )
                                {
                                bol_needed = true;

                                if ( performance_report > 1 )
                                        pinpoint_message(
                        "'^' operator results in sub-optimal performance" );
                                }
                        ;
    break;}
case 24:
#line 256 "parse.y"
{
                        pat = yyvsp[0];
                        finish_rule( pat, variable_trail_rule,
                                headcnt, trailcnt );

                        if ( scon_stk_ptr > 0 )
                                {
                                for ( i = 1; i <= scon_stk_ptr; ++i )
                                        scset[scon_stk[i]] =
                                                mkbranch( scset[scon_stk[i]],
                                                                pat );
                                }

                        else
                                {
                                for ( i = 1; i <= lastsc; ++i )
                                        if ( ! scxclu[i] )
                                                scset[i] =
                                                        mkbranch( scset[i],
                                                                pat );
                                }
                        ;
    break;}
case 25:
#line 280 "parse.y"
{
                        if ( scon_stk_ptr > 0 )
                                build_eof_action();

                        else
                                {
                                /* This EOF applies to all start conditions
                                 * which don't already have EOF actions.
                                 */
                                for ( i = 1; i <= lastsc; ++i )
                                        if ( ! sceof[i] )
                                                scon_stk[++scon_stk_ptr] = i;

                                if ( scon_stk_ptr == 0 )
                                        warn(
                        "all start conditions already have <<EOF>> rules" );

                                else
                                        build_eof_action();
                                }
                        ;
    break;}
case 26:
#line 303 "parse.y"
{ synerr( "unrecognized rule" ); ;
    break;}
case 27:
#line 307 "parse.y"
{ yyval = scon_stk_ptr; ;
    break;}
case 28:
#line 311 "parse.y"
{ yyval = yyvsp[-2]; ;
    break;}
case 29:
#line 314 "parse.y"
{
                        yyval = scon_stk_ptr;

                        for ( i = 1; i <= lastsc; ++i )
                                {
                                int j;

                                for ( j = 1; j <= scon_stk_ptr; ++j )
                                        if ( scon_stk[j] == i )
                                                break;

                                if ( j > scon_stk_ptr )
                                        scon_stk[++scon_stk_ptr] = i;
                                }
                        ;
    break;}
case 30:
#line 331 "parse.y"
{ yyval = scon_stk_ptr; ;
    break;}
case 33:
#line 339 "parse.y"
{ synerr( "bad start condition list" ); ;
    break;}
case 34:
#line 343 "parse.y"
{
                        if ( (scnum = sclookup( nmstr )) == 0 )
                                format_pinpoint_message(
                                        "undeclared start condition %s",
                                        nmstr );
                        else
                                {
                                for ( i = 1; i <= scon_stk_ptr; ++i )
                                        if ( scon_stk[i] == scnum )
                                                {
                                                format_warn(
                                                        "<%s> specified twice",
                                                        scname[scnum] );
                                                break;
                                                }

                                if ( i > scon_stk_ptr )
                                        scon_stk[++scon_stk_ptr] = scnum;
                                }
                        ;
    break;}
case 35:
#line 366 "parse.y"
{
                        if ( transchar[lastst[yyvsp[0]]] != SYM_EPSILON )
                                /* Provide final transition \now/ so it
                                 * will be marked as a trailing context
                                 * state.
                                 */
                                yyvsp[0] = link_machines( yyvsp[0],
                                                mkstate( SYM_EPSILON ) );

                        mark_beginning_as_normal( yyvsp[0] );
                        current_state_type = STATE_NORMAL;

                        if ( previous_continued_action )
                                {
                                /* We need to treat this as variable trailing
                                 * context so that the backup does not happen
                                 * in the action but before the action switch
                                 * statement.  If the backup happens in the
                                 * action, then the rules "falling into" this
                                 * one's action will *also* do the backup,
                                 * erroneously.
                                 */
                                if ( ! varlength || headcnt != 0 )
                                        warn(
                "trailing context made variable due to preceding '|' action" );

                                /* Mark as variable. */
                                varlength = true;
                                headcnt = 0;
                                }

                        if ( lex_compat || (varlength && headcnt == 0) )
                                { /* variable trailing context rule */
                                /* Mark the first part of the rule as the
                                 * accepting "head" part of a trailing
                                 * context rule.
                                 *
                                 * By the way, we didn't do this at the
                                 * beginning of this production because back
                                 * then current_state_type was set up for a
                                 * trail rule, and add_accept() can create
                                 * a new state ...
                                 */
                                add_accept( yyvsp[-1],
                                        num_rules | YY_TRAILING_HEAD_MASK );
                                variable_trail_rule = true;
                                }

                        else
                                trailcnt = rulelen;

                        yyval = link_machines( yyvsp[-1], yyvsp[0] );
                        ;
    break;}
case 36:
#line 421 "parse.y"
{ synerr( "trailing context used twice" ); ;
    break;}
case 37:
#line 424 "parse.y"
{
                        headcnt = 0;
                        trailcnt = 1;
                        rulelen = 1;
                        varlength = false;

                        current_state_type = STATE_TRAILING_CONTEXT;

                        if ( trlcontxt )
                                {
                                synerr( "trailing context used twice" );
                                yyval = mkstate( SYM_EPSILON );
                                }

                        else if ( previous_continued_action )
                                {
                                /* See the comment in the rule for "re2 re"
                                 * above.
                                 */
                                warn(
                "trailing context made variable due to preceding '|' action" );

                                varlength = true;
                                }

                        if ( lex_compat || varlength )
                                {
                                /* Again, see the comment in the rule for
                                 * "re2 re" above.
                                 */
                                add_accept( yyvsp[-1],
                                        num_rules | YY_TRAILING_HEAD_MASK );
                                variable_trail_rule = true;
                                }

                        trlcontxt = true;

                        eps = mkstate( SYM_EPSILON );
                        yyval = link_machines( yyvsp[-1],
                                link_machines( eps, mkstate( '\n' ) ) );
                        ;
    break;}
case 38:
#line 467 "parse.y"
{
                        yyval = yyvsp[0];

                        if ( trlcontxt )
                                {
                                if ( lex_compat || (varlength && headcnt == 0) )
                                        /* Both head and trail are
                                         * variable-length.
                                         */
                                        variable_trail_rule = true;
                                else
                                        trailcnt = rulelen;
                                }
                        ;
    break;}
case 39:
#line 485 "parse.y"
{
                        varlength = true;
                        yyval = mkor( yyvsp[-2], yyvsp[0] );
                        ;
    break;}
case 40:
#line 491 "parse.y"
{ yyval = yyvsp[0]; ;
    break;}
case 41:
#line 496 "parse.y"
{
                        /* This rule is written separately so the
                         * reduction will occur before the trailing
                         * series is parsed.
                         */

                        if ( trlcontxt )
                                synerr( "trailing context used twice" );
                        else
                                trlcontxt = true;

                        if ( varlength )
                                /* We hope the trailing context is
                                 * fixed-length.
                                 */
                                varlength = false;
                        else
                                headcnt = rulelen;

                        rulelen = 0;

                        current_state_type = STATE_TRAILING_CONTEXT;
                        yyval = yyvsp[-1];
                        ;
    break;}
case 42:
#line 523 "parse.y"
{
                        /* This is where concatenation of adjacent patterns
                         * gets done.
                         */
                        yyval = link_machines( yyvsp[-1], yyvsp[0] );
                        ;
    break;}
case 43:
#line 531 "parse.y"
{ yyval = yyvsp[0]; ;
    break;}
case 44:
#line 535 "parse.y"
{
                        varlength = true;

                        yyval = mkclos( yyvsp[-1] );
                        ;
    break;}
case 45:
#line 542 "parse.y"
{
                        varlength = true;
                        yyval = mkposcl( yyvsp[-1] );
                        ;
    break;}
case 46:
#line 548 "parse.y"
{
                        varlength = true;
                        yyval = mkopt( yyvsp[-1] );
                        ;
    break;}
case 47:
#line 554 "parse.y"
{
                        varlength = true;

                        if ( yyvsp[-3] > yyvsp[-1] || yyvsp[-3] < 0 )
                                {
                                synerr( "bad iteration values" );
                                yyval = yyvsp[-5];
                                }
                        else
                                {
                                if ( yyvsp[-3] == 0 )
                                        {
                                        if ( yyvsp[-1] <= 0 )
                                                {
                                                synerr(
                                                "bad iteration values" );
                                                yyval = yyvsp[-5];
                                                }
                                        else
                                                yyval = mkopt(
                                                        mkrep( yyvsp[-5], 1, yyvsp[-1] ) );
                                        }
                                else
                                        yyval = mkrep( yyvsp[-5], yyvsp[-3], yyvsp[-1] );
                                }
                        ;
    break;}
case 48:
#line 582 "parse.y"
{
                        varlength = true;

                        if ( yyvsp[-2] <= 0 )
                                {
                                synerr( "iteration value must be positive" );
                                yyval = yyvsp[-4];
                                }

                        else
                                yyval = mkrep( yyvsp[-4], yyvsp[-2], INFINITY );
                        ;
    break;}
case 49:
#line 596 "parse.y"
{
                        /* The singleton could be something like "(foo)",
                         * in which case we have no idea what its length
                         * is, so we punt here.
                         */
                        varlength = true;

                        if ( yyvsp[-1] <= 0 )
                                {
                                synerr( "iteration value must be positive" );
                                yyval = yyvsp[-3];
                                }

                        else
                                yyval = link_machines( yyvsp[-3],
                                                copysingl( yyvsp[-3], yyvsp[-1] - 1 ) );
                        ;
    break;}
case 50:
#line 615 "parse.y"
{
                        if ( ! madeany )
                                {
                                /* Create the '.' character class. */
                                anyccl = cclinit();
                                ccladd( anyccl, '\n' );
                                cclnegate( anyccl );

                                if ( useecs )
                                        mkeccl( ccltbl + cclmap[anyccl],
                                                ccllen[anyccl], nextecm,
                                                ecgroup, csize, csize );

                                madeany = true;
                                }

                        ++rulelen;

                        yyval = mkstate( -anyccl );
                        ;
    break;}
case 51:
#line 637 "parse.y"
{
                        if ( ! cclsorted )
                                /* Sort characters for fast searching.  We
                                 * use a shell sort since this list could
                                 * be large.
                                 */
                                cshell( ccltbl + cclmap[yyvsp[0]], ccllen[yyvsp[0]], true );

                        if ( useecs )
                                mkeccl( ccltbl + cclmap[yyvsp[0]], ccllen[yyvsp[0]],
                                        nextecm, ecgroup, csize, csize );

                        ++rulelen;

                        yyval = mkstate( -yyvsp[0] );
                        ;
    break;}
case 52:
#line 655 "parse.y"
{
                        ++rulelen;

                        yyval = mkstate( -yyvsp[0] );
                        ;
    break;}
case 53:
#line 662 "parse.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 54:
#line 665 "parse.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 55:
#line 668 "parse.y"
{
                        ++rulelen;

                        if ( caseins && yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
                                yyvsp[0] = clower( yyvsp[0] );

                        yyval = mkstate( yyvsp[0] );
                        ;
    break;}
case 56:
#line 679 "parse.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 57:
#line 682 "parse.y"
{
                        cclnegate( yyvsp[-1] );
                        yyval = yyvsp[-1];
                        ;
    break;}
case 58:
#line 689 "parse.y"
{
                        if ( caseins )
                                {
                                if ( yyvsp[-2] >= 'A' && yyvsp[-2] <= 'Z' )
                                        yyvsp[-2] = clower( yyvsp[-2] );
                                if ( yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
                                        yyvsp[0] = clower( yyvsp[0] );
                                }

                        if ( yyvsp[-2] > yyvsp[0] )
                                synerr( "negative range in character class" );

                        else
                                {
                                for ( i = yyvsp[-2]; i <= yyvsp[0]; ++i )
                                        ccladd( yyvsp[-3], i );

                                /* Keep track if this ccl is staying in
                                 * alphabetical order.
                                 */
                                cclsorted = cclsorted && (yyvsp[-2] > lastchar);
                                lastchar = yyvsp[0];
                                }

                        yyval = yyvsp[-3];
                        ;
    break;}
case 59:
#line 717 "parse.y"
{
                        if ( caseins && yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
                                yyvsp[0] = clower( yyvsp[0] );

                        ccladd( yyvsp[-1], yyvsp[0] );
                        cclsorted = cclsorted && (yyvsp[0] > lastchar);
                        lastchar = yyvsp[0];
                        yyval = yyvsp[-1];
                        ;
    break;}
case 60:
#line 728 "parse.y"
{
                        /* Too hard to properly maintain cclsorted. */
                        cclsorted = false;
                        yyval = yyvsp[-1];
                        ;
    break;}
case 61:
#line 735 "parse.y"
{
                        cclsorted = true;
                        lastchar = 0;
                        currccl = yyval = cclinit();
                        ;
    break;}
case 62:
#line 742 "parse.y"
{ CCL_EXPR(isalnum) ;
    break;}
case 63:
#line 743 "parse.y"
{ CCL_EXPR(isalpha) ;
    break;}
case 64:
#line 744 "parse.y"
{ CCL_EXPR(IS_BLANK) ;
    break;}
case 65:
#line 745 "parse.y"
{ CCL_EXPR(iscntrl) ;
    break;}
case 66:
#line 746 "parse.y"
{ CCL_EXPR(isdigit) ;
    break;}
case 67:
#line 747 "parse.y"
{ CCL_EXPR(isgraph) ;
    break;}
case 68:
#line 748 "parse.y"
{ CCL_EXPR(islower) ;
    break;}
case 69:
#line 749 "parse.y"
{ CCL_EXPR(isprint) ;
    break;}
case 70:
#line 750 "parse.y"
{ CCL_EXPR(ispunct) ;
    break;}
case 71:
#line 751 "parse.y"
{ CCL_EXPR(isspace) ;
    break;}
case 72:
#line 752 "parse.y"
{
                                if ( caseins )
                                        CCL_EXPR(islower)
                                else
                                        CCL_EXPR(isupper)
                                ;
    break;}
case 73:
#line 758 "parse.y"
{ CCL_EXPR(isxdigit) ;
    break;}
case 74:
#line 762 "parse.y"
{
                        if ( caseins && yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
                                yyvsp[0] = clower( yyvsp[0] );

                        ++rulelen;

                        yyval = link_machines( yyvsp[-1], mkstate( yyvsp[0] ) );
                        ;
    break;}
case 75:
#line 772 "parse.y"
{ yyval = mkstate( SYM_EPSILON ); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
        fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
        {
          int size = 0;
          char *msg;
          int x, count;

          count = 0;
          /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
          for (x = (yyn < 0 ? -yyn : 0);
               x < (sizeof(yytname) / sizeof(char *)); x++)
            if (yycheck[x + yyn] == x)
              size += strlen(yytname[x]) + 15, count++;
          msg = (char *) malloc(size + 15);
          if (msg != 0)
            {
              strcpy(msg, "parse error");

              if (count < 5)
                {
                  count = 0;
                  for (x = (yyn < 0 ? -yyn : 0);
                       x < (sizeof(yytname) / sizeof(char *)); x++)
                    if (yycheck[x + yyn] == x)
                      {
                        strcat(msg, count == 0 ? ", expecting `" : " or `");
                        strcat(msg, yytname[x]);
                        strcat(msg, "'");
                        count++;
                      }
                }
              yyerror(msg);
              free(msg);
            }
          else
            yyerror ("parse error; also virtual memory exceeded");
        }
      else
#endif /* YYERROR_VERBOSE */
        yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
        YYABORT;

#if YYDEBUG != 0
      if (yydebug)
        fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;              /* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
        fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
        goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 775 "parse.y"



/* build_eof_action - build the "<<EOF>>" action for the active start
 *                    conditions
 */

void build_eof_action()
        {
        register int i;
        char action_text[MAXLINE];

        for ( i = 1; i <= scon_stk_ptr; ++i )
                {
                if ( sceof[scon_stk[i]] )
                        format_pinpoint_message(
                                "multiple <<EOF>> rules for start condition %s",
                                scname[scon_stk[i]] );

                else
                        {
                        sceof[scon_stk[i]] = true;
                        sprintf( action_text, "case YY_STATE_EOF(%s):\n",
                                scname[scon_stk[i]] );
                        add_action( action_text );
                        }
                }

        line_directive_out( (FILE *) 0, 1 );

        /* This isn't a normal rule after all - don't count it as
         * such, so we don't have any holes in the rule numbering
         * (which make generating "rule can never match" warnings
         * more difficult.
         */
        --num_rules;
        ++num_eof_rules;
        }


/* format_synerr - write out formatted syntax error */

void format_synerr( msg, arg )
char msg[], arg[];
        {
        char errmsg[MAXLINE];

        (void) sprintf( errmsg, msg, arg );
        synerr( errmsg );
        }


/* synerr - report a syntax error */

void synerr( str )
char str[];
        {
        syntaxerror = true;
        pinpoint_message( str );
        }


/* format_warn - write out formatted warning */

void format_warn( msg, arg )
char msg[], arg[];
        {
        char warn_msg[MAXLINE];

        (void) sprintf( warn_msg, msg, arg );
        warn( warn_msg );
        }


/* warn - report a warning, unless -w was given */

void warn( str )
char str[];
        {
        line_warning( str, linenum );
        }

/* format_pinpoint_message - write out a message formatted with one string,
 *                           pinpointing its location
 */

void format_pinpoint_message( msg, arg )
char msg[], arg[];
        {
        char errmsg[MAXLINE];

        (void) sprintf( errmsg, msg, arg );
        pinpoint_message( errmsg );
        }


/* pinpoint_message - write out a message, pinpointing its location */

void pinpoint_message( str )
char str[];
        {
        line_pinpoint( str, linenum );
        }


/* line_warning - report a warning at a given line, unless -w was given */

void line_warning( str, line )
char str[];
int line;
        {
        char warning[MAXLINE];

        if ( ! nowarn )
                {
                sprintf( warning, "warning, %s", str );
                line_pinpoint( warning, line );
                }
        }


/* line_pinpoint - write out a message, pinpointing it at the given line */

void line_pinpoint( str, line )
char str[];
int line;
        {
        fprintf( stderr, "\"%s\", line %d: %s\n", infilename, line, str );
        }


/* yyerror - eat up an error message from the parser;
 *           currently, messages are ignore
 */

void yyerror( msg )
char msg[];
        {
        }
