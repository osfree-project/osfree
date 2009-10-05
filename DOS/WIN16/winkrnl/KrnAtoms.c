/*    
	KrnAtoms.c	1.10
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
#ifdef LATER
	MAKEINTATOM types are unchecked, they may fail...
	if so it would be in a string function which expects
	a pointer to a string...

	they are stored with a 'q' value == to the int value
	but take NO storage in the data segment

	the refcnt is set to 1 and never changes...
	they cannot be deleted, as per windows
#endif

/*************************************************************

	Implementation notes;

	for both the global and local functions, a set
	of corresponding Ex functions are used.  These Ex
	functions allow us to define ATOMTABLE's over and
	above the standard application local and global 
	table.  Windows does have an additional table that
	is used internally.  For us we could add both a
	global USER atom table, and a private USER atom
	table.  This would allow us to separate application
	atoms for internally used atoms.  Private USER 
	atoms could be things like class names, that may
	not have to be unique across processes, while others
	could be unique across processes, yet still not known
	to the application directly.

	The global tables are initialized by calling the
	IPCH_INITATOMTABLE.

	Additional private atom tables can be had by including
	Atoms.h and calling the Ex functions with a pointer
	to a private AtomTable.

**************************************************************/	


#include "windows.h"

#include "Log.h"
#include "KrnAtoms.h"
#include "Driver.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* system global and local atom tables */
static ATOMTABLE LocalTable;
static ATOMTABLE GlobalTable;

/* internal functions */
static ATOMENTRY *GetAtomPointer(ATOMTABLE *,int);
static ATOMID    AtomHashString(LPCSTR,LPINT);

#define ATOMBASE	 0xcc00

/********************************************************/
/* convert alphanumeric string to a 'q' value. 		*/
/* 'q' values do not need to be unique, they just limit */
/* the search we need to make to find a string		*/
/********************************************************/

static ATOMID
AtomHashString(LPCSTR lp,int *lplen)
{
	ATOMID 	q;
	char   *p,ch;
	int	len;

	/* if we have an intatom... */
	if(HIWORD(lp) == 0) {
		if(lplen) *lplen = 0;
		return (ATOMID)lp;
	}

	/* convert the string to an internal representation */
	for(p=(LPSTR)lp,q=0,len=0;(ch=*p++);len++)
		q = (q<<1) + islower(ch)?toupper(ch):ch;

	/* 0 is reserved for empty slots */
	if(q == 0)
		q++;

	/* avoid strlen later */
	if(lplen) {
		*lplen = ++len;
	}
	return q;
}

/********************************************************/
/*	convert an atom index into a pointer into an 	*/
/* 	atom table.  This validates the pointer is in   */
/*	range, and that the data is accessible		*/
/********************************************************/

static ATOMENTRY *
GetAtomPointer(ATOMTABLE *at,int index)
{
	ATOMENTRY *lp;
	
	/* if no table, then no pointers */
	if(at->AtomTable == 0)
		return 0;

	/* bad index */
	if((index < 0) || (index >= at->TableSize))
		return 0;

	/* we have a pointer */
	lp = &at->AtomTable[index];

	/* is the index past stored data, validity check		*/
	/* LATER: is the size of the entry within the available space 	*/
	if(lp->idx > at->DataSize)
		return 0;

	return lp;
}

/********************************************************/
/*							*/
/* Extended versions of atom functions 			*/
/*	same as both local/global functions, except	*/
/* 	these take a pointer to an atomtable		*/
/*							*/
/********************************************************/

ATOM
FindAtomEx(ATOMTABLE *at,LPCSTR lpstr)
{
	ATOMID		q;
	LPATOMENTRY   	lp;
	int		index;
	int		atomlen;

	/* convert string to 'q', and get length */
	q = AtomHashString(lpstr,&atomlen);

	/* find the q value, note: this could be INTATOM */
	/* if q matches, then do case insensitive compare*/
	for(index = 0;(lp = GetAtomPointer(at,index));index++) {
		if(lp->q == q) {	
			if(HIWORD(lpstr) == 0)
				return ATOMBASE + index;
			if(strcasecmp(&at->AtomData[lp->idx],lpstr) == 0)
				return ATOMBASE + index;
		}
	}
	return 0;
}


