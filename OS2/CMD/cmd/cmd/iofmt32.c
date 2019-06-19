// IOFMT32.C
// Smaller (s)scanf() / (s)printf() replacements
// Copyright (c) 1993 - 1996  Rex C. Conn  All rights reserved.

#include "product.h"

#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "4all.h"

static int _fmtin(char *, const char *, va_list);
static int _fmtout(int, char *, const char *, va_list);
static void WriteString(char **, const char *, int, int, int);


// INPUT
//    %[*][width][size]type
//	  *:
//	      Scan but don't assign this field.
//	  width:
//	      Controls the maximum # of characters read.
//	  size:
//	      F     Specifies argument is FAR
//	      l     Specifies 'd', 'u' is long
//            h     Specifies 'd', 'u' is short
//	  type:
//	      c     character
//	      s     string
//	     [ ]    array of characters
//	      d     signed integer
//	      u     unsigned integer
//            x     hex integer
//	      n     number of characters read so far

static int _fmtin(char *source, const char *fmt, va_list arg)
{
	char *base, *fptr;
	char ignore, reject, using_table, sign_flag;
	int n, fields = 0, width, value;
	char table[256];		// ASCII table for %[^...]

	// save start point (for %n)
	base = source;

	while (*fmt != '\0') {

		if (*fmt == '%') {

			ignore = reject = using_table = sign_flag = 0;

			if (*(++fmt) == '*') {
				fmt++;
				ignore++;
			}

			// get max field width
			if ((*fmt >= '0') && (*fmt <= '9')) {
				for (width = atoi(fmt); ((*fmt >= '0') && (*fmt <= '9')); fmt++)
					;
			} else
				width = INT_MAX;

next_fmt:
			switch (*fmt) {

			case 'l':		// long int
			case 'F':		// far pointer
				fmt++;
				goto next_fmt;

			case '[':
				using_table++;
				if (*(++fmt) == '^') {
					reject++;
					fmt++;
				}

				memset( table, reject, 256 );
				for ( ; ((*fmt) && (*fmt != ']')); fmt++)
					table[*fmt] ^= 1;

			case 'c':
			case 's':

				if ((*fmt == 'c') && (width == INT_MAX))
					width = 1;
				else if (*fmt == 's') {
					// skip leading whitespace
					while ((*source == ' ') || (*source == '\t'))
						source++;
				}

				if (ignore == 0)
					fptr = (char *)(va_arg(arg, char *));

				for ( ; ((width > 0) && (*source)); source++, width--) {

					if (using_table) {
						if (table[*source] == 0)
							break;
					} else if ((*fmt == 's') && ((*source == ' ') || (*source == '\t')))
						break;

					if (ignore == 0)
						*fptr++ = *source;
				}

				if (ignore == 0) {
					if (*fmt != 'c')
						*fptr = '\0';
					fields++;
				}
				break;

			case 'd':
			case 'u':
			case 'x':
				// skip leading whitespace
				while ((*source == ' ') || (*source == '\t'))
					source++;

				if (*fmt == 'x') {

				    for (fptr = source, value = 0; ((width > 0) && (isxdigit(*source))); source++, width--) {
					n = ((isdigit(*source)) ? *source - '0' : (_ctoupper(*source) - 'A') + 10);
					value = (value * 16) + n;
				    }

				} else {

				    // check for leading sign
				    if (*source == '-') {
					sign_flag++;
					source++;
				    } else if (*source == '+')
					source++;

				    for (fptr = source, value = 0; ((width > 0) && (*source >= '0') && (*source <= '9')); source++, width--)
					value = (value * 10) + (*source - '0');

				    if (sign_flag)
					value = -value;
				}

			case 'n':
				// number of characters read so far
				if (*fmt == 'n')
					value = (int)(source - base);

				if (ignore == 0) {

					*(va_arg(arg,unsigned int *)) = (unsigned int)value;

					// if start was a digit, inc field count
					if ((*fmt == 'n') || ((*fptr >= '0') && (*fptr <= '9')))
						fields++;
				}
				break;

			default:
				if (*fmt != *source++)
					return fields;
			}

			fmt++;

		} else if (iswhite(*fmt)) {

			// skip leading whitespace
			while ((*fmt == ' ') || (*fmt == '\t'))
				fmt++;
			while ((*source == ' ') || (*source == '\t'))
				source++;

		} else if (*fmt++ != *source++)
			return fields;
	}

	return fields;
}


