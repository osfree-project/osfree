/****************************************************************
  
        bwb_exp.c       Expression Parser
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



#include "bwbasic.h"


/* 
--------------------------------------------------------------------------------------------
                               EXPRESSION PARSER

Inspired by https://groups.google.com/forum/m/#!topic/comp.compilers/RCyhEbLfs40
...
// Permission is given to use this source provided an acknowledgement is given.
// I'd also like to know if you've found it useful.
//
// The following Research Report describes the idea, and shows how the
// parsing method may be understood as an encoding of the usual family-of-
// parsing-procedures technique as used e.g. in Pascal compilers.
//     @techreport{QMW-DCS-383-1986a,
//       author       ="Clarke, Keith",
//       title        ="The Top-Down Parsing of Expressions",
//       institution  ="Department of Computer Science, Queen Mary College, University of London, England",
//       year         ="1986",
//       month        ="June",
//       number       ="QMW-DCS-1986-383",
//       scope        ="theory",
//       abstractURL  ="http://www.dcs.qmw.ac.uk/publications/report_abstracts/1986/383",
//       keywords     ="Recursive-descent parsing, expression parsing, operator precedence parsing."
//     }
// A formal proof of the algorithm was made, as part of his PhD thesis work,
// by A.M. Abbas of QMC, London, in the framework of Constructive Set Theory.
// copyright Keith Clarke, Dept of Computer Science, QMW, University of London,
// England.    email kei...@dcs.qmw.ac.uk
...
--------------------------------------------------------------------------------------------
*/

/* 
For all functions named "line_*",  "LineType * line" is the first parameter.
For all functions named "buff_*",  "char * buffer, int * position" are the first two parameters.
FALSE must be zero.
TRUE  must be non-zero.
*/



/* OperatorType.Arity */
#define UNARY  1
#define BINARY 2

/* OperatorType.IsAlpha */
#define IS_ALPHA  'T'
#define NO_ALPHA  'F'


#define COPY_VARIANT( X, Y ) if( X != NULL ) { bwb_memcpy( X, Y, sizeof( VariantType ) ); bwb_memset( Y, 0, sizeof( VariantType ) ); }

typedef ResultType (OperatorFunctionType) (VariantType * X, VariantType * Y);

struct OperatorStruct
{
  const unsigned char ThisPrec;
  const unsigned char NextPrec;        /* if BINARY and LEFT assoc, then ThisPrec+1, else ThisPrec */
  const unsigned char Arity;        /* UNARY or BINARY */
  const char IsAlpha;                /* IS_ALPHA or NO_ALPHA, determines how operator is matched */
  const char *Name;
  OperatorFunctionType *Eval;
  const char *Syntax;
  const char *Description;
  OptionVersionType OptionVersionBitmask;        /* OPTION VERSION bitmask */
};
typedef struct OperatorStruct OperatorType;

static int both_are_long (VariantType * X, VariantType * Y);
static int both_integer_type (VariantType * X, VariantType * Y);
static int both_number_type (VariantType * X, VariantType * Y);
static int both_string_type (VariantType * X, VariantType * Y);
static ResultType buff_read_expr (char *buffer, int *position,
                                  VariantType * X, unsigned char LastPrec);
static ResultType buff_read_function (char *buffer, int *position,
                                      VariantType * X);
static ResultType buff_read_internal_constant (char *buffer, int *position,
                                               VariantType * X);
static OperatorType *buff_read_operator (char *buffer, int *position,
                                         unsigned char LastPrec,
                                         unsigned char Arity);
static ResultType buff_read_primary (char *buffer, int *position,
                                     VariantType * X);
static ResultType buff_read_string_constant (char *buffer, int *position,
                                             VariantType * X);
static ResultType buff_read_variable (char *buffer, int *position,
                                      VariantType * X);
static int bwb_isodigit (int C);
static int is_integer_type (VariantType * X);
static int is_long_value (VariantType * X);
static int is_number_type (VariantType * X);
static int is_string_type (VariantType * X);
static char Largest_TypeCode (char TypeCode, VariantType * X);
static char math_type (VariantType * X, VariantType * Y);
static char max_number_type (char X, char Y);
static char min_value_type (VariantType * X);
static ResultType OP_ADD (VariantType * X, VariantType * Y);
static ResultType OP_AMP (VariantType * X, VariantType * Y);
static ResultType OP_AND (VariantType * X, VariantType * Y);
static ResultType OP_DIV (VariantType * X, VariantType * Y);
static ResultType OP_EQ (VariantType * X, VariantType * Y);
static ResultType OP_EQV (VariantType * X, VariantType * Y);
static ResultType OP_EXP (VariantType * X, VariantType * Y);
static ResultType OP_GE (VariantType * X, VariantType * Y);
static ResultType OP_GT (VariantType * X, VariantType * Y);
static ResultType OP_IDIV (VariantType * X, VariantType * Y);
static ResultType OP_IMP (VariantType * X, VariantType * Y);
static ResultType OP_LE (VariantType * X, VariantType * Y);
static ResultType OP_LIKE (VariantType * X, VariantType * Y);
static ResultType OP_LT (VariantType * X, VariantType * Y);
static ResultType OP_MAX (VariantType * X, VariantType * Y);
static ResultType OP_MIN (VariantType * X, VariantType * Y);
static ResultType OP_MOD (VariantType * X, VariantType * Y);
static ResultType OP_MUL (VariantType * X, VariantType * Y);
static ResultType OP_NE (VariantType * X, VariantType * Y);
static ResultType OP_NEG (VariantType * X, VariantType * Y);
static ResultType OP_NOT (VariantType * X, VariantType * Y);
static ResultType OP_OR (VariantType * X, VariantType * Y);
static ResultType OP_POS (VariantType * X, VariantType * Y);
static ResultType OP_SUB (VariantType * X, VariantType * Y);
static ResultType OP_XOR (VariantType * X, VariantType * Y);
static void SortAllOperatorsForManual (void);
static ResultType test_eq (VariantType * X, VariantType * Y, int TrueValue,
                           int FalseValue);
static ResultType test_gt (VariantType * X, VariantType * Y, int TrueValue,
                           int FalseValue);
static ResultType test_lt (VariantType * X, VariantType * Y, int TrueValue,
                           int FalseValue);


/* table of operators */

/* 
In BASIC, 2 ^ 3 ^ 2 = ( 2 ^ 3 ) ^ 2 = 64, and -2 ^ 2 = - (2 ^ 2) = -4.
*/


