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

/*
* Change History:
*
*/
/*
* Description: Defines types and macros which are used throughout the CHKDSK
*              source code.
*
*/

#ifndef LVM_GLBS_H_INCLUDED

#define LVM_GLBS_H_INCLUDED 1

/* An INTEGER number is a whole number, either + or -.
The number appended to the INTEGER key word indicates the number of bits
used to represent an INTEGER of that type.                               */
typedef short int INTEGER16;
typedef long  int INTEGER32;
typedef int       INTEGER;    /* Use compiler default. */

/* A CARDINAL number is a positive integer >= 0.
The number appended to the CARDINAL key word indicates the number of bits
used to represent a CARDINAL of that type.                               */

typedef unsigned short int CARDINAL16;
typedef unsigned long      CARDINAL32;
typedef unsigned int       CARDINAL;     /* Use compiler default. */

#ifdef NEED_BYTE_DEFINED

/* A BYTE is 8 bits of memory with no interpretation attached. */
typedef unsigned char BYTE;

#else

#ifndef BYTE

#define BYTE unsigned char

#endif

#endif

/* A REAL number is a floating point number. */
typedef float   REAL32;
typedef double  REAL64;

/* A BOOLEAN variable is one which is either TRUE or FALSE. */
typedef unsigned char  BOOLEAN;

#ifndef TRUE

#define TRUE  1
#define FALSE 0

#endif

/* An ADDRESS variable is one which holds an address.  The address can contain
anything, or even be invalid.  It is just an address which is presumed to
hold some kind of data. */

#ifdef ADDRESS

#undef ADDRESS

#endif

typedef void * ADDRESS;

typedef char * pSTRING;

#endif

