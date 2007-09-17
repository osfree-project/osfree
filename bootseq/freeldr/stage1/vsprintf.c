//
// $Header: d:\\32bits\\ext2-os2\\microfsd\\rcs\\vsprintf.c,v 1.3 1997/03/15 22:24:42 Willm Exp $
//

// 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
// access your Linux ext2fs partitions as normal drive letters.
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <stdarg.h>
#ifndef OS2
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#else
#include <string.h>
#include <ctype.h>
//#include <os2/types.h>
//#include <os2/magic.h>
//#include <microfsd.h>
#include "freeldr.h"
#include "vsprintf.h"
//#include <i86.h>
char   scratch_buffer[1024];
extern unsigned short current_seg;
#endif

//Вывод строки, заканчивающейся нуль-символом
void __cdecl puts(char *s)
{
  DispNTS(s);
}


//Вывод одного символа в режиме телетайпа
void __cdecl putchar(char c)
{
  DispC(&c);
}


int
strcmp (const char *s1, const char *s2)
{
  while (*s1 || *s2)
    {
      if (*s1 < *s2)
        return -1;
      else if (*s1 > *s2)
        return 1;
      s1 ++;
      s2 ++;
    }

  return 0;
}

unsigned int
strlen (const char *str)
{
  int len = 0;

  while (*str++ != '\0')
    len++;

  return len;
}


size_t strnlen(const char * s, size_t count)
{
        const char *sc;

        for (sc = s; *sc != '\0' && count--; ++sc)
                /* nothing */;
        return sc - s;
}

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
        unsigned long result = 0,value;

        if (!base) {
                base = 10;
                if (*cp == '0') {
                        base = 8;
                        cp++;
                        if ((*cp == 'x') && isxdigit(cp[1])) {
                                cp++;
                                base = 16;
                        }
                }
        }
        while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
            ? toupper(*cp) : *cp)-'A'+10) < base) {
                result = result*base + value;
                cp++;
        }
        if (endp)
                *endp = (char *)cp;
        return result;
}

/* we use this so that we can do without the ctype library */
#define is_digit(c)     ((c) >= '0' && (c) <= '9')

static int skip_atoi(const char near *(far *s))
{
        int i=0;

        while (is_digit(**s))
                i = i*10 + *((*s)++) - '0';
        return i;
}

#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define PLUS    4               /* show plus */
#define SPACE   8               /* space if plus */
#define LEFT    16              /* left justified */
#define SPECIAL 32              /* 0x */
#define LARGE   64              /* use 'ABCDEF' instead of 'abcdef' */

#ifndef OS2
#define do_div(n,base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })
#endif

