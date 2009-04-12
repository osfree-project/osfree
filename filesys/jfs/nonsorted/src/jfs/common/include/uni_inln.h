/* $Id: uni_inln.h,v 1.1 2000/04/21 10:58:00 ktk Exp $ */

/* static char *SCCSID = "@(#)1.2  12/8/98 13:56:54 src/jfs/common/include/uni_inln.h, sysjfs, w45.fs32, 990417.1";*/
/*
 * unistrk:  Unicode kernel case support
 *
 * Function:
 *     Convert a unicode character to upper or lower case using
 *     compressed tables.
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
  *
 * Notes:
 *     These APIs are based on the C library functions.  The semantics
 *     should match the C functions but with expanded size operands.
 *
 *     The upper/lower functions are based on a table created by mkupr.
 *     This is a compressed table of upper and lower case conversion.
 *
 */
#include "unidefk.h"
#define  UNIUPR_NOLOWER    /* Example to not expand lower case tables */
/* #include "uniupr.h" */

/* Just define what we want from uniupr.h.  We don't want to define the tables
 * in each source file.
 */
#ifndef	UNICASERANGE_DEFINED
typedef struct {
	UniChar		start;
	UniChar		end;
	signed char *	table;
} UNICASERANGE;
#endif	/* UNICASERANGE_DEFINED */

#ifndef UNIUPR_NOUPPER
extern signed char UniUpperTable[512];
extern UNICASERANGE UniUpperRange[];
#endif	/* UNIUPR_NOUPPER */

#ifndef UNIUPR_NOLOWER
extern signed char UniLowerTable[512];
extern UNICASERANGE UniLowerRange[];
#endif	/* UNIUPR_NOLOWER */

/*
 * UniStrcat:  Concatenate the second string to the first
 *
 * Returns:
 *     Address of the first string
 */
_Inline UniChar * UniStrcat  ( UniChar * ucs1, const UniChar * ucs2 ) {
    UniChar * anchor = ucs1;      /* save a pointer to start of ucs1 */

    while (*ucs1++);              /* To end of first string          */
    ucs1--;                       /* Return to the null              */
    while (*ucs1++ = *ucs2++);    /* copy string 2 over              */
    return anchor;
}


/*
 * UniStrchr:  Find a character in a string
 *
 * Returns:
 *     Address of first occurance of character in string
 *     or NULL if the character is not in the string
 */
_Inline UniChar * UniStrchr (const UniChar * ucs, UniChar uc) {
    while( (*ucs != uc) && *ucs )
        ucs++;

    if (*ucs == uc)
        return (UniChar *) ucs;
    return NULL;
}


/*
 * UniStrcmp:  Compare two strings
 *
 * Returns:
 *     < 0:  First string is less than second
 *     = 0:  Strings are equal
 *     > 0:  First string is greater than second
 */
_Inline int  UniStrcmp  ( const UniChar * ucs1, const UniChar * ucs2 ) {
    while ( (*ucs1 == *ucs2) && *ucs1 ) {
        ucs1++;
        ucs2++;
    }
    return  (int)*ucs1 - (int)*ucs2;
}


/*
 * UniStrcpy:  Copy a string
 */
_Inline UniChar * UniStrcpy  ( UniChar * ucs1, const UniChar * ucs2 ) {
    UniChar * anchor = ucs1;     /* save the start of result string */

    while(*ucs1++ = *ucs2++);
    return anchor;
}


/*
 * UniStrlen:  Return the length of a string
 */
_Inline size_t  UniStrlen  ( const UniChar * ucs1 ) {
    int  i = 0;

    while (*ucs1++)
        i++;
    return  i;
}


/*
 * UniStrncat:  Concatenate length limited string
 */
_Inline UniChar * UniStrncat ( UniChar * ucs1, const UniChar * ucs2, size_t n ) {
    UniChar * anchor = ucs1;         /* save pointer to string 1        */

    while(*ucs1++);
    ucs1--;                          /* point to null terminator of s1  */
    while( n-- && (*ucs1 = *ucs2)) { /* copy s2 after s1                */
        ucs1++;
        ucs2++;
    }
    *ucs1 = 0;                       /* Null terminate the result       */
    return( anchor );
}


