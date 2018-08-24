/***************************************************************f
  
        bwb_int.c       Line Interpretation Routines
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

static int buff_read_keyword (char *buffer, int *position, char *keyword);
static int bwb_chartype (int C);
static int char_is_varfirst (char C);
static char char_is_varhead (char C);
static int char_is_varnext (char C);
static int char_is_vartail (char C);
static int GetKeyword (LineType * l, char *Keyword);
static void internal_DEF8SUB (LineType * l);
static int is_cmd (char *name);
static int is_let (char *buffer);
static int line_read_keyword (LineType * line, char *keyword);

extern void
buff_skip_spaces (char *buffer, int *position)
{
  /*
     skip spaces in 'buffer'.
     'position' is always updated.
   */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);

  p = *position;

  while (buffer[p] == ' ')
  {
    p++;
  }

  *position = p;
}

extern void
line_skip_spaces (LineType * line)
{
   
  assert (line != NULL);
  buff_skip_spaces (line->buffer, &(line->position));        /* keep this */
}

extern void
buff_skip_eol (char *buffer, int *position)
{
  /*
     skip to the NUL (NulChar) in 'buffer'.
     always updates 'position'.
   */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);

  p = *position;

  while (buffer[p])
  {
    p++;
  }

  *position = p;
}

extern void
line_skip_eol (LineType * line)
{
   
  assert (line != NULL);
  buff_skip_eol (line->buffer, &(line->position));
}

extern int
buff_is_eol (char *buffer, int *position)
{
  /*
     determines whether 'position' is effectively at the NUL (NulChar) in 'buffer'.
     if successful then 'position' is updated and returns TRUE
     otherwise returns FALSE.
   */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buffer[p] == NulChar)
  {
    *position = p;
    return TRUE;
  }
  return FALSE;
}

extern int
line_is_eol (LineType * line)
{
   
  assert (line != NULL);
  return buff_is_eol (line->buffer, &(line->position));
}

extern int
buff_peek_char (char *buffer, int *position, char find)
{
  /*
     determine whether the next non-space character in 'buffer' is 'find'.
     if successful then returns TRUE
     otherwise returns FALSE.
     'position' is unchanged.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);

  if (find != NulChar && find != ' ')
  {
    int p;
    p = *position;

    buff_skip_spaces (buffer, &p);        /* keep this */
    if (buffer[p] == find)
    {
      return TRUE;
    }
  }
  return FALSE;
}

#if FALSE /* keep this ... */
extern int
line_peek_char (LineType * line, char find)
{
   
  assert (line != NULL);
  return buff_peek_char (line->buffer, &(line->position), find);
}
#endif

extern int
buff_peek_EqualChar (char *buffer, int *position)
{
  /*
     determine whether the next non-space character in 'buffer' is 'find'.
     if successful then returns TRUE
     otherwise returns FALSE.
     'position' is unchanged.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_peek_char (buffer, position, '=');
}

extern int
line_peek_EqualChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_peek_EqualChar (line->buffer, &(line->position));
}

extern int
buff_peek_QuoteChar (char *buffer, int *position)
{
  /*
     determine whether the next non-space character in 'buffer' is 'find'.
     if successful then returns TRUE
     otherwise returns FALSE.
     'position' is unchanged.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_peek_char (buffer, position,
                         My->CurrentVersion->OptionQuoteChar);
}

extern int
line_peek_QuoteChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_peek_QuoteChar (line->buffer, &(line->position));
}

extern int
buff_peek_LparenChar (char *buffer, int *position)
{
  /*
     determine whether the next non-space character in 'buffer' is 'find'.
     if successful then returns TRUE
     otherwise returns FALSE.
     'position' is unchanged.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_peek_char (buffer, position,
                         My->CurrentVersion->OptionLparenChar);
}

extern int
line_peek_LparenChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_peek_LparenChar (line->buffer, &(line->position));
}

extern int
buff_skip_char (char *buffer, int *position, char find)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     'find' is NOT an alphabetic (A-Z,a-z) character.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);

  if (find)
  {
    int p;

    p = *position;
    buff_skip_spaces (buffer, &p);        /* keep this */
    if (buffer[p] == find)
    {
      p++;
      *position = p;
      return TRUE;
    }
  }
  return FALSE;
}

extern int
line_skip_char (LineType * line, char find)
{
   
  assert (line != NULL);
  return buff_skip_char (line->buffer, &(line->position), find);
}

extern int
buff_skip_FilenumChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position,
                         My->CurrentVersion->OptionFilenumChar);
}

extern int
line_skip_FilenumChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_FilenumChar (line->buffer, &(line->position));
}

extern int
buff_skip_AtChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position, 
     My->CurrentVersion->OptionAtChar);
}

extern int
line_skip_AtChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_AtChar (line->buffer, &(line->position));
}

extern int
buff_skip_LparenChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position,
                         My->CurrentVersion->OptionLparenChar);
}

extern int
line_skip_LparenChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_LparenChar (line->buffer, &(line->position));
}

extern int
buff_skip_RparenChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position,
                         My->CurrentVersion->OptionRparenChar);
}

extern int
line_skip_RparenChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_RparenChar (line->buffer, &(line->position));
}

extern int
buff_skip_CommaChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position, ',');
}

extern int
line_skip_CommaChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_CommaChar (line->buffer, &(line->position));
}

extern int
buff_skip_SemicolonChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position, ';');
}

extern int
line_skip_SemicolonChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_SemicolonChar (line->buffer, &(line->position));
}

extern int
buff_skip_EqualChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position, '=');
}

extern int
line_skip_EqualChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_EqualChar (line->buffer, &(line->position));
}

extern int
buff_skip_StarChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position, '*');
}

extern int
line_skip_StarChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_StarChar (line->buffer, &(line->position));
}

extern int
buff_skip_PlusChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position, '+');
}

extern int
line_skip_PlusChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_PlusChar (line->buffer, &(line->position));
}

extern int
buff_skip_MinusChar (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is 'find'.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
   
  assert (buffer != NULL);
  assert (position != NULL);
  return buff_skip_char (buffer, position, '-');
}

extern int
line_skip_MinusChar (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_MinusChar (line->buffer, &(line->position));
}

extern char
buff_skip_seperator (char *buffer, int *position)
{
  /*
     skip the next non-space character in 'buffer' if it is a seperator (comma, semicolon, or colon).
     if successful then 'position' is updated past the character and returns the character skipped
     otherwise 'position' is unchanged and returns NulChar.
   */
  int p;
  char C;
   
  assert (buffer != NULL);
  assert (position != NULL);

  p = *position;


  buff_skip_spaces (buffer, &p);        /* keep this */
  C = buffer[p];
  switch (C)
  {
  case ',':                        /* COMMA */
  case ';':                        /* SEMICOLON */
  case ':':                        /* COLON */
    p++;
    buff_skip_spaces (buffer, &p);        /* keep this */
    *position = p;
    return C;
  }
  return NulChar;
}


extern char
line_skip_seperator (LineType * line)
{
   
  assert (line != NULL);
  return buff_skip_seperator (line->buffer, &(line->position));
}

static int
char_is_varfirst (char C)
{
  /*
     determine whether the character is allowed to be the first character of a BASIC variable name.
     if successful then returns TRUE
     otherwise returns FALSE.
   */
   
  if (C == NulChar || C == ' ')
  {
    return FALSE;                                /* never allowed */
  }
  if (bwb_isalpha (C))
  {
    return TRUE;                                /* always allowed */
  }
  /* dialect specific */
  switch (C)
  {
  case '@':
  case '#':
  case '$':
    if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
    {
      /* alphabet extenders */
      return TRUE;
    }
    break;
  }
  /* NOT FOUND */
  return FALSE;
}

static int
char_is_varnext (char C)
{
  /*
     determine whether the character is allowed to be the second character of a BASIC variable name.
     if successful then returns TRUE
     otherwise returns FALSE.
   */
   
  if (C == NulChar || C == ' ')
  {
    return FALSE;                                /* never allowed */
  }
  if (bwb_isalnum (C))
  {
    return TRUE;                                /* always allowed */
  }
  /* dialect specific */
  switch (C)
  {
  case '.':
  case '_':
    if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_ON)) /* varname: period and underscore are allowed */
    {
      return TRUE;
    }
    break;
  case '@':
  case '#':
  case '$':
    if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73)) /* alphabet extenders */
    {
      return TRUE;                            
    }
    break;
  }
  /* NOT FOUND */
  return FALSE;
}

extern char
TypeCode_to_Char (char TypeCode)
{
  /*
     Convert the internal TypeCode value into the dialect-specifc tail character.
     if successful then returns the dialect-specifc tail character
     otherwise returns NulChar.
   */
  switch (TypeCode)
  {
  case ByteTypeCode:
    return My->CurrentVersion->OptionByteChar;
  case IntegerTypeCode:
    return My->CurrentVersion->OptionIntegerChar;
  case LongTypeCode:
    return My->CurrentVersion->OptionLongChar;
  case CurrencyTypeCode:
    return My->CurrentVersion->OptionCurrencyChar;
  case SingleTypeCode:
    return My->CurrentVersion->OptionSingleChar;
  case DoubleTypeCode:
    return My->CurrentVersion->OptionDoubleChar;
  case StringTypeCode:
    return My->CurrentVersion->OptionStringChar;
  }
  /* NOT FOUND */
  return NulChar;
}

