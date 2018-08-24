/***************************************************************
  
        bwd_fun.c       Function Table
                        for Bywater BASIC Interpreter
  
                        Copyright (c) 1993, Ted A. Campbell
                        Bywater Software
  
                        email: tcamp@delphi.com
  
        Copyright and Permissions Information:
  
        All U.S. and international rights are claimed by the author,
        Ted A. Campbell.
  
   This software is released under the terms of the GNU General
   Public License (GPL), which is distributed with this software
   in the file "COPYING".  The GPL specifies the terms under
   which users may copy and use the software in this distribution.
  
   A separate license is available for commercial distribution,
   for information on which you should contact the author.
  
***************************************************************/

/*---------------------------------------------------------------*/
/* NOTE: Modifications marked "JBV" were made by Jon B. Volkoff, */
/* 11/1995 (eidetics@cerf.net).                                  */
/*                                                               */
/* Those additionally marked with "DD" were at the suggestion of */
/* Dale DePriest (daled@cadence.com).                            */
/*                                                               */
/* Version 3.00 by Howard Wulf, AF5NE                            */
/*                                                               */
/* Version 3.10 by Howard Wulf, AF5NE                            */
/*                                                               */
/* Version 3.20 by Howard Wulf, AF5NE                            */
/*                                                               */
/*---------------------------------------------------------------*/


/* FUNCTION TABLE */

#include "bwbasic.h"

