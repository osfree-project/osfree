/*  Output to serial port
 *
 */

#define COM1_BASE 0x3f8
#define COM2_BASE 0x2f8

// which commport to use
#define COMPORT COM1_BASE

#include <stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);

char buf[0x400];

/* Read a byte from a port.  */
static inline unsigned char
inb (unsigned short port)
{
  unsigned char value;

  __asm {
    mov dx, port
    in  al, dx
    out 80h, al
    mov value, al
  }

  return value;
}

/* Write a byte to a port.  */
static inline void
outb (unsigned short port, unsigned char value)
{
  __asm {
    mov dx, port
    mov al, value
    out dx, al
    out 80h, al
  }
}

void serout(char *s)
{
  char *p = s;

  while (*p)
  {
    while (!(inb(COMPORT + 5) & 0x20)); // wait while comport is ready
    outb(COMPORT, *p++);
  }
  if (*p == '\n')
  {
    while (!(inb(COMPORT + 5) & 0x20)); // wait while comport is ready
    outb(COMPORT, '\r');
    while (!(inb(COMPORT + 5) & 0x20)); // wait while comport is ready
    outb(COMPORT, '\n');
  }
}

int kprintf(const char *format, ...)
{
  va_list arg;

  va_start(arg, format);
  vsprintf(buf, format, arg);
  va_end(arg);

  serout(buf);

  return 0;
}
