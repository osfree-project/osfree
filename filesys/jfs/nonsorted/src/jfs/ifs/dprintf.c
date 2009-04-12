/* $Id: dprintf.c,v 1.1 2000/04/21 10:58:02 ktk Exp $ */

static char *SCCSID = "@(#)1.7  9/13/99 14:46:43 src/jfs/ifs/dprintf.c, sysjfs, w45.fs32, fixbld";
/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
/***	dprintf.c - COM2 output
 *
 */

/*
 * Change History :
 *
 */

#ifdef _JFS_DEBUG

#define INCL_NOPMAPI
#include <os2.h>
#include <bsekee.h>						// D230860

SpinLock_t	print_lock;					// D230860
int		first_time = 1;					// D230860

void	PUTC(int);

void putl (ULONG ul, ULONG base)
{
    if (ul / base != 0)
	putl (ul / base, base);
    PUTC ("0123456789ABCDEF"[ul % base]);
}

void putll (unsigned long long ul, ULONG base)
{
    if (ul / base != 0)
	putll (ul / base, base);
    PUTC ("0123456789ABCDEF"[ul % base]);
}

/*  format - replace the C runtime formatting routines.
 *
 *  format is a near-replacement for the *printf routines in the C runtime.
 *
 *  fmt 	formatting string.  Formats currently understood are:
 *		    %c single character
 *		    %[l[l]]d %[l[l]]x
 *		    %[F]s
 *		    %p
 *		      * may be used to copy in values for m and n from arg
 *			list.
 *		    %%
 *  arg 	is a list of arguments
 */
void format (char *fmt, PVOID pv)
{
    char c;
    PSZ psz;
    ULONG base;

    while (c = *fmt++) {
	if (c == '\n') {
	    PUTC ('\r');
	    PUTC ('\n');
	}
	else
	if (c != '%')
	    PUTC (c);
	else {
	    BOOL fLongLong = FALSE;
	    BOOL fFar = FALSE;

	    base = 10;
	    while ((*fmt >= '0') && (*fmt <= '9')) /* Ignore length for now */
		fmt++;
	    if (*fmt == 'l') {
		fmt++;
		if (*fmt == 'l') {
		    fLongLong = TRUE;
		    fmt++;
		}
	    }
	    if (*fmt == 'F') {
		fFar = TRUE;
		fmt++;
	    }

	    switch (*fmt++) {
	    case 'c':
		PUTC ((char) * MAKETYPE (pv, PULONG));
		++ MAKETYPE (pv, PULONG);
		break;
	    case 'x':
		base = 16;
	    case 'd':
		if (fLongLong) {
		    putll (* MAKETYPE (pv, unsigned long long *), base);
		    ++ MAKETYPE (pv, unsigned long long *);
		} else {
		    putl (* MAKETYPE (pv, PULONG), base);
		    ++ MAKETYPE (pv, PULONG);
		}
		break;
	    case 's':
		if (fFar) {
		    psz = * MAKETYPE (pv, PSZ *);
		    ++ MAKETYPE (pv, PSZ *);
		} else {
		    psz = * MAKETYPE (pv, NPSZ *);
		    ++ MAKETYPE (pv, NPSZ *);
		}
		while (*psz != 0)
		    PUTC (*psz++);
		break;
	    case '%':
		PUTC ('%');
		break;
	    case 'p':
		psz = * MAKETYPE (pv, PSZ *);
		++ MAKETYPE (pv, PSZ *);
		putl ((LONG)SELECTOROF (psz), 16);
		PUTC (':');
		putl ((LONG)OFFSETOF (psz), 16);
		break;
	    default:
		PUTC ('?');PUTC ('\'');PUTC (fmt[-1]);PUTC('\'');
	    }
	}
    }
}

void printf (char * fmt, ...)
{
// BEGIN D230860
    if (first_time)
    {
	first_time = 0;
	KernAllocSpinLock(&print_lock);
    }
    KernAcquireSpinLock(&print_lock);
// END D230860
    format (fmt, (PVOID) (&fmt+1));
    KernReleaseSpinLock(&print_lock);				// D230860
}
#else /* _JFS_DEBUG */
void printf (char * fmt, ...)
{
}
#endif /* _JFS_DEBUG */
