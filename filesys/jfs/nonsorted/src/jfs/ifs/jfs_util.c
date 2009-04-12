/* $Id: jfs_util.c,v 1.1 2000/04/21 10:58:18 ktk Exp $ */

static char *SCCSID = "@(#)1.23  4/9/99 14:02:52 src/jfs/ifs/jfs_util.c, sysjfs, w45.fs32, currbld";
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
 *
 * Module: jfs_util.c
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_debug.h"

/*
 * Global Data
 */
int	InitTime = 1;

/*
 * External Declarations
 */
void	DefineSelector(uint16, uint32);
void	ForgetSelector(uint32);


/*
 *	object pool management
 *	----------------------
 */

/* get pointer to the freelist chain field of the object */
#define freelink(obj, ap)\
	(*(caddr_t *)((caddr_t)(obj) + (ap)->a_freeoff))

/*
 * NAME:	initpool(nobj, inobj, objsize, freeoff, handle, app, flags)
 *
 * FUNCTION:	initialize object table, and
 *		allocate initial number of objects specified.
 *
 * PARAMETER:	nobj	- # of total elements in table
 *		inobj	- # of initial allocation
 *		objsize	- object size
 *		freeoff	- offset in struct that is avail to be used as 
 *			  a free list pointer(must be on long word boundary).
 *		handle	- Allocation id, debug info.
 *		app	- returned allocation info
 *		flags	- flags defining behavior
 *			  POOL16 - request 16-bit selector
 *			  POOL_TAKE_LOCK - take lock for serialization
 *
 * RETURN:	0	- success
 *		ENOMEM	- failure
 *
 * NOTES:	Initpool is usually called at DD Init time (on OS/2).
 *		In order to fully utilize memory on a loaded machine, we make
 *		all memory requests at init time asking for memory above 16M
 *		where possible.
 */
initpool(
	int32 nobj,	/* number of max objects in object table */
	int32 inobj,	/* initial # entries to initialize */
	int32 objsize,	/* object size */
	int32 freeoff,	/* offset in object to be used as freelist link */
	caddr_t handle,	/* object id, optional */
	pool_t **app,	/* returned allocation status structure */
	uint32 flags)	/* flags defining behavior	*/
{
	pool_t *ap;
#ifdef _JFS_OS2
	uint32	flag = kernel_heap|XMALLOC_HIGH;
#endif
	caddr_t	obj;
	int32	tblsize;
	int	i;

	/*
	 * allocate the allocation structure of the object table
	 */
#ifdef	_JFS_OS2
	ap = (pool_t *)xmalloc(sizeof(pool_t), 0, kernel_heap|XMALLOC_HIGH);
	if (ap == NULL && InitTime)
	{
		/* We aren't necessarily called at init time, lets try without
		 * asking for high memory.  (This flag is only valid at init
		 * time.)
		 */
		InitTime = 0;
		ap = (pool_t *)xmalloc(sizeof(pool_t), 0, kernel_heap);
	}
#endif	/* _JFS_OS2 */
	if (ap == NULL)
		return ENOMEM;

	/*
	 * allocate the object table
	 */
	ap->a_flags = flags;
	if (flags & POOL16)
		flag |= XMALLOC16;
	/*
	 * In OS/2 we're pinning all of our memory, so only allocate it as
	 * needed.  Initialize the special fields in the pool structure
	 * structure to deal with this.
	 */
	ap->a_objcnt = inobj;
	ap->a_objcur = inobj;
	ap->a_objmax = nobj;

	tblsize = objsize * inobj;
	ap->a_table = (caddr_t)xmalloc((uint)tblsize, 0, flag);
	if (ap->a_table == NULL)
		return ENOMEM;

	/* zero out the initial number of objects to initialize
	 */
	bzero(ap->a_table, inobj * objsize);

	/* initialize the allocation structure of the object table:
	 * address of end of table, object size and freelist offset in object.
	 */
	ap->a_end = ap->a_table + tblsize;
	ap->a_objsize = objsize;
	ap->a_freeoff = freeoff;

	/*
	 * construct freelist of a singly-linked NULL-terminated list
	 * (linked via freelist offset in object)
	 * with the initial number of objects to initialize
	 */
	ap->a_freelist = obj = ap->a_table;

	for (i = 1; i < inobj; i++, obj += objsize)
		freelink(obj, ap) = obj + objsize;
	freelink(obj, ap) = NULL;

	/* set high water mark of the object table with the address of 
	 * the next object of the last object of the freelist
	 */
	ap->a_hwm = obj + objsize;

#ifdef	_JFS_OS2
	if (flags & POOL_TAKE_LOCK)
	{
		SPINLOCK_ALLOC(&ap->a_lock, LOCK_ALLOC_PIN,
				JFS_POOL_LOCK_CLASS, 0);
		SPINLOCK_INIT(&ap->a_lock);
		ap->a_sleeplock.locked = 0;
		ap->a_sleeplock.wanted = 0;
	}
#endif	/* _JFS_OS2 */
	/* set the object id in the allocation structure of the object table
	 */
	strncpy (ap->a_handle, handle, sizeof (ap->a_handle));

	*app = ap;
	return 0;
}


