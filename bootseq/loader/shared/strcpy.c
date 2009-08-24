/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

char *
grub_strcpy (char *dest, const char *src)
{
  grub_memmove (dest, src, grub_strlen (src) + 1);
  return dest;
}
