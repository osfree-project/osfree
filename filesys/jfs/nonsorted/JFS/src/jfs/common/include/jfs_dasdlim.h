/* $Id: jfs_dasdlim.h,v 1.1.1.1 2003/05/21 13:35:42 pasha Exp $ */

/* static char *SCCSID = "@(#)1.8  11/1/99 11:50:00 src/jfs/common/include/jfs_dasdlim.h, sysjfs, w45.fs32, fixbld";
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
 * Change History :
*/

#ifndef _H_JFS_DASDLIM
#define	_H_JFS_DASDLIM

// BEGIN D233382
/*
 * When _JFS_FASTDASD is defined, changes to DASD usage in directories are
 * not committed as part of the responsible transaction.  This requires that
 * DASD usage be recalculated after a system crash
 */
#define _JFS_FASTDASD
// END D233382

/*
 *	jfs_dasdlim.h: 
 *
 * DASD Limits (Directory Limits)
 *
 */
#define MIXED_32
#include <net32/netcons.h>
#include <net32/neterr.h>
#include <net32/dasd.h>
#include <priv/jfs_dlim.h>

/*
 * These are from the private LAN server version of dasd.h
 */
#define DLIM_INSTALL		1
#define DLIM_SYNCH		2
#define DLIM_RUNNING		4

/*
 * The following are defined in dasdioc.h, a LAN server private header file.
 */

/***********************************************
 * FSCTL interface between HPFS386 and NETAPI  *
 ***********************************************/

/*
 * FsCtl parameter packet for Pinball DASD apis:
 *
 *   FsDASDAdd,      FsDASDDelete
 *   FsDASDGetInfo,  FsDASDSetInfo
 */

struct _DASDINFO0PARM {
    void * _Seg16 dlags_sess;         /* info needed for audit   */
    unsigned long  dlags_uid;         /* user descriptor         */
    unsigned short dlags_rc;          /* return value            */
    unsigned short dlags_level;       /* info level              */
    unsigned short dlags_flag;        /* Used by Add and SetInfo */
    struct dasd_info_0 dlags_data;    /* DASD record             */
};  /* _DASDINFO0PARM */

typedef struct _DASDINFO0PARM DASDINFO0PARM;
typedef DASDINFO0PARM * PDASDINFO0PARM;

/*
 *  FsCtl parameter packet for NetDASDEnum
 */

struct _DLIMENUMPARM {
    unsigned long   pdenu_uid;      /* user descriptor */
    void * _Seg16 pdenu_sess;       /* user info for audit */
    unsigned char   pdenu_handle;   /* Enum Handle */
    unsigned char   pdenu_flags;    /* enum info flags */
    unsigned short  pdenu_count;    /* # DLIM records in list */
    unsigned short  pdenu_return;   /* error code */
    char            pdenu_name[MAXPATHLEN];   /* Lanman style path    */
    char            pdenu_data[1];            /* Buffer for DASD recs */
};      /* _DLIMENUMPARM */
typedef struct _DLIMENUMPARM DLIMENUMPARM;

/*
 * FsCtl parameter packet for NetDASDInit
 */
struct _DASDINIT0PARM {
    void * _Seg16 dlinit_sess;       /* info needed for audit   */
    unsigned long  dlinit_uid;       /* user descriptor         */
    unsigned short dlinit_rc;        /* return value            */
    unsigned short dlinit_level;     /* info level              */
    struct dasd_init_0 dlinit_data;  /* DASD init info          */
};   /* _DASDINIT0PARM */

typedef struct _DASDINIT0PARM  DASDINIT0PARM;
typedef DASDINIT0PARM * PDASDINIT0PARM;

/*
 * FsCtl parameter packet for NetDASDCheck
 */
struct _DASDCHECK0PARM {               /*                         @p02a */
    void * _Seg16 dlcheck_sess;        /* info needed for audit   @p02a */
    unsigned long  dlcheck_uid;        /* user descriptor         @p02a */
    unsigned short dlcheck_rc;         /* return value            @p02a */
    unsigned short dlcheck_level;      /* info level              @p02a */
    struct dasd_check_0 dlcheck_data;  /* DASD check information  @p02a */
    char dlcheck_path[MAXPATHLEN];     /* To store returned path  @p02a */
};    /* _DASDCHECK0PARM */