ATOM
AddAtomEx(ATOMTABLE *at,LPCSTR lpstr)
{
	ATOM atom;
	ATOMID		q;
	LPATOMENTRY   	lp,lpfree;
	int		index,freeindex;
	int		atomlen;
	int		newlen;
	
	/* if we already have it, bump refcnt */
	if((atom = FindAtomEx(at,lpstr))) {
		lp = GetAtomPointer(at,atom - ATOMBASE);
		if(lp->idsize) lp->refcnt++;
		return atom;
	}

	/* add to a free slot */
	q = AtomHashString(lpstr,&atomlen);

	lpfree 	  = 0;
	freeindex = 0;

	for(index = 0;(lp = GetAtomPointer(at,index));index++) {
		if(lp->q == 0 && lp->refcnt == 0) {	
			if(lp->idsize > atomlen) {
				if ((lpfree == 0) ||
					    (lpfree->idsize > lp->idsize)) {
					lpfree = lp;
					freeindex = index;
				}
			}
		}
	}
	/* intatoms do not take space in data, but do get new entries */
	/* an INTATOM will have length of 0 			      */
	if(lpfree && atomlen) {
		lpfree->q = q;
		lpfree->refcnt = 1;			
		strncpy(&at->AtomData[lpfree->idx],lpstr,atomlen);
		return freeindex + ATOMBASE;
	}

	/* no space was available, or we have an INTATOM		*/
	/* so expand or create the table 				*/
	if(at->AtomTable == 0) {
		at->AtomTable = (ATOMENTRY *) WinMalloc(sizeof(ATOMENTRY));
		at->TableSize = 1;
		lp = at->AtomTable;
		index = 0;
	} else {
		at->TableSize++;
		at->AtomTable = (ATOMENTRY *) WinRealloc(
			(char *) at->AtomTable,
			at->TableSize * sizeof(ATOMENTRY));
		lp = &at->AtomTable[at->TableSize - 1];
	}

	/* set in the entry */
	lp->refcnt = 1;
	lp->q      = q;
	lp->idsize = atomlen;
	lp->idx    = 0;

	/* add an entry if not intatom... */
	if(atomlen) {
		newlen = at->DataSize + atomlen;

		if(at->AtomData == 0) {
			at->AtomData = (char *) WinMalloc(newlen);
			lp->idx = 0;
		} else {
			at->AtomData = (char *) WinRealloc(at->AtomData,newlen);
			lp->idx = at->DataSize;
		}

		strcpy(&at->AtomData[lp->idx],lpstr);
		at->DataSize = newlen;
	}	

	return index + ATOMBASE;
}

ATOM
DeleteAtomEx(ATOMTABLE *at,ATOM atom)
{
	ATOMENTRY *lp;
	
	/* a free slot has q == 0 && refcnt == 0 */
	if((lp = GetAtomPointer(at,atom - ATOMBASE))) {
		if(lp->idsize)
			lp->refcnt--;

		if(lp->refcnt == 0) {
			return lp->q = 0;
		}
	}
	return atom;
}

UINT
GetAtomNameEx(ATOMTABLE *at,ATOM atom,LPSTR lpstr,int len)
{
	ATOMENTRY *lp;
	char 	  *atomstr;
	int	   atomlen;
	
	/* return the atom name, or create the INTATOM */
	if((lp = GetAtomPointer(at,atom - ATOMBASE))) {
		if(lp->idsize) {
			atomlen = strlen(atomstr = &at->AtomData[lp->idx]);
			if (atomlen < len)
			    strcpy(lpstr,atomstr);
			else {
			    strncpy(lpstr,atomstr,len-1);
			    lpstr[len-1] = '\0';
			}
			return (UINT)strlen(lpstr);
		} else {
			wsprintf(lpstr,"#%d",lp->q);
			return (UINT)strlen(lpstr);
		}
	}
	return 0;
}