/*
 * UniStrncmp:  Compare length limited string
 */
_Inline int  UniStrncmp ( const UniChar * ucs1, const UniChar * ucs2, size_t n) {
    if (!n)
        return 0;                  /* Null strings are equal */
    while ( (*ucs1 == *ucs2) && *ucs1 && --n ) {
        ucs1++;
        ucs2++;
    }
    return (int)*ucs1 - (int)*ucs2;
}


/*
 * UniStrncpy:  Copy length limited string with pad
 */
_Inline UniChar * UniStrncpy ( UniChar * ucs1, const UniChar * ucs2, size_t n ) {
    UniChar * anchor = ucs1;

    while ( n-- && *ucs2 )         /* Copy the strings */
        *ucs1++ = *ucs2++;

    n++;
    while ( n-- )                  /* Pad with nulls   */
       *ucs1++ = 0;
    return anchor;
}


/*
 * UniStrstr:  Find a string in a string
 *
 * Returns:
 *     Address of first match found
 *     NULL if no matching string is found
 */
_Inline UniChar * UniStrstr (const UniChar *ucs1, const UniChar *ucs2) {
    const UniChar *anchor1 = ucs1;
    const UniChar *anchor2 = ucs2;

    while (*ucs1) {
        if (*ucs1 == *ucs2) {          /* Partial match found         */
            ucs1++;
            ucs2++;
        } else {
            if (!*ucs2)                /* Match found                 */
                return (UniChar *)anchor1;
            ucs1 = ++anchor1;          /* No match                    */
            ucs2 = anchor2;
        }
    }

    if (!*ucs2)                        /* Both end together           */
       return (UniChar *)anchor1;      /* Match found                 */
    return NULL;                       /* No match                    */
}


#ifndef UNIUPR_NOUPPER
/*
 * UniToupper:  Convert a unicode character to upper case
 */
_Inline UniChar  UniToupper(register UniChar uc) {
    register UNICASERANGE * rp;

    if (uc < sizeof(UniUpperTable)) {  /* Latin characters      */
        return uc+UniUpperTable[uc];   /* Use base tables       */
    } else {
        rp = UniUpperRange;            /* Use range tables      */
        while (rp->start) {
            if (uc<rp->start)          /* Before start of range */
                return uc;             /* Uppercase = input     */
            if (uc<=rp->end)           /* In range              */
                return uc+rp->table[uc-rp->start];
            rp++;                      /* Try next range        */
        }
    }
    return uc;                         /* Past last range       */
}


/*
 * UniStrupr:  Upper case a unicode string
 */
_Inline UniChar * UniStrupr(register UniChar * upin) {
    register UniChar * up;

    up = upin;
    while (*up) {                      /* For all characters    */
        *up = UniToupper(*up);
        up++;
    }
    return upin;                       /* Return input pointer  */
}
#endif	/* UNIUPR_NOUPPER */


#ifndef UNIUPR_NOLOWER
/*
 * UniTolower:  Convert a unicode character to lower case
 */
_Inline UniChar  UniTolower(UniChar uc) {
    register UNICASERANGE * rp;

    if (uc < sizeof(UniLowerTable)) {  /* Latin characters      */
        return uc+UniLowerTable[uc];   /* Use base tables       */
    } else {
        rp = UniLowerRange;            /* Use range tables      */
        while (rp->start) {
            if (uc<rp->start)          /* Before start of range */
                return uc;             /* Uppercase = input     */
            if (uc<=rp->end)           /* In range              */
                return uc+rp->table[uc-rp->start];
            rp++;                      /* Try next range        */
        }
    }
    return uc;                         /* Past last range       */
}


/*
 * UniStrlwr:  Lower case a unicode string
 */
_Inline UniChar * UniStrlwr(register UniChar * upin) {
    register UniChar * up;

    up = upin;
    while (*up) {                      /* For all characters    */
        *up = UniTolower(*up);
        up++;
    }
    return upin;                       /* Return input pointer  */
}
#endif