typedef  struct _DASDCHECK0PARM   DASDCHECK0PARM;              /* @p02a */
typedef  DASDCHECK0PARM * PDASDCHECK0PARM;                     /* @p02a */


#ifdef   DL_ALERTS                  /* @p02a */
/* Support DosFSCtl() calls that are not part of NETAPI
 * See net/fileserv/netservr/lm386a.c for more details.
 */
#define  DLINIT_GET_INI_FILENAME    101   /* Get ini filename from HPFS386 */
#define  DLINIT_INIT_ALERTS_R0      102   /* Init alerts stuff at R0       */
#define  DLINIT_STOP_ALERTS_R0      103   /* Stop alerts stuff at R0       */
#endif   /* DL_ALERTS */

/*
 * End of defininitions from dasdioc.h
 */

/*
 * From LAN private version of dasd.h.  Missing from public version.
 */
#define DASD_CTL_QUERY		4	/* Query DASD Limits state */

/*
 * JFS internal definitions
 */

/*
 * Directory limits usage structure
 */
#define MAX_LIMITS	128
#define SMALL_LIMITS	27

struct dasd_usage {
	uint16	flag;
	uint8	limiting_inode;
	uint8	first_locked;					// D230860
	uint64 	avail;
	uint32	num_limits;
	struct inode	**pLimits;
	struct inode	*Limits[SMALL_LIMITS];
};

struct dasd_alert {
	struct vfs		*vfsp;
	ino_t			inum;
	struct inode		*ip;
	uint32			freeblocks;
	uint16			interval;
	uint16			session;
	uint32			expires;
	struct dasd_alert	*next;
};

#define DLIM_LARGE	0x0001	/* Set if Limits array overflow */
#define DLIM_READONLY	0x0002	/* Set if reading only, ignore limits */
#define DLIM_ADMIN	0x0004	/* Set if admin/local user */
#define	DLIM_RELEASE	0x0008	/* DLIM_free should release inodes */
#define	DLIM_LOGGED	0x0010	/* TxLock taken for ancestor inodes */
#define DLIM_IP_IN_LIST	0x0020	/* Target IP is in Limits list */
#define DLIM_DIRS_LOCKED 0x0040 /* Write is taken on ancestor inodes */
#define DLIM_RENAME	0x0080	/* rename (move) operation	D233382 */

#define LIMITS_OFFSET \
	(sizeof(struct dasd_usage) - (SMALL_LIMITS*sizeof(struct inode *)))

#define DLIM_SIZE(du)	(LIMITS_OFFSET + \
    (((du)->flag & DLIM_LARGE) ? 0 : (((du)->num_limits)*sizeof(struct inode *))))

#define BIG_DLIM_SIZE	(LIMITS_OFFSET + (MAX_LIMITS * sizeof (struct inode *)))

#define DLIM_init(du, Flag)\
{\
	(du)->flag = Flag;\
	(du)->avail = -1;\
	(du)->num_limits = 0;\
	(du)->pLimits = (du)->Limits;\
}

#define DLIM_add(DU, IP, RC)\
{\
	(RC) = 0;\
	if ((((DU)->flag & DLIM_LARGE) == 0) &&\
	    ((DU)->num_limits == SMALL_LIMITS))\
		DLIM_grow(DU, RC)\
	if ((RC) == 0)\
		(DU)->pLimits[(DU)->num_limits++] = (IP);\
}	

#define DLIM_grow(du, rc)\
{\
	struct dasd_usage *dasd_ptr;\
	ASSERT(((du)->flag & DLIM_LARGE) == 0);\
	ASSERT((du)->num_limits == SMALL_LIMITS);\
	dasd_ptr = (struct dasd_usage *)allocpool(big_dlim_pool, 0);\
	if (dasd_ptr)\
	{\
		memcpy(&dasd_ptr->Limits[0], &(du)->Limits[0],\
		       SMALL_LIMITS*sizeof(struct inode *));\
		(du)->pLimits = &dasd_ptr->Limits[0];\
		(du)->flag |= DLIM_LARGE;\
		(rc) = 0;\
	}\
	else\
		(rc) = ENOMEM;\
}
		

