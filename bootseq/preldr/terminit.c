/*  terminal blackbox init
 *  (setting term_entry)
 */
 
#include "term.h"
#include "fsd.h"

#pragma aux relshift    "*"
#pragma aux base        "*"
#pragma aux base32      "*"
#pragma aux set_gdt     "*"

#pragma aux u_msg "*"

void set_gdt(void);

extern base;
struct term_entry *t;
unsigned int relshift;
unsigned int base32;
unsigned int s;

void init(struct term_entry *te, unsigned int shift)
{
  unsigned int *p;
  t = te;
#ifdef TERM_CONSOLE
  te->flags         = 0;
  te->putchar       = &console_putchar;
  te->checkkey      = &console_checkkey;
  te->getkey        = &console_getkey;
  te->getxy         = &console_getxy;
  te->gotoxy        = &console_gotoxy;
  te->cls           = &console_cls;
  te->setcolorstate = &console_setcolorstate;
  te->setcolor      = &console_setcolor;
  te->setcursor     = &console_setcursor;
#else
#ifdef TERM_HERCULES
  te->flags         = 0;
  te->putchar       = &hercules_putchar;
  te->checkkey      = &console_checkkey;
  te->getkey        = &console_getkey;
  te->getxy         = &hercules_getxy;
  te->gotoxy        = &hercules_gotoxy;
  te->cls           = &hercules_cls;
  te->setcolorstate = &hercules_setcolorstate;
  te->setcolor      = &hercules_setcolor;
  te->setcursor     = &hercules_setcursor;
#else
#ifdef TERM_SERIAL
  te->flags         = 0; //TERM_NEED_INIT;
  te->putchar       = &serial_putchar;
  te->checkkey      = &serial_checkkey;
  te->getkey        = &serial_getkey;
  te->getxy         = &serial_getxy;
  te->gotoxy        = &serial_gotoxy;
  te->cls           = &serial_cls;
  te->setcolorstate = &serial_setcolorstate;
  te->setcolor      = 0;
  te->setcursor     = 0;
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
