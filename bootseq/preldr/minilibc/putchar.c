/* Display an ASCII character.  */


#include <shared.h>
#include "libc.h"
#include "fsys.h"
#include "term.h"

extern struct term_entry *t;
/* if use terminal or not (on early stages) */
unsigned char use_term = 1;

void
grub_putchar (int c)
{
  char buf[2];

  if (!use_term)
  {
    buf[0] = (char)c;
    buf[1] = '\0';
    u_msg(buf);
  }
  else
  {
    t->putchar(c);
  }
}

