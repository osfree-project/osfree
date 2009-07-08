/*  memory/strings libc manipulation functions
 *  for far pointers
 *
 */

#define MAXINT     0x7FFF

void far *
fmemset (void far *start, int c, int len)
{
  char far *p = start;

  while (len -- > 0)
    *p ++ = c;

  return start;
}

int
fstrlen (const char far *str)
{
  int len = 0;

  while (*str++)
    len++;

  return len;
}

void far *
fmemmove (void far *_to, const void far *_from, int _len)
{
  char far *from = (char far *)_from;
  char far *to   = _to;

  if (from == to)
  {
    return( to );
  }
  if ( from < to  &&  from + _len > to )
  {
    to += _len;
    from += _len;
    while( _len != 0 )
    {
      *--to = *--from;
      _len--;
    }
  }
  else
  {
    while( _len != 0 )
    {
      *to++ = *from++;
      _len--;
    }
  }

  return( to );
}

char far *
fstrcpy (char far *dest, const char far *src)
{
  fmemmove (dest, src, fstrlen (src) + 1);
  return dest;
}

int
toupper (int c)
{
  if (c >= 'a' && c <= 'z')
    return (c + ('A' - 'a'));

  return c;
}

int
tolower (int c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));

  return c;
}

int
isspace (int c)
{
  switch (c)
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      return 1;
    default:
      break;
    }

  return 0;
}

/*  Strip leading and trailing
 *  spaces
 */
char far *
strip(char far *s)
{
  char far *p = s;
  int  i;

  i = fstrlen(p) - 1;
  while (isspace(p[i])) p[i--] = '\0'; // strip trailing spaces
  while (isspace(*p)) p++;             // strip leading spaces

  return p;
}

char far *
fstrstr (const char far *s1, const char far *s2)
{
  while (*s1)
    {
      const char far *ptr, far *tmp;

      ptr = s1;
      tmp = s2;

      while (*tmp && *ptr == *tmp)
        ptr++, tmp++;

      if (tmp > s2 && ! *tmp)
        return (char far *) s1;

      s1++;
    }

  return 0;
}

int
safe_parse_maxint (char **str_ptr, int *myint_ptr)
{
  char *ptr = *str_ptr;
  int myint = 0;
  int mult = 10, found = 0;

  /*
   *  Is this a hex number?
   */
  if (*ptr == '0' && tolower (*(ptr + 1)) == 'x')
    {
      ptr += 2;
      mult = 16;
    }

  while (1)
    {
      /* A bit tricky. This below makes use of the equivalence:
         (A >= B && A <= C) <=> ((A - B) <= (C - B))
         when C > B and A is unsigned.  */
      unsigned int digit;

      digit = tolower (*ptr) - '0';
      if (digit > 9)
        {
          digit -= 'a' - '0';
          if (mult == 10 || digit > 5)
            break;
          digit += 10;
        }

      found = 1;
      if (myint > ((MAXINT - digit) / mult))
        {
          // = ERR_NUMBER_OVERFLOW;
#ifndef STAGE1_5
          //u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
#endif
          return 0;
        }
      myint = (myint * mult) + digit;
      ptr++;
    }

  if (!found)
    {
      //errnum = ERR_NUMBER_PARSING;
#ifndef STAGE1_5
      //u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
#endif
      return 0;
    }

  *str_ptr = ptr;
  *myint_ptr = myint;

  return 1;
}

char *
strstr (const char *s1, const char *s2)
{
  while (*s1)
    {
      const char *ptr, *tmp;

      ptr = s1;
      tmp = s2;

      while (*tmp && *ptr == *tmp)
        ptr++, tmp++;

      if (tmp > s2 && ! *tmp)
        return (char *) s1;

      s1++;
    }

  return 0;
}

int
strlen (const char *str)
{
   int len = 0;

  while (*str++)
    len++;

  return len;
}

void *
memset (void *start, int c, int len)
{
  char *p = start;

  while (len -- > 0)
    *p ++ = c;

  return start;
}

void *
memmove (void *_to, const void *_from, int _len)
{
  char *from = (char *)_from;
  char *to   = _to;

  if (from == to)
  {
    return( to );
  }
  if ( from < to  &&  from + _len > to )
  {
    to += _len;
    from += _len;
    while( _len != 0 )
    {
      *--to = *--from;
      _len--;
    }
  }
  else
  {
    while( _len != 0 )
    {
      *to++ = *from++;
      _len--;
    }
  }

  return( to );
}

char *
strcpy (char *dest, const char *src)
{
  memmove (dest, src, strlen (src) + 1);
  return dest;
}

int
strcmp (const char *s1, const char *s2)
{
  while (*s1 || *s2)
    {
      if (*s1 < *s2)
        return -1;
      else if (*s1 > *s2)
        return 1;
      s1 ++;
      s2 ++;
    }

  return 0;
}
