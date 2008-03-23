/********************************************************************
	@(#)ModTable.h	1.7
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

********************************************************************/
 
#ifndef ModTable__h
#define ModTable__h

typedef struct keyMODULEDSCR
{   char *name;                 		/* module name 		*/
    struct keyENTRYTAB *entry_table;		/* entry data for module */
    struct keySEGTAB *seg_table;		/* segment data for module */
    long *resource;				/* resource table	*/
} MODULEDSCR;

typedef MODULEDSCR *LPMODULEDSCR;

typedef struct keyMODULETAB
{   unsigned int flags;		/* flags for module */
    MODULEDSCR *dscr;		/* pointer to descriptor for module */
                                /* NULL => end of table */
} MODULETAB;

typedef MODULETAB *LPMODULETAB;

/* masks for flags field in struct hsmt_module_tab: */

#define MODULE_EXTERN      0x01    /* only module name, no descriptor */
#define MODULE_NO_LOAD     0x02    /* if module is dyn-dyn linked */
#define MODULE_SYSTEM      0x04    /* if module is system DLL	*/

typedef struct keyENTRYTAB
{   char *name;        /* entry point name */
                        /* will be NULL to indicate end of entry table */
                        /* will point to "" if only ordinal number known */
    unsigned short sel; /* selector number for entry.  will be 0 if no */
                        /* binary-world address exists for entry */
    unsigned short off; /* offset of entry point in selected segment */
    long int (*fun)();  /* pointer to C-callable native-world entry point */
                        /* will be NULL if there is none. May be pointer */
                        /* to native unit or interface to binary unit */
} ENTRYTAB;

typedef struct keySEGTAB
{	char *image;			/* pointer to image and image header */
	unsigned long size;		/* number bytes in segment image (0 => 64k) */
	unsigned short transfer;	/* transfer type of the segment	*/
					/* NULL indicates the end of the segment table */
	unsigned long alloc;	/* number of bytes to allocate for seg (0 => 64k) */
	unsigned short flags;	/* flags for the segment */
	unsigned short reserved;
} SEGTAB;

#endif	/* ModTable__h */
