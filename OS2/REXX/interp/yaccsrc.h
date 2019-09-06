/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ADDRESS = 258,
     ARG = 259,
     CALL = 260,
     DO = 261,
     TO = 262,
     BY = 263,
     FOR = 264,
     WHILE = 265,
     UNTIL = 266,
     EXIT = 267,
     IF = 268,
     THEN = 269,
     ELSE = 270,
     ITERATE = 271,
     INTERPRET = 272,
     LEAVE = 273,
     NOP = 274,
     NUMERIC = 275,
     PARSE = 276,
     EXTERNAL = 277,
     SOURCE = 278,
     VAR = 279,
     VALUE = 280,
     WITH = 281,
     PROCEDURE = 282,
     EXPOSE = 283,
     PULL = 284,
     PUSH = 285,
     QUEUE = 286,
     SAY = 287,
     RETURN = 288,
     SELECT = 289,
     WHEN = 290,
     DROP = 291,
     OTHERWISE = 292,
     SIGNAL = 293,
     ON = 294,
     OFF = 295,
     SYNTAX = 296,
     HALT = 297,
     NOVALUE = 298,
     TRACE = 299,
     END = 300,
     UPPER = 301,
     ASSIGNMENTVARIABLE = 302,
     STATSEP = 303,
     FOREVER = 304,
     DIGITS = 305,
     FORM = 306,
     FUZZ = 307,
     SCIENTIFIC = 308,
     ENGINEERING = 309,
     NOT = 310,
     CONCATENATE = 311,
     MODULUS = 312,
     GTE = 313,
     GT = 314,
     LTE = 315,
     LT = 316,
     DIFFERENT = 317,
     EQUALEQUAL = 318,
     NOTEQUALEQUAL = 319,
     OFFSET = 320,
     SPACE = 321,
     EXP = 322,
     XOR = 323,
     PLACEHOLDER = 324,
     NOTREADY = 325,
     CONSYMBOL = 326,
     SIMSYMBOL = 327,
     EXFUNCNAME = 328,
     INFUNCNAME = 329,
     LABEL = 330,
     DOVARIABLE = 331,
     HEXSTRING = 332,
     STRING = 333,
     VERSION = 334,
     LINEIN = 335,
     WHATEVER = 336,
     NAME = 337,
     FAILURE = 338,
     BINSTRING = 339,
     OPTIONS = 340,
     ENVIRONMENT = 341,
     LOSTDIGITS = 342,
     GTGT = 343,
     LTLT = 344,
     NOTGTGT = 345,
     NOTLTLT = 346,
     GTGTE = 347,
     LTLTE = 348,
     INPUT = 349,
     OUTPUT = 350,
     ERROR = 351,
     NORMAL = 352,
     APPEND = 353,
     REPLACE = 354,
     STREAM = 355,
     STEM = 356,
     LIFO = 357,
     FIFO = 358,
     LOWER = 359,
     CASELESS = 360,
     PLUSASSIGNMENTVARIABLE = 361,
     MINUSASSIGNMENTVARIABLE = 362,
     MULTASSIGNMENTVARIABLE = 363,
     DIVASSIGNMENTVARIABLE = 364,
     MODULUSASSIGNMENTVARIABLE = 365,
     INTDIVASSIGNMENTVARIABLE = 366,
     ORASSIGNMENTVARIABLE = 367,
     XORASSIGNMENTVARIABLE = 368,
     ANDASSIGNMENTVARIABLE = 369,
     CONCATASSIGNMENTVARIABLE = 370,
     CCAT = 371,
     UPLUS = 372,
     UMINUS = 373
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE __reginalval;