static char * number(char * str, long num, int base, int size, int precision
        ,int type)
{
        char c,sign,tmp[66];
        const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
        int i;

        if (type & LARGE)
                digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if (type & LEFT)
                type &= ~ZEROPAD;
        if (base < 2 || base > 36)
                return 0;
        c = (type & ZEROPAD) ? '0' : ' ';
        sign = 0;
        if (type & SIGN) {
                if (num < 0) {
                        sign = '-';
                        num = -num;
                        size--;
                } else if (type & PLUS) {
                        sign = '+';
                        size--;
                } else if (type & SPACE) {
                        sign = ' ';
                        size--;
                }
        }
        if (type & SPECIAL) {
                if (base == 16)
                        size -= 2;
                else if (base == 8)
                        size--;
        }
        i = 0;
        if (num == 0)
                tmp[i++]='0';
#ifndef OS2
        else while (num != 0)
                tmp[i++] = digits[do_div(num,base)];
#else
        else while (num != 0) {
                int __res;
//                unsigned long __res;
                __res = ((unsigned long) num) % (unsigned long) base;
                num = ((unsigned long) num) / (unsigned long) base;
                tmp[i++] = digits[__res];
             }
#endif
        if (i > precision)
                precision = i;
        size -= precision;
        if (!(type&(ZEROPAD+LEFT)))
                while(size-->0)
                        *str++ = ' ';
        if (sign)
                *str++ = sign;
        if (type & SPECIAL)
                if (base==8)
                        *str++ = '0';
                else if (base==16) {
                        *str++ = '0';
                        *str++ = digits[33];
                }
        if (!(type & LEFT))
                while (size-- > 0)
                        *str++ = c;
        while (i < precision--)
                *str++ = '0';
        while (i-- > 0)
                *str++ = tmp[i];
        while (size-- > 0)
                *str++ = ' ';
        return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
        int len;
        unsigned long num;
        int i, base;
        char * str;
        char *s;

        int flags;              /* flags to number() */

        int field_width;        /* width of output field */
        int precision;          /* min. # of digits for integers; max
                                   number of chars for from string */
        int qualifier;          /* 'h', 'l', or 'L' for integer fields */

        for (str=buf ; *fmt ; ++fmt) {
                if (*fmt != '%') {
                        *str++ = *fmt;
                        continue;
                }

                /* process flags */
                flags = 0;
                repeat:
                        ++fmt;          /* this also skips first '%' */
                        switch (*fmt) {
                                case '-': flags |= LEFT; goto repeat;
                                case '+': flags |= PLUS; goto repeat;
                                case ' ': flags |= SPACE; goto repeat;
                                case '#': flags |= SPECIAL; goto repeat;
                                case '0': flags |= ZEROPAD; goto repeat;
                                }

                /* get field width */
                field_width = -1;
                if (is_digit(*fmt))
                        field_width = skip_atoi(&fmt);
                else if (*fmt == '*') {
                        ++fmt;
                        /* it's the next argument */
                        field_width = va_arg(args, int);
                        if (field_width < 0) {
                                field_width = -field_width;
                                flags |= LEFT;
                        }
                }

                /* get the precision */
                precision = -1;
                if (*fmt == '.') {
                        ++fmt;
                        if (is_digit(*fmt))
                                precision = skip_atoi(&fmt);
                        else if (*fmt == '*') {
                                ++fmt;
                                /* it's the next argument */
                                precision = va_arg(args, int);
                        }
                        if (precision < 0)
                                precision = 0;
                }

                /* get the conversion qualifier */
                qualifier = -1;
                if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
                        qualifier = *fmt;
                        ++fmt;
                }

                /* default base */
                base = 10;

                switch (*fmt) {
                case 'c':
                        if (!(flags & LEFT))
                                while (--field_width > 0)
                                        *str++ = ' ';
                        *str++ = (unsigned char) va_arg(args, int);
                        while (--field_width > 0)
                                *str++ = ' ';
                        continue;

                case 's':
                        s = va_arg(args, char *);
                        if (!s)
                                s = "<NULL>";

                        len = strnlen(s, precision);

                        if (!(flags & LEFT))
                                while (len < field_width--)
                                        *str++ = ' ';
                        for (i = 0; i < len; ++i)
                                *str++ = *s++;
                        while (len < field_width--)
                                *str++ = ' ';
                        continue;

                case 'p':
                        if (field_width == -1) {
                                field_width = 2*sizeof(void *);
                                flags |= ZEROPAD;
                        }
                        str = number(str,
                                (unsigned long) va_arg(args, void *), 16,
                                field_width, precision, flags);
                        continue;


                case 'n':
                        if (qualifier == 'l') {
                                long * ip = va_arg(args, long *);
                                *ip = (str - buf);
                        } else {
                                int * ip = va_arg(args, int *);
                                *ip = (str - buf);
                        }
                        continue;

                /* integer number formats - set up the flags and "break" */
                case 'o':
                        base = 8;
                        break;

                case 'X':
                        flags |= LARGE;
                case 'x':
                        base = 16;
                        break;

                case 'd':
                case 'i':
                        flags |= SIGN;
                case 'u':
                        break;

                default:
                        if (*fmt != '%')
                                *str++ = '%';
                        if (*fmt)
                                *str++ = *fmt;
                        else
                                --fmt;
                        continue;
                }
                if (qualifier == 'l')
                        num = va_arg(args, unsigned long);
                else if (qualifier == 'h')
                        if (flags & SIGN)
                                num = va_arg(args, short);
                        else
                                num = va_arg(args, unsigned short);
                else if (flags & SIGN)
                        num = va_arg(args, int);
                else
                        num = va_arg(args, unsigned int);
                str = number(str, num, base, field_width, precision, flags);
        }
        *str = '\0';
        return str-buf;
}