extern char
Char_to_TypeCode (char C)
{
  /*
     Convert the dialect-specifc tail character into the internal TypeCode value.
     if successful then returns the internal TypeCode value
     otherwise returns NulChar.
   */
   
  if (C == NulChar || C == ' ')
  {
    return NulChar;                                /* never allowed */
  }
  /* dialect specific */
  if (C == My->CurrentVersion->OptionByteChar)
  {
    return ByteTypeCode;
  }
  if (C == My->CurrentVersion->OptionIntegerChar)
  {
    return IntegerTypeCode;
  }
  if (C == My->CurrentVersion->OptionLongChar)
  {
    return LongTypeCode;
  }
  if (C == My->CurrentVersion->OptionCurrencyChar)
  {
    return CurrencyTypeCode;
  }
  if (C == My->CurrentVersion->OptionSingleChar)
  {
    return SingleTypeCode;
  }
  if (C == My->CurrentVersion->OptionDoubleChar)
  {
    return DoubleTypeCode;
  }
  if (C == My->CurrentVersion->OptionStringChar)
  {
    return StringTypeCode;
  }
  /* NOT FOUND */
  return NulChar;
}

extern char
var_nametype (char *name)
{
  /*
     determine the internal TypeCode associated with the vaariable name.
     if successful then returns the internal TypeCode value
     otherwise returns NulChar.
   */
   
  assert (name != NULL);

  if (name == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NulChar;
  }
  /* look only at the last charactr of the variable name */
  if (*name)
  {
    while (*name)
    {
      name++;
    }
    name--;
  }
  return Char_to_TypeCode (*name);
}

static char
char_is_varhead (char C)
{
  /*
     determine whether the character is allowed at the head of a variable name.
     if successful then returns TRUE
     otherwise retuns FALSE.
   */
   
  if (C == NulChar || C == ' ')
  {
    return NulChar;
  }                                /* never allowed */
  if (char_is_varfirst (C))
  {
    return C;
  }
  if (char_is_varnext (C))
  {
    return C;
  }
  return NulChar;
}

static int
char_is_vartail (char C)
{
  /*
     determine whether the character is allowed at the tail of a variable name.
     if successful then returns TRUE
     otherwise retuns FALSE.
   */
   
  if (C == NulChar || C == ' ')
  {
    return FALSE;                                /* never allowed */
  }
  if (char_is_varnext (C))
  {
    return TRUE;
  }
  if (Char_to_TypeCode (C))
  {
    return TRUE;
  }
  return FALSE;
}

#if FALSE /* kepp this ... */
extern int
buff_peek_word (char *buffer, int *position, char *find)
{
  /*
     determine whether the next non-space word in 'buffer' is 'find';
     the word 'find' is not allowed to be a sub-string of a bigger word.
     if successful then returns TRUE
     otherwise returns FALSE.
     'position' is unchanged.
   */
  int p;
  int n;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (find != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_is_eol (buffer, &p))
  {
    return FALSE;
  }
  n = bwb_strlen (find);
  if (bwb_strnicmp (&(buffer[p]), find, n) == 0)
  {
    if (p > 0)
    {
      if (char_is_varhead (buffer[p - 1]))
      {
        /* _TO */
        return FALSE;
      }
    }
    if (char_is_vartail (buffer[p + n]))
    {
      /* TO_ */
      return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}
#endif

#if FALSE /* keep this ... */
extern int
line_peek_word (LineType * line, char *find)
{
   
  assert (line != NULL);
  assert (find != NULL);
  return buff_peek_word (line->buffer, &(line->position), find);
}
#endif

extern int
buff_skip_word (char *buffer, int *position, char *find)
{
  /*
     skip the next non-space word in 'buffer' if it is 'find';
     the word 'find' is not a sub-string of a bigger word.
     if successful then 'position' is updated past 'find' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
  int p;
  int n;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (find != NULL);

  p = *position;


  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_is_eol (buffer, &p))
  {
    return FALSE;
  }
  n = bwb_strlen (find);
  if (bwb_strnicmp (&(buffer[p]), find, n) == 0)
  {
    if (p > 0)
    {
      if (char_is_varhead (buffer[p - 1]))
      {
        /* _TO */
        return FALSE;
      }
    }
    if (char_is_vartail (buffer[p + n]))
    {
      /* TO_ */
      return FALSE;
    }
    p += n;
    *position = p;
    return TRUE;
  }
  return FALSE;
}

extern int
line_skip_word (LineType * line, char *find)
{
   
  assert (line != NULL);
  assert (find != NULL);
  return buff_skip_word (line->buffer, &(line->position), find);
}

extern int
buff_read_varname (char *buffer, int *position, char *varname)
{
  /*
     read the next non-space word in 'buffer' that conforms to a BASIC variable name into 'varname'.
     if successful then 'position' is updated past 'varname' and returns TRUE
     otherwise 'position' is unchanged ('varname' is truncated) and returns FALSE.
     'varname' shall be declared "char varname[NameLengthMax + 1]".
   */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (varname != NULL);

  p = *position;

  buff_skip_spaces (buffer, &p);        /* keep this */
  if (char_is_varfirst (buffer[p]))
  {
    int i;
    i = 0;

    if (i > NameLengthMax)
    {
      i = NameLengthMax;
    }
    varname[i] = buffer[p];
    p++;
    i++;
    while (char_is_varnext (buffer[p]))
    {
      if (i > NameLengthMax)
      {
        i = NameLengthMax;
      }
      varname[i] = buffer[p];
      p++;
      i++;
    }
    if (Char_to_TypeCode (buffer[p]))
    {
      if (i > NameLengthMax)
      {
        i = NameLengthMax;
      }
      varname[i] = buffer[p];
      p++;
      i++;
    }
    varname[i] = NulChar;
    *position = p;
    return TRUE;
  }
  varname[0] = NulChar;
  return FALSE;
}

extern int
line_read_varname (LineType * line, char *varname)
{
   
  assert (line != NULL);
  assert (varname != NULL);
  return buff_read_varname (line->buffer, &(line->position), varname);
}

extern int
buff_read_label (char *buffer, int *position, char *label)
{
  /*
     read the next non-space word in 'buffer' that conforms to a BASIC label name into 'label'.
     if successful then 'position' is updated past 'label' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
     'label' shall be declared "char label[NameLengthMax + 1]".
   */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (label != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (char_is_varfirst (buffer[p]))
  {
    int i;
    i = 0;

    if (i > NameLengthMax)
    {
      i = NameLengthMax;
    }
    label[i] = buffer[p];
    p++;
    i++;
    while (char_is_varnext (buffer[p]))
    {
      if (i > NameLengthMax)
      {
        i = NameLengthMax;
      }
      label[i] = buffer[p];
      p++;
      i++;
    }
    label[i] = NulChar;
    *position = p;
    return TRUE;
  }
  return FALSE;
}

extern int
line_read_label (LineType * line, char *label)
{
   
  assert (line != NULL);
  assert (label != NULL);
  return buff_read_label (line->buffer, &(line->position), label);
}

static int
buff_read_keyword (char *buffer, int *position, char *keyword)
{
  /*
     read the next non-space word in 'buffer' that conforms to a BASIC keyword into 'keyword'.
     if successful then 'position' is updated past 'keyword' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
     'label' shall be declared "char keyword[NameLengthMax + 1]".
   */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (keyword != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (char_is_varfirst (buffer[p]))
  {
    int i;
    i = 0;

    if (i > NameLengthMax)
    {
      i = NameLengthMax;
    }
    keyword[i] = buffer[p];
    p++;
    i++;
    while (char_is_varnext (buffer[p]))
    {
      if (i > NameLengthMax)
      {
        i = NameLengthMax;
      }
      keyword[i] = buffer[p];
      p++;
      i++;
    }
    if (Char_to_TypeCode (buffer[p]) == StringTypeCode)
    {
      if (i > NameLengthMax)
      {
        i = NameLengthMax;
      }
      keyword[i] = buffer[p];
      p++;
      i++;
    }
    keyword[i] = NulChar;
    *position = p;
    return TRUE;
  }
  keyword[0] = NulChar;
  return FALSE;
}

static int
line_read_keyword (LineType * line, char *keyword)
{
   
  assert (line != NULL);
  assert (keyword != NULL);
  return buff_read_keyword (line->buffer, &(line->position), keyword);
}

extern VariableType *
buff_read_scalar (char *buffer, int *position)
{
  /*
     read the next non-space word in 'buffer' that conforms to a BASIC variable name,
     including both scalar variables and subscripted array variables.
     if successful then 'position' is updated 
     past 'varname' for scalar variables
     (past right parenthesis for subscripted array variables).
     and returns a pointer to the variable.
     otherwise 'position' is unchanged and returns NULL.
   */
  int p;
  VariableType *v;
  char varname[NameLengthMax + 1];
   
  assert (buffer != NULL);
  assert (position != NULL);


  p = *position;

  /* Read a variable name */
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_read_varname (buffer, &p, varname) == FALSE)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return NULL;
  }
  if (buff_peek_LparenChar (buffer, &p))
  {
    /* MUST be a an array */
    int n;
    int n_params;                /* number of parameters */
    int pp[MAX_DIMS];

    /* get parameters because the variable is dimensioned */
    if (buff_read_array_dimensions (buffer, &p, &n_params, pp) == FALSE)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return NULL;
    }
    /* get the array variable */
    if ((v = var_find (varname, n_params, TRUE)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return NULL;
    }
    for (n = 0; n < v->dimensions; n++)
    {
      v->VINDEX[n] = pp[n];
    }
  }
  else
  {
    /* simple scalar variable */

    if ((v = var_find (varname, 0, TRUE)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return NULL;
    }
  }
  *position = p;
  return v;
}

extern VariableType *
line_read_scalar (LineType * line)
{
   
  assert (line != NULL);
  return buff_read_scalar (line->buffer, &(line->position));
}

extern VariableType *
buff_read_matrix (char *buffer, int *position)
{
  /*
     read the next non-space word in 'buffer' that conforms to a BASIC matrix name,
     including both simple matrix variables and redimensioned matrix variables.
     if successful then 'position' is updated 
     past 'varname' for matrix variables
     (past right parenthesis for redimensioned matrix variables).
     and returns a pointer to the variable.
     otherwise 'position' is unchanged and returns NULL.
   */
  int p;
  VariableType *v;
  char varname[NameLengthMax + 1];
   
  assert (buffer != NULL);
  assert (position != NULL);


  p = *position;

  /* Read a variable name */
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_read_varname (buffer, &p, varname) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return NULL;
  }
  v = mat_find (varname);
  if (v == NULL)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return NULL;
  }
  if (buff_peek_LparenChar (buffer, &p))
  {
    /* get requested matrix size, which is <= original matrix size */
    size_t array_units;
    int n;
    int dimensions;
    int LBOUND[MAX_DIMS];
    int UBOUND[MAX_DIMS];

    if (buff_read_array_redim (buffer, &p, &dimensions, LBOUND, UBOUND) ==
        FALSE)
    {
      WARN_SYNTAX_ERROR;
      return NULL;
    }
    /* update array dimensions */
    array_units = 1;
    for (n = 0; n < dimensions; n++)
    {
      if (UBOUND[n] < LBOUND[n])
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return FALSE;
      }
      array_units *= UBOUND[n] - LBOUND[n] + 1;
    }
    if (array_units > v->array_units)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return FALSE;
    }
    v->dimensions = dimensions;
    for (n = 0; n < dimensions; n++)
    {
      v->LBOUND[n] = LBOUND[n];
      v->UBOUND[n] = UBOUND[n];
    }
  }
  *position = p;
  return v;
}

