/* $Id: jfs_types.h,v 1.1 2000/04/21 10:57:59 ktk Exp $ */

/* static char *SCCSID = "@(#)1.17.1.2  12/2/99 11:13:20 src/jfs/common/include/jfs_types.h, sysjfs, w45.fs32, fixbld";
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
 */

/*
 * Change History :
 *
 */

#ifndef _H_JFS_TYPES
#define	_H_JFS_TYPES

/*
 *	jfs_types.h: 
 *
 * basic type/utility  definitions
 *
 * note: this header file must be the 1st include file 
 * of JFS include list in all JFS .c file.
 */

#ifdef	_JFS_OS2
#include <string.h>	// define size_t.  bsekee.h defines it wrong.  D233382
#include <bsekee.h>

typedef char *		caddr_t;
typedef char *		va_list;
/* Default behavior for OS/2 is UNICODE */
#define _JFS_UNICODE
#endif	/* _JFS_OS2 */

#ifndef _ULS_UNICHAR_DEFINED
	typedef unsigned short UniChar;
	#define _ULS_UNICHAR_DEFINED
#endif

/*
 *	primitive types
 */
#ifndef _UINT_TYPES
	/*	unicode includes also define these */
	typedef	unsigned short		uint16;
	typedef	unsigned int		uint32;
	#define _UINT_TYPES
#endif

typedef	signed char		int8;
typedef	unsigned char		uint8;
typedef	short			int16;
typedef	int			int32;
typedef	long long		int64;
typedef	unsigned long long	uint64;

#ifdef	_JFS_OS2
typedef	struct {
	unsigned	flag	:24;
	unsigned	result	:8;
} xevent_t;
#endif	/* _JFS_OS2 */

/*
 *	handy
 */
#undef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#undef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#undef ROUNDUP
#define	ROUNDUP(x, y)	( ((x) + ((y) - 1)) & ~((y) - 1) )

#define LEFTMOSTONE	0x80000000
#define	HIGHORDER	0x80000000u	/* high order bit on		*/
#define	ONES		0xffffffffu	/* all bit on			*/
#define	BITSPERINT32	32		/* number of bits in uint32	*/
#define	BITSPERINT64	64		/* number of bits in uint64	*/



/*
 *	simple extent descriptor (xd)
 */
typedef struct {
	unsigned	flag:8;		/* 1: flags */
	unsigned	rsrvd:8;	/* 1: */
	unsigned	len:8;		/* 1: length in unit of fsblksize */
	unsigned	addr1:8;	/* 1: address in unit of fsblksize */
	uint32		addr2;		/* 4: address in unit of fsblksize */
} xd_t;					/* - 8 - */

/* xd_t field construction */
#define	XDlength(xd, length32)	( (xd)->len = length32 )
#define	XDaddress(xd, address64)\
{\
	(xd)->addr1 = ((uint64)address64) >> 32;\
	(xd)->addr2 = (address64) & 0xffffffff;\
}

/* xd_t field extraction */
#define	lengthXD(xd)	( (xd)->len )
#define	addressXD(xd)\
	( ((int64)((xd)->addr1)) << 32 | (xd)->addr2 )

/*
 *	logical xd (lxd)
 */
typedef struct {
	unsigned	len:24;
	unsigned	off1:8;
	uint32		off2;
} lxd_t;

/* lxd_t field construction */
#define	LXDlength(lxd, length32)	( (lxd)->len = length32 )
#define	LXDoffset(lxd, offset64)\
{\
	(lxd)->off1 = ((uint64)offset64) >> 32;\
	(lxd)->off2 = (offset64) & 0xffffffff;\
}

/* lxd_t field extraction */
#define	lengthLXD(lxd)	( (lxd)->len )
#define	offsetLXD(lxd)\
	( ((int64)((lxd)->off1)) << 32 | (lxd)->off2 )

/* lxd list */
typedef struct {
	int16	maxnlxd;
	int16	nlxd;
	lxd_t	*lxd;
} lxdlist_t;

/*
 *	physical xd (pxd)
 */
typedef struct {
	unsigned	len:24;
	unsigned	addr1:8;
	uint32		addr2;
} pxd_t;

/* xd_t field construction */
#define	PXDlength(pxd, length32)	( (pxd)->len = length32 )
#define	PXDaddress(pxd, address64)\
{\
	(pxd)->addr1 = ((uint64)address64) >> 32;\
	(pxd)->addr2 = (address64) & 0xffffffff;\
}

