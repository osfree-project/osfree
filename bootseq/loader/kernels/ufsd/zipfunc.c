/*  Zipfile specific filesystem access functions
 *
 */

#include <shared.h>

#include "fsd.h"

int kprintf(const char *format, ...);

extern int filemax;
extern int filepos;
extern int fileaddr;

/* multiboot structure pointer */
extern struct multiboot_info *m;

#pragma aux m            "*"
#pragma aux filemax      "*"
#pragma aux filepos      "*"
#pragma aux ufs_open     "*"
#pragma aux ufs_read     "*"
#pragma aux ufs_seek     "*"
#pragma aux ufs_close    "*"
#pragma aux ufs_term     "*"

int
ufs_open (char *filename)
{
  return 0;
}

int
ufs_read (char *buf, int len)
{
  return 0;
}

int
ufs_seek (int offset)
{
  int i;
  kprintf("**** ufs_seek(\"%ld\")\n", offset);

  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;
  //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x3fa + 0x1386 + i)));
  //kprintf("\n");

  return offset;
}

void
ufs_close (void)
{
  int i;
  kprintf("**** ufs_close()\n");

  for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x3fa + 0x1426 + i)));
  kprintf("\n");
}

void
ufs_term (void)
{
  int i;
  kprintf("**** ufs_close()\n");

  for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x3fa + 0x1426 + i)));
  kprintf("\n");
}