extern VariableType *
line_read_matrix (LineType * line)
{
   
  assert (line != NULL);
  return buff_read_matrix (line->buffer, &(line->position));
}

extern int
buff_read_line_number (char *buffer, int *position, int *linenum)
{
  /*
     read the next non-space word in 'buffer' that conforms to a BASIC line number.
     if successful then 'position' is updated past 'linenum' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (linenum != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (bwb_isdigit (buffer[p]))
  {
    int i;
    int n;
    char label[NameLengthMax + 1];

    i = 0;
    while (bwb_isdigit (buffer[p]))
    {
      if (i > NameLengthMax)
      {
        i = NameLengthMax;
      }
      label[i] = buffer[p];
      p++;
      i++;
    }
    label[i] = NulChar;
    n = atoi (label);
    *linenum = n;
    *position = p;
    return TRUE;
  }
  return FALSE;
}

extern int
line_read_line_number (LineType * line, int *linenum)
{
   
  assert (line != NULL);
  assert (linenum != NULL);
  return buff_read_line_number (line->buffer, &(line->position), linenum);
}

extern int
buff_read_line_sequence (char *buffer, int *position, int *head, int *tail)
{
  /*
     read the next non-space words in 'buffer' that conforms to a BASIC line number sequnence.
     if successful then 'position' is updated past the line number sequence and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
  /*
     ###          head == tail
     ### -        head to BMAX
     ### - ###    head to tail
     - ###    BMIN to tail
   */
  int p;                        /* position */
  int h;                        /* head */
  int t;                        /* tail */
  char c;                        /* line range seperator for BREAK, DELETE and LIST */
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (head != NULL);
  assert (tail != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  c = '-';
  if (My->CurrentVersion->OptionVersionValue & (D70 | H80))
  {
    c = ',';
  }

  if (buff_skip_char (buffer, &p, c) /* line sequence seperator */ )
  {
    /* - ... */
    if (buff_read_line_number (buffer, &p, &t))
    {
      /* - ### */
      *head = MINLIN;
      *tail = t;
      *position = p;
      return TRUE;
    }
  }
  else
    if (buff_read_line_number (buffer, &p, &h) /* line sequence seperator */ )
  {
    /* ### ... */
    if (buff_skip_char (buffer, &p, c))
    {
      /* ### - ... */
      if (buff_read_line_number (buffer, &p, &t))
      {
        /* ### - ### */
        *head = h;
        *tail = t;
        *position = p;
        return TRUE;
      }
      else
      {
        /* ### - */
        *head = h;
        *tail = MAXLIN;
        *position = p;
        return TRUE;
      }
    }
    else
    {
      /* ### */
      *head = h;
      *tail = h;
      *position = p;
      return TRUE;
    }
  }
  return FALSE;
}

extern int
line_read_line_sequence (LineType * line, int *head, int *tail)
{
   
  assert (line != NULL);
  assert (head != NULL);
  assert (tail != NULL);
  return buff_read_line_sequence (line->buffer, &(line->position), head,
                                  tail);
}

extern int
buff_read_integer_expression (char *buffer, int *position, int *Value)
{
  /*
     read the next non-space words in 'buffer' that conforms to a BASIC integer expression into 'Value'.
     if successful then 'position' is updated past 'Value' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
     'Value' shall be declared "int Value".
   */
  DoubleType X;
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (Value != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_read_numeric_expression (buffer, &p, &X))
  {
    /* we want the rounded value */
    X = bwb_rint (X);
    if (INT_MIN <= X && X <= INT_MAX)
    {
      /* OK */
      *Value = (int) bwb_rint (X);
      *position = p;
      return TRUE;
    }
  }
  /* ERROR */
  return FALSE;
}

extern int
line_read_integer_expression (LineType * line, int *Value)
{
   
  assert (line != NULL);
  assert (Value != NULL);
  return buff_read_integer_expression (line->buffer, &(line->position),
                                       Value);
}


extern int
buff_read_numeric_expression (char *buffer, int *position, DoubleType * Value)
{
  /*
     read the next non-space words in 'buffer' that conforms to a BASIC numeric expression into 'Value'.
     if successful then 'position' is updated past 'Value' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
     'Value' shall be declared "DoubleType Value".
   */
  int p;
  VariantType x;
  VariantType *X;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (Value != NULL);

  X = &x;
  p = *position;
  CLEAR_VARIANT (X);

  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_read_expression (buffer, &p, X) == FALSE)        /* buff_read_numeric_expression */
  {
    return FALSE;
  }
  if (X->VariantTypeCode != StringTypeCode)
  {
    /* OK */
    *Value = X->Number;
    *position = p;
    return TRUE;
  }
  RELEASE_VARIANT (X);
  return FALSE;
}

extern int
line_read_numeric_expression (LineType * line, DoubleType * Value)
{
   
  assert (line != NULL);
  assert (Value != NULL);
  return buff_read_numeric_expression (line->buffer, &(line->position),
                                       Value);
}

extern int
buff_read_string_expression (char *buffer, int *position, char **Value)
{
  /*
     read the next non-space words in 'buffer' that conforms to a BASIC string expression into 'Value'.
     if successful then 'position' is updated past 'Value' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
     'Value' shall be declared "char * Value = NULL".
   */
  int p;
  VariantType x;
  VariantType *X;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (Value != NULL);

  X = &x;
  p = *position;
  CLEAR_VARIANT (X);

  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_read_expression (buffer, &p, X) == FALSE)        /* buff_read_string_expression */
  {
    return FALSE;
  }
  if (X->VariantTypeCode == StringTypeCode)
  {
    /* OK */
    X->Buffer[X->Length] = NulChar;
    *Value = X->Buffer;
    *position = p;
    return TRUE;
    /* the caller is responsible to free() the returned pointer */
  }
  return FALSE;
}

extern int
line_read_string_expression (LineType * line, char **Value)
{
   
  assert (line != NULL);
  assert (Value != NULL);
  return buff_read_string_expression (line->buffer, &(line->position), Value);
}

extern int
buff_read_index_item (char *buffer, int *position, int Index, int *Value)
{
  /*
     read the next non-space words in 'buffer' that conforms to a BASIC integer expression list into 'Value',
     selecting the item matching 'Index'.  The first 'Index' value is one; 
     if successful then 'position' is updated past 'Value' and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
     'Value' shall be declared "int Value".
   */
  int p;
  int i;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (Value != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_is_eol (buffer, &p))
  {
    return FALSE;
  }
  if (Index < 1)
  {
    return FALSE;
  }
  /* Index >= 1 */
  i = 0;
  do
  {
    int v;

    if (buff_read_integer_expression (buffer, &p, &v))
    {
      i++;
      if (i == Index)
      {
        *Value = v;
        *position = p;
        return TRUE;
      }
    }
    else
    {
      return FALSE;
    }
  }
  while (buff_skip_seperator (buffer, &p));
  return FALSE;
}

extern int
line_read_index_item (LineType * line, int Index, int *Value)
{
   
  assert (line != NULL);
  assert (Value != NULL);
  return buff_read_index_item (line->buffer, &(line->position), Index, Value);
}


extern int
buff_read_letter_sequence (char *buffer, int *position, char *head,
                           char *tail)
{
  /*
     read the next non-space alphabetic character in 'buffer' into 'start';
     if seperated by a hyphen ('-') then read the next non-space alphabetic character into 'end'.
     if successful then 'position' is updated past 'start' (or 'end') and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
  int p;
  char h;
  char t;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (head != NULL);
  assert (tail != NULL);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (bwb_isalpha (buffer[p]) == FALSE)
  {
    /* character at this position must be a letter */
    return FALSE;
  }
  h = buffer[p];
  p++;

  /* check for hyphen, indicating sequence of more than one letter */
  if (buff_skip_MinusChar (buffer, &p))
  {
    buff_skip_spaces (buffer, &p);        /* keep this */
    if (bwb_isalpha (buffer[p]) == FALSE)
    {
      /* character at this position must be a letter */
      return FALSE;
    }
    t = buffer[p];
    p++;
  }
  else
  {
    t = h;
  }
  *head = h;
  *tail = t;
  *position = p;
  return TRUE;
}

extern int
line_read_letter_sequence (LineType * line, char *head, char *tail)
{
   
  assert (line != NULL);
  assert (head != NULL);
  assert (tail != NULL);
  return buff_read_letter_sequence (line->buffer, &(line->position), head,
                                    tail);
}

