/*
 *
 *
 */

#include <shared.h>
#include "serial.h"
#include "fsd.h"

#pragma aux m          "*"
#pragma aux rel_start  "*"
#pragma aux ufsd_start "*"
#pragma aux ufsd_size  "*"

/* multiboot structure pointer */
extern struct multiboot_info *m;
/* uFSD relocation info start  */
extern unsigned long rel_start;
/* uFSD start                  */
extern unsigned long ufsd_start;
/* uFSD size                   */
extern unsigned long ufsd_size;

int kprintf(const char *format, ...);
int serial_init (unsigned short port, unsigned int speed,
                int word_len, int parity, int stop_bit_len);

void init (void)
{

}

/*  Relocate a file in memory using its
 *  .rel file.
 *  base is a file base, rel_start is .rel
 *  file in memory pointer and shift is the
 *  relocation shift
 */
void reloc(char *base, char *rel_start, unsigned long shift)
{
  int  i, n, rc;

  typedef _Packed struct {
    unsigned short addr;
    unsigned char  shift;
  } rel_item;

  rel_item *p;
  unsigned long *addr;

  /* number of reloc items */
  n = *((unsigned short *)(rel_start)) / 3;
  p = (rel_item *)(rel_start + 2);

  for (i = 0; i < n; i++) {
    addr  = (unsigned long *)(base + p[i].addr);
    *addr += shift >> p[i].shift;
  }
}

int cmain(void)
{
  int relshift;
  unsigned int uFSD_base;
  char *p;
  unsigned short *d;
  int  port;

  if (p = strstr((char *)m->cmdline, "--serial"))
  {
    p = skip_to(1, p);
    safe_parse_maxint(&p, &port);
  }

  // init serial port
  serial_init(port, 9600, UART_8BITS_WORD, UART_NO_PARITY, UART_1_STOP_BIT);

  kprintf("Hello MBI OS/2 booter!\n");
  kprintf("comport = 0x%x\n", port);
  // where to copy uFSD
  uFSD_base = ((m->mem_lower << 10) - 0x10000 - ufsd_size - 0x3000 - 0x200) & 0xfffc000;
  kprintf("uFSD base: 0x%08lx\n", uFSD_base);

  // a shift relative to REL1_BASE
  relshift  = uFSD_base - REL1_BASE;
  kprintf("relshift: %ld\n", relshift);

  // copy uFSD where needed
  memmove((char *)uFSD_base, (char *)&ufsd_start, ufsd_size);
  kprintf("uFSD is moved...\n");

  // save port value in 16-bit area in uFSD header
  d  = (unsigned short *)(uFSD_base + 0x24);
  *d = port;

  // fixup uFSD
  reloc((char *)uFSD_base, (char *)&rel_start, relshift);
  kprintf("uFSD is relocated...\n");

  return uFSD_base;
}
