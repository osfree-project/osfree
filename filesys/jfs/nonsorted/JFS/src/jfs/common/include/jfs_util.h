/* $Id: jfs_util.h,v 1.1.1.1 2003/05/21 13:35:56 pasha Exp $ */

/* static char *SCCSID = "@(#)1.8  7/30/98 14:07:59 src/jfs/common/include/jfs_util.h, sysjfs, w45.fs32, 990417.1";
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
#ifndef _H_JFS_UTIL
#define	_H_JFS_UTIL

#include <jfs_lock.h>

/* Prototypes for utility functions
 */

void	printf(char *, ...);

int32 jfs_hold(struct vnode *vp);
int32 jfs_rele(struct vnode *vp);

#define bzero(string, count)		memset(string, 0, count)
#define bcopy(source, dest, count)	memcpy(dest, source, count)

#define copyin(src, dest, len) KernCopyIn(dest, src, len)
#define copyout(src, dest, len) KernCopyOut(dest, src, len)

/*
 *	Object pool management
 */
int32 initpool(
	int32	nobj,	/* max number of objects in object table */
	int32	inobj,	/* initial # entries to initialize */
	int32	objsize, /* object size */
	int32	freeoff, /* offset in object to be used as freelist link */
	caddr_t handle,	/* object id, optional */
	pool_t	**app,	/* returned allocation status structure */
	uint32	flags);	/* flags defining behavior	*/
			/* POOL16 - request 16-bit selector */
			/* POOL_TAKE_LOCK - take lock to serialize */

caddr_t allocpool(pool_t *ap, TSPINLOCK_T *lock);

void freepool(pool_t  *ap,
	      caddr_t *obj);

#define	POOL16		0x00000001	/* request 16-bit selector */
#define	POOL_TAKE_LOCK	0x00000002	/* take lock to serialize */


int32 log2shift(uint32 n);
int32 roundup2power(int32 x);


/*
 *	Memory Allocation
 */

#define kernel_heap     0
#define pinned_heap     1

/*
 * The following flags may be or'ed with heap indicator (which itself is
 * ignored.)
 */
#define XMALLOC16	0x10	/* Define 16-bit selector for malloc'ed space */
#define XMALLOC_HIGH	0x20	/* Only valid at DD Init time - May use memory
				 * above 16M.	*/
#define XMALLOC_CONTIG	0x40	/* Allocate contiguous memory	*/

  void *xmalloc(uint32 size,    /* number of bytes to allocate  */
                uint32 align,   /* alignment (ignored)          */
                uint32 heap);   /* heap indicator               */

    void xmfree(void * addr,    /* address to be freed          */
                uint32 heap);   /* heap indicator               */


      int32 pin(void * addr,    /* address to lock in memory    */
		uint32 size);   /* number of bytes in range     */

    int32 unpin(void * addr,    /* address to be unlocked       */
		uint32 size);   /* number of bytes in range     */

#endif /* _H_JFS_UTIL */