extern int
buff_read_array_dimensions (char *buffer, int *position, int *n_params,
                            int params[ /* MAX_DIMS */ ])
{
  /*
     read the next non-space words in 'buffer' that conform to BASIC array index values;
     if successful then 'position' is updated past the right parenthesis and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
  int p;
  int n;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (n_params != NULL);
  assert (params != NULL);

  p = *position;
  n = 0;

  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_skip_LparenChar (buffer, &p))
  {
    /* matrix */
    do
    {
      int Value;

      if (n >= MAX_DIMS)
      {
        /* ERROR */
        return FALSE;
      }
      /* OK */
      if (buff_read_integer_expression (buffer, &p, &Value) == FALSE)
      {
        /* ERROR */
        return FALSE;
      }
      /* OK */
      params[n] = Value;
      n++;
    }
    while (buff_skip_seperator (buffer, &p));

    if (buff_skip_RparenChar (buffer, &p) == FALSE)
    {
      /* ERROR */
      return FALSE;
    }
  }
  else
  {
    /* scalar */
    n = 0;
  }
  *n_params = n;
  *position = p;
  return TRUE;
}

#if FALSE /* keep this ... */
extern int
line_read_array_dimensions (LineType * line, int *n_params,
                            int params[ /* MAX_DIMS */ ])
{
   
  assert (line != NULL);
  assert (n_params != NULL);
  assert (params != NULL);
  return buff_read_array_dimensions (line->buffer, &(line->position),
                                     n_params, params);
}
#endif

extern int
buff_read_array_redim (char *buffer, int *position, int *dimensions,
                       int LBOUND[ /* MAX_DIMS */ ],
                       int UBOUND[ /* MAX_DIMS */ ])
{
  /*
     read the next non-space words in 'buffer' that conform to BASIC array index values;
     if successful then 'position' is updated past the right parenthesis and returns TRUE
     otherwise 'position' is unchanged and returns FALSE.
   */
  int p;
  int n;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (dimensions != NULL);
  assert (LBOUND != NULL);
  assert (UBOUND != NULL);

  p = *position;
  n = 0;

  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_skip_LparenChar (buffer, &p))
  {
    /* matrix */
    do
    {
      int Value;

      if (n >= MAX_DIMS)
      {
        /* ERROR */
        return FALSE;
      }
      /* OK */
      if (buff_read_integer_expression (buffer, &p, &Value) == FALSE)
      {
        /* ERROR */
        return FALSE;
      }
      /* OK */
      if (buff_skip_word (buffer, &p, "TO") == TRUE)
      {
        LBOUND[n] = Value;        /* explicit lower bound */
        if (buff_read_integer_expression (buffer, &p, &Value) == FALSE)
        {
          /* ERROR */
          return FALSE;
        }
        /* OK */
        UBOUND[n] = Value;        /* explicit upper bound */
      }
      else
      {
        LBOUND[n] = My->CurrentVersion->OptionBaseInteger;        /* implicit lower bound */
        UBOUND[n] = Value;        /* explicit upper bound */
      }
      n++;
    }
    while (buff_skip_seperator (buffer, &p));

    if (buff_skip_RparenChar (buffer, &p) == FALSE)
    {
      /* ERROR */
      return FALSE;
    }
  }
  else
  {
    /* scalar */
    n = 0;
  }
  *dimensions = n;
  *position = p;
  return TRUE;
}

extern int
line_read_array_redim (LineType * line, int *dimensions,
                       int LBOUND[ /* MAX_DIMS */ ],
                       int UBOUND[ /* MAX_DIMS */ ])
{
   
  assert (line != NULL);
  assert (dimensions != NULL);
  assert (LBOUND != NULL);
  assert (UBOUND != NULL);
  return buff_read_array_redim (line->buffer, &(line->position), dimensions,
                                LBOUND, UBOUND);
}

extern int
buff_peek_array_dimensions (char *buffer, int *position, int *n_params)
{
  /*
     peek the next non-space words in 'buffer' that conform to BASIC array index values;
     if successful then 'n_params' is updated and returns TRUE
     otherwise 'n_params' is unchanged and returns FALSE.
     'position' is always unchanged.
   */
  int p;
  int ParenLevel;
  int NumDimensions;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (n_params != NULL);

  ParenLevel = 0;
  NumDimensions = 1;
  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  while (buffer[p])
  {
    /* check the current character */
    buff_skip_spaces (buffer, &p);        /* keep this */
    if (buff_skip_LparenChar (buffer, &p))
    {
      ParenLevel++;
    }
    else if (buff_skip_RparenChar (buffer, &p))
    {
      ParenLevel--;
      if (ParenLevel < 0)
      {
        return FALSE;
      }
      if (ParenLevel == 0)
      {
        *n_params = NumDimensions;
        return TRUE;
      }
    }
    else if (buff_skip_seperator (buffer, &p))
    {
      if (ParenLevel == 1)
      {
        NumDimensions++;
      }
    }
    else if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
    {
      /* embedded string constant */
      p++;
      while ((buffer[p] != My->CurrentVersion->OptionQuoteChar)
             && (buffer[p] != NulChar))
      {
        p++;
      }
      if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
      {
        p++;
      }
    }
    else
    {
      /* normal character */
      p++;
    }
  }
  return FALSE;
}

#if FALSE /* keep this ... */
extern int
line_peek_array_dimensions (LineType * line, int *n_params)
{
   
  assert (line != NULL);
  assert (n_params != NULL);
  return buff_peek_array_dimensions (line->buffer, &(line->position),
                                     n_params);
}
#endif

extern char
buff_read_type_declaration (char *buffer, int *position)
{
  /*
     skip the next non-space words in 'buffer' if it is a BASIC type declaration.
     if successful then 'position' is updated past the BASIC type declaration and returns the TypeCode
     otherwise 'position' is unchanged and returns NulChar.
   */
  int p;
  char TypeCode;
   
  assert (buffer != NULL);
  assert (position != NULL);


  p = *position;
  TypeCode = NulChar;


  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_is_eol (buffer, &p))
  {
    return TypeCode;
  }

  if (buff_skip_word (buffer, &p, "AS") == TRUE)
  {
    /* AS ... */

    if (buff_skip_word (buffer, &p, "BYTE"))
    {
      /* AS BYTE */
      TypeCode = ByteTypeCode;
    }
    else if (buff_skip_word (buffer, &p, "INTEGER"))
    {
      /* AS INTEGER */
      TypeCode = IntegerTypeCode;
    }
    else if (buff_skip_word (buffer, &p, "LONG"))
    {
      /* AS LONG */
      TypeCode = LongTypeCode;
    }
    else if (buff_skip_word (buffer, &p, "CURRENCY"))
    {
      /* AS CURRENCY */
      TypeCode = CurrencyTypeCode;
    }
    else if (buff_skip_word (buffer, &p, "SINGLE"))
    {
      /* AS SINGLE */
      TypeCode = SingleTypeCode;
    }
    else if (buff_skip_word (buffer, &p, "DOUBLE"))
    {
      /* AS DOUBLE */
      TypeCode = DoubleTypeCode;
    }
    else if (buff_skip_word (buffer, &p, "STRING"))
    {
      /* AS STRING */
      TypeCode = StringTypeCode;
    }
    else
    {
      /* invalid type */
    }
  }
  if (TypeCode)
  {
    /* success */
    *position = p;
  }
  return TypeCode;
}

extern char
line_read_type_declaration (LineType * line)
{
   
  assert (line != NULL);
  return buff_read_type_declaration (line->buffer, &(line->position));
}



