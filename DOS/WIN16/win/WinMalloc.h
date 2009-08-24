/*	
	"@(#)WinMalloc.h	1.4
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

*/

#ifndef WinMalloc__h
#define WinMalloc__h


/*
 *	WinMallocInfo 
 *	tool to examine the WinMalloc history.
 *
 *	WinMallocInfo(opcode , string, type , handle )
 * 
 *	opcodes
 *		one of the operations available
 *		0	dump winmalloc info
 *		1	how much has been winmalloced so far -
 *			how much has been freed
 *	type
 *		one of the types of memory allocation
 *		0	unknown allocation type
 *		1	persistent (will not be freed)
 *		2	transient, (allocated/freed short term)
 *		3	library object, with handle defined
 *		4	globalalloc object,with handle
 *		5	internal use by library, not persistent	
 *		6	dll allocation
 *		7	driver allocation
 */ 

/* opcodes */
#define	WMI_DUMP	0
#define WMI_STAT	1
#define WMI_TAG		2
#define WMI_CHECK	3

#ifdef WINMALLOC_CHECK

#define WINMALLOCINFO(a,b,c,d)	WinMallocInfo(a,b,c,d)

#else

#define WINMALLOCINFO(a,b,c,d)	

#endif

long WinMallocInfo(int , char * ,int , int );


#endif /* WinMalloc__h */
