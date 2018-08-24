/****************************************************************
  
        bwb_fnc.c       Interpretation Routines
                        for Predefined Functions
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


#ifndef   RAND_MAX
#define   RAND_MAX     32767
#endif /* RAND_MAX */

#ifndef   PI
#define   PI           3.14159265358979323846
#endif /* PI */

#define FromDegreesToRadians(  X ) ( X * PI / 180.0 )
#define FromRadiansToDegrees(  X ) ( X * 180.0 / PI )

#define FromGradiansToRadians( X ) ( X * PI / 200.0 )
#define FromRadiansToGradians( X ) ( X * 200.0 / PI )


static time_t t;
static struct tm *lt;


/*  ORD() Table 1    */

 /* ACRONYM */
typedef struct
{
  const int Value;
  const char *Name;
} Acronym;

#define NUM_ACRONYMS (34)

Acronym AcronymTable[NUM_ACRONYMS] = {
  {0, "NUL"},
  {1, "SOH"},
  {2, "STX"},
  {3, "ETX"},
  {4, "EOT"},
  {5, "ENQ"},
  {6, "ACK"},
  {7, "BEL"},
  {8, "BS"},
  {9, "HT"},
  {10, "LF"},
  {11, "VT"},
  {12, "FF"},
  {13, "CR"},
  {14, "SO"},
  {15, "SI"},
  {16, "DLE"},
  {17, "DC1"},
  {18, "DC2"},
  {19, "DC3"},
  {20, "DC4"},
  {21, "NAK"},
  {22, "SYN"},
  {23, "ETB"},
  {24, "CAN"},
  {25, "EM"},
  {26, "SUB"},
  {27, "ESC"},
  {28, "FS"},
  {29, "GS"},
  {30, "RS"},
  {31, "US"},
  {32, "SP"},
  {127, "DEL"}
};

/* ... ORD() */




