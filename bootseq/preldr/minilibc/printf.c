/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

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
