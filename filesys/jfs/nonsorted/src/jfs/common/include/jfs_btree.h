/* $Id: jfs_btree.h,v 1.1 2000/04/21 10:57:55 ktk Exp $ */

/* static char *SCCSID = "@(#)1.5  3/11/99 15:50:29 src/jfs/common/include/jfs_btree.h, sysjfs, w45.fs32, 990417.1";
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
#ifndef	_H_JFS_BTREE
#define _H_JFS_BTREE	
/*
 *	jfs_btree.h: B+-tree
 *
 * JFS B+-tree (dtree and xtree) common definitions
 */

#include "jfs_bufmgr.h"

/*
 *	basic btree page - btpage_t
 */
typedef struct {
	int64		next;		/* 8: right sibling bn */
	int64		prev;		/* 8: left sibling bn */

	uint8		flag;		/* 1: */
	uint8		rsrvd[7];	/* 7: type specific */
	int64		self;		/* 8: self address */

	uint8		entry[4064];	/* 4064: */
} btpage_t;				/* (4096) */

/* btpaget_t flag */
#define BT_TYPE		0x07		/* B+-tree index */
#define	BT_ROOT		0x01		/* root page */
#define	BT_LEAF		0x02		/* leaf page */
#define	BT_INTERNAL	0x04		/* internal page */
#define	BT_RIGHTMOST	0x10		/* rightmost page */
#define	BT_LEFTMOST	0x20		/* leftmost page */


#ifndef	_JFS_UTILITY
/*
 *	btree page buffer cache access
 */
/* get page from buffer page */
#define BT_PAGE(IP, BP, TYPE)\
	(BP->j_xflag & B_BUFFER) ? (TYPE *)BP->b_bdata : (TYPE *)&IP->i_btroot

/* get the page buffer and the page for specified block address */
#define BT_GETPAGE(IP, BN, BP, TYPE, SIZE, P, RC)\
{\
	if ((BN) == 0)\
	{\
		BP = (jbuf_t *)&IP->i_bxflag;\
		P = (TYPE *)&IP->i_btroot;\
		RC = 0;\
	}\
	else\
	{\
		RC = bmRead(IP, (BN), (SIZE), bmREAD_BLOCK, &(BP));\
		if (RC == 0)\
			P = (TYPE *)BP->b_bdata;\
	}\
}

/* put the page buffer */
#define BT_PUTPAGE(BP)\
{\
	if ((BP)->j_xflag & B_BUFFER)\
		bmRelease(BP);\
}


/*
 *	btree traversal stack
 *
 * record the path traversed during the search;
 * top frame record the leaf page/entry selected.
 */
#define	MAXTREEHEIGHT		8
typedef struct btframe {	/* stack frame */
	int64	bn;		/* 8: */
	int16	index;		/* 2: */
	int16	lastindex;	/* 2: */
	jbuf_t	*bp;		/* 4: */
} btframe_t;			/* (16) */

typedef struct btstack {
	btframe_t	*top;	/* 4: */
	int32		nsplit;	/* 4: */
	btframe_t	stack[MAXTREEHEIGHT];
} btstack_t;

#define BT_CLR(btstack)\
	(btstack)->top = (btstack)->stack

#define BT_PUSH(BTSTACK, BN, INDEX)\
{\
	(BTSTACK)->top->bn = BN;\
	(BTSTACK)->top->index = INDEX;\
	++(BTSTACK)->top;\
	assert((BTSTACK)->top != &((BTSTACK)->stack[MAXTREEHEIGHT]));\
}

#define BT_POP(btstack)\
	( (btstack)->top == (btstack)->stack ? NULL : --(btstack)->top )

#define BT_STACK(btstack)\
	( (btstack)->top == (btstack)->stack ? NULL : (btstack)->top )

/* retrieve search results */
#define BT_GETSEARCH(IP, LEAF, BN, BP, TYPE, P, INDEX)\
{\
	BN = (LEAF)->bn;\
	BP = (LEAF)->bp;\
	if (BN)\
		P = (TYPE *)BP->b_bdata;\
	else\
		P = (TYPE *)&IP->i_btroot;\
	INDEX = (LEAF)->index;\
}

/* put the page buffer of search */
#define BT_PUTSEARCH(BTSTACK)\
{\
	if ((BTSTACK)->top->bp->j_xflag & B_BUFFER)\
		bmRelease((BTSTACK)->top->bp);\
}
#endif	/* _JFS_UTILITY */

#endif /* _H_JFS_BTREE */
