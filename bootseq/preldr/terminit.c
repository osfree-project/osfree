/*  terminal blackbox init
 *  (setting term_entry)
 */
 
#include "term.h"
#include "fsd.h"

#pragma aux relshift    "*"
#pragma aux base        "*"
#pragma aux base32      "*"
#pragma aux set_gdt     "*"

void set_gdt(void);

extern base;
unsigned int relshift;
unsigned int base32;

unsigned int s;

void init(struct term_entry *t, unsigned int shift)
{
  unsigned int *p;
#ifdef TERM_CONSOLE
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
#else
#ifdef TERM_HERCULES
  t->flags         = 0;
  t->putchar       = &hercules_putchar;
  t->checkkey      = &console_checkkey;
  t->getkey        = &console_getkey;
  t->getxy         = &hercules_getxy;
  t->gotoxy        = &hercules_gotoxy;
  t->cls           = &hercules_cls;
  t->setcolorstate = &hercules_setcolorstate;
  t->setcolor      = &hercules_setcolor;
  t->setcursor     = &hercules_setcursor;
#else
#ifdef TERM_SERIAL
  t->flags         = 0; //TERM_NEED_INIT;
  t->putchar       = &serial_putchar;
  t->checkkey      = &serial_checkkey;
  t->getkey        = &serial_getkey;
  t->getxy         = &serial_getxy;
  t->gotoxy        = &serial_gotoxy;
  t->cls           = &serial_cls;
  t->setcolorstate = &serial_setcolorstate;
  t->setcolor      = 0;
  t->setcursor     = 0;
#endif
#endif
#endif
  // remember a pre-loader relocation shift
  relshift = shift;
  // base is located in 16-bit realmode segment.
  // it is a base of segment before relocation.
  // adding shift to it gives us its value after relocation.
  s = 0x10; // base offset
  p = (unsigned int *)((char *)s + (TERMLO_BASE + shift));
  base32 = *p;
  *p += shift;
  base32 += shift;
  // set bases of GDT descriptors
  set_gdt();
}

void cmain(void)
{

}
