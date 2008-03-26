/*
 *   mini-libc functions
 *
 */

#include <shared.h>

#include "fsys.h"

int
substring (const char *s1, const char *s2)
{
  while (*s1 == *s2)
    {
      /* The strings match exactly. */
      if (! *(s1++))
        return 0;
      s2 ++;
    }

  /* S1 is a substring of S2. */
  if (*s1 == 0)
    return -1;

  /* S1 isn't a substring. */
  return 1;

}

char *
grub_strstr (const char *s1, const char *s2)
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
grub_strlen (const char *str)
{
  int len = 0;

  while (*str++)
    len++;

  return len;
}

int grub_index(char c, char *s)
{
  int  i  = 0;
  char *p = s;

  while (*p != '\0' && *p++ != c) i++;
  if (!*p) return 0;

  return ++i;
}

int
grub_strcmp (const char *s1, const char *s2)
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


char *
grub_strcpy (char *dest, const char *src)
{
  grub_memmove (dest, src, grub_strlen (src) + 1);
  return dest;
}

char *
grub_strncpy (char *dest, const char *src, int n)
{
  int i;

  for (i = 0; i < n; i++) {
    dest[i] = src[i];
    if (!src[i]) break;
  }

  return dest;
}


int
grub_toupper (int c)
{
  if (c >= 'a' && c <= 'z')
    return (c + ('A' - 'a'));

  return c;
}


int
grub_tolower (int c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));

  return c;
}


int
grub_aton(char *h)
{
  char *s = h;
  int  i = 0;
  int  j, k, l;
  char c;
  int  base;

  if (s[0] == '0' && s[1] == 'x') {
    base = 16;
    s += 2; // Delete "0x"
  } else {
    base = 10;
  }

  l = grub_strlen(s) - 1;

  while (*s) {
    c = grub_tolower(*s);

    if ('a' <= c && c <= 'f') {
      if (base == 16) {
        c = c - 'a' + 10;
      } else {
        return 0;
      }
    } else if ('0' <= c && c <= '9') {
      c -= '0';
    } else {
      return 0;
    }

    for (j = 0, k = c; j < l; j++)
      k *= base;

    i += k;
    s++;
    l--;
  }

  return i;
}

//#if defined(FSYS_ISO9660)
int
grub_memcmp (const char *s1, const char *s2, int n)
{
  while (n)
    {
      if (*s1 < *s2)
        return -1;
      else if (*s1 > *s2)
        return 1;
      s1++;
      s2++;
      n--;
    }

  return 0;
}
//#endif /* ! FSYS_ISO9660 */


long
grub_memcheck (unsigned long addr, long len)
{
    // Physical address:
    //if ( (addr < RAW_ADDR (0x1000))
    //if ( (addr < RAW_ADDR (0x600)) )
    //    || ((addr <  RAW_ADDR (0x100000)) && (RAW_ADDR(mem_lower * 1024) < (addr + len)))
    //    || ((addr >= RAW_ADDR (0x100000)) && (RAW_ADDR(mem_upper * 1024) < ((addr - 0x100000) + len))) )
    //{
    //    errnum = ERR_WONT_FIT;
    //    //printk("freeldr_memcheck: ERR_WONT_FIT");
    //    //printk("freeldr_memcheck(): addr = 0x%08lx, len = %u", addr, len);
    //    return 0;
    //}

    return 1;
}


void *
grub_memmove (void *_to, const void *_from, int _len)
{

    char *from = (char *)_from;
    char *to   = _to;

    if (grub_memcheck ((unsigned long)(_to), _len))
    {
        if ( from == to )
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
    }

    return( to );
}


void *
grub_memset (void *start, int c, int len)
{
  char *p = start;

  if (grub_memcheck ((int) start, len))
    {
      while (len -- > 0)
        *p ++ = c;
    }
#ifndef STAGE1_5
  u_parm(PARM_ERRNUM, ACT_GET, (unsigned long *)&errnum);
#endif
  return errnum ? NULL : start;
}


int
grub_isspace (int c)
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

char *
convert_to_ascii (char *buf, int c,...)
{
  unsigned long num = *((&c) + 1), mult = 10;
  char *ptr = buf;

#ifndef STAGE1_5
  if (c == 'x' || c == 'X')
    mult = 16;

  if ((num & 0x80000000uL) && c == 'd')
    {
      num = (~num) + 1;
      *(ptr++) = '-';
      buf++;
    }
#endif

  do
    {
      int dig = num % mult;
      *(ptr++) = ((dig > 9) ? dig + 'a' - 10 : '0' + dig);
    }
  while (num /= mult);

  /* reorder to correct direction!! */
  {
    char *ptr1 = ptr - 1;
    char *ptr2 = buf;
    while (ptr1 > ptr2)
      {
        int tmp = *ptr1;
        *ptr1 = *ptr2;
        *ptr2 = tmp;
        ptr1--;
        ptr2++;
      }
  }

  return ptr;
}

#ifndef STAGE1_5

void
grub_putstr (const char *str)
{
  while (*str)
    grub_putchar (*str++);
}

void
grub_printf (const char *format,...)
{
  int *dataptr = (int *) &format;
  char c, str[16];

  dataptr++;

  while ((c = *(format++)) != 0)
    {
      if (c != '%')
        grub_putchar (c);
      else
        switch (c = *(format++))
          {
#ifndef STAGE1_5
          case 'd':
          case 'x':
          case 'X':
#endif
          case 'u':
            *convert_to_ascii (str, c, *((unsigned long *) dataptr++)) = 0;
            grub_putstr (str);
            break;

#ifndef STAGE1_5
          case 'c':
            grub_putchar ((*(dataptr++)) & 0xff);
            break;

          case 's':
            grub_putstr ((char *) *(dataptr++));
            break;
#endif
          }
    }
}


int
grub_sprintf (char *buffer, const char *format, ...)
{
  /* XXX hohmuth
     ugly hack -- should unify with printf() */
  int *dataptr = (int *) &format;
  char c, *ptr, str[16];
  char *bp = buffer;

  dataptr++;

  while ((c = *format++) != 0)
    {
      if (c != '%')
        *bp++ = c; /* putchar(c); */
      else
        switch (c = *(format++))
          {
          case 'd': case 'u': case 'x':
            *convert_to_ascii (str, c, *((unsigned long *) dataptr++)) = 0;

            ptr = str;

            while (*ptr)
              *bp++ = *(ptr++); /* putchar(*(ptr++)); */
            break;

          case 'c': *bp++ = (*(dataptr++))&0xff;
            /* putchar((*(dataptr++))&0xff); */
            break;

          case 's':
            ptr = (char *) (*(dataptr++));

            while ((c = *ptr++) != 0)
              *bp++ = c; /* putchar(c); */
            break;
          }
    }

  *bp = 0;
  return bp - buffer;
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
          errnum = ERR_NUMBER_OVERFLOW;
#ifndef STAGE1_5
          u_parm(PARM_ERRNUM, ACT_SET, (unsigned long *)&errnum);
#endif
          return 0;
        }
      myint = (myint * mult) + digit;
      ptr++;
    }

  if (!found)
    {
      errnum = ERR_NUMBER_PARSING;
#ifndef STAGE1_5
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned long *)&errnum);
#endif
      return 0;
    }

  *str_ptr = ptr;
  *myint_ptr = myint;

  return 1;
}

/* Display an ASCII character.  */
void
grub_putchar (int c)
{
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  u_msg(buf);
}

#endif