/* xd_t field extraction */
#define	lengthPXD(pxd)	( (pxd)->len )
#define	addressPXD(pxd)\
	( ((int64)((pxd)->addr1)) << 32 | (pxd)->addr2 )

/* pxd list */
typedef struct {
	int16	maxnpxd;
	int16	npxd;
	pxd_t	pxd[8];
} pxdlist_t;


/*
 *	data extent descriptor (dxd)
 */
typedef struct {
	unsigned	flag:8;		/* 1: flags */
	unsigned	rsrvd:24;	/* 3: */
	uint32		size;		/* 4: size in byte */
	unsigned	len:24;		/* 3: length in unit of fsblksize */
	unsigned	addr1:8;	/* 1: address in unit of fsblksize */
	uint32		addr2;		/* 4: address in unit of fsblksize */
} dxd_t;				/* - 16 - */

/* dxd_t flags */
#define	DXD_INDEX	0x80	/* B+-tree index */
#define	DXD_INLINE	0x40	/* in-line data extent */
#define	DXD_EXTENT	0x20	/* out-of-line single extent */	
#define	DXD_FILE	0x10	/* out-of-line file (inode) */	
#define DXD_CORRUPT	0x08	/* Inconsistency detected */

/* dxd_t field construction */
#define	DXDlength(dxd, length32)	( (dxd)->len = length32 )
#define	DXDaddress(dxd, address64)\
{\
	(dxd)->addr1 = ((uint64)address64) >> 32;\
	(dxd)->addr2 = (address64) & 0xffffffff;\
}

/* dxd_t field extraction */
#define	lengthDXD(dxd)	( (dxd)->len )
#define	addressDXD(dxd)\
	( ((int64)((dxd)->addr1)) << 32 | (dxd)->addr2 )


/*
 *	DFS primitive types
 */
/* DCE globally unique name */
typedef struct {
	uint32 uuid[4];
} uuid_t;

typedef uuid_t	dfsUUID;

/* a 64-bit identifier */
typedef struct {
	uint32 id[2];
} dfsHyper;

#ifdef	_JFS_OS2
/*	This should be in jfs_lock.h, but it is used in jfs_aixisms.h */
typedef struct {
	SpinLock_t	SpinLock;
	int32		SleepLock;
} TSPINLOCK_T;
#endif	/* _JFS_OS2 */

/*
 * pool allocation information/status 
 */
typedef struct {
	char	a_handle[8];		/* 8: object id */
	int32	a_objsize;		/* 4: object size in bytes */
	caddr_t	a_table;		/* 4: object table */
	caddr_t	a_end;			/* 4: addr of (end of table + 1) */
	caddr_t	a_hwm;			/* 4: current high water mark */
	caddr_t	a_freelist;		/* 4: Head of the free list */
	int32	a_freeoff;		/* 4: offset of freelist link */
#ifdef _JFS_OS2
	uint32	a_objcnt;	/* 4: number of objects per allocation */
	uint32	a_objmax;	/* 4: maximum number of objects */
	uint32	a_objcur;	/* 4: current number of objects */
	uint32	a_flags;	/* 4: behavior flags */
	SpinLock_t	a_lock;	/* 4: lock taken if POOL_TAKE_LOCK set */
	struct {
		unsigned	wanted:31;
		unsigned	locked:1;
	}	a_sleeplock;	/* 4: sleeplock taken when xmalloc is called */
#endif
} pool_t;

/*
 *      directory entry argument
 */
typedef struct component_name {
        int32		namlen;
        UniChar         *name;
} component_t;


// BEGIN F226941

/*
 *	DASD limit information - stored in directory inode
 */
typedef	struct	dasd {
	uint8	thresh;		/* Alert Threshold (in percent)	*/
	uint8	delta;		/* Alert Threshold delta (in percent)	*/
	uint8	rsrvd1;
	uint8	limit_hi;	/* DASD limit (in logical blocks)	*/
	uint32	limit_lo;	/* DASD limit (in logical blocks)	*/
	uint8	rsrvd2[3];
	uint8	used_hi;	/* DASD usage (in logical blocks)	*/
	uint32	used_lo;	/* DASD usage (in logical blocks)	*/
}	dasd_t;

#define DASDLIMIT(dasdp) \
	(((uint64)((dasdp)->limit_hi) << 32) + (dasdp)->limit_lo)
#define setDASDLIMIT(dasdp, limit)\
{\
	(dasdp)->limit_hi = ((uint64)limit) >> 32;\
	(dasdp)->limit_lo = (uint32)(limit);\
}
#define DASDUSED(dasdp) \
	(((uint64)((dasdp)->used_hi) << 32) + (dasdp)->used_lo)
