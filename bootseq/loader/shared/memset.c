/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

void *
grub_memset (void *start, int c, int len)
{
  char *p = start;

  //if (grub_memcheck ((int) start, len))
    {
      while (len -- > 0)
        *p ++ = c;
    }
#ifndef STAGE1_5
  //u_parm(PARM_ERRNUM, ACT_GET, (unsigned int *)&errnum);
#endif
  //return errnum ? NULL : start;
  return start;
}