/*
 * NAME:	allocpool(ap, lock)
 *
 * FUNCTION:	allocate object(s) from the object table.
 *
 *		for objectsize < pagesize, new region is allocated
 *		consisting of remainder of current page if any (which
 *		is less than object size) and a new page, and maximum 
 *		number of objects that fits in the region are created.
 *
 * PARAMETERS:	ap	- allocation struct
 *		lock	- transform spinlock held by caller
 *
 * RETURNS:	Pointer to allocated object
 *		NULL on overflow
 *
 * SERIALIZATION: iCache: ICACHE_LOCK() held on entry/exit (getip()/getdp()).
 *		  Others: Specify POOL_TAKE_LOCK to initpool, and we take
 *			  ap->a_lock to serialize contention of the data
 *			  structure.
 *			  If we must call xmalloc, we must transform TSPINLOCK
 *			  to sleep locks before calling xmalloc, and trade
 *			  spin lock, a_lock for sleep lock a_sleeplock.
 *			  a_lock cannot be a TSPINLOCK, because we may block
 *			  trying to aquire it while holding untransormed
 *			  TSPINLOCK.
 */
caddr_t
allocpool(
pool_t		*ap,
TSPINLOCK_T	*lock)
{
	caddr_t	obj;
#ifdef	_JFS_OS2
	int32	take_lock = ap->a_flags & POOL_TAKE_LOCK;

	if (take_lock)
		SPINLOCK_LOCK(&ap->a_lock);
#endif	/* _JFS_OS2 */

allocpool_retry:
	/* allocate new objects if freelist is empty
	 */
	if (ap->a_freelist == NULL)
	{
		caddr_t pb;
		int	nobj, sz, nalloc, nfree, i;
#ifdef _JFS_OS2
		uint32	flag = kernel_heap;

		if (take_lock & ap->a_sleeplock.locked)
		{
			/* another thread is already calling xmalloc */
			if (lock)
				TSPINLOCK_XBLOCK(lock);

			ap->a_sleeplock.wanted++;
			KernBlock((ULONG)&ap->a_sleeplock,-1,
				  T_SPINLOCK|BLOCK_UNINTERRUPTABLE,
				  &ap->a_lock, 0);
			
			if (lock)
			{
				/* Can't take TSPINLOCK while holding SPINLOCK*/
				SPINLOCK_UNLOCK(&ap->a_lock);
				TSPINLOCK_XSPIN(lock);
				SPINLOCK_LOCK(&ap->a_lock);
			}

			goto allocpool_retry;
		}

		/* check for the object table overflow */
		if (ap->a_objcur >= ap->a_objmax)
		{
			if (take_lock)
				SPINLOCK_UNLOCK(&ap->a_lock);
			return NULL;
		}

		/* compute allocation area to allocate new object(s) */
		nobj = MIN(ap->a_objcnt, ap->a_objmax - ap->a_objcur);
		sz = nobj * ap->a_objsize;

		/* allocate the additional space */
		if (ap->a_flags & POOL16)
			flag |= XMALLOC16;

		/* Transform TSPINLOCKs to sleep locks */
		if (take_lock)
		{
			ap->a_sleeplock.locked = 1;
			SPINLOCK_UNLOCK(&ap->a_lock);
		}
		if (lock)
			TSPINLOCK_XBLOCK(lock);

		pb = (caddr_t)xmalloc((uint)sz, 0, flag);

		/* Transform back to spinlocks */
		if (lock)
			TSPINLOCK_XSPIN(lock);
		if (take_lock) {
			SPINLOCK_LOCK(&ap->a_lock);
			ap->a_sleeplock.locked = 0;
			if (ap->a_sleeplock.wanted)
			{
				ap->a_sleeplock.wanted = 0;
				KernWakeup((ULONG)(&ap->a_sleeplock), 0, 0, 0);
			}
		}

		if (pb == NULL)
		{
			if (take_lock)
				SPINLOCK_UNLOCK(&ap->a_lock);
			return NULL;
		}

		/* zero out the allocation area */
		memset(pb, 0, sz);

		/* insert object of allocation area into freelist
		 * We can't assume a_freelist is empty since we gave up lock.
		 */

		obj = pb;
		for (i = 1; i < nobj; i++, obj += ap->a_objsize)
			freelink(obj, ap) = obj + ap->a_objsize;
		freelink(obj, ap) = ap->a_freelist;
		ap->a_freelist = pb;

		/* set high water mark of the object table
	 	 */
		ap->a_hwm = obj + ap->a_objsize;
#endif	/* _JFS_OS2 */
	}

	/* remove/return object from head of free list
	 */
	obj = (caddr_t) ap->a_freelist;
	ap->a_freelist = freelink(obj, ap);

	freelink(obj, ap) = NULL;

#ifdef	_JFS_OS2
	if (take_lock)
		SPINLOCK_UNLOCK(&ap->a_lock);
#endif	/* _JFS_OS2 */

	return obj;
}