/***************************************************************
  
        FUNCTION:       line_start()
  
        DESCRIPTION:    This function reads a line buffer in
                        <buffer> beginning at the position
                        <pos> and attempts to determine (a)
                        the position of the line number in the
                        buffer (returned in <lnpos>), (b) the
                        line number at this position (returned
                        in <lnum>), (c) the position of the
                        BASIC command in the buffer (returned
                        in <cmdpos>), (d) the position of this
                        BASIC command in the command table
                        (returned in <cmdnum>), and (e) the
                        position of the beginning of the rest
                        of the line (returned in <Startpos>).
                        Although <Startpos> must be returned
                        as a positive integer, the other
                        searches may fail, in which case FALSE
                        will be returned in their positions.
                        <pos> is not incremented.
  
***************************************************************/
static void
internal_DEF8SUB (LineType * l)
{
  /* 
   **
   ** User is executing a function as though it were a command, such as 100 COS X.
   ** This applies to both intrinsic functions and user defined functions and subroutines.
   ** No special parsing is required, just insert "CALL" before the name and 
   ** add parentheses around the parameters:
   **   100 fna 1,2,3  -->> 100 CALL fna(1,2,3)
   **
   */
  const char *A = "CALL ";
  int a;
  int n;
  char *buffer;
   
  assert (l != NULL);

  a = bwb_strlen (A);
  n = bwb_strlen (l->buffer) + a /* "CALL " */  + 1 /* '(' */  + 1 /* ')' */ ;
  buffer = calloc (n + 1 /* NulChar */ , sizeof (char));
  if (buffer == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return;
  }
  bwb_strcpy (buffer, A);
  /* buffer == "CALL " */
  l->position = 0;
  if (line_read_varname (l, &(buffer[a])) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return;
  }
  /* buffer == "CALL name" */
  line_skip_spaces (l);
  if (line_is_eol (l))
  {
    /* buffer == "CALL name" */
  }
  else
  {
    /* buffer == "CALL name" */
    bwb_strcat (buffer, "(");
    /* buffer == "CALL name(" */
    bwb_strcat (buffer, &(l->buffer[l->position]));
    /* buffer == "CALL name(...parameters..." */
    bwb_strcat (buffer, ")");
    /* buffer == "CALL name(...parameters...)" */
  }
  /*
     printf("%s\n", buffer );
   */
  free (l->buffer);
  l->buffer = buffer;
  l->position = a;
  l->Startpos = a;
  l->cmdnum = C_CALL;
}
extern void
line_start (LineType * l)
{
  char tbuf[NameLengthMax + 1];
   
  assert (l != NULL);


  /* set initial values */
  l->cmdnum = 0;                /* NOT FOUND */
  l->Startpos = 0;
  l->position = 0;

  line_skip_spaces (l);                /* keep this */

  /* handle special cases */
  if (line_is_eol (l))
  {
    /* the NUL (0) char must be handled first */
    l->cmdnum = C_REM;
    return;
  }
  if (line_skip_char (l, My->CurrentVersion->OptionCommentChar))
  {
    line_skip_eol (l);
    l->Startpos = l->position;
    l->cmdnum = C_REM;
    return;
  }
  if (line_skip_char (l, My->CurrentVersion->OptionPrintChar))
  {
    line_skip_spaces (l);        /* keep this */
    l->Startpos = l->position;
    l->cmdnum = C_PRINT;
    return;
  }
  if (line_skip_char (l, My->CurrentVersion->OptionInputChar))
  {
    line_skip_spaces (l);        /* keep this */
    l->Startpos = l->position;
    l->cmdnum = C_INPUT;
    return;
  }
  if (line_skip_char (l, My->CurrentVersion->OptionImageChar))
  {
    line_skip_spaces (l);        /* keep this */
    l->Startpos = l->position;
    l->cmdnum = C_IMAGE;
    return;
  }
  if (bwb_strnicmp (&l->buffer[l->position], "REM", 3) == 0)
  {
    line_skip_eol (l);
    l->Startpos = l->position;
    l->cmdnum = C_REM;
    return;
  }
  /* not a SPECIAL */

  /* get the first keyword */
  if (line_read_keyword (l, tbuf) == FALSE)
  {
    /* ERROR */
    return;
  }
  line_skip_spaces (l);                /* keep this */


  /*
   **
   ** check for COMMAND
   **
   */
  l->cmdnum = is_cmd (tbuf);
  if (l->cmdnum)
  {
    /*
     ** 
     ** NOTE:  This is NOT a full parser, this exists only to 
     ** handle STRUCTURED commands. It is true that we also handle 
     ** some other easy cases, but remember that this only exists 
     ** to support STRUCTURED commands. Whether any other commands 
     ** get processed here is simply because it was easy to do so. 
     **
     */

    int cmdnum;
    char *xbuf;
    int xlen;

    cmdnum = 0;
    xbuf = My->ConsoleInput;
    xlen = MAX_LINE_LENGTH;
    bwb_strcpy (xbuf, tbuf);

    do
    {
      cmdnum = 0;
      l->Startpos = l->position;
      if (line_read_keyword (l, tbuf))
      {
        int n;
        n = bwb_strlen (xbuf) + 1 /* SpaceChar */  + bwb_strlen (tbuf);
        if (n < xlen)
        {
          /* not too long */
          bwb_strcat (xbuf, " ");
          bwb_strcat (xbuf, tbuf);
          cmdnum = is_cmd (xbuf);
          if (cmdnum)
          {
            /* longer command is valid */
            line_skip_spaces (l);        /* keep this */
            l->Startpos = l->position;
            l->cmdnum = cmdnum;
          }
        }
      }
    }
    while (cmdnum);
    /*
     **
     ** process special cases here
     **
     */
    l->position = l->Startpos;
    switch (l->cmdnum)
    {
    case C_CLOAD:
      {
        if (line_skip_StarChar (l))
        {
          /*
           **
           ** CLOAD*
           **
           */
          line_skip_spaces (l);        /* keep this */
          l->Startpos = l->position;
          l->cmdnum = C_CLOAD8;
        }
      }
      break;
    case C_CSAVE:
      {
        if (line_skip_StarChar (l))
        {
          /*
           **
           ** CSAVE*
           **
           */
          line_skip_spaces (l);        /* keep this */
          l->Startpos = l->position;
          l->cmdnum = C_CSAVE8;
        }
      }
      break;
    case C_DEF:
      if (bwb_strchr (l->buffer, '=') == NULL)
      {
        /*
         **
         ** multi-line DEF ... FNEND
         **
         */
        l->cmdnum = C_FUNCTION;
      }
      /*
       **
       ** we look up declared USER functions as we load
       **
       */
      UserFunction_add (l);
      break;
    case C_FEND:
      /*
       **
       ** this makes bwb_scan() simpler
       **
       */
      l->cmdnum = C_END_FUNCTION;
      break;
    case C_FNEND:
      /*
       **
       ** this makes bwb_scan() simpler
       **
       */
      l->cmdnum = C_END_FUNCTION;
      break;
    case C_FUNCTION:
      /*
       **
       ** we look up declared USER functions as we load
       **
       */
      UserFunction_add (l);
      break;
    case C_IF:
      /*
       **
       ** CLASSIC vs STRUCTURED
       **
       */
      if (IsLastKeyword (l, " THEN"))
      {
        /*
         **
         ** STRUCTURED 
         **
         */
        l->cmdnum = C_IF8THEN;
      }
      break;
    case C_OPEN:
      /*
       **
       ** CLASSIC vs STRUCTURED
       **
       */
      if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73 | D71))
      {
        /*
         **
         ** STRUCTURED 
         **
         */
        /* OPEN filenum, filename$, INPUT | OUTPUT */
      }
      else if (GetKeyword (l, " AS "))
      {
        /*
         **
         ** STRUCTURED 
         **
         */
        /* OPEN ... AS ... */
      }
      else
      {
        /*
         **
         ** CLASSIC 
         **
         */
        /*
           l->cmdnum = C_DEF8SUB;
           l->Startpos = 0;
         */
        internal_DEF8SUB (l);
      }
      break;
    case C_SUB:
      /*
       **
       ** we look up declared USER functions as we load
       **
       */
      UserFunction_add (l);
      break;
    case C_SUBEND:
    case C_SUB_END:
      /*
       **
       ** this makes bwb_scan() simpler
       **
       */
      l->cmdnum = C_END_SUB;
      break;
    case C_SUBEXIT:
    case C_SUB_EXIT:
      /*
       **
       ** this makes bwb_scan() simpler
       **
       */
      l->cmdnum = C_EXIT_SUB;
      break;
    case C_DEF8LBL:
      /*
       **
       ** we look up declared USER functions as we load
       **
       */
      UserFunction_add (l);
      break;
    }
    return;
  }
  /* not a COMMAND */

  /*
   **
   ** check for implied LET
   **
   */
  if (is_let (l->buffer))
  {
    /*
     **
     ** this is an implied LET, such as:
     ** 100 A = 123
     **
     */
    l->Startpos = 0;
    l->cmdnum = C_LET;
    return;
  }
  /* not an implied LET */

  /*
   **
   ** check for FUNCTION called as a SUBROUTINE
   **
   */
  if (UserFunction_name (tbuf) || IntrinsicFunction_name (tbuf))
  {
    /*
     **
     ** check for a bogus assignment to a FUNCTION called as a SUBROUTINE, such as:
     ** 100 COS = X
     **
     */
    if (line_peek_EqualChar (l))
    {
      /* SYNTAX ERROR */
      l->cmdnum = 0;
      return;
    }
    /*
     **
     ** FUNCTION called as a SUBROUTINE, such as:
     ** 100 OUT X, Y
     **
     */
    /*
       l->Startpos = 0;
       l->cmdnum = C_DEF8SUB;
     */
    internal_DEF8SUB (l);
    return;
  }
  /* not a FUNCTION */

  /*
   **
   ** check for LABEL
   **
   */
  if (My->CurrentVersion->OptionFlags & OPTION_LABELS_ON)        /* labels are enabled */
    if (My->CurrentVersion->OptionStatementChar)        /* a Statement seperator exists */
      if (line_skip_char (l, My->CurrentVersion->OptionStatementChar))        /* this is a label */
        if (line_is_eol (l))        /* we finish the line */
        {
          /*
           **
           ** LABEL, such as:
           ** 100 MyLabel: 
           **
           */
          l->Startpos = l->position;
          l->cmdnum = C_DEF8LBL;
          return;
        }
  /* not a LABEL */


  /* SYNTAX ERROR */
  l->cmdnum = 0;
  return;
}


/***************************************************************
  
        FUNCTION:       is_cmd()
  
   DESCRIPTION:    This function determines whether the
         string in 'buffer' is a BASIC command
         statement, returning 'id' or 0.
         
  
***************************************************************/

static int
is_cmd (char *name)
{
  int i;
   
  assert (name != NULL);


#if THE_PRICE_IS_RIGHT
  /* start with the closest command, without going over */
  i = VarTypeIndex (name[0]);
  if (i < 0)
  {
    /* non-alpha, all commands start with an alpha character */
    /* NOT FOUND */
    return 0;
  }
  i = My->CommandStart[i];        /* first command starting with this letter */
  if (i < 0)
  {
    /* no command starts with that letter */
    /* NOT FOUND */
    return 0;
  }
#else /* THE_PRICE_IS_RIGHT */
  i = 0;
#endif /* THE_PRICE_IS_RIGHT */
  for (; i < NUM_COMMANDS; i++)
  {
    if (My->CurrentVersion->OptionVersionValue & IntrinsicCommandTable[i].
        OptionVersionBitmask)
    {
      int result;
      result = bwb_stricmp (IntrinsicCommandTable[i].name, name);
      if (result == 0)
      {
        /* FOUND */
        return IntrinsicCommandTable[i].CommandID;
      }
      if (result > 0 /* found > searched */ )
      {
        /* NOT FOUND */
        return 0;
      }
      /* result < 0 : found < searched */
    }
  }
  /* NOT FOUND */
  return 0;
}

