/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

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
