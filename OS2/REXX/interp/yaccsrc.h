/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

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
     ERROR = 296,
     SYNTAX = 297,
     HALT = 298,
     NOVALUE = 299,
     TRACE = 300,
     END = 301,
     UPPER = 302,
     ASSIGNMENTVARIABLE = 303,
     STATSEP = 304,
     FOREVER = 305,
     DIGITS = 306,
     FORM = 307,
     FUZZ = 308,
     SCIENTIFIC = 309,
     ENGINEERING = 310,
     NOT = 311,
     CONCATENATE = 312,
     MODULUS = 313,
     GTE = 314,
     GT = 315,
     LTE = 316,
     LT = 317,
     DIFFERENT = 318,
     EQUALEQUAL = 319,
     NOTEQUALEQUAL = 320,
     OFFSET = 321,
     SPACE = 322,
     EXP = 323,
     XOR = 324,
     PLACEHOLDER = 325,
     NOTREADY = 326,
     CONSYMBOL = 327,
     SIMSYMBOL = 328,
     EXFUNCNAME = 329,
     INFUNCNAME = 330,
     LABEL = 331,
     DOVARIABLE = 332,
     HEXSTRING = 333,
     STRING = 334,
     VERSION = 335,
     LINEIN = 336,
     WHATEVER = 337,
     NAME = 338,
     FAILURE = 339,
     BINSTRING = 340,
     OPTIONS = 341,
     ENVIRONMENT = 342,
     LOSTDIGITS = 343,
     GTGT = 344,
     LTLT = 345,
     NOTGTGT = 346,
     NOTLTLT = 347,
     GTGTE = 348,
     LTLTE = 349,
     INPUT = 350,
     OUTPUT = 351,
     NORMAL = 352,
     APPEND = 353,
     REPLACE = 354,
     STREAM = 355,
     STEM = 356,
     LIFO = 357,
     FIFO = 358,
     LOWER = 359,
     CASELESS = 360,
     CCAT = 361,
     UPLUS = 362,
     UMINUS = 363
   };
#endif
#define ADDRESS 258
#define ARG 259
#define CALL 260
#define DO 261
#define TO 262
#define BY 263
#define FOR 264
#define WHILE 265
#define UNTIL 266
#define EXIT 267
#define IF 268
#define THEN 269
#define ELSE 270
#define ITERATE 271
#define INTERPRET 272
#define LEAVE 273
#define NOP 274
#define NUMERIC 275
#define PARSE 276
#define EXTERNAL 277
#define SOURCE 278
#define VAR 279
#define VALUE 280
#define WITH 281
#define PROCEDURE 282
#define EXPOSE 283
#define PULL 284
#define PUSH 285
#define QUEUE 286
#define SAY 287
#define RETURN 288
#define SELECT 289
#define WHEN 290
#define DROP 291
#define OTHERWISE 292
#define SIGNAL 293
#define ON 294
#define OFF 295
#define ERROR 296
#define SYNTAX 297
#define HALT 298
#define NOVALUE 299
#define TRACE 300
#define END 301
#define UPPER 302
#define ASSIGNMENTVARIABLE 303
#define STATSEP 304
#define FOREVER 305
#define DIGITS 306
#define FORM 307
#define FUZZ 308
#define SCIENTIFIC 309
#define ENGINEERING 310
#define NOT 311
#define CONCATENATE 312
#define MODULUS 313
#define GTE 314
#define GT 315
#define LTE 316
#define LT 317
#define DIFFERENT 318
#define EQUALEQUAL 319
#define NOTEQUALEQUAL 320
#define OFFSET 321
#define SPACE 322
#define EXP 323
#define XOR 324
#define PLACEHOLDER 325
#define NOTREADY 326
#define CONSYMBOL 327
#define SIMSYMBOL 328
#define EXFUNCNAME 329
#define INFUNCNAME 330
#define LABEL 331
#define DOVARIABLE 332
#define HEXSTRING 333
#define STRING 334
#define VERSION 335
#define LINEIN 336
#define WHATEVER 337
#define NAME 338
#define FAILURE 339
#define BINSTRING 340
#define OPTIONS 341
#define ENVIRONMENT 342
#define LOSTDIGITS 343
#define GTGT 344
#define LTLT 345
#define NOTGTGT 346
#define NOTLTLT 347
#define GTGTE 348
#define LTLTE 349
#define INPUT 350
#define OUTPUT 351
#define NORMAL 352
#define APPEND 353
#define REPLACE 354
#define STREAM 355
#define STEM 356
#define LIFO 357
#define FIFO 358
#define LOWER 359
#define CASELESS 360
#define CCAT 361
#define UPLUS 362
#define UMINUS 363




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE __reginalval;



