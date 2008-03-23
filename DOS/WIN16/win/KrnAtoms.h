/********************************************************************

	@(#)KrnAtoms.h	1.3 Atom structure definitions.
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
 
#ifndef Atoms__h
#define Atoms__h


typedef unsigned long ATOMID;

typedef struct {
	ATOMID	q;		/* what a string 'hashes' to 	*/
	long   	idx;		/* index into data table	*/
	long	refcnt;		/* how many clients have it 	*/
	long	idsize;		/* space used by this slot	*/
} ATOMENTRY;
typedef ATOMENTRY *LPATOMENTRY;

typedef struct {
	ATOMENTRY 	*AtomTable;	/* pointer to table data 	*/
	char		*AtomData;	/* pointer to name data 	*/
	unsigned long	 TableSize;	/* number items in this table   */
	unsigned long	 DataSize;	/* space used by string data    */
	LPVOID		lpDrvData;
} ATOMTABLE;
typedef ATOMTABLE *LPATOMTABLE;

/* Prototypes for the additional atom APIs */
ATOM AddAtomEx(ATOMTABLE *,LPCSTR);
ATOM DeleteAtomEx(ATOMTABLE *,ATOM);
ATOM FindAtomEx(ATOMTABLE *,LPCSTR);
UINT GetAtomNameEx(ATOMTABLE *,ATOM,LPSTR,int);
ATOM GlobalAddAtomEx(ATOMTABLE *,LPCSTR);
ATOM GlobalDeleteAtomEx(ATOMTABLE *,ATOM);
ATOM GlobalFindAtomEx(ATOMTABLE *,LPCSTR);
UINT GlobalGetAtomNameEx(ATOMTABLE *,ATOM,LPSTR,int);
UINT LockAtom(ATOM);
UINT LockAtomEx(ATOMTABLE *,ATOM);
UINT GlobalLockAtom(ATOM);
UINT GlobalLockAtomEx(ATOMTABLE *,ATOM);

#endif /* Atoms__h */