/*
 * NAME:	freepool(ap, addr)
 *
 * FUNCTION:	Free objects previously allocated by the commom
 *		allocator, genalloc().
 *
 * PARAMETERS:	ap	- allocation struct as defined in <sys/genalloc.h>
 *		addr	- pointer to the object
 *
 * RETURNS:	None
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit (iunhash()/dquhash()).
 */
void
freepool(pool_t 	*ap,
	 caddr_t	*obj)
{
#ifdef	_JFS_OS2
	int32 take_lock = ap->a_flags & POOL_TAKE_LOCK;
	if (take_lock)
		SPINLOCK_LOCK(&ap->a_lock);
#endif	/* _JFS_OS2 */
	/* insert at head of free list
	 */
	freelink(obj, ap) = ap->a_freelist;
	ap->a_freelist = (caddr_t) obj;
#ifdef	_JFS_OS2
	if (take_lock)
		SPINLOCK_UNLOCK(&ap->a_lock);
#endif	/* _JFS_OS2 */
}


/*
 *	Miscellaneous Utilities
 *	-----------------------
 */

/*
 * NAME:	log2shift(n)
 *
 * FUNCTION:	calculate the log base 2 of n.
 *
 * PARAMETER:	n	- number to find log base 2 of
 *
 * RETURN:	log base 2 of number
 */
int
log2shift(register uint32	n)
{
	register uint32		shift = 0;

	while (n > 1)
	{
		/* n is not power of 2 */
		if (n & 1)
			return -1;

		shift++;
		n >>= 1;
	}

	return shift;
}


