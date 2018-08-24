/***************************************************************
  
        bwb_var.c       Variable-Handling Routines
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


/* Prototypes for functions visible to this file only */

static void clear_virtual (VirtualType * Z);
static void clear_virtual_by_variable (VariableType * Variable);
static int dim_check (VariableType * variable);
static size_t dim_unit (VariableType * v, int *pp);
static LineType *dio_lrset (LineType * l, int rset);
static void field_clear (FieldType * Field);
static FieldType *field_new (void);
static VirtualType *find_virtual_by_variable (VariableType * Variable);
static LineType *internal_swap (LineType * l);
static VariableType *mat_islocal (char *buffer);
static VirtualType *new_virtual (void);
static int var_defx (LineType * l, int TypeCode);
static VariableType *var_islocal (char *buffer, int dimensions);
static void var_link_new_variable (VariableType * v);

extern int
var_init (void)
{
  assert( My != NULL );

  My->VariableHead = NULL;

  return TRUE;
}

extern LineType *
bwb_COMMON (LineType * l)
{
  /*
     SYNTAX: COMMON scalar
     SYNTAX: COMMON matrix( dimnesions ) ' COMMON A(1), B(2), C(3)
     SYNTAX: COMMON matrix( [, [,]] )    ' COMMON A(), B(,), C(,,)
   */
   
  assert (l != NULL);

  do
  {
    int dimensions;
    VariableType *variable;
    char varname[NameLengthMax + 1];

    dimensions = 0;
    /* get variable name and find variable */
    if (line_read_varname (l, varname) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_skip_LparenChar (l))
    {
      line_skip_spaces (l);        /* keep this */
      if (bwb_isdigit (l->buffer[l->position]))
      {
        /* COMMON A(3) : DIM A( 5, 10, 20 ) */
        if (line_read_integer_expression (l, &dimensions) == FALSE)
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
      }
      else
      {
        /* COMMON A(,,) : DIM A( 5, 10, 20 ) */
        dimensions++;
        while (line_skip_seperator (l));
        {
          dimensions++;
        }
      }
      if (line_skip_RparenChar (l) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    if ((variable = var_find (varname, dimensions, TRUE)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }

    /* mark as COMMON */
    variable->VariableFlags |= VARIABLE_COMMON;
  }
  while (line_skip_seperator (l));

  return (l);
}

extern LineType *
bwb_ERASE (LineType * l)
{
  /*
     SYNTAX:     ERASE variable [, ...] ' ERASE A, B, C
   */
   
  assert (l != NULL);
  assert( My != NULL );

  do
  {
    char varname[NameLengthMax + 1];

    /* get variable name and find variable */

    if (line_read_varname (l, varname))
    {
      /* erase all matching SCALAR and ARRAY variables */
      int dimensions;

      for (dimensions = 0; dimensions < MAX_DIMS; dimensions++)
      {
        VariableType *variable;

        variable = var_find (varname, dimensions, FALSE);
        if (variable != NULL)
        {
          /* found a variable */
          VariableType *p;        /* previous variable in linked list */

          /* find then previous variable in chain */
          if (variable == My->VariableHead)
          {
            /* free head */
            My->VariableHead = variable->next;
            variable->next = NULL;
            var_free (variable);
          }
          else
          {
            /* free tail */
            for (p = My->VariableHead; p != NULL && p->next != variable;
                 p = p->next)
            {
              ;
            }
            if (p == NULL)
            {
              /* this should never happen */
              WARN_INTERNAL_ERROR;
              return NULL;
            }
            if (p->next != variable)
            {
              /* this should never happen */
              WARN_INTERNAL_ERROR;
              return NULL;
            }
            /* reassign linkage */
            p->next = variable->next;
            variable->next = NULL;
            var_free (variable);
          }
        }
      }
    }
  }
  while (line_skip_seperator (l));
  return (l);
}

static LineType *
internal_swap (LineType * l)
{
  VariableType *lhs;
  VariableType *rhs;
   
  assert (l != NULL);

  if (line_skip_LparenChar (l))
  {
    /* optional */
  }

  /* get left variable */
  if ((lhs = line_read_scalar (l)) == NULL)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  /* get required comma */
  if (line_skip_seperator (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  /* get right variable */
  if ((rhs = line_read_scalar (l)) == NULL)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (line_skip_RparenChar (l))
  {
    /* optional */
  }

  /* check to be sure that both variables are compatible */
  if (VAR_IS_STRING (rhs) != VAR_IS_STRING (lhs))
  {
    WARN_TYPE_MISMATCH;
    return (l);
  }

  /* swap the values */
  {
    VariantType L;
    VariantType R;
    CLEAR_VARIANT (&L);
    CLEAR_VARIANT (&R);

    if (var_get (lhs, &L) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (var_get (rhs, &R) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    if (var_set (lhs, &R) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (var_set (rhs, &L) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  /* return */
  return (l);
}

extern LineType *
bwb_EXCHANGE (LineType * l)
{
  /*
     SYNTAX:     EXCHANGE   variable, variable
     SYNTAX:     EXCHANGE ( variable, variable )
   */
   
  assert (l != NULL);
  return internal_swap (l);
}



extern LineType *
bwb_SWAP (LineType * l)
{
  /*
     SYNTAX:     SWAP   variable, variable
     SYNTAX:     SWAP ( variable, variable )
   */
   
  assert (l != NULL);
  return internal_swap (l);
}

extern VariableType *
var_free (VariableType * variable)
{
  /* 
     Release all the memory associated with a specific variable.
     This function returns NULL, so you can use it like this:
     variable = var_new(...);
     ...
     variable = var_free( variable ); 
   */
   

  if (variable != NULL)
  {
    if (variable->next != NULL)
    {
      /* This allows variable chains to be easily released. */
      variable->next = var_free (variable->next);
    }
    /* cleanup this variable */
    field_free_variable (variable);
    clear_virtual_by_variable (variable);
    if (VAR_IS_STRING (variable))
    {
      if (variable->Value.String != NULL)
      {
        int j;
        for (j = 0; j < variable->array_units; j++)
        {
          if (variable->Value.String[j].sbuffer != NULL)
          {
            free (variable->Value.String[j].sbuffer);
          }
          variable->Value.String[j].length = 0;
        }
        free (variable->Value.String);
        variable->Value.String = NULL;
      }
    }
    else
    {
      if (variable->Value.Number != NULL)
      {
        free (variable->Value.Number);
        variable->Value.Number = NULL;
      }
    }
    free (variable);
  }
  return NULL;
}

extern void
var_CLEAR (void)
{
  /*
     free all variables except PRESET
   */
  VariableType *variable;
  assert( My != NULL );
   

  for (variable = My->VariableHead; variable != NULL;)
  {
    if (variable->VariableFlags & VARIABLE_PRESET)
    {
      /* keep */
      variable = variable->next;
    }
    else if (variable == My->VariableHead)
    {
      /* free head */
      My->VariableHead = variable->next;
      variable->next = NULL;
      var_free (variable);
      variable = My->VariableHead;
    }
    else
    {
      /* free tail */
      VariableType *z;
      z = variable->next;
      variable->next = NULL;
      var_free (variable);
      variable = z;
    }
  }
}

extern LineType *
bwb_CLEAR (LineType * l)
{
  /*
     SYNTAX:     CLEAR
   */
   
  assert (l != NULL);
  var_CLEAR ();
  line_skip_eol (l);
  return (l);
}


LineType *
bwb_CLR (LineType * l)
{
   
  assert (l != NULL);
  return bwb_CLEAR (l);
}

/***********************************************************
  
   FUNCTION:       var_delcvars()
  
   DESCRIPTION:    This function deletes all variables
         in memory except those previously marked
         as common.
  
***********************************************************/

int
var_delcvars (void)
{
  VariableType *v;
   
  assert( My != NULL );

  for (v = My->VariableHead; v != NULL;)
  {
    if (v->VariableFlags & VARIABLE_PRESET)
    {
      /* keep */
      v = v->next;
    }
    else if (v->VariableFlags & VARIABLE_COMMON)
    {
      /* keep */
      v = v->next;
    }
    else if (v == My->VariableHead)
    {
      /* free head */
      My->VariableHead = v->next;
      v->next = NULL;
      var_free (v);
      v = My->VariableHead;
    }
    else
    {
      /* free tail */
      VariableType *z;                /* next variable */

      z = v->next;
      v->next = NULL;
      var_free (v);
      v = z;
    }
  }
  return TRUE;
}

/***********************************************************
  
        FUNCTION: bwb_mid()
  
   DESCRIPTION:    This function implements the BASIC
         MID$ command.
  
         Same as MID$ function, except it will set
         the desired substring and not return its
         value.  Added by JBV 10/95
  
   SYNTAX:     MID$( string-variable$, start-position-in-string
         [, number-of-spaces ] ) = expression
  
***********************************************************/

LineType *
bwb_MID4 (LineType * l)
{
  /* MID$( target$, start% [ , length% ] ) = source$ */
  VariableType *variable;
  VariantType target;
  int start;
  int length;
  VariantType source;
  int maxlen;
   
  assert (l != NULL);

  CLEAR_VARIANT (&source);
  CLEAR_VARIANT (&target);
  start = 0;
  length = 0;
  maxlen = 0;
  if (line_skip_LparenChar (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if ((variable = line_read_scalar (l)) == NULL)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (VAR_IS_STRING (variable))
  {
    /* OK */
  }
  else
  {
    /* ERROR */
    WARN_TYPE_MISMATCH;
    return (l);
  }
  if (var_get (variable, &target) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (target.VariantTypeCode != StringTypeCode)
  {
    WARN_TYPE_MISMATCH;
    return (l);
  }
  if (line_skip_seperator (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_integer_expression (l, &start) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (start < 1)
  {
    WARN_ILLEGAL_FUNCTION_CALL;
    return (l);
  }
  if (start > target.Length)
  {
    WARN_ILLEGAL_FUNCTION_CALL;
    return (l);
  }
  maxlen = 1 + target.Length - start;
  if (line_skip_seperator (l))
  {
    if (line_read_integer_expression (l, &length) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (length < 0)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
  }
  else
  {
    length = -1;                /* MAGIC */
  }
  if (line_skip_RparenChar (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* skip the equal sign */
  if (line_skip_EqualChar (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_expression (l, &source) == FALSE)        /* bwb_MID4 */
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (source.VariantTypeCode != StringTypeCode)
  {
    WARN_TYPE_MISMATCH;
    return (l);
  }
  if (length == -1 /* MAGIC */ )
  {
    length = source.Length;
  }
  length = MIN (length, maxlen);
  length = MIN (length, source.Length);
  if (length < 0)
  {
    WARN_INTERNAL_ERROR;
    return (l);
  }
  if (length > 0)
  {
    int i;

    start--;                        /* BASIC to C */
    for (i = 0; i < length; i++)
    {
      target.Buffer[start + i] = source.Buffer[i];
    }
    target.Buffer[target.Length] = NulChar;
    if (var_set (variable, &target) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  RELEASE_VARIANT (&source);
  RELEASE_VARIANT (&target);
  return (l);
}


/***********************************************************
  
        FUNCTION: bwb_ddbl()
  
   DESCRIPTION:    This function implements the BASIC
         DEFDBL command.
  
   SYNTAX:     DEFDBL letter[-letter](, letter[-letter])...
  
***********************************************************/

LineType *
bwb_DEFBYT (LineType * l)
{
  /*
     DEFBYT letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  var_defx (l, ByteTypeCode);
  return (l);
}

LineType *
bwb_DEFCUR (LineType * l)
{
  /*
     DEFCUR letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  var_defx (l, CurrencyTypeCode);
  return (l);
}

LineType *
bwb_DEFDBL (LineType * l)
{
  /*
     DEFDBL letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  var_defx (l, DoubleTypeCode);
  return (l);
}

/***********************************************************
  
        FUNCTION: bwb_dint()
  
   DESCRIPTION:    This function implements the BASIC
         DEFINT command.
  
   SYNTAX:     DEFINT letter[-letter](, letter[-letter])...
  
***********************************************************/

LineType *
bwb_DEFINT (LineType * l)
{
  /*
     DEFINT letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  var_defx (l, IntegerTypeCode);
  return (l);
}

LineType *
bwb_DEFLNG (LineType * l)
{
  /*
     DEFLNG letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  var_defx (l, LongTypeCode);
  return (l);
}

/***********************************************************
  
        FUNCTION: bwb_dsng()
  
   DESCRIPTION:    This function implements the BASIC
         DEFSNG command.
  
   SYNTAX:     DEFSNG letter[-letter](, letter[-letter])...
  
***********************************************************/

LineType *
bwb_DEFSNG (LineType * l)
{
  /*
     DEFSNG letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  var_defx (l, SingleTypeCode);
  return (l);
}

/***********************************************************
  
        FUNCTION: bwb_dstr()
  
   DESCRIPTION:    This function implements the BASIC
         DEFSTR command.
  
   SYNTAX:     DEFSTR letter[-letter](, letter[-letter])...
  
***********************************************************/

LineType *
bwb_DEFSTR (LineType * l)
{
  /*
     DEFSTR letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  var_defx (l, StringTypeCode);
  return (l);
}

LineType *
bwb_TEXT (LineType * l)
{
  /*
     TEXT letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  var_defx (l, StringTypeCode);
  return (l);
}

LineType *
bwb_TRACE (LineType * l)
{
  assert (l != NULL);

  return bwb_TRACE_ON(l);
}

LineType *
bwb_TRACE_ON (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );

  fprintf (My->SYSOUT->cfp, "Trace is ON\n");
  ResetConsoleColumn ();
  My->IsTraceOn = TRUE;

  return (l);
}

LineType *
bwb_TRACE_OFF (LineType * l)
{
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );

  fprintf (My->SYSOUT->cfp, "Trace is OFF\n");
  ResetConsoleColumn ();
  My->IsTraceOn = FALSE;

  return (l);
}

int
VarTypeIndex (char C)
{
   
  switch (C)
  {
  case 'A':
    return 0;
  case 'B':
    return 1;
  case 'C':
    return 2;
  case 'D':
    return 3;
  case 'E':
    return 4;
  case 'F':
    return 5;
  case 'G':
    return 6;
  case 'H':
    return 7;
  case 'I':
    return 8;
  case 'J':
    return 9;
  case 'K':
    return 10;
  case 'L':
    return 11;
  case 'M':
    return 12;
  case 'N':
    return 13;
  case 'O':
    return 14;
  case 'P':
    return 15;
  case 'Q':
    return 16;
  case 'R':
    return 17;
  case 'S':
    return 18;
  case 'T':
    return 19;
  case 'U':
    return 20;
  case 'V':
    return 21;
  case 'W':
    return 22;
  case 'X':
    return 23;
  case 'Y':
    return 24;
  case 'Z':
    return 25;
  case 'a':
    return 0;
  case 'b':
    return 1;
  case 'c':
    return 2;
  case 'd':
    return 3;
  case 'e':
    return 4;
  case 'f':
    return 5;
  case 'g':
    return 6;
  case 'h':
    return 7;
  case 'i':
    return 8;
  case 'j':
    return 9;
  case 'k':
    return 10;
  case 'l':
    return 11;
  case 'm':
    return 12;
  case 'n':
    return 13;
  case 'o':
    return 14;
  case 'p':
    return 15;
  case 'q':
    return 16;
  case 'r':
    return 17;
  case 's':
    return 18;
  case 't':
    return 19;
  case 'u':
    return 20;
  case 'v':
    return 21;
  case 'w':
    return 22;
  case 'x':
    return 23;
  case 'y':
    return 24;
  case 'z':
    return 25;
  }
  return -1;
}

/***********************************************************
  
        Function: var_defx()
  
   DESCRIPTION:    This function is a generalized DEFxxx handler.
  
***********************************************************/

static int
var_defx (LineType * l, int TypeCode)
{
  /*
     DEFxxx letter[-letter](, letter[-letter])...
   */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->DefaultVariableType != NULL );

  do
  {
    char firstc;
    char lastc;
    int first;
    int last;
    int c;

    /* find a sequence of letters for variables */
    if (line_read_letter_sequence (l, &firstc, &lastc) == FALSE)
    {
      /* DEFINT 0-9 */
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    first = VarTypeIndex (firstc);
    if (first < 0)
    {
      /* DEFINT 0-Z */
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    last = VarTypeIndex (lastc);
    if (last < 0)
    {
      /* DEFINT A-9 */
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    if (first > last)
    {
      /* DEFINT Z-A */
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    for (c = first; c <= last; c++)
    {
      My->DefaultVariableType[c] = TypeCode;        /* var_defx */
    }
  }
  while (line_skip_seperator (l));

  return TRUE;

}

/***************************************************************
  
        FUNCTION:       var_find()
  
   DESCRIPTION:    This C function attempts to find a variable
         name matching the argument in buffer. If
         it fails to find a matching name, it
         sets up a new variable with that name.
  
***************************************************************/

VariableType *
mat_find (char *name)
{
  /*
     similar to var_find, but returns the first matrix found
   */
  VariableType *v;
  assert( My != NULL );
   

  /* check for NULL variable name */
  if (name == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (is_empty_string (name))
  {
    WARN_SYNTAX_ERROR;
    return NULL;
  }
  /* check for a local variable at this EXEC level */

  v = mat_islocal (name);
  if (v != NULL)
  {
    return v;
  }
  /* now run through the global variable list and try to find a match */
  for (v = My->VariableHead; v != NULL; v = v->next)
  {
    assert( v != NULL );
    if (v->dimensions > 0)
    {
      if (bwb_stricmp (v->name, name) == 0)
      {
        return v;
      }
    }
  }
  return NULL;
}

VariableType *
var_find (char *name, int dimensions, int IsImplicit)
{
  VariableType *v;
  int n;
   
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  assert( My->DefaultVariableType != NULL );

  /* check for NULL variable name */
  if (name == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (is_empty_string (name))
  {
    WARN_SYNTAX_ERROR;
    return NULL;
  }
  if (dimensions < 0)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }

  /* check for a local variable at this EXEC level */

  v = var_islocal (name, dimensions);
  if (v != NULL)
  {
    return v;
  }
  /* now run through the global variable list and try to find a match */
  for (v = My->VariableHead; v != NULL; v = v->next)
  {
    assert( v != NULL );
    if (v->dimensions == dimensions)
    {
      if (bwb_stricmp (v->name, name) == 0)
      {
        return v;
      }
    }
  }
  if (IsImplicit == FALSE)
  {
    return NULL;
  }
  if (My->CurrentVersion->OptionFlags & OPTION_EXPLICIT_ON)
  {
    /* NO implicit creation - all variables must be created via DIM */
    WARN_VARIABLE_NOT_DECLARED;
    return NULL;
  }
  if (My->CurrentVersion->OptionFlags & OPTION_STRICT_ON)
  {
    if (dimensions > 0)
    {
      /* Implicit ARRAY is not allowed */
      WARN_VARIABLE_NOT_DECLARED;
      return NULL;
    }
  }

  /* this is a IMPLICIT variable, so initialize it... */

  /* initialize new variable */
  if ((v = (VariableType *) calloc (1, sizeof (VariableType))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return NULL;
  }

  /* copy the name into the appropriate structure */

  assert( v != NULL );
  bwb_strcpy (v->name, name);

  /* determine variable TypeCode */
  v->VariableTypeCode = var_nametype (name);
  if (v->VariableTypeCode == NulChar)
  {
    /* variable name has no declared TypeCode */
    n = VarTypeIndex (name[0]);
    if (n < 0)
    {
      v->VariableTypeCode = DoubleTypeCode;        /* default */
    }
    else
    {
      v->VariableTypeCode = My->DefaultVariableType[n];
    }
  }
  v->VariableFlags = 0;
  v->dimensions = dimensions;
  v->array_units = 1;
  for (n = 0; n < v->dimensions; n++)
  {
    v->LBOUND[n] = My->CurrentVersion->OptionBaseInteger;        /* implicit lower bound */
    v->UBOUND[n] = 10;                /* implicit upper bound */
    if (v->UBOUND[n] < v->LBOUND[n])
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return NULL;
    }
    v->VINDEX[n] = v->LBOUND[n];
    v->array_units *= v->UBOUND[n] - v->LBOUND[n] + 1;
  }

  /* assign array memory */
  if (VAR_IS_STRING (v))
  {
    if ((v->Value.String =
         (StringType *) calloc (v->array_units, sizeof (StringType))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return NULL;
    }
  }
  else
  {
    if ((v->Value.Number =
         (DoubleType *) calloc (v->array_units, sizeof (DoubleType))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return NULL;
    }
  }

  /* insert variable at the beginning of the variable chain */
  v->next = My->VariableHead;
  My->VariableHead = v;
  return v;
}

/***************************************************************
  
        FUNCTION:       var_new()
  
   DESCRIPTION:    This function assigns memory for a new variable.
  
***************************************************************/

VariableType *
var_new (char *name, char TypeCode)
{
  VariableType *v;
   

  /* get memory for new variable */

  if (name == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (is_empty_string (name))
  {
    WARN_SYNTAX_ERROR;
    return NULL;
  }
  if ((v = (VariableType *) calloc (1, sizeof (VariableType))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return NULL;
  }
  /* copy the name into the appropriate structure */

  assert( v != NULL );
  bwb_strcpy (v->name, name);

  /* set memory in the new variable */
  var_make (v, TypeCode);

  /* and return */

  return v;

}


/***************************************************************
  
   FUNCTION:   bwb_dim()
  
   DESCRIPTION:   This function implements the BASIC DIM
         statement, allocating memory for a
         dimensioned array of variables.
  
   SYNTAX:     DIM variable(elements...)[,variable(elements...)]
  
***************************************************************/

static void
var_link_new_variable (VariableType * v)
{
  /* 
     We are called by DIM, so this is an explicitly created variable.
     There are only two possibilities:
     1.  We are a LOCAL variable of a SUB or FUNCTION.
     2.  We are a GLOBAL variable.
   */
   
  assert (v != NULL);
  assert( My != NULL );

  if (My->StackHead != NULL)
  {
    StackType *StackItem;
    for (StackItem = My->StackHead; StackItem != NULL;
         StackItem = StackItem->next)
    {
      if (StackItem->LoopTopLine != NULL)
      {
        switch (StackItem->LoopTopLine->cmdnum)
        {
        case C_FUNCTION:
        case C_SUB:
          /* we have found a FUNCTION or SUB boundary, must be LOCAL */
          v->next = StackItem->local_variable;
          StackItem->local_variable = v;
          return;
          /* break; */
        }
      }
    }
  }
  /* no FUNCTION or SUB on the stack, must be GLOBAL */
  v->next = My->VariableHead;
  My->VariableHead = v;
}


static VirtualType *
new_virtual (void)
{
  VirtualType *Z;
  assert( My != NULL );
   

  /* look for an empty slot */
  for (Z = My->VirtualHead; Z != NULL; Z = Z->next)
  {
    if (Z->Variable == NULL)
    {
      /* FOUND */
      return Z;
    }
  }
  /* NOT FOUND */
  if ((Z = (VirtualType *) calloc (1, sizeof (VirtualType))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return NULL;
  }
  Z->next = My->VirtualHead;
  My->VirtualHead = Z;
  return Z;
}
static void
clear_virtual (VirtualType * Z)
{
   
  assert (Z != NULL);

  Z->Variable = NULL;
  Z->FileNumber = 0;
  Z->FileOffset = 0;
  Z->FileLength = 0;
}
static void
clear_virtual_by_variable (VariableType * Variable)
{
  VirtualType *Z;
   
  assert (Variable != NULL);
  assert( My != NULL );

  for (Z = My->VirtualHead; Z != NULL; Z = Z->next)
  {
    if (Z->Variable == Variable)
    {
      /* FOUND */
      clear_virtual (Z);
    }
  }
}
extern void
clear_virtual_by_file (int FileNumber)
{
  /* called by file_clear() */
  VirtualType *Z;
   
  assert( My != NULL );

  for (Z = My->VirtualHead; Z != NULL; Z = Z->next)
  {
    if (Z->FileNumber == FileNumber)
    {
      /* FOUND */
      clear_virtual (Z);
    }
  }
}
static VirtualType *
find_virtual_by_variable (VariableType * Variable)
{
  VirtualType *Z;
   
  assert (Variable != NULL);
  assert( My != NULL );

  for (Z = My->VirtualHead; Z != NULL; Z = Z->next)
  {
    if (Z->Variable == Variable)
    {
      /* FOUND */
      return Z;
    }
  }
  /* NOT FOUND */
  return NULL;
}

LineType *
bwb_LOCAL (LineType * l)
{
  /* only supported inside a FUNCTION or SUB */
   
  assert (l != NULL);
  return bwb_DIM (l);
}

LineType *
bwb_DIM (LineType * l)
{
  int FileNumber;                /* the file might not be OPEN when the variable is declared */
  size_t FileOffset;                /* from beginning of file */
  int FileLength;                /* sizeof( DoubleType ) or Fixed String Length */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->DefaultVariableType != NULL );


  FileNumber = 0;
  FileOffset = 0;
  FileLength = 0;
  if (line_skip_FilenumChar (l))
  {
    /* DIM # filenum , ... */
    if (line_read_integer_expression (l, &FileNumber) == FALSE)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (FileNumber <= 0)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (line_skip_seperator (l) == FALSE)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    FileOffset = 0;
    FileLength = 0;
  }

  do
  {
    VariableType *v;
    int n;
    int dimensions;
    int LBOUND[MAX_DIMS];
    int UBOUND[MAX_DIMS];
    char TypeCode;
    char varname[NameLengthMax + 1];


    /* Get variable name */
    if (line_read_varname (l, varname) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    /* read parameters */
    dimensions = 0;
    if (line_peek_LparenChar (l))
    {
      if (line_read_array_redim (l, &dimensions, LBOUND, UBOUND) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      /* check array dimensions */
      for (n = 0; n < dimensions; n++)
      {
        if (UBOUND[n] < LBOUND[n])
        {
          WARN_SUBSCRIPT_OUT_OF_RANGE;
          return (l);
        }
      }
    }

    /* determine variable TypeCode */
    TypeCode = var_nametype (varname);
    if (TypeCode == NulChar)
    {
      /* variable has no explicit TypeCode char */
      TypeCode = line_read_type_declaration (l);        /* AS DOUBLE and so on */
      if (TypeCode == NulChar)
      {
        /* variable has no declared TypeCode */
        int i;
        i = VarTypeIndex (varname[0]);
        if (i < 0)
        {
          TypeCode = DoubleTypeCode;        /* default */
        }
        else
        {
          TypeCode = My->DefaultVariableType[i];
        }
      }
    }

    switch (TypeCode)
    {
    case ByteTypeCode:
      /* DIM # file_num , var_name AS BYTE */
      FileLength = sizeof (ByteType);
      break;
    case IntegerTypeCode:
      /* DIM # file_num , var_name AS INTEGER */
      FileLength = sizeof (IntegerType);
      break;
    case LongTypeCode:
      /* DIM # file_num , var_name AS LONG */
      FileLength = sizeof (LongType);
      break;
    case CurrencyTypeCode:
      /* DIM # file_num , var_name AS CURRENCY */
      FileLength = sizeof (CurrencyType);
      break;
    case SingleTypeCode:
      /* DIM # file_num , var_name AS SINGLE */
      FileLength = sizeof (SingleType);
      break;
    case DoubleTypeCode:
      /* DIM # file_num , var_name AS DOUBLE */
      FileLength = sizeof (DoubleType);
      break;
    case StringTypeCode:
      /* DIM # file_num , var_name AS STRING * fixed_length */

      FileLength = 16;                /* default */
      if (line_skip_StarChar (l) || line_skip_EqualChar (l))
      {
        /* optional fixed length */
        if (line_read_integer_expression (l, &FileLength) == FALSE)
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
        if (FileLength <= 0)
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
        if (FileLength > MAXLEN)
        {
          WARN_STRING_TOO_LONG;        /* bwb_DIM */
          FileLength = MAXLEN;
        }
      }
      break;
    default:
      {
        WARN_INTERNAL_ERROR;
        return (l);
      }
    }

    v = var_find (varname, dimensions, FALSE);
    if (v == NULL)
    {
      /*  a new variable */
      if ((v = (VariableType *) calloc (1, sizeof (VariableType))) == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return (l);
      }
      bwb_strcpy (v->name, varname);
      v->VariableTypeCode = TypeCode;
      /* assign array dimensions */
      v->dimensions = dimensions;
      for (n = 0; n < dimensions; n++)
      {
        v->LBOUND[n] = LBOUND[n];
        v->UBOUND[n] = UBOUND[n];
      }
      /* assign initial array position */
      for (n = 0; n < dimensions; n++)
      {
        v->VINDEX[n] = v->LBOUND[n];
      }
      /* calculate the array size */
      v->array_units = 1;
      for (n = 0; n < dimensions; n++)
      {
        v->array_units *= v->UBOUND[n] - v->LBOUND[n] + 1;
      }
      /* assign array memory */

      if (FileNumber > 0)
      {
        /* the new variable is VIRTUAL */
        v->VariableFlags = VARIABLE_VIRTUAL;
        /* if( TRUE ) */
        {
          /* OK */
          VirtualType *Z;
          Z = find_virtual_by_variable (v);
          if (Z == NULL)
          {
            Z = new_virtual ();
            if (Z == NULL)
            {
              WARN_OUT_OF_MEMORY;
              return (l);
            }
            Z->Variable = v;
          }
          /* update file information */
          Z->FileNumber = FileNumber;
          Z->FileOffset = FileOffset;
          Z->FileLength = FileLength;
          FileOffset += FileLength * v->array_units;
        }
      }
      else if (VAR_IS_STRING (v))
      {
        if ((v->Value.String =
             (StringType *) calloc (v->array_units,
                                    sizeof (StringType))) == NULL)
        {
          WARN_OUT_OF_MEMORY;
          return (l);
        }
      }
      else
      {
        if ((v->Value.Number =
             (DoubleType *) calloc (v->array_units,
                                    sizeof (DoubleType))) == NULL)
        {
          WARN_OUT_OF_MEMORY;
          return (l);
        }
      }
      /* set place at beginning of variable chain */
      var_link_new_variable (v);

      /* end of conditional for new variable */
    }
    else
    {
      /* old variable */
      if (v->VariableTypeCode != TypeCode)
      {
        WARN_TYPE_MISMATCH;
        return (l);
      }

      /* check to be sure the number of dimensions is the same */
      if (v->dimensions != dimensions)
      {
        WARN_REDIMENSION_ARRAY;
        return (l);
      }
      /* check to be sure sizes for each dimension are the same */
      for (n = 0; n < dimensions; n++)
      {
        if (v->LBOUND[n] != LBOUND[n])
        {
          WARN_REDIMENSION_ARRAY;
          return (l);
        }
        if (v->UBOUND[n] != UBOUND[n])
        {
          WARN_REDIMENSION_ARRAY;
          return (l);
        }
      }
      if (FileNumber > 0)
      {
        /* the existing variable MUST be Virtual */
        if (v->VariableFlags & VARIABLE_VIRTUAL)
        {
          /* OK */
          VirtualType *Z;
          Z = find_virtual_by_variable (v);
          if (Z == NULL)
          {
            Z = new_virtual ();
            if (Z == NULL)
            {
              WARN_OUT_OF_MEMORY;
              return (l);
            }
            Z->Variable = v;
          }
          /* update file information */
          Z->FileNumber = FileNumber;
          Z->FileOffset = FileOffset;
          Z->FileLength = FileLength;
          FileOffset += FileLength * v->array_units;
        }
        else
        {
          /* the existing variable is NOT virtual */
          WARN_TYPE_MISMATCH;
          return (l);
        }
      }
      else
      {
        /* the existing variable CANNOT be Virtual */
        if (v->VariableFlags & VARIABLE_VIRTUAL)
        {
          /* the existing variable IS virtual */
          WARN_TYPE_MISMATCH;
          return (l);
        }
        else
        {
          /* OK */
        }
      }
      /* end of conditional for old variable */
    }

  }
  while (line_skip_seperator (l));

  /* return */
  return (l);
}




/***************************************************************
  
        FUNCTION:       dim_unit()
  
        DESCRIPTION:    This function calculates the unit
            position for an array.
  
***************************************************************/

static size_t
dim_unit (VariableType * v, int *pp)
{
  size_t r;
  size_t b;
  int n;
   
  assert (v != NULL);
  assert (pp != NULL);

  /* Calculate and return the address of the dimensioned array */

  /* Check EACH dimension for out-of-bounds, AND check correct number
   * of dimensions.  NBS_P076_0250 errors correctly. */

  /*
     Ux = Upper bound of dimension
     Lx = Lower bound of dimension
     Ix = Selected idex in dimension

     dimensions   b   
     0            1   
     1            b0 * ( U0 - L0 + 1 )
     2            b1 * ( U1 - L1 + 1 )
     3            b2 * ( U2 - L2 + 1 )


     dimensions   r   
     0            0   
     1            r0 + ( I0 - L0 ) * b0
     2            r1 + ( I1 - L1 ) * b1
     3            r2 + ( I2 - L2 ) * b2

   */

  r = 0;
  b = 1;
  for (n = 0; n < v->dimensions; n++)
  {
    if (pp[n] < v->LBOUND[n] || pp[n] > v->UBOUND[n])
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return 0;
    }
    r += b * (pp[n] - v->LBOUND[n]);
    b *= v->UBOUND[n] - v->LBOUND[n] + 1;
  }


  if (r > v->array_units)
  {
    WARN_SUBSCRIPT_OUT_OF_RANGE;
    return 0;
  }
  return r;

}


/***************************************************************
  
        FUNCTION:       bwb_option()
  
        DESCRIPTION:    This function implements the BASIC OPTION
                        BASE statement, designating the base (1 or
                        0) for addressing DIM arrays.
  
   SYNTAX:     OPTION BASE number
  
***************************************************************/

void
OptionVersionSet (int i)
{
  assert( i >= 0 && i < NUM_VERSIONS );
  assert( My != NULL );
   
  My->CurrentVersion = &bwb_vertable[i];
}

LineType *
bwb_OPTION (LineType * l)
{
  assert (l != NULL);

  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_ANGLE (LineType * l)
{
  assert (l != NULL);

  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_ANGLE_DEGREES (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  /* OPTION ANGLE DEGREES */
  My->CurrentVersion->OptionFlags |= OPTION_ANGLE_DEGREES;
  My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_GRADIANS;
  return (l);
}

LineType *
bwb_OPTION_ANGLE_GRADIANS (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION ANGLE GRADIANS */
  My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_DEGREES;
  My->CurrentVersion->OptionFlags |= OPTION_ANGLE_GRADIANS;
  return (l);
}

LineType *
bwb_OPTION_ANGLE_RADIANS (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION ANGLE RADIANS */
  My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_DEGREES;
  My->CurrentVersion->OptionFlags &= ~OPTION_ANGLE_GRADIANS;
  return (l);
}

LineType *
bwb_OPTION_ARITHMETIC (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_ARITHMETIC_DECIMAL (LineType * l)
{
  /* OPTION ARITHMETIC DECIMAL */
  assert (l != NULL);
  return (l);
}

LineType *
bwb_OPTION_ARITHMETIC_FIXED (LineType * l)
{
  /* OPTION ARITHMETIC FIXED */
  assert (l != NULL);
  return (l);
}

LineType *
bwb_OPTION_ARITHMETIC_NATIVE (LineType * l)
{
  /* OPTION ARITHMETIC NATIVE */
  assert (l != NULL);
  return (l);
}

LineType *
bwb_OPTION_BASE (LineType * l)
{
  /* OPTION BASE integer */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  return bwb_option_range_integer (l,
                                   &(My->CurrentVersion->OptionBaseInteger),
                                   MININT, MAXINT);
}

LineType *
bwb_OPTION_BUGS (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_BUGS_BOOLEAN (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION BUGS BOOLEAN */
  My->CurrentVersion->OptionFlags |= OPTION_BUGS_BOOLEAN;
  return (l);
}

LineType *
bwb_OPTION_BUGS_ON (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION BUGS ON */
  My->CurrentVersion->OptionFlags |= OPTION_BUGS_ON;
  return (l);
}

LineType *
bwb_OPTION_BUGS_OFF (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION BUGS OFF */
  My->CurrentVersion->OptionFlags &= ~OPTION_BUGS_ON;
  My->CurrentVersion->OptionFlags &= ~OPTION_BUGS_BOOLEAN;
  return (l);
}

LineType *
bwb_option_punct_char (LineType * l, char *c)
{
  /* OPTION ... char$ */
   
  assert (l != NULL);
  assert (c != NULL);

  {
    char *Value;
    char C;

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
    C = Value[0];
    free (Value);
    /* OK */
    if (bwb_ispunct (C))
    {
      /* enable */
      *c = C;
    }
    else
    {
      /* disable */
      *c = NulChar;
    }
  }
  return (l);
}

LineType *
bwb_option_range_integer (LineType * l, int *Integer, int MinVal, int MaxVal)
{
  /* OPTION ... integer */
   
  assert (l != NULL);
  assert (Integer != NULL);
  assert (MinVal < MaxVal);

  {
    int Value;

    Value = 0;
    if (line_read_integer_expression (l, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (Value < MinVal || Value > MaxVal)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    *Integer = Value;
  }
  return (l);
}

LineType *
bwb_OPTION_PUNCT_COMMENT (LineType * l)
{
  /* OPTION PUNCT COMMENT char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionCommentChar));
}

LineType *
bwb_OPTION_COMPARE (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_COMPARE_BINARY (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION COMPARE BINARY */
  My->CurrentVersion->OptionFlags &= ~OPTION_COMPARE_TEXT;
  return (l);
}

LineType *
bwb_OPTION_COMPARE_DATABASE (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION COMPARE DATABASE */
  My->CurrentVersion->OptionFlags |= OPTION_COMPARE_TEXT;
  return (l);
}

LineType *
bwb_OPTION_COMPARE_TEXT (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION COMPARE TEXT */
  My->CurrentVersion->OptionFlags |= OPTION_COMPARE_TEXT;
  return (l);
}

LineType *
bwb_OPTION_COVERAGE (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_COVERAGE_ON (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION COVERAGE ON */
  My->CurrentVersion->OptionFlags |= OPTION_COVERAGE_ON;
  return (l);
}

LineType *
bwb_OPTION_COVERAGE_OFF (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION COVERAGE OFF */
  My->CurrentVersion->OptionFlags &= ~OPTION_COVERAGE_ON;
  return (l);
}

LineType *
bwb_OPTION_DATE (LineType * l)
{
  /* OPTION DATE format$ */
  char *Value;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );


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
  /* OK */
  My->CurrentVersion->OptionDateFormat = Value;
#if FALSE                        /* keep this ... */
  /*
   ** Yes, this can theoretically cause a memory leak.
   ** No, we are not going to fix it.
   ** This command is only supported in the profile.
   ** This will only execute at most once,
   ** so there is no actual memory leak.
   ** 
   */
  free (Value);
#endif
  return (l);
}

LineType *
bwb_OPTION_DIGITS (LineType * l)
{
  int Value;
   
  assert (l != NULL);
  assert( My != NULL );

  /* OPTION DIGITS integer */
  Value = 0;
  if (line_read_integer_expression (l, &Value))
  {
    /* OK */
    if (Value == 0)
    {
      /* default */
      Value = SIGNIFICANT_DIGITS;
    }
    if (Value < MINIMUM_DIGITS || Value > MAXIMUM_DIGITS)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    My->OptionDigitsInteger = Value;
  }
  return (l);
}

LineType *
bwb_OPTION_DISABLE (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_DISABLE_COMMAND (LineType * l)
{
  /* OPTION DISABLE COMMAND name$ */
  int IsFound;
  char *Value;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );


  IsFound = FALSE;
  Value = NULL;

  /* Get COMMAND */
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
  /* OK */
  {
    /* Name */
    int i;
    for (i = 0; i < NUM_COMMANDS; i++)
    {
      if (bwb_stricmp (Value, IntrinsicCommandTable[i].name) == 0)
      {
        /* FOUND */
        /* DISABLE COMMAND */
        IntrinsicCommandTable[i].OptionVersionBitmask &=
          ~My->CurrentVersion->OptionVersionValue;
        IsFound = TRUE;
      }
    }
  }
  free (Value);
  if (IsFound == FALSE)
  {
    /* display warning message */
    fprintf (My->SYSOUT->cfp, "IGNORED: %s\n", l->buffer);
    ResetConsoleColumn ();
  }
  return (l);
}


LineType *
bwb_OPTION_DISABLE_FUNCTION (LineType * l)
{
  /* OPTION DISABLE FUNCTION  name$ */
  int IsFound;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );


  IsFound = FALSE;
  /* Get FUNCTION */
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
    /* OK */
    {
      /* Name */
      int i;
      for (i = 0; i < NUM_FUNCTIONS; i++)
      {
        if (bwb_stricmp (Value, IntrinsicFunctionTable[i].Name) == 0)
        {
          /* FOUND */
          /* DISABLE FUNCTION */
          IntrinsicFunctionTable[i].OptionVersionBitmask &=
            ~My->CurrentVersion->OptionVersionValue;
          IsFound = TRUE;
        }
      }
    }
    free (Value);
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
bwb_OPTION_EDIT (LineType * l)
{
  /* OPTION EDIT string$ */
  char *Value;
   
  assert (l != NULL);
  assert( My != NULL );

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
  /* OK */
  My->OptionEditString = Value;
#if FALSE                        /* keep this ... */
  /*
   ** Yes, this can theoretically cause a memory leak.
   ** No, we are not going to fix it.
   ** This command is only supported in the profile.
   ** This will only execute at most once,
   ** so there is no actual memory leak.
   ** 
   */
  free (Value);
#endif
  return (l);
}

LineType *
bwb_OPTION_EXTENSION (LineType * l)
{
  /* OPTION EXTENSION ext$ */
  char *Value;
   
  assert (l != NULL);
  assert( My != NULL );

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
  /* OK */
  My->OptionExtensionString = Value;
#if FALSE                        /* keep this ... */
  /*
   ** Yes, this can theoretically cause a memory leak.
   ** No, we are not going to fix it.
   ** This command is only supported in the profile.
   ** This command will only execute at most once,
   ** so there is no actual memory leak.
   ** 
   */
  free (Value);
#endif
  return (l);
}

LineType *
bwb_OPTION_FILES (LineType * l)
{
  /* OPTION FILES name$ */
  char *Value;
   
  assert (l != NULL);
  assert( My != NULL );


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
  /* OK */
  My->OptionFilesString = Value;
#if FALSE                        /* keep this ... */
  /*
   ** Yes, this can theoretically cause a memory leak.
   ** No, we are not going to fix it.
   ** This command is only supported in the profile.
   ** This will only execute at most once,
   ** so there is no actual memory leak.
   ** 
   */
  free (Value);
#endif
  return (l);
}

LineType *
bwb_OPTION_PROMPT (LineType * l)
{
  /* OPTION PROMPT prompt$ */
  char *Value;
   
  assert (l != NULL);
  assert( My != NULL );


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
  /* OK */
  My->OptionPromptString = Value;
#if FALSE                        /* keep this ... */
  /*
   ** Yes, this can theoretically cause a memory leak.
   ** No, we are not going to fix it.
   ** This command is only supported in the profile.
   ** This will only execute at most once,
   ** so there is no actual memory leak.
   ** 
   */
  free (Value);
#endif
  return (l);
}

LineType *
bwb_OPTION_RENUM (LineType * l)
{
  /* OPTION RENUM name$ */
  char *Value;
   
  assert (l != NULL);
  assert( My != NULL );


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
  /* OK */
  My->OptionRenumString = Value;
#if FALSE                        /* keep this ... */
  /*
   ** Yes, this can theoretically cause a memory leak.
   ** No, we are not going to fix it.
   ** This command is only supported in the profile.
   ** This will only execute at most once,
   ** so there is no actual memory leak.
   ** 
   */
  free (Value);
#endif
  return (l);
}

LineType *
bwb_OPTION_ENABLE (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_ENABLE_COMMAND (LineType * l)
{
  /* OPTION ENABLE COMMAND name$ */
  int IsFound;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );


  IsFound = FALSE;
  /* Get COMMAND */
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
    /* OK */
    {
      /* Name */
      int i;
      for (i = 0; i < NUM_COMMANDS; i++)
      {
        if (bwb_stricmp (Value, IntrinsicCommandTable[i].name) == 0)
        {
          /* FOUND */
          /* ENABLE COMMAND */
          IntrinsicCommandTable[i].OptionVersionBitmask |=
            My->CurrentVersion->OptionVersionValue;
          IsFound = TRUE;
        }
      }
    }
    free (Value);
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
bwb_OPTION_ENABLE_FUNCTION (LineType * l)
{
  /* OPTION ENABLE FUNCTION name$ */
  int IsFound;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );


  IsFound = FALSE;
  /* Get FUNCTION */
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
    /* OK */
    {
      /* Name */
      int i;
      for (i = 0; i < NUM_FUNCTIONS; i++)
      {
        if (bwb_stricmp (Value, IntrinsicFunctionTable[i].Name) == 0)
        {
          /* FOUND */
          /* ENABLE FUNCTION */
          IntrinsicFunctionTable[i].OptionVersionBitmask |=
            My->CurrentVersion->OptionVersionValue;
          IsFound = TRUE;
        }
      }
    }
    free (Value);
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
bwb_OPTION_ERROR (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_ERROR_GOSUB (LineType * l)
{
  /* OPTION ERROR GOSUB */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  My->CurrentVersion->OptionFlags |= OPTION_ERROR_GOSUB;
  return (l);
}

LineType *
bwb_OPTION_ERROR_GOTO (LineType * l)
{
  /* OPTION ERROR GOTO */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  My->CurrentVersion->OptionFlags &= ~OPTION_ERROR_GOSUB;
  return (l);
}

LineType *
bwb_OPTION_EXPLICIT (LineType * l)
{
  /* OPTION EXPLICIT */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  My->CurrentVersion->OptionFlags |= OPTION_EXPLICIT_ON;
  return (l);
}


LineType *
bwb_OPTION_PUNCT_IMAGE (LineType * l)
{
  /* OPTION PUNCT IMAGE char$ */   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionImageChar));
}

LineType *
bwb_OPTION_IMPLICIT (LineType * l)
{
  /* OPTION IMPLICIT */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  My->CurrentVersion->OptionFlags &= ~OPTION_EXPLICIT_ON;
  return (l);
}

LineType *
bwb_OPTION_INDENT (LineType * l)
{
  /* OPTION INDENT integer */
  assert (l != NULL);
  assert( My != NULL );

  return bwb_option_range_integer (l, &(My->OptionIndentInteger), 0, 7);
}

LineType *
bwb_OPTION_PUNCT_INPUT (LineType * l)
{
  /* OPTION PUNCT INPUT char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionInputChar));
}

LineType *
bwb_OPTION_LABELS (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_LABELS_ON (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION LABELS ON */
  My->CurrentVersion->OptionFlags |= OPTION_LABELS_ON;
  return (l);
}

LineType *
bwb_OPTION_LABELS_OFF (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  /* OPTION LABELS OFF */
  My->CurrentVersion->OptionFlags &= ~OPTION_LABELS_ON;
  return (l);
}

LineType *
bwb_OPTION_PUNCT_PRINT (LineType * l)
{
  /* OPTION PUNCT PRINT char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionPrintChar));
}

LineType *
bwb_OPTION_PUNCT_QUOTE (LineType * l)
{
  /* OPTION PUNCT QUOTE char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionQuoteChar));
}

LineType *
bwb_OPTION_ROUND (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_ROUND_BANK (LineType * l)
{
  /* OPTION ROUND BANK */
  assert (l != NULL);
  assert( My != NULL );

  My->OptionRoundType = C_OPTION_ROUND_BANK;
  return (l);
}

LineType *
bwb_OPTION_ROUND_MATH (LineType * l)
{
  /* OPTION ROUND MATH */
  assert (l != NULL);
  assert( My != NULL );

  My->OptionRoundType = C_OPTION_ROUND_MATH;
  return (l);
}

LineType *
bwb_OPTION_ROUND_TRUNCATE (LineType * l)
{
  /* OPTION ROUND TRUNCATE */
  assert (l != NULL);
  assert( My != NULL );

  My->OptionRoundType = C_OPTION_ROUND_TRUNCATE;
  return (l);
}

LineType *
bwb_OPTION_SCALE (LineType * l)
{
  /* OPTION SCALE integer */
  assert (l != NULL);
  assert( My != NULL );

  return bwb_option_range_integer (l, &(My->OptionScaleInteger),
                                   MINIMUM_SCALE, MAXIMUM_SCALE);
}


LineType *
bwb_OPTION_SLEEP (LineType * l)
{
  /* OPTION SLEEP number */
  assert (l != NULL);
  assert( My != NULL );

  if (line_read_numeric_expression (l, &My->OptionSleepDouble) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  return (l);
}

LineType *
bwb_OPTION_STDERR (LineType * l)
{
  /* OPTION STDERR filename$ */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSPRN != NULL );
  assert( My->SYSPRN->cfp != NULL );


  if (line_is_eol (l))
  {
    bwb_fclose (My->SYSPRN->cfp);
    My->SYSPRN->cfp = stderr;
  }
  else
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
    /* OK */
    if (is_empty_string (Value))
    {
      bwb_fclose (My->SYSPRN->cfp);
      My->SYSPRN->cfp = stderr;
    }
    else
    {
      bwb_fclose (My->SYSPRN->cfp);
      My->SYSPRN->cfp = fopen (Value, "w+");
      if (My->SYSPRN->cfp == NULL)
      {
        /* sane default */
        My->SYSPRN->cfp = stderr;
        WARN_BAD_FILE_NAME;
      }
    }
    free (Value);
  }
  return (l);
}

LineType *
bwb_OPTION_STDIN (LineType * l)
{
  /* OPTION STDIN filename$ */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSIN != NULL );
  assert( My->SYSIN->cfp != NULL );

  if (line_is_eol (l))
  {
    bwb_fclose (My->SYSIN->cfp);
    My->SYSIN->cfp = stdin;
  }
  else
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
    /* OK */
    if (is_empty_string (Value))
    {
      bwb_fclose (My->SYSIN->cfp);
      My->SYSIN->cfp = stdin;
    }
    else
    {
      bwb_fclose (My->SYSIN->cfp);
      My->SYSIN->cfp = fopen (Value, "r");
      if (My->SYSIN->cfp == NULL)
      {
        /* sane default */
        My->SYSIN->cfp = stdin;
        WARN_BAD_FILE_NAME;
      }
    }
    free (Value);
  }
  return (l);
}

LineType *
bwb_OPTION_STDOUT (LineType * l)
{
  /* OPTION STDOUT filename$ */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );

  if (line_is_eol (l))
  {
    bwb_fclose (My->SYSOUT->cfp);
    My->SYSOUT->cfp = stdout;
  }
  else
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
    /* OK */
    if (is_empty_string (Value))
    {
      bwb_fclose (My->SYSOUT->cfp);
      My->SYSOUT->cfp = stdout;
    }
    else
    {
      bwb_fclose (My->SYSOUT->cfp);
      My->SYSOUT->cfp = fopen (Value, "w+");
      if (My->SYSOUT->cfp == NULL)
      {
        /* sane default */
        My->SYSOUT->cfp = stdout;
        WARN_BAD_FILE_NAME;
      }
    }
    free (Value);
  }
  return (l);
}

LineType *
bwb_OPTION_PUNCT_STATEMENT (LineType * l)
{
  /* OPTION PUNCT STATEMENT char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l,
                                &(My->CurrentVersion->OptionStatementChar));
}

LineType *
bwb_OPTION_STRICT (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_STRICT_ON (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  /* OPTION STRICT ON */
  My->CurrentVersion->OptionFlags |= OPTION_STRICT_ON;
  return (l);
}

LineType *
bwb_OPTION_STRICT_OFF (LineType * l)
{
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  /* OPTION STRICT OFF */
  My->CurrentVersion->OptionFlags &= ~OPTION_STRICT_ON;
  return (l);
}

LineType *
bwb_OPTION_PUNCT (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_PUNCT_STRING (LineType * l)
{
  /* OPTION PUNCT STRING char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionStringChar));
}

LineType *
bwb_OPTION_PUNCT_DOUBLE (LineType * l)
{
  /* OPTION PUNCT DOUBLE char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionDoubleChar));
}

LineType *
bwb_OPTION_PUNCT_SINGLE (LineType * l)
{
  /* OPTION PUNCT SINGLE char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionSingleChar));
}

LineType *
bwb_OPTION_PUNCT_CURRENCY (LineType * l)
{
  /* OPTION PUNCT CURRENCY char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionCurrencyChar));
}

LineType *
bwb_OPTION_PUNCT_LONG (LineType * l)
{
  /* OPTION PUNCT LONG char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionLongChar));
}

LineType *
bwb_OPTION_PUNCT_INTEGER (LineType * l)
{
  /* OPTION PUNCT INTEGER char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionIntegerChar));
}

LineType *
bwb_OPTION_PUNCT_BYTE (LineType * l)
{
  /* OPTION PUNCT BYTE char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionByteChar));
}

LineType *
bwb_OPTION_PUNCT_LPAREN (LineType * l)
{
  /* OPTION PUNCT LPAREN char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionLparenChar));
}

LineType *
bwb_OPTION_PUNCT_RPAREN (LineType * l)
{
  /* OPTION PUNCT RPAREN char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionRparenChar));
}

LineType *
bwb_OPTION_PUNCT_FILENUM (LineType * l)
{
  /* OPTION PUNCT FILENUM char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionFilenumChar));
}

LineType *
bwb_OPTION_PUNCT_AT (LineType * l)
{
  /* OPTION PUNCT AT char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionAtChar));
}

LineType *
bwb_OPTION_RECLEN (LineType * l)
{
  /* OPTION RECLEN integer */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_range_integer (l,
                                   &(My->CurrentVersion->OptionReclenInteger),
                                   0, MAXINT);
}

LineType *
bwb_OPTION_TERMINAL (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_TERMINAL_NONE (LineType * l)
{
  /* OPTION TERMINAL NONE */
  assert (l != NULL);
  assert( My != NULL );
  
  My->OptionTerminalType = C_OPTION_TERMINAL_NONE;
  return (l);
}

LineType *
bwb_OPTION_TERMINAL_ADM (LineType * l)
{
  /* OPTION TERMINAL ADM-3A */
  assert (l != NULL);
  assert( My != NULL );
  
  My->OptionTerminalType = C_OPTION_TERMINAL_ADM;
  return (l);
}

LineType *
bwb_OPTION_TERMINAL_ANSI (LineType * l)
{
  /* OPTION TERMINAL ANSI */
  assert (l != NULL);
  assert( My != NULL );
  
  My->OptionTerminalType = C_OPTION_TERMINAL_ANSI;
  return (l);
}

LineType *
bwb_OPTION_TIME (LineType * l)
{
  /* OPTION TIME format$ */
  char *Value;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
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
  /* OK */
  My->CurrentVersion->OptionTimeFormat = Value;
#if FALSE                        /* keep this ... */
  /*
   ** Yes, this can theoretically cause a memory leak.
   ** No, we are not going to fix it.
   ** This command is only supported in the profile.
   ** This will only execute at most once,
   ** so there is no actual memory leak.
   ** 
   */
  free (Value);
#endif
  return (l);
}

LineType *
bwb_OPTION_TRACE (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_TRACE_ON (LineType * l)
{
  /* OPTION TRACE ON */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  My->CurrentVersion->OptionFlags |= OPTION_TRACE_ON;
  return (l);
}

LineType *
bwb_OPTION_TRACE_OFF (LineType * l)
{
  /* OPTION TRACE OFF */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  My->CurrentVersion->OptionFlags &= ~OPTION_TRACE_ON;
  return (l);
}

LineType *
bwb_OPTION_USING (LineType * l)
{
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_OPTION_USING_DIGIT (LineType * l)
{
  /* OPTION USING DIGIT char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingDigit));
}

LineType *
bwb_OPTION_USING_COMMA (LineType * l)
{
  /* OPTION USING COMMA char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingComma));
}

LineType *
bwb_OPTION_USING_PERIOD (LineType * l)
{
  /* OPTION USING PERIOD char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingPeriod));
}

LineType *
bwb_OPTION_USING_PLUS (LineType * l)
{
  /* OPTION USING PLUS char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingPlus));
}

LineType *
bwb_OPTION_USING_MINUS (LineType * l)
{
  /* OPTION USING MINUS char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingMinus));
}

LineType *
bwb_OPTION_USING_EXRAD (LineType * l)
{
  /* OPTION USING EXRAD char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingExrad));
}

LineType *
bwb_OPTION_USING_DOLLAR (LineType * l)
{
  /* OPTION USING DOLLAR char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingDollar));
}

LineType *
bwb_OPTION_USING_FILLER (LineType * l)
{
  /* OPTION USING FILLER char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingFiller));
}

LineType *
bwb_OPTION_USING_LITERAL (LineType * l)
{
  /* OPTION USING LITERAL char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingLiteral));
}

LineType *
bwb_OPTION_USING_FIRST (LineType * l)
{
  /* OPTION USING FIRST char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingFirst));
}

LineType *
bwb_OPTION_USING_ALL (LineType * l)
{
  /* OPTION USING ALL char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingAll));
}

LineType *
bwb_OPTION_USING_LENGTH (LineType * l)
{
  /* OPTION USING LENGTH char$ */
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  
  return bwb_option_punct_char (l, &(My->CurrentVersion->OptionUsingLength));
}

extern LineType *
bwb_OPTION_VERSION (LineType * l)
{
  /* OPTION VERSION [version$] */
  char *Name;
  int i;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );
  

  Name = NULL;
  if (line_is_eol (l))
  {
    /* OPTIONAL */
  }
  else if (line_read_string_expression (l, &Name))
  {
    if (is_empty_string (Name) == FALSE)
    {
      /*  a version was specified */
      for (i = 0; i < NUM_VERSIONS; i++)
      {
        if (bwb_stricmp (Name, bwb_vertable[i].Name) == 0)
        {
          /* FOUND */
          OptionVersionSet (i);
          return (l);
        }
      }
      /* NOT FOUND */
      fprintf (My->SYSOUT->cfp, "OPTION VERSION \"%s\" IS INVALID\n", Name);
    }
  }
  fprintf (My->SYSOUT->cfp, "VALID CHOICES ARE:\n");
  for (i = 0; i < NUM_VERSIONS; i++)
  {
    char *tbuf;

    tbuf = My->ConsoleOutput;
    bwb_strcpy (tbuf, "\"");
    bwb_strcat (tbuf, bwb_vertable[i].Name);
    bwb_strcat (tbuf, "\"");
    fprintf (My->SYSOUT->cfp, "OPTION VERSION %-16s ' %s\n", tbuf,
             bwb_vertable[i].Description);
  }
  ResetConsoleColumn ();
  line_skip_eol (l);
  return (l);
}

LineType *
bwb_OPTION_ZONE (LineType * l)
{
  /* OPTION ZONE integer */
  int Value;
   
  assert (l != NULL);
  assert( My != NULL );

  Value = 0;
  if (line_read_integer_expression (l, &Value))
  {
    /* OK */
    if (Value == 0)
    {
      /* default */
      Value = ZONE_WIDTH;
    }
    if (Value < MINIMUM_ZONE || Value > MAXIMUM_ZONE)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    My->OptionZoneInteger = Value;
  }
  return (l);
}



int
var_get (VariableType * variable, VariantType * variant)
{
  size_t offset;
   
  /* check sanity */
  if (variable == NULL)
  {
    WARN_INTERNAL_ERROR;
    return FALSE;
  }
  if (variant == NULL)
  {
    WARN_INTERNAL_ERROR;
    return FALSE;
  }

  /* Check subscripts */
  if (dim_check (variable) == FALSE)
  {
    WARN_SUBSCRIPT_OUT_OF_RANGE;
    return FALSE;
  }

  /* Determine offset from array base ( for scalars the offset is always zero ) */
  offset = dim_unit (variable, variable->VINDEX);

  CLEAR_VARIANT (variant);

  /* Force compatibility */
  variant->VariantTypeCode = variable->VariableTypeCode;

  if (variable->VariableTypeCode == StringTypeCode)
  {
    /* Variable is a STRING */
    StringType Value;

    Value.sbuffer = NULL;
    Value.length = 0;
    /* both STRING */

    if (variable->VariableFlags & VARIABLE_VIRTUAL)        /* var_get() */
    {
      /* get file information */
      VirtualType *Z;
      FileType *F;

      Z = find_virtual_by_variable (variable);
      if (Z == NULL)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
      offset *= Z->FileLength;        /* Byte offset */
      offset += Z->FileOffset;        /* Beginning of this data */
      /* update file information */
      F = find_file_by_number (Z->FileNumber);
      if (F == NULL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (F->DevMode != DEVMODE_VIRTUAL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (F->cfp == NULL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (fseek (F->cfp, offset, SEEK_SET) != 0)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      Value.length = Z->FileLength;
      if ((Value.sbuffer =
           (char *) calloc (Value.length + 1 /* NulChar */ ,
                            sizeof (char))) == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return FALSE;
      }
      if (fread (Value.sbuffer, Value.length, 1, F->cfp) != 1)
      {
        WARN_DISK_IO_ERROR;
        return FALSE;
      }
    }
    else
    {
      StringType *string;

      string = variable->Value.String;
      if (string == NULL)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
      string += offset;
      if (str_btob (&Value, string) == FALSE)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
    }
    variant->Buffer = Value.sbuffer;
    variant->Length = Value.length;
  }
  else
  {
    /* Variable is a NUMBER */
    DoubleType Value;
    /* both NUMBER */

    if (variable->VariableFlags & VARIABLE_VIRTUAL)        /* var_get() */
    {
      /* get file information */
      VirtualType *Z;
      FileType *F;

      Z = find_virtual_by_variable (variable);
      if (Z == NULL)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
      offset *= Z->FileLength;        /* Byte offset */
      offset += Z->FileOffset;        /* Beginning of this data */
      /* update file information */
      F = find_file_by_number (Z->FileNumber);
      if (F == NULL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (F->DevMode != DEVMODE_VIRTUAL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (F->cfp == NULL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (fseek (F->cfp, offset, SEEK_SET) != 0)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      switch (variable->VariableTypeCode)
      {
      case ByteTypeCode:
        {
          ByteType X;
          if (fread (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
          Value = X;
        }
        break;
      case IntegerTypeCode:
        {
          IntegerType X;
          if (fread (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
          Value = X;
        }
        break;
      case LongTypeCode:
        {
          LongType X;
          if (fread (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
          Value = X;
        }
        break;
      case CurrencyTypeCode:
        {
          CurrencyType X;
          if (fread (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
          Value = X;
        }
        break;
      case SingleTypeCode:
        {
          SingleType X;
          if (fread (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
          Value = X;
        }
        break;
      case DoubleTypeCode:
        {
          DoubleType X;
          if (fread (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
          Value = X;
        }
        break;
      case StringTypeCode:
        {
          WARN_INTERNAL_ERROR;
          return FALSE;
        }
        /* break; */
      default:
        {
          WARN_INTERNAL_ERROR;
          return FALSE;
        }
      }
    }
    else
    {
      DoubleType *number;

      number = variable->Value.Number;
      if (number == NULL)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
      number += offset;
      /* copy value */
      Value = *number;
    }

    /* VerifyNumeric */
    if (isnan (Value))
    {
         /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
    if (isinf (Value))
    {
      /* - Evaluation of an expression results in an overflow
       * (nonfatal, the recommended recovery procedure is to supply
       * machine in- finity with the algebraically correct sign and
       * continue). */
      if (Value < 0)
      {
        Value = MINDBL;
      }
      else
      {
        Value = MAXDBL;
      }
      if (WARN_OVERFLOW)
      {
        /* ERROR */
        return FALSE;
      }
      /* CONTINUE */
    }
    /* OK */
    switch (variable->VariableTypeCode)
    {
    case ByteTypeCode:
    case IntegerTypeCode:
    case LongTypeCode:
    case CurrencyTypeCode:
      /* integer values */
      Value = bwb_rint (Value);
      break;
    case SingleTypeCode:
    case DoubleTypeCode:
      /* float values */
      break;
    default:
      /* ERROR */
      WARN_INTERNAL_ERROR;
      return FALSE;
      /* break; */
    }
    variant->Number = Value;
  }
  return TRUE;
}

int
var_set (VariableType * variable, VariantType * variant)
{
  size_t offset;
   
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );

  /* check sanity */
  if (variable == NULL)
  {
    WARN_INTERNAL_ERROR;
    return FALSE;
  }
  if (variant == NULL)
  {
    WARN_INTERNAL_ERROR;
    return FALSE;
  }

  /* check CONST */
  if (variable->VariableFlags & (VARIABLE_CONSTANT))
  {
    /* attempting to assign to a constant */
    WARN_VARIABLE_NOT_DECLARED;
    return FALSE;
  }

  /* Check subscripts */
  if (dim_check (variable) == FALSE)
  {
    WARN_SUBSCRIPT_OUT_OF_RANGE;
    return FALSE;
  }

  /* Determine offset from array base ( for scalars the offset is always zero ) */
  offset = dim_unit (variable, variable->VINDEX);

  /* Verify compatibility */
  if (variable->VariableTypeCode == StringTypeCode)
  {
    /* Variable is a STRING */
    StringType Value;

    /* Verify value is a STRING */
    if (variant->VariantTypeCode != StringTypeCode)
    {
      WARN_TYPE_MISMATCH;
      return FALSE;
    }
    Value.sbuffer = variant->Buffer;
    Value.length = variant->Length;
    /* both STRING */

    if (variable->VariableFlags & VARIABLE_VIRTUAL)        /* var_set() */
    {
      /* get file information */
      VirtualType *Z;
      FileType *F;
      int count;

      Z = find_virtual_by_variable (variable);
      if (Z == NULL)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
      offset *= Z->FileLength;        /* Byte offset */
      offset += Z->FileOffset;        /* Beginning of this data */
      /* update file information */
      F = find_file_by_number (Z->FileNumber);
      if (F == NULL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (F->DevMode != DEVMODE_VIRTUAL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (F->cfp == NULL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (fseek (F->cfp, offset, SEEK_SET) != 0)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      count = MIN (Value.length, Z->FileLength);
      if (fwrite (Value.sbuffer, sizeof (char), count, F->cfp) != count)
      {
        WARN_DISK_IO_ERROR;
        return FALSE;
      }
      /* PADR */
      while (count < Z->FileLength)
      {
        if (fputc (' ', F->cfp) == EOF)
        {
          WARN_BAD_FILE_MODE;
          return FALSE;
        }
        count++;
      }
    }
    else
    {
      StringType *string;

      string = variable->Value.String;
      if (string == NULL)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
      string += offset;
      if (str_btob (string, &Value) == FALSE)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
    }
    if (variable->VariableFlags & VARIABLE_DISPLAY)        /* var_set() */
    {
      if (My->ThisLine)                /* var_set() */
      {
        if (My->ThisLine->LineFlags & (LINE_USER))        /* var_set() */
        {
          /* immediate mode */
        }
        else
        {
          fprintf (My->SYSOUT->cfp, "#%d %s=%s\n", My->ThisLine->number, variable->name, variant->Buffer);        /* var_set() */
          ResetConsoleColumn ();
        }
      }
    }
  }
  else
  {
    /* Variable is a NUMBER */
    DoubleType Value;

    /* Verify value is a NUMBER */
    if (variant->VariantTypeCode == StringTypeCode)
    {
      WARN_TYPE_MISMATCH;
      return FALSE;
    }

    /* both NUMBER */

    /* VerifyNumeric */
    if (isnan (variant->Number))
    {
         /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
    if (isinf (variant->Number))
    {
      /* - Evaluation of an expression results in an overflow
       * (nonfatal, the recommended recovery procedure is to supply
       * machine in- finity with the algebraically correct sign and
       * continue). */
      if (variant->Number < 0)
      {
        variant->Number = MINDBL;
      }
      else
      {
        variant->Number = MAXDBL;
      }
      if (WARN_OVERFLOW)
      {
        /* ERROR */
        return FALSE;
      }
      /* CONTINUE */
    }
    /* OK */
    switch (variable->VariableTypeCode)
    {
    case ByteTypeCode:
      variant->Number = bwb_rint (variant->Number);
      if (variant->Number < MINBYT)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MINBYT;
      }
      else if (variant->Number > MAXBYT)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MAXBYT;
      }
      break;
    case IntegerTypeCode:
      variant->Number = bwb_rint (variant->Number);
      if (variant->Number < MININT)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MININT;
      }
      else if (variant->Number > MAXINT)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MAXINT;
      }
      break;
    case LongTypeCode:
      variant->Number = bwb_rint (variant->Number);
      if (variant->Number < MINLNG)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MINLNG;
      }
      else if (variant->Number > MAXLNG)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MAXLNG;
      }
      break;
    case CurrencyTypeCode:
      variant->Number = bwb_rint (variant->Number);
      if (variant->Number < MINCUR)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MINCUR;
      }
      else if (variant->Number > MAXCUR)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MAXCUR;
      }
      break;
    case SingleTypeCode:
      if (variant->Number < MINSNG)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MINSNG;
      }
      else if (variant->Number > MAXSNG)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MAXSNG;
      }
      break;
    case DoubleTypeCode:
      if (variant->Number < MINDBL)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MINDBL;
      }
      else if (variant->Number > MAXDBL)
      {
        if (WARN_OVERFLOW)
        {
          return FALSE;
        }
        variant->Number = MAXDBL;
      }
      break;
    default:
      WARN_INTERNAL_ERROR;
      return FALSE;
      /* break; */
    }
    Value = variant->Number;
    if (variable->VariableFlags & VARIABLE_VIRTUAL)        /* var_set() */
    {
      /* get file information */
      VirtualType *Z;
      FileType *F;

      Z = find_virtual_by_variable (variable);
      if (Z == NULL)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
      offset *= Z->FileLength;        /* Byte offset */
      offset += Z->FileOffset;        /* Beginning of this data */
      /* update file information */
      F = find_file_by_number (Z->FileNumber);
      if (F == NULL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (F->DevMode != DEVMODE_VIRTUAL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (F->cfp == NULL)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (fseek (F->cfp, offset, SEEK_SET) != 0)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      switch (variable->VariableTypeCode)
      {
      case ByteTypeCode:
        {
          ByteType X;
          X = Value;
          if (fwrite (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
        }
        break;
      case IntegerTypeCode:
        {
          IntegerType X;
          X = Value;
          if (fwrite (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
        }
        break;
      case LongTypeCode:
        {
          LongType X;
          X = Value;
          if (fwrite (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
        }
        break;
      case CurrencyTypeCode:
        {
          CurrencyType X;
          X = Value;
          if (fwrite (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
        }
        break;
      case SingleTypeCode:
        {
          SingleType X;
          X = Value;
          if (fwrite (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
        }
        break;
      case DoubleTypeCode:
        {
          DoubleType X;
          X = Value;
          if (fwrite (&X, sizeof (X), 1, F->cfp) != 1)
          {
            WARN_DISK_IO_ERROR;
            return FALSE;
          }
        }
        break;
      case StringTypeCode:
        {
          WARN_INTERNAL_ERROR;
          return FALSE;
        }
        /* break; */
      default:
        {
          WARN_INTERNAL_ERROR;
          return FALSE;
        }
      }
    }
    else
    {
      DoubleType *number;
      number = variable->Value.Number;
      if (number == NULL)
      {
        WARN_INTERNAL_ERROR;
        return FALSE;
      }
      number += offset;
      *number = Value;
    }
    if (variable->VariableFlags & VARIABLE_DISPLAY)        /* var_set() */
    {
      if (My->ThisLine)                /* var_set() */
      {
        if (My->ThisLine->LineFlags & (LINE_USER))        /* var_set() */
        {
          /* immediate mode */
        }
        else
        {
          FormatBasicNumber (Value, My->NumLenBuffer);
          fprintf (My->SYSOUT->cfp, "#%d %s=%s\n", My->ThisLine->number, variable->name, My->NumLenBuffer);        /* var_set() */
          ResetConsoleColumn ();
        }
      }
    }
  }
  return TRUE;
}

/***************************************************************
  
        FUNCTION:       dim_check()
  
        DESCRIPTION:    This function checks subscripts of a
                        specific variable to be sure that they
                        are within the correct range.
  
***************************************************************/

static int
dim_check (VariableType * variable)
{
  /* Check for validly allocated array */
  int n;
   
  assert (variable != NULL);


  if (variable->VariableFlags & VARIABLE_VIRTUAL)        /* var_set() */
  {
    if (variable->Value.String != NULL)
    {
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
    if (variable->Value.Number != NULL)
    {
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
  }
  else if (VAR_IS_STRING (variable))
  {
    if (variable->Value.String == NULL)
    {
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
  }
  else
  {
    if (variable->Value.Number == NULL)
    {
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
  }
  /* Now check subscript values */
  for (n = 0; n < variable->dimensions; n++)
  {
    if (variable->VINDEX[n] < variable->LBOUND[n]
        || variable->VINDEX[n] > variable->UBOUND[n])
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return FALSE;
    }
  }
  /* No problems found */
  return TRUE;
}

/***************************************************************
  
        FUNCTION:       var_make()
  
        DESCRIPTION: This function initializes a variable,
            allocating necessary memory for it.
  
***************************************************************/

int
var_make (VariableType * variable, char TypeCode)
{
  /* ALL variables are created here */
   
  assert (variable != NULL);

  switch (TypeCode)
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
    /* ERROR */
    WARN_TYPE_MISMATCH;
    return FALSE;
  }

  variable->VariableTypeCode = TypeCode;

  /* get memory for array */

  /* First cleanup the joint (JBV) */
  if (variable->Value.Number != NULL)
  {
    free (variable->Value.Number);
    variable->Value.Number = NULL;
  }
  if (variable->Value.String != NULL)
  {
    /* Remember to deallocate those far-flung branches! (JBV) */
    StringType *sp;                /* JBV */
    int n;                        /* JBV */

    sp = variable->Value.String;
    for (n = 0; n < (int) variable->array_units; n++)
    {
      if (sp[n].sbuffer != NULL)
      {
        free (sp[n].sbuffer);
        sp[n].sbuffer = NULL;
      }
      sp[n].length = 0;
    }
    free (variable->Value.String);
    variable->Value.String = NULL;
  }

  variable->dimensions = 0;
  variable->array_units = 1;

  if (VAR_IS_STRING (variable))
  {
    if ((variable->Value.String =
         calloc (variable->array_units, sizeof (StringType))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return FALSE;
    }
  }
  else
  {
    if ((variable->Value.Number =
         calloc (variable->array_units, sizeof (DoubleType))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return FALSE;
    }
  }
  return TRUE;

}

/***************************************************************
  
   FUNCTION:       var_islocal()
  
   DESCRIPTION:    This function determines whether the string
         pointed to by 'buffer' has the name of
         a local variable at the present EXEC stack
         level.
  
***************************************************************/

static VariableType *
mat_islocal (char *buffer)
{
  /*
     similar to var_islocal, but returns first matrix found.
   */
   
  assert (buffer != NULL);
  assert( My != NULL );

  if (My->StackHead != NULL)
  {
    StackType *StackItem;
    for (StackItem = My->StackHead; StackItem != NULL;
         StackItem = StackItem->next)
    {
      if (StackItem->LoopTopLine != NULL)
      {
        switch (StackItem->LoopTopLine->cmdnum)
        {
        case C_DEF:
        case C_FUNCTION:
        case C_SUB:
          /* we have found a FUNCTION or SUB boundary */
          {
            VariableType *variable;

            for (variable = StackItem->local_variable; variable != NULL;
                 variable = variable->next)
            {
              if (variable->dimensions > 0)
              {
                if (bwb_stricmp (variable->name, buffer) == 0)
                {
                  /* FOUND */
                  return variable;
                }
              }
            }
          }
          /* we have checked all the way to a FUNCTION or SUB boundary */
          /* NOT FOUND */
          return NULL;
          /* break; */
        }
      }
    }
  }
  /* NOT FOUND */
  return NULL;
}


static VariableType *
var_islocal (char *buffer, int dimensions)
{
   
  assert (buffer != NULL);
  assert( My != NULL );

  if (My->StackHead != NULL)
  {
    StackType *StackItem;
    for (StackItem = My->StackHead; StackItem != NULL;
         StackItem = StackItem->next)
    {
      if (StackItem->LoopTopLine != NULL)
      {
        switch (StackItem->LoopTopLine->cmdnum)
        {
        case C_DEF:
        case C_FUNCTION:
        case C_SUB:
          /* we have found a FUNCTION or SUB boundary */
          {
            VariableType *variable;

            for (variable = StackItem->local_variable; variable != NULL;
                 variable = variable->next)
            {
              if (variable->dimensions == dimensions)
              {
                if (bwb_stricmp (variable->name, buffer) == 0)
                {
                  /* FOUND */
                  return variable;
                }
              }
            }
          }
          /* we have checked all the way to a FUNCTION or SUB boundary */
          /* NOT FOUND */
          return NULL;
          /* break; */
        }
      }
    }
  }
  /* NOT FOUND */
  return NULL;
}

/***************************************************************
  
        FUNCTION:       bwb_vars()
  
        DESCRIPTION:    This function implements the Bywater-
            specific debugging command VARS, which
            gives a list of all variables defined
            in memory.
  
***************************************************************/


LineType *
bwb_VARS (LineType * l)
{
  VariableType *variable;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );

  /* run through the variable list and print variables */


  fprintf (My->SYSOUT->cfp, "%-*s %-6s %-4s %s\n", NameLengthMax, "Name",
           "Type", "Dims", "Value");

  for (variable = My->VariableHead; variable != NULL;
       variable = variable->next)
  {
    VariantType variant;
    CLEAR_VARIANT (&variant);

    if (var_get (variable, &variant) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }
    if (variant.VariantTypeCode == StringTypeCode)
    {
      fprintf (My->SYSOUT->cfp, "%-*s %-6s %-4d %s\n", NameLengthMax,
               variable->name, "STRING", variable->dimensions,
               variant.Buffer);
    }
    else
    {
      FormatBasicNumber (variant.Number, My->NumLenBuffer);
      fprintf (My->SYSOUT->cfp, "%-*s %-6s %-4d %s\n", NameLengthMax,
               variable->name, "NUMBER", variable->dimensions,
               My->NumLenBuffer);
    }
    RELEASE_VARIANT (&variant);
  }
  ResetConsoleColumn ();
  return (l);
}

/***************************************************************
  
        FUNCTION:       bwb_field()
  
        DESCRIPTION: This C function implements the BASIC
         FIELD command.
  
***************************************************************/

static void
field_clear (FieldType * Field)
{
  int i;
   
  assert (Field != NULL);

  Field->File = NULL;
  Field->FieldOffset = 0;
  Field->FieldLength = 0;
  Field->Var = NULL;
  for (i = 0; i < MAX_DIMS; i++)
  {
    Field->VINDEX[i] = 0;
  }
}

static FieldType *
field_new (void)
{
  /* search for an empty slot */
  FieldType *Field;
   
  assert( My != NULL );

  for (Field = My->FieldHead; Field != NULL; Field = Field->next)
  {
    if (Field->File == NULL || Field->Var == NULL)
    {
      field_clear (Field);
      return Field;
    }
  }
  /* not found */
  if ((Field = calloc (1, sizeof (FieldType))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return NULL;
  }
  Field->next = My->FieldHead;
  My->FieldHead = Field;
  return Field;
}

void
field_close_file (FileType * File)
{
  /* a CLOSE of a file is in progress, release associated fields */
  FieldType *Field;
   
  assert (File != NULL);
  assert( My != NULL );

  for (Field = My->FieldHead; Field != NULL; Field = Field->next)
  {
    if (Field->File == File)
    {
      Field->File = NULL;
      Field->Var = NULL;
    }
  }
}
void
field_free_variable (VariableType * Var)
{
  /* an ERASE of a variable is in progress, release associated fields */
  FieldType *Field;
   
  assert (Var != NULL);
  assert( My != NULL );

  for (Field = My->FieldHead; Field != NULL; Field = Field->next)
  {
    if (Field->Var == Var)
    {
      Field->File = NULL;
      Field->Var = NULL;
    }
  }
}


void
field_get (FileType * File)
{
  /* a GET of the RANDOM file is in progress, update variables from FILE buffer */
  FieldType *Field;
   
  assert( My != NULL );

  if (File == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return;
  }
  if (File->buffer == NULL)
  {
    WARN_BAD_FILE_MODE;
    return;
  }
  for (Field = My->FieldHead; Field != NULL; Field = Field->next)
  {
    if (Field->File == File && Field->Var != NULL)
    {
      /* from file to variable */
      VariantType variant;
      CLEAR_VARIANT (&variant);

      if (Field->FieldOffset < 0)
      {
        WARN_FIELD_OVERFLOW;
        return;
      }
      if (Field->FieldLength <= 0)
      {
        WARN_FIELD_OVERFLOW;
        return;
      }
      if ((Field->FieldOffset + Field->FieldLength) > File->width)
      {
        WARN_FIELD_OVERFLOW;
        return;
      }
      variant.VariantTypeCode = StringTypeCode;
      variant.Length = Field->FieldLength;
      if ((variant.Buffer =
           (char *) calloc (variant.Length + 1 /* NulChar */ ,
                            sizeof (char))) == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return;
      }
      /* if( TRUE ) */
      {
        int i;

        for (i = 0; i < Field->Var->dimensions; i++)
        {
          Field->Var->VINDEX[i] = Field->VINDEX[i];
        }
      }
      /* if( TRUE ) */
      {
        int i;
        char *Buffer;

        Buffer = File->buffer;
        Buffer += Field->FieldOffset;
        for (i = 0; i < variant.Length; i++)
        {
          variant.Buffer[i] = Buffer[i];
        }
        variant.Buffer[variant.Length] = NulChar;
      }
      if (var_set (Field->Var, &variant) == FALSE)
      {
        WARN_VARIABLE_NOT_DECLARED;
        return;
      }
      RELEASE_VARIANT (&variant);
    }
  }
}
void
field_put (FileType * File)
{
  /* a PUT of the RANDOM file is in progress, update FILE buffer from variables  */
  FieldType *Field;
   
  assert( My != NULL );

  if (File == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return;
  }
  if (File->buffer == NULL)
  {
    WARN_BAD_FILE_MODE;
    return;
  }
  for (Field = My->FieldHead; Field != NULL; Field = Field->next)
  {
    if (Field->File == File && Field->Var != NULL)
    {
      /* from variable to file */
      VariantType variant;
      CLEAR_VARIANT (&variant);

      if (Field->FieldOffset < 0)
      {
        WARN_FIELD_OVERFLOW;
        return;
      }
      if (Field->FieldLength <= 0)
      {
        WARN_FIELD_OVERFLOW;
        return;
      }
      if ((Field->FieldOffset + Field->FieldLength) > File->width)
      {
        WARN_FIELD_OVERFLOW;
        return;
      }
      /* if( TRUE ) */
      {
        int i;

        for (i = 0; i < Field->Var->dimensions; i++)
        {
          Field->Var->VINDEX[i] = Field->VINDEX[i];
        }
      }
      if (var_get (Field->Var, &variant) == FALSE)
      {
        WARN_VARIABLE_NOT_DECLARED;
        return;
      }
      if (variant.VariantTypeCode != StringTypeCode)
      {
        WARN_TYPE_MISMATCH;
        return;
      }
      /* if( TRUE ) */
      {
        int i;
        int n;
        char *Buffer;

        i = 0;
        n = 0;
        Buffer = File->buffer;
        Buffer += Field->FieldOffset;

        if (variant.Buffer != NULL)
        {
          n = MIN (variant.Length, Field->FieldLength);
        }
        for (i = 0; i < n; i++)
        {
          Buffer[i] = variant.Buffer[i];
        }
        for (i = n; i < Field->FieldLength; i++)
        {
          /* Pad on the right with spaces */
          Buffer[i] = ' ';
        }
      }
      RELEASE_VARIANT (&variant);
    }
  }
}


LineType *
bwb_FIELD (LineType * l)
{
  FileType *File;
  int FileNumber;
  int FieldOffset;
   
  assert (l != NULL);

  FileNumber = 0;
  FieldOffset = 0;

  /* first read device number */
  if (line_skip_FilenumChar (l))
  {
    /* optional */
  }
  if (line_read_integer_expression (l, &FileNumber) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (FileNumber <= 0)
  {
    /* FIELD # 0 is an error */
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  File = find_file_by_number (FileNumber);
  if (File == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (File->DevMode != DEVMODE_RANDOM)
  {
    WARN_BAD_FILE_MODE;
    return (l);
  }
  /* loop to read variables */


  /* read the comma and advance beyond it */
  while (line_skip_seperator (l))
  {
    int FieldLength;
    VariableType *variable;
    VariantType variant;

    CLEAR_VARIANT (&variant);

    /* first find the size of the field */
    FieldLength = 0;
    if (line_read_integer_expression (l, &FieldLength) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (FieldLength <= 0)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    /* read the AS */
    if (line_skip_word (l, "AS") == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    /* read the string variable name */
    if ((variable = line_read_scalar (l)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }

    if (VAR_IS_STRING (variable))
    {
      /* OK */
    }
    else
    {
      WARN_TYPE_MISMATCH;
      return (l);
    }
    /* check for overflow of record length */
    if ((FieldOffset + FieldLength) > File->width)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
    /* set buffer */
    variant.VariantTypeCode = StringTypeCode;
    /* if( TRUE ) */
    {
      FieldType *Field;
      int i;

      Field = field_new ();
      if (Field == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return (l);
      }
      Field->File = File;
      Field->FieldOffset = FieldOffset;
      Field->FieldLength = FieldLength;
      Field->Var = variable;
      for (i = 0; i < variable->dimensions; i++)
      {
        Field->VINDEX[i] = variable->VINDEX[i];
      }
      variant.Length = FieldLength;
      if ((variant.Buffer =
           (char *) calloc (variant.Length + 1 /* NulChar */ ,
                            sizeof (char))) == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return (l);
      }
      bwb_memset (variant.Buffer, ' ', variant.Length);
      variant.Buffer[variant.Length] = NulChar;
    }
    if (var_set (variable, &variant) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }
    RELEASE_VARIANT (&variant);
    FieldOffset += FieldLength;
  }
  /* return */
  return (l);
}

/***************************************************************
  
        FUNCTION:       bwb_lset()
  
        DESCRIPTION: This C function implements the BASIC
         LSET command.
  
   SYNTAX:     LSET string-variable$ = expression
  
***************************************************************/

LineType *
bwb_LSET (LineType * l)
{
   
  assert (l != NULL);
  return dio_lrset (l, FALSE);
}

/***************************************************************
  
        FUNCTION:       bwb_rset()
  
        DESCRIPTION: This C function implements the BASIC
         RSET command.
  
   SYNTAX:     RSET string-variable$ = expression
  
***************************************************************/

LineType *
bwb_RSET (LineType * l)
{
   
  assert (l != NULL);
  return dio_lrset (l, TRUE);
}

/***************************************************************
  
        FUNCTION:       dio_lrset()
  
        DESCRIPTION: This C function implements the BASIC
         RSET and LSET commands.
  
***************************************************************/

static LineType *
dio_lrset (LineType * l, int rset)
{
  /* LSET and RSET */
  VariantType variant;
  int n;
  int i;
  int startpos;
  VariableType *v;
  VariantType t;
  VariantType *T;
   
  assert (l != NULL);

  T = &t;
  CLEAR_VARIANT (T);
  CLEAR_VARIANT (&variant);
  /* get the variable */
  if ((v = line_read_scalar (l)) == NULL)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }
  if (VAR_IS_STRING (v) == FALSE)
  {
    WARN_TYPE_MISMATCH;
    return (l);
  }

  /* skip the equals sign */
  if (line_skip_EqualChar (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  /* get the value */
  if (line_read_expression (l, T) == FALSE)        /* dio_lrset */
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (T->VariantTypeCode != StringTypeCode)
  {
    WARN_TYPE_MISMATCH;
    return (l);
  }
  if (var_get (v, &variant) == FALSE)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }
  /* determine starting position */
  startpos = 0;
  if (rset == TRUE && T->Length < variant.Length)
  {
    /* 
       LET A$ = "123_456" ' variant.Length = 7
       LET B$ = "789"     '      T->Length = 3
       RSET A$ = B$       '       startpos = 4
       PRINT "[";A$;"]"   ' [123_789]
     */
    startpos = variant.Length - T->Length;
  }
  /* write characters to new position */
  for (n = startpos, i = 0;
       (n < (int) variant.Length) && (i < (int) T->Length); n++, i++)
  {
    variant.Buffer[n] = T->Buffer[i];
  }
  if (var_set (v, &variant) == FALSE)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }
  /* OK  */
  RELEASE_VARIANT (T);
  RELEASE_VARIANT (&variant);

  return (l);
}

/* EOF */