// OUTPUT
//  %[flags][width][.precision][size]type
//    flags:
//	- left justify the result
//	+ right justify the result
//    width:
//	Minimum number of characters to write (pad with ' ' or '0').
//	If width is a *, get it from the next argument in the list.
//    .precision
//	Precision - Maximum number of characters or digits for output field
//	If precision is a *, get it from the next argument in the list.
//    size:
//	F     Specifies argument is FAR
//	l     Specifies 'd', 'u', 'x' is long
//      L     Use commas in numeric field
//    type:
//	c     character
//	s     string
//	[ ]    array of characters
//	d     signed integer
//	u     unsigned integer
//      x     hex integer
//	n     number of characters written so far

// write a string to memory
static void WriteString(char **pdest, const char *str, int minlen, int maxlen, int fill)
{
	int count;

	if ((count = strlen(str)) > maxlen)
		count = maxlen;

	// right justified?
	if (minlen > count) {
		memset( *pdest, fill, (minlen - count));
		*pdest += minlen - count;
	}

	memmove( *pdest, str, count );
	*pdest += count;

	// left justified?
	if ((minlen = -minlen) > count) {
		memset( *pdest, fill, (minlen - count));
		*pdest += (minlen - count);
	}
}


// This routine does the actual parsing & formatting of the (s)printf() statement
static int _fmtout(int handle, char *dest, const char *fmt, va_list arg)
{
	char *ptr;
	int MinWidth, MaxWidth, fSign = 0;
	char fComma, fill;
	char *save_dest;
	char *ptr2, pbuffer[2048], ibuffer[32];
	int nDecimal, nSign;

	if (dest == NULL)
		dest = pbuffer;

	save_dest = dest;

	for ( ; *fmt; fmt++) {

		// is it an argument spec?
		if (*fmt == '%') {

			// set the default values
			MinWidth = 0;
			MaxWidth = INT_MAX;
			fComma = 0;
			fill = ' ';

			// get the minimum width
			fmt++;
			if (*fmt == '-') {
				fmt++;
				fSign++;
			} else if (*fmt == '+')
				fmt++;

			if (*fmt == '0')
				fill = '0'; 		// save fill character

			if (*fmt == '*') {
				// next argument is the value
				fmt++;
				MinWidth = va_arg(arg, int);
			} else {
				// convert the width from ASCII to binary
				for (MinWidth = atoi(fmt); ((*fmt >= '0') && (*fmt <= '9')); fmt++)
					;
			}

			if (fSign) {
				MinWidth = -MinWidth;
				fSign = 0;
			}

			// get maximum precision
			if (*fmt == '.') {

				fmt++;
				if (*fmt == '-') {
					fmt++;
					fSign++;
				} else if (*fmt == '+')
					fmt++;

				// save fill character
				if (*fmt == '0')
					fill = '0';

				if (*fmt == '*') {
					// next argument is the value
					fmt++;
					MaxWidth = va_arg(arg, int);
				} else {
					// convert the width from ASCII to binary
					for (MaxWidth = atoi(fmt); ((*fmt >= '0') && (*fmt <= '9')); fmt++)
						;
				}

				if (fSign) {
					MaxWidth = -MaxWidth;
					fSign = 0;
				}
			}
next_fmt:
			switch (*fmt) {
			case 'L':	// print commas in string
				fComma = 1;
				// fall thru 'cause it's a long int

			case 'l':       // long int
			case 'F':       // Far data
				fmt++;
				goto next_fmt;		// ignore in 32-bit!

			case 's':       // string
				WriteString( &dest, (const char *)va_arg( arg, const char * ), MinWidth, MaxWidth, fill );
				continue;

			case 'q':	// 64-bit (quad) decimal
				ptr = _fcvt( (double)va_arg( arg, double ), 0, &nDecimal, &nSign );
				ptr = strcpy( ibuffer, ptr );
				if ( nSign )
					strins( ibuffer, "-" );
				goto write_int;

			case 'u':       // unsigned decimal int
				ptr = _ultoa( va_arg( arg, unsigned int ), ibuffer, 10 );
				goto write_int;

			case 'x':	// hex integer
				ptr = strupr( _ultoa( va_arg( arg, unsigned int ), ibuffer, 16 ));
				goto write_int;

			case 'd':       // signed decimal int
				ptr = _ltoa( va_arg( arg, int ), ibuffer, 10 );
write_int:
				// format a long integer by inserting
				// commas (or other character specified
				// by country_info.szThousandsSeparator)
				if ( fComma ) {
					for (ptr2 = strend(ptr); ((ptr2 -= 3) > ptr); )
						strins( ptr2, gaCountryInfo.szThousandsSeparator );
				}

				WriteString( &dest, ptr, MinWidth, 32, fill );
				continue;

			case 'c':       // character
				*dest++ = (char)va_arg( arg, int );
				continue;

			case 'n':       // number of characters written so far
				WriteString(&dest,_ltoa((long)(dest - save_dest),ibuffer,10),MinWidth,32,fill);
				continue;
			}
		}

		*dest++ = *fmt;
	}

	*dest = '\0';

	// return string length
	return ((handle >= 0) ? _write( handle, save_dest, strlen(save_dest)) : (int)(dest - save_dest) );
}