#define setDASDUSED(dasdp, used)\
{\
	(dasdp)->used_hi = ((uint64)used) >> 32;\
	(dasdp)->used_lo = (uint32)(used);\
}

// END F226941
	
/*
 *		circular doubly-linked list (cdll)
 *
 * A circular doubly-linked list (cdll) is anchored by a pair of pointers,
 * one to the head of the list and the other to the tail of the list. 
 * The elements are doubly linked so that an arbitrary element can be 
 * removed without a need to traverse the list. 
 * New elements can be added to the list before or after an existing element, 
 * at the head of the list, or at the tail of the list.
 * A circle queue may be traversed in either direction.
 *
 * +----------+        +-------------------------------------+
 * |          |        |                                     |
 * +->+-----+ |        +->+-----+  +->+-----+    +->+-----+  |
 * |  |  h  +-+        |  |  h  +--+  |  n  +----+  |  n  +--+
 * |  +-----+          |  +-----+  |  +-----+    |  +-----+
 * |  |  t  +-+     +-----+  t  |  |  |  p  +--+ |  |  p  +--+
 * |  +-----+ |     |  |  +-----+  |  +-----+  | |  +-----+  |
 * +----------+     |  +-----------------------+ |           |
 *                  |              |             |           |
 *                  |              +-------------------------+
 *                  |                            |
 *                  +----------------------------+
 */
/*
 *	define header
 *
 * list header field definition in header element:
 *
 * type - type of list element struct embedding the link field
 */
#define CDLL_HEADER(type)\
struct {\
	struct type *head;\
	struct type *tail;\
}

struct cdll_header {
	struct cdll_header *head;
	struct cdll_header *tail;
};

/*
 *	define link
 *
 * list link field definition in list element: 
 *
 * type - type of parent list element struct embedding the link field
 */
#define CDLL_ENTRY(type)\
struct {\
	struct type *next;\
	struct type *prev;\
}

struct cdll_entry {
	struct cdll_entry *next;
	struct cdll_entry *prev;
};

/*
 *	initialize header
 *
 * header - ptr to the header field in the header element
 */
#define	CDLL_INIT(header) {\
	(header)->head = (void *)(header);\
	(header)->tail = (void *)(header);\
}

/*
 *	scan list
 *
 * header - ptr to the header field in the header element
 * elm - ptr to the element to be inserted
 * field - name of the link field in the list element
 *
 * struct header_container	*container;
 * struct header_type	*header;
 * struct element_type	*elm;
 *
 * header = &container->header_field;
 * for (elm = header->head; elm != (void *)header; elm = elm->field.next)
 */

/*
 *	insert <elm> at head of list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * elm - ptr to the list element to be inserted
 * field - name of the link field in the list element
 */
#define CDLL_INSERT_HEAD(header, elm, field) {\
	(elm)->field.next = (header)->head;\
	(elm)->field.prev = (void *)(header);\
	if ((header)->tail == (void *)(header))\
		(header)->tail = (elm);\
	else\
		(header)->head->field.prev = (elm);\
	(header)->head = (elm);\
}

/*
 *	insert <elm> at tail of list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * elm - ptr to the list element to be inserted
 * field - name of the link field in the list element
 */
#define CDLL_INSERT_TAIL(header, elm, field) {\
	(elm)->field.next = (void *)(header);\
	(elm)->field.prev = (header)->tail;\
	if ((header)->head == (void *)(header))\
		(header)->head = (elm);\
	else\
		(header)->tail->field.next = (elm);\
	(header)->tail = (elm);\
}

/*
 *	insert <elm> after <listelm> of list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * listelm - ptr to the list element at insertion point
 * elm - ptr to the list element to be inserted
 * field - name of the link field in the list element
 */
#define CDLL_INSERT_AFTER(header, listelm, elm, field) {\
	(elm)->field.next = (listelm)->field.next;\
	(elm)->field.prev = (listelm);\
	if ((listelm)->field.next == (void *)(header))\
		(header)->tail = (elm);\
	else\
		(listelm)->field.next->field.prev = (elm);\
	(listelm)->field.next = (elm);\
}

/*
 *	insert <elm> before <listelm> of list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * listelm - ptr to list element at insertion point
 * elm - ptr to the element to be inserted
 * field - name of the link field in the list element
 */