extern VariableType *
IntrinsicFunction_execute (int argc, VariableType * argv,
                           IntrinsicFunctionType * f)
{
  /* this is the generic handler for all intrinsic BASIC functions */
  /* Follow the BASIC naming conventions, so the code is easier to read and maintain */

  /* assign reasonable default values */
  VariableType *argn;
  /* Follow the BASIC naming conventions, so the code is easier to maintain */
  char *S;                        /* S$ - STRING functions */
  size_t s;                        /* LEN( S$ ) */
  DoubleType N;                        /* N  -  NUMBER functions */
  char *A;                        /* A$ - 1st STRING parameter */
  size_t a;                        /* LEN( A$ ) */
  char *B;                        /* B$ - 2nd STRING parameter */
  size_t b;                        /* LEN( B$ ) */
#if FALSE                        /* keep third parameter */
  char *C;                        /* C$ - 3rd STRING parameter */
  size_t c;                        /* LEN( C$ ) */
#endif
  DoubleType X;                        /* X  - 1st NUMBER parameter */
  IntegerType x;                /* CINT( X ) */
  DoubleType Y;                        /* Y  - 2nd NUMBER parameter */
  IntegerType y;                /* CINT( Y ) */
#if FALSE                        /* keep third parameter */
  DoubleType Z;                        /* Z  - 3rd NUMBER parameter */
  IntegerType z;                /* CINT( Z ) */
#endif
   
  assert (argc >= 0);
  assert (argv != NULL);
  assert (f != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  S = NULL;
  s = 0;
  N = 0;
  A = NULL;
  a = 0;
  B = NULL;
  b = 0;
#if FALSE                        /* keep third parameter */
  C = NULL;
  c = 0;
#endif
  X = 0;
  x = 0;
  Y = 0;
  y = 0;
#if FALSE                        /* keep third parameter */
  Z = 0;
  z = 0;
#endif

  if (f == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (argc < 0)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  /* the RETURN variable is the first variable in the 'argv' vaariable chain */
  if (argv == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (VAR_IS_STRING (argv))
  {
    if (argv->Value.String == NULL)
    {
      WARN_INTERNAL_ERROR;
      return NULL;
    }
    if (RESULT_BUFFER == NULL)
    {
      WARN_INTERNAL_ERROR;
      return NULL;
    }
    RESULT_LENGTH = 0;
    RESULT_BUFFER[RESULT_LENGTH] = NulChar;
  }
  else
  {
    if (argv->Value.Number == NULL)
    {
      WARN_INTERNAL_ERROR;
      return NULL;
    }
    RESULT_NUMBER = 0;
  }
  argn = argv;
  /* don't make a bad situation worse */
  if (My->IsErrorPending /* Keep This */ )
  {
    /* An unrecognized NON-FATAL ERROR is pending.  Just return a sane value. */
    /* LET N = LOG(SQR(X)) ' X = -1 */
    return argv;
  }
  /* so the following code is easier to read and maintain */
  {
    /* assign actual values */
    if (f->ReturnTypeCode == StringTypeCode)
    {
      S = RESULT_BUFFER;
      s = RESULT_LENGTH;
    }
    else
    {
      N = RESULT_NUMBER;
    }
    if (f->ParameterCount == 255 /* (...) */ )
    {
      /* ... VARIANT number of parameters */
    }
    else
    {
      int i;
      int StrCount;                /* count of STRING parameters - NEVER > 3 */
      int NumCount;                /* count of NUMBER parameters - NEVER > 3 */
      ParamTestType ParameterTests;

      StrCount = 0;
      NumCount = 0;
      ParameterTests = f->ParameterTests;
      for (i = 0; i < argc && i < MAX_TESTS && My->IsErrorPending == FALSE;
           i++)
      {
        argn = argn->next;
        if (argn == NULL)
        {
          WARN_INTERNAL_ERROR;
          return NULL;
        }
        if (VAR_IS_STRING (argn))
        {
          if (argn->Value.String == NULL)
          {
            WARN_INTERNAL_ERROR;
            return NULL;
          }
          StrCount++;
          switch (StrCount)
          {
          case 1:
            /* 1st STRING parameter = A$ */
            A = PARAM_BUFFER;
            a = PARAM_LENGTH;
            if (StringLengthCheck (ParameterTests, a))
            {
              WARN_ILLEGAL_FUNCTION_CALL;
            }
            else
            {
              A[a] = NulChar;
            }
            break;
          case 2:
            /* 2nd STRING parameter = B$ */
            B = PARAM_BUFFER;
            b = PARAM_LENGTH;
            if (StringLengthCheck (ParameterTests, b))
            {
              WARN_ILLEGAL_FUNCTION_CALL;
            }
            else
            {
              B[b] = NulChar;
            }
            break;
#if FALSE                        /* keep third parameter */
          case 3:
            /* 3rd STRING parameter = C$ */
            /* not currently used */
            C = PARAM_BUFFER;
            c = PARAM_LENGTH;
            if (StringLengthCheck (ParameterTests, c))
            {
              WARN_ILLEGAL_FUNCTION_CALL;
            }
            else
            {
              C[c] = NulChar;
            }
            break;
#endif
          default:
            /* Nth STRING parameter = ERROR */
            WARN_ILLEGAL_FUNCTION_CALL;
            break;
          }
        }
        else
        {
          if (argn->Value.Number == NULL)
          {
            WARN_INTERNAL_ERROR;
            return NULL;
          }
          NumCount++;
          switch (NumCount)
          {
          case 1:
            /* 1st NUMBER parameter = X */
            X = PARAM_NUMBER;
            if (NumberValueCheck (ParameterTests, X))
            {
              WARN_ILLEGAL_FUNCTION_CALL;
            }
            else
            {
              DoubleType R;
              R = bwb_rint (X);
              if (R < INT_MIN || R > INT_MAX)
              {
                /* certainly not a
                 * classic BASIC
                 * integer */
              }
              else
              {
                /* Many classic BASIC
                 * intrinsic
                 * functions use the
                 * rounded integer
                 * value. */
                x = (int) R;
              }
            }
            break;
          case 2:
            /* 2nd NUMBER parameter = Y */
            Y = PARAM_NUMBER;
            if (NumberValueCheck (ParameterTests, Y))
            {
              WARN_ILLEGAL_FUNCTION_CALL;
            }
            else
            {
              DoubleType R;
              R = bwb_rint (Y);
              if (R < INT_MIN || R > INT_MAX)
              {
                /* certainly not a
                 * classic BASIC
                 * integer */
              }
              else
              {
                /* Many classic BASIC
                 * intrinsic
                 * functions use the
                 * rounded integer
                 * value. */
                y = (int) R;
              }
            }
            break;
#if FALSE                        /* keep third parameter */
          case 3:
            /* 3rd NUMBER parameter = Z */
            /* not currently used */
            Z = PARAM_NUMBER;
            if (NumberValueCheck (ParameterTests, Z))
            {
              WARN_ILLEGAL_FUNCTION_CALL;
            }
            else
            {
              DoubleType R;
              R = bwb_rint (Z);
              if (R < INT_MIN || R > INT_MAX)
              {
                /* certainly not a
                 * classic BASIC
                 * integer */
              }
              else
              {
                /* Many classic BASIC
                 * intrinsic
                 * functions use the
                 * rounded integer
                 * value. */
                z = (int) R;
              }
            }
            break;
#endif
          default:
            /* Nth NUMBER parameter = ERROR */
            WARN_ILLEGAL_FUNCTION_CALL;
            break;
          }
        }
        ParameterTests = ParameterTests >> 4;
      }
    }
  }
  if (My->IsErrorPending /* Keep This */ )
  {
    /* An unrecognized NON-FATAL ERROR is pending.  Just return a sane value. */
    /* LET N = LOG(SQR(X)) ' X = -1 */
    return argv;
  }
  /* 
   **
   ** all parameters have been checked and are OK
   ** execute the intrinsic function
   **
   */
  switch (f->FunctionID)
  {
    /* 
     ** 
     ** ALL paramters have  been checked 
     ** for TYPE MISMATCH and INVALID RANGE.
     ** ONLY A HANDFUL OF ERRORS CAN OCCUR
     **
     */
  case 0:
    {
      /* INTERNAL ERROR */
      WARN_INTERNAL_ERROR;
    }
    break;
  case F_ARGC_N:
    /* N = ARGC */
    {
      /* determine number of parameters to the current USER DEFINED FUNCTION */
      int n;
      n = 0;
      if (My->StackHead != NULL)
      {
        int Loop;
        StackType *StackItem;
        Loop = TRUE;
        for (StackItem = My->StackHead; StackItem != NULL && Loop == TRUE;
             StackItem = StackItem->next)
        {
          if (StackItem->LoopTopLine != NULL)
          {
            switch (StackItem->LoopTopLine->cmdnum)
            {
            case C_FUNCTION:
            case C_SUB:
              /* we have checked all the way to a FUNCTION or SUB boundary */
              /* FOUND */
              {
                VariableType *v;

                for (v = StackItem->local_variable; v != NULL && Loop == TRUE;
                     v = v->next)
                {
                  n++;
                }
              }
              Loop = FALSE;
              break;
            }
          }
        }
      }
      n--;                        /* FUNCTION or SUB name */
      N = n;
    }
    break;
  case F_ARGT4_X_S:
    /* S$ = ARGT$( X ) */
    {
      /* determine parameter type to the current USER DEFINED FUNCTION */
      int Found;
      int n;
      Found = FALSE;
      n = 0;
      s = 0;
      if (x < 1)
      {
        /* bad param number  */
      }
      else if (My->StackHead != NULL)
      {
        int Loop;
        StackType *StackItem;
        Loop = TRUE;
        for (StackItem = My->StackHead; StackItem != NULL && Loop == TRUE;
             StackItem = StackItem->next)
        {
          if (StackItem->LoopTopLine != NULL)
          {
            switch (StackItem->LoopTopLine->cmdnum)
            {
            case C_FUNCTION:
            case C_SUB:
              /* we have checked all the way to a FUNCTION or SUB boundary */
              /* FOUND */
              {
                VariableType *v;


                for (v = StackItem->local_variable; v != NULL && Loop == TRUE;
                     v = v->next)
                {
                  if (n == x)
                  {
                    char Char;
                    Char = TypeCode_to_Char (v->VariableTypeCode);
                    if (Char)
                    {
                      S[0] = Char;
                      s = 1;
                      Found = TRUE;
                    }
                    Loop = FALSE;
                  }
                  n++;
                }
              }
              Loop = FALSE;
              break;
            }
          }
        }
      }
      if (Found == FALSE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
    }
    break;

  case F_ARGV4_X_S:
    /* S$ = ARGV$( X ) */
    {
      /* determine parameter value to the current
       * USER DEFINED FUNCTION */
      int Found;
      int n;
      Found = FALSE;
      n = 0;
      if (x < 1)
      {
        /* bad param number  */
      }
      else if (My->StackHead != NULL)
      {
        int Loop;
        StackType *StackItem;
        Loop = TRUE;
        for (StackItem = My->StackHead; StackItem != NULL && Loop == TRUE;
             StackItem = StackItem->next)
        {
          if (StackItem->LoopTopLine != NULL)
          {
            switch (StackItem->LoopTopLine->cmdnum)
            {
            case C_FUNCTION:
            case C_SUB:
              /* we have checked all the way to a FUNCTION or SUB boundary */
              /* FOUND */
              {
                VariableType *v;


                for (v = StackItem->local_variable; v != NULL && Loop == TRUE;
                     v = v->next)
                {
                  if (n == x)
                  {
                    if (VAR_IS_STRING (v))
                    {
                      s = v->Value.String->length;
                      bwb_memcpy (S, v->Value.String->sbuffer, s);
                      Found = TRUE;
                    }
                    else
                    {
                    }
                    Loop = FALSE;
                  }
                  n++;
                }
              }
              Loop = FALSE;
              break;
            }
          }
        }
      }
      if (Found == FALSE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
    }
    break;

  case F_ARGV_X_N:
    /* S$ = ARGV( X ) */
    {
      /* determine parameter value to the current USER DEFINED FUNCTION */
      int Found;
      int n;
      Found = FALSE;
      n = 0;
      if (x < 1)
      {
        /* bad param number  */
      }
      else if (My->StackHead != NULL)
      {
        int Loop;
        StackType *StackItem;
        Loop = TRUE;
        for (StackItem = My->StackHead; StackItem != NULL && Loop == TRUE;
             StackItem = StackItem->next)
        {
          if (StackItem->LoopTopLine != NULL)
          {
            switch (StackItem->LoopTopLine->cmdnum)
            {
            case C_FUNCTION:
            case C_SUB:
              /* we have checked all the way to a FUNCTION or SUB boundary */
              /* FOUND */
              {
                VariableType *v;


                for (v = StackItem->local_variable; v != NULL && Loop == TRUE;
                     v = v->next)
                {
                  if (n == x)
                  {
                    if (VAR_IS_STRING (v))
                    {
                    }
                    else
                    {
                      N = *v->Value.Number;
                      Found = TRUE;
                    }
                    Loop = FALSE;
                  }
                  n++;
                }
              }
              Loop = FALSE;
              break;
            }
          }
        }
      }
      if (Found == FALSE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
    }
    break;
  case F_BASE_N:
    /* N = BASE */
    {
      /* PNONE */
      N = My->CurrentVersion->OptionBaseInteger;        /* implicit lower bound */
    }
    break;
  case F_RESIDUE_N:
    /* N = RESIDUE */
    {
      /* PNONE */
      N = My->RESIDUE;                /* Residue of the last integer divide */
    }
  case F_DIGITS_X_N:
    /* N = DIGITS( X ) */
    {
      /* P1BYT */
      if (x == 0)
      {
        /* default */
        x = SIGNIFICANT_DIGITS;
      }
      if (x < MINIMUM_DIGITS || x > MAXIMUM_DIGITS)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        My->OptionDigitsInteger = x;
      }
    }
    break;
  case F_SCALE_X_N:
  case F_PRECISION_X_N:
    /* N = SCALE( X ) */
    /* N = PRECISION( X ) */
    {
      /* P1BYT */
      if (x < MINIMUM_SCALE || x > MAXIMUM_SCALE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        My->OptionScaleInteger = x;
      }
    }
    break;
  case F_DIGITS_X_Y_N:
    /* N = DIGITS( X, Y ) */
    {
      /* P1BYT | P2BYT */
      if (x == 0)
      {
        /* default */
        x = SIGNIFICANT_DIGITS;
      }
      if (x < MINIMUM_DIGITS || x > MAXIMUM_DIGITS)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else if (y < MINIMUM_SCALE || y > MAXIMUM_SCALE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        My->OptionDigitsInteger = x;
        My->OptionScaleInteger = y;
      }
    }
    break;
  case F_ASC_A_N:
  case F_ASCII_A_N:
  case F_CODE_A_N:
    /* N = ASC( A$ ) */
    /* N = ASCII( A$ ) */
    /* N = CODE( A$ ) */
    {
      /* P1BYT */
      N = A[0];
    }
    break;
  case F_ASC_A_X_N:
    /* N = ASC( A$, X ) */
    {
      /* P1BYT|P2POS */
      x--;                        /* BASIC -> C */
      if (x < a)
      {
        N = A[x];
      }
      else
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
    }
    break;
  case F_CDBL_X_N:
    /* N = CDBL( X ) */
    {
      /* P1DBL */
      N = X;
    }
    break;
  case F_CSNG_X_N:
    /* N = CSNG( X ) */
    {
      /* P1FLT */
      N = X;
    }
    break;
  case F_CCUR_X_N:
    /* N = CCUR( X ) */
    {
      /* P1CUR */
      N = bwb_rint (X);
    }
    break;
  case F_CLNG_X_N:
    /* N = CLNG( X ) */
    {
      /* P1LNG */
      N = bwb_rint (X);
    }
    break;
  case F_CINT_X_N:
    /* N = CINT( X ) */
    {
      /* P1INT */
      N = bwb_rint (X);
    }
    break;
  case F_MKD4_X_S:
    /* S$ = MKD$( X ) */
    {
      /* P1DBL */
      DoubleType x;
      x = (DoubleType) X;
      s = sizeof (DoubleType);
      bwb_memcpy (S, &x, s);
    }
    break;
  case F_MKS4_X_S:
    /* S$ = MKS$( X ) */
    {
      /* P1FLT */
      SingleType x;
      x = (SingleType) X;
      s = sizeof (SingleType);
      bwb_memcpy (S, &x, s);
    }
    break;
  case F_MKI4_X_S:
    /* S$ = MKI$( X ) */
    {
      /* P1INT */
      IntegerType x;
      x = (IntegerType) bwb_rint (X);
      s = sizeof (IntegerType);
      bwb_memcpy (S, &x, s);
    }
    break;
  case F_MKL4_X_S:
    /* S$ = MKL$( X ) */
    {
      /* P1LNG */
      LongType x;
      x = (LongType) bwb_rint (X);
      s = sizeof (LongType);
      bwb_memcpy (S, &x, s);
    }
    break;
  case F_MKC4_X_S:
    /* S$ = MKC$( X ) */
    {
      /* P1CUR */
      CurrencyType x;
      x = (CurrencyType) bwb_rint (X);
      s = sizeof (CurrencyType);
      bwb_memcpy (S, &x, s);
    }
    break;
  case F_CVD_A_N:
    /* N = CVD( A$ ) */
    {
      /* P1DBL */
      DoubleType n;
      a = sizeof (DoubleType);
      bwb_memcpy (&n, A, a);
      N = n;
    }
    break;
  case F_CVS_A_N:
    /* N = CVS( X$ ) */
    {
      /* P1FLT */
      SingleType n;
      a = sizeof (SingleType);
      bwb_memcpy (&n, A, a);
      N = n;
    }
    break;
  case F_CVI_A_N:
    /* N = CVI( X$ ) */
    {
      /* P1INT */
      IntegerType n;
      a = sizeof (IntegerType);
      bwb_memcpy (&n, A, a);
      N = n;
    }
    break;
  case F_CVL_A_N:
    /* N = CVL( X$ ) */
    {
      /* P1LNG */
      LongType n;
      a = sizeof (LongType);
      bwb_memcpy (&n, A, a);
      N = n;
    }
    break;
  case F_CVC_A_N:
    /* N = CVC( X$ ) */
    {
      /* P1CUR */
      CurrencyType n;
      a = sizeof (CurrencyType);
      bwb_memcpy (&n, A, a);
      N = n;
    }
    break;
  case F_ENVIRON4_A_S:
    /* S$ = ENVIRON$( A$ ) */
    {
      /* P1BYT */
      char *CharPointer;

      CharPointer = getenv (A);
      if (CharPointer == NULL)
      {
        /* empty string */
      }
      else
      {
        s = bwb_strlen (CharPointer);
        if (s > MAXLEN)
        {
          WARN_STRING_TOO_LONG;        /* F_ENVIRON4_A_S */
          s = MAXLEN;
        }
        if (s == 0)
        {
          /* empty string */
        }
        else
        {
          bwb_memcpy (S, CharPointer, s);
        }
      }
    }
    break;
  case F_ENVIRON_A_N:
    /* ENVIRON A$ */
    {
      /* P1BYT */

      char *CharPointer;

      CharPointer = bwb_strchr (A, '=');
      if (CharPointer == NULL)
      {
        /* missing required '=' */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        if (putenv (A) == -1)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          /* OK */
          N = 0;
        }
      }
    }
    break;

  case F_OPEN_A_X_B_Y_N:
    /* OPEN "I"|"O"|"R"|"A", [#]n, filename [,rlen] */
    {
      /* P1STR|P2NUM|P3STR|P4NUM */
      /* P1BYT|P2INT|P3BYT|P4INT */

      while (*A == ' ')
      {
        A++;                        /* LTRIM$ */
      }
      bwb_file_open (*A, x, B, y);
    }
    break;
  case F_OPEN_A_X_B_N:
    /* default  LEN is 128 for RANDOM, 0 for all others */
    /* OPEN "I"|"O"|"R"|"A", [#]n, filename [,rlen] */
    {
      /* P1STR|P2NUM|P3STR|P4NUM */
      /* P1BYT|P2INT|P3BYT|P4INT */
      y = 0;
      while (*A == ' ')
      {
        A++;                        /* LTRIM$ */
      }
      if (bwb_toupper (*A) == 'R')
      {
        /* default RANDOM record size */
        y = 128;
      }
      bwb_file_open (*A, x, B, y);
    }
    break;
  case F_LOC_X_N:
    /* N = LOC( X ) */
    {
      /* P1INT */
      if (x <= 0)
      {
        /* Printer and Console */
        N = 0;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          N = 0;
        }
        else if (F == My->SYSOUT)
        {
          N = 0;
        }
        else if (F == My->SYSPRN)
        {
          N = 0;
        }
        else
        {
          FILE *fp;
          fp = F->cfp;
          N = ftell (fp);
          if (My->CurrentVersion->OptionVersionValue & (G65 | G67 | G74))
          {
            /* byte position, regardless of 'mode' */
          }
          else if (F->DevMode == DEVMODE_RANDOM)
          {
            /* record number */
            if (F->width == 0)
            {
              /* byte position */
            }
            else
            {
              N /= F->width;
            }
          }
          else if (F->DevMode == DEVMODE_BINARY)
          {
            /* byte position */
          }
          else
          {
            /* byte positiion / 128 */
            N /= 128;
          }
          N = floor (N);
          N++;                        /* C to BASIC */
        }
      }
    }
    break;
  case F_SEEK_X_N:
    /* N = SEEK( X ) */
    {
      /* P1INT */
      if (x <= 0)
      {
        /* Printer and Console */
        N = 0;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          N = 0;
        }
        else if (F == My->SYSOUT)
        {
          N = 0;
        }
        else if (F == My->SYSPRN)
        {
          N = 0;
        }
        else
        {
          FILE *fp;
          fp = F->cfp;
          N = ftell (fp);
          if (F->DevMode == DEVMODE_RANDOM)
          {
            /* record number */
            if (F->width > 0)
            {
              N /= F->width;
            }
          }
          else
          {
            /* byte positiion */
          }
          N = floor (N);
          N++;                        /* C to BASIC */
        }
      }
    }
    break;
  case F_SEEK_X_Y_N:
    /* SEEK X, Y */
    {
      /* P1INT|P2INT */
      if (x <= 0)
      {
        /* Printer and Console */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSOUT)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSPRN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (y < 1)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          long offset;
          offset = y;
          offset--;                /* BASIC to C */
          if (F->DevMode == DEVMODE_RANDOM)
          {
            if (F->width > 0)
            {
              offset *= F->width;
            }
          }
          if (fseek (F->cfp, offset, SEEK_SET) != 0)
          {
            WARN_ILLEGAL_FUNCTION_CALL;
          }
          else
          {
            /* OK */
            N = 0;
          }
        }
      }
    }
    break;
  case F_LOF_X_N:
    /* N = LOF( X ) */
    {
      /* P1INT */
      if (x <= 0)
      {
        /* Printer and Console */
        N = 0;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          N = 0;
        }
        else if (F == My->SYSOUT)
        {
          N = 0;
        }
        else if (F == My->SYSPRN)
        {
          N = 0;
        }
        else
        {
          /* file size in bytes */
          FILE *fp;
          long current;
          long total;
          fp = F->cfp;
          current = ftell (fp);
          fseek (fp, 0, SEEK_END);
          total = ftell (fp);
          if (total == current)
          {
            /* EOF */
          }
          else
          {
            fseek (fp, current, SEEK_SET);
          }
          N = total;
        }
      }
    }
    break;
  case F_EOF_X_N:
    /* N = EOF( X ) */
    {
      /* P1INT */
      if (x <= 0)
      {
        /* Printer and Console */
        N = 0;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          N = 0;
        }
        else if (F == My->SYSOUT)
        {
          N = 0;
        }
        else if (F == My->SYSPRN)
        {
          N = 0;
        }
        else
        {
          /* are we at the end? */
          N = bwb_is_eof (F->cfp);
        }
      }
    }
    break;
  case F_FILEATTR_X_Y_N:
    /* N = FILEATTR( X, Y ) */
    {
      /* P1INT|P2INT */

      if (x <= 0)
      {
        /* Printer and Console */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else if (y == 1)
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          /* normal CLOSED file */
          N = 0;
        }
        else
        {
          /* normal OPEN file */
          N = F->DevMode;
        }
      }
      else if (y == 2)
      {
        N = 0;
      }
      else
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
    }
    break;
  case F_CLOSE_X_N:
    /* CLOSE X */
    {
      /* P1INT */
      if (x <= 0)
      {
        /* Printer and Console */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSOUT)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSPRN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          field_close_file (F);
          file_clear (F);
          N = 0;
        }
      }
    }
    break;
  case F_RESET_N:
  case F_CLOSE_N:
    /* RESET */
    /* CLOSE */
    {
      /* PNONE */
      FileType *F;

      for (F = My->FileHead; F != NULL; F = F->next)
      {
        field_close_file (F);
        file_clear (F);
      }
    }
    break;
  case F_FREEFILE_N:
    /* N = FREEFILE */
    {
      /* PNONE */
      FileType *F;

      x = 0;
      y = 0;
      for (F = My->FileHead; F != NULL; F = F->next)
      {
        if (F->DevMode != DEVMODE_CLOSED)
        {
          if (F->FileNumber > x)
          {
            x = F->FileNumber;
          }
          y++;
        }
      }
      /* 'x' is the highest FileNumber that is currently open */
      /* 'y' is the number of files that are currently open */
      x++;
      if (y >= MAXDEV)
      {
        /* no more slots available */
        x = 0;
      }
      N = x;
    }
    break;
  case F_GET_X_Y_N:
    /* GET X, Y */
    {
      /* P1INT|P2INT */
      if (x <= 0)
      {
        /* Printer and Console */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSOUT)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSPRN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F->DevMode != DEVMODE_RANDOM)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (y < 1)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          long offset;
          offset = y;
          offset--;                /* BASIC to C */
          if (F->DevMode == DEVMODE_RANDOM)
          {
            if (F->width > 0)
            {
              offset *= F->width;
            }
          }
          if (fseek (F->cfp, offset, SEEK_SET) != 0)
          {
            WARN_ILLEGAL_FUNCTION_CALL;
          }
          else
          {
            int i;
            for (i = 0; i < F->width; i++)
            {
              F->buffer[i] = fgetc (F->cfp);
            }
            field_get (F);
            N = 0;
          }
        }
      }
    }
    break;
  case F_GET_X_N:
    if (My->CurrentVersion->OptionVersionValue & (D73))
    {
      /* GET( X ) == ASC(INKEY$), X is ignored */
      /* P1ANY */
      int c;

      c = fgetc (My->SYSIN->cfp);
      N = c;
    }
    else
    {
      /* GET X */
      /* P1INT */
      if (x <= 0)
      {
        /* Printer and Console */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSOUT)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSPRN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F->DevMode != DEVMODE_RANDOM)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          {
            int i;
            for (i = 0; i < F->width; i++)
            {
              F->buffer[i] = fgetc (F->cfp);
            }
            field_get (F);
            N = 0;
          }
        }
      }
    }
    break;
  case F_PUT_X_Y_N:
    /* PUT X, Y */
    {
      /* P1INT|P2INT */
      if (x <= 0)
      {
        /* Printer and Console */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSOUT)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSPRN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F->DevMode != DEVMODE_RANDOM)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (y < 1)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          long offset;
          offset = y;
          offset--;                /* BASIC to C */
          if (F->DevMode == DEVMODE_RANDOM)
          {
            if (F->width > 0)
            {
              offset *= F->width;
            }
          }
          if (fseek (F->cfp, offset, SEEK_SET) != 0)
          {
            WARN_ILLEGAL_FUNCTION_CALL;
          }
          else
          {
            int i;
            field_put (F);
            for (i = 0; i < F->width; i++)
            {
              fputc (F->buffer[i], F->cfp);
              F->buffer[i] = ' ';        /* flush  */
            }
            N = 0;
          }
        }
      }
    }
    break;
  case F_PUT_X_N:
    if (My->CurrentVersion->OptionVersionValue & (D73))
    {
      /* PUT( X ) == PRINT CHR$(X); */
      /* P1BYT */
      fputc (x, My->SYSOUT->cfp);
      N = x;
    }
    else
    {
      /* PUT X */
      /* P1INT */
      if (x <= 0)
      {
        /* Printer and Console */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSIN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSOUT)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F == My->SYSPRN)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else if (F->DevMode != DEVMODE_RANDOM)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          {
            int i;
            field_put (F);
            for (i = 0; i < F->width; i++)
            {
              fputc (F->buffer[i], F->cfp);
              F->buffer[i] = ' ';        /* flush  */
            }
            N = 0;
          }
        }
      }
    }
    break;
  case F_WIDTH_X_N:
    /* WIDTH X */
    {
      /* P1BYT */
      /* console is #0 */
      My->SYSIN->width = x;
      My->SYSIN->col = 1;
      My->SYSOUT->width = x;
      My->SYSOUT->col = 1;
      N = 0;
    }
    break;
  case F_WIDTH_X_Y_N:
    /* WIDTH X, Y */
    {
      /*  WIDTH #file, cols */
      /* P1INT|PB2YT */
      if (x == 0)
      {
        My->SYSIN->width = y;
        My->SYSOUT->width = y;
        N = 0;
      }
      else if (x < 0)
      {
        My->SYSPRN->width = y;
        N = 0;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          /* WIDTH rows, cols */
          My->SCREEN_ROWS = x;
          My->SYSIN->width = y;
          My->SYSIN->col = 1;
          My->SYSOUT->width = y;
          My->SYSOUT->col = 1;
          N = 0;
        }
        else if (F->DevMode == DEVMODE_RANDOM)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          /* WIDTH # file, cols */
          F->width = y;
          F->col = 1;
          N = 0;
        }
      }
    }
    break;
  case F_INSTR_X_A_B_N:
  case F_INSTR_A_B_X_N:
    /* N = INSTR( X, A$, B$ ) */
    /* N = INSTR( A$, B$, X ) */
    {
      /* P1POS */
      if (a == 0)
      {
        /* empty searched */
      }
      else if (b == 0)
      {
        /* empty pattern */
      }
      else if (b > a)
      {
        /* pattern is longer than searched */
      }
      else
      {
        /* search */
        int i;
        int n;
        n = a - b;                /* last valid search position */
        n++;

        x--;                        /* BASIC to C */
        A += x;                        /* advance to the start
                                 * position */
        for (i = x; i < n; i++)
        {
          if (bwb_memcmp (A, B, b) == 0)
          {
            /* FOU ND */
            i++;                /* C to BASIC */
            N = i;
            i = n;                /* exit for */
          }
          A++;
        }
      }
    }
    break;
  case F_INSTR_A_B_N:
  case F_INDEX_A_B_N:
    /* N = INSTR( A$, B$ ) */
    /* N = INDEX( A$, B$ ) */
    {
      if (a == 0)
      {
        /* empty searched */
      }
      else if (b == 0)
      {
        /* empty pattern */
      }
      else if (b > a)
      {
        /* pattern is longer than searched */
      }
      else
      {
        /* search */
        int i;
        int n;
        n = a - b;                /* last valid search
                                 * position */
        n++;
        /* search */
        for (i = 0; i < n; i++)
        {
          if (bwb_memcmp (A, B, b) == 0)
          {
            /* FOU ND */
            i++;                /* C to BASIC */
            N = i;
            i = n;                /* exit for */
          }
          A++;
        }
      }
    }
    break;
  case F_SPACE4_X_S:
  case F_SPACE_X_S:
  case F_SPA_X_S:
    /* S$ = SPACE$( X ) */
    /* S$ = SPACE( X ) */
    /* S$ = SPA( X ) */
    {
      /* P1LEN */
      if (x == 0)
      {
        /* no copies */
      }
      else
      {
        bwb_memset (S, (char) ' ', x);
        s = x;
      }
    }
    break;
  case F_STRING4_X_Y_S:
  case F_STRING_X_Y_S:
  case F_STR_X_Y_S:
    /* S$ = STRING$( X, Y ) */
    /* S$ = STRING( X, Y ) */
    /* S$ = STR( X, Y ) */
    {
      /* P1LEN|P2BYT */
      if (x == 0)
      {
        /* no copies */
      }
      else
      {
        bwb_memset (S, (char) y, x);
        s = x;
      }
    }
    break;
  case F_STRING4_X_A_S:
    /* S$ = STRING$( X, A$ ) */
    {
      /* P1LEN|P2BYT */
      if (x == 0)
      {
        /* no copies */
      }
      else
      {
        bwb_memset (S, (char) A[0], x);
        s = x;
      }
    }
    break;
  case F_LIN_X_S:
    /* S$ = LIN( X ) */
    {
      /* P1LEN */
      if (x == 0)
      {
        /* no copies */
      }
      else
      {
        bwb_memset (S, (char) '\n', x);
        s = x;
      }
    }
    break;
  case F_MID4_A_X_S:
  case F_MID_A_X_S:
    /* S$ = MID$( A$, X ) */
    /* S$ = MID( A$, X ) */
    {
      /* P1ANY|P2POS */
      if (a == 0)
      {
        /* empty string */
      }
      else if (x > a)
      {
        /* start beyond length */
      }
      else
      {
        x--;                        /* BASIC to C */
        a -= x;                        /* nummber of characters to
                                 * copy */
        A += x;                        /* pointer to first character
                                 * to copy */
        bwb_memcpy (S, A, a);
        s = a;
      }
    }
    break;
  case F_MID4_A_X_Y_S:
  case F_MID_A_X_Y_S:
  case F_SEG4_A_X_Y_S:
  case F_SEG_A_X_Y_S:
    /* S$ = MID$( A$, X, Y ) */
    /* S$ = MID( A$, X, Y ) */
    /* S$ = SEG$( A$, X, Y ) */
    /* S$ = SEG( A$, X, Y ) */
    {
      /* P1ANY|P2POS|P3LEN */
      if (a == 0)
      {
        /* empty string */
      }
      else if (x > a)
      {
        /* start beyond length */
      }
      else if (y == 0)
      {
        /* empty string */
      }
      else
      {
        x--;                        /* BASIC to C */
        a -= x;
        /* maximum nummber of characters to
         * copy */
        a = MIN (a, y);
        A += x;
        /* pointer to first character to copy */
        bwb_memcpy (S, A, a);
        s = a;
      }
    }
    break;
  case F_LEFT4_A_X_S:
  case F_LEFT_A_X_S:
    /* S$ = LEFT$( A$, X ) */
    /* S$ = LEFT( A$, X ) */
    {
      /* P1ANY|P2LEN */
      if (a == 0)
      {
        /* empty string */
      }
      else if (x == 0)
      {
        /* empty string */
      }
      else
      {
        a = MIN (a, x);
        bwb_memcpy (S, A, a);
        s = a;
      }
    }
    break;
  case F_RIGHT4_A_X_S:
  case F_RIGHT_A_X_S:
    /* S$ = RIGHT$( A$, X ) */
    /* S$ = RIGHT( A$, X ) */
    {
      /* P1ANY|P2LEN */
      if (a == 0)
      {
        /* empty string */
      }
      else if (x == 0)
      {
        /* empty string */
      }
      else
      {
        x = MIN (a, x);
        A += a;
        A -= x;
        bwb_memcpy (S, A, x);
        s = x;
      }
    }
    break;
  case F_HEX_A_N:
    /* N  = HEX( A$ ) */
    {
      if (a == 0)
      {
        /* empty string */
      }
      else
      {
        N = strtoul (A, (char **) NULL, 16);
      }
    }
    break;
  case F_HEX4_X_S:
    /* S$ = HEX$( X ) */
    {
      /* P1NUM */
      /* P1INT */
      sprintf (S, "%X", x);
      s = bwb_strlen (S);
    }
    break;
  case F_HEX4_X_Y_S:
    /* S$ = HEX$( X, Y ) */
    {
      /* P1NUM | P2NUM */
      /* P1INT | P2BYT */
      if (y == 0)
      {
        /* empty string */
      }
      else
      {
        sprintf (S, "%0*X", y, x);
        s = bwb_strlen (S);
        if (y < s)
        {
          A = S;
          a = s - y;                /* number of characters to trim */
          A += a;
          bwb_strcpy (S, A);
        }
      }
    }
    break;
  case F_OCT4_X_S:
    /* S$ = OCT$( X ) */
    {
      /* P1NUM */
      /* P1INT */
      sprintf (S, "%o", x);
      s = bwb_strlen (S);
    }
    break;
  case F_OCT4_X_Y_S:
    /* S$ = OCT$( X, Y ) */
    {
      /* P1NUM | P2NUM */
      /* P1INT | P2BYT */
      if (y == 0)
      {
        /* empty string */
      }
      else
      {
        sprintf (S, "%0*o", y, x);
        s = bwb_strlen (S);
        if (y < s)
        {
          A = S;
          a = s - y;                /* number of characters to trim */
          A += a;
          bwb_strcpy (S, A);
        }
      }
    }
    break;
  case F_BIN4_X_S:
    /* S$ = BIN$( X ) */
    {
      /* P1NUM */
      /* P1INT */
      /*
       **
       ** we break this problem into two parts:
       ** 1.  generate the default string
       ** 2.  trim leading zeroes on the left
       **
       */
      unsigned long z;
      z = (unsigned long) x;
      A = My->NumLenBuffer;
      a = sizeof (z) * CHAR_BIT;
      s = a;
      bwb_memset (A, '0', a);
      A[a] = NulChar;
      while (a)
      {
        /* look at the Least Significant Bit */
        a--;
        if (z & 1)
        {
          A[a] = '1';
        }
        z /= 2;
      }
      /* bwb_strcpy( S, A ); */
      /* same as HEX$(X) and OCT$(X), trim leading zeroes */
      while (*A == '0')
      {
        A++;
      }
      if (*A)
      {
        bwb_strcpy (S, A);
      }
      else
      {
        /* special case (x == 0), we trimmed all the zeroes above */
        S[0] = '0';
        s = 1;
      }
    }
    break;
  case F_BIN4_X_Y_S:
    /* S$ = BIN$( X, Y ) */
    {
      /* P1NUM | P2NUM */
      /* P1INT | P2BYT */
      /*
       **
       ** we break this problem into two parts:
       ** 1.  generate the default string
       ** 2.  pad or trim on the left
       **
       */
      if (y == 0)
      {
        /* empty string */
      }
      else
      {
        unsigned long z;
        z = (unsigned long) x;
        A = My->NumLenBuffer;
        a = sizeof (z) * CHAR_BIT;
        s = a;
        bwb_memset (A, '0', a);
        A[a] = NulChar;
        while (a)
        {
          /* look at the Least Significant Bit */
          a--;
          if (z & 1)
          {
            A[a] = '1';
          }
          z /= 2;
        }
        /* bwb_strcpy( S, A ); */
        if (y > s)
        {
          /* pad left */
          a = y - s;                /* number of characters to pad (at least one) */
          bwb_memset (S, '0', a);
          S[a] = NulChar;
          bwb_strcat (S, A);
        }
        else
        {
          /* trim left (y <= s) */
          a = s - y;                /* number of characters to trim (may be zero) */
          A += a;
          bwb_strcpy (S, A);
        }
        s = y;
      }
    }
    break;
  case F_EDIT4_A_X_S:
    /* S$ = EDIT$( A$, X ) */
    {
      /* P1ANY|P2INT */
      if (x < 0)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else if (a == 0)
      {
        /* empty string */
      }
      else if (x == 0)
      {
        /* no changes */
        bwb_memcpy (S, A, a);
        s = a;
      }
      else
      {
        int n;
        char IsSuppress;
        char LastC;
        n = a;
        a = 0;
        IsSuppress = NulChar;
        LastC = NulChar;

        if (x & 8)
        {
          /* discard leading spaces and tabs */
          while (A[a] == ' ' || A[a] == '\t')
            a++;
        }
        while (a < n)
        {
          char C;

          C = A[a];
          if (x & 256)
          {
            /*
             ** suppress editing for characters within quotes. 
             */
            if (IsSuppress)
            {
              if (C == IsSuppress)
                IsSuppress = NulChar;
              goto VERBATIM;
            }
            if (C == '"')
            {
              IsSuppress = C;
              goto VERBATIM;
            }
            if (C == '\'')
            {
              IsSuppress = C;
              goto VERBATIM;
            }
          }
          /* edit the character */
          if (x & 1)
          {
            /* discard parity bit */
            C = C & 0x7F;
          }
          if (x & 2)
          {
            /* discard all spaces and tabs */
            if (C == ' ')
              goto SKIP;
            if (C == '\t')
              goto SKIP;
          }
          if (x & 4)
          {
            /* discard all carriage returns, line feeds, form feeds, deletes, escapes and nulls */
            if (C == '\r')
              goto SKIP;
            if (C == '\n')
              goto SKIP;
            if (C == '\f')
              goto SKIP;
            if (C == 127)
              goto SKIP;
            if (C == 26)
              goto SKIP;
            if (C == 0)
              goto SKIP;
          }
          if (x & 16)
          {
            /* convert multiple spaces and tabs to one space */
            if (C == '\t')
              C = ' ';
            if (C == ' ' && LastC == ' ')
              goto SKIP;
          }
          if (x & 32)
          {
            /* convert lower case to upper case */
            C = bwb_toupper (C);
          }
          if (x & 64)
          {
            /* convert left brackets to left parentheses and right brackes to right parentheses */
            if (C == '[')
              C = '(';
            if (C == ']')
              C = ')';
          }
          /* save results of editing */
        VERBATIM:
          S[s] = C;
          s++;
        SKIP:
          LastC = C;
          a++;
        }
        if (x & 128)
        {
          /* discard trailing spaces and tabs */
          while (s > 0 && (S[s - 1] == ' ' || S[s - 1] == '\t'))
            s--;
        }
      }
    }
    break;
  case F_CHR_X_S:
  case F_CHR4_X_S:
  case F_CHAR4_X_S:
    /* S$ = CHR( X ) */
    /* S$ = CHR$( X ) */
    /* S$ = CHAR$( X ) */

    /* P1ANY */
    if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
    {
      /* IBM System/360 & System/370 BASIC dialects: the opposite of N = NUM( A$ ) */
      FormatBasicNumber (X, S);
      s = bwb_strlen (S);
    }
    else
    {
      if (x < MINBYT || x > MAXBYT)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        S[0] = (char) x;
        s = 1;
      }
    }
    break;
  case F_CHAR_X_Y_S:
    /* S$ = CHAR( X, Y ) ' same as STRING$(Y,X) */
    {
      /* P1BYT|P2LEN */
      if (y == 0)
      {
        /* no copies */
      }
      else
      {
        bwb_memset (S, (char) x, y);
        s = y;
      }
    }
    break;
  case F_LEN_A_N:
    /* N = LEN( A$ ) */
    {
      N = a;
    }
    break;
  case F_POS_A_B_N:
    /* N = POS( A$, B$ ) */
    {
      if (b == 0)
      {
        /* empty pattern */
        N = 1;
      }
      else if (a == 0)
      {
        /* empty searched */
      }
      else if (b > a)
      {
        /* pattern is longer than searched */
      }
      else
      {
        /* search */
        int i;
        int n;
        n = a - b;                /* last valid search
                                 * position */
        n++;
        /* search */
        for (i = 0; i < n; i++)
        {
          if (bwb_memcmp (A, B, b) == 0)
          {
            /* FOU ND */
            i++;                /* C to BASIC */
            N = i;
            i = n;                /* exit for */
          }
          A++;
        }
      }
    }
    break;
  case F_MATCH_A_B_X_N:
    /* N = POS( A$, B$, X ) */
    {
      N = str_match (A, a, B, b, x);
    }
    break;
  case F_POS_A_B_X_N:
    /* N = POS( A$, B$, X ) */
    {
      if (b == 0)
      {
        /* empty pattern */
        N = 1;
      }
      else if (a == 0)
      {
        /* empty searched */
      }
      else if (b > a)
      {
        /* pattern is longer than searched */
      }
      else
      {
        /* search */
        int i;
        int n;
        n = a - b;                /* last valid search position */
        n++;

        /* search */
        x--;                        /* BASIC to C */
        A += x;                        /* advance to the start
                                 * position */
        for (i = x; i < n; i++)
        {
          if (bwb_memcmp (A, B, b) == 0)
          {
            /* FOUND */
            N = i + 1;                /* C to BASIC */
            i = n;                /* exit for */
          }
          A++;
        }
      }
    }
    break;
  case F_VAL_A_N:
  case F_NUM_A_N:
    /* N = VAL( A$ ) */
    /* N = NUM( A$ ) */
    {
      /* P1ANY */
      int n;                        /* number of characters read */
      DoubleType Value;

      n = 0;
      if (sscanf (A, DecScanFormat, &Value, &n) == 1)
      {
        /* OK */
        N = Value;
      }
      else
      {
        /* not a number */
        if (My->CurrentVersion->OptionFlags & OPTION_BUGS_ON ) /* VAL("X") = 0 */ 
        {
          /* IGNORE */
          N = 0;
        }
        else
        {
          /* ERROR */
          WARN_ILLEGAL_FUNCTION_CALL;
        }
      }
    }
    break;
  case F_STR4_X_S:
  case F_NUM4_X_S:
    /* S$ = STR$( X ) */
    /* S$ = NUM$( X ) */
    {
      /* P1ANY */
      FormatBasicNumber (X, S);
      s = bwb_strlen (S);
    }
    break;
  case F_DATE_N:
    /* N = DATE ' YYYYDDD  */
    {
      /* PNONE */

      /* ECMA-116 */
      time (&t);
      lt = localtime (&t);
      N = lt->tm_year;
      N *= 1000;
      N += lt->tm_yday;
      N += 1;
    }
    break;
  case F_DATE4_X_S:
  case F_DATE4_S:
  case F_DAT4_S:
    /* S$ = DATE$( X ) ' value of X is ignored */
    /* S$ = DATE$ */
    /* S$ = DAT$ */
    {
      /* PNONE */
      if (!is_empty_string (My->CurrentVersion->OptionDateFormat))
      {
        time (&t);
        lt = localtime (&t);
        s = strftime (S, MAXLEN, My->CurrentVersion->OptionDateFormat, lt);
      }
    }
    break;
  case F_CLK_X_S:
  case F_CLK4_S:
  case F_TI4_S:
  case F_TIME4_S:
  case F_TIME4_X_S:
    /* S$ = CLK(X) ' the value of paameter X is ignored */
    /* S$ = CLK$ */
    /* S$ = TI$ */
    /* S$ = TIME$ */
    /* S$ = TIME$(X) ' the value of paameter X is ignored */
    {
      /* PNONE */
      if (!is_empty_string (My->CurrentVersion->OptionTimeFormat))
      {
        time (&t);
        lt = localtime (&t);
        s = strftime (S, MAXLEN, My->CurrentVersion->OptionTimeFormat, lt);
      }
    }
    break;
  case F_TI_N:
  case F_TIM_N:
  case F_TIME_N:
  case F_TIME_X_N:
  case F_TIMER_N:
    /* N = TI */
    /* N = TIM */
    /* N = TIME */
    /* N = TIME( X ) ' value of X is ignored */
    /* N = TIMER */
    /* N = CPU */
    {
      /* PNONE */
      time (&t);
      lt = localtime (&t);
      if (My->CurrentVersion->OptionVersionValue & (G67 | G74))
      {
        N = lt->tm_hour;
        N *= 60;
        N += lt->tm_min;
        N *= 60;
        N += lt->tm_sec;
        /* number of seconds since midnight */
        N -= My->StartTimeInteger;
        /* elapsed run time */
      }
      else
      {
        N = lt->tm_hour;
        N *= 60;
        N += lt->tm_min;
        N *= 60;
        N += lt->tm_sec;
        /* number of seconds since midnight */
      }
    }
    break;
  case F_CLK_X_N:
    /* N = CLK( X ) ' value of X is ignored */
    {
      /* PNONE */
      time (&t);
      lt = localtime (&t);
      N = lt->tm_hour;
      N *= 60;
      N += lt->tm_min;
      N *= 60;
      N += lt->tm_sec;
      N /= 3600;
      /* decimal hours: 3:30 PM = 15.50 */
    }
    break;

  case F_TIM_X_N:
    /* N = TIM( X ) */
    {
      /* P1BYT */
      time (&t);
      lt = localtime (&t);

      if (My->CurrentVersion->OptionVersionValue & (G65 | G67 | G74))
      {
        /* value of 'X' is ignored */
        N = lt->tm_hour;
        N *= 60;
        N += lt->tm_min;
        N *= 60;
        N += lt->tm_sec;
        /* number of seconds since midnight */
        N -= My->StartTimeInteger;
        /* elapsed run time */
      }
      else
      {
        switch (x)
        {
        case 0:
          /* TIM(0) == minute (0..59) */
          N += lt->tm_min;
          break;
        case 1:
          /* TIM(1) == hour (0..23) */
          N = lt->tm_hour;
          break;
        case 2:
          /* TIM(2) == day of year (1..366) */
          N = 1 + lt->tm_yday;
          break;
        case 3:
          /* TIM(3) == year since 1900  (0..) */
          N = lt->tm_year;
          break;
        default:
          WARN_ILLEGAL_FUNCTION_CALL;
        }
      }
    }
    break;
  case F_COMMAND4_S:
    /* S$ = COMMAND$ */
    {
      S[0] = NulChar;
      for (x = 0; x < 10 && My->COMMAND4[x] != NULL; x++)
      {
        if (x > 0)
        {
          bwb_strcat (S, " ");
        }
        bwb_strcat (S, My->COMMAND4[x]);
      }
      s = bwb_strlen (S);
    }
    break;
  case F_COMMAND4_X_S:
    /* S$ = COMMAND$(X) */
    if (x < 0 || x > 9)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
    }
    else
    {
      if (My->COMMAND4[x] == NULL)
      {
        s = 0;
      }
      else
      {
        bwb_strcpy (S, My->COMMAND4[x]);
        s = bwb_strlen (My->COMMAND4[x]);
      }
    }
    break;
  case F_COSH_X_N:
  case F_CSH_X_N:
  case F_HCS_X_N:
    /* N = COSH( X ) */
    /* N = CSH( X ) */
    /* N = HCS( X ) */
    {
      /* P1ANY */
      N = cosh (X);
    }
    break;
  case F_SINH_X_N:
  case F_SNH_X_N:
  case F_HSN_X_N:
    /* N = SINH( X ) */
    /* N = SNH( X ) */
    /* N = HSN( X ) */
    {
      /* P1ANY */
      N = sinh (X);
    }
    break;
  case F_TANH_X_N:
  case F_HTN_X_N:
    /* N = TANH( X ) */
    /* N = HTN( X ) */
    {
      /* P1ANY */
      N = tanh (X);
    }
    break;
  case F_CLG_X_N:
  case F_CLOG_X_N:
  case F_LOG10_X_N:
  case F_LGT_X_N:
    /* N = CLG( X ) */
    /* N = CLOG( X ) */
    /* N = LOG10( X ) */
    /* N = LGT( X ) */
    {
      /* P1GTZ */
      N = log10 (X);
    }
    break;
  case F_SLEEP_X_N:
  case F_WAIT_X_N:
  case F_PAUSE_X_N:
    /* N = SLEEP( X ) */
    /* N = WAIT( X ) */
    /* N = PAUSE( X ) */
    {
      /* P1ANY */
      X = X * My->OptionSleepDouble;
      if (X <= 0 || X > MAXINT)
      {
        /* do nothing */
      }
      else
      {
        x = (int) bwb_rint (X);
        sleep (x);
      }
    }
    break;
  case F_LOG2_X_N:
  case F_LTW_X_N:
    /* N = LOG2( X ) */
    /* N = LTW( X ) */
    {
      /* P1GTZ */
      N = log (X) / log ((DoubleType) 2);
    }
    break;
  case F_ACOS_X_N:
  case F_ACS_X_N:
  case F_ARCCOS_X_N:
    /* N = ACOS( X ) */
    /* N = ACS( X ) */
    /* N = ARCCOS( X ) */
    {
      /* P1ANY */
      if (X < -1 || X > 1)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = acos (X);
        if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
        {
          N = FromRadiansToDegrees (N);
        }
        else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
        {
          N = FromRadiansToGradians (N);
        }
      }
    }
    break;
  case F_ACSD_X_N:
    /* N = ACSD( X ) */
    {
      /* P1ANY */
      if (X < -1 || X > 1)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = acos (X);
        /* result is always in DEGREES, regardless of OPTION ANGLE setting */
        N = FromRadiansToDegrees (N);
      }
    }
    break;
  case F_ACSG_X_N:
    /* N = ACSG( X ) */
    {
      /* P1ANY */
      if (X < -1 || X > 1)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = acos (X);
        /* result is always in GRADIANS, regardless of OPTION ANGLE setting */
        N = FromRadiansToGradians (N);
      }
    }
    break;

  case F_ASIN_X_N:
  case F_ASN_X_N:
  case F_ARCSIN_X_N:
    /* N = ASIN( X ) */
    /* N = ASN( X ) */
    /* N = ARCSIN( X ) */
    {
      /* P1ANY */
      if (X < -1 || X > 1)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = asin (X);
        if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
        {
          N = FromRadiansToDegrees (N);
        }
        else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
        {
          N = FromRadiansToGradians (N);
        }
      }
    }
    break;


  case F_ASND_X_N:
    /* N = ASND( X ) */
    {
      /* P1ANY */
      if (X < -1 || X > 1)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = asin (X);
        /* result is always in DEGREES, regardless of OPTION ANGLE setting */
        N = FromRadiansToDegrees (N);
      }
    }
    break;
  case F_ASNG_X_N:
    /* N = ASNG( X ) */
    {
      /* P1ANY */
      if (X < -1 || X > 1)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = asin (X);
        /* result is always in GRADIANS, regardless of OPTION ANGLE setting */
        N = FromRadiansToGradians (N);
      }
    }
    break;


  case F_COT_X_N:
    /* N = COT( X ) ' = 1 / TAN( X ) */
    {
      /* P1ANY */
      DoubleType T;
      if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
      {
        X = FromDegreesToRadians (X);
      }
      else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
      {
        X = FromGradiansToRadians (X);
      }
      T = tan (X);
      if (T == 0)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = 1.0 / T;
      }
    }
    break;
  case F_CSC_X_N:
    /* N = CSC( X ) ' = 1 / SIN( X ) */
    {
      /* P1ANY */
      DoubleType T;
      if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
      {
        X = FromDegreesToRadians (X);
      }
      else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
      {
        X = FromGradiansToRadians (X);
      }
      T = sin (X);
      if (T == 0)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = 1.0 / T;
      }
    }
    break;
  case F_SEC_X_N:
    /* N = SEC( X ) ' = 1 / COS( X ) */
    {
      /* P1ANY */
      DoubleType T;
      if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
      {
        X = FromDegreesToRadians (X);
      }
      else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
      {
        X = FromGradiansToRadians (X);
      }
      T = cos (X);
      if (T == 0)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = 1.0 / T;
      }
    }
    break;
  case F_UCASE4_A_S:
  case F_UPPER4_A_S:
    /* S$ = UCASE$( A$ ) */
    /* S$ = UPPER$( A$ ) */
    {
      /* P1ANY */
      if (a == 0)
      {
        /* empty string */
      }
      else
      {
        int i;
        bwb_memcpy (S, A, a);
        s = a;
        /* BASIC allows embedded NULL
         * characters */
        for (i = 0; i < a; i++)
        {
          S[i] = bwb_toupper (S[i]);
        }
      }
    }
    break;
  case F_LCASE4_A_S:
  case F_LOWER4_A_S:
    /* S$ = LCASE$( A$ ) */
    /* S$ = LOWER$( A$ ) */
    {
      /* P1ANY */
      if (a == 0)
      {
        /* empty string */
      }
      else
      {
        int i;
        bwb_memcpy (S, A, a);
        s = a;
        /* BASIC allows embedded NULL
         * characters */
        for (i = 0; i < a; i++)
        {
          S[i] = bwb_tolower (S[i]);
        }
      }
    }
    break;
  case F_ANGLE_X_Y_N:
    /* N = ANGLE( X, Y ) */
    {
      /* P1ANY|P2ANY */
      if (X == 0 && Y == 0)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        N = atan2 (Y, X);
        if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
        {
          N = FromRadiansToDegrees (N);
        }
        else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
        {
          N = FromRadiansToGradians (N);
        }
      }
    }
    break;
  case F_CEIL_X_N:
    /* N = CEIL( X ) */
    {
      /* P1ANY */
      N = ceil (X);
    }
    break;
  case F_DET_N:
    /* N = DET */
    {
      /* PNONE */
      N = My->LastDeterminant;
    }
    break;
  case F_NUM_N:
    /* N = NUM */
    {
      /* PNONE */
      N = My->LastInputCount;
    }
    break;
  case F_DEG_N:
  case F_DEGREE_N:
    /* N = DEG */
    /* N = DEGREE */
    {
      /* PNONE */
      My->CurrentVersion->OptionFlags |= OPTION_ANGLE_DEGREES;
      My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_GRADIANS;
      N = 0;
    }
    break;
  case F_RAD_N:
  case F_RADIAN_N:
    /* N = RAD */
    /* N = RADIAN */
    {
      /* PNONE */
      My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_DEGREES;
      My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_GRADIANS;
      N = 0;
    }
    break;
  case F_GRAD_N:
  case F_GRADIAN_N:
    /* N = GRAD */
    /* N = GRADIAN */
    {
      /* PNONE */
      My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_DEGREES;
      My->CurrentVersion->OptionFlags |= OPTION_ANGLE_GRADIANS;
      N = 0;
    }
    break;
  case F_DEG_X_N:
  case F_DEGREE_X_N:
    /* N = DEG( X ) */
    /* N = DEGREE( X ) */
    {
      /* P1ANY */
      if (My->CurrentVersion->OptionVersionValue & (R86))
      {
        if (x == 0)
        {
          /* DEG 0 */
          My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_DEGREES;
          My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_GRADIANS;
        }
        else
        {
          /* DEG 1 */
          My->CurrentVersion->OptionFlags |= OPTION_ANGLE_DEGREES;
          My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_GRADIANS;
        }
        N = 0;
      }
      else
      {
        N = FromRadiansToDegrees (X);
      }
    }
    break;
  case F_RAD_X_N:
    /* N = RAD( X ) */
    {
      /* P1ANY */
      N = FromDegreesToRadians (X);
    }
    break;
  case F_PI_N:
    /* N = PI */
    {
      /* PNONE */
      N = PI;
    }
    break;
  case F_PI_X_N:
    /* N = PI(X) */
    {
      /* P1ANY */
      N = PI * X;
    }
    break;
  case F_LTRIM4_A_S:
    /* S$ = LTRIM$( A$ ) */
    {
      /* P1ANY */
      if (a == 0)
      {
        /* empty string */
      }
      else
      {
        int i;
        /* BASIC allows embedded NULL characters */
        for (i = 0; i < a && A[i] == ' '; i++)
        {
          /* skip spaces */
        }
        /* 'A[ i ]' is first non-space character */
        if (i >= a)
        {
          /* empty string */
        }
        else
        {
          A += i;
          a -= i;
          bwb_memcpy (S, A, a);
          s = a;
        }
      }
    }
    break;
  case F_RTRIM4_A_S:
    /* S$ = RTRIM$( A$ ) */
    {
      /* P1ANY */
      if (a == 0)
      {
        /* empty string */
      }
      else
      {
        int i;
        /* BASIC allows embedded NULL characters */
        for (i = a - 1; i >= 0 && A[i] == ' '; i--)
        {
          /* skip spaces */
        }
        /* 'A[ i ]' is last non-space character */
        if (i < 0)
        {
          /* empty string */
        }
        else
        {
          a = i + 1;
          bwb_memcpy (S, A, a);
          s = a;
        }
      }
    }
    break;
  case F_STRIP4_A_S:
    /* S$ = STRIP$( A$ ) */
    {
      /* P1ANY */
      if (a == 0)
      {
        /* empty string */
      }
      else
      {
        int i;
        for (i = 0; i < a; i++)
        {
          S[i] = A[i] & 0x7F;
        }
        s = a;
        S[s] = NulChar;
      }
    }
    break;
  case F_TRIM4_A_S:
    /* S$ = TRIM$( A$ ) */
    {
      /* P1ANY */
      if (a == 0)
      {
        /* empty string */
      }
      else
      {
        /*
         **
         ** LTRIM
         **
         */
        int i;
        /* BASIC allows embedded NULL characters */
        for (i = 0; i < a && A[i] == ' '; i++)
        {
          /* skip spaces */
        }
        /* 'A[ i ]' is first non-space character */
        if (i >= a)
        {
          /* empty string */
        }
        else
        {
          A += i;
          a -= i;
          bwb_memcpy (S, A, a);
          s = a;
          /*
           **
           ** RTRIM
           **
           */
          A = S;
          a = s;
          if (a == 0)
          {
            /* empty string */
          }
          else
          {
            int i;
            /* BASIC allows embedded NULL characters */
            for (i = a - 1; i >= 0 && A[i] == ' '; i--)
            {
              /* skip spaces */
            }
            /* 'A[ i ]' is last non-space character */
            if (i < 0)
            {
              /* empty string */
            }
            else
            {
              a = i + 1;
              /* bwb_memcpy( S, A, a ); */
              s = a;
            }
          }
        }
      }
    }
    break;
  case F_MAX_X_Y_N:
    /* N = MAX( X, Y ) */
    {
      N = MAX (X, Y);
    }
    break;
  case F_MAX_A_B_S:
    /* S$ = MAX( A$, B$ ) */
    {
      StringType L;
      StringType R;

      L.length = a;
      R.length = b;
      L.sbuffer = A;
      R.sbuffer = B;
      if (str_cmp (&L, &R) >= 0)
      {
        /* A >= B */
        bwb_memcpy (S, A, a);
        s = a;
      }
      else
      {
        /* A < B */
        bwb_memcpy (S, B, b);
        s = b;
      }
    }
    break;
  case F_MIN_X_Y_N:
    /* N = MIN( X, Y ) */
    {
      N = MIN (X, Y);
    }
    break;
  case F_MIN_A_B_S:
    /* S$ = MIN( A$, B$ ) */
    {
      StringType L;
      StringType R;

      L.length = a;
      R.length = b;
      L.sbuffer = A;
      R.sbuffer = B;
      if (str_cmp (&L, &R) <= 0)
      {
        /* A <= B */
        bwb_memcpy (S, A, a);
        s = a;
      }
      else
      {
        /* A > B */
        bwb_memcpy (S, B, b);
        s = b;
      }
    }
    break;
  case F_FP_X_N:
  case F_FRAC_X_N:
    /* N = FP( X ) */
    /* N = FRAC( X ) */
    {
      DoubleType FP;
      DoubleType IP;
      FP = modf (X, &IP);
      N = FP;
    }
    break;
  case F_IP_X_N:
    /* N = IP( X ) */
    {
      DoubleType IP;
      modf (X, &IP);
      N = IP;
    }
    break;
  case F_EPS_X_N:
    /* N = EPS( Number ) */
    {
      N = DBL_MIN;
    }
    break;
  case F_MAXLVL_N:
    /* N = MAXLVL */
    {
      N = EXECLEVELS;
    }
    break;
  case F_MAXNUM_N:
    /* N = MAXNUM */
    {
      N = MAXDBL;
    }
    break;
  case F_MINNUM_N:
    /* N = MINNUM */
    {
      N = MINDBL;
    }
    break;
  case F_MAXDBL_N:
    /* N = MAXDBL */
    {
      N = MAXDBL;
    }
    break;
  case F_MINDBL_N:
    /* N = MINDBL */
    {
      N = MINDBL;
    }
    break;
  case F_MAXSNG_N:
    /* N = MAXSNG */
    {
      N = MAXSNG;
    }
    break;
  case F_MINSNG_N:
    /* N = MINSNG */
    {
      N = MINSNG;
    }
    break;
  case F_MAXCUR_N:
    /* N = MAXCUR */
    {
      N = MAXCUR;
    }
    break;
  case F_MINCUR_N:
    /* N = MINCUR */
    {
      N = MINCUR;
    }
    break;
  case F_MAXLNG_N:
    /* N = MAXLNG */
    {
      N = MAXLNG;
    }
    break;
  case F_MINLNG_N:
    /* N = MINLNG */
    {
      N = MINLNG;
    }
    break;
  case F_MAXINT_N:
    /* N = MAXINT */
    {
      N = MAXINT;
    }
    break;
  case F_MININT_N:
    /* N = MININT */
    {
      N = MININT;
    }
    break;
  case F_MAXBYT_N:
    /* N = MAXBYT */
    {
      N = MAXBYT;
    }
    break;
  case F_MINBYT_N:
    /* N = MINBYT */
    {
      N = MINBYT;
    }
    break;
  case F_MAXDEV_N:
    /* N = MAXDEV */
    {
      N = MAXDEV;
    }
    break;
  case F_MINDEV_N:
    /* N = MINDEV */
    {
      N = MINDEV;
    }
    break;

  case F_MOD_X_Y_N:
    /* N = MOD( X, Y ) */
    {
      /* P1ANY|P2NEZ */
      DoubleType IP;

      IP = floor (X / Y);
      N = X - (Y * IP);
    }
    break;
  case F_REMAINDER_X_Y_N:
    /* REMAINDER( X, Y ) */
    {
      /* P1ANY|P2NEZ */
      DoubleType Value;
      DoubleType IP;

      Value = X / Y;
      modf (Value, &IP);
      N = X - (Y * IP);
    }
    break;
  case F_ROUND_X_Y_N:
    /* N = ROUND( X, Y ) == INT(X*10^Y+.5)/10^Y */
    {
      /* P1ANY | P2INT */
      if (y < -32 || y > 32)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        DoubleType T;                /* 10^Y */

        T = pow (10.0, Y);
        if (T == 0)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          N = floor (X * T + 0.5) / T;
        }
      }
    }
    break;
  case F_TRUNCATE_X_Y_N:
    /* N = TRUNCATE( X, Y ) == INT(X*10^Y)/10^Y */
    {
      /* P1ANY | P2INT */
      if (y < -32 || y > 32)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        DoubleType T;                /* 10^Y */

        T = pow (10.0, Y);
        if (T == 0)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          N = floor (X * T) / T;
        }
      }
    }
    break;
  case F_MAXLEN_A_N:
  case F_MAXLEN_N:
    /* N = MAXLEN( A$ ) */
    /* N = MAXLEN       */
    {
      N = MAXLEN;
    }
    break;
  case F_ORD_A_N:
    /* N = ORD( A$ ) */
    {
      /* P1BYT */
      if (a == 1)
      {
        /* same as ASC(A$) */
        N = A[0];
      }
      else
      {
        /* lookup Acronym */
        N = -1;
        for (x = 0; x < NUM_ACRONYMS; x++)
        {
          if (bwb_stricmp (AcronymTable[x].Name, A) == 0)
          {
            /* FOUND */
            N = AcronymTable[x].Value;
            break;
          }
        }
        if (N < 0)
        {
          /* NOT FOUND */
          WARN_ILLEGAL_FUNCTION_CALL;
          N = 0;
        }
      }
    }
    break;
  case F_RENAME_A_B_N:
    /* N = RENAME( A$, B$ ) */
    {
      /* P1BYT | P2BYT */
      if (rename (A, B))
      {
        /* ERROR -- return FALSE */
        N = 0;
      }
      else
      {
        /* OK -- return TRUE */
        N = -1;
      }
    }
    break;
  case F_SIZE_A_N:
    /* N = SIZE( A$ ) */
    {
      /* P1BYT */
      FILE *F;

      F = fopen (A, "rb");
      if (F != NULL)
      {
        long n;

        fseek (F, 0, SEEK_END);
        n = ftell (F);
        bwb_fclose (F);

        if (n > 0)
        {
          /* round up filesize to next whole kilobyte */
          n += 1023;
          n /= 1024;
        }
        else
        {
          /* a zero-length file returns 0 */
          n = 0;
        }
        N = n;
      }
      /* a non-existing file returns 0 */
    }
    break;
  case F_REPEAT4_X_Y_S:
    /* S$ = REPEAT$( X, Y ) ' X is count, Y is code */
    {
      /* P1LEN | P2BYT */
      if (x == 0)
      {
        /* empty string */
      }
      else
      {
        bwb_memset (S, (char) y, x);
        s = x;
      }
    }
    break;
  case F_REPEAT4_X_A_S:
    /* S$ = REPEAT$( X, A$ ) ' X is count, A$ is code */
    {
      /* P1LEN | P2BYT */
      if (x == 0)
      {
        /* empty string */
      }
      else
      {
        bwb_memset (S, (char) A[0], x);
        s = x;
      }
    }
    break;
  case F_FIX_X_N:
    /* N = FIX( X ) */
    {
      /* N = bwb_rint(X); */
      if (X < 0)
      {
        N = -floor (-X);
      }
      else
      {
        N = floor (X);
      }
    }
    break;
  case F_ABS_X_N:
    /* N = ABS( X ) */
    {
      N = fabs (X);
    }
    break;
  case F_ATN_X_N:
  case F_ATAN_X_N:
  case F_ARCTAN_X_N:
    /* N = ATN( X ) */
    /* N = ATAN( X ) */
    /* N = ARCTAN( X ) */
    {
      N = atan (X);
      if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
      {
        N = FromRadiansToDegrees (N);
      }
      else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
      {
        N = FromRadiansToGradians (N);
      }
    }
    break;
  case F_ATND_X_N:
    /* N = ATND( X ) */
    {
      N = atan (X);
      N = FromRadiansToDegrees (N);
    }
    break;
  case F_ATNG_X_N:
    /* N = ATNG( X ) */
    {
      N = atan (X);
      N = FromRadiansToGradians (N);
    }
    break;
  case F_COS_X_N:
    /* N = COS( X ) */
    {
      if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
      {
        X = FromDegreesToRadians (X);
      }
      else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
      {
        X = FromGradiansToRadians (X);
      }
      N = cos (X);
    }
    break;
  case F_COSD_X_N:
    /* N = COSD( X ) */
    {
      X = FromDegreesToRadians (X);
      N = cos (X);
    }
    break;
  case F_COSG_X_N:
    /* N = COSG( X ) */
    {
      X = FromGradiansToRadians (X);
      N = cos (X);
    }
    break;
  case F_EXP_X_N:
    /* N = EXP( X ) */
    {
      N = exp (X);
    }
    break;
  case F_INT_X_N:
    /* N = INT( X ) */
    {
      N = floor (X);
    }
    break;
  case F_FLOAT_X_N:
  case F_INT5_X_N:
    /* N = FLOAT( X ) */
    /* N = INT%( X ) */
    {
      N = bwb_rint (X);
    }
    break;
  case F_INITIALIZE_N:
    /* INITIALIZE */
    {
      N = 0;
    }
    break;
  case F_LOG_X_N:
  case F_LN_X_N:
  case F_LOGE_X_N:
    /* N = LOG( X ) */
    /* N = LN( X ) */
    /* N = LOGE( X ) */
    {
      /* P1GTZ */
      N = log (X);
    }
    break;
  case F_RND_N:
    /* N = RND */
    {
      N = rand ();
      N /= RAND_MAX;
    }
    break;
  case F_RND_X_N:
    /* N = RND( X ) */
    {
      N = rand ();
      N /= RAND_MAX;
    }
    break;
  case F_SGN_X_N:
    /* N = SGN( X ) */
    {
      if (X > 0)
      {
        N = 1;
      }
      else if (X < 0)
      {
        N = -1;
      }
      else
      {
        N = 0;
      }
    }
    break;
  case F_SIN_X_N:
    /* N = SIN( X ) */
    {
      if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
      {
        X = FromDegreesToRadians (X);
      }
      else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
      {
        X = FromGradiansToRadians (X);
      }
      N = sin (X);
    }
    break;
  case F_SIND_X_N:
    /* N = SIND( X ) */
    {
      X = FromDegreesToRadians (X);
      N = sin (X);
    }
    break;
  case F_SING_X_N:
    /* N = SING( X ) */
    {
      X = FromGradiansToRadians (X);
      N = sin (X);
    }
    break;
  case F_SQR_X_N:
  case F_SQRT_X_N:
    /* N = SQR( X ) */
    /* N = SQRT( X ) */
    {
      /* P1GEZ */
      N = sqrt (X);
    }
    break;
  case F_TAN_X_N:
    /* N = TAN( X ) */
    {
      if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_DEGREES)
      {
        X = FromDegreesToRadians (X);
      }
      else if (My->CurrentVersion->OptionFlags & OPTION_ANGLE_GRADIANS)
      {
        X = FromGradiansToRadians (X);
      }
      N = tan (X);
    }
    break;
  case F_TAND_X_N:
    /* N = TAND( X ) */
    {
      X = FromDegreesToRadians (X);
      N = tan (X);
    }
    break;
  case F_TANG_X_N:
    /* N = TANG( X ) */
    {
      X = FromGradiansToRadians (X);
      N = tan (X);
    }
    break;
  case F_SPC_X_S:
    /* S$ = SPC( X ) */
    {
      /* P1ANY */
      /* SPECIAL RULES APPLY.  PART OF PRINT COMMAND.  WIDTH > 0 */
      X = bwb_rint (X);
      if (X < 1 || X > 255)
      {
        if (WARN_OVERFLOW)
        {
          /* ERROR */
        }
        /* CONTINUE */
        X = 1;
      }
      x = (int) X;
      bwb_memset (S, ' ', x);
      s = x;
    }
    break;
  case F_TAB_X_S:
    /* S$ = TAB( X ) */
    {
      /* P1ANY */
      /* SPECIAL RULES APPLY.  PART OF PRINT COMMAND.  WIDTH > 0 */
      int w;
      int c;

      X = bwb_rint (X);
      if (X < 1 || X > 255)
      {
        if (WARN_OVERFLOW)
        {
          /* ERROR */
        }
        /* CONTINUE */
        X = 1;
      }
      x = (int) X;
      if (My->CurrentFile)
      {
        w = My->CurrentFile->width;
        c = My->CurrentFile->col;
      }
      else
      {
        w = My->SYSOUT->width;
        c = My->SYSOUT->col;
      }
      if (w > 0)
      {
        /* WIDTH 80 */
        while (x > w)
        {
          /*
           **
           ** If n is greater than the margin m, then n is
           ** reduced by an integral multiple of m so that it is
           ** in the range 1 <= n <= m; 
           **
           */
          x -= w;
        }
        /* 190 PRINT TAB(A);"X" ' A = 0 */
        if (x == 0)
        {
          /* use the value of one */
          x = 1;
          /* continue processing */
        }
      }
      if (x < c)
      {
        S[0] = '\n';
        s = 1;
        c = 1;
      }
      if (c < x)
      {
        x -= c;
        bwb_memset (&(S[s]), ' ', x);
        s += x;
      }
    }
    break;
  case F_POS_N:
    /* N = POS */
    {
      /* PNONE */
      N = My->SYSOUT->col;
    }
    break;
  case F_COUNT_N:
    /* N = COUNT */
    /* COUNT = POS - 1 */
    {
      /* PNONE */
      N = My->SYSOUT->col;
      N--;
    }
    break;
  case F_POS_X_N:
    /* N = POS( X ) */
    {
      /* P1INT */
      if (x == 0)
      {
        N = My->SYSOUT->col;
      }
      else if (x < 0)
      {
        N = My->SYSPRN->col;
      }
      else
      {
        FileType *F;
        F = find_file_by_number (x);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          N = F->col;
        }
      }
    }
    break;
  case F_INPUT4_X_Y_S:
    /* S$ = INPUT$( X, Y )  */
    {
      /* P1LEN|P2INT */
      if (y <= 0)
      {
        /* Printer and Console */
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (y);
        if (F == NULL)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          if ((F->DevMode & DEVMODE_READ) == 0)
          {
            WARN_ILLEGAL_FUNCTION_CALL;
          }
          else if (x == 0)
          {
            /* empty string */
          }
          else
          {
            FILE *fp;
            fp = F->cfp;
            if (fp == NULL)
            {
              WARN_ILLEGAL_FUNCTION_CALL;
            }
            else
            {
              s = fread (S, 1, x, fp);
              s = MAX (s, 0);        /* if( s < 0 ) s = 0; */
            }
          }
        }
      }
    }
    break;
  case F_ERROR_X_N:
    /* ERROR X */
    {
      /* P1BYT */
      bwx_Error (x, NULL);
      N = 0;
    }
    break;
  case F_ERROR_X_A_N:
    /* ERROR X, A$ */
    {
      /* P1BYT */
      bwx_Error (x, A);
      N = 0;
    }
    break;
  case F_ERR_N:
  case F_ERRN_N:
    /* N = ERR */
    /* N = ERRN */
    {
      /* PNONE */
      N = My->ERR;
    }
    break;
  case F_ERL_N:
  case F_ERRL_N:
    /* N = ERL */
    /* N = ERRL */
    {
      /* PNONE */
      if (My->ERL != NULL)
      {
        N = My->ERL->number;
      }
    }
    break;
  case F_ERR4_S:
  case F_ERROR4_S:
    /* S = ERR$ */
    /* S = ERROR$ */
    {
      /* PNONE */
      s = bwb_strlen (My->ERROR4);
      if (s > 0)
      {
        bwb_strcpy (S, My->ERROR4);
      }
    }
    break;


      /********************************************************************************************
      **  Keep the platform specific functions together.
      *********************************************************************************************/
  case F_INP_X_N:
  case F_PIN_X_N:
    /* N = INP( X ) */
    /* N = PIN( X ) */
    {
      /* P1BYT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_PDL_X_N:
    /* N = PDL( X ) */
    {
      /* P1BYT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_WAIT_X_Y_N:
    /* WAIT X, Y */
    {
      /* P1NUM|P2NUM */
      /* P1INT|P2BYT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_WAIT_X_Y_Z_N:
    /* WAIT X, Y, Z */
    {
      /* P1NUM|P2NUM|P3NUM */
      /* P1INT|P2BYT|P3BYT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_OUT_X_Y_N:
    /* OUT X, Y */
    {
      /* P1NUM|P2NUM */
      /* P1INT|P2BYT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_PEEK_X_N:
  case F_EXAM_X_N:
  case F_FETCH_X_N:
  case F_DPEEK_X_N:
    /* N = PEEK( X ) */
    /* N = EXAM( X ) */
    /* N = FETCH( X ) */
    /* N = DPEEK( X ) */
    {
      /* P1INT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_POKE_X_Y_N:
  case F_FILL_X_Y_N:
  case F_STUFF_X_Y_N:
  case F_DPOKE_X_Y_N:
    /* POKE X, Y */
    /* FILL X, Y */
    /* STUFF X, Y */
    /* DPOKE X, Y */
    {
      /* P1NUM|P2NUM */
      /* P1INT|P2BYT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_LOCK_X_N:
    /* LOCK X  */
    {
      /* P1INT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_UNLOCK_X_N:
    /* UNLOCK X  */
    {
      /* P1INT */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_USR_N:
  case F_USR0_N:
  case F_USR1_N:
  case F_USR2_N:
  case F_USR3_N:
  case F_USR4_N:
  case F_USR5_N:
  case F_USR6_N:
  case F_USR7_N:
  case F_USR8_N:
  case F_USR9_N:
  case F_EXF_N:
  case F_UUF_N:
    /* N = USR( ... ) */
    /* N = USR0( ... ) */
    /* N = USR1( ... ) */
    /* N = USR2( ... ) */
    /* N = USR3( ... ) */
    /* N = USR4( ... ) */
    /* N = USR5( ... ) */
    /* N = USR6( ... ) */
    /* N = USR7( ... ) */
    /* N = USR8( ... ) */
    /* N = USR9( ... ) */
    /* N = EXF( ... ) */
    /* N = UUF( ... ) */
    {
      /* ... */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_VARPTR_N:
  case F_NAME_N:
  case F_PTR_N:
    /* N = VARPTR( ... ) */
    /* N = NAME( ... ) */
    /* N = PTR( ... ) */
    {
      /* ... */
      WARN_ADVANCED_FEATURE;
    }
    break;
  case F_FRE_N:
  case F_FRE_X_N:
  case F_FRE_A_N:
  case F_FREE_N:
  case F_FREE_X_N:
  case F_FREE_A_N:
  case F_MEM_N:
  case F_TOP_N:
    /* N = FRE(    ) */
    /* N = FRE( X  ) */
    /* N = FRE( X$ ) */
    /* N = FREE(    ) */
    /* N = FREE( X  ) */
    /* N = FREE( X$ ) */
    /* N = MEM(    ) */
    /* N = TOP(    ) */
    {
      N = 32000;                /* reasonable value */
    }
    break;
  case F_CLS_N:
  case F_HOME_N:
    /* CLS */
    /* HOME */
    {
      /* PNONE */
      bwx_CLS ();
    }
    break;
  case F_LOCATE_X_Y_N:
    /* LOCATE X, Y */
    {
      /* P1NUM|P2NUM */
      /* P1BYT|P2BYT */
      bwx_LOCATE (x, y);
    }
    break;
  case F_CUR_X_Y_S:
    /* CUR X, Y */
    {
      /* P1NUM|P2NUM */
      /* P1BYT|P2BYT */
      x++;                        /* 0-based to 1-based row */
      y++;                        /* 0-based to 1-based col */
      bwx_LOCATE (x, y);
      s = 0;
    }
    break;
  case F_VTAB_X_N:
    /* VTAB X  */
    {
      /* P1BYT */
      /* X is 1-based row */
      /* col is 1 */
      bwx_LOCATE (x, 1);
    }
    break;
  case F_COLOR_X_Y_N:
    /* COLOR X, Y */
    {
      /* P1NUM|P2NUM */
      /* P1BYT|P2BYT */
      /* X is Foreground color */
      /* Y is Background color */
      bwx_COLOR (X, Y);
    }
    break;
  case F_SHELL_A_N:
  case F_EXEC_A_N:
    /* N = SHELL( A$ ) */
    /* N = EXEC( A$ ) */
    {
      /* P1BYT */
      N = system (A);
    }
    break;
  case F_FILES_N:
  case F_CATALOG_N:
    /* FILES */
    /* CATALOG */
    {
      /* PNONE */
      if (is_empty_string (My->OptionFilesString))
      {
        WARN_ADVANCED_FEATURE;
      }
      else
      {
        N = system (My->OptionFilesString);
      }
    }
    break;
  case F_FILES_A_N:
  case F_CATALOG_A_N:
    /* FILES A$ */
    /* CATALOG A$ */
    {
      /* P1BYT */
      if (is_empty_string (My->OptionFilesString))
      {
        WARN_ADVANCED_FEATURE;
      }
      else
      {
        size_t n;
        char *Buffer;

        n = bwb_strlen (My->OptionFilesString) + 1 /* SpaceChar */  + a;
        if ((Buffer =
             (char *) calloc (n + 1 /* NulChar */ , sizeof (char))) == NULL)
        {
          WARN_OUT_OF_MEMORY;
        }
        else
        {
          bwb_strcpy (Buffer, My->OptionFilesString);
          bwb_strcat (Buffer, " ");
          bwb_strcat (Buffer, A);
          N = system (Buffer);
          free (Buffer);
          Buffer = NULL;
        }
      }
    }
    break;
  case F_CHDIR_A_N:
    /* CHDIR A$ */
    {
      /* P1BYT */
#if DIRECTORY_CMDS
      N = chdir (A);
#else
      WARN_ADVANCED_FEATURE;
#endif
    }
    break;
  case F_MKDIR_A_N:
    /* MKDIR A$ */
    {
      /* P1BYT */
#if DIRECTORY_CMDS
#if MKDIR_ONE_ARG
      N = mkdir (A);
#else
      N = mkdir (A, PERMISSIONS);
#endif
#else
      WARN_ADVANCED_FEATURE;
#endif
    }
    break;
  case F_RMDIR_A_N:
    /* RMDIR A$ */
    {
      /* P1BYT */
#if DIRECTORY_CMDS
      N = rmdir (A);
#else
      WARN_ADVANCED_FEATURE;
#endif
    }
    break;
  case F_KILL_A_N:
  case F_UNSAVE_A_N:
    /* KILL A$ */
    /* UNSAVE A$ */
    {
      /* P1BYT */
      N = remove (A);
    }
    break;
  case F_NAME_A_B_N:
    /* NAME A$ AS B$      */
    /* N = NAME( A$, B$ ) */
    {
      /* P1BYT|P2BYT */
      N = rename (A, B);
    }
    break;
  case F_INPUT4_X_S:
    /* S$ = INPUT$( X )  */
    {
      /* P1LEN */
      if (x == 0)
      {
        /* empty string */
      }
      else
      {
        for (s = 0; s < x; s++)
        {
          int c;
          c = fgetc (My->SYSIN->cfp);
          if ((c == EOF) || (c == '\n') || (c == '\r'))
          {
            break;
          }
          S[s] = c;
        }
        S[s] = 0;
      }
    }
    break;
  case F_INKEY4_S:
  case F_KEY4_S:
  case F_KEY_S:
  case F_INCH4_S:
    /* S$ = INKEY$ */
    /* S$ = KEY$ */
    /* S$ = KEY */
    /* S$ = INCH$ */
    {
      /* PNONE */
      int c;

      c = fgetc (My->SYSIN->cfp);
      if (c < MINBYT || c > MAXBYT)
      {
        /* EOF */
      }
      else
      {
        S[s] = c;
        s++;
      }
      S[s] = 0;
    }
    break;
  case F_NULL_X_N:
    /* NULL X */
    {
      /* P1NUM */
      /* P1BYT */
      My->LPRINT_NULLS = x;
      N = 0;
    }
    break;
  case F_LWIDTH_X_N:
    /* LWIDTH X */
    {
      /* P1NUM */
      /* P1BYT */
      My->SYSPRN->width = x;
      My->SYSPRN->col = 1;
      N = 0;
    }
    break;
  case F_LPOS_N:
    /* N = LPOS */
    {
      /* PNONE */
      /* PNONE */
      N = My->SYSPRN->col;
    }
    break;
  case F_TRON_N:
  case F_TRACE_N:
  case F_FLOW_N:
    /* TRON */
    /* TRACE */
    /* FLOW */
    {
      /* PNONE */
      fprintf (My->SYSOUT->cfp, "Trace is ON\n");
      ResetConsoleColumn ();
      My->IsTraceOn = TRUE;
      N = 0;
    }
    break;
  case F_TROFF_N:
  case F_NOTRACE_N:
  case F_NOFLOW_N:
    /* TROFF */
    /* NOTRACE */
    /* NOFLOW */
    {
      /* PNONE */
      fprintf (My->SYSOUT->cfp, "Trace is OFF\n");
      ResetConsoleColumn ();
      My->IsTraceOn = FALSE;
      N = 0;
    }
    break;
  case F_TRACE_X_N:
    /* TRACE X */
    {
      /* P1BYTE */
      if (x == 0)
      {
        fprintf (My->SYSOUT->cfp, "Trace is OFF\n");
        ResetConsoleColumn ();
        My->IsTraceOn = FALSE;
      }
      else
      {
        fprintf (My->SYSOUT->cfp, "Trace is ON\n");
        ResetConsoleColumn ();
        My->IsTraceOn = TRUE;
      }
      N = 0;
    }
    break;
  case F_RANDOMIZE_N:
  case F_RAN_N:
  case F_RANDOM_N:
    /* RANDOMIZE */
    /* RAN */
    /* RANDOM */
    {
      /* PNONE */
      /* USE THE CURRENT TIME AS THE SEED */
      time (&t);
      lt = localtime (&t);
      x = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
      srand (x);
      N = 0;
    }
    break;
  case F_RANDOMIZE_X_N:
  case F_RAN_X_N:
  case F_RANDOM_X_N:
    /* RANDOMIZE X */
    /* RAN X */
    /* RANDOM X */
    {
      /* P1NUM */
      /* P1ANY */
      /* USE 'X' AS THE SEED */
      x = (int) bwb_rint (X);
      srand (x);
      N = 0;
    }
    break;
  case F_LNO_X_N:
    /* N = LNO( X, Y ) */
    {
      /* P1NUM */
      /* P1ANY */
      N = X;
    }
    break;
  case F_PAD_X_N:
  case F_SEG_X_N:
    /* N = PAD( X ) */
    /* N = SEG( X ) */
    {
      /* P1NUM */
      /* P1ANY */
      N = 0;
    }
    break;
  case F_CNTRL_X_Y_N:
    /* N = CNTRL( X, Y ) */
    {
      /* P1NUM | P2NUM */
      /* P1INT | P2INT */
      switch (x)
      {
      case 0:
        /*
           CNTRL 0,line
           This specifies a line to go to when the user presses Ctl-B.
         */
        break;
      case 1:
        /*
           CNTRL 1,value
           This sets the number of digits (1 to 6) to print
         */
        if (y == 0)
        {
          /* default */
          y = SIGNIFICANT_DIGITS;
        }
        if (y < MINIMUM_DIGITS || y > MAXIMUM_DIGITS)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          My->OptionDigitsInteger = y;
        }
        break;
      case 2:
        /*
           CNTRL 2,value
           This controls the front panel LED display.
         */
        break;
      case 3:
        /*
           CNTRL 3,value
           This command sets the width of the print zones.
         */
        if (y == 0)
        {
          /* default */
          y = ZONE_WIDTH;
        }
        if (y < MINIMUM_ZONE || y > MAXIMUM_ZONE)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
        }
        else
        {
          My->OptionZoneInteger = y;
        }
        break;
      case 4:
        /*
           CNTRL 4,value
           This command is used to load and unload the main HDOS overlay.
         */
        break;
      default:
        WARN_ILLEGAL_FUNCTION_CALL;
        break;
      }
      N = 0;
    }
    break;
  case F_ZONE_X_N:
    /* N = ZONE( X ) */
    {
      /* P1NUM */
      /* P1INT */
      if (x == 0)
      {
        /* default */
        x = ZONE_WIDTH;
      }
      if (x < MINIMUM_ZONE || x > MAXIMUM_ZONE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        My->OptionZoneInteger = x;
      }
    }
    break;
  case F_ZONE_X_Y_N:
    /* N = ZONE( X, Y ) */
    {
      /* P1NUM | P2NUM */
      /* P1INT | P2INT */
      /* value of X is ignored */
      if (y == 0)
      {
        /* default */
        y = ZONE_WIDTH;
      }
      if (y < MINIMUM_ZONE || y > MAXIMUM_ZONE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
      }
      else
      {
        My->OptionZoneInteger = y;
      }
    }
    break;
  case F_CIN_X_N:
    /* N = CIN( X ) */
    {
      /* P1INT */
      if (x <= 0)
      {
        /* Printer and Console */
        N = -1;
      }
      else
      {
        FileType *F;

        F = find_file_by_number (x);
        if (F == NULL)
        {
          N = -1;
        }
        else if (F->DevMode & DEVMODE_READ)
        {
          N = fgetc (F->cfp);
        }
        else
        {
          N = -1;
        }
      }
    }
    break;
  case F_TRUE_N:
    /* N = TRUE */
    {
      /* PNONE */
      N = TRUE;
    }
    break;
  case F_FALSE_N:
    /* N = FALSE */
    {
      /* PNONE */
      N = FALSE;
    }
    break;
  default:
    {
      /* an unknown function code */
      WARN_INTERNAL_ERROR;
    }
  }
  /* sanity check */
  if (f->ReturnTypeCode == StringTypeCode)
  {
    /* STRING */
    if ( /* s < 0 || */ s > MAXLEN)
    {
      WARN_INTERNAL_ERROR;
      s = 0;
    }
    if (S != RESULT_BUFFER)
    {
      WARN_INTERNAL_ERROR;
      S = RESULT_BUFFER;
    }
    RESULT_LENGTH = s;
    RESULT_BUFFER[RESULT_LENGTH] = NulChar;
  }
  else
  {
    /* NUMBER */
    if (isnan (N))
    {
      /* ERROR */
      /* this means the parameters were not properly checked */
      WARN_INTERNAL_ERROR;
      N = 0;
    }
    else if (isinf (N))
    {
      /* Evaluation of an expression results in an
       * overflow (nonfatal, the recommended
       * recovery procedure is to supply machine
       * in- finity with the algebraically correct
       * sign and continue). */
      if (N < 0)
      {
        N = MINDBL;
      }
      else
      {
        N = MAXDBL;
      }
      WARN_OVERFLOW;
    }
    RESULT_NUMBER = N;
  }
  return argv;                        /* released by exp_function() in bwb_elx.c */
}

/* EOF */
