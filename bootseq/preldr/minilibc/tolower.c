/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

int
grub_tolower (int c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));

  return c;
}