#define CDLL_INSERT_BEFORE(header, listelm, elm, field) {\
	(elm)->field.next = (listelm);\
	(elm)->field.prev = (listelm)->field.prev;\
	if ((listelm)->field.prev == (void *)(header))\
		(header)->head = (elm);\
	else\
		(listelm)->field.prev->field.next = (elm);\
	(listelm)->field.prev = (elm);\
}

/*
 *	remove <elm> from list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * elm - ptr to the list element to be removed
 * field - name of the link field in the list element
 */
#define	CDLL_REMOVE(header, elm, field) {\
	if ((elm)->field.next == (void *)(header))\
		(header)->tail = (elm)->field.prev;\
	else\
		(elm)->field.next->field.prev = (elm)->field.prev;\
	if ((elm)->field.prev == (void *)(header))\
		(header)->head = (elm)->field.next;\
	else\
		(elm)->field.prev->field.next = (elm)->field.next;\
}

#define CDLL_MOVE_TO_HEAD(header, elm, field) {\
	if ((elm)->field.prev != (void *)(header))\
	{\
		if ((elm)->field.next == (void *)(header))\
			(header)->tail = (elm)->field.prev;\
		else\
			(elm)->field.next->field.prev = (elm)->field.prev;\
		(elm)->field.prev->field.next = (elm)->field.next;\
		(elm)->field.next = (header)->head;\
		(elm)->field.prev = (void *)(header);\
		(header)->head->field.prev = (elm);\
		(header)->head = (elm);\
	}\
}

#define CDLL_MOVE_TO_TAIL(header, elm, field) {\
	if ((elm)->field.next != (void *)(header))\
	{\
		(elm)->field.next->field.prev = (elm)->field.prev;\
		if ((elm)->field.prev == (void *)(header))\
			(header)->head = (elm)->field.next;\
		else\
			(elm)->field.prev->field.next = (elm)->field.next;\
		(elm)->field.next = (void *)(header);\
		(elm)->field.prev = (header)->tail;\
		(header)->tail->field.next = (elm);\
		(header)->tail = (elm);\
	}\
}

/*
 *	orphan list element
 */
#define	CDLL_SELF(elm, field)\
	(elm)->field.next = (elm)->field.prev = (elm);


/*
 *		single head doubly-linked list
 *
 * A list is headed by a single head pointer.
 * The elements are doubly linked so that an arbitrary element can be 
 * removed without a need to traverse the list. 
 * New elements can be added to the list at the head of the list, or 
 * after an existing element (NO insert at tail).
 * A list may only be traversed in the forward direction.
 * (note: the list is NULL terminated in next field.)
 *
 *   +-----+          +->+-----+  +->+-----+    +->+-----+  
 *   | NULL|          |  |  h  +--+  |  n  +----+  | NULL|     
 *   +-----+          |  +-----+  |  +-----+       +-----+
 *                    |           |  |  p  +--+    |  p  +--+
 *                    |           |  +-----+  |    +-----+  |
 *                    +-----------------------+             |
 *                                |                         |
 *                                +-------------------------+
 */
#define LIST_HEADER(type)\
struct {\
	struct type *head;\
}

#define LIST_ENTRY(type)\
struct {\
	struct type *next;\
	struct type **prev;\
}

#define	LIST_INIT(header)	{ (header)->head = NULL; }

/*
 *	scan list
 *
 * header - ptr to the header (field in header element)
 * elm - ptr to the element to be inserted
 * field - name of the link field in list element
 *
 * struct header_container	*container;
 * struct header_type	*header;
 * struct element_type	*elm;
 *
 * header = &container->header_field;
 * for (elm = header->head; elm; elm = elm->field.next)
 */

#define LIST_INSERT_HEAD(header, elm, field) {\
	if (((elm)->field.next = (header)->head) != NULL)\
		(header)->head->field.prev = &(elm)->field.next;\
	(header)->head = (elm);\
	(elm)->field.prev = &(header)->head;\
}

#define LIST_INSERT_AFTER(listelm, elm, field) {\
	if (((elm)->field.next = (listelm)->field.next) != NULL)\
		(listelm)->field.next->field.prev = &(elm)->field.next;\
	(listelm)->field.next = (elm);\
	(elm)->field.prev = &(listelm)->field.next;\
}

#define LIST_REMOVE(elm, field) {\
	if ((elm)->field.next != NULL)\
		(elm)->field.next->field.prev = (elm)->field.prev;\
	*(elm)->field.prev = (elm)->field.next;\
}

#define LIST_SELF(elm, field) {\
	(elm)->field.next = NULL;\
	(elm)->field.prev = &(elm)->field.next;\
}

#endif /* !_H_JFS_TYPES */