// replace the RTL sscanf()
int sscanf(const char *source, const char *fmt, ...)
{
	va_list arglist;

	va_start(arglist,fmt);
	return (_fmtin((char *)source,fmt,arglist));
}


// replaces the RTL sprintf()
int sprintf(char *dest, const char *fmt, ...)
{
	va_list arglist;

	va_start(arglist,fmt);
	return (_fmtout(-1,dest,fmt,arglist));
}


// convert integer to ASCII
void IntToAscii(int nVal, char *pszBuf)
{
	sprintf( pszBuf, FMT_INT, nVal );
}


// fast printf() replacement function, with handle specification
int qprintf(int handle, const char *format, ...)
{
	va_list arglist;

	va_start( arglist, format );
	return ( _fmtout( handle, NULL, format, arglist ));
}


// fast printf() replacement function (always writes to STDOUT)
int printf(const char *format, ... )
{
	va_list arglist;

	va_start( arglist, format );
	return ( _fmtout( STDOUT, NULL, format, arglist ));
}


// colorized printf()
int color_printf( int attrib, const char *format, ...)
{
	int length;
	va_list arglist;
	char dest[1024];
	int row, column;

	va_start( arglist, format );

	if (attrib != -1) {

		length = _fmtout( -1, dest, format, arglist );

		// get current cursor position
		GetCurPos(&row,&column);

		// write the string & attribute
		WriteStrAtt(row,column,attrib,dest);

		SetCurPos(row,column+length);

	} else
		length =_fmtout(STDOUT,0L,format,arglist);

	return length;
}


// write a string to STDOUT
int qputs(const char *string)
{
	return (_write(STDOUT,string,strlen(string)));
}


// print a CR/LF pair to STDOUT
void crlf(void)
{
	_write( STDOUT, "\n", 1 );
}


// write a character (no buffering) to the specified handle
void qputc( int handle, char c )
{
	_write( handle, &c, 1 );
}