static OperatorType OperatorTable[ /* NUM_OPERATORS */ ] =
{
  /* LOGICAL */
  {0x01, 0x02, BINARY, IS_ALPHA, "IMP", OP_IMP, "X IMP Y", "Bitwise IMP",
   B15 | B93 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71 | M80 | T80
   | H14},
  {0x02, 0x03, BINARY, IS_ALPHA, "EQV", OP_EQV, "X EQV Y", "Bitwise EQV",
   B15 | B93 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71 | M80 | T80
   | H14},
  {0x03, 0x04, BINARY, IS_ALPHA, "XOR", OP_XOR, "X XOR Y",
   "Bitwise Exclusive OR",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | M80 | T79 | R86 | T80 | H14},
  {0x03, 0x04, BINARY, IS_ALPHA, "XRA", OP_XOR, "X XRA Y",
   "Bitwise Exclusive OR",
   HB2},
  {0x04, 0x05, BINARY, IS_ALPHA, "OR", OP_OR, "X OR Y", "Bitwise OR",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x05, 0x06, BINARY, IS_ALPHA, "AND", OP_AND, "X AND Y", "Bitwise AND",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x06, 0x06, UNARY, IS_ALPHA, "NOT", OP_NOT, "NOT X", "Bitwise NOT",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
/* RELATIONAL */
  {0x07, 0x08, BINARY, IS_ALPHA, "NE", OP_NE, "X NE Y", "Not Equal",
   0},
  {0x07, 0x08, BINARY, NO_ALPHA, "#", OP_NE, "X # Y", "Not Equal",
   0},
  {0x07, 0x08, BINARY, NO_ALPHA, "<>", OP_NE, "X <> Y", "Not Equal",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, NO_ALPHA, "><", OP_NE, "X >< Y", "Not Equal",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, IS_ALPHA, "GE", OP_GE, "X GE Y",
   "Greater than or Equal",
   0},
  {0x07, 0x08, BINARY, NO_ALPHA, ">=", OP_GE, "X >= Y",
   "Greater than or Equal",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, NO_ALPHA, "=>", OP_GE, "X => Y",
   "Greater than or Equal",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, IS_ALPHA, "LE", OP_LE, "X LE Y", "Less than or Equal",
   0},
  {0x07, 0x08, BINARY, NO_ALPHA, "<=", OP_LE, "X <= Y", "Less than or Equal",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, NO_ALPHA, "=<", OP_LE, "X =< Y", "Less than or Equal",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, IS_ALPHA, "EQ", OP_EQ, "X EQ Y", "Equal",
   0},
  {0x07, 0x08, BINARY, NO_ALPHA, "=", OP_EQ, "X = Y", "Equal",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, IS_ALPHA, "LT", OP_LT, "X LT Y", "Less than",
   0},
  {0x07, 0x08, BINARY, NO_ALPHA, "<", OP_LT, "X < Y", "Less than",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, IS_ALPHA, "GT", OP_GT, "X GT Y", "Greater than",
   0},
  {0x07, 0x08, BINARY, NO_ALPHA, ">", OP_GT, "X > Y", "Greater than",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x07, 0x08, BINARY, IS_ALPHA, "LIKE", OP_LIKE, "A$ LIKE B$",
   "Compare A$ to the pattern in B$",
   B15},
  {0x07, 0x08, BINARY, IS_ALPHA, "MAX", OP_MAX, "X MAX Y", "Maximum",
   0},
  {0x07, 0x08, BINARY, IS_ALPHA, "MIN", OP_MIN, "X MIN Y", "Minimum",
   0},
/* CONCATENATION */
  {0x08, 0x09, BINARY, NO_ALPHA, "&", OP_AMP, "X & Y", "Concatenation",
   B15 | B93 | HB2},
/* ARITHMETIC */
  {0x09, 0x0A, BINARY, NO_ALPHA, "+", OP_ADD, "X + Y", "Addition",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x09, 0x0A, BINARY, NO_ALPHA, "-", OP_SUB, "X - Y", "Subtraction",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x0A, 0x0B, BINARY, IS_ALPHA, "MOD", OP_MOD, "X MOD Y", "Integer Modulus",
   B15 | B93 | HB1 | HB2 | D71 | M80 | R86 | T80 | H14},
  {0x0B, 0x0C, BINARY, NO_ALPHA, "\\", OP_IDIV, "X \\ Y", "Integer Division",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | E78 | E86 | M80 | T80 | H14},
  {0x0C, 0x0D, BINARY, NO_ALPHA, "*", OP_MUL, "X * Y", "Multiplication",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x0C, 0x0D, BINARY, NO_ALPHA, "/", OP_DIV, "X / Y", "Division",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x0D, 0x0D, UNARY, NO_ALPHA, "#", OP_POS, "# X", "Posation",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | C77 | D71 | E86 | M80 | T79
   | R86 | T80 | H80 | H14},
  {0x0D, 0x0D, UNARY, NO_ALPHA, "+", OP_POS, "+ X", "Posation",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x0D, 0x0D, UNARY, NO_ALPHA, "-", OP_NEG, "- X", "Negation",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | T80 | H80 | V09 | H14},
  {0x0E, 0x0F, BINARY, NO_ALPHA, "^", OP_EXP, "X ^ Y", "Exponential",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78 | E86 | M80 | T79 | R86 | H80 | V09 | H14},
  {0x0E, 0x0F, BINARY, NO_ALPHA, "[", OP_EXP, "X [ Y", "Exponential",
   B15 | HB1 | HB2 | T80},
  {0x0E, 0x0F, BINARY, NO_ALPHA, "**", OP_EXP, "X ** Y", "Exponential",
   B15 | B93 | HB1 | HB2 | D64 | G65 | G67 | G74 | S70 | I70 | I73 | C77 | D71
   | D70 | D73 | E78},
};

static const size_t NUM_OPERATORS =
  sizeof (OperatorTable) / sizeof (OperatorType);

/*
--------------------------------------------------------------------------------------------
                               Helpers
--------------------------------------------------------------------------------------------
*/

extern void
SortAllOperators (void)                /* SortAllOperators() should be called by bwb_init() */
{
  /* sort the operators by decreasing length, so "**" matches before "*" and so on. */
  int i;
   

  for (i = 0; i < NUM_OPERATORS - 1; i++)
  {
    int j;
    int k;
    int m;

    k = i;
    m = bwb_strlen (OperatorTable[i].Name);

    for (j = i + 1; j < NUM_OPERATORS; j++)
    {
      int n;
      n = bwb_strlen (OperatorTable[j].Name);
      if (n > m)
      {
        m = n;
        k = j;
      }
    }
    if (k > i)
    {
      /* swap */
      OperatorType t;
      OperatorType *T;
      OperatorType *I;
      OperatorType *K;

      T = &t;
      I = &OperatorTable[i];
      K = &OperatorTable[k];

      bwb_memcpy (T, I, sizeof (t));
      bwb_memcpy (I, K, sizeof (t));
      bwb_memcpy (K, T, sizeof (t));
    }
  }
}

static void
SortAllOperatorsForManual (void)        /* SortAllOperators() should be called aftwards */
{
  /* sort the operators by by precedence (high-to-low) then name (alphabetically). */
  int i;
   

  for (i = 0; i < NUM_OPERATORS - 1; i++)
  {
    int j;
    int k;
    int m;

    k = i;
    m = OperatorTable[i].ThisPrec;

    for (j = i + 1; j < NUM_OPERATORS; j++)
    {
      int n;
      n = OperatorTable[j].ThisPrec;
      if (n > m)
      {
        m = n;
        k = j;
      }
      else
        if (n == m
            && bwb_stricmp (OperatorTable[j].Name, OperatorTable[k].Name) < 0)
      {
        m = n;
        k = j;
      }
    }
    if (k > i)
    {
      /* swap */
      OperatorType t;
      OperatorType *T;
      OperatorType *I;
      OperatorType *K;

      T = &t;
      I = &OperatorTable[i];
      K = &OperatorTable[k];

      bwb_memcpy (T, I, sizeof (t));
      bwb_memcpy (I, K, sizeof (t));
      bwb_memcpy (K, T, sizeof (t));
    }
  }
}
static char
min_value_type (VariantType * X)
{
  /* returns the minimal TypeCode, based upon a NUMBER's value */
   
  assert (X != NULL);


  if (isnan (X->Number))
  {
      /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
    WARN_INTERNAL_ERROR;
    return NulChar;
  }
  if (X->Number == bwb_rint (X->Number))
  {
    /* INTEGER */
    if (MINBYT <= X->Number && X->Number <= MAXBYT)
    {
      return ByteTypeCode;
    }
    if (MININT <= X->Number && X->Number <= MAXINT)
    {
      return IntegerTypeCode;
    }
    if (MINLNG <= X->Number && X->Number <= MAXLNG)
    {
      return LongTypeCode;
    }
    if (MINCUR <= X->Number && X->Number <= MAXCUR)
    {
      return CurrencyTypeCode;
    }
  }
  /* FLOAT */
  if (MINSNG <= X->Number && X->Number <= MAXSNG)
  {
    return SingleTypeCode;
  }
  if (MINDBL <= X->Number && X->Number <= MAXDBL)
  {
    return DoubleTypeCode;
  }
  /* OVERFLOW */
  if (X->Number < 0)
  {
    X->Number = MINDBL;
  }
  else
  {
    X->Number = MAXDBL;
  }
  if (WARN_OVERFLOW)
  {
    /* ERROR */
  }
  /* CONTINUE */
  return DoubleTypeCode;
}



static char
max_number_type (char X, char Y)
{
  /* returns the maximal TypeCode, given two NUMBER TypeCode's */
   


  if (X == DoubleTypeCode || Y == DoubleTypeCode)
  {
    return DoubleTypeCode;
  }
  if (X == SingleTypeCode || Y == SingleTypeCode)
  {
    return SingleTypeCode;
  }
  if (X == CurrencyTypeCode || Y == CurrencyTypeCode)
  {
    return CurrencyTypeCode;
  }
  if (X == LongTypeCode || Y == LongTypeCode)
  {
    return LongTypeCode;
  }
  if (X == IntegerTypeCode || Y == IntegerTypeCode)
  {
    return IntegerTypeCode;
  }
  if (X == ByteTypeCode || Y == ByteTypeCode)
  {
    return ByteTypeCode;
  }
   /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
  WARN_INTERNAL_ERROR;
  return NulChar;
}
static char
math_type (VariantType * X, VariantType * Y)
{
  /*
   **
   ** Returns the TypeCode resulting from a math operation, such as addition.
   ** The return TypeCode should be the maximal of:
   ** a.  The original X's TypeCode.
   ** b.  The original Y's TypeCode.
   ** c.  The result's minimal TypeCode.
   **
   */
   
  assert (X != NULL);
  assert (Y != NULL);
  return
    max_number_type (max_number_type (X->VariantTypeCode, Y->VariantTypeCode),
                     min_value_type (X));
}

static char
Largest_TypeCode (char TypeCode, VariantType * X)
{
  assert (X != NULL);
  if (is_integer_type (X))
  {
    X->Number = bwb_rint (X->Number);
  }
  return max_number_type (TypeCode, min_value_type (X));
}
static int
is_string_type (VariantType * X)
{
  /* if value is a STRING, then TRUE, else FALSE */
   
  assert (X != NULL);
  switch (X->VariantTypeCode)
  {
  case ByteTypeCode:
  case IntegerTypeCode:
  case LongTypeCode:
  case CurrencyTypeCode:
  case SingleTypeCode:
  case DoubleTypeCode:
    if (X->Buffer != NULL)
    {
         /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
    return FALSE;
  case StringTypeCode:
    if (X->Buffer == NULL)
    {
         /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
    return TRUE;
  }
   /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
  WARN_INTERNAL_ERROR;
  return FALSE;
}
static int
is_number_type (VariantType * X)
{
  /* if value is a NUMBER, then TRUE, else FALSE */
   
  assert (X != NULL);
  switch (X->VariantTypeCode)
  {
  case ByteTypeCode:
  case IntegerTypeCode:
  case LongTypeCode:
  case CurrencyTypeCode:
  case SingleTypeCode:
  case DoubleTypeCode:
    if (X->Buffer != NULL)
    {
         /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
    return TRUE;
  case StringTypeCode:
    if (X->Buffer == NULL)
    {
         /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
    return FALSE;
  }
   /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
  WARN_INTERNAL_ERROR;
  return FALSE;                        /* never reached */
}
static int
is_integer_type (VariantType * X)
{
  /* if value is an INTEGER, then TRUE, else FALSE */
   
  assert (X != NULL);
  switch (X->VariantTypeCode)
  {
  case ByteTypeCode:
    return TRUE;
  case IntegerTypeCode:
    return TRUE;
  case LongTypeCode:
    return TRUE;
  case CurrencyTypeCode:
    return TRUE;
  case SingleTypeCode:
    return FALSE;
  case DoubleTypeCode:
    return FALSE;
  case StringTypeCode:
    return FALSE;
  }
   /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
  WARN_INTERNAL_ERROR;
  return FALSE;
}
static int
both_string_type (VariantType * X, VariantType * Y)
{
  /* if both values are a STRING, then TRUE, else FALSE */
   
  assert (X != NULL);
  assert (Y != NULL);
  if (is_string_type (X) && is_string_type (Y))
  {
    return TRUE;
  }
  return FALSE;
}
static int
both_number_type (VariantType * X, VariantType * Y)
{
  /* if both values are a NUMBER, then TRUE, else FALSE */
   
  assert (X != NULL);
  assert (Y != NULL);
  if (is_number_type (X) && is_number_type (Y))
  {
    return TRUE;
  }
  return FALSE;
}
static int
both_integer_type (VariantType * X, VariantType * Y)
{
  /* if both values are an INTEGER, then TRUE, else FALSE */
   
  assert (X != NULL);
  assert (Y != NULL);
  if (is_integer_type (X) && is_integer_type (Y))
  {
    return TRUE;
  }
  return FALSE;
}
static int
is_long_value (VariantType * X)
{
  /* if the NUMBER's value can be a LONG, then TRUE, else FALSE */
   
  assert (X != NULL);
  if (isnan (X->Number))
  {
      /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
    WARN_INTERNAL_ERROR;
    return FALSE;
  }
  if (X->Number == bwb_rint (X->Number))
  {
    if (MINCUR <= X->Number && X->Number <= MAXCUR)
    {
      return TRUE;
    }
  }
  return FALSE;
}
static int
both_are_long (VariantType * X, VariantType * Y)
{
  /* if both values can be a LONG, then TRUE, else FALSE */
   
  assert (X != NULL);
  assert (Y != NULL);
  if (is_long_value (X) && is_long_value (Y))
  {
    return TRUE;
  }
  return FALSE;
}
static int
bwb_isodigit (int C)
{
   
  switch (C)
  {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
    return TRUE;
  }
  return FALSE;
}



/*
--------------------------------------------------------------------------------------------
                               Operators
--------------------------------------------------------------------------------------------
*/

static ResultType
OP_ADD (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_number_type (X, Y))
  {
    /* X = (X + Y) */
    X->Number += Y->Number;
    if (both_integer_type (X, Y))
    {
      X->Number = bwb_rint (X->Number);
    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  if (both_string_type (X, Y))
  {
    /* X$ = (X$ + Y$) */
    return OP_AMP (X, Y);
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_AMP (VariantType * X, VariantType * Y)
{
  /* X$ = (X  & Y ) */
  /* X$ = (X  & Y$) */
  /* X$ = (X$ & Y ) */
  /* X$ = (X$ & Y$) */
  size_t CharsRemaining;
  VariantType t;
  VariantType *T;
   
  assert (X != NULL);
  assert (Y != NULL);

  T = &t;
  if (X->VariantTypeCode != StringTypeCode)
  {
    /* coerce X to X$ */
    if ((X->Buffer = (char *) calloc (NUMLEN, sizeof (char))) == NULL)        /* free() called by OP_ADD() */
    {
      WARN_OUT_OF_MEMORY;
      return RESULT_ERROR;
    }
    FormatBasicNumber (X->Number, X->Buffer);
    X->Length = bwb_strlen (X->Buffer);
    X->VariantTypeCode = StringTypeCode;
  }
  if (Y->VariantTypeCode != StringTypeCode)
  {
    /* coerce Y to Y$ */
    if ((Y->Buffer = (char *) calloc (NUMLEN, sizeof (char))) == NULL)        /* free() called by OP_ADD() */
    {
      WARN_OUT_OF_MEMORY;
      return RESULT_ERROR;
    }
    FormatBasicNumber (Y->Number, Y->Buffer);
    Y->Length = bwb_strlen (Y->Buffer);
    Y->VariantTypeCode = StringTypeCode;
  }
  if (X->Length > MAXLEN)
  {
    WARN_STRING_TOO_LONG;
    X->Length = MAXLEN;
  }
  if (Y->Length > MAXLEN)
  {
    WARN_STRING_TOO_LONG;
    Y->Length = MAXLEN;
  }
  T->VariantTypeCode = StringTypeCode;
  T->Length = X->Length + Y->Length;
  if (T->Length > MAXLEN)
  {
    WARN_STRING_TOO_LONG;
    T->Length = MAXLEN;
  }
  /* we always allocate a buffer, even for non-empty strings */
  if ((T->Buffer =
       (char *) calloc (T->Length + 1 /* NulChar */ , sizeof (char))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return RESULT_ERROR;
  }
  CharsRemaining = T->Length;
  if (X->Length > CharsRemaining)
  {
    X->Length = CharsRemaining;
  }
  if (X->Length > 0)
  {
    bwb_memcpy (T->Buffer, X->Buffer, X->Length);
    CharsRemaining -= X->Length;
  }
  if (Y->Length > CharsRemaining)
  {
    Y->Length = CharsRemaining;
  }
  if (Y->Length > 0)
  {
    bwb_memcpy (&T->Buffer[X->Length], Y->Buffer, Y->Length);
    CharsRemaining -= Y->Length;
  }
  if (CharsRemaining != 0)
  {
    WARN_INTERNAL_ERROR;
    return RESULT_ERROR;
  }
  T->Buffer[T->Length] = NulChar;
  RELEASE_VARIANT (X);
  RELEASE_VARIANT (Y);
  COPY_VARIANT (X, T);
  return RESULT_SUCCESS;
}
static ResultType
OP_SUB (VariantType * X, VariantType * Y)
{
  /* X = (X - Y) */
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_number_type (X, Y))
  {
    X->Number -= Y->Number;
    if (both_integer_type (X, Y))
    {
      X->Number = bwb_rint (X->Number);
    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_MUL (VariantType * X, VariantType * Y)
{
  /* X = (X * Y) */
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_number_type (X, Y))
  {
    X->Number *= Y->Number;
    if (both_integer_type (X, Y))
    {
      X->Number = bwb_rint (X->Number);
    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_IDIV (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (both_number_type (X, Y))
  {
    /* X = (X \ Y) */
    X->Number = bwb_rint (X->Number);
    Y->Number = bwb_rint (Y->Number);
    if (Y->Number == 0)
    {
      /* - Evaluation of an expression results in division
       * by zero (nonfatal, the recommended recovery
       * procedure is to supply machine infinity with the
       * sign of the numerator and continue) 
       */
      if (X->Number < 0)
      {
        /* NEGATIVE */
        X->Number = MINDBL;        /* NEGATIVE INFINITY */
      }
      else
      {
        /* POSITIVE  */
        X->Number = MAXDBL;        /* POSITIVE INFINITY */
      }
      if (WARN_DIVISION_BY_ZERO)
      {
        return RESULT_ERROR;
      }
      /* CONTINUE */
    }
    else
    {
      DoubleType N;

      N = bwb_rint (X->Number / Y->Number);
      if (My->CurrentVersion->OptionVersionValue & (R86))
      {
        /* for RBASIC's RESIDUE function */
        My->RESIDUE = bwb_rint (X->Number - N * Y->Number);
      }
      X->Number = N;

    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_DIV (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_number_type (X, Y))
  {
    /* X = (X / Y) */
    if (both_integer_type (X, Y))
    {
      return OP_IDIV (X, Y);
    }
    if (Y->Number == 0)
    {
      /* - Evaluation of an expression results in division
       * by zero (nonfatal, the recommended recovery
       * procedure is to supply machine infinity with the
       * sign of the numerator and continue) 
       */
      if (X->Number < 0)
      {
        /* NEGATIVE */
        X->Number = MINDBL;        /* NEGATIVE INFINITY */
      }
      else
      {
        /* POSITIVE  */
        X->Number = MAXDBL;        /* POSITIVE INFINITY */
      }
      if (WARN_DIVISION_BY_ZERO)
      {
        return RESULT_ERROR;
      }
      /* CONTINUE */
    }
    else
    {
      X->Number /= Y->Number;
    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_MOD (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_number_type (X, Y))
  {
    /* X = (X MOD Y) */
    X->Number = bwb_rint (X->Number);
    Y->Number = bwb_rint (Y->Number);
    if (Y->Number == 0)
    {
      /* - Evaluation of an expression results in division
       * by zero (nonfatal, the recommended recovery
       * procedure is to supply machine infinity with the
       * sign of the numerator and continue) 
       */
      if (X->Number < 0)
      {
        /* NEGATIVE */
        X->Number = MINDBL;        /* NEGATIVE INFINITY */
      }
      else
      {
        /* POSITIVE  */
        X->Number = MAXDBL;        /* POSITIVE INFINITY */
      }
      if (WARN_DIVISION_BY_ZERO)
      {
        return RESULT_ERROR;
      }
      /* CONTINUE */
    }
    else
    {
      DoubleType N;
      DoubleType I;
      N = X->Number / Y->Number;
      modf (N, &I);
      N = X->Number - Y->Number * I;
      X->Number = bwb_rint (N);
    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_EXP (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_number_type (X, Y))
  {
    /* X = (X ^ Y) */
    if (X->Number < 0 && Y->Number != bwb_rint (Y->Number))
    {
         /*** FATAL ***/
      /* - Evaluation of the operation of
       * involution results in a negative number
       * being raised to a non-integral power
       * (fatal). */
      X->Number = 0;
      WARN_ILLEGAL_FUNCTION_CALL;
      return RESULT_ERROR;
    }
    if (X->Number == 0 && Y->Number < 0)
    {
      /* - Evaluation of the operation of
       * involution results in a zero being
       * raised to a negative value (nonfatal, the
       * recommended recovery procedure is to
       * supply positive machine infinity and
       * continue). */

      X->Number = MAXDBL;
      if (WARN_OVERFLOW)
      {
        /* ERROR */
      }
      /* CONTINUE */
    }
    else
    {
      X->Number = pow (X->Number, Y->Number);
    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_NEG (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y == NULL);
  if (Y != NULL)
  {
    WARN_INTERNAL_ERROR;
    return RESULT_ERROR;
  }
  if (is_number_type (X))
  {
    /* X = (- X) */
    X->Number = -X->Number;
    X->VariantTypeCode = min_value_type (X);
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_POS (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y == NULL);
  if (Y != NULL)
  {
    WARN_INTERNAL_ERROR;
    return RESULT_ERROR;
  }
  if (is_number_type (X))
  {
    /* X = (+ X) */
    /*
       X->Number = X->Number;
       X->VariantTypeCode = min_value_type( X );
     */
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_OR (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  
  if (both_number_type (X, Y))
  {
    /* X = (X OR Y) */
    if (both_are_long (X, Y))
    {
      long x;
      long y;

      x = (long) bwb_rint (X->Number);
      y = (long) bwb_rint (Y->Number);

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* OR */ )
      {
        if (x)
        {
          x = -1;
        }
        if (y)
        {
          y = -1;
        }
      }

      x = x | y;

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* OR */ )
      {
        if (x)
        {
          x = 1;
        }
      }

      X->Number = x;
      X->VariantTypeCode = min_value_type (X);
      return RESULT_SUCCESS;
    }
    WARN_OVERFLOW;
    return RESULT_ERROR;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_AND (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (both_number_type (X, Y))
  {
    /* X = (X AND Y) */
    if (both_are_long (X, Y))
    {
      long x;
      long y;

      x = (long) bwb_rint (X->Number);
      y = (long) bwb_rint (Y->Number);

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* AND */ )
      {
        if (x)
        {
          x = -1;
        }
        if (y)
        {
          y = -1;
        }
      }

      x = x & y;

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* AND */ )
      {
        if (x)
        {
          x = 1;
        }
      }

      X->Number = x;
      X->VariantTypeCode = min_value_type (X);
      return RESULT_SUCCESS;
    }
    WARN_OVERFLOW;
    return RESULT_ERROR;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_XOR (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (both_number_type (X, Y))
  {
    /* X = (X XOR Y) */
    if (both_are_long (X, Y))
    {
      long x;
      long y;

      x = (long) bwb_rint (X->Number);
      y = (long) bwb_rint (Y->Number);

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* XOR */ )
      {
        if (x)
        {
          x = -1;
        }
        if (y)
        {
          y = -1;
        }
      }

      x = x ^ y;

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* XOR */ )
      {
        if (x)
        {
          x = 1;
        }
      }

      X->Number = x;
      X->VariantTypeCode = min_value_type (X);
      return RESULT_SUCCESS;
    }
    WARN_OVERFLOW;
    return RESULT_ERROR;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_EQV (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (both_number_type (X, Y))
  {
    /* X = (X EQV Y)  = NOT ( X XOR Y ) */
    if (both_are_long (X, Y))
    {
      long x;
      long y;

      x = (long) bwb_rint (X->Number);
      y = (long) bwb_rint (Y->Number);

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* EQV */ )
      {
        if (x)
        {
          x = -1;
        }
        if (y)
        {
          y = -1;
        }
      }

      x = ~(x ^ y);

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* EQV */ )
      {
        if (x)
        {
          x = 1;
        }
      }

      X->Number = x;
      X->VariantTypeCode = min_value_type (X);
      return RESULT_SUCCESS;
    }
    WARN_OVERFLOW;
    return RESULT_ERROR;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_IMP (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (both_number_type (X, Y))
  {
    /* X = (X IMP Y)  = (X AND Y) OR (NOT X) */
    if (both_are_long (X, Y))
    {
      long x;
      long y;

      x = (long) bwb_rint (X->Number);
      y = (long) bwb_rint (Y->Number);

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* IMP */ )
      {
        if (x)
        {
          x = -1;
        }
        if (y)
        {
          y = -1;
        }
      }

      x = (x & y) | (~x);

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* IMP */ )
      {
        if (x)
        {
          x = 1;
        }
      }

      X->Number = x;
      X->VariantTypeCode = min_value_type (X);
      return RESULT_SUCCESS;
    }
    WARN_OVERFLOW;
    return RESULT_ERROR;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_NOT (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y == NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (Y != NULL)
  {
    WARN_INTERNAL_ERROR;
    return RESULT_ERROR;
  }
  if (is_number_type (X))
  {
    /* X = (NOT X) */
    if (is_long_value (X))
    {
      long x;

      x = (long) bwb_rint (X->Number);

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* NOT */ )
      {
        if (x)
        {
          x = -1;
        }
      }

      x = ~x;

      if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_BOOLEAN) /* NOT */ )
      {
        if (x)
        {
          x = 1;
        }
      }

      X->Number = x;
      X->VariantTypeCode = min_value_type (X);
      return RESULT_SUCCESS;
    }
    WARN_OVERFLOW;
    return RESULT_ERROR;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_MAX (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_number_type (X, Y))
  {
    /* X = (X MAX Y) = IIF( X < Y, Y, X ) */
    if (X->Number < Y->Number)
    {
      X->Number = Y->Number;
    }
    if (both_integer_type (X, Y))
    {
      X->Number = bwb_rint (X->Number);
    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  if (both_string_type (X, Y))
  {
    /* X$ = ( X$ MAX Y$ ) == IIF( X$ < Y$,  Y$, X$ ) */
    if (bwb_stricmp (X->Buffer, Y->Buffer) < 0)
    {
      RELEASE_VARIANT (X);
      COPY_VARIANT (X, Y);
    }
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_MIN (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_number_type (X, Y))
  {
    /* X = (X MIN Y) = IIF( X > Y, Y, X ) */
    if (X->Number > Y->Number)
    {
      X->Number = Y->Number;
    }
    if (both_integer_type (X, Y))
    {
      X->Number = bwb_rint (X->Number);
    }
    X->VariantTypeCode = math_type (X, Y);
    return RESULT_SUCCESS;
  }
  if (both_string_type (X, Y))
  {
    /* X$ = ( X$ MIN Y$ ) == IIF( X$ > Y$, Y$, X$ ) */
    if (bwb_stricmp (X->Buffer, Y->Buffer) > 0)
    {
      RELEASE_VARIANT (X);
      COPY_VARIANT (X, Y);
    }
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}

/*
COMPARISON OPERATORS - these all return a TRUE/FALSE result in X
*/


/* ------------------- equality */

static ResultType
test_eq (VariantType * X, VariantType * Y, int TrueValue, int FalseValue)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (both_number_type (X, Y))
  {
    /* X = IIF( X = Y, TrueValue, FalseValue ) */
    if (both_are_long (X, Y))
    {
      long x;
      long y;

      x = (long) bwb_rint (X->Number);
      y = (long) bwb_rint (Y->Number);

      if (x == y)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    else
    {
      if (X->Number == Y->Number)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }

    }
    X->VariantTypeCode = IntegerTypeCode;
    return RESULT_SUCCESS;
  }
  if (both_string_type (X, Y))
  {
    /* X = IIF( X$ = Y$, TrueValue, FalseValue ) */
    /* NOTE: embedded NulChar terminate comparison */
    if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
    {
      /* case insensitive */
      if (bwb_stricmp (X->Buffer, Y->Buffer) == 0)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    else
    {
      /* case sensitive */
      if (bwb_strcmp (X->Buffer, Y->Buffer) == 0)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    RELEASE_VARIANT (X);
    RELEASE_VARIANT (Y);
    X->VariantTypeCode = IntegerTypeCode;
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_EQ (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  return test_eq (X, Y, TRUE, FALSE);
}
static ResultType
OP_NE (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  return test_eq (X, Y, FALSE, TRUE);
}

/* ------------------- greater */

static ResultType
test_gt (VariantType * X, VariantType * Y, int TrueValue, int FalseValue)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (both_number_type (X, Y))
  {
    /* X = IIF( X > Y, TrueValue, FalseValue ) */
    if (both_are_long (X, Y))
    {
      long x;
      long y;

      x = (long) bwb_rint (X->Number);
      y = (long) bwb_rint (Y->Number);

      if (x > y)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    else
    {
      if (X->Number > Y->Number)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }

    }
    X->VariantTypeCode = IntegerTypeCode;
    return RESULT_SUCCESS;
  }
  if (both_string_type (X, Y))
  {
    /* X = IIF( X$ > Y$, TrueValue, FalseValue ) */
    /* NOTE: embedded NUL characters terminate comparison */
    if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
    {
      /* case insensitive */
      if (bwb_stricmp (X->Buffer, Y->Buffer) > 0)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    else
    {
      /* case sensitive */
      if (bwb_strcmp (X->Buffer, Y->Buffer) > 0)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    RELEASE_VARIANT (X);
    RELEASE_VARIANT (Y);
    X->VariantTypeCode = IntegerTypeCode;
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_GT (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  return test_gt (X, Y, TRUE, FALSE);
}
static ResultType
OP_LE (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  return test_gt (X, Y, FALSE, TRUE);
}

/* ------------------- lesser */

static ResultType
test_lt (VariantType * X, VariantType * Y, int TrueValue, int FalseValue)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (both_number_type (X, Y))
  {
    /* X = IIF( X < Y, TrueValue, FalseValue ) */
    if (both_are_long (X, Y))
    {
      long x;
      long y;

      x = (long) bwb_rint (X->Number);
      y = (long) bwb_rint (Y->Number);

      if (x < y)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    else
    {
      if (X->Number < Y->Number)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }

    }
    X->VariantTypeCode = IntegerTypeCode;
    return RESULT_SUCCESS;
  }
  if (both_string_type (X, Y))
  {
    /* X = IIF( X$ < Y$, TrueValue, FalseValue ) */
    /* NOTE: embedded NUL characters terminate comparison */
    if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
    {
      /* case insensitive */
      if (bwb_stricmp (X->Buffer, Y->Buffer) < 0)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    else
    {
      /* case sensitive */
      if (bwb_strcmp (X->Buffer, Y->Buffer) < 0)
      {
        X->Number = TrueValue;
      }
      else
      {
        X->Number = FalseValue;
      }
    }
    RELEASE_VARIANT (X);
    RELEASE_VARIANT (Y);
    X->VariantTypeCode = IntegerTypeCode;
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}
static ResultType
OP_LT (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  return test_lt (X, Y, TRUE, FALSE);
}
static ResultType
OP_GE (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  return test_lt (X, Y, FALSE, TRUE);
}

/* ------------------- like */

static ResultType
OP_LIKE (VariantType * X, VariantType * Y)
{
   
  assert (X != NULL);
  assert (Y != NULL);
  if (both_string_type (X, Y))
  {
    /* X = (X$ LIKE Y$) */
    int X_count;
    int Y_count;

    X_count = 0;
    Y_count = 0;

    if (IsLike (X->Buffer, &X_count, X->Length,
                Y->Buffer, &Y_count, Y->Length))
    {
      X->Number = TRUE;
    }
    else
    {
      X->Number = FALSE;
    }
    RELEASE_VARIANT (X);
    RELEASE_VARIANT (Y);
    X->VariantTypeCode = IntegerTypeCode;
    return RESULT_SUCCESS;
  }
  WARN_TYPE_MISMATCH;
  return RESULT_ERROR;
}


/*
--------------------------------------------------------------------------------------------
                               Line Parsing Utilities
--------------------------------------------------------------------------------------------
*/

static OperatorType *
buff_read_operator (char *buffer, int *position, unsigned char LastPrec,
                    unsigned char Arity)
{
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  p = *position;
  if (bwb_isalpha (buffer[p]))
  {
    /* only consider alphabetic operators */
    /* spaces between any character of the operator is not allowed */
    char name[NameLengthMax + 1];

    if (buff_read_varname (buffer, &p, name))
    {
      int i;
      for (i = 0; i < NUM_OPERATORS; i++)
      {
        OperatorType *T;

        T = &OperatorTable[i];
        if (T->OptionVersionBitmask & My->CurrentVersion->OptionVersionValue)
        {
          if (T->ThisPrec >= LastPrec && T->Arity == Arity
              && T->IsAlpha == IS_ALPHA)
          {
            /* possible */
            if (bwb_stricmp (T->Name, name) == 0)
            {
              /* FOUND */
              *position = p;
              return T;
            }
          }
        }
      }
    }
  }
  else
  {
    /* only consider non-alphabetic operators */
    /* spaces between any character of the operator is allowed */
    int i;
    for (i = 0; i < NUM_OPERATORS; i++)
    {
      OperatorType *T;

      T = &OperatorTable[i];
      if (T->OptionVersionBitmask & My->CurrentVersion->OptionVersionValue)
      {
        if (T->ThisPrec >= LastPrec && T->Arity == Arity
            && T->IsAlpha == NO_ALPHA)
        {
          /* possible */
          int m;                /* number of characters actually matched */
          int n;                /* number of characters to match */
          int q;                /* position after skipping the characters */

          n = bwb_strlen (T->Name);        /* number of characters to match */
          q = p;

          for (m = 0; m < n && buff_skip_char (buffer, &q, T->Name[m]); m++);
          if (m == n)
          {
            /* FOUND */
            *position = q;
            return T;
          }
        }
      }
    }
  }
  /* NOT FOUND */
  return NULL;
}

#if FALSE                        /* keep line_... */
static OperatorType *
line_read_operator (LineType * line, unsigned char LastPrec,
                    unsigned char Arity)
{
   
  assert (line != NULL);
  return buff_read_operator (line->buffer, &(line->position), LastPrec,
                             Arity);
}
#endif
static ResultType
buff_read_string_constant (char *buffer, int *position, VariantType * X)
{
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  p = *position;
  if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
  {
    int q;                        /* start of constant */
    X->VariantTypeCode = StringTypeCode;
    p++;                        /* skip leading quote */
    /* determine the length of the quoted string */
    X->Length = 0;
    q = p;
    while (buffer[p])
    {
      if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
      {
        p++;                        /* quote */
        if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
        {
          /* embedded string "...""..." */
        }
        else
        {
          /* properly terminated string "...xx..." */
          break;
        }
      }
      X->Length++;
      p++;
    }
    if ((X->Buffer =
         (char *) calloc (X->Length + 1 /* NulChar */ ,
                          sizeof (char))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return RESULT_ERROR;
    }
    /* copy the quoted string */
    X->Length = 0;
    p = q;
    while (buffer[p])
    {
      if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
      {
        p++;                        /* skip quote */
        if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
        {
          /* embedded string "...""..." */
        }
        else
        {
          /* properly terminated string "...xx..." */
          break;
        }
      }
      X->Buffer[X->Length] = buffer[p];
      X->Length++;
      p++;
    }
    X->Buffer[X->Length] = NulChar;
    *position = p;
    return RESULT_SUCCESS;
  }
  /* NOT FOUND */
  return RESULT_UNPARSED;
}

#if FALSE                        /* keep line_... */
static ResultType
line_read_string_constant (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_string_constant (line->buffer, &(line->position), X);
}
#endif
extern ResultType
buff_read_hexadecimal_constant (char *buffer, int *position, VariantType * X,
                                int IsConsoleInput)
{
  /* &h... */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);


  p = *position;
  if (My->CurrentVersion->OptionFlags & OPTION_BUGS_ON ) /* allows hexadecimal constants */
  {
    if (buffer[p] == '&')
    {
      p++;                        /* skip '&' */
      if (bwb_tolower (buffer[p]) == 'h')
      {
        /* &h... */
        p++;                        /* skip 'h' */
        if (bwb_isxdigit (buffer[p]))
        {
          /* &hABCD */
          int n;                /* number of characters read */
          unsigned long x;        /* value read */

          n = 0;
          x = 0;

          /* if( sscanf( &buffer[ p ], "%lx%n", &x, &n ) == 1 ) */
          if (sscanf (&buffer[p], HexScanFormat, &x, &n) == 1)
          {
            /* FOUND */
            p += n;

            X->Number = x;
            X->VariantTypeCode = min_value_type (X);
            if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_ON)) /* TypeSuffix allowed on constants */
            {
              char TypeCode;
              TypeCode = Char_to_TypeCode (buffer[p]);
              switch (TypeCode)
              {
              case ByteTypeCode:
              case IntegerTypeCode:
              case LongTypeCode:
              case CurrencyTypeCode:
              case SingleTypeCode:
              case DoubleTypeCode:
                p++;                /* skip TypeCode */
                /* verify the value actually fits in the declared type */
                X->VariantTypeCode = TypeCode;
                TypeCode = Largest_TypeCode (TypeCode, X);
                if (X->VariantTypeCode != TypeCode)
                {
                  /* declared type is too small */
                  if (IsConsoleInput)
                  {
                    /*
                     **
                     ** The user will re-enter the data
                     **
                     */
                    return RESULT_UNPARSED;
                  }
                  if (WARN_OVERFLOW)
                  {
                    /* ERROR */
                    return RESULT_ERROR;
                  }
                  /* CONTINUE */
                  X->VariantTypeCode = TypeCode;
                }
                break;
              case StringTypeCode:
                /* oops */
                if (IsConsoleInput)
                {
                  /*
                   **
                   ** The user will re-enter the data
                   **
                   */
                  return RESULT_UNPARSED;
                }
                WARN_SYNTAX_ERROR;
                return RESULT_ERROR;
                /* break; */
              default:
                X->VariantTypeCode = min_value_type (X);
              }
            }
            *position = p;
            return RESULT_SUCCESS;
          }
        }
        /* not HEXADECIMAL */
      }
    }
  }
  /* NOT FOUND */
  return RESULT_UNPARSED;
}

#if FALSE                        /* keep line_... */
static ResultType
line_read_hexadecimal_constant (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_hexadecimal_constant (line->buffer, &(line->position), X,
                                         FALSE);
}
#endif
extern ResultType
buff_read_octal_constant (char *buffer, int *position, VariantType * X,
                          int IsConsoleInput)
{
  /* &o... */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);


  p = *position;

  if (My->CurrentVersion->OptionFlags & OPTION_BUGS_ON ) /* allows octal constants */
  {
    if (buffer[p] == '&')
    {
      p++;                        /* skip '&' */
      if (bwb_tolower (buffer[p]) == 'o')
      {
        /* &o777 */
        p++;                        /* skip 'o' */
        /* fall-thru */
      }
      if (bwb_isodigit (buffer[p]))
      {
        /* &o777 */
        /* &777 */
        int n;                        /* number of characters read */
        unsigned long x;        /* value read */

        n = 0;
        x = 0;

        /* if( sscanf( &buffer[ p ], "%64lo%n", &x, &n ) == 1 ) */
        if (sscanf (&buffer[p], OctScanFormat, &x, &n) == 1)
        {
          /* FOUND */
          p += n;

          X->Number = x;
          X->VariantTypeCode = min_value_type (X);
          if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_ON)) /* TypeSuffix allowed on constants */
          {
            char TypeCode;

            TypeCode = Char_to_TypeCode (buffer[p]);
            switch (TypeCode)
            {
            case ByteTypeCode:
            case IntegerTypeCode:
            case LongTypeCode:
            case CurrencyTypeCode:
            case SingleTypeCode:
            case DoubleTypeCode:
              p++;                /* skip TypeCode */
              /* verify the value actually fits in the declared type */
              X->VariantTypeCode = TypeCode;
              TypeCode = Largest_TypeCode (TypeCode, X);
              if (X->VariantTypeCode != TypeCode)
              {
                /* declared type is too small */
                if (IsConsoleInput)
                {
                  /*
                   **
                   ** The user will re-enter the data
                   **
                   */
                  return RESULT_UNPARSED;
                }
                if (WARN_OVERFLOW)
                {
                  /* ERROR */
                  return RESULT_ERROR;
                }
                /* CONTINUE */
                X->VariantTypeCode = TypeCode;
              }
              break;
            case StringTypeCode:
              /* oops */
              if (IsConsoleInput)
              {
                /*
                 **
                 ** The user will re-enter the data
                 **
                 */
                return RESULT_UNPARSED;
              }
              WARN_SYNTAX_ERROR;
              return RESULT_ERROR;
              /* break; */
            default:
              X->VariantTypeCode = min_value_type (X);
            }
          }
          *position = p;
          return RESULT_SUCCESS;
        }
      }
    }
  }
  /* NOT FOUND */
  return RESULT_UNPARSED;
}

#if FALSE                        /* keep line_... */
static ResultType
line_read_octal_constant (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_octal_constant (line->buffer, &(line->position), X, FALSE);
}
#endif
static ResultType
buff_read_internal_constant (char *buffer, int *position, VariantType * X)
{
  /* &... */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);


  p = *position;

  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    /* IBM System/360 and System/370 BASIC dialects */
    if (buffer[p] == '&')
    {
      p++;                        /* skip '&' */
      if (bwb_isalpha (buffer[p]))
      {
        char *S;
        S = &(buffer[p]);
        if (bwb_strnicmp (S, "PI", 2) == 0)
        {
          /* &PI */
          p += 2;
          X->Number = 3.14159265358979;
          X->VariantTypeCode = DoubleTypeCode;
          *position = p;
          return RESULT_SUCCESS;
        }
        if (bwb_strnicmp (S, "E", 1) == 0)
        {
          /* &E */
          p += 1;
          X->Number = 2.71828182845905;
          X->VariantTypeCode = DoubleTypeCode;
          *position = p;
          return RESULT_SUCCESS;
        }
        if (bwb_strnicmp (S, "SQR2", 4) == 0)
        {
          /* &SQR2 */
          p += 4;
          X->Number = 1.41421356237309;
          X->VariantTypeCode = DoubleTypeCode;
          *position = p;
          return RESULT_SUCCESS;
        }
        /* NOT a magic word */
      }
    }
  }
  /* NOT FOUND */
  return RESULT_UNPARSED;
}

#if FALSE                        /* keep line_... */
static ResultType
line_read_internal_constant (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_internal_constant (line->buffer, &(line->position), X);
}
#endif
extern ResultType
buff_read_decimal_constant (char *buffer, int *position, VariantType * X,
                            int IsConsoleInput)
{
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);


  p = *position;
  if (bwb_isdigit (buffer[p]) || buffer[p] == '.')
  {
    /* .12345 */
    /* 123.45 */
    /* 123456 */
    /* 123E45 */
    /* TODO:  'D' instead of 'E' */
    int n;                        /* number of characters read */
    DoubleType x;                /* value read */


    n = 0;
    x = 0;

    /* if( sscanf( &buffer[ p ], "%lg%n", &X->Number, &n ) == 1 ) */
    if (sscanf (&buffer[p], DecScanFormat, &x, &n) == 1)
    {
      /* FOUND */
      p += n;

      /* VerifyNumeric */
      if (isnan (x))
      {
            /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
        WARN_INTERNAL_ERROR;
        return RESULT_ERROR;
      }
      if (isinf (x))
      {
        /* - Evaluation of an expression results in an overflow
         * (nonfatal, the recommended recovery procedure is to supply
         * machine in- finity with the algebraically correct sign and
         * continue). */
        if (x < 0)
        {
          x = MINDBL;
        }
        else
        {
          x = MAXDBL;
        }
        if (IsConsoleInput)
        {
          /*
           **
           ** The user will re-enter the data
           **
           */
          return RESULT_UNPARSED;
        }
        if (WARN_OVERFLOW)
        {
          /* ERROR */
          return RESULT_ERROR;
        }
        /* CONTINUE */
      }
      /* OK */
      X->Number = x;
      X->VariantTypeCode = DoubleTypeCode;        /* min_value_type( X ); */
      if (My->CurrentVersion->OptionFlags & OPTION_BUGS_ON ) /* TypeSuffix allowed on constants */
      {
        char TypeCode;
        TypeCode = Char_to_TypeCode (buffer[p]);
        switch (TypeCode)
        {
        case ByteTypeCode:
        case IntegerTypeCode:
        case LongTypeCode:
        case CurrencyTypeCode:
        case SingleTypeCode:
        case DoubleTypeCode:
          p++;                        /* skip TypeCode */
          /* verify the value actually fits in the declared type */
          X->VariantTypeCode = TypeCode;
          TypeCode = Largest_TypeCode (TypeCode, X);
          if (X->VariantTypeCode != TypeCode)
          {
            /* declared type is too small */
            if (IsConsoleInput)
            {
              /*
               **
               ** The user will re-enter the data
               **
               */
              return RESULT_UNPARSED;
            }
            if (WARN_OVERFLOW)
            {
              /* ERROR */
              return RESULT_ERROR;
            }
            /* CONTINUE */
            X->VariantTypeCode = TypeCode;
          }
          break;
        case StringTypeCode:
          /* oops */
          if (IsConsoleInput)
          {
            /*
             **
             ** The user will re-enter the data
             **
             */
            return RESULT_UNPARSED;
          }
          WARN_SYNTAX_ERROR;
          return RESULT_ERROR;
          /* break; */
        default:
          X->VariantTypeCode = DoubleTypeCode;        /* min_value_type( X ); */
        }
      }
      *position = p;
      return RESULT_SUCCESS;
    }
  }
  /* NOT FOUND */
  return RESULT_UNPARSED;
}

#if FALSE                        /* keep line_... */
static int
line_read_decimal_constant (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_decimal_constant (line->buffer, &(line->position), X,
                                     FALSE);
}
#endif

static ResultType
buff_read_function (char *buffer, int *position, VariantType * X)
{
  int p;
  char name[NameLengthMax + 1];
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);


  p = *position;
  if (buff_read_varname (buffer, &p, name))
  {
    if (UserFunction_name (name) || IntrinsicFunction_name (name))
    {
      /* ---------------------------------------------------------------------------- */
      /* if( TRUE ) */
      {
        /* here we handle some pseudo-functions that return information about arrays */
        char Xbound;

        Xbound = NulChar;
        if (buff_peek_LparenChar (buffer, &p))
        {
          if (bwb_stricmp (name, "DET") == 0)
          {
            /* N = DET( varname ) */
            /* N = DET is handled by F_DET_N */
            Xbound = 'd';
          }
          else if (bwb_stricmp (name, "DIM") == 0)
          {
            /* N = DIM( varname ) */
            /* return total number of dimensions */
            Xbound = 'D';
          }
          else if (bwb_stricmp (name, "SIZE") == 0)
          {
            if (My->CurrentVersion->OptionVersionValue & (C77))
            {
              /* N = SIZE( filename ) is handled by F_SIZE_A_N */
            }
            else
            {
              /* N = SIZE( varname ) */
              /* return total number of elements */
              Xbound = 'S';
            }
          }
          else if (bwb_stricmp (name, "LBOUND") == 0)
          {
            /* N = LBOUND( varname [ , dimension ] ) */
            /* return LOWER bound */
            Xbound = 'L';
          }
          else if (bwb_stricmp (name, "UBOUND") == 0)
          {
            /* N = UBOUND( varname [ , dimension ] ) */
            /* return UPPER bound */
            Xbound = 'U';
          }
        }
        if (Xbound)
        {
          VariableType *v;
          int dimension;
          char varname[NameLengthMax + 1];

          v = NULL;
          dimension = 0;        /* default */


          if (buff_skip_LparenChar (buffer, &p) == FALSE)
          {
            WARN_SYNTAX_ERROR;
            return RESULT_ERROR;
          }
          if (buff_read_varname (buffer, &p, varname) == FALSE)
          {
            WARN_SYNTAX_ERROR;
            return RESULT_ERROR;
          }
          /* search for array */
          v = mat_find (varname);
          if (v == NULL)
          {
            WARN_TYPE_MISMATCH;
            return RESULT_ERROR;
          }
          if (v->dimensions == 0)
          {
            /* calling DET(), DIM(), SIZE(), LBOUND() or UBOUND() on a scalar is an ERROR */
            WARN_TYPE_MISMATCH;
            return RESULT_ERROR;
          }
          switch (Xbound)
          {
          case 'd':                /* DET() */
          case 'D':                /* DIM() */
          case 'S':                /* SIZE() */
            break;
          case 'L':                /* LBOUND() */
          case 'U':                /* UBOUND() */
            if (buff_skip_seperator (buffer, &p))
            {
              ResultType ResultCode;
              VariantType t;
              VariantType *T;

              T = &t;
              ResultCode = buff_read_expr (buffer, &p, T, 1);
              if (ResultCode != RESULT_SUCCESS)
              {
                /* ERROR */
                RELEASE_VARIANT (T);
                return ResultCode;
              }
              if (is_string_type (T))
              {
                RELEASE_VARIANT (T);
                WARN_TYPE_MISMATCH;
                return RESULT_ERROR;
              }
              T->Number = bwb_rint (T->Number);
              if (T->Number < 1 || T->Number > v->dimensions)
              {
                WARN_TYPE_MISMATCH;
                return RESULT_ERROR;
              }
              dimension = (int) bwb_rint (T->Number);
              dimension--;        /* BASIC to C */
            }
            else
            {
              dimension = 0;        /* default */
            }
            break;
          default:
            WARN_INTERNAL_ERROR;
            return RESULT_ERROR;
            /* break; */
          }
          if (buff_skip_RparenChar (buffer, &p) == FALSE)
          {
            WARN_SYNTAX_ERROR;
            return RESULT_ERROR;
          }
          /* OK */
          switch (Xbound)
          {
          case 'd':                /* DET() */
            Determinant (v);
            X->Number = My->LastDeterminant;
            break;
          case 'D':                /* DIM() */
            X->Number = v->dimensions;
            break;
          case 'S':                /* SIZE() */
            X->Number = v->array_units;
            break;
          case 'L':                /* LBOUND() */
            X->Number = v->LBOUND[dimension];
            break;
          case 'U':                /* UBOUND() */
            X->Number = v->UBOUND[dimension];
            break;
          default:
            WARN_INTERNAL_ERROR;
            return RESULT_ERROR;
            /* break; */
          }
          X->VariantTypeCode = LongTypeCode;
          *position = p;
          return RESULT_SUCCESS;
        }
      }
      /* ---------------------------------------------------------------------------- */
      /* if( TRUE ) */
      {
        /* it is a function */
        UserFunctionType *L;
        unsigned char ParameterCount;
        ParamBitsType ParameterTypes;
        VariableType *argv;
        VariableType *argn;

        ParameterCount = 0;
        ParameterTypes = 0;
        argv = var_chain (NULL);        /* RETURN variable */
        argn = NULL;

        if (buff_skip_LparenChar (buffer, &p))
        {
          if (buff_skip_RparenChar (buffer, &p))
          {
            /*  RND() */
          }
          else
          {
            /*  RND( 1, 2, 3 ) */
            do
            {
              ResultType ResultCode;
              VariantType T;

              ResultCode = buff_read_expr (buffer, &p, &T, 1);
              if (ResultCode != RESULT_SUCCESS)
              {
                /* ERROR */
                var_free (argv);        /* free ARGV chain */
                return ResultCode;
              }
              /* add value to ARGV chain      */
              argn = var_chain (argv);
              /* 'argn' is the variable to use */
              if (is_string_type (&T))
              {
                /* STRING */
                var_make (argn, StringTypeCode);
                if ((argn->Value.String =
                     (StringType *) calloc (1, sizeof (StringType))) == NULL)
                {
                  WARN_OUT_OF_MEMORY;
                  return RESULT_ERROR;
                }
                PARAM_LENGTH = T.Length;
                /* PARAM_BUFFER = T.Buffer; */
                if ((PARAM_BUFFER =
                     (char *) calloc (T.Length + 1 /* NulChar */ ,
                                      sizeof (char))) == NULL)
                {
                  WARN_OUT_OF_MEMORY;
                  return RESULT_ERROR;
                }
                bwb_memcpy (PARAM_BUFFER, T.Buffer, T.Length);
                PARAM_BUFFER[PARAM_LENGTH] = NulChar;
                /* add type  to ParameterTypes */
                if (ParameterCount < MAX_FARGS)
                {
                  ParameterTypes |= (1 << ParameterCount);
                }
              }
              else
              {
                /* NUMBER */
                var_make (argn, DoubleTypeCode);
                PARAM_NUMBER = T.Number;
              }
              /* increment ParameterCount */
              if (ParameterCount < 255 /* (...) */ )
              {
                ParameterCount++;
              }
              /* RELEASE_VARIANT( &T ); */
            }
            while (buff_skip_seperator (buffer, &p));


            if (buff_skip_RparenChar (buffer, &p) == FALSE)
            {
              /* ERROR */
              var_free (argv);        /* free ARGV chain */
              WARN_SYNTAX_ERROR;
              return RESULT_ERROR;
            }
          }
        }
        else
        {
          /* RND */
        }

        /* search for exact match to the function parameter signature */
        if (ParameterCount > MAX_FARGS)
        {
          /* FORCE (...) */
          ParameterCount = 255;        /* (...) */
          ParameterTypes = 0;
        }
        /* did we find the correct function above? */
        L = UserFunction_find_exact (name, ParameterCount, ParameterTypes);
        if (L == NULL)
        {
          L = UserFunction_find_exact (name, 255 /* (...) */ , 0);
        }
        if (L != NULL)
        {
          /* USER function */
          if (L->line == NULL)
          {
            var_free (argv);        /* free ARGV chain */
            WARN_INTERNAL_ERROR;
            return RESULT_ERROR;
          }
          /* defaullt the return value */
          var_make (argv, L->ReturnTypeCode);
          bwb_strcpy (argv->name, name);
          if (VAR_IS_STRING (argv))
          {
            RESULT_BUFFER = My->MaxLenBuffer;
            RESULT_LENGTH = 0;
            RESULT_BUFFER[RESULT_LENGTH] = NulChar;
          }
          else
          {
            RESULT_NUMBER = 0;
          }
          /* execute function */
          /* for all USER DEFINED FUNCTIONS: f->UniqueID == line number of DEF FN... */
          switch (L->line->cmdnum)
          {
          case C_DEF:                /* execute a user function   declared using DEF FN   ...(...) = ... */
          case C_FUNCTION:        /* execute a user function   declared using FUNCTION ...(...) */
          case C_SUB:                /* execute a user subroutine declared using SUB      ...(...) */
            IntrinsicFunction_deffn (ParameterCount, argv, L);
            break;
          case C_DEF8LBL:        /* IF ERL > label1 AND ERL < label2 THEN ... */
            if (ParameterCount > 0)
            {
              var_free (argv);        /* free ARGV chain */
              WARN_ILLEGAL_FUNCTION_CALL;
              return RESULT_ERROR;
            }
            /* return the line number associated with the label */
            RESULT_NUMBER = L->line->number;
            break;
          default:
                  /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
            var_free (argv);        /* free ARGV chain */
            WARN_INTERNAL_ERROR;
            return RESULT_ERROR;
            /* break; */
          }
        }
        else
        {
          /* INTRINSIC */
          IntrinsicFunctionType *f;

          f =
            IntrinsicFunction_find_exact (name, ParameterCount,
                                          ParameterTypes);
          if (f == NULL)
          {
            /* NOT FOUND */
            f = IntrinsicFunction_find_exact (name, 255 /* (...) */ , 0);
          }
          if (f == NULL)
          {
            /* NOT FOUND */
            var_free (argv);        /* free ARGV chain */
            WARN_ILLEGAL_FUNCTION_CALL;
            return RESULT_ERROR;
          }
          /* FOUND */
          /* defaullt the return value */
          var_make (argv, f->ReturnTypeCode);
          bwb_strcpy (argv->name, name);
          if (VAR_IS_STRING (argv))
          {
            RESULT_BUFFER = My->MaxLenBuffer;
            RESULT_LENGTH = 0;
            RESULT_BUFFER[RESULT_LENGTH] = NulChar;
          }
          else
          {
            RESULT_NUMBER = 0;
          }
          /* execute function */
          /* for all INTRINSIC FUNCTIONS: f->UniqueID == #define F_... */
          IntrinsicFunction_execute (ParameterCount, argv, f);
        }
        /* return results */
        X->VariantTypeCode = argv->VariableTypeCode;
        if (VAR_IS_STRING (argv))
        {
          if (RESULT_LENGTH > MAXLEN)
          {
            WARN_STRING_TOO_LONG;        /* buff_read_function */
            RESULT_LENGTH = MAXLEN;
          }
          X->Length = RESULT_LENGTH;
          if ((X->Buffer =
               (char *) calloc (X->Length + 1 /* NulChar */ ,
                                sizeof (char))) == NULL)
          {
            WARN_OUT_OF_MEMORY;
            return RESULT_ERROR;
          }
          bwb_memcpy (X->Buffer, RESULT_BUFFER, X->Length);
          X->Buffer[X->Length] = NulChar;
          RESULT_BUFFER = NULL;
        }
        else
        {
          X->Number = RESULT_NUMBER;
        }
        /* free ARGV chain */
        var_free (argv);
        /* OK */
        *position = p;
        return RESULT_SUCCESS;
      }
      /* ---------------------------------------------------------------------------- */
    }
  }
  /* NOT FOUND */
  return RESULT_UNPARSED;
}

#if FALSE                        /* keep line_... */
static int
line_read_function (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_function (line->buffer, &(line->position), X);
}
#endif


static ResultType
buff_read_variable (char *buffer, int *position, VariantType * X)
{
  int p;
  char name[NameLengthMax + 1];
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);


  p = *position;
  if (buff_read_varname (buffer, &p, name))
  {
    VariableType *v;
    int n_params;
    int pp[MAX_DIMS];

    if (buff_peek_LparenChar (buffer, &p))
    {
      /* array */
      if (buff_peek_array_dimensions (buffer, &p, &n_params) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return RESULT_ERROR;
      }
      v = var_find (name, n_params, TRUE);
    }
    else
    {
      /* scalar */
      v = var_find (name, 0, TRUE);
    }
    if (v == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return RESULT_ERROR;
    }
    if (v->dimensions > 0)
    {
      /* array */
      int n;

      if (buff_read_array_dimensions (buffer, &p, &n_params, pp) == FALSE)
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return RESULT_ERROR;
      }
      for (n = 0; n < v->dimensions; n++)
      {
        if (pp[n] < v->LBOUND[n] || pp[n] > v->UBOUND[n])
        {
          WARN_SUBSCRIPT_OUT_OF_RANGE;
          return RESULT_ERROR;
        }
        v->VINDEX[n] = pp[n];
      }
    }
    if (var_get (v, X) == FALSE)
    {
      WARN_TYPE_MISMATCH;
      return RESULT_ERROR;
    }
    *position = p;
    return RESULT_SUCCESS;
  }
  /* NOT FOUND */
  return RESULT_UNPARSED;
}

#if FALSE                        /* keep line_... */
static int
line_read_variable (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_variable (line->buffer, &(line->position), X);
}
#endif
/*
--------------------------------------------------------------------------------------------
                               Precedence Climbing Expression Parser
--------------------------------------------------------------------------------------------
*/

/*
// Read an infix expression containing top-level operators that bind at least
// as tightly as the given precedence.
// Don't consume the first non-digit character after the last number.
// Complain if you can't even find the first number,
// or if there is an operator with no following number.
*/
static ResultType
buff_read_expr (char *buffer, int *position, VariantType * X,
                unsigned char LastPrec)
{
  ResultType ResultCode;
  OperatorType *C;
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);


  p = *position;
  bwb_memset (X, 0, sizeof (VariantType));        /* NOTE */

  ResultCode = buff_read_primary (buffer, &p, X);
  if (ResultCode != RESULT_SUCCESS)
  {
    return ResultCode;
  }
  if (X->VariantTypeCode == NulChar)
  {
    /* we do not know the primary's type */
    WARN_INTERNAL_ERROR;
    return RESULT_ERROR;
  }
  buff_skip_spaces (buffer, &p);        /* keep this */
  while ((C = buff_read_operator (buffer, &p, LastPrec, BINARY)) != NULL)
  {
    VariantType Y;

    ResultCode = buff_read_expr (buffer, &p, &Y, C->NextPrec);
    if (ResultCode != RESULT_SUCCESS)
    {
      /* ERROR */
      if (Y.Buffer != NULL)
      {
        free (Y.Buffer);
        Y.Buffer = NULL;
      }
      return ResultCode;
    }
    ResultCode = C->Eval (X, &Y);
    if (Y.Buffer != NULL)
    {
      free (Y.Buffer);
      Y.Buffer = NULL;
    }
    if (ResultCode != RESULT_SUCCESS)
    {
      /* ERROR */
      return ResultCode;
    }
    /* OK */
  }
  /*
     Normal termination, such as end-of-line, ',', or "THEN".
   */
  *position = p;
  return RESULT_SUCCESS;
}

#if FALSE                        /* keep line_... */
static ResultType
line_read_expr (LineType * line, VariantType * X, unsigned char LastPrec)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_expr (line->buffer, &(line->position), X, LastPrec);
}
#endif
static ResultType
buff_read_primary (char *buffer, int *position, VariantType * X)
{
  ResultType ResultCode;
  OperatorType *C;
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);


  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_is_eol (buffer, &p))
  {
    /* we expected to find something, but there is nothing here */
    WARN_SYNTAX_ERROR;
    return RESULT_ERROR;
  }
  /* there is something to parse */
  if (buff_skip_LparenChar (buffer, &p))
  {
    /* nested expression */
    ResultCode = buff_read_expr (buffer, &p, X, 1);
    if (ResultCode != RESULT_SUCCESS)
    {
      return ResultCode;
    }
    if (buff_skip_RparenChar (buffer, &p) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return RESULT_ERROR;
    }
    *position = p;
    return RESULT_SUCCESS;
  }
  /* not a nested expression */
  C = buff_read_operator (buffer, &p, 1, UNARY);
  if (C != NULL)
  {
    ResultCode = buff_read_expr (buffer, &p, X, C->NextPrec);
    if (ResultCode != RESULT_SUCCESS)
    {
      return ResultCode;
    }
    ResultCode = C->Eval (X, NULL);
    if (ResultCode != RESULT_SUCCESS)
    {
      return ResultCode;
    }
    *position = p;
    return RESULT_SUCCESS;
  }
  /* not an operator */
  ResultCode = buff_read_string_constant (buffer, &p, X);
  if (ResultCode != RESULT_UNPARSED)
  {
    /* either OK or ERROR */
    if (ResultCode == RESULT_SUCCESS)
    {
      *position = p;
    }
    return ResultCode;
  }
  ResultCode = buff_read_hexadecimal_constant (buffer, &p, X, FALSE);
  if (ResultCode != RESULT_UNPARSED)
  {
    /* either OK or ERROR */
    if (ResultCode == RESULT_SUCCESS)
    {
      *position = p;
    }
    return ResultCode;
  }
  ResultCode = buff_read_octal_constant (buffer, &p, X, FALSE);
  if (ResultCode != RESULT_UNPARSED)
  {
    /* either OK or ERROR */
    if (ResultCode == RESULT_SUCCESS)
    {
      *position = p;
    }
    return ResultCode;
  }
  ResultCode = buff_read_internal_constant (buffer, &p, X);
  if (ResultCode != RESULT_UNPARSED)
  {
    /* either OK or ERROR */
    if (ResultCode == RESULT_SUCCESS)
    {
      *position = p;
    }
    return ResultCode;
  }
  ResultCode = buff_read_decimal_constant (buffer, &p, X, FALSE);
  if (ResultCode != RESULT_UNPARSED)
  {
    /* either OK or ERROR */
    if (ResultCode == RESULT_SUCCESS)
    {
      *position = p;
    }
    return ResultCode;
  }
  /* not a constant */
  ResultCode = buff_read_function (buffer, &p, X);
  if (ResultCode != RESULT_UNPARSED)
  {
    /* either OK or ERROR */
    if (ResultCode == RESULT_SUCCESS)
    {
      *position = p;
    }
    return ResultCode;
  }
  /* not a function */
  ResultCode = buff_read_variable (buffer, &p, X);
  /* 
     the variable will be implicitly created unless:
     OPTION EXPLICIT ON, or
     the varname matches an existing command/function/operator.
   */
  if (ResultCode != RESULT_UNPARSED)
  {
    /* either OK or ERROR */
    if (ResultCode == RESULT_SUCCESS)
    {
      *position = p;
    }
    return ResultCode;
  }
  /* not a variable */
  WARN_SYNTAX_ERROR;
  return RESULT_ERROR;
}

#if FALSE                        /* keep line_... */
static ResultType
line_read_primary (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_primary (line->buffer, &(line->position), X);
}
#endif


int
buff_read_expression (char *buffer, int *position, VariantType * X)
{
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);

  p = *position;
  if (buff_read_expr (buffer, &p, X, 1) == RESULT_SUCCESS)
  {
    switch (X->VariantTypeCode)
    {
    case ByteTypeCode:
    case IntegerTypeCode:
    case LongTypeCode:
    case CurrencyTypeCode:
    case SingleTypeCode:
    case DoubleTypeCode:
    case StringTypeCode:
      /* OK */
      break;
    default:
         /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
      RELEASE_VARIANT (X);
      WARN_INTERNAL_ERROR;
      return FALSE;
      /* break; */
    }
    *position = p;
    return TRUE;
  }
  RELEASE_VARIANT (X);                /* NEW */
  return FALSE;
}


int
line_read_expression (LineType * line, VariantType * X)
{
   
  assert (line != NULL);
  assert (X != NULL);
  return buff_read_expression (line->buffer, &(line->position), X);
}

/*
--------------------------------------------------------------------------------------------
                               BASIC commands
--------------------------------------------------------------------------------------------
*/

#if FALSE                        /* keep line_... */
LineType *
bwb_EVAL (LineType * line)
{
  /*
     EVAL 1 + 2 + 3
     EVAL "ABC" & "DEF"
   */
  ResultType ResultCode;
  VariantType x;
  VariantType *X;
   
  assert (line != NULL);


  VX = &x;
  ResultCode = line_read_expression (line, X);
  if (ResultCode != RESULT_SUCCESS)
  {
    return (line);
  }

  switch (X->VariantTypeCode)
  {
  case ByteTypeCode:
  case IntegerTypeCode:
  case LongTypeCode:
  case CurrencyTypeCode:
  case SingleTypeCode:
  case DoubleTypeCode:
    printf (" NUMBER: %g, %c\n", X->Number, X->VariantTypeCode);
    ResetConsoleColumn ();
    break;
  case StringTypeCode:
    printf (" STRING: %s, %c\n", X->Buffer, X->VariantTypeCode);
    ResetConsoleColumn ();
    break;
  default:
      /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
    WARN_INTERNAL_ERROR;
    break;
  }
  RELEASE_VARIANT (X);
  return (line);
}
#endif

LineType *
bwb_OPTION_DISABLE_OPERATOR (LineType * l)
{
  /* OPTION DISABLE OPERATOR name$ */
  int IsFound;
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);

  IsFound = FALSE;
  /* Get OPERATOR */
  {
    char *Value;

    Value = NULL;
    if (line_read_string_expression (l, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (Value == NULL)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    {
      /* Name */
      int i;
      for (i = 0; i < NUM_OPERATORS; i++)
      {
        if (bwb_stricmp (Value, OperatorTable[i].Name) == 0)
        {
          /* FOUND */
          /* DISABLE OPERATOR */
          OperatorTable[i].OptionVersionBitmask &=
            ~My->CurrentVersion->OptionVersionValue;
          IsFound = TRUE;
        }
      }
    }
    free (Value);
    Value = NULL;
  }
  if (IsFound == FALSE)
  {
    /* display warning message */
    fprintf (My->SYSOUT->cfp, "IGNORED: %s\n", l->buffer);
    ResetConsoleColumn ();
  }
  return (l);
}

LineType *
bwb_OPTION_ENABLE_OPERATOR (LineType * l)
{
  /* OPTION ENABLE OPERATOR name$ */
  int IsFound;
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);


  IsFound = FALSE;
  /* Get OPERATOR */
  {
    char *Value;

    Value = NULL;
    if (line_read_string_expression (l, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (Value == NULL)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    {
      /* Name */
      int i;
      for (i = 0; i < NUM_OPERATORS; i++)
      {
        if (bwb_stricmp (Value, OperatorTable[i].Name) == 0)
        {
          /* FOUND */
          /* ENABLE OPERATOR */
          OperatorTable[i].OptionVersionBitmask |=
            My->CurrentVersion->OptionVersionValue;
          IsFound = TRUE;
        }
      }
    }
    free (Value);
    Value = NULL;
  }
  if (IsFound == FALSE)
  {
    /* display warning message */
    fprintf (My->SYSOUT->cfp, "IGNORED: %s\n", l->buffer);
    ResetConsoleColumn ();
  }
  return (l);
}

void
DumpOneOperatorSyntax (FILE * file, int IsXref, int n)
{
   
  assert (file != NULL);

  if (n < 0 || n >= NUM_OPERATORS)
  {
    return;
  }
  /* NAME */
  {
    FixDescription (file, "     SYNTAX: ", OperatorTable[n].Syntax);
  }
  /* DESCRIPTION */
  {

    FixDescription (file, "DESCRIPTION: ", OperatorTable[n].Description);
  }
  /* PRECEDENCE */
  {
    fprintf (file, " PRECEDENCE: %d\n", OperatorTable[n].ThisPrec);
  }
  /* COMPATIBILITY */
  if (IsXref)
  {
    int i;
    fprintf (file, "   VERSIONS:\n");
    for (i = 0; i < NUM_VERSIONS; i++)
    {
      char X;
      if (OperatorTable[n].OptionVersionBitmask & bwb_vertable[i].
          OptionVersionValue)
      {
        /* SUPPORTED */
        X = 'X';
      }
      else
      {
        /* NOT SUPPORTED */
        X = '_';
      }
      fprintf (file, "             [%c] %s\n", X, bwb_vertable[i].Name);
    }
  }

  fflush (file);
}

void
DumpAllOperatorSyntax (FILE * file, int IsXref,
                       OptionVersionType OptionVersionValue)
{
  /* for the C maintainer */
  int n;
   
  assert (file != NULL);

  fprintf (file,
           "============================================================\n");
  fprintf (file,
           "                    OPERATORS                               \n");
  fprintf (file,
           "============================================================\n");
  fprintf (file, "\n");
  fprintf (file, "\n");
  SortAllOperatorsForManual ();
  for (n = 0; n < NUM_OPERATORS; n++)
  {
    if (OperatorTable[n].OptionVersionBitmask & OptionVersionValue)
    {
      fprintf (file,
               "------------------------------------------------------------\n");
      DumpOneOperatorSyntax (file, IsXref, n);
    }
  }
  SortAllOperators ();
  fprintf (file,
           "------------------------------------------------------------\n");

  fprintf (file, "\n");
  fprintf (file, "\n");
  fflush (file);
}

/* EOF */
