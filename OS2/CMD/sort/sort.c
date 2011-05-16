/*
*    SORT - reads line of a file and sorts them in order
*    Copyright  1995  Jim Lynch
*    Updated 2003 by Eric Auer: NLS for sort order, smaller binary.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kitten.h"
#include <fcntl.h> /* O_RDONLY */
#include <io.h>    /* write */
#define StdIN  0
#define StdOUT 1
#define StdERR 2

#ifndef __MSDOS__
#include <malloc.h>
#endif

#ifdef __OS2__
#define INCL_DOSERRORS
//#define INCL_VIO

#include <osfree.h>             // Include file for osfree
#include <cmd_shared.h>         // Include file for cmd tools
#endif

#ifdef __MSDOS__
#include <dos.h> /* intdosx */

/*
NLS is int 21.65 -> get tables... country=-1 is current... The data:
02 - upcase table: 128 bytes of "what is that 0x80..0xff char in uppercase?"
03 - downcase table: 256 bytes of "what is that 0x00..0xff char in lowercase?"
  (only DOS 6.2+ with country.sys loaded, probably only codepage 866)
04 - filename uppercase table
05 - filename terminator table
06 - collating: 256 "values used to sort char 0x00..0xff" (I think you can
  use those byte values to "XLAT" (ASM) / look up (C) a number for each
  char, and then use that number for sorting comparisons)
07 - DBCS is somewhat different, see RBIL (double byte chars, DOS 4.0+)

The returned buffer contents are NOT the tables but a byte followed by
a far POINTER to the actual tables!
All tables are prefixed by a word indicating the table size (128, 256...)!
Minimum DOS version for NLS stuff is 3.3 ... For table request, give BX
and DX as codepage / country, or -1 to use current. You must provide a
buffer at ES:DI and tell its length in CX. On return, CY / buffer / CX set.
Only if NLSFUNC loaded, other codepages / countries than current are avail.

From DOS 4.0 on, you also have:
Upcase char:   21.6520.dl -> dl
Upcase array:  21.6521.ds:dx.cx (string at ds:dx, length cx)
Upcase string: 21.6522.ds:dx (string terminated by 0 byte)
*/

unsigned char far * collate;
#endif



#ifndef MAXPATH
#define MAXPATH 80
#endif
#define MAXRECORDS  10000       /* maximum number of records that can be
                                 * sorted */
#define MAXLEN  1023            /* maximum record length */

int             rev;            /* reverse flag */
int             nls;            /* NLS use flag */
int             help;           /* help flag */
int             sortcol;        /* sort column */
int             err = 0;        /* error counter */

void
WriteString(char *s, int handle) /* much smaller than fputs */
{
  write(handle, s, strlen(s));
}

int
cmpr(void *a, void *b)
{
    unsigned char *A, *B, *C;

    A = *(unsigned char **) a;
    B = *(unsigned char **) b;

    if (sortcol > 0) { /* "sort from column... " */
        if (strlen(A) > sortcol)
            A += sortcol;
        else
            A = "";
        if (strlen(B) > sortcol)
            B += sortcol;
        else
            B = "";
    }

    if (rev) { /* reverse sort: swap strings */
        /* (or swap sign of result, of course) */
        C = A;
        A = B;
        B = C;
    }

#ifdef __MSDOS__
    if (nls) {
        while (collate[A[0]] == collate[B[0]]) {
            /* we use collate in the while as well: two different *
             * bytes may have the same collate position...        */
            if (A[0] == '\0') return 0; /* both at end */
            A++;
            B++;
        }
        if (collate[A[0]] < collate[B[0]]) return -1;
        return 1;
    } else {
#endif
        return strcmp(A, B);
#ifdef __MSDOS__
    }
#endif
}

void
usage(nl_catd cat)
{
    if (cat != cat) {}; /* avoid unused argument error in kitten */

    WriteString("SORT: V1.2\r\n", StdERR);

    if (err)
        WriteString(catgets(cat, 2, 0, "Invalid parameter\r\n"), StdERR);
    WriteString(catgets(cat, 0, 0, "    SORT [/R] [/+num] [/?]\r\n"), StdERR);
    WriteString(catgets(cat, 0, 1, "    /R    Reverse order\r\n"), StdERR);
#ifdef __MSDOS__
    WriteString(catgets(cat, 0, 2, "    /N    Enable NLS support\r\n"), StdERR);
#endif
    WriteString(catgets(cat, 0, 3,
        "    /+num start sorting with column num, 1 based\r\n"), StdERR);
    WriteString(catgets(cat, 0, 4, "    /?    help\r\n"), StdERR);

}

