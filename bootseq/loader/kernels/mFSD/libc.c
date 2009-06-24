/*  memory/strings libc manipulation functions
 *  for far pointers
 *
 */

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