IntrinsicFunctionType IntrinsicFunctionTable[ /* NUM_FUNCTIONS */ ] =
{
{
  F_ABS_X_N, /* UniqueID */
  "N  = ABS( X )", /* Syntax */
  "The absolute value of X.",  /* Description */
  "ABS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_ACOS_X_N, /* UniqueID */
  "N  = ACOS( X )", /* Syntax */
  "The arccosine of X in radians, where 0 <= ACOS(X) <= PI.  X shall "
  "be in the range -1 <= X <= 1.",  /* Description */
  "ACOS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_ACS_X_N, /* UniqueID */
  "N  = ACS( X )", /* Syntax */
  "The arccosine of X in radians, where 0 <= ACS(X) <= PI.  X shall "
  "be in the range -1 <= X <= 1.",  /* Description */
  "ACS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | HB2 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_ACSD_X_N, /* UniqueID */
  "N  = ACSD( X )", /* Syntax */
  "The arccosine of X in degrees, where 0 <= ACSD(X) <= 180.  X "
  "shall be in the range -1 <= X <= 1.",  /* Description */
  "ACSD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_ACSG_X_N, /* UniqueID */
  "N  = ACSG( X )", /* Syntax */
  "The arccosine of X in gradians, where 0 <= ACS(X) <= 200.  X "
  "shall be in the range -1 <= X <= 1.",  /* Description */
  "ACSG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_ANGLE_X_Y_N, /* UniqueID */
  "N  = ANGLE( X, Y )", /* Syntax */
  "The angle in radians between the positive x-axis and the vector "
  "joining the origin to the point with coordinates (X, Y), where "
  "-PI < ANGLE(X,Y) <= PI.  X and Y must not both be 0.  Note that "
  "the counterclockwise is positive, e.g., ANGLE(1,1) = 45 degrees.",  /* Description */
  "ANGLE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1ANY | P2ANY, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_ARCCOS_X_N, /* UniqueID */
  "N  = ARCCOS( X )", /* Syntax */
  "The arccosine of X in radians, where 0 <= ARCCOS(X) <= PI.  X "
  "shall be in the range -1 <= X <= 1.",  /* Description */
  "ARCCOS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_ARCSIN_X_N, /* UniqueID */
  "N  = ARCSIN( X )", /* Syntax */
  "The arcsine of X in radians, where -PI/2 <= ARCSIN(X) <= PI/2; "
  "X shall be in the range -1 <= X <= 1.",  /* Description */
  "ARCSIN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 | R86 | T79 /* OptionVersionBitmask */
},
{
  F_ARCTAN_X_N, /* UniqueID */
  "N  = ARCTAN( X )", /* Syntax */
  "The arctangent of X in radians, i.e. the angle whose tangent "
  "is X, where -PI/2 < ARCTAN(X) < PI/2.",  /* Description */
  "ARCTAN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB1 | HB2 | R86 | T79 /* OptionVersionBitmask */
},
{
  F_ARGC_N, /* UniqueID */
  "N  = ARGC", /* Syntax */
  "The number of parameters passed to a FUNCTION or SUB.  If not "
  "in a FUNCTION or SUB, returns -1.",  /* Description */
  "ARGC", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_ARGT4_X_S, /* UniqueID */
  "S$ = ARGT$( X )", /* Syntax */
  "The type of the Xth parameter to a FUNCTION or SUB.  If the Xth "
  "parameter is a string, then return value is \"$\".  If the Xth "
  "parameter is a number, then return value is not \"$\".  X in [1,ARGC].",  /* Description */
  "ARGT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_ARGV_X_N, /* UniqueID */
  "N  = ARGV( X )", /* Syntax */
  "The numeric value of the Xth parameter to a FUNCTION or SUB. "
  " X in [1,ARGC] and ARGT$( X ) <> \"$\".",  /* Description */
  "ARGV", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_ARGV4_X_S, /* UniqueID */
  "S$ = ARGV$( X )", /* Syntax */
  "The string value of the Xth parameter to a FUNCTION or SUB.  "
  "X in [1,ARGC] and ARGT$( X ) = \"$\".",  /* Description */
  "ARGV$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_ASC_A_N, /* UniqueID */
  "N  = ASC( A$ )", /* Syntax */
  "The numeric code for the first letter in A$.  For example, ASC(\"ABC\") "
  "returns 65 on ASCII systems.",  /* Description */
  "ASC", /* Name */
  ByteTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_ASC_A_X_N, /* UniqueID */
  "N  = ASC( A$, X )", /* Syntax */
  "The numeric code of the Xth character in A$.  Same as ASC(MID$(A$,X)).",  /* Description */
  "ASC", /* Name */
  ByteTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2NUM, /* ParameterTypes */
  P1BYT | P2POS, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_ASCII_A_N, /* UniqueID */
  "N  = ASCII( A$ )", /* Syntax */
  "The numeric code for the first letter in A$.  For example, ASCII(\"ABC\") "
  "returns 65 on ASCII systems.",  /* Description */
  "ASCII", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_ASIN_X_N, /* UniqueID */
  "N  = ASIN( X )", /* Syntax */
  "The arcsine of X in radians, where -PI/2 <= ASIN(X) <= PI/2; "
  "X shall be in the range -1 <= X <= 1.",  /* Description */
  "ASIN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_ASN_X_N, /* UniqueID */
  "N  = ASN( X )", /* Syntax */
  "The arcsine of X in radians, where -PI/2 <= ASN(X) <= PI/2; X "
  "shall be in the range -1 <= X <= 1.",  /* Description */
  "ASN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | HB1 | HB2 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_ASND_X_N, /* UniqueID */
  "N  = ASND( X )", /* Syntax */
  "The arcsine of X in degrees, where -90 <= ASN(X) <= 90; X shall "
  "be in the range -1 <= X <= 1.",  /* Description */
  "ASND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_ASNG_X_N, /* UniqueID */
  "N  = ASNG( X )", /* Syntax */
  "The arcsine of X in gradians, where -100 <= ASNG(X) <= 100; X "
  "shall be in the range -1 <= X <= 1.",  /* Description */
  "ASNG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_ATAN_X_N, /* UniqueID */
  "N  = ATAN( X )", /* Syntax */
  "The arctangent of X in radians, i.e. the angle whose tangent "
  "is X, where -PI/2 < ATAN(X) < PI/2.",  /* Description */
  "ATAN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_ATN_X_N, /* UniqueID */
  "N  = ATN( X )", /* Syntax */
  "The arctangent of X in radians, i.e. the angle whose tangent "
  "is X, where -PI/2 < ATN(X) < PI/2.",  /* Description */
  "ATN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_ATND_X_N, /* UniqueID */
  "N  = ATND( X )", /* Syntax */
  "The arctangent of X in degrees, i.e. the angle whose tangent "
  "is X, where -90 < ATND(X) < 90.",  /* Description */
  "ATND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_ATNG_X_N, /* UniqueID */
  "N  = ATNG( X )", /* Syntax */
  "The arctangent of X in gradians, i.e. the angle whose tangent "
  "is X, where -100 < ATND(X) < 100.",  /* Description */
  "ATNG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_BASE_N, /* UniqueID */
  "N  = BASE", /* Syntax */
  "The current OPTION BASE setting.",  /* Description */
  "BASE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_BIN4_X_S, /* UniqueID */
  "S$ = BIN$( X )", /* Syntax */
  "The the binary (base 2) representation of X.",  /* Description */
  "BIN$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_BIN4_X_Y_S, /* UniqueID */
  "S$ = BIN$( X, Y )", /* Syntax */
  "The the binary (base 2) representation of X.  The result will "
  "be at least Y digits long. ",  /* Description */
  "BIN$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2BYT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_CATALOG_N, /* UniqueID */
  "N  = CATALOG", /* Syntax */
  "Displays all the file names.",  /* Description */
  "CATALOG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  D64 | G74 | G65 | G67 | D71 /* OptionVersionBitmask */
},
{
  F_CATALOG_A_N, /* UniqueID */
  "N  = CATALOG( A$ )", /* Syntax */
  "Displays all the file names matching A$.",  /* Description */
  "CATALOG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  D64 | G74 | G65 | G67 | D71 /* OptionVersionBitmask */
},
{
  F_CCUR_X_N, /* UniqueID */
  "N  = CCUR( X )", /* Syntax */
  "The currency (64-bit) integer value of X.",  /* Description */
  "CCUR", /* Name */
  CurrencyTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1CUR, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_CDBL_X_N, /* UniqueID */
  "N  = CDBL( X )", /* Syntax */
  "The double-precision value of X.",  /* Description */
  "CDBL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1DBL, /* ParameterTests */
  B15 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_CEIL_X_N, /* UniqueID */
  "N  = CEIL( X )", /* Syntax */
  "The smallest integer not less than X.",  /* Description */
  "CEIL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_CHAR_X_Y_S, /* UniqueID */
  "S$ = CHAR( X, Y )", /* Syntax */
  "The string Y bytes long consisting of CHR$(X).  Same as STRING$(Y,X).",  /* Description */
  "CHAR", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1BYT | P2LEN, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_CHAR4_X_S, /* UniqueID */
  "S$ = CHAR$( X )", /* Syntax */
  "The one-character string with the character corresponding to "
  "the numeric code X.  On ASCII systems, CHAR$(65) returns \"A\".",  /* Description */
  "CHAR$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_CHDIR_A_N, /* UniqueID */
  "N  = CHDIR( A$ )", /* Syntax */
  "Changes to the directory named to A$.",  /* Description */
  "CHDIR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_CHR_X_S, /* UniqueID */
  "S$ = CHR( X )", /* Syntax */
  "The one-character string with the character corresponding to "
  "the numeric code X.  On ASCII systems, CHR(65) returns \"A\".",  /* Description */
  "CHR", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_CHR_X_S, /* UniqueID */
  "S$ = CHR( X )", /* Syntax */
  "The output of PRINT X.  A$ = CHR(X) is the opposite of X = NUM(A$).",  /* Description */
  "CHR", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  S70 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_CHR4_X_S, /* UniqueID */
  "S$ = CHR$( X )", /* Syntax */
  "The one-character string with the character corresponding to "
  "the numeric code X.  On ASCII systems, CHR$(65) returns \"A\".",  /* Description */
  "CHR$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | D71 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_CIN_X_N, /* UniqueID */
  "N  = CIN( X )", /* Syntax */
  "If file # X is <= 0 then returns -1.  If file # X is not opened "
  "for reading then returns -1.  If file # X is at EOF then returns "
  "-1,  Otherwise returns the next byte value read from file X.",  /* Description */
  "CIN", /* Name */
  IntegerTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  H80 /* OptionVersionBitmask */
},
{
  F_CINT_X_N, /* UniqueID */
  "N  = CINT( X )", /* Syntax */
  "The short (16-bit) integer value of X.",  /* Description */
  "CINT", /* Name */
  IntegerTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_CLG_X_N, /* UniqueID */
  "N  = CLG( X )", /* Syntax */
  "The common logarithm of X; X shall be greater than zero.",  /* Description */
  "CLG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_CLK_X_N, /* UniqueID */
  "N  = CLK( X )", /* Syntax */
  "The time elapsed since the previous midnight, expressed in hours; "
  "e.g., the value of CLK at 3:15 PM is 15.25.  If there is no clock "
  "available, then the value of CLK shall be -1.  The value of CLK "
  "at midnight shall be zero (not 24).  The value of X is ignored.",  /* Description */
  "CLK", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  G74 | G65 | G67 /* OptionVersionBitmask */
},
{
  F_CLK_X_S, /* UniqueID */
  "S$ = CLK( X )", /* Syntax */
  "The time of day in 24-hour notation according to ISO 3307.  For "
  "example, the value of CLK at 11:15 AM is \"11:15:00\".  If there "
  "is no clock available, then the value of CLK shall be \"99:99:99\". "
  " The value of TIME$ at midnight is \"00:00:00\".  The value of "
  "parameter X is ignored.",  /* Description */
  "CLK", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_CLK4_S, /* UniqueID */
  "S$ = CLK$", /* Syntax */
  "The time of day in 24-hour notation according to ISO 3307.  For "
  "example, the value of TIME$ at 11:15 AM is \"11:15:00\".  If there "
  "is no clock available, then the value of TIME$ shall be \"99:99:99\". "
  " The value of TIME$ at midnight is \"00:00:00\".",  /* Description */
  "CLK$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | G74 | HB1 | HB2 | G67 /* OptionVersionBitmask */
},
{
  F_CLNG_X_N, /* UniqueID */
  "N  = CLNG( X )", /* Syntax */
  "The long (32-bit) integer value of X.",  /* Description */
  "CLNG", /* Name */
  LongTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LNG, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_CLOG_X_N, /* UniqueID */
  "N  = CLOG( X )", /* Syntax */
  "The common logarithm of X; X shall be greater than zero.",  /* Description */
  "CLOG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_CLOSE_N, /* UniqueID */
  "N  = CLOSE", /* Syntax */
  "Close all open files.",  /* Description */
  "CLOSE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | H14 | H80 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_CLOSE_X_N, /* UniqueID */
  "N  = CLOSE( X )", /* Syntax */
  "Close file number X.",  /* Description */
  "CLOSE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | C77 | H14 | HB2 | H80 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_CLS_N, /* UniqueID */
  "N  = CLS", /* Syntax */
  "Clears the screen. Cursor is positioned at row 1, column 1.",  /* Description */
  "CLS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | H14 | HB1 | HB2 | M80 | T80 /* OptionVersionBitmask */
},
{
  F_CNTRL_X_Y_N, /* UniqueID */
  "N  = CNTRL( X, Y )", /* Syntax */
  "CNTRL 1,Y sets the number of significant digits to print.  CNTRL "
  "3,Y sets the width of the print zones.",  /* Description */
  "CNTRL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2INT, /* ParameterTests */
  H80 /* OptionVersionBitmask */
},
{
  F_CODE_A_N, /* UniqueID */
  "N  = CODE( A$ )", /* Syntax */
  "The numeric code for the first letter in A$.  For example, CODE(\"ABC\") "
  "returns 65 on ASCII systems.",  /* Description */
  "CODE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | H14 | HB2 /* OptionVersionBitmask */
},
{
  F_COLOR_X_Y_N, /* UniqueID */
  "N  = COLOR( X, Y )", /* Syntax */
  "Sets the foreground text color to X, and the background text "
  "color to Y.",  /* Description */
  "COLOR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1BYT | P2BYT, /* ParameterTests */
  B15 | H14 | M80 | T80 /* OptionVersionBitmask */
},
{
  F_COMMAND4_S, /* UniqueID */
  "S$ = COMMAND$", /* Syntax */
  "All command line parameters, concatenated with one space between "
  "each.  Support for parameters varies by operating system, compiler, "
  "and so on.",  /* Description */
  "COMMAND$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_COMMAND4_X_S, /* UniqueID */
  "S$ = COMMAND$( X )", /* Syntax */
  "The command line parameters.  COMMAND$(0) is the BASIC program "
  "name.  COMMAND$(1) is the first parameter after the BASIC program "
  "name, and so on.  Support for parameters varies by operating "
  "system, compiler, and so on.  X in [0..9]  ",  /* Description */
  "COMMAND$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | C77 | H14 /* OptionVersionBitmask */
},
{
  F_COS_X_N, /* UniqueID */
  "N  = COS( X )", /* Syntax */
  "The cosine of X, where X is in radians.",  /* Description */
  "COS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_COSD_X_N, /* UniqueID */
  "N  = COSD( X )", /* Syntax */
  "The cosine of X, where X is in degrees.",  /* Description */
  "COSD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_COSG_X_N, /* UniqueID */
  "N  = COSG( X )", /* Syntax */
  "The cosine of X, where X is in gradians.",  /* Description */
  "COSG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_COSH_X_N, /* UniqueID */
  "N  = COSH( X )", /* Syntax */
  "The hyperbolic cosine of X.",  /* Description */
  "COSH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_COT_X_N, /* UniqueID */
  "N  = COT( X )", /* Syntax */
  "The cotangent of X, where X is in radians.",  /* Description */
  "COT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | E86 | G74 | G67 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_COUNT_N, /* UniqueID */
  "N  = COUNT", /* Syntax */
  "The current cursor position in the line.",  /* Description */
  "COUNT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_CSC_X_N, /* UniqueID */
  "N  = CSC( X )", /* Syntax */
  "The cosecant of X, where X is in radians.",  /* Description */
  "CSC", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | E86 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_CSH_X_N, /* UniqueID */
  "N  = CSH( X )", /* Syntax */
  "The hyperbolic cosine of X.",  /* Description */
  "CSH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_CSNG_X_N, /* UniqueID */
  "N  = CSNG( X )", /* Syntax */
  "The single-precision value of X.",  /* Description */
  "CSNG", /* Name */
  SingleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1FLT, /* ParameterTests */
  B15 | B93 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_CUR_X_Y_S, /* UniqueID */
  "S$ = CUR( X, Y )", /* Syntax */
  "Locates the cursor to row X, column Y.",  /* Description */
  "CUR", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1BYT | P2BYT, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_CVC_A_N, /* UniqueID */
  "N  = CVC( A$ )", /* Syntax */
  "The currency (64-bit) integer value in A$, which was created "
  "by MKC$.",  /* Description */
  "CVC", /* Name */
  CurrencyTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1CUR, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_CVD_A_N, /* UniqueID */
  "N  = CVD( A$ )", /* Syntax */
  "The double-precision value in A$, which was created by MKD$.",  /* Description */
  "CVD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1DBL, /* ParameterTests */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_CVI_A_N, /* UniqueID */
  "N  = CVI( A$ )", /* Syntax */
  "The short (16-bit) integer value in A$, which was created by "
  "MKI$.",  /* Description */
  "CVI", /* Name */
  IntegerTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_CVL_A_N, /* UniqueID */
  "N  = CVL( A$ )", /* Syntax */
  "The long (32-bit) integer value in A$, which was created by MKL$.",  /* Description */
  "CVL", /* Name */
  LongTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1LNG, /* ParameterTests */
  B15 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_CVS_A_N, /* UniqueID */
  "N  = CVS( A$ )", /* Syntax */
  "The single-precision value in A$, which was created by MKS$.",  /* Description */
  "CVS", /* Name */
  SingleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1FLT, /* ParameterTests */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_DAT4_S, /* UniqueID */
  "S$ = DAT$", /* Syntax */
  "The current date based on the internal clock as a string in the "
  "format set by OPTION DATE.",  /* Description */
  "DAT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  G74 | G67 /* OptionVersionBitmask */
},
{
  F_DATE_N, /* UniqueID */
  "N  = DATE", /* Syntax */
  "The current date in decimal form YYYDDD, where YYY are the number "
  "of years since 1900 and DDD is the ordinal number of the current "
  "day of the year; e.g., the value of DATE on May 9, 1977 was 77129. "
  " If there is no calendar available, then the value of DATE shall "
  "be -1.",  /* Description */
  "DATE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_DATE4_S, /* UniqueID */
  "S$ = DATE$", /* Syntax */
  "The current date based on the internal clock as a string in the "
  "format set by OPTION DATE.",  /* Description */
  "DATE$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | E86 | H14 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_DATE4_X_S, /* UniqueID */
  "S$ = DATE$( X )", /* Syntax */
  "The current date based on the internal clock as a string in the "
  "format set by OPTION DATE.  The value of parameter X is ignored.",  /* Description */
  "DATE$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  D71 /* OptionVersionBitmask */
},
{
  F_DEG_N, /* UniqueID */
  "N  = DEG", /* Syntax */
  "Configures the math functions to accept and return angles in "
  "degrees.",  /* Description */
  "DEG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_DEG_X_N, /* UniqueID */
  "N  = DEG( X )", /* Syntax */
  "When X is zero sets RADIANS.  When X is non-zero sets DEGREES.",  /* Description */
  "DEG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  R86 /* OptionVersionBitmask */
},
{
  F_DEG_X_N, /* UniqueID */
  "N  = DEG( X )", /* Syntax */
  "The number of degrees in X radians.",  /* Description */
  "DEG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | E86 | H14 | HB1 | HB2 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_DEGREE_N, /* UniqueID */
  "N  = DEGREE", /* Syntax */
  "Configures the math functions to accept and return angles in "
  "degrees.",  /* Description */
  "DEGREE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_DEGREE_X_N, /* UniqueID */
  "N  = DEGREE( X )", /* Syntax */
  "The number of degrees in X radians.",  /* Description */
  "DEGREE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_DET_N, /* UniqueID */
  "N  = DET", /* Syntax */
  "The determinant of the last MAT INV.  Zero means error.",  /* Description */
  "DET", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | S70 | G74 | H14 | HB2 | G67 | D71 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_DIGITS_X_N, /* UniqueID */
  "N  = DIGITS( X )", /* Syntax */
  "X is the number of significiant digits to print for numbers (0..17). "
  " If X = 0 then disabled.  ",  /* Description */
  "DIGITS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_DIGITS_X_Y_N, /* UniqueID */
  "N  = DIGITS( X, Y )", /* Syntax */
  "X is the number of significiant digits to print for numbers (0..17). "
  " If X = 0 then disabled.  Y is the number of decimal places to "
  "round (0..17).  If Y = 0 then disabled.",  /* Description */
  "DIGITS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1BYT | P2BYT, /* ParameterTests */
  HB2 | R86 | T79 /* OptionVersionBitmask */
},
{
  F_DIM_N, /* UniqueID */
  "N  = DIM( ... )", /* Syntax */
  "DIM( arrayname ).  The total number of dimensions of the array.",  /* Description */
  "DIM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_DPEEK_X_N, /* UniqueID */
  "N  = DPEEK( X )", /* Syntax */
  "The value read from hardware address X. Causes ERROR 73.",  /* Description */
  "DPEEK", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LNG, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_DPOKE_X_Y_N, /* UniqueID */
  "N  = DPOKE( X, Y )", /* Syntax */
  "Sends Y to hardware address X.  Causes ERROR 73.",  /* Description */
  "DPOKE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LNG | P2INT, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_EDIT4_A_X_S, /* UniqueID */
  "S$ = EDIT$( A$, X )", /* Syntax */
  "Edits the characters of A$, according to the bits set in X.\n1 "
  "= discard parity bit.\n2 = discard all spaces.\n4 = discard all "
  "carriage returns, line feeds, form feeds, deletes, escapes and "
  "nulls.\n8 = discard leading spaces.\n16 = replace multiple spaces "
  "with one space.\n32 = change lower case to upper case.\n64 = change "
  "[ to ( and change ] to ).\n128 = discard trailing spacess.\n256 "
  "= suppress editing within quotes.\nThe order in which edits occur "
  "is implementation defined.",  /* Description */
  "EDIT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2NUM, /* ParameterTypes */
  P1ANY | P2INT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_ENVIRON_A_N, /* UniqueID */
  "N  = ENVIRON( A$ )", /* Syntax */
  "Sends the environment variable expression contained in A$ to "
  "the host operating system.  A$ must contain the  \"=\" character.",  /* Description */
  "ENVIRON", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_ENVIRON4_A_S, /* UniqueID */
  "S$ = ENVIRON$( A$ )", /* Syntax */
  "The value of the environment variable named A$.",  /* Description */
  "ENVIRON$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_EOF_X_N, /* UniqueID */
  "N  = EOF( X )", /* Syntax */
  "If device number X is at the end-of-file, then -1, otherwise "
  "0.",  /* Description */
  "EOF", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | H14 | H80 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_EPS_X_N, /* UniqueID */
  "N  = EPS( X )", /* Syntax */
  "The maximum of (X-X1,X2-X, sigma) where X1 and X2 are the predecessor "
  "and successor of X and signma is the smallest positive value "
  "representable.  If X has no predecessor the X1=X and if X has "
  "no successor the X2=X.  Note EPS(0) is the smallest positive "
  "number representable by the implementation, and is therefor implementation-defined. "
  " Note also that EPS may produce different results for different "
  "arithmetic options (see OPTION ARITHMETIC).",  /* Description */
  "EPS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_ERL_N, /* UniqueID */
  "N  = ERL", /* Syntax */
  "The line number of the most recent error.",  /* Description */
  "ERL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_ERR_N, /* UniqueID */
  "N  = ERR", /* Syntax */
  "The error number of the most recent error.",  /* Description */
  "ERR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_ERR4_S, /* UniqueID */
  "S$ = ERR$", /* Syntax */
  "The last error message.",  /* Description */
  "ERR$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_ERRL_N, /* UniqueID */
  "N  = ERRL", /* Syntax */
  "The line number of the most recent error.",  /* Description */
  "ERRL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_ERRN_N, /* UniqueID */
  "N  = ERRN", /* Syntax */
  "The error number of the most recent error.",  /* Description */
  "ERRN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_ERROR_X_N, /* UniqueID */
  "N  = ERROR( X )", /* Syntax */
  "Simulate the error number in X.",  /* Description */
  "ERROR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_ERROR_X_A_N, /* UniqueID */
  "N  = ERROR( X, A$ )", /* Syntax */
  "Simulate the error number in X, with a custom message in A$.",  /* Description */
  "ERROR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2STR, /* ParameterTypes */
  P1BYT | P2ANY, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_ERROR4_S, /* UniqueID */
  "S$ = ERROR$", /* Syntax */
  "The last error message.",  /* Description */
  "ERROR$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_EXAM_X_N, /* UniqueID */
  "N  = EXAM( X )", /* Syntax */
  "The value read from hardware address X.  Causes ERROR 73.",  /* Description */
  "EXAM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LNG, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_EXEC_A_N, /* UniqueID */
  "N  = EXEC( A$ )", /* Syntax */
  "The exit code resulting from the execution of an operating system "
  "command.",  /* Description */
  "EXEC", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_EXF_N, /* UniqueID */
  "N  = EXF( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "EXF", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  D70 /* OptionVersionBitmask */
},
{
  F_EXP_X_N, /* UniqueID */
  "N  = EXP( X )", /* Syntax */
  "The exponential value of X, i.e., the value of the base of natural "
  "logarithms (e = 2.71828) raised to the power of X;  if EXP(X) "
  "is less that machine infinitesimal, then its value shall be replaced "
  "with zero.",  /* Description */
  "EXP", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_FALSE_N, /* UniqueID */
  "N  = FALSE", /* Syntax */
  "The value of FALSE.",  /* Description */
  "FALSE", /* Name */
  IntegerTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_FETCH_X_N, /* UniqueID */
  "N  = FETCH( X )", /* Syntax */
  "The value read from hardware address X.  Causes ERROR 73.",  /* Description */
  "FETCH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LNG, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_FILEATTR_X_Y_N, /* UniqueID */
  "N  = FILEATTR( X, Y )", /* Syntax */
  "For file X, if Y = 1 then returns open mode, otherwise returns "
  "zero.",  /* Description */
  "FILEATTR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2INT, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_FILES_N, /* UniqueID */
  "N  = FILES", /* Syntax */
  "Displays all the file names.",  /* Description */
  "FILES", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_FILES_A_N, /* UniqueID */
  "N  = FILES( A$ )", /* Syntax */
  "Displays all the file names matching A$.",  /* Description */
  "FILES", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_FILL_X_Y_N, /* UniqueID */
  "N  = FILL( X, Y )", /* Syntax */
  "Sends Y to hardware address X.  Causes ERROR 73.",  /* Description */
  "FILL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LNG | P2BYT, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_FIX_X_N, /* UniqueID */
  "N  = FIX( X )", /* Syntax */
  "The truncated integer, part of X. FIX (X) is equivalent to SGN(X)*INT(ABS(X)). "
  "The major difference between FIX and INT is that FIX does not "
  "return the next lower number for negative X.",  /* Description */
  "FIX", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | G65 | G67 | M80 | D71 | I70 | I73 | T80 /* OptionVersionBitmask */
},
{
  F_FLOAT_X_N, /* UniqueID */
  "N  = FLOAT( X )", /* Syntax */
  "The rounded integer valuet of X.",  /* Description */
  "FLOAT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  C77 /* OptionVersionBitmask */
},
{
  F_FLOW_N, /* UniqueID */
  "N  = FLOW", /* Syntax */
  "Turn tracing ON",  /* Description */
  "FLOW", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_FP_X_N, /* UniqueID */
  "N  = FP( X )", /* Syntax */
  "The fractional part of X, i.e. X - IP(X).",  /* Description */
  "FP", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 | H14 /* OptionVersionBitmask */
},
{
  F_FRAC_X_N, /* UniqueID */
  "N  = FRAC( X )", /* Syntax */
  "The fractional part of X, i.e. X - IP(X).",  /* Description */
  "FRAC", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | H14 | HB2 /* OptionVersionBitmask */
},
{
  F_FRE_N, /* UniqueID */
  "N  = FRE", /* Syntax */
  "The number of bytes of available memory.  This function is provided "
  "for backward compatibility only and it always returns a fixed "
  "value of 32000.",  /* Description */
  "FRE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | C77 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_FRE_A_N, /* UniqueID */
  "N  = FRE( A$ )", /* Syntax */
  "The number of bytes of available memory.  This function is provided "
  "for backward compatibility only and it always returns a fixed "
  "value of 32000.The value of A$ is ignored.",  /* Description */
  "FRE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_FRE_X_N, /* UniqueID */
  "N  = FRE( X )", /* Syntax */
  "The number of bytes of available memory.  This function is provided "
  "for backward compatibility only and it always returns a fixed "
  "value of 32000.  The value of X is ignored.",  /* Description */
  "FRE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_FREE_N, /* UniqueID */
  "N  = FREE", /* Syntax */
  "The number of bytes of available memory.  This function is provided "
  "for backward compatibility only and it always returns a fixed "
  "value of 32000.",  /* Description */
  "FREE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  HB1 | HB2 | H80 /* OptionVersionBitmask */
},
{
  F_FREE_X_N, /* UniqueID */
  "N  = FREE( X )", /* Syntax */
  "The number of bytes of available memory.  This function is provided "
  "for backward compatibility only and it always returns a fixed "
  "value of 32000.  The value of X is ignored.",  /* Description */
  "FREE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_FREE_A_N, /* UniqueID */
  "N  = FREE( A$ )", /* Syntax */
  "The number of bytes of available memory.  This function is provided "
  "for backward compatibility only and it always returns a fixed "
  "value of 32000.The value of A$ is ignored.",  /* Description */
  "FREE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_FREEFILE_N, /* UniqueID */
  "N  = FREEFILE", /* Syntax */
  "The next available file number.",  /* Description */
  "FREEFILE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | H14 /* OptionVersionBitmask */
},
{
  F_GET_X_N, /* UniqueID */
  "N  = GET( X )", /* Syntax */
  "For file X, gets the next available record.",  /* Description */
  "GET", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_GET_X_N, /* UniqueID */
  "N  = GET( X )", /* Syntax */
  "Get character code from input.  The value of X is ignored. Similar "
  "to ASC(INKEY$).",  /* Description */
  "GET", /* Name */
  IntegerTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  D73 /* OptionVersionBitmask */
},
{
  F_GET_X_Y_N, /* UniqueID */
  "N  = GET( X, Y )", /* Syntax */
  "For file X, gets record number Y.  The first record number is "
  "1.",  /* Description */
  "GET", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2INT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_GRAD_N, /* UniqueID */
  "N  = GRAD", /* Syntax */
  "Configures the math functions to accept and return angles in "
  "gradians.",  /* Description */
  "GRAD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_GRADIAN_N, /* UniqueID */
  "N  = GRADIAN", /* Syntax */
  "Configures the math functions to accept and return angles in "
  "gradians.",  /* Description */
  "GRADIAN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_HCS_X_N, /* UniqueID */
  "N  = HCS( X )", /* Syntax */
  "The hyperbolic cosine of X.",  /* Description */
  "HCS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  S70 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_HEX_A_N, /* UniqueID */
  "N  = HEX( A$ )", /* Syntax */
  "The numeric value of the hexadecimal string in A$.  For example, "
  "HEX(\"FFFF\") returns 65535.",  /* Description */
  "HEX", /* Name */
  ByteTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_HEX4_X_S, /* UniqueID */
  "S$ = HEX$( X )", /* Syntax */
  "The the hexadecimal (base 16) representation of X.",  /* Description */
  "HEX$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_HEX4_X_Y_S, /* UniqueID */
  "S$ = HEX$( X, Y )", /* Syntax */
  "The the hexadecimal (base 16) representation of X.  The result "
  "will be at least Y digits long. ",  /* Description */
  "HEX$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2BYT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_HOME_N, /* UniqueID */
  "N  = HOME", /* Syntax */
  "Clears the screen. Cursor is positioned at row 1, column 1.",  /* Description */
  "HOME", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | H14 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_HSN_X_N, /* UniqueID */
  "N  = HSN( X )", /* Syntax */
  "The hyperbolic sine of X.",  /* Description */
  "HSN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  S70 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_HTN_X_N, /* UniqueID */
  "N  = HTN( X )", /* Syntax */
  "The hyperbolic tangent of X.",  /* Description */
  "HTN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  S70 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_INCH4_S, /* UniqueID */
  "S$ = INCH$", /* Syntax */
  "The keypress, if available. If a keypress is not available, then "
  "immediately returns an empty string.  If not supported by the "
  "platform, then always returns an empty string, so use INPUT$(1) "
  "instead.",  /* Description */
  "INCH$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_INDEX_A_B_N, /* UniqueID */
  "N  = INDEX( A$, B$ )", /* Syntax */
  "The position at which B$ occurs in A$, beginning at position "
  "1.",  /* Description */
  "INDEX", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2STR, /* ParameterTypes */
  P1ANY | P2ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_INITIALIZE_N, /* UniqueID */
  "N  = INITIALIZE", /* Syntax */
  "This function is provided for backward compatibility only and "
  "it always returns a fixed value of 0.",  /* Description */
  "INITIALIZE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  C77 /* OptionVersionBitmask */
},
{
  F_INKEY4_S, /* UniqueID */
  "S$ = INKEY$", /* Syntax */
  "The keypress, if available. If a keypress is not available, then "
  "immediately returns an empty string.  If not supported by the "
  "platform, then always returns an empty string, so use INPUT$(1) "
  "instead.",  /* Description */
  "INKEY$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_INP_X_N, /* UniqueID */
  "N  = INP( X )", /* Syntax */
  "The value read from machine port X.  Causes ERROR 73.",  /* Description */
  "INP", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | C77 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_INPUT4_X_S, /* UniqueID */
  "S$ = INPUT$( X )", /* Syntax */
  "The string of X characters, read from the terminal.",  /* Description */
  "INPUT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LEN, /* ParameterTests */
  B15 | H14 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_INPUT4_X_Y_S, /* UniqueID */
  "S$ = INPUT$( X, Y )", /* Syntax */
  "The string of X characters, read from file Y.",  /* Description */
  "INPUT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LEN | P2INT, /* ParameterTests */
  B15 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_INSTR_A_B_N, /* UniqueID */
  "N  = INSTR( A$, B$ )", /* Syntax */
  "The position at which B$ occurs in A$, beginning at position "
  "1.",  /* Description */
  "INSTR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2STR, /* ParameterTypes */
  P1ANY | P2ANY, /* ParameterTests */
  B15 | B93 | H14 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_INSTR_A_B_X_N, /* UniqueID */
  "N  = INSTR( A$, B$, X )", /* Syntax */
  "The position at which B$ occurs in A$, beginning at position "
  "X.",  /* Description */
  "INSTR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1STR | P2STR | P3NUM, /* ParameterTypes */
  P1ANY | P2ANY | P3POS, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_INSTR_X_A_B_N, /* UniqueID */
  "N  = INSTR( X, A$, B$ )", /* Syntax */
  "The position at which B$ occurs in A$, beginning at position "
  "X.",  /* Description */
  "INSTR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1NUM | P2STR | P3STR, /* ParameterTypes */
  P1POS | P2ANY | P3ANY, /* ParameterTests */
  B15 | B93 | H14 | HB2 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_INT_X_N, /* UniqueID */
  "N  = INT( X )", /* Syntax */
  "The largest integer not greater than X; e.g. INT(1.3) = 1 and "
  "INT(-1.3) = 2.",  /* Description */
  "INT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_INT5_X_N, /* UniqueID */
  "N  = INT%( X )", /* Syntax */
  "The rounded integer valuet of X.",  /* Description */
  "INT%", /* Name */
  LongTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  C77 | H14 /* OptionVersionBitmask */
},
{
  F_IP_X_N, /* UniqueID */
  "N  = IP( X )", /* Syntax */
  "The integer part of X, i.e., SGN(X)*INT(ABS(X)).",  /* Description */
  "IP", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 | H14 /* OptionVersionBitmask */
},
{
  F_KEY_S, /* UniqueID */
  "S$ = KEY", /* Syntax */
  "The keypress, if available. If a keypress is not available, then "
  "immediately returns an empty string.  If not supported by the "
  "platform, then always returns an empty string, so use INPUT$(1) "
  "instead.",  /* Description */
  "KEY", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_KEY4_S, /* UniqueID */
  "S$ = KEY$", /* Syntax */
  "The keypress, if available. If a keypress is not available, then "
  "immediately returns an empty string.  If not supported by the "
  "platform, then always returns an empty string, so use INPUT$(1) "
  "instead.",  /* Description */
  "KEY$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_KILL_A_N, /* UniqueID */
  "N  = KILL( A$ )", /* Syntax */
  "Removes the file named in A$.",  /* Description */
  "KILL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_LBOUND_N, /* UniqueID */
  "N  = LBOUND( ... )", /* Syntax */
  "LBOUND( arrayname [, dimension] ).  The lower bound of the array. "
  " The dimension defaults to 1.  dimension in [1,DIM(arrayname)]",  /* Description */
  "LBOUND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_LCASE4_A_S, /* UniqueID */
  "S$ = LCASE$( A$ )", /* Syntax */
  "The string of characters from the value associatedwith A$ by "
  "replacing each upper-case-letter in the string by its lower-case "
  "version.",  /* Description */
  "LCASE$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | C77 | E86 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_LEFT_A_X_S, /* UniqueID */
  "S$ = LEFT( A$, X )", /* Syntax */
  "The X left-most characters of A$, beginning from postion 1.",  /* Description */
  "LEFT", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2NUM, /* ParameterTypes */
  P1ANY | P2LEN, /* ParameterTests */
  HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_LEFT4_A_X_S, /* UniqueID */
  "S$ = LEFT$( A$, X )", /* Syntax */
  "The X left-most characters of A$, beginning from postion 1.",  /* Description */
  "LEFT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2NUM, /* ParameterTypes */
  P1ANY | P2LEN, /* ParameterTests */
  B15 | B93 | C77 | E86 | H14 | HB1 | HB2 | H80 | M80 | D71 | T80 | V09 /* OptionVersionBitmask */
},
{
  F_LEN_A_N, /* UniqueID */
  "N  = LEN( A$ )", /* Syntax */
  "The length of A$.",  /* Description */
  "LEN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_LGT_X_N, /* UniqueID */
  "N  = LGT( X )", /* Syntax */
  "The common logarithm of X; X shall be greater than zero.",  /* Description */
  "LGT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  S70 | HB1 | HB2 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_LIN_X_S, /* UniqueID */
  "S$ = LIN( X )", /* Syntax */
  "The string X bytes long of newline characters.",  /* Description */
  "LIN", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LEN, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_LN_X_N, /* UniqueID */
  "N  = LN( X )", /* Syntax */
  "The natural logarithm of X; X shall be greater than zero.",  /* Description */
  "LN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_LNO_X_N, /* UniqueID */
  "N  = LNO( X )", /* Syntax */
  "Returns X.",  /* Description */
  "LNO", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  H80 /* OptionVersionBitmask */
},
{
  F_LOC_X_N, /* UniqueID */
  "N  = LOC( X )", /* Syntax */
  "The location of file X; the next record that GET or PUT statements "
  "will use.",  /* Description */
  "LOC", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | E86 | G74 | H14 | G65 | G67 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_LOCATE_X_Y_N, /* UniqueID */
  "N  = LOCATE( X, Y )", /* Syntax */
  "Locates the cursor to row X, column Y.",  /* Description */
  "LOCATE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1BYT | P2BYT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_LOCK_X_N, /* UniqueID */
  "N  = LOCK( X )", /* Syntax */
  "Lock file number X.  Causes ERROR 73.",  /* Description */
  "LOCK", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_LOF_X_N, /* UniqueID */
  "N  = LOF( X )", /* Syntax */
  "The length of file X.",  /* Description */
  "LOF", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | E86 | G74 | H14 | G65 | G67 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_LOG_X_N, /* UniqueID */
  "N  = LOG( X )", /* Syntax */
  "The natural logarithm of X; X shall be greater than zero.",  /* Description */
  "LOG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_LOG10_X_N, /* UniqueID */
  "N  = LOG10( X )", /* Syntax */
  "The common logarithm of X; X shall be greater than zero.",  /* Description */
  "LOG10", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  B15 | E86 | H14 | HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_LOG2_X_N, /* UniqueID */
  "N  = LOG2( X )", /* Syntax */
  "The base 2 logarithm of X; X shall be greater than zero.",  /* Description */
  "LOG2", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  B15 | E86 | H14 /* OptionVersionBitmask */
},
{
  F_LOGE_X_N, /* UniqueID */
  "N  = LOGE( X )", /* Syntax */
  "The natural logarithm of X; X shall be greater than zero.",  /* Description */
  "LOGE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_LOWER4_A_S, /* UniqueID */
  "S$ = LOWER$( A$ )", /* Syntax */
  "The string of characters from the value associatedwith A$ by "
  "replacing each upper-case-letter in the string by its lower-case "
  "version.",  /* Description */
  "LOWER$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_LPOS_N, /* UniqueID */
  "N  = LPOS", /* Syntax */
  "The current cursor position in the line for the printer.",  /* Description */
  "LPOS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_LTRIM4_A_S, /* UniqueID */
  "S$ = LTRIM$( A$ )", /* Syntax */
  "The string of characters resulting from the value associated "
  "with A$ by deleting all leading space characters.",  /* Description */
  "LTRIM$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_LTW_X_N, /* UniqueID */
  "N  = LTW( X )", /* Syntax */
  "The base 2 logarithm of X; X shall be greater than zero.",  /* Description */
  "LTW", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GTZ, /* ParameterTests */
  S70 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_LWIDTH_X_N, /* UniqueID */
  "N  = LWIDTH( X )", /* Syntax */
  "For printer, sets the line width to X. Zero means no wrapping "
  "will occur.",  /* Description */
  "LWIDTH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_MATCH_A_B_X_N, /* UniqueID */
  "N  = MATCH( A$, B$, X )", /* Syntax */
  "The character position, within the value assocated with A$, of "
  "the first character of the first occurence of the value associated "
  "with B$, starting at the Xth character of A$.  If there is not "
  "such occurence, then the value returned is zero.",  /* Description */
  "MATCH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1STR | P2STR | P3NUM, /* ParameterTypes */
  P1ANY | P2ANY | P3POS, /* ParameterTests */
  C77 | H14 | H80 /* OptionVersionBitmask */
},
{
  F_MAX_A_B_S, /* UniqueID */
  "S$ = MAX( A$, B$ )", /* Syntax */
  "The larger of the parameters.",  /* Description */
  "MAX", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2STR, /* ParameterTypes */
  P1ANY | P2ANY, /* ParameterTests */
  B15 | S70 | HB2 | H80 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_MAX_X_Y_N, /* UniqueID */
  "N  = MAX( X, Y )", /* Syntax */
  "The larger of the parameters.",  /* Description */
  "MAX", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1ANY | P2ANY, /* ParameterTests */
  B15 | S70 | E86 | H14 | HB2 | H80 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_MAXBYT_N, /* UniqueID */
  "N  = MAXBYT", /* Syntax */
  "The largest finite positive number representable as a BYTE; implementation-defined.",  /* Description */
  "MAXBYT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MAXCUR_N, /* UniqueID */
  "N  = MAXCUR", /* Syntax */
  "The largest finite positive number representable as a CURRENCY; "
  "implementation-defined.",  /* Description */
  "MAXCUR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MAXDBL_N, /* UniqueID */
  "N  = MAXDBL", /* Syntax */
  "The largest finite positive number representable as a DOUBLE; "
  "implementation-defined.",  /* Description */
  "MAXDBL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MAXDEV_N, /* UniqueID */
  "N  = MAXDEV", /* Syntax */
  "The largest finite positive number useable as a FILE NUMBER; "
  "implementation-defined.",  /* Description */
  "MAXDEV", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MAXINT_N, /* UniqueID */
  "N  = MAXINT", /* Syntax */
  "The largest finite positive number representable as an INTEGER; "
  "implementation-defined.",  /* Description */
  "MAXINT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MAXLEN_N, /* UniqueID */
  "N  = MAXLEN", /* Syntax */
  "The maximum string length.",  /* Description */
  "MAXLEN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_MAXLEN_A_N, /* UniqueID */
  "N  = MAXLEN( A$ )", /* Syntax */
  "The maximum length associated with the simple-string-variable "
  "A$.",  /* Description */
  "MAXLEN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_MAXLNG_N, /* UniqueID */
  "N  = MAXLNG", /* Syntax */
  "The largest finite positive number representable as a LONG; implementation-defined.",  /* Description */
  "MAXLNG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MAXLVL_N, /* UniqueID */
  "N  = MAXLVL", /* Syntax */
  "The maximum stack level; implementation-defined.",  /* Description */
  "MAXLVL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MAXNUM_N, /* UniqueID */
  "N  = MAXNUM", /* Syntax */
  "The largest finite positive number representable and manipulable "
  "by the implementation; implementation-defined.  MAXNUM may represent "
  "diffent number for different arithmetic options (see OPTION ARITHMETIC).",  /* Description */
  "MAXNUM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_MAXSNG_N, /* UniqueID */
  "N  = MAXSNG", /* Syntax */
  "The largest finite positive number representable as a SINGLE; "
  "implementation-defined.",  /* Description */
  "MAXSNG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MEM_N, /* UniqueID */
  "N  = MEM", /* Syntax */
  "The number of bytes of available memory.  This function is provided "
  "for backward compatibility only and it always returns a fixed "
  "value of 32000.",  /* Description */
  "MEM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_MID_A_X_S, /* UniqueID */
  "S$ = MID( A$, X )", /* Syntax */
  "The characters of A$, starting from postion X.",  /* Description */
  "MID", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2NUM, /* ParameterTypes */
  P1ANY | P2POS, /* ParameterTests */
  HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_MID_A_X_Y_S, /* UniqueID */
  "S$ = MID( A$, X, Y )", /* Syntax */
  "The Y characters of A$, starting from postion X.",  /* Description */
  "MID", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1STR | P2NUM | P3NUM, /* ParameterTypes */
  P1ANY | P2POS | P3LEN, /* ParameterTests */
  HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_MID4_A_X_S, /* UniqueID */
  "S$ = MID$( A$, X )", /* Syntax */
  "The characters of A$, starting from postion X.",  /* Description */
  "MID$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2NUM, /* ParameterTypes */
  P1ANY | P2POS, /* ParameterTests */
  B15 | B93 | C77 | E86 | H14 | HB1 | HB2 | H80 | M80 | R86 | D71 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_MID4_A_X_Y_S, /* UniqueID */
  "S$ = MID$( A$, X, Y )", /* Syntax */
  "The Y characters of A$, starting from postion X.",  /* Description */
  "MID$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1STR | P2NUM | P3NUM, /* ParameterTypes */
  P1ANY | P2POS | P3LEN, /* ParameterTests */
  B15 | B93 | C77 | E86 | H14 | HB1 | HB2 | H80 | M80 | R86 | D71 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_MIN_X_Y_N, /* UniqueID */
  "N  = MIN( X, Y )", /* Syntax */
  "The smaller of the parameters.",  /* Description */
  "MIN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1ANY | P2ANY, /* ParameterTests */
  B15 | S70 | E86 | H14 | HB2 | H80 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_MIN_A_B_S, /* UniqueID */
  "S$ = MIN( A$, B$ )", /* Syntax */
  "The smaller of the parameters.",  /* Description */
  "MIN", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2STR, /* ParameterTypes */
  P1ANY | P2ANY, /* ParameterTests */
  B15 | S70 | HB2 | H80 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_MINBYT_N, /* UniqueID */
  "N  = MINBYT", /* Syntax */
  "The largest finite negative number representable as a BYTE; implementation-defined.",  /* Description */
  "MINBYT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MINCUR_N, /* UniqueID */
  "N  = MINCUR", /* Syntax */
  "The largest finite negative number representable as a CURRENCY; "
  "implementation-defined.",  /* Description */
  "MINCUR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MINDBL_N, /* UniqueID */
  "N  = MINDBL", /* Syntax */
  "The largest finite negative number representable as a DOUBLE; "
  "implementation-defined.",  /* Description */
  "MINDBL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MINDEV_N, /* UniqueID */
  "N  = MINDEV", /* Syntax */
  "The largest finite negative number useable as a FILE NUMBER; "
  "implementation-defined.",  /* Description */
  "MINDEV", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MININT_N, /* UniqueID */
  "N  = MININT", /* Syntax */
  "The largest finite negative number representable as an INTEGER; "
  "implementation-defined.",  /* Description */
  "MININT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MINLNG_N, /* UniqueID */
  "N  = MINLNG", /* Syntax */
  "The largest finite negative number representable as a LONG; implementation-defined.",  /* Description */
  "MINLNG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MINNUM_N, /* UniqueID */
  "N  = MINNUM", /* Syntax */
  "The largest finite negative number representable and manipulable "
  "by the implementation; implementation-defined.  MINNUM may represent "
  "diffent number for different arithmetic options (see OPTION ARITHMETIC).",  /* Description */
  "MINNUM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MINSNG_N, /* UniqueID */
  "N  = MINSNG", /* Syntax */
  "The largest finite negative number representable as a SINGLE; "
  "implementation-defined.",  /* Description */
  "MINSNG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MKC4_X_S, /* UniqueID */
  "S$ = MKC$( X )", /* Syntax */
  "The internal representation of the currency (64-bit) integer "
  "X as a string.",  /* Description */
  "MKC$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1CUR, /* ParameterTests */
  B15 /* OptionVersionBitmask */
},
{
  F_MKD4_X_S, /* UniqueID */
  "S$ = MKD$( X )", /* Syntax */
  "The internal representation of X as a string.",  /* Description */
  "MKD$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1DBL, /* ParameterTests */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_MKDIR_A_N, /* UniqueID */
  "N  = MKDIR( A$ )", /* Syntax */
  "Makes the directory named in A$.",  /* Description */
  "MKDIR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_MKI4_X_S, /* UniqueID */
  "S$ = MKI$( X )", /* Syntax */
  "The internal representation of the short (16-bit) integer X as "
  "a string.",  /* Description */
  "MKI$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_MKL4_X_S, /* UniqueID */
  "S$ = MKL$( X )", /* Syntax */
  "The internal representation of the long (32-bit) integer X as "
  "a string.",  /* Description */
  "MKL$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LNG, /* ParameterTests */
  B15 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_MKS4_X_S, /* UniqueID */
  "S$ = MKS$( X )", /* Syntax */
  "The internal representation of X as a string.",  /* Description */
  "MKS$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1FLT, /* ParameterTests */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_MOD_X_Y_N, /* UniqueID */
  "N  = MOD( X, Y )", /* Syntax */
  "X modulo Y, i.e., X-Y*INT(X/Y).  Y shall not equal zero.",  /* Description */
  "MOD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1ANY | P2NEZ, /* ParameterTests */
  E86 /* OptionVersionBitmask */
},
{
  F_NAME_A_B_N, /* UniqueID */
  "N  = NAME( A$, B$ )", /* Syntax */
  "Rename the file named A$ into B$.",  /* Description */
  "NAME", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2STR, /* ParameterTypes */
  P1BYT | P2BYT, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_NAME_N, /* UniqueID */
  "N  = NAME( ... )", /* Syntax */
  "Returns hardware address of variable.  Causes ERROR 73.",  /* Description */
  "NAME", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_NOFLOW_N, /* UniqueID */
  "N  = NOFLOW", /* Syntax */
  "Turn tracing OFF",  /* Description */
  "NOFLOW", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_NOTRACE_N, /* UniqueID */
  "N  = NOTRACE", /* Syntax */
  "Turn tracing OFF",  /* Description */
  "NOTRACE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_NULL_X_N, /* UniqueID */
  "N  = NULL( X )", /* Syntax */
  "Appends X null characters after each line printed by LPRINT or "
  "LLIST.",  /* Description */
  "NULL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_NUM_N, /* UniqueID */
  "N  = NUM", /* Syntax */
  "The number of values processed by the last MAT INPUT.  Zero means "
  "error.",  /* Description */
  "NUM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | G74 | HB2 | G67 /* OptionVersionBitmask */
},
{
  F_NUM_A_N, /* UniqueID */
  "N  = NUM( A$ )", /* Syntax */
  "The value of the numeric-constant associated with A$, if the "
  "string associated with A$ is a numeric-constant.  Leading and "
  "trailing spaces in the string are ignored.  If the evaluation "
  "of the numeric-constant would result in a value which causes "
  "an underflow, then the value returned shall be zero.  For example, "
  "NUM( \" 123.5 \" ) = 123.5, NUM( \"2.E-99\" ) could be zero, and "
  "NUM( \"MCMXVII\" ) causes an exception.",  /* Description */
  "NUM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | HB2 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_NUM4_X_S, /* UniqueID */
  "S$ = NUM$( X )", /* Syntax */
  "The string generated by the print-statement as the numeric-representation "
  "of the value associated with X.",  /* Description */
  "NUM$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | D64 | E78 | E86 | G74 | HB1 | HB2 | G65 | G67 | M80 | D71 | I70 | I73 | T80 /* OptionVersionBitmask */
},
{
  F_OCT4_X_S, /* UniqueID */
  "S$ = OCT$( X )", /* Syntax */
  "The the octal (base 8) representation of X.",  /* Description */
  "OCT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_OCT4_X_Y_S, /* UniqueID */
  "S$ = OCT$( X, Y )", /* Syntax */
  "The the octal (base 8) representation of X.  The result will "
  "be at least Y digits long. ",  /* Description */
  "OCT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2BYT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_OPEN_A_X_B_N, /* UniqueID */
  "N  = OPEN( A$, X, B$ )", /* Syntax */
  "Open file number X.  A$ is the mode: \"I\", \"O\", \"A\", \"R\".  B$ "
  "is the file name.  Default the record length.",  /* Description */
  "OPEN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1STR | P2NUM | P3STR, /* ParameterTypes */
  P1BYT | P2INT | P3BYT, /* ParameterTests */
  B15 | B93 | H14 | H80 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_OPEN_A_X_B_Y_N, /* UniqueID */
  "N  = OPEN( A$, X, B$, Y )", /* Syntax */
  "Open file number X.  A$ is the mode: \"I\", \"O\", \"A\", \"R\".  B$ "
  "is the file name.  Y is the record length.",  /* Description */
  "OPEN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  4, /* ParameterCount */
  P1STR | P2NUM | P3STR | P4NUM, /* ParameterTypes */
  P1BYT | P2INT | P3BYT | P4INT, /* ParameterTests */
  B15 | B93 | H14 | H80 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_ORD_A_N, /* UniqueID */
  "N  = ORD( A$ )", /* Syntax */
  "The ordinal position of the character named by the string associated "
  "with A$ in the collating sequence of ASCII character set, where "
  "the first member of the character set is in position zero.  The "
  "acceptable values for the standard character set are shown in "
  "Table 1.",  /* Description */
  "ORD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_OUT_X_Y_N, /* UniqueID */
  "N  = OUT( X, Y )", /* Syntax */
  "Sends Y to hardware port X.  Causes ERROR 73.",  /* Description */
  "OUT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2BYT, /* ParameterTests */
  B15 | C77 | H14 | HB1 | HB2 | H80 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_PAD_X_N, /* UniqueID */
  "N  = PAD( X )", /* Syntax */
  "Returns zero.",  /* Description */
  "PAD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  H80 /* OptionVersionBitmask */
},
{
  F_PAUSE_X_N, /* UniqueID */
  "N  = PAUSE( X )", /* Syntax */
  "The program pauses for X times the value of OPTION SLEEP seconds. "
  " If the result is zero, negative, or more than INT_MAX then PAUSE "
  "does nothing.  The resolution is implementation defined.",  /* Description */
  "PAUSE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 | H80 /* OptionVersionBitmask */
},
{
  F_PDL_X_N, /* UniqueID */
  "N  = PDL( X )", /* Syntax */
  "The value read from machine port X.  Causes ERROR 73.",  /* Description */
  "PDL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_PEEK_X_N, /* UniqueID */
  "N  = PEEK( X )", /* Syntax */
  "The value read from hardware address X.  Causes ERROR 73.",  /* Description */
  "PEEK", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LNG, /* ParameterTests */
  B15 | C77 | H14 | HB1 | HB2 | H80 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_PI_N, /* UniqueID */
  "N  = PI", /* Syntax */
  "The constant 3.14159 which is the ratio of the circumference "
  "of a circle to its diameter.",  /* Description */
  "PI", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | E86 | H14 | HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_PI_X_N, /* UniqueID */
  "N  = PI( X )", /* Syntax */
  "The constant 3.14159 which is the ratio of the circumference "
  "of a circle to its diameter, times X.",  /* Description */
  "PI", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_PIN_X_N, /* UniqueID */
  "N  = PIN( X )", /* Syntax */
  "The value read from machine port X.  Causes ERROR 73.",  /* Description */
  "PIN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  HB1 | HB2 | H80 /* OptionVersionBitmask */
},
{
  F_POKE_X_Y_N, /* UniqueID */
  "N  = POKE( X, Y )", /* Syntax */
  "Sends Y to hardware address X.  Causes ERROR 73.",  /* Description */
  "POKE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LNG | P2BYT, /* ParameterTests */
  B15 | C77 | H14 | HB1 | HB2 | H80 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_POS_N, /* UniqueID */
  "N  = POS", /* Syntax */
  "The current cursor position in the line.",  /* Description */
  "POS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | C77 | H14 | H80 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_POS_X_N, /* UniqueID */
  "N  = POS( X )", /* Syntax */
  "The current cursor position in the line for file X.",  /* Description */
  "POS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | H14 | HB2 | H80 | M80 | R86 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_POS_A_B_N, /* UniqueID */
  "N  = POS( A$, B$ )", /* Syntax */
  "The character position, within the value assocated with A$, of "
  "the first character of the first occurence of the value associated "
  "with B$, starting at the first character of A$.  If there is "
  "not such occurence, then the value returned is zero.",  /* Description */
  "POS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2STR, /* ParameterTypes */
  P1ANY | P2ANY, /* ParameterTests */
  B15 | E86 | H14 | HB2 /* OptionVersionBitmask */
},
{
  F_POS_A_B_X_N, /* UniqueID */
  "N  = POS( A$, B$, X )", /* Syntax */
  "The character position, within the value assocated with A$, of "
  "the first character of the first occurence of the value associated "
  "with B$, starting at the Xth character of A$.  If there is not "
  "such occurence, then the value returned is zero.",  /* Description */
  "POS", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1STR | P2STR | P3NUM, /* ParameterTypes */
  P1ANY | P2ANY | P3POS, /* ParameterTests */
  B15 | E86 | H14 | HB2 /* OptionVersionBitmask */
},
{
  F_PRECISION_X_N, /* UniqueID */
  "N  = PRECISION( X )", /* Syntax */
  "X is the number of decimal places to round (0..17).  If X = 0 "
  "then disabled.",  /* Description */
  "PRECISION", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  HB2 /* OptionVersionBitmask */
},
{
  F_PTR_N, /* UniqueID */
  "N  = PTR( ... )", /* Syntax */
  "Returns hardware address of variable.  Causes ERROR 73.",  /* Description */
  "PTR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_PUT_X_N, /* UniqueID */
  "N  = PUT( X )", /* Syntax */
  "Send character code to output.  Returns the value of X.  Similar "
  "to PRINT CHR$(X).",  /* Description */
  "PUT", /* Name */
  IntegerTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  D73 /* OptionVersionBitmask */
},
{
  F_PUT_X_N, /* UniqueID */
  "N  = PUT( X )", /* Syntax */
  "For file X, puts the next available record.",  /* Description */
  "PUT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_PUT_X_Y_N, /* UniqueID */
  "N  = PUT( X, Y )", /* Syntax */
  "For file X, puts record number Y.  The first record number is "
  "1.",  /* Description */
  "PUT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2INT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_RAD_N, /* UniqueID */
  "N  = RAD", /* Syntax */
  "Configures the math functions to accept and return angles in "
  "radians.",  /* Description */
  "RAD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_RAD_X_N, /* UniqueID */
  "N  = RAD( X )", /* Syntax */
  "The number of radians in X degrees.",  /* Description */
  "RAD", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | E86 | H14 | HB2 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_RADIAN_N, /* UniqueID */
  "N  = RADIAN", /* Syntax */
  "Configures the math functions to accept and return angles in "
  "radians.",  /* Description */
  "RADIAN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_RAN_N, /* UniqueID */
  "N  = RAN", /* Syntax */
  "Seeds the pseudo-random number generator with TIME.",  /* Description */
  "RAN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_RAN_X_N, /* UniqueID */
  "N  = RAN( X )", /* Syntax */
  "Seeds the pseudo-random number generator with X.",  /* Description */
  "RAN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_RANDOM_N, /* UniqueID */
  "N  = RANDOM", /* Syntax */
  "Seeds the pseudo-random number generator with TIME.",  /* Description */
  "RANDOM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  D64 | G74 | HB1 | HB2 | G65 | G67 /* OptionVersionBitmask */
},
{
  F_RANDOM_X_N, /* UniqueID */
  "N  = RANDOM( X )", /* Syntax */
  "Seeds the pseudo-random number generator with X.",  /* Description */
  "RANDOM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_RANDOMIZE_N, /* UniqueID */
  "N  = RANDOMIZE", /* Syntax */
  "Seeds the pseudo-random number generator with TIME.",  /* Description */
  "RANDOMIZE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | D71 | I70 | I73 | T80 | V09 /* OptionVersionBitmask */
},
{
  F_RANDOMIZE_X_N, /* UniqueID */
  "N  = RANDOMIZE( X )", /* Syntax */
  "Seeds the pseudo-random number generator with X.",  /* Description */
  "RANDOMIZE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D71 | I70 | I73 | T80 /* OptionVersionBitmask */
},
{
  F_REMAINDER_X_Y_N, /* UniqueID */
  "N  = REMAINDER( X, Y )", /* Syntax */
  "The remainder function, i.e., X-Y*IP(X/Y).  Y shall not equal "
  "zero.",  /* Description */
  "REMAINDER", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1ANY | P2NEZ, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_RENAME_A_B_N, /* UniqueID */
  "N  = RENAME( A$, B$ )", /* Syntax */
  "Rename file A$ to B$.  If successful, the returns -1 else returns "
  "0.",  /* Description */
  "RENAME", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2STR, /* ParameterTypes */
  P1BYT | P2BYT, /* ParameterTests */
  C77 | R86 | T79 /* OptionVersionBitmask */
},
{
  F_REPEAT4_X_A_S, /* UniqueID */
  "S$ = REPEAT$( X, A$ )", /* Syntax */
  "The string consisting of X copies of LEFT$(A$,1); X > 0.",  /* Description */
  "REPEAT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2STR, /* ParameterTypes */
  P1LEN | P2BYT, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_REPEAT4_X_Y_S, /* UniqueID */
  "S$ = REPEAT$( X, Y )", /* Syntax */
  "The string consisting of X copies of CHR$(Y); X > 0.",  /* Description */
  "REPEAT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LEN | P2BYT, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_RESET_N, /* UniqueID */
  "N  = RESET", /* Syntax */
  "Close all open files.",  /* Description */
  "RESET", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_RESIDUE_N, /* UniqueID */
  "N  = RESIDUE", /* Syntax */
  "The residue of the last Integer Divide operation.  The RESIDUE "
  "of -17 / 5 is -2.  For X / Y, RESIDUE = CINT( X - Y * CINT( X "
  "/ Y ) ).",  /* Description */
  "RESIDUE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  R86 /* OptionVersionBitmask */
},
{
  F_RIGHT_A_X_S, /* UniqueID */
  "S$ = RIGHT( A$, X )", /* Syntax */
  "The right-most X characters of A$.",  /* Description */
  "RIGHT", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2NUM, /* ParameterTypes */
  P1ANY | P2LEN, /* ParameterTests */
  HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_RIGHT4_A_X_S, /* UniqueID */
  "S$ = RIGHT$( A$, X )", /* Syntax */
  "The right-most X characters of A$.",  /* Description */
  "RIGHT$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1STR | P2NUM, /* ParameterTypes */
  P1ANY | P2LEN, /* ParameterTests */
  B15 | B93 | C77 | E86 | H14 | HB1 | HB2 | H80 | M80 | R86 | D71 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_RMDIR_A_N, /* UniqueID */
  "N  = RMDIR( A$ )", /* Syntax */
  "Removes the directory named in A$.",  /* Description */
  "RMDIR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_RND_N, /* UniqueID */
  "N  = RND", /* Syntax */
  "The next pseudo-random number in an implementation-defined sequence "
  "of pseudo-random numbers uniformly distributed in the range 0 "
  "<= RND < 1.",  /* Description */
  "RND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_RND_X_N, /* UniqueID */
  "N  = RND( X )", /* Syntax */
  "Returns a pseudorandom number in the range [0,1].  The value "
  "of X is ignored.",  /* Description */
  "RND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_ROUND_X_Y_N, /* UniqueID */
  "N  = ROUND( X, Y )", /* Syntax */
  "The value of X rounded to Y decimal digits to the right of the "
  "decimal point (or -Y digits to the left if Y < 0); i.e., INT(X*10^Y+.5)/10^Y. "
  " Y must be in [-32,32].",  /* Description */
  "ROUND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1ANY | P2INT, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_RTRIM4_A_S, /* UniqueID */
  "S$ = RTRIM$( A$ )", /* Syntax */
  "The string of characters resulting from the value associated "
  "with A$ by deleting all trailing space characters.",  /* Description */
  "RTRIM$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_SCALE_X_N, /* UniqueID */
  "N  = SCALE( X )", /* Syntax */
  "X is the number of decimal places to round (0..17).  If X = 0 "
  "then disabled.",  /* Description */
  "SCALE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  R86 | T79 /* OptionVersionBitmask */
},
{
  F_SEC_X_N, /* UniqueID */
  "N  = SEC( X )", /* Syntax */
  "The secant of X, where X is in radians.",  /* Description */
  "SEC", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | S70 | E86 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_SEEK_X_N, /* UniqueID */
  "N  = SEEK( X )", /* Syntax */
  "The location of file X; the next record that GET or PUT statements "
  "will use.",  /* Description */
  "SEEK", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  B15 | E86 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_SEEK_X_Y_N, /* UniqueID */
  "N  = SEEK( X, Y )", /* Syntax */
  "For file X, move to record number Y; the first record number "
  "is 1.",  /* Description */
  "SEEK", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2INT, /* ParameterTests */
  B15 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_SEG_X_N, /* UniqueID */
  "N  = SEG( X )", /* Syntax */
  "Returns zero.",  /* Description */
  "SEG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  H80 /* OptionVersionBitmask */
},
{
  F_SEG_A_X_Y_S, /* UniqueID */
  "S$ = SEG( A$, X, Y )", /* Syntax */
  "The Y characters of A$, starting from postion X.",  /* Description */
  "SEG", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1STR | P2NUM | P3NUM, /* ParameterTypes */
  P1ANY | P2POS | P3LEN, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_SEG4_A_X_Y_S, /* UniqueID */
  "S$ = SEG$( A$, X, Y )", /* Syntax */
  "The Y characters of A$, starting from postion X.",  /* Description */
  "SEG$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1STR | P2NUM | P3NUM, /* ParameterTypes */
  P1ANY | P2POS | P3LEN, /* ParameterTests */
  B15 | H14 | HB2 /* OptionVersionBitmask */
},
{
  F_SGN_X_N, /* UniqueID */
  "N  = SGN( X )", /* Syntax */
  "The sign of X: -1 if X < 0, 0 if X = 0, and +1 if X > 0.",  /* Description */
  "SGN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_SHELL_A_N, /* UniqueID */
  "N  = SHELL( A$ )", /* Syntax */
  "The exit code resulting from the execution of an operating system "
  "command.",  /* Description */
  "SHELL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_SIN_X_N, /* UniqueID */
  "N  = SIN( X )", /* Syntax */
  "The sine of X, where X is in radians.",  /* Description */
  "SIN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_SIND_X_N, /* UniqueID */
  "N  = SIND( X )", /* Syntax */
  "The sine of X, where X is in degrees.",  /* Description */
  "SIND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_SING_X_N, /* UniqueID */
  "N  = SING( X )", /* Syntax */
  "The sine of X, where X is in gradians.",  /* Description */
  "SING", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_SINH_X_N, /* UniqueID */
  "N  = SINH( X )", /* Syntax */
  "The hyperbolic sine of X.",  /* Description */
  "SINH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 | HB2 /* OptionVersionBitmask */
},
{
  F_SIZE_A_N, /* UniqueID */
  "N  = SIZE( A$ )", /* Syntax */
  "The total number of 1KB blockes required to contain the file.",  /* Description */
  "SIZE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  C77 /* OptionVersionBitmask */
},
{
  F_SIZE_N, /* UniqueID */
  "N  = SIZE( ... )", /* Syntax */
  "SIZE( arrayname ).  The total number of items in the array.",  /* Description */
  "SIZE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  E86 /* OptionVersionBitmask */
},
{
  F_SLEEP_X_N, /* UniqueID */
  "N  = SLEEP( X )", /* Syntax */
  "The program pauses for X times the value of OPTION SLEEP seconds. "
  " If the result is zero, negative, or more than INT_MAX then SLEEP "
  "does nothing.  The resolution is implementation defined.",  /* Description */
  "SLEEP", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | H14 | HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_SNH_X_N, /* UniqueID */
  "N  = SNH( X )", /* Syntax */
  "The hyperbolic sine of X.",  /* Description */
  "SNH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_SPA_X_S, /* UniqueID */
  "S$ = SPA( X )", /* Syntax */
  "The string of X blank spaces.",  /* Description */
  "SPA", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LEN, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_SPACE_X_S, /* UniqueID */
  "S$ = SPACE( X )", /* Syntax */
  "The string of X blank spaces.",  /* Description */
  "SPACE", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LEN, /* ParameterTests */
  HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_SPACE4_X_S, /* UniqueID */
  "S$ = SPACE$( X )", /* Syntax */
  "The string of X blank spaces.",  /* Description */
  "SPACE$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1LEN, /* ParameterTests */
  B15 | B93 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_SPC_X_S, /* UniqueID */
  "S$ = SPC( X )", /* Syntax */
  "The string of X spaces.  Only for use within the PRINT command.",  /* Description */
  "SPC", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_SQR_X_N, /* UniqueID */
  "N  = SQR( X )", /* Syntax */
  "The non-negative square root of X;  X shall be non-negative.",  /* Description */
  "SQR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GEZ, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_SQRT_X_N, /* UniqueID */
  "N  = SQRT( X )", /* Syntax */
  "The non-negative square root of X;  X shall be non-negative.",  /* Description */
  "SQRT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1GEZ, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_STR_X_Y_S, /* UniqueID */
  "S$ = STR( X, Y )", /* Syntax */
  "The string X bytes long consisting of CHR$(Y).",  /* Description */
  "STR", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LEN | P2BYT, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_STR4_X_S, /* UniqueID */
  "S$ = STR$( X )", /* Syntax */
  "The string generated by the print-statement as the numeric-representation "
  "of the value associated with X.",  /* Description */
  "STR$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_STRING_X_Y_S, /* UniqueID */
  "S$ = STRING( X, Y )", /* Syntax */
  "The string X bytes long consisting of CHR$(Y).",  /* Description */
  "STRING", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LEN | P2BYT, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_STRING4_X_A_S, /* UniqueID */
  "S$ = STRING$( X, A$ )", /* Syntax */
  "The string X bytes long consisting of the first character of "
  "A$.",  /* Description */
  "STRING$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2STR, /* ParameterTypes */
  P1LEN | P2BYT, /* ParameterTests */
  B15 | B93 | E86 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_STRING4_X_Y_S, /* UniqueID */
  "S$ = STRING$( X, Y )", /* Syntax */
  "The string X bytes long consisting of CHR$(Y).",  /* Description */
  "STRING$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LEN | P2BYT, /* ParameterTests */
  B15 | B93 | E86 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_STRIP4_A_S, /* UniqueID */
  "S$ = STRIP$( A$ )", /* Syntax */
  "Return the string with the eighth bit of each character cleared.",  /* Description */
  "STRIP$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_STUFF_X_Y_N, /* UniqueID */
  "N  = STUFF( X, Y )", /* Syntax */
  "Sends Y to hardware address X.  Causes ERROR 73.",  /* Description */
  "STUFF", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1LNG | P2BYT, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_TAB_X_S, /* UniqueID */
  "S$ = TAB( X )", /* Syntax */
  "The string required to advance to column X.  Only for use within "
  "the PRINT command.",  /* Description */
  "TAB", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_TAN_X_N, /* UniqueID */
  "N  = TAN( X )", /* Syntax */
  "The tangent of X, where X is in radians.",  /* Description */
  "TAN", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_TAND_X_N, /* UniqueID */
  "N  = TAND( X )", /* Syntax */
  "The tangent of X, where X is in degrees.",  /* Description */
  "TAND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_TANG_X_N, /* UniqueID */
  "N  = TANG( X )", /* Syntax */
  "The tangent of X, where X is in gradians.",  /* Description */
  "TANG", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 /* OptionVersionBitmask */
},
{
  F_TANH_X_N, /* UniqueID */
  "N  = TANH( X )", /* Syntax */
  "The hyperbolic tangent of X.",  /* Description */
  "TANH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 | HB2 /* OptionVersionBitmask */
},
{
  F_TI_N, /* UniqueID */
  "N  = TI", /* Syntax */
  "The time elapsed since the previous midnight, expressed in seconds; "
  "e.g., the value of TIME at 11:15 AM is 40500.  If there is no "
  "clock available, then the value of TIME shall be -1.  The value "
  "of TIME at midnight shall be zero (not 86400).",  /* Description */
  "TI", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_TI4_S, /* UniqueID */
  "S$ = TI$", /* Syntax */
  "The time of day in 24-hour notation according to ISO 3307.  For "
  "example, the value of TIME$ at 11:15 AM is \"11:15:00\".  If there "
  "is no clock available, then the value of TIME$ shall be \"99:99:99\". "
  " The value of TIME$ at midnight is \"00:00:00\".",  /* Description */
  "TI$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_TIM_N, /* UniqueID */
  "N  = TIM", /* Syntax */
  "The time elapsed since the previous midnight, expressed in seconds; "
  "e.g., the value of TIME at 11:15 AM is 40500.  If there is no "
  "clock available, then the value of TIME shall be -1.  The value "
  "of TIME at midnight shall be zero (not 86400).",  /* Description */
  "TIM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  S70 | G74 | HB1 | HB2 | G67 | I70 | I73 /* OptionVersionBitmask */
},
{
  F_TIM_X_N, /* UniqueID */
  "N  = TIM( X )", /* Syntax */
  "If X is 0, returns minutes in current hour.  If X is 1, returns "
  "hours in current day.  If X is 2, returns days in current year. "
  " If X is 3, returns years since 1900.  Any other value for X "
  "is an ERROR.",  /* Description */
  "TIM", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  G74 | HB1 | HB2 | G65 | G67 /* OptionVersionBitmask */
},
{
  F_TIME_N, /* UniqueID */
  "N  = TIME", /* Syntax */
  "The time elapsed since the previous midnight, expressed in seconds; "
  "e.g., the value of TIME at 11:15 AM is 40500.  If there is no "
  "clock available, then the value of TIME shall be -1.  The value "
  "of TIME at midnight shall be zero (not 86400).",  /* Description */
  "TIME", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | E86 | H14 | HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_TIME_X_N, /* UniqueID */
  "N  = TIME( X )", /* Syntax */
  "The time elapsed since the previous midnight, expressed in seconds; "
  "e.g., the value of TIME at 11:15 AM is 40500.  If there is no "
  "clock available, then the value of TIME shall be -1.  The value "
  "of TIME at midnight shall be zero (not 86400).  The value of "
  "the parameter X is ignored.",  /* Description */
  "TIME", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_TIME4_S, /* UniqueID */
  "S$ = TIME$", /* Syntax */
  "The time of day in 24-hour notation according to ISO 3307.  For "
  "example, the value of TIME$ at 11:15 AM is \"11:15:00\".  If there "
  "is no clock available, then the value of TIME$ shall be \"99:99:99\". "
  " The value of TIME$ at midnight is \"00:00:00\".",  /* Description */
  "TIME$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | E86 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_TIME4_X_S, /* UniqueID */
  "S$ = TIME$( X )", /* Syntax */
  "The time of day in 24-hour notation according to ISO 3307.  For "
  "example, the value of TIME$ at 11:15 AM is \"11:15:00\".  If there "
  "is no clock available, then the value of TIME$ shall be \"99:99:99\". "
  " The value of TIME$ at midnight is \"00:00:00\".  The value of "
  "X is ignored.",  /* Description */
  "TIME$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_TIMER_N, /* UniqueID */
  "N  = TIMER", /* Syntax */
  "The time in the system clock in seconds elapsed since midnight.",  /* Description */
  "TIMER", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_TOP_N, /* UniqueID */
  "N  = TOP", /* Syntax */
  "The address of the top of available memory.  This function is "
  "provided for backward compatibility only and it always returns "
  "a fixed value of 32000.",  /* Description */
  "TOP", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  HB2 /* OptionVersionBitmask */
},
{
  F_TRACE_N, /* UniqueID */
  "N  = TRACE", /* Syntax */
  "Turn tracing ON",  /* Description */
  "TRACE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_TRACE_X_N, /* UniqueID */
  "N  = TRACE( X )", /* Syntax */
  "When X is zero turn tracing off.  When X is non-zero turn tracing "
  "ON",  /* Description */
  "TRACE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  R86 /* OptionVersionBitmask */
},
{
  F_TRIM4_A_S, /* UniqueID */
  "S$ = TRIM$( A$ )", /* Syntax */
  "The string resulting from removing both leading and trailing "
  "spaces from A$.",  /* Description */
  "TRIM$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | E86 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_TROFF_N, /* UniqueID */
  "N  = TROFF", /* Syntax */
  "Turn tracing OFF",  /* Description */
  "TROFF", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | E86 | H14 | HB1 | HB2 | M80 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_TRON_N, /* UniqueID */
  "N  = TRON", /* Syntax */
  "Turn tracing ON",  /* Description */
  "TRON", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | B93 | E86 | H14 | HB1 | HB2 | M80 | D71 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_TRUE_N, /* UniqueID */
  "N  = TRUE", /* Syntax */
  "The value of TRUE.",  /* Description */
  "TRUE", /* Name */
  IntegerTypeCode, /* ReturnTypeCode */
  0, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_TRUNCATE_X_Y_N, /* UniqueID */
  "N  = TRUNCATE( X, Y )", /* Syntax */
  "The value of X truncated to Y decimal digits to the right of "
  "the decimal point (or -Y digits to the left if Y < 0); i.e., "
  "IP(X*10^Y)/10^Y.  Y in [-32,32].",  /* Description */
  "TRUNCATE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1ANY | P2INT, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_UBOUND_N, /* UniqueID */
  "N  = UBOUND( ... )", /* Syntax */
  "UBOUND( arrayname [, dimension] ).  The upper bound of the array. "
  " The dimension defaults to 1.  dimension in [1,DIM(arrayname)]",  /* Description */
  "UBOUND", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  B15 | E86 /* OptionVersionBitmask */
},
{
  F_UCASE4_A_S, /* UniqueID */
  "S$ = UCASE$( A$ )", /* Syntax */
  "The string of characters resulting from the value associated "
  "with A$ by replacing each lower-case-letter in the string by "
  "its upper-case version.",  /* Description */
  "UCASE$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | C77 | E86 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_UNLOCK_X_N, /* UniqueID */
  "N  = UNLOCK( X )", /* Syntax */
  "Unlock file number X.  Causes ERROR 73.",  /* Description */
  "UNLOCK", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_UNSAVE_A_N, /* UniqueID */
  "N  = UNSAVE( A$ )", /* Syntax */
  "Removes the file named in A$.",  /* Description */
  "UNSAVE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  D64 | G74 | H80 | G65 | G67 | D71 /* OptionVersionBitmask */
},
{
  F_UPPER4_A_S, /* UniqueID */
  "S$ = UPPER$( A$ )", /* Syntax */
  "The string of characters resulting from the value associated "
  "with A$ by replacing each lower-case-letter in the string by "
  "its upper-case version.",  /* Description */
  "UPPER$", /* Name */
  StringTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_USR_N, /* UniqueID */
  "N  = USR( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR0_N, /* UniqueID */
  "N  = USR0( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR0", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR1_N, /* UniqueID */
  "N  = USR1( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR1", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR2_N, /* UniqueID */
  "N  = USR2( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR2", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR3_N, /* UniqueID */
  "N  = USR3( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR3", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR4_N, /* UniqueID */
  "N  = USR4( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR4", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR5_N, /* UniqueID */
  "N  = USR5( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR5", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR6_N, /* UniqueID */
  "N  = USR6( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR6", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR7_N, /* UniqueID */
  "N  = USR7( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR7", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR8_N, /* UniqueID */
  "N  = USR8( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR8", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_USR9_N, /* UniqueID */
  "N  = USR9( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "USR9", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 | R86 | T80 | T79 /* OptionVersionBitmask */
},
{
  F_UUF_N, /* UniqueID */
  "N  = UUF( ... )", /* Syntax */
  "Execute hardware program.  Causes ERROR 73.",  /* Description */
  "UUF", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  D73 /* OptionVersionBitmask */
},
{
  F_VAL_A_N, /* UniqueID */
  "N  = VAL( A$ )", /* Syntax */
  "The value of the numeric-constant associated with A$, if the "
  "string associated with A$ is a numeric-constant.  Leading and "
  "trailing spaces in the string are ignored.  If the evaluation "
  "of the numeric-constant would result in a value which causes "
  "an underflow, then the value returned shall be zero.  For example, "
  "VAL( \" 123.5 \" ) = 123.5, VAL( \"2.E-99\" ) could be zero, and "
  "VAL( \"MCMXVII\" ) causes an exception.",  /* Description */
  "VAL", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1STR, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | V09 | T79 /* OptionVersionBitmask */
},
{
  F_VARPTR_N, /* UniqueID */
  "N  = VARPTR( ... )", /* Syntax */
  "Returns hardware address of variable.  Causes ERROR 73.",  /* Description */
  "VARPTR", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  255, /* ParameterCount */
  PNONE, /* ParameterTypes */
  PNONE, /* ParameterTests */
  M80 /* OptionVersionBitmask */
},
{
  F_VTAB_X_N, /* UniqueID */
  "N  = VTAB( X )", /* Syntax */
  "Savme as LOCATE X, 1.",  /* Description */
  "VTAB", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  HB1 | HB2 /* OptionVersionBitmask */
},
{
  F_WAIT_X_N, /* UniqueID */
  "N  = WAIT( X )", /* Syntax */
  "The program pauses for X times the value of OPTION SLEEP seconds. "
  " If the result is zero, negative, or more than INT_MAX then WAIT "
  "does nothing.  The resolution is implementation defined.",  /* Description */
  "WAIT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1ANY, /* ParameterTests */
  B15 | HB1 | HB2 | D71 /* OptionVersionBitmask */
},
{
  F_WAIT_X_Y_N, /* UniqueID */
  "N  = WAIT( X, Y )", /* Syntax */
  "Waits for the value of (INP(X) AND Y) to become nonzero.  Causes "
  "ERROR 73.",  /* Description */
  "WAIT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2BYT, /* ParameterTests */
  B15 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_WAIT_X_Y_Z_N, /* UniqueID */
  "N  = WAIT( X, Y, Z )", /* Syntax */
  "Waits for the value of ((INP(X) XOR Z) AND Y) to become nonzero. "
  " Causes ERROR 73.",  /* Description */
  "WAIT", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  3, /* ParameterCount */
  P1NUM | P2NUM | P3NUM, /* ParameterTypes */
  P1INT | P2BYT | P3BYT, /* ParameterTests */
  B15 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_WIDTH_X_N, /* UniqueID */
  "N  = WIDTH( X )", /* Syntax */
  "For console, sets the line width to X. Zero means no wrapping "
  "will occur.",  /* Description */
  "WIDTH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1BYT, /* ParameterTests */
  B15 | B93 | H14 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_WIDTH_X_Y_N, /* UniqueID */
  "N  = WIDTH( X, Y )", /* Syntax */
  "If X = 0, sets the console width to Y.\nIf X < 0, sets the printer "
  "width to Y.\nIf X is an open file number, sets the file line width "
  "to Y.\nOtherwise sets the console rows to X and the line width "
  "to Y.\nA value of zero for Y means no wrapping will occur.",  /* Description */
  "WIDTH", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2BYT, /* ParameterTests */
  B15 | B93 | H14 | HB1 | HB2 | M80 | D71 | T80 /* OptionVersionBitmask */
},
{
  F_ZONE_X_N, /* UniqueID */
  "N  = ZONE( X )", /* Syntax */
  "Sets the width of the print zones.",  /* Description */
  "ZONE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  1, /* ParameterCount */
  P1NUM, /* ParameterTypes */
  P1INT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
{
  F_ZONE_X_Y_N, /* UniqueID */
  "N  = ZONE( X, Y )", /* Syntax */
  "X is ignored.  Y sets the width of the print zones.",  /* Description */
  "ZONE", /* Name */
  DoubleTypeCode, /* ReturnTypeCode */
  2, /* ParameterCount */
  P1NUM | P2NUM, /* ParameterTypes */
  P1INT | P2INT, /* ParameterTests */
  H14 /* OptionVersionBitmask */
},
};

const size_t NUM_FUNCTIONS =
  sizeof (IntrinsicFunctionTable) / sizeof (IntrinsicFunctionType);


/* EOF */
