/*
 *
 *
 */

#include "term.h"
#include "fsd.h"

struct term_entry term, *t;
unsigned long base32;

#pragma aux base32 "*"

#pragma aux console_putchar   "*"
#pragma aux console_getxy     "*"
#pragma aux console_gotoxy    "*"
#pragma aux console_cls       "*"
#pragma aux console_setcursor "*"
#pragma aux console_getkey    "*"
#pragma aux console_checkkey  "*"

void terminit(void)
{
  base32 = REL1_BASE;
  t = &term;

  t->flags         = 0;
  t->putchar       = &console_putchar;
  t->checkkey      = &console_checkkey;
  t->getkey        = &console_getkey;
  t->getxy         = &console_getxy;
  t->gotoxy        = &console_gotoxy;
  t->cls           = &console_cls;
  t->setcolorstate = &console_setcolorstate;
  t->setcolor      = &console_setcolor;
  t->setcursor     = &console_setcursor;
}