static int
is_let (char *buffer)
{
  /*
   **
   ** returns TRUE if 'buffer' contains an implied LET statement,
   ** which is detected by an unquoted '='
   **
   */
  int n;
   
  assert (buffer != NULL);

  /* Go through the expression and search for an unquoted assignment operator. */

  for (n = 0; buffer[n]; n++)
  {
    if (buffer[n] == '=')
    {
      return TRUE;
    }
    if (buffer[n] == My->CurrentVersion->OptionQuoteChar)
    {
      /* string constant */
      n++;
      while (buffer[n] != My->CurrentVersion->OptionQuoteChar)
      {
        n++;
        if (buffer[n] == NulChar)
        {
          WARN_SYNTAX_ERROR;
          return FALSE;
        }
      }
      n++;
    }
  }

  /* No command name was found */

  return FALSE;

}


extern int
bwb_freeline (LineType * l)
{
  /*
   **
   ** free memory associated with a program line
   **
   */
   

  if (l != NULL)
  {
    /* free arguments if there are any */
    if (l->buffer != NULL)
    {
      free (l->buffer);
      l->buffer = NULL;
    }
    free (l);
    /* l = NULL; */
    My->IsScanRequired = TRUE;        /* program needs to be scanned again */
  }
  return TRUE;
}

static int
GetKeyword (LineType * l, char *Keyword)
{
  /*
   *
   *  Returns TRUE if Keyword is found unquoted
   *
   */
  char *S;
  char *C;
  int n;
   
  assert (l != NULL);
  assert (Keyword != NULL);

  S = l->buffer;
  S += l->position;
  C = S;
  n = bwb_strlen (Keyword);


  while (*C)
  {
    if (bwb_strnicmp (C, Keyword, n) == 0)
    {
      /* FOUND */
      return TRUE;
    }
    else if (*C == My->CurrentVersion->OptionQuoteChar)
    {
      /* skip string constant */
      C++;
      while (*C != NulChar && *C != My->CurrentVersion->OptionQuoteChar)
      {
        C++;
      }
      if (*C == My->CurrentVersion->OptionQuoteChar)
      {
        C++;
      }
    }
    else
    {
      /* skip normal character */
      C++;
    }
  }
  /* NOT FOUND */
  return FALSE;
}

extern int
IsLastKeyword (LineType * l, char *Keyword)
{
  /* find the end of the line */
  /* backup thru spaces */
  int n;
  char *S;
  char *C;
   
  assert (l != NULL);
  assert (Keyword != NULL);


  S = l->buffer;
  S += l->position;
  C = S;
  n = bwb_strlen (Keyword);
  S += n;
  /*
   ** IF x THEN  0 
   ** IF x THEN 
   */
  while (*C)
  {
    /* skip string constants */
    if (*C == My->CurrentVersion->OptionQuoteChar)
    {
      /* skip leading quote */
      C++;
      while (*C != NulChar && *C != My->CurrentVersion->OptionQuoteChar)
      {
        C++;
      }
      /* skip trailing quote */
      if (*C == My->CurrentVersion->OptionQuoteChar)
      {
        C++;
      }
    }
    else
    {
      C++;
    }
  }
  if (C > S)
  {
    C--;
    while (C > S && *C == ' ')
    {
      C--;
    }
    C++;
    if (C > S)
    {
      C -= n;
      if (bwb_strnicmp (C, Keyword, n) == 0)
      {
        /* FOUND */
        return TRUE;
      }
    }
  }
  /* NOT FOUND */
  return FALSE;
}

/* bitmask values returned by bwb_chartype() */
#define CHAR_IS_CNTRL  0x01
#define CHAR_IS_SPACE  0x02
#define CHAR_IS_PRINT  0x04
#define CHAR_IS_PUNCT  0x08
#define CHAR_IS_DIGIT  0x10
#define CHAR_IS_XDIGIT 0x20
#define CHAR_IS_UPPER  0x40
#define CHAR_IS_LOWER  0x80

#define CHAR_IS_ALPHA  (CHAR_IS_UPPER | CHAR_IS_LOWER)
#define CHAR_IS_ALNUM  (CHAR_IS_ALPHA | CHAR_IS_DIGIT)
#define CHAR_IS_GRAPH  (CHAR_IS_ALNUM | CHAR_IS_PUNCT)


static int
bwb_chartype (int C)
{
  /* returns the the character type bitmask */
   
  switch (C)
  {
  case EOF:
    return 0;                        /* Special Case */
  case '\t':
  case '\n':
  case '\v':
  case '\f':
  case '\r':
    return CHAR_IS_CNTRL | CHAR_IS_SPACE;
  case ' ':
    return CHAR_IS_PRINT | CHAR_IS_SPACE;
  case '!':
  case '"':
  case '#':
  case '$':
  case '%':
  case '&':
  case '\'':
  case '(':
  case ')':
  case '*':
  case '+':
  case ',':
  case '-':
  case '.':
  case '/':
    return CHAR_IS_PRINT | CHAR_IS_PUNCT;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return CHAR_IS_PRINT | CHAR_IS_DIGIT | CHAR_IS_XDIGIT;
  case ':':
  case ';':
  case '<':
  case '=':
  case '>':
  case '?':
  case '@':
    return CHAR_IS_PRINT | CHAR_IS_PUNCT;
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
    return CHAR_IS_PRINT | CHAR_IS_UPPER | CHAR_IS_XDIGIT;
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
    return CHAR_IS_PRINT | CHAR_IS_UPPER;
  case '[':
  case '\\':
  case ']':
  case '^':
  case '_':
  case '`':
    return CHAR_IS_PRINT | CHAR_IS_PUNCT;
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
    return CHAR_IS_PRINT | CHAR_IS_LOWER | CHAR_IS_XDIGIT;
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':
    return CHAR_IS_PRINT | CHAR_IS_LOWER;
  case '{':
  case '|':
  case '}':
  case '~':
    return CHAR_IS_PRINT | CHAR_IS_PUNCT;
  }
  return CHAR_IS_CNTRL;
}

extern int
bwb_isalnum (int C)
{
  /*
     4.3.1.1 The isalnum function

     Synopsis

     #include <ctype.h>
     int isalnum(int c);

     Description

     The isalnum function tests for any character for which isalpha or
     isdigit is true.
   */
   
  if (bwb_chartype (C) & CHAR_IS_ALNUM)
  {
    return TRUE;
  }
  return FALSE;
}

int
bwb_isalpha (int C)
{
  /*
     4.3.1.2 The isalpha function

     Synopsis

     #include <ctype.h>
     int isalpha(int c);

     Description

     The isalpha function tests for any character for which isupper or
     islower is true, or any of an implementation-defined set of characters
     for which none of iscntrl , isdigit , ispunct , or isspace is true.
     In the C locale, isalpha returns true only for the characters for
     which isupper or islower is true.
   */
   
  if (bwb_chartype (C) & CHAR_IS_ALPHA)
  {
    return TRUE;
  }
  return FALSE;
}

#if FALSE /* keep this ... */
extern int
bwb_iscntrl (int C)
{
  /*
     4.3.1.3 The iscntrl function

     Synopsis

     #include <ctype.h>
     int iscntrl(int c);

     Description

     The iscntrl function tests for any control character.  
   */
   
  if (bwb_chartype (C) & CHAR_IS_CNTRL)
  {
    return TRUE;
  }
  return FALSE;
}
#endif

extern int
bwb_isdigit (int C)
{
  /*
     4.3.1.4 The isdigit function

     Synopsis

     #include <ctype.h>
     int isdigit(int c);

     Description

     The isdigit function tests for any decimal-digit character (as
     defined in $2.2.1).
   */
   
  if (bwb_chartype (C) & CHAR_IS_DIGIT)
  {
    return TRUE;
  }
  return FALSE;
}

extern int
bwb_isgraph (int C)
{
  /*
     4.3.1.5 The isgraph function

     Synopsis

     #include <ctype.h>
     int isgraph(int c);

     Description

     The isgraph function tests for any printing character except space (' ').  
   */
   
  if (bwb_chartype (C) & CHAR_IS_GRAPH)
  {
    return TRUE;
  }
  return FALSE;
}

#if FALSE /* keep this ... */
extern int
bwb_islower (int C)
{
  /*
     4.3.1.6 The islower function

     Synopsis

     #include <ctype.h>
     int islower(int c);

     Description

     The islower function tests for any lower-case letter or any of an
     implementation-defined set of characters for which none of iscntrl ,
     isdigit , ispunct , or isspace is true.  In the C locale, islower
     returns true only for the characters defined as lower-case letters (as
     defined in $2.2.1).
   */
   
  if (bwb_chartype (C) & CHAR_IS_LOWER)
  {
    return TRUE;
  }
  return FALSE;
}
#endif

extern int
bwb_isprint (int C)
{
  /*
     4.3.1.7 The isprint function

     Synopsis

     #include <ctype.h>
     int isprint(int c);

     Description

     The isprint function tests for any printing character including
     space (' ').
   */
   
  if (bwb_chartype (C) & CHAR_IS_PRINT)
  {
    return TRUE;
  }
  return FALSE;
}

extern int
bwb_ispunct (int C)
{
  /*
     4.3.1.8 The ispunct function

     Synopsis

     #include <ctype.h>
     int ispunct(int c);

     Description

     The ispunct function tests for any printing character except space
     (' ') or a character for which isalnum is true.
   */
   
  if (bwb_chartype (C) & CHAR_IS_PUNCT)
  {
    return TRUE;
  }
  return FALSE;
}