#define DLIM_free(du)\
{\
	int32	i;\
	ASSERT(((du)->flag & DLIM_DIRS_LOCKED) == 0);\
	if (((du)->flag & DLIM_RELEASE) && ((du)->num_limits))\
	{\
		if ((du)->flag & DLIM_IP_IN_LIST)\
			(du)->num_limits--;\
		for (i = 0; i < (du)->num_limits; i++)\
			jfs_rele(IP2VP((du)->pLimits[i]));\
	}\
	if ((du)->flag & DLIM_LARGE)\
		freepool(big_dlim_pool, \
			(caddr_t *)(((int32)((du)->pLimits))-LIMITS_OFFSET)); \
}

#define DLIM_WRITE_LOCK(ip, du)\
{\
	if ((du) && ((du)->num_limits))\
		dasd_write_lock(ip, du);\
	else\
		IWRITE_LOCK(ip);\
}

#define DLIM_WRITE_UNLOCK(ip, du)\
{\
	if ((du) && ((du)->num_limits))\
		dasd_write_unlock(ip, du);\
	else\
		IWRITE_UNLOCK(ip);\
}

#define DLIM_WRITE_LOCK_ATTACH(ip, du)\
{\
	DLIM_WRITE_LOCK(ip, du)\
	if (((ip)->i_dasdlim == 0) && (du)->num_limits)\
		(ip)->i_dasdlim = (du);\
}

#define DLIM_WRITE_UNLOCK_DETACH(ip, du)\
{\
	if ((ip)->i_dasdlim == (du))\
		(ip)->i_dasdlim = 0;\
	DLIM_WRITE_UNLOCK(ip, du)\
}

#define DLIM_LOCK_DIRS(dip, du)\
{\
	if ((du)->num_limits)\
	{\
		dasd_write_lock(0, du);\
		(dip)->i_dasdlim = du;\
	}\
	else\
		IWRITE_LOCK(dip);\
}

#define DLIM_UNLOCK_DIRS(dip,du)\
{\
	if ((dip)->i_dasdlim)\
	{\
		(dip)->i_dasdlim = 0;\
		dasd_write_unlock(0, du);\
	}\
	else\
		IWRITE_UNLOCK(dip);\
}

#ifdef _JFS_FASTDASD
#define DLIM_TXCOMMIT(TID, IP, FLAG) txCommit(TID,1,&(IP),FLAG)	// D233382
#else
#define DLIM_TXCOMMIT(TID, IP, FLAG)\
	(((IP)->i_dasdlim)?dasd_commit(TID,IP,FLAG):txCommit(TID,1,&(IP),FLAG))
#endif

// BEGIN D233382
#ifdef _JFS_FASTDASD
#define DLIM_UPDATE(TID, IP, DELTA)\
	if ((IP)->i_dasdlim) \
	{ \
		if (DELTA)\
			dasd_update(TID, IP, DELTA); \
		dasd_unlockParents((IP)->i_dasdlim); \
	}
#else /* ! _JFS_FASTDASD */
// END D233382
#define DLIM_UPDATE(TID, IP, DELTA)\
	if ((IP)->i_dasdlim && (DELTA))\
		dasd_update(TID, IP, DELTA);
#endif /* _JFS_FASTDASD */

#define OVER_LIMIT(IP, NBLOCKS)\
	((IP)->i_dasdlim && over_limit((IP)->i_dasdlim, NBLOCKS, 0))
extern	pool_t	*dlim_pool;
extern	pool_t	*big_dlim_pool;

int32	dasd_prime(struct vfs *);
void	dasd_init(struct vfs *);
void	dasd_write_lock(struct inode *, struct dasd_usage *);
void	dasd_write_unlock(struct inode *, struct dasd_usage *);
struct dasd_usage *dlim_copy(struct dasd_usage *);
void	dlim_release(struct inode *);
#ifdef _JFS_FASTDASD
void	dasd_unlockParents(struct dasd_usage *);		// D233382
#else /* ! _JFS_FASTDASD */
int32	dasd_commit(int32, struct inode *, uint32);
#endif /* _JFS_FASTDASD */
void	dasd_update(int32, struct inode *, int64);
int32	over_limit(struct dasd_usage *, int64, uint32);

#endif /* !_H_JFS_DASDLIM */