UINT
LockAtomEx(ATOMTABLE *at, ATOM atom)
{
	ATOMENTRY *lp;

	if((lp = GetAtomPointer(at,atom - ATOMBASE)))
		return (UINT)++lp->refcnt;
	return (UINT)-1;
}

/****************************************************************/
/*	local atom functions 					*/
/****************************************************************/

ATOM WINAPI
AddAtom(LPCSTR lpstr)
{
	return AddAtomEx(&LocalTable,lpstr);
}

ATOM WINAPI
FindAtom(LPCSTR lpstr)
{
	return FindAtomEx(&LocalTable,lpstr);
}

ATOM WINAPI
DeleteAtom(ATOM atom)
{
	return DeleteAtomEx(&LocalTable,atom);
}

UINT WINAPI
GetAtomName(ATOM atom,LPSTR lpstr,int len)
{
	return GetAtomNameEx(&LocalTable,atom,lpstr,len);
}

BOOL WINAPI
InitAtomTable(int nSize)
{
	return TRUE;
}

UINT
LockAtom(ATOM atom)
{
	return LockAtomEx(&LocalTable,atom);
}

/****************************************************************/
/*	extended global atom functions				*/
/*								*/
/*	these allow multiple global atom tables			*/
/* 	the default is to use the application wide global table	*/
/*	to use alternate global tables call IPCH_INITATOMTABLE  */
/*								*/
/****************************************************************/

ATOM
GlobalAddAtomEx(ATOMTABLE *at,LPCSTR lpstr)
{
	UINT	rc;

	APISTR((LF_API,"GlobalAddAtomEx: string %s table %x\n",
		lpstr,at));

	DRVCALL_IPC(IPCH_GLOBALATOMDATA,GAD_READWRITE,0,at);
	rc = AddAtomEx(at,lpstr);	
	DRVCALL_IPC(IPCH_GLOBALATOMDATA,GAD_UPDATE,0,at);
	return rc;
}

ATOM
GlobalFindAtomEx(ATOMTABLE *at,LPCSTR lpstr)
{
	DRVCALL_IPC(IPCH_GLOBALATOMDATA,GAD_READONLY,0,at);
	return  FindAtomEx(at,lpstr);
}

ATOM
GlobalDeleteAtomEx(ATOMTABLE *at,ATOM atom)
{
	UINT	rc;

	APISTR((LF_API,"GlobalDeleteAtomEx: atom %x table %x\n",
		atom,at));

	DRVCALL_IPC(IPCH_GLOBALATOMDATA,GAD_READWRITE,0,at);
	rc = DeleteAtomEx(at,atom);
	DRVCALL_IPC(IPCH_GLOBALATOMDATA,GAD_UPDATE,0,at);
	return rc;
}

UINT
GlobalGetAtomNameEx(ATOMTABLE *at,ATOM atom,LPSTR lpszbuf,int len)
{
	DRVCALL_IPC(IPCH_GLOBALATOMDATA,GAD_READONLY,0,at);
	return GetAtomNameEx(at,atom,lpszbuf,len);
}

UINT
GlobalLockAtomEx(ATOMTABLE *at, ATOM atom)
{
	return LockAtomEx(at,atom);
}

/****************************************************************/
/*	global atom functions 					*/
/****************************************************************/

ATOM WINAPI
GlobalAddAtom(LPCSTR lpstr)
{
	return GlobalAddAtomEx(&GlobalTable,lpstr);
}

ATOM WINAPI
GlobalFindAtom(LPCSTR lpstr)
{
	return GlobalFindAtomEx(&GlobalTable,lpstr);
}

ATOM WINAPI
GlobalDeleteAtom(ATOM atom)
{
	return GlobalDeleteAtomEx(&GlobalTable,atom);
}

UINT WINAPI
GlobalGetAtomName(ATOM atom,LPSTR lpszbuf,int len)
{
	return GlobalGetAtomNameEx(&GlobalTable,atom,lpszbuf,len);
}

UINT
GlobalLockAtom(ATOM atom)
{
	return GlobalLockAtomEx(&GlobalTable,atom);
}