#if FALSE /* keep this ... */
extern int
bwb_isspace (int C)
{
  /*
     4.3.1.9 The isspace function

     Synopsis

     #include <ctype.h>
     int isspace(int c);

     Description

     The isspace function tests for the standard white-space characters
     or for any of an implementation-defined set of characters for which
     isalnum is false.  The standard white-space characters are the
     following: space (' '), form feed ('\f'), new-line ('\n'), carriage
     return ('\r'), horizontal tab ('\t'), and vertical tab ('\v').  In the
     C locale, isspace returns true only for the standard white-space
     characters.
   */
   
  if (bwb_chartype (C) & CHAR_IS_SPACE)
  {
    return TRUE;
  }
  return FALSE;
}
#endif

#if FALSE /* keep this ... */
extern int
bwb_isupper (int C)
{
  /*
     4.3.1.10 The isupper function

     Synopsis

     #include <ctype.h>
     int isupper(int c);

     Description

     The isupper function tests for any upper-case letter or any of an
     implementation-defined set of characters for which none of iscntrl ,
     isdigit , ispunct , or isspace is true.  In the C locale, isupper
     returns true only for the characters defined as upper-case letters (as
     defined in $2.2.1).
   */
   
  if (bwb_chartype (C) & CHAR_IS_UPPER)
  {
    return TRUE;
  }
  return FALSE;
}
#endif

extern int
bwb_isxdigit (int C)
{
  /*
     4.3.1.11 The isxdigit function

     Synopsis

     #include <ctype.h>
     int isxdigit(int c);

     Description

     The isxdigit function tests for any hexadecimal-digit character (as
     defined in $3.1.3.2).
   */
   
  if (bwb_chartype (C) & CHAR_IS_XDIGIT)
  {
    return TRUE;
  }
  return FALSE;
}

extern int
bwb_tolower (int C)
{
  /*
     4.3.2.1 The tolower function

     Synopsis

     #include <ctype.h>
     int tolower(int c);

     Description

     The tolower function converts an upper-case letter to the
     corresponding lower-case letter.

     Returns

     If the argument is an upper-case letter, the tolower function
     returns the corresponding lower-case letter if there is one; otherwise
     the argument is returned unchanged.  In the C locale, tolower maps
     only the characters for which isupper is true to the corresponding
     characters for which islower is true.
   */
   
  switch (C)
  {
  case 'A':
    return 'a';
  case 'B':
    return 'b';
  case 'C':
    return 'c';
  case 'D':
    return 'd';
  case 'E':
    return 'e';
  case 'F':
    return 'f';
  case 'G':
    return 'g';
  case 'H':
    return 'h';
  case 'I':
    return 'i';
  case 'J':
    return 'j';
  case 'K':
    return 'k';
  case 'L':
    return 'l';
  case 'M':
    return 'm';
  case 'N':
    return 'n';
  case 'O':
    return 'o';
  case 'P':
    return 'p';
  case 'Q':
    return 'q';
  case 'R':
    return 'r';
  case 'S':
    return 's';
  case 'T':
    return 't';
  case 'U':
    return 'u';
  case 'V':
    return 'v';
  case 'W':
    return 'w';
  case 'X':
    return 'x';
  case 'Y':
    return 'y';
  case 'Z':
    return 'z';
  }
  return C;
}

extern int
bwb_toupper (int C)
{
  /*
     4.3.2.2 The toupper function

     Synopsis

     #include <ctype.h>
     int toupper(int c);

     Description

     The toupper function converts a lower-case letter to the corresponding upper-case letter.  

     Returns

     If the argument is a lower-case letter, the toupper function
     returns the corresponding upper-case letter if there is one; otherwise
     the argument is returned unchanged.  In the C locale, toupper maps
     only the characters for which islower is true to the corresponding
     characters for which isupper is true.
   */
   
  switch (C)
  {
  case 'a':
    return 'A';
  case 'b':
    return 'B';
  case 'c':
    return 'C';
  case 'd':
    return 'D';
  case 'e':
    return 'E';
  case 'f':
    return 'F';
  case 'g':
    return 'G';
  case 'h':
    return 'H';
  case 'i':
    return 'I';
  case 'j':
    return 'J';
  case 'k':
    return 'K';
  case 'l':
    return 'L';
  case 'm':
    return 'M';
  case 'n':
    return 'N';
  case 'o':
    return 'O';
  case 'p':
    return 'P';
  case 'q':
    return 'Q';
  case 'r':
    return 'R';
  case 's':
    return 'S';
  case 't':
    return 'T';
  case 'u':
    return 'U';
  case 'v':
    return 'V';
  case 'w':
    return 'W';
  case 'x':
    return 'X';
  case 'y':
    return 'Y';
  case 'z':
    return 'Z';
  }
  return C;
}


extern void *
bwb_memcpy (void *s1, const void *s2, size_t n)
{
  /*
     4.11.2.1 The memcpy function

     Synopsis

     #include <string.h>
     void *memcpy(void *s1, const void *s2, size_t n);

     Description

     The memcpy function copies n characters from the object pointed to
     by s2 into the object pointed to by s1 .  If copying takes place
     between objects that overlap, the behavior is undefined.

     Returns

     The memcpy function returns the value of s1 .  
   */
   

  if (n > 0)
  {
    char *Target;
    char *Source;
    int p;
    assert (s1 != NULL);
    assert (s2 != NULL);


    Target = (char *) s1;
    Source = (char *) s2;
    p = 0;
    while (p < n)
    {
      Target[p] = Source[p];
      p++;
    }
  }
  return s1;
}

#if FALSE /* keep this ... */
extern void *
bwb_memmove (void *s1, const void *s2, size_t n)
{
  /*
     4.11.2.2 The memmove function

     Synopsis

     #include <string.h>
     void *memmove(void *s1, const void *s2, size_t n);

     Description

     The memmove function copies n characters from the object pointed to
     by s2 into the object pointed to by s1 .  Copying takes place as if
     the n characters from the object pointed to by s2 are first copied
     into a temporary array of n characters that does not overlap the
     objects pointed to by s1 and s2 , and then the n characters from the
     temporary array are copied into the object pointed to by s1 .

     Returns

     The memmove function returns the value of s1 .  
   */
   

  if (n > 0)
  {
    char *Target;
    char *Source;
    char *Temp;
    assert (s1 != NULL);
    assert (s2 != NULL);

    Target = (char *) s1;
    Source = (char *) s2;
    Temp = (char *) malloc (n);
    if (Temp != NULL)
    {
      int p;

      p = 0;
      while (p < n)
      {
        Temp[p] = Source[p];
        p++;
      }
      p = 0;
      while (p < n)
      {
        Target[p] = Temp[p];
        p++;
      }
      free (Temp);
      Temp = NULL;
    }
  }
  return s1;
}
#endif

extern char *
bwb_strcpy (char *s1, const char *s2)
{
  /*
     4.11.2.3 The strcpy function

     Synopsis

     #include <string.h>
     char *strcpy(char *s1, const char *s2);

     Description

     The strcpy function copies the string pointed to by s2 (including
     the terminating null character) into the array pointed to by s1 .  If
     copying takes place between objects that overlap, the behavior is
     undefined.

     Returns

     The strcpy function returns the value of s1 .  
   */
  char C;
  int p;
   
  assert (s1 != NULL);
  assert (s2 != NULL);

  p = 0;
  do
  {
    C = s2[p];
    s1[p] = C;
    p++;
  }
  while (C);
  return s1;
}


extern char *
bwb_strncpy (char *s1, const char *s2, size_t n)
{
  /*
     4.11.2.4 The strncpy function

     Synopsis

     #include <string.h>
     char *strncpy(char *s1, const char *s2, size_t n);

     Description

     The strncpy function copies not more than n characters (characters
     that follow a null character are not copied) from the array pointed to
     by s2 to the array pointed to by s1 ./120/ If copying takes place
     between objects that overlap, the behavior is undefined.

     If the array pointed to by s2 is a string that is shorter than n
     characters, null characters are appended to the copy in the array
     pointed to by s1 , until n characters in all have been written.

     Returns

     The strncpy function returns the value of s1 .  
   */
   

  if (n > 0)
  {
    char C;
    int p;
    assert (s1 != NULL);
    assert (s2 != NULL);

    p = 0;
    do
    {
      C = s2[p];
      s1[p] = C;
      p++;
    }
    while (C != NulChar && p < n);
    while (p < n)
    {
      s1[p] = NulChar;
      p++;
    }
  }
  return s1;
}


extern char *
bwb_strcat (char *s1, const char *s2)
{
  /*
     4.11.3.1 The strcat function

     Synopsis

     #include <string.h>
     char *strcat(char *s1, const char *s2);

     Description

     The strcat function appends a copy of the string pointed to by s2
     (including the terminating null character) to the end of the string
     pointed to by s1 .  The initial character of s2 overwrites the null
     character at the end of s1 .  If copying takes place between objects
     that overlap, the behavior is undefined.

     Returns

     The strcat function returns the value of s1 .  
   */
  char *Temp;
   
  assert (s1 != NULL);
  assert (s2 != NULL);

  Temp = bwb_strchr (s1, NulChar);
  bwb_strcpy (Temp, s2);
  return s1;
}

#if FALSE /* keep this ... */
extern char *
bwb_strncat (char *s1, const char *s2, size_t n)
{
  /*
     4.11.3.2 The strncat function

     Synopsis

     #include <string.h>
     char *strncat(char *s1, const char *s2, size_t n);

     Description

     The strncat function appends not more than n characters (a null
     character and characters that follow it are not appended) from the
     array pointed to by s2 to the end of the string pointed to by s1 .
     The initial character of s2 overwrites the null character at the end
     of s1 .  A terminating null character is always appended to the
     result./121/ If copying takes place between objects that overlap, the
     behavior is undefined.

     Returns

     The strncat function returns the value of s1 .  
   */
  char *Temp;
   
  assert (s1 != NULL);
  assert (s2 != NULL);

  Temp = bwb_strchr (s1, NulChar);
  bwb_strncpy (Temp, s2, n);
  return s1;
}
#endif