int sprintf(char * buf, const char *fmt, ...)
{
        va_list args;
        int i;

        va_start(args, fmt);
        i=vsprintf(buf,fmt,args);
        va_end(args);
        return i;
}

#if 0
void output_com(char *bufptr, int debug_port) {
    while (*bufptr) {
        while(!(_inp(debug_port + 5) & 0x20));  // Waits for COM port to be ready
        _outp(debug_port, *bufptr++);           // Outputs our character
    }
    while(!(_inp(debug_port + 5) & 0x20));      // Waits for COM port to be ready
    _outp(debug_port, '\r');                    // Outputs our character
    while(!(_inp(debug_port + 5) & 0x20));      // Waits for COM port to be ready
    _outp(debug_port, '\n');                    // Outputs our character
}
#endif

void outchar_comport(char c, int port) {
    __asm {
        .286
        push ds
        push es
        pusha
        mov ah, 01h
        mov dx, port
        mov al, c
        int 14h
        popa
        pop es
        pop ds
    };
}

void output_com(char *bufptr ,int debug_port) {
    while(*bufptr)
        outchar_comport(*bufptr++, debug_port);
    outchar_comport('\r', debug_port);
    outchar_comport('\n', debug_port);
}

void output_comc(char *bufptr ,int debug_port) {
    while(*bufptr)
        outchar_comport(*bufptr++, debug_port);
    //outchar_comport('\r', debug_port);
    //outchar_comport('\n', debug_port);
}

#define OUTPUT_COM1 1
#define OUTPUT_COM2 0

int __cdecl
printk(const char *fmt, ...) {
    va_list args;
    //char   scratch_buffer[1024];
    char   *bufptr = scratch_buffer;
    char   *save;

    va_start(args, fmt);
    vsprintf(scratch_buffer, fmt, args);
    va_end(args);

    video_output(scratch_buffer, strlen(scratch_buffer));
    video_crlf();
    output_com(bufptr, OUTPUT_COM1);
    output_com(bufptr, OUTPUT_COM2);

    return 0;
}


int __cdecl
printkc(const char *fmt, ...) {
    va_list args;
    //char   scratch_buffer[1024];
    char   *bufptr = scratch_buffer;
    char   *save;

    va_start(args, fmt);
    vsprintf(scratch_buffer, fmt, args);
    va_end(args);

    video_output(scratch_buffer, strlen(scratch_buffer));
    output_comc(bufptr, OUTPUT_COM1);
    output_comc(bufptr, OUTPUT_COM2);

    return 0;
}

int __far __cdecl
freeldr_printk(const char *fmt, ...) {
    va_list args;
    //char   scratch_buffer[1024];
    char   *bufptr = scratch_buffer;
    char   *save;

    va_start(args, fmt);
    vsprintf(scratch_buffer, fmt, args);
    va_end(args);

    video_output(scratch_buffer, strlen(scratch_buffer));
    video_crlf();
    output_com(bufptr, OUTPUT_COM1);
    output_com(bufptr, OUTPUT_COM2);

    return 0;
}


int __far __cdecl
freeldr_printkc(const char *fmt, ...) {
    va_list args;
    //char   scratch_buffer[1024];
    char   *bufptr = scratch_buffer;
    char   *save;

    va_start(args, fmt);
    vsprintf(scratch_buffer, fmt, args);
    va_end(args);

    video_output(scratch_buffer, strlen(scratch_buffer));
    output_comc(bufptr, OUTPUT_COM1);
    output_comc(bufptr, OUTPUT_COM2);

    return 0;
}
