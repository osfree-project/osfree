/****************************************************************/
/*								*/
/*			      prf.c				*/
/*								*/
/*		    Abbreviated printf Function			*/
/*								*/
/*			Copyright (c) 1995			*/
/*			Pasquale J. Villani			*/
/*			All Rights Reserved			*/
/*								*/
/* This file is part of DOS-C.					*/
/*								*/
/* DOS-C is free software; you can redistribute it and/or	*/
/* modify it under the terms of the GNU General Public License	*/
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.			*/
/*								*/
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of	*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See	*/
/* the GNU General Public License for more details.		*/
/*								*/
/* You should have received a copy of the GNU General Public	*/
/* License along with DOS-C; see the file COPYING.  If not,	*/
/* write to the Free Software Foundation, 675 Mass Ave,		*/
/* Cambridge, MA 02139, USA.					*/
/****************************************************************/

#include <io.h>
#include <conio.h>
#include <dos.h>
#include <stdarg.h>
#include <stdlib.h>
#include <bios.h>

#include "kitten.h"
#define _(set,message_number,message) kittengets(set,message_number,message)

#ifdef __WATCOMC__
#define bioskey(x) getch()
#endif

#define FALSE 0
#define TRUE 1

static char *charp = 0;

#ifndef __WATCOMC__
int fstrlen(char far * s)
{
  int i = 0;

  while (*s++)
    i++;

  return i;
}
#endif

extern int num_lines;

static void do_printf(const char * fmt, va_list arg);
int printf(const char * fmt, ...);

/* paging printf() function...added by brian reifsnyder */
void put_console(int c)
{
  static int line_counter = 0;

  if (c == '\n')
  {
    if(num_lines >= 0)
    {
      line_counter++;
      if(line_counter >= num_lines - 1)
      {
	line_counter = 0;
	printf(_(8,0,"\nPress <Enter> to continue or <Esc> to exit . . ."));
	if (bioskey(0) == 27)
	    exit(1);
	line_counter = 0;
      }
    }
    put_console('\r');
  }

#ifdef __WATCOMC__
  {
    unsigned bytes;
    _dos_write(1, &c, 1, &bytes);
  }
#else
  write(1, &c, 1);		/* write character to stdout */
#endif
}

/* special handler to switch between sprintf and printf */
static void handle_char(int c)
{
  if (charp == 0)
    put_console(c);
  else
    *charp++ = c;
}

#define LEFT	0
#define RIGHT	1
#define ZEROSFILL 2
#define LONGARG 4

/* printf -- short version of printf to conserve space */
int vprintf(const char * fmt, va_list arg)
{
  charp = 0;
  do_printf(fmt, arg);
  return 0;
}

int printf(const char * fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  charp = 0;
  do_printf(fmt, arg);
  return 0;
}

int sprintf(char * buff, const char * fmt, ...)
{
  va_list arg;

  va_start(arg, fmt);
  charp = buff;
  do_printf(fmt, arg);
  handle_char('\0');
  return charp - buff - 1;
}

