/* Display an ASCII character.  */


#include <shared.h>
#include "libc.h"
#include "fsys.h"

void
grub_putchar (int c)
{
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  u_msg(buf);
}