/*
 *	roundup2power()
 *
 * round up to next smallest power of 2;
 */
roundup2power(x)
{
	int32	i;
	uint32	m;

	m = 0x80000000;
	
	for (i = 0; i < 32; i++)
	{
		if (x & m)
			break;
		m >>= 1;
	}
	
	x = (x + (m - 1)) & ~(m - 1);
	return x;
}


#ifdef _JFS_OS2
/*
 *	Memory Management Services
 *	--------------------------
 */

/*
 * NAME:	xmalloc
 *
 * FUNCTION:	Allocate memory.
 *
 * PARAMETERS:	size - number of bytes to allocate
 *		alignment - ignored
 *		heap - non-zero for pinned storage
 *
 * RETURN:	pointer to allocated storage
 */
void *
xmalloc(uint32 size,
	uint32 align,
	uint32 heap)
{
	int	flag;
	void	*addr;
	void	*addr16;

	flag = VMDHA_FIXED;	/* pin */

	/* is shared with antique DD ? */
	if (heap & XMALLOC16)
	{
		assert(size <= 65536);
		flag |= VMDHA_SELMAP;
	}
	if (heap & XMALLOC_CONTIG)
		flag |= VMDHA_CONTIG;

	/*  We may be called at DD Init time and may want memory above 16M */
	if (InitTime && (heap & XMALLOC_HIGH))
		flag |= VMDHA_USEHIGHMEM;

	if (KernVMAlloc(size, flag, &addr, (VOID *)-1, &addr16) != 0)
	{
		jFYI(1,("KernVMAlloc failed\n"));	/* XXX */
		addr = NULL;
	}

	if (heap & XMALLOC16)
		DefineSelector(((uint32)addr16 >> 16) & 0xFFF8, 0);

	jFYI(0,("KernVMAlloc %d at %x\n", size, addr));
	return addr;
}


/*
 * NAME:	xmfree
 *
 * FUNCTION:	Free memory acquired through xmalloc
 *
 * PARAMETER:	addr - address of memory to be freed
 *		heap - ignored
 *
 * RETURN:	none
 */
void
xmfree(void  *addr,
       uint32 heap)
{
	ForgetSelector((uint32)addr);
	KernVMFree(addr);
}


/*
 * NAME:	VPBtoVFS
 *
 * FUNCTION:	Map a volume parameter block handle to a vfs
 *
 * PARAMETER:	hVPB	- VPB handle
 *
 * RETURN:	pointer to vfs, or NULL if not found
 */
struct vfs *
VPBtoVFS(SHANDLE hVPB)
{
	struct vfs *vfsp;
	extern struct vfs *vfs_anchor;

	for (vfsp = vfs_anchor;
	     vfsp != NULL;
	     vfsp = vfsp->vfs_next)
	{
		if (vfsp->vfs_hVPB == hVPB)
			break;
	}
	
	return vfsp;
}


/*
 * NAME:	PATHtoVFS
 *
 * FUNCTION:	Map path name to a vfs
 *
 * PARAMETER:	path	- Ascii pathname of file beginning w/drive letter
 *			  (Note: only first three characters are examined)
 *
 * RETURN:	pointer to vfs, or NULL if not found
 */
struct vfs *
PATHtoVFS(
char *path)
{
	struct vfs		*vfsp;
	extern struct vfs	*vfs_anchor;
	uint32			drive;

	drive = path[0];
	if (drive >= 'a')
		drive -= 'a';
	else
		drive -= 'A';

	if ((path[1] != ':') || (path[2] != '\\'))
		return 0;

	for (vfsp = vfs_anchor;
	     vfsp != NULL;
	     vfsp = vfsp->vfs_next)
	{
		if (vfsp->vfs_vpfsi->vpi_drive == drive)
			break;
	}
	
	return vfsp;
}
#endif	/* _JFS_OS2 */
