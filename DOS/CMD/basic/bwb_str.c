/***************************************************************
  
   bwb_str.c       String-Management Routines
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



/***************************************************************

BASIC allows embedded NUL (0) characters.  C str*() does not. 

ALL the StringType code should use mem*() and ->length.
ALL the StringType code should prevent string overflow.

***************************************************************/




#include "bwbasic.h"


static int CharListToSet (char *pattern, int start, int stop);
static int IndexOf (char *buffer, char find, int start);


/***************************************************************
  
        FUNCTION:       str_btob()
  
        DESCRIPTION:    This C function assigns a bwBASIC string
         structure to another bwBASIC string
         structure.
  
***************************************************************/

int
str_btob (StringType * d, StringType * s)
{
   
  assert (d != NULL);
  assert (s != NULL);
  /*  assert( s->length >= 0 ); */
  assert( My != NULL );

  if (s->length > MAXLEN)
  {
    WARN_STRING_TOO_LONG;        /* str_btob */
    s->length = MAXLEN;
  }

  /* get memory for new buffer */
  if (d->sbuffer != NULL)
  {
    /* free the destination's buffer */
    if (d->sbuffer == My->MaxLenBuffer)
    {
      /*
      ** this occurs when setting the return value of a multi-line string user function
      **
      ** FUNCTION INKEY$
      **   DIM A$
      **   LINE INPUT A$
      **   LET INKEY$ = LEFT$( A$, 1 )
      ** END FUNCTION
      **
      */
    }
    else if (d->sbuffer == My->ConsoleOutput)
    {
      /* this should never occur, but let's make sure we don't crash if it does */
      WARN_INTERNAL_ERROR;
      /* continue processing */
    }
    else if (d->sbuffer == My->ConsoleInput)
    {
      /* this should never occur, but let's make sure we don't crash if it does */
      WARN_INTERNAL_ERROR;
      /* continue processing */
    }
    else
    {
      free (d->sbuffer);
      d->sbuffer = NULL;
    }
    d->sbuffer = NULL;
    d->length = 0;
  }
  if (d->sbuffer == NULL)
  {
    /* allocate the destination's buffer */
    d->length = 0;
    if ((d->sbuffer =
         (char *) calloc (s->length + 1 /* NulChar */ ,
                          sizeof (char))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return FALSE;
    }
  }
  /* write the b string to the b string */
  assert( d->sbuffer != NULL );
  if( s->length > 0 )
  { 
    assert( s->sbuffer != NULL );
    bwb_memcpy (d->sbuffer, s->sbuffer, s->length);
  }
  d->length = s->length;
  d->sbuffer[d->length] = NulChar;
  return TRUE;
}


/***************************************************************
  
        FUNCTION:       str_cmp()
  
   DESCRIPTION:    This C function performs the equivalent
         of the C strcmp() function, using BASIC
         strings.
  
***************************************************************/

int
str_cmp (StringType * a, StringType * b)
{
   
  assert (a != NULL);
  assert (b != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  if (a->length > MAXLEN)
  {
    WARN_STRING_TOO_LONG;        /* str_cmp */
    a->length = MAXLEN;
  }
  if (b->length > MAXLEN)
  {
    WARN_STRING_TOO_LONG;        /* str_cmp */
    b->length = MAXLEN;
  }
  if (a->sbuffer == NULL)
  {
    if (b->sbuffer == NULL)
    {
      return 0;
    }
    if (b->length == 0)
    {
      return 0;
    }
    return 1;
  }
  assert( a->sbuffer != NULL );
  a->sbuffer[a->length] = NulChar;

  if (b->sbuffer == NULL)
  {
    if (a->sbuffer == NULL)
    {
      return 0;
    }
    if (a->length == 0)
    {
      return 0;
    }
    return -1;
  }
  assert( b->sbuffer != NULL );
  b->sbuffer[b->length] = NulChar;

  if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
  {
    /* case insensitive */
    return bwb_stricmp (a->sbuffer, b->sbuffer);        /* NOTE: embedded NUL characters terminate comparison */
  }
  else
  {
    /* case sensitive */
    return bwb_strcmp (a->sbuffer, b->sbuffer);        /* NOTE: embedded NUL characters terminate comparison */
  }
}

/***************************************************************

                     MATCH

***************************************************************/

int
str_match (char *A, int A_Length, char *B, int B_Length, int I_Start)
{
  /*
     SYNTAX: j% = MATCH( a$, b$, i% )

     MATCH returns the position of the first occurrence of a$ in b$ 
     starting with the character position given by the third parameter. 
     A zero is returned if no MATCH is found. 

     The following pattern-matching features are available: 
     # matches any digit (0-9). 
     ! matches any upper-or lower-case letter. 
     ? matches any character. 
     \ serves as an escape character indicating the following character does not have special meaning. 

     For example, a ? signifies any character is a MATCH unless preceded by a \. 
     a$ and b$ must be strings. 
     If either of these arguments are numeric, an error occurs. 
     If i% is real, it is converted to an integer. 
     If i% is a string, an error occurs. 
     If i% is negative or zero, a run-time error occurs. 
     When i% is greater than the length of b$, zero is returned. 
     If b$ is a null string, a 0 is returned. 
     If b$ is not null, but a$ is null, a 1 is returned.    

     Examples: 
     MATCH( "is", "Now is the", 1) returns 5
     MATCH( "##", "October 8, 1876", 1) returns 12
     MATCH( "a?", "character", 4 ) returns 5
     MATCH( "\#", "123#45", 1) returns 4
     MATCH( "ABCD", "ABC", 1 ) returns 0
     MATCH( "\#1\\\?", "1#1\?2#", 1 ) returns 2
   */


  int a;                        /* current position in A$ */
  int b;                        /* current position in B$ */
   
  assert (A != NULL);
  assert (B != NULL);


  if (I_Start <= 0)
  {
    return 0;
  }
  if (I_Start > B_Length)
  {
    return 0;
  }
  if (B_Length <= 0)
  {
    return 0;
  }
  if (A_Length <= 0)
  {
    return 1;
  }
  I_Start--;                        /* BASIC to C */
  for (b = I_Start; b < B_Length; b++)
  {
    int n;                        /* number of characters in A$ matched with B$ */

    n = 0;
    for (a = 0; a < A_Length; a++)
    {
      int bn;
      bn = b + n;
      if (A[a] == '#' && bwb_isdigit (B[bn]))
      {
        n++;
      }
      else if (A[a] == '!' && bwb_isalpha (B[bn]))
      {
        n++;
      }
      else if (A[a] == '?')
      {
        n++;
      }
      else if (a < (A_Length - 1) && A[a] == '\\' && A[a + 1] == B[bn])
      {
        n++;
        a++;
      }
      else if (A[a] == B[bn])
      {
        n++;
      }
      else
      {
        break;
      }
    }
    if (a == A_Length)
    {
      b++;                        /* C to BASIC */
      return b;
    }
  }
  return 0;
}



/***************************************************************
  
        FUNCTION:       str_like()
  
   DESCRIPTION:    This C function performs the equivalent
         of the BASIC LIKE operator, using BASIC
         strings.
  
***************************************************************/

/* 
inspired by http://www.blackbeltcoder.com/Articles/net/implementing-vbs-like-operator-in-c 
*/

/*
KNOWN ISSUES:
To match the character '[', use "[[]".
To match the character '?', use "[?]".
To match the character '*', use "[*]".
Does not match "" with "[]" or "[!]".
*/

#define CHAR_SET '*'
#define CHAR_CLR ' '

static char charList[256];

static int
IndexOf (char *buffer, char find, int start)
{
  int buffer_count;
  int buffer_length;
   
  assert (buffer != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  buffer_length = bwb_strlen (buffer);

  if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
  {
    find = bwb_toupper (find);
  }

  for (buffer_count = start; buffer_count < buffer_length; buffer_count++)
  {
    char theChar;

    theChar = buffer[buffer_count];

    if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
    {
      theChar = bwb_toupper (theChar);
    }


    if (theChar == find)
    {
      /* FOUND */
      return buffer_count;
    }

  }
  /* NOT FOUND */
  return -1;
}


static int
CharListToSet (char *pattern, int start, int stop)
{
  /*
     Converts a string of characters to a HashSet of characters. If the string
     contains character ranges, such as A-Z, all characters in the range are
     also added to the returned set of characters.
   */
  int pattern_Count;
   
  assert (pattern != NULL);

  bwb_memset (charList, CHAR_CLR, 256);

  if (start > stop)
  {
    /* ERROR */
    return FALSE;
  }
  /* Leading '-' */
  while (pattern[start] == '-')
  {
    /* Match character '-' */

    charList[0x00FF & pattern[start]] = CHAR_SET;
    start++;
    if (start > stop)
    {
      /* DONE */
      return TRUE;
    }
  }
  /* Trailing '-' */
  while (pattern[stop] == '-')
  {
    /* Match character '-' */
    charList[0x00FF & pattern[stop]] = CHAR_SET;
    stop--;
    if (start > stop)
    {
      /* DONE */
      return TRUE;
    }
  }


  for (pattern_Count = start; pattern_Count <= stop; pattern_Count++)
  {
    if (pattern[pattern_Count] == '-')
    {
      /* Character range */

      char startChar;
      char endChar;
      char theChar;

      if (pattern_Count > start)
      {
        startChar = pattern[pattern_Count - 1];
        if (startChar == '-')
        {
          /* ERROR */
          return FALSE;
        }
        if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
        {
          startChar = bwb_toupper (startChar);
        }
      }
      else
      {
        /* ERROR */
        return FALSE;
      }
      if (pattern_Count < stop)
      {
        endChar = pattern[pattern_Count + 1];
        if (endChar == '-')
        {
          /* ERROR */
          return FALSE;
        }
        if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
        {
          endChar = bwb_toupper (endChar);
        }
        if (endChar < startChar)
        {
          /* ERROR */
          return FALSE;
        }
      }
      else
      {
        /* ERROR */
        return FALSE;
      }
      /* 
         Although the startChar has already been set,
         and the endChar will be set on the next loop,
         we go ahead and set them here too.
         Not the most efficient, but easy to understand,
         and we do not have to do anything special
         for edge cases such as [A-A] and [A-B].  
       */
      for (theChar = startChar; theChar <= endChar; theChar++)
      {
        charList[0x00FF & theChar] = CHAR_SET;
      }
    }
    else
    {
      /* Single character */
      char theChar;

      theChar = pattern[pattern_Count];
      if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
      {
        theChar = bwb_toupper (theChar);
      }
      charList[0x00FF & theChar] = CHAR_SET;
    }
  }

  return TRUE;
}



int
IsLike (char *buffer, int *buffer_count, int buffer_Length,
        char *pattern, int *pattern_count, int pattern_Length)
{

  /* Implement's VB's Like operator logic. */

  /* 
     if matched then
     buffer_count is updated
     pattern_count is updated
     returns TRUE
     else
     returns FALSE
     end if
   */

  int bc;
  int pc;
   
  assert (buffer != NULL);
  assert (buffer_count != NULL);
  assert (pattern != NULL);
  assert (pattern_count != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  bc = *buffer_count;
  pc = *pattern_count;


  /* Loop through pattern string */
  while (pc < pattern_Length)
  {

    /* Get next pattern character */
    if (pattern[pc] == '[')
    {
      /* Character list */
      /* [] and [!] are special */
      char IsExclude;

      IsExclude = CHAR_CLR;
      pc++;
      /* pc is first character after '[' */
      if (pattern[pc] == '!')
      {
        pc++;
        IsExclude = CHAR_SET;
      }
      /* pc is first character after '[' */
      if (pattern[pc] == ']')
      {
        /* [] and [!] are special */
        /* pc is first character after '[' and is a ']' */
        pc++;
        if (IsExclude == CHAR_CLR)
        {
          /* [] */
          /* matches "" */
        }
        else
        {
          /* [!] */
          /* same as '?' */
          if (bc >= buffer_Length)
          {
            /* we have completed the buffer without completing the pattern */
            return FALSE;
          }
          bc++;
        }
      }
      else
      {
        /* Build character list */
        /* pc is first character after '[' and is not a ']' */
        int stop_count;

        stop_count = IndexOf (pattern, ']', pc);
        /* stop_count is the character ']' */
        if (stop_count < 0)
        {
          /* NOT FOUND */
          return FALSE;
        }
        /* pc is first character after '['  */
        /* stop_count is the character ']' */
        CharListToSet (pattern, pc, stop_count - 1);
        pc = stop_count + 1;
        /* pc is first character after ']' */
        if (bc >= buffer_Length)
        {
          /* we have completed the buffer without completing the pattern */
          return FALSE;
        }
        if (charList[0x00FF & buffer[bc]] == IsExclude)
        {
          /* not matched */
          return FALSE;
        }
        bc++;
      }
    }
    else if (pattern[pc] == '?' /* LIKE char */ )
    {
      /* Matches a single character */
      pc++;
      if (bc >= buffer_Length)
      {
        /* Check for end of string */
        /* we have completed the buffer without completing the pattern */
        return FALSE;
      }
      bc++;
    }
    else if (pattern[pc] == '#' /* LIKE digit */ )
    {
      /* Matches a single digit */
      pc++;
      if (bc >= buffer_Length)
      {
        /* Check for end of string */
        /* we have completed the buffer without completing the pattern */
        return FALSE;
      }
      if (bwb_isdigit (buffer[bc]))
      {
        bc++;
      }
      else
      {
        /* not matched */
        return FALSE;
      }
    }
    else if (pattern[pc] == '*' /* LIKE chars */ )
    {
      /* Zero or more characters */
      while (pattern[pc] == '*' /* LIKE chars */ )
      {
        pc++;
      }
      if (pc == pattern_Length)
      {
        /* Matches all remaining characters */
        bc = buffer_Length;
        pc = pattern_Length;
        break;
      }
      else
      {
        int p;
        int b;
        int IsMatched;

        p = pc;
        b = bc;
        IsMatched = FALSE;
        while (b <= buffer_Length && IsMatched == FALSE)
        {
          int last_b;

          last_b = b;
          IsMatched =
            IsLike (buffer, &b, buffer_Length, pattern, &p, pattern_Length);
          if (IsMatched == FALSE)
          {
            /* not matched, try again begining at next buffer position */
            p = pc;
            b = last_b + 1;
          }
        }
        if (IsMatched == FALSE)
        {
          /* not matched */
          return FALSE;
        }
        pc = p;
        bc = b;
      }
    }
    else
    {
      char pattChar;
      char buffChar;

      pattChar = pattern[pc];
      if (bc >= buffer_Length)
      {
        /* Check for end of string */
        /* we have completed the buffer without completing the pattern */
        return FALSE;
      }
      buffChar = buffer[bc];

      if (My->CurrentVersion->OptionFlags & OPTION_COMPARE_TEXT)
      {
        pattChar = bwb_toupper (pattChar);
        buffChar = bwb_toupper (buffChar);
      }

      if (pattChar == buffChar)
      {
        /* matched specified character */
        pc++;
        bc++;
      }
      else
      {
        /* not matched */
        return FALSE;
      }
    }
  }
  /* Return true if all characters matched */
  if (pc < pattern_Length)
  {
    /* not matched */
    return FALSE;
  }
  if (bc < buffer_Length)
  {
    /* not matched */
    return FALSE;
  }
  /* all characters matched */
  *buffer_count = bc;
  *pattern_count = pc;
  return TRUE;
}


/* EOF */
