/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

char *
grub_strncpy (char *dest, const char *src, int n)
{
  int i;

  for (i = 0; i < n; i++) {
    dest[i] = src[i];
    if (!src[i]) break;
  }
  dest[n] = '\0';

  return dest;
}