extern int
bwb_memcmp (const void *s1, const void *s2, size_t n)
{
  /*
     4.11.4.1 The memcmp function

     Synopsis

     #include <string.h>
     int memcmp(const void *s1, const void *s2, size_t n);

     Description

     The memcmp function compares the first n characters of the object
     pointed to by s1 to the first n characters of the object pointed to by
     s2 ./122/

     Returns

     The memcmp function returns an integer greater than, equal to, or
     less than zero, according as the object pointed to by s1 is greater
     than, equal to, or less than the object pointed to by s2 .
   */
   

  if (n > 0)
  {
    int p;
    char *L;
    char *R;
    assert (s1 != NULL);
    assert (s2 != NULL);

    p = 0;
    L = (char *) s1;
    R = (char *) s2;
    while (p < n)
    {
      if (L[p] > R[p])
      {
        return 1;
      }
      if (L[p] < R[p])
      {
        return -1;
      }
      /* L[ p ] == R[ p ] */
      p++;
    }
  }
  return 0;
}


extern int
bwb_strcmp (const char *s1, const char *s2)
{
  /*
     4.11.4.2 The strcmp function

     Synopsis

     #include <string.h>
     int strcmp(const char *s1, const char *s2);

     Description

     The strcmp function compares the string pointed to by s1 to the
     string pointed to by s2 .

     Returns

     The strcmp function returns an integer greater than, equal to, or
     less than zero, according as the string pointed to by s1 is greater
     than, equal to, or less than the string pointed to by s2 .
   */
  char C;
  int p;
   
  assert (s1 != NULL);
  assert (s2 != NULL);

  p = 0;
  do
  {
    if (s1[p] > s2[p])
    {
      return 1;
    }
    if (s1[p] < s2[p])
    {
      return -1;
    }
    /* s1[ p ] == s2[ p ] */
    C = s1[p];
    p++;
  }
  while (C);
  return 0;
}

#if FALSE /* keep this ... */
extern int
bwb_strncmp (const char *s1, const char *s2, size_t n)
{
  /*
     4.11.4.4 The strncmp function

     Synopsis

     #include <string.h>
     int strncmp(const char *s1, const char *s2, size_t n);

     Description

     The strncmp function compares not more than n characters
     (characters that follow a null character are not compared) from the
     array pointed to by s1 to the array pointed to by s2 .

     Returns

     The strncmp function returns an integer greater than, equal to, or
     less than zero, according as the possibly null-terminated array
     pointed to by s1 is greater than, equal to, or less than the possibly
     null-terminated array pointed to by s2 .
   */
   

  if (n > 0)
  {
    char C;
    int p;
    assert (s1 != NULL);
    assert (s2 != NULL);

    p = 0;
    do
    {
      if (s1[p] > s2[p])
      {
        return 1;
      }
      if (s1[p] < s2[p])
      {
        return -1;
      }
      /* s1[ p ] == s2[ p ] */
      C = s1[p];
      p++;
    }
    while (C != NulChar && p < n);
  }
  return 0;
}
#endif

#if FALSE /* keep this ... */
extern void *
bwb_memchr (const void *s, int c, size_t n)
{
  /*
     4.11.5.1 The memchr function

     Synopsis

     #include <string.h>
     void *memchr(const void *s, int c, size_t n);

     Description

     The memchr function locates the first occurrence of c (converted to
     an unsigned char ) in the initial n characters (each interpreted as
     unsigned char ) of the object pointed to by s .

     Returns

     The memchr function returns a pointer to the located character, or
     a null pointer if the character does not occur in the object.
   */
   

  if (n > 0)
  {
    int p;
    unsigned char *Check;
    unsigned char Find;
    assert (s != NULL);

    p = 0;
    Check = (unsigned char *) s;
    Find = (unsigned char) c;
    do
    {
      if (Check[p] == Find)
      {
        return (void *) &(Check[p]);
      }
      p++;
    }
    while (p < n);
  }
  return NULL;
}
#endif

extern char *
bwb_strchr (const char *s, int c)
{
  /*
     4.11.5.2 The strchr function

     Synopsis

     #include <string.h>
     char *strchr(const char *s, int c);

     Description

     The strchr function locates the first occurrence of c (converted to
     a char ) in the string pointed to by s .  The terminating null
     character is considered to be part of the string.

     Returns

     The strchr function returns a pointer to the located character, or
     a null pointer if the character does not occur in the string.
   */
  int p;
  char Find;
  char C;
   
  assert (s != NULL);

  p = 0;
  Find = (char) c;
  do
  {
    C = s[p];
    if (C == Find)
    {
      return (char *) &(s[p]);
    }
    p++;
  }
  while (C);
  return NULL;
}


extern char *
bwb_strrchr (const char *s, int c)
{
  /*
     4.11.5.5 The strrchr function

     Synopsis

     #include <string.h>
     char *strrchr(const char *s, int c);

     Description

     The strrchr function locates the last occurrence of c (converted to
     a char ) in the string pointed to by s .  The terminating null
     character is considered to be part of the string.

     Returns

     The strrchr function returns a pointer to the character, or a null
     pointer if c does not occur in the string.
   */
  int p;
  char Find;
  char *Found;
  char C;
   
  assert (s != NULL);

  p = 0;
  Find = (char) c;
  Found = NULL;
  do
  {
    C = s[p];
    if (C == Find)
    {
      Found = (char *) &(s[p]);
    }
    p++;
  }
  while (C);
  return Found;
}


extern void *
bwb_memset (void *s, int c, size_t n)
{
  /*
     4.11.6.1 The memset function

     Synopsis

     #include <string.h>
     void *memset(void *s, int c, size_t n);

     Description

     The memset function copies the value of c (converted to an unsigned
     char ) into each of the first n characters of the object pointed to by
     s .

     Returns

     The memset function returns the value of s .  
   */
   

  if (n > 0)
  {
    int p;
    unsigned char *Target;
    unsigned char Value;
    assert (s != NULL);

    p = 0;
    Target = (unsigned char *) s;
    Value = (unsigned char) c;
    do
    {
      Target[p] = Value;
      p++;
    }
    while (p < n);
  }
  return s;
}

extern size_t
bwb_strlen (const char *s)
{
  /*
     4.11.6.3 The strlen function

     Synopsis

     #include <string.h>
     size_t strlen(const char *s);

     Description

     The strlen function computes the length of the string pointed to by s .  

     Returns

     The strlen function returns the number of characters that precede
     the terminating null character.
   */
  size_t p;
   
  assert (s != NULL);

  p = 0;
  while (s[p])
  {
    p++;
  }
  return p;
}

extern char *
bwb_strdup (char *s)
{
  size_t n;
  char *r;
  assert (s != NULL);

  /* r = NULL; */
  n = bwb_strlen (s);
  r = calloc (n + 1 /* NulChar */ , sizeof (char));
  if (r != NULL)
  {
    bwb_strcpy (r, s);
  }
  return r;
}

extern char *
bwb_strdup2 (char *s, char *t)
{
  size_t n;
  char *r;
  assert (s != NULL);
  assert (t != NULL);

  /* r = NULL; */
  n = bwb_strlen (s) + bwb_strlen (t);
  r = calloc (n + 1 /* NulChar */ , sizeof (char));
  if (r != NULL)
  {
    bwb_strcpy (r, s);
    bwb_strcat (r, t);
  }
  return r;
}

#if HAVE_UNIX_GCC

/* these are intrinsic C functions in my environment using -ansi */

#else /*  ! HAVE_UNIX_GCC */

extern unsigned int
sleep (unsigned int X)
{
  /* do nothing */
  return X;
}

#endif /* ! HAVE_UNIX_GCC */


extern double
bwb_rint (double x)
{
  /* BASIC dialects have different rounding rules */
  double Result;
   

  if (x < 0)
  {
    return -bwb_rint (-x);
  }
  /* x >= 0 */
  switch (My->OptionRoundType)
  {
  case C_OPTION_ROUND_BANK:
    /* normal financial rounding */
    Result = floor (x + 0.5);
    if (x - floor (x) == 0.5)
    {
      /* midway */
      double Half;
      Half = Result / 2.0;
      if (Half != floor (Half))
      {
        /* odd -> even */
        Result--;
      }
    }
    break;
  case C_OPTION_ROUND_MATH:
    /* normal mathematical rounding */
    Result = floor (x + 0.5);
    break;
  case C_OPTION_ROUND_TRUNCATE:
    /* simple truncation */
    Result = floor (x);
    break;
  }
  return Result;
}

extern int
bwb_stricmp (const char *s1, const char *s2)
{
  const unsigned char *p1;
  const unsigned char *p2;
   
  assert (s1 != NULL);
  assert (s2 != NULL);

  p1 = (const unsigned char *) s1;
  p2 = (const unsigned char *) s2;
  while (*p1)
  {
    char c1;
    char c2;
    c1 = bwb_toupper (*p1);
    c2 = bwb_toupper (*p2);
    if (c1 < c2)
    {
      return -1;
    }
    if (c1 > c2)
    {
      return 1;
    }
    p1++;
    p2++;
  }
  if (*p2 == NulChar)
  {
    return 0;
  }
  return -1;
}

extern int
bwb_strnicmp (const char *s1, const char *s2, size_t n)
{
  const unsigned char *p1;
  const unsigned char *p2;
  size_t x = 0;
   
  assert (s1 != NULL);
  assert (s2 != NULL);

  p1 = (const unsigned char *) s1;
  p2 = (const unsigned char *) s2;
  while (x < n)
  {
    char c1;
    char c2;
    c1 = bwb_toupper (p1[x]);
    c2 = bwb_toupper (p2[x]);
    if (c1 < c2)
    {
      return -1;
    }
    if (c1 > c2)
    {
      return 1;
    }
    if (c1 == NulChar)
    {
      return 0;
    }
    x++;
  }
  return 0;
}


/* EOF */
