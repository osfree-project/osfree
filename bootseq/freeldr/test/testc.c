#include <string.h>
#include <i86.h>
#include <stdlib.h>
#include <malloc.h>

#include "vsprintf.h"

// Copy readbuf contents to physical address buf
unsigned short __cdecl loadhigh(unsigned long buf,
                                unsigned short chunk,
                                unsigned char far *readbuf);

unsigned char far *s = "qwertyuiop";
unsigned char far *buf1;
unsigned long buf;

#define PHYS_ADDR(x) (((((unsigned long)(x)) >> 16) << 4) + ((unsigned short)(x)))

int main(int argc, char **argv)
{
  int i;

  buf1 = _fmalloc(100);
  buf  = PHYS_ADDR(buf1);

  printk("buf1 = 0x%04X:0x%04X", FP_SEG(buf1), FP_OFF(buf1));
  printk("buf = 0x%08lX", buf);
  printk("s = 0x%04X:0x%04X", FP_SEG(s), FP_OFF(s));

  loadhigh(buf, _fstrlen(s), s);

  printk("s = <%s>", s);
  printk("buf1 = 0x%08lX, = <%s>", buf1, buf1);

 /*
  for (i = 0; i < 16; i++) printk("buf1[%u] = %c", i, buf1[i]);
  */

  return 0;
}