typedef int (*cmpr_t)(const void *, const void *);

int main(int argc, char **argv)
{
    char        filename[MAXPATH];
    char        temp[MAXLEN + 1];
    char        *list[MAXRECORDS];
    char        *cp;    /* option character pointer */
    int         nr;
    int         i;
    /* FILE     *fi; */ /* file descriptor */
    int         fi;     /* file HANDLE (fopen/... are big) */
    nl_catd     cat;    /* handle for localized messages (catalog) */

#ifdef __MSDOS__
    /* MAKE SURE THAT YOU USE BYTE ALIGNMENT HERE! */
    struct NLSBUF {
        char id;
        unsigned char far * content;
    } collbuf;
    union REGS     dosr;
    struct SREGS   doss;
#endif

    cat = catopen ("sort", 0);

    sortcol = 0;
    strcpy(filename, "");
    rev = 0;
    nls = 0;
    while (--argc) {
        if (*(cp = *++argv) == '/') {
            switch (cp[1]) {
            case 'R':
            case 'r':
                rev = 1;
                break;
#ifdef __MSDOS__
            case 'N':
            case 'n':
                if ( ((_osmajor >= 3) && (_osminor >= 3)) ||
                     (_osmajor > 3) ) {
                    dosr.x.ax = 0x6506; /* get collate table */
                    dosr.x.bx = 0xffff; /* default codepage  */
                    dosr.x.dx = 0xffff; /* default country   */
                    dosr.x.cx = 5;      /* buffer size   */
                    doss.es = FP_SEG(&collbuf);
                    dosr.x.di = FP_OFF(&collbuf);
                    intdosx(&dosr,&dosr,&doss);
                    if ((dosr.x.flags & 1) == 1) {
                        WriteString(catgets(cat, 2, 1,
/* catgets ... */           "Error reading NLS collate table\r\n"),
                            StdERR);
                        nls = 0;
                    } else {
                        /* ... CX is returned as table length ... */
                        collate = collbuf.content; /* table pointer */
                        collate++; /* skip leading word, which is */
                        collate++; /* not part of the table */
                        nls = 1;
                    }
                } else {
                    WriteString(catgets(cat, 2, 2,
/* catgets ... */       "Only DOS 3.3 or newer supports NLS!\r\n"),
                        StdERR);
                }
                break;
#endif
            case '?':
            case 'h':
            case 'H':
                help = 1;
                break;
            case '+':
                sortcol = atoi(cp + 1);
                if (sortcol)
                    sortcol--;
                break;
            default:
                err++;
            }
        } else {                /* must be a file name */
            strcpy(filename, *argv);
        }
    }
    if (err || help) {
        usage(cat);
        catclose(cat);
        exit(1);
    }
    fi = StdIN;                 /* just in case */
    if (strlen(filename)) {
        if ((fi = open(filename, O_RDONLY)) == -1) {
            /* was: ... fopen(...,"r") ... == NULL ... */
            WriteString(catgets(cat, 2, 3, "SORT: Can't open "), StdERR);
            WriteString(filename,StdERR);
            WriteString(catgets(cat, 2, 4, " for read\r\n"), StdERR);
            /* avoided 1.5k-2k overhead of *printf()... */
            catclose(cat);
            exit(2);
        }
    }

    (void)get_line(fi, NULL, 0); /* initialize buffers */

    for (nr = 0; nr < MAXRECORDS; nr++) {
        if (!get_line(fi, temp, MAXLEN))
            /* was: fgets(temp, MAXLEN, fi) == NULL */
            break;
        list[nr] = (char *) malloc(strlen(temp) + 1);
            /* malloc might have big overhead, but we cannot avoid it */
        if (list[nr] == NULL) {
            WriteString(catgets(cat, 2, 5, "SORT: Insufficient memory\r\n"),
                StdERR);
            catclose(cat);
            exit(3);
        }
        strcpy(list[nr], temp);
    }
    if (nr == MAXRECORDS) {
        WriteString(catgets(cat, 2, 6,
            "SORT: number of records exceeds maximum\r\n"), /* catgets ... */
            StdERR);
        catclose(cat);
        exit(4);
    }
    qsort((void *) list, nr, sizeof(char *), (cmpr_t)cmpr);
    for (i = 0; i < nr; i++) {
        WriteString(list[i], StdOUT);
        WriteString("\r\n",StdOUT);
    }
    catclose(cat);
    return 0;
}

