/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

char *
grub_strcat (char *dest, const char *src1, const char *src2)
{
  int l;

  l = grub_strlen(src1);
  grub_strcpy(dest, src1);
  grub_strcpy(dest + l, src2);

  return dest;
}
