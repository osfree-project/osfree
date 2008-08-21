/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

int
grub_strlen (const char *str)
{
   int len = 0;

  while (*str++)
    len++;

  return len;
}