static void do_printf(const char * fmt, va_list arg)
{
  int base;
  char s[11], far * p;
  int size;
  unsigned char flags;

  for (;*fmt != '\0'; fmt++)
  {
    if (*fmt != '%')
    {
      handle_char(*fmt);
      continue;
    }

    fmt++;
    flags = RIGHT;

    if (*fmt == '-')
    {
      flags = LEFT;
      fmt++;
    }

    if (*fmt == '0')
    {
      flags |= ZEROSFILL;
      fmt++;
    }

    size = 0;
    while (1)
    {
      unsigned c = (unsigned char)(*fmt - '0');
      if (c > 9)
	break;
      fmt++;
      size = size * 10 + c;
    }

    if (*fmt == 'l')
    {
      flags |= LONGARG;
      fmt++;
    }

    switch (*fmt)
    {
      case '\0':
	va_end(arg);
	return;

      case 'c':
	handle_char(va_arg(arg, int));
	continue;

      case 'p':
	{
	  unsigned w0 = va_arg(arg, unsigned);
	  char *tmp = charp;
	  sprintf(s, "%04x:%04x", va_arg(arg, unsigned), w0);
	  p = s;
	  charp = tmp;
	  break;
	}

      case 's':
	p = va_arg(arg, char *);
	break;

      case 'F':
	fmt++;
	/* we assume %Fs here */
      case 'S':
	p = va_arg(arg, char far *);
	break;

      case 'i':
      case 'd':
	base = -10;
	goto lprt;

      case 'o':
	base = 8;
	goto lprt;

      case 'u':
	base = 10;
	goto lprt;

      case 'X':
      case 'x':
	base = 16;

    lprt:
	{
	  long currentArg;
	  if (flags & LONGARG)
	    currentArg = va_arg(arg, long);
	  else
	  {
	    currentArg = va_arg(arg, int);
	    if (base >= 0)
	      currentArg =  (long)(unsigned)currentArg;
	  }
	  if (base<0)
	    ltoa(currentArg, s, -base);
	  else
	    ultoa(currentArg, s, base);
	  p = s;
	}
	break;

      default:
	handle_char('?');

	handle_char(*fmt);
	continue;

    }
    {
      size_t i = 0;
      while(p[i]) i++;
      size -= i;
    }

    if (flags & RIGHT)
    {
      int ch = ' ';
      if (flags & ZEROSFILL) ch = '0';
      for (; size > 0; size--)
	handle_char(ch);
    }
    for (; *p != '\0'; p++)
      handle_char(*p);

    for (; size > 0; size--)
      handle_char(' ');
  }
  va_end(arg);
}

#ifdef TEST
/*
	this testprogram verifies that the strings are printed correctly
	( or the way, I expect them to print)

	compile like (note -DTEST !)

	c:\tc\tcc -DTEST -DI86 -I..\hdr prf.c

	and run. if strings are wrong, the program will wait for the ANYKEY

*/
#include <stdio.h>
#include <string.h>

void cso(char c)
{
  putchar(c);
}

struct {
  char *should;
  char *format;
  unsigned lowint;
  unsigned highint;

} testarray[] =	{
  {
  "hello world", "%s %s", (unsigned)"hello", (unsigned)"world"},
  {
  "hello", "%3s", (unsigned)"hello", 0},
  {
  "  hello", "%7s", (unsigned)"hello", 0},
  {
  "hello  ", "%-7s", (unsigned)"hello", 0},
  {
  "hello", "%s", (unsigned)"hello", 0},
  {
  "1", "%d", 1, 0},
  {
  "-1", "%d", -1, 0},
  {
  "65535", "%u", -1, 0},
  {
  "-32768", "%d", 0x8000, 0},
  {
  "32767", "%d", 0x7fff, 0},
  {
  "-32767", "%d", 0x8001, 0},
  {
  "8000", "%x", 0x8000, 0},
  {
  "   1", "%4x", 1, 0},
  {
  "0001", "%04x", 1, 0},
  {
  "1   ", "%-4x", 1, 0},
  {
  "1   ", "%-04x", 1, 0},
  {
  "1", "%ld", 1, 0},
  {
  "-1", "%ld", -1, -1},
  {
  "65535", "%ld", -1, 0},
  {
  "65535", "%u", -1, 0},
  {
  "8000", "%lx", 0x8000, 0},
  {
  "80000000", "%lx", 0, 0x8000},
  {
  "   1", "%4lx", 1, 0},
  {
  "0001", "%04lx", 1, 0},
  {
  "1   ", "%-4lx", 1, 0},
  {
  "1   ", "%-04lx", 1, 0},
  {
  "-2147483648", "%ld", 0, 0x8000},
  {
  "2147483648", "%lu", 0, 0x8000},
  {
  "2147483649", "%lu", 1, 0x8000},
  {
  "-2147483647", "%ld", 1, 0x8000},
  {
  "32767", "%ld", 0x7fff, 0},
  {
"ptr 1234:5678", "ptr %p", 0x5678, 0x1234}, {0}};

void test(char *should, char *format, unsigned lowint, unsigned highint)
{
  char b[100];

  sprintf(b, format, lowint, highint);

  printf("'%s' = '%s'\n", should, b);

  if (strcmp(b, should))
  {
    printf("\nhit ENTER\n");
    getchar();
  }
}

int main(void)
{
  int i;
  printf("hello world\n");

  for (i = 0; testarray[i].should; i++)
  {
    test(testarray[i].should, testarray[i].format, testarray[i].lowint,
	 testarray[i].highint);
  }
  return 0;
}
#endif

