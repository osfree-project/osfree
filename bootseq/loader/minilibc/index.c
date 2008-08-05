/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

int grub_index(char c, char *s)
{
  int  i  = 0;
  char *p = s;

  for (;*p != '\0' && *p != c; p++) i++;
  if (!*p) return 0;

  return i + 1;
}
