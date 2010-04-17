/*  Output to serial port
 *
 */

#define COM1_BASE 0x3f8
#define COM2_BASE 0x2f8

// which commport to use
#define COMPORT COM1_BASE

#include <stdarg.h>
#include <serial.h>

#ifndef MINIFSD
#define memmove grub_memmove
#else
#define grub_memmove memmove
#endif

/* An input buffer.  */
//static char input_buf[8];
//static int npending = 0;

int vsprintf(char *buf, const char *fmt, va_list args);
void *memmove (void *_to, const void *_from, int _len);

char buf[0x400];

struct divisor
{
  long speed;
  unsigned short div;
};

/* Store the port number of a serial unit.  */
unsigned short serial_hw_port = 0;
extern char debug;

/* The table which lists common configurations.  */
static struct divisor divisor_tab[] =
  {
    { 2400,   0x0030 },
    { 4800,   0x0018 },
    { 9600,   0x000C },
    { 19200,  0x0006 },
    { 38400,  0x0003 },
    { 57600,  0x0002 },
    { 115200, 0x0001 }
  };

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

void comwait(unsigned short port)
{
  while (!(inb(port + 5) & 0x20)) ; // wait while comport is ready
}

/* Fetch a key.  */
/*
int
serial_hw_fetch (void)
{
  if (inb (serial_hw_port + UART_LSR) & UART_DATA_READY)
    return inb (serial_hw_port + UART_RX);

  return -1;
}
*/

void comout(unsigned short port, unsigned char c)
{
  if (!debug)
    return;

  comwait(port);
  outb(port, c);
}

void serout(unsigned short port, char *s)
{
  char *p = s;

  if (!debug) return;

  while (*p)
  {
    if (*p == '\n')
    {
      comout(port, '\r');
      comout(port, '\n');
    }
    else
      comout(port, *p);
    p++;
  }
}

#if 0

static int __cdecl
fill_input_buf (int nowait)
{
  int i;

  for (i = 0; i < 10000 && npending < sizeof (input_buf); i++)
    {
      int c;

      c = serial_hw_fetch ();
      if (c >= 0)
        {
          input_buf[npending++] = c;

          /* Reset the counter to zero, to wait for the same interval.  */
          i = 0;
        }

      if (nowait)
        break;
    }

  /* Translate some key sequences.  */
  //serial_translate_key_sequence ();

  return npending;
}

/* The serial version of getkey.  */
int __cdecl
serial_getkey (void)
{
  int c;

  while (! fill_input_buf (0))
    ;

  npending--;
  if ((c = input_buf[0]))
        memmove (input_buf, input_buf + 1, npending);
  else
  {
        npending--;
        c = *(unsigned short *)input_buf;
        memmove (input_buf, input_buf + 2, npending);
  }

  return c;
}

/* The serial version of checkkey.  */
int __cdecl
serial_checkkey (void)
{
  if (fill_input_buf (1))
    return input_buf[0] ? input_buf[0] : *(unsigned short *)input_buf;

  return -1;
}

#endif

int
serial_init (long port, long speed,
             int word_len, int parity, int stop_bit_len)
{
  int i;
  unsigned short div = 0;
  unsigned char status = 0;

  if (!debug)
    return 0;

  /* Turn off the interrupt.  */
  outb (port + UART_IER, 0);

  /* Set DLAB.  */
  outb (port + UART_LCR, UART_DLAB);

  /* Set the baud rate.  */
  for (i = 0; i < sizeof (divisor_tab) / sizeof (divisor_tab[0]); i++)
    if (divisor_tab[i].speed == speed)
      {
        div = divisor_tab[i].div;
        break;
      }

  if (div == 0)
    return 0;

  outb (port + UART_DLL, div & 0xFF);
  outb (port + UART_DLH, div >> 8);

  /* Set the line status.  */
  status |= parity | word_len | stop_bit_len;
  outb (port + UART_LCR, status);

  /* Enable the FIFO.  */
  outb (port + UART_FCR, UART_ENABLE_FIFO);

  /* Turn on DTR, RTS, and OUT2.  */
  outb (port + UART_MCR, UART_ENABLE_MODEM);

  /* Store the port number.  */
  serial_hw_port = port;

  /* Drain the input buffer.  */
  //while (serial_checkkey () != -1)
  //  serial_getkey ();

  /* Get rid of TERM_NEED_INIT from the serial terminal.  */
  /*
  //for (i = 0; term_table[i].name; i++)
  //  if (grub_strcmp (term_table[i].name, "serial") == 0)
  //    {
  //      term_table[i].flags &= ~TERM_NEED_INIT;
  //      break;
  //    } */

  /* FIXME: should check if the serial terminal was found.  */

  return 1;
}

int kprintf(const char *format, ...)
{
  va_list arg;

  if (!debug)
    return 1;

  if (!serial_hw_port)
    return 1;

  va_start(arg, format);
  vsprintf(buf, format, arg);
  va_end(arg);

  serout(serial_hw_port, buf);

  /* Drain the input buffer.  */
  //while (serial_checkkey () != -1)
  //  serial_getkey ();

  return 0;
}
