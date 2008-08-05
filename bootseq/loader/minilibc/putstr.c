/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

void
grub_putstr (const char *str)
{
  while (*str)
    grub_putchar (*str++);
}
