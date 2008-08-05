/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

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
