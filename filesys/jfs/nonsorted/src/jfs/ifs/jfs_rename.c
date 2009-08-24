/* $Id: jfs_rename.c,v 1.1 2000/04/21 10:58:16 ktk Exp $ */

static char *SCCSID = "@(#)1.15.1.2  12/2/99 11:13:39 src/jfs/ifs/jfs_rename.c, sysjfs, w45.fs32, fixbld";
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
 * Module: jfs_rename.c
 */

/*
 * Change History :
 *
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
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_txnmgr.h"
#include "jfs_dnlc.h"
#include "jfs_proto.h"
#include "jfs_debug.h"
#include <uni_inln.h>

/*
 *      External references
 */
void    iwritelocklist(int, ... );
void	sort_and_lock(inode_t *, int32, inode_t **);		// F226941

/*
 * NAME:        jfs_checkpath
 *
 * FUNCTION:    Search the directory tree rooted with topar_ip to see if
 *              it includes from_ip
 *
 * PARAMETERS:
 *      from_ip         - pointer to original directory inode
 *      frompar_ip      - pointer to parent of from_ip
 *      topar_ip        - pointer to destination parent inode
 *      vfst            -
 *
 * NOTES:       RENAME_LOCK() of mount inode is used to serialize renaming
 *              within an aggregate.  This prevents someone from modifying
 *              behind us as we walk up the tree.  Since we hold the rename
 *              lock, we don't need to lock each parent along the directory
 *              tree.  We already have a lock on a directory child, so none of
 *              the directories we visit will be empty.  Therefore, they cannot
 *              be removed until our rename is complete.  This will work as long
 *              as we continue to not support unlink() of a directory.
 *
 * RETURN:      0       - from_ip is not a parent of topar_ip.
 *              != 0    - from_ip is a parent or some other error.
 */
static int32
jfs_checkpath(  inode_t *from_ip,
                inode_t *frompar_ip,
                inode_t *topar_ip,
                struct vfs      *vfst)
{
        ino_t   par_ino;
        inode_t *parent_ip;
        int32   rc = 0;

        if (topar_ip->i_number == ROOT_I)
        {
                /*
                 * We have reached the root and not found the source so we know
                 * we don't have an orphan
                 */
                return 0;
        }

        /*
         * Walk up the destination directory tree looking for orphans
         */
        par_ino = ((dtroot_t *)&(topar_ip->i_btroot))->header.idotdot;
        while(1)
        {
                if (par_ino <= 0)
                {
                        /* We have hit a bad dotdot entry */
                        assert(0);
                }

                if (par_ino == ROOT_I ||
                        par_ino == frompar_ip->i_number)
                {
                        rc = 0;
                        break;
                }

                if (par_ino == from_ip->i_number)
                {
                        /*
                         * Uh-oh: we have an orphan situation
                         */
                        rc = EINVAL;
                        break;
                }

                ICACHE_LOCK();
                rc = iget(vfst, par_ino, &parent_ip, 0);
                ICACHE_UNLOCK();
                if (rc != 0)
                {
                         break;
                }

                par_ino = ((dtroot_t *)&(parent_ip->i_btroot))->header.idotdot;
                ICACHE_LOCK();
                iput(parent_ip, vfst);
                ICACHE_UNLOCK();
        }

        return rc;
}

/*
 * NAME:        jfs_rename
 *
 * FUNCTION:    rename a file or directory
 *
 * PARAMETER:   from_vp         - pointer to source vnode
 *              frompar_vp      - pointer to source parent vnode
 *              from_name       - original name
 *              to_vp           - pointer to target vnode (if it exists)
 *              topar_vp        - pointer to target parent vnode
 *              to_name         - new name
 *              crp             - pointer to caller's credentials
 *
 * RETURN:      errors from subroutines
 *
 * JFS supports sticky bit permission.
 *
 * normally, to rename an object, user needs execute and write
 * permission of the directory containing the object as well as
 * on the target directory.
 * EACCESS: if S_ISVTX (aka sticky) bit is set for a directory,
 * a file in the directory can be renamed only if
 * the user has write permission for the directory, and
 * either owns the file, owns the directory, or
 * is the superuser (or have appropriate privileges).
 * (e.g., /tmp in which any user can create a file but
 * should not be able to delete or rename files owned
 * by others) [XPG4.2]
 *
 * Basic algorithm is:
 *
 * 1) Check validity of parameters and then obtain locks on the source parent
 *    directory, destination parent directory, source, and the destination
 *    inode.  This ensures neither the source or destination will be deleted out
 *    from underneath us.  It also ensures the source or destination won't be
 *    modified before we finish.
 * 2) Link source to destination.  If destination already exists,
 *    delete it first.
 * 3) Unlink source reference to inode if still around. If a
 *    directory was moved and the parent of the destination
 *    is different from the source, patch the ".." entry in the
 *    directory.
 *
 * The transaction log needs to have the changes journalled in such a
 * way that a crash meets the requirement that the destination always
 * exists (if it existed before).  For directory objects it is critical
 * that the changes in i-numbers occur in such a way that loops and
 * orphans aren't created by a crash.
 */
int32
jfs_rename(
        struct vnode    *from_vp,       /* source vnode */
        struct vnode    *frompar_vp,    /* source parent vnode */
	UniChar		*from_name,     /* source name */
        struct vnode    *to_vp,         /* destination vnode */
        struct vnode    *topar_vp,      /* destination parent vnode */
	UniChar		*to_name,       /* destination name */
        struct dasd_usage  *from_dlim,	/* dasd usage structure for source  F226941 */
        struct dasd_usage  *to_dlim,	/* dasd usage structure for dest  F226941 */
	uint32		flags)		/* INEWNAME if new pathname not 8.3 */
{
	inode_t	*ipmnt;
        int32   tid = -1;               /* Transaction ID */
        int32   txCount = 0;            /* Count of inodes in txList */
        inode_t *txList[4];             /* List of modified inodes */
        inode_t *from_ip = VP2IP (from_vp);     /* Source inode */
        inode_t *frompar_ip = VP2IP (frompar_vp); /* Source parent inode */
        inode_t *to_ip = to_vp ? VP2IP (to_vp) : 0;
                                        /* Target inode (if it exists) */
        inode_t *topar_ip = VP2IP (topar_vp);   /* Target parent inode */
        struct vfs      *vfsf = frompar_vp->v_vfsp;
        struct vfs      *vfst = topar_vp->v_vfsp;
        int32   doingdirectory = 0,     /* Source inode is a directory */
                newparent = 0;          /* New i-number of parent */
        int32   got_from = 0, got_to = 0;
        btstack_t btstack;              /* Temporary stack for B-tree struct */
        component_t     dname;          /* Directory name structure */
        ino_t   ino;                    /* I-number for directory searches */
        int32   error = 0, rc;
	tlock_t		*tlck;		/* Transaction lock */
	dtlock_t	*dtlck;		/* dtree line lock */
	lv_t		*lv;		/* line lock vector */

// BEGIN F226941

#ifdef	_JFS_OS2
	int64		blocks_moving;	/* Number of blocks being moved */
	int64		orig_fromblocks; /* original dasd usage of from dir */
	int64		orig_toblocks;	/* original dasd usage of to dir */
	uint32		start_here;	/* index in limits array */
	int32		i;
	inode_t		*lock_arr[64];	/* Array of inodes to lock */
	inode_t		**lock_list;	/* List of inodes to lock */
	int32		num_locks;	/* Number of inodes to lock */
	int32		upper_limit;	/* Upper limit to number of inodes
					 * we need to lock */
#ifdef _JFS_FASTDASD
	int32		first_locked;	/* index in lock_list array of first
					 * inode still locked.		D233382
					 */
#endif /* _JFS_FASTDASD */
#endif	/* _JFS_OS2 */

// BEGIN F226941

        assert( to_name != NULL && from_name != NULL);

	ipmnt = frompar_ip->i_ipmnt;
	RENAME_LOCK(ipmnt);

        /*
         * Make sure that we are not renaming the "." or ".." entries in
         * a directory.
         */

        if (from_name[0] == '.' &&
                (!from_name[1] || (from_name[1] == '.' && !from_name[2])))
        {
                error = EINVAL;
                goto abortit;
        }

        /*
         * Now we want to verify the proper realationship between the inodes.
         * If everything checks out we need to obtain locks on the source parent
         * directory, destination parent directory, source, and destination
         * inodes.  This allows us to proceed knowing that noone can delete
         * either the source or destination from underneath us.  It also allows
         * us to know noone will be modifying the destination so we can delete
         * it safely.  We need the lock on the source since we might need to
         * modify its .. entry.
         */

	/*
	 * In OS/2, the destination cannot exist unless it is the same as
	 * the source.  This is to allow DosMove to change the case of a
	 * filename.
	 */
	if (to_ip && (from_ip != to_ip))
	{
		error = EINVAL;
		goto abortit;
	}

	/*
	 * Check for cross-device rename.
	 */
	if (frompar_ip->i_dev != topar_ip->i_dev)
	{
		error = EXDEV;
		goto abortit;
	}

	/*
	 * Check if source is the destination parent.  Can't do this
	 * since this would orphan everything under the source.
	 */
	if (from_ip == topar_ip)
	{
		error = EINVAL;
		goto abortit;
	}

	/*
	 * Now we know both the parents are okay.  We will lock both the
	 * parent inodes so we can guarantee the children will not be
	 * deleted underneath us.  We will also lock the source.
	 * When locking the inodes we will use the protocol of locking
	 * regular files first, then directories.  When locking more than
	 * one of each type, we do it in descending inode order.  We also need
	 * to make sure we don't attempt to lock one inode twice if the
	 * two parents are the same.
	 */
	if ((from_ip->i_mode & IFMT) == IFDIR)
		doingdirectory = 1;

// BEGIN F226941
	if (from_dlim->num_limits)
	{
		/* We need to lock all directories from the root to each of
		 * the from and to parent directories.  The DASD usage lists
		 * will contain at least one common entry (the root).  We need
		 * to compile a list of all unique inodes to lock.  The upper
		 * limit will be the sum of both num_limits fields.
		 */
		if (frompar_ip == topar_ip)
			upper_limit = from_dlim->num_limits + 1;
		else
			upper_limit = from_dlim->num_limits +
				       to_dlim->num_limits;
		if (upper_limit > 64)
			lock_list = (inode_t **)
					xmalloc(upper_limit* sizeof(inode_t *),
						0, kernel_heap);
		else
			lock_list = lock_arr;

		/* Put from parents dasd usage list into lock_list */
		for (num_locks = 0; num_locks < from_dlim->num_limits;
		     num_locks++)
			lock_list[num_locks] = from_dlim->pLimits[num_locks];

		/*
		 * We need to lock regular files before directories.  If
		 * we are moving a regular file, lock it now, otherwise add
		 * it to the lock list.
		 */
		if (doingdirectory)
			lock_list[num_locks++] = from_ip;
		else
			IWRITE_LOCK(from_ip);

		if (frompar_ip != topar_ip)
		{
			/* Add unique members of to parent's dasd usage list */
			for (i = 0; i < to_dlim->num_limits; i++)
				if (to_dlim->pLimits[i] != lock_list[i])
					break;
			start_here = i;		/* Needed by over_limit() */
			while (i < to_dlim->num_limits)
				lock_list[num_locks++] = to_dlim->pLimits[i++];
		}

		/* Lock them! */
		sort_and_lock(0, num_locks, lock_list);

		/*
		 * If this was a regular file, add the inode to the lock list
		 * now, so we include it in the txCommit, and unlock it
		 */
		if (!doingdirectory)
			lock_list[num_locks++] = from_ip;

		from_dlim->flag |= DLIM_DIRS_LOCKED;

		frompar_ip->i_dasdlim = from_dlim;
		if (frompar_ip != topar_ip)			// D230860
		{
			to_dlim->flag |= DLIM_DIRS_LOCKED;
			topar_ip->i_dasdlim = to_dlim;
		}
	}
	else
// END F226941

	{
		if (doingdirectory)
		{
			if (frompar_ip == topar_ip)
				iwritelocklist(2, frompar_ip, from_ip);
			else
				iwritelocklist(3, frompar_ip, topar_ip,
					       from_ip);
		}
		else
		{
			if (frompar_ip == topar_ip)
			{
				IWRITE_LOCK(from_ip);
				IWRITE_LOCK(frompar_ip);
			}
			else
			{
				IWRITE_LOCK(from_ip);
				iwritelocklist(2, frompar_ip, topar_ip);
			}
		}
	}
	/*
	 * Now that we have the source parent locked we can lookup the
	 * source inode to make sure it hasn't changed since we did our
	 * initial verification.  If it has we will need to release our
	 * locks and repeat our verification with the new version.
	 * Otherwise we know the source is valid and we can continue.
	 */
	dname.name = from_name;
	dname.namlen = UniStrlen (from_name);

	rc = dtSearch(frompar_ip, &dname, &ino, &btstack, JFS_LOOKUP);
	switch (rc)
	{
		case 0:
			/*
			 * An entry was found, need to see if it is the
			 * same inode as we had before.
			 */
			if (ino != from_ip->i_number)
			{
					rc = ENOENT;
					goto cleanup;
			}
			break;
		default:
			/*
			 * Either the source cannot be found or
			 * something went wrong in the search
			 */
			error = rc;
			goto cleanup;
	}

	/*
	 * If changing case, no need to search for destination inode
	 */
	if (!to_ip)
	{
		/*
		 * Now we can lookup the destination inode to make sure it
		 * hasn't been created since we did our verifications.  If it
		 * has we abort.
		 */
		dname.name = to_name;
		dname.namlen = UniStrlen (to_name);
		if (dname.namlen > JFS_NAME_MAX-1)
		{
			error = ERROR_FILENAME_EXCED_RANGE;
			goto cleanup;
		}
		rc = dtSearch(topar_ip, &dname, &ino, &btstack, JFS_LOOKUP);

		switch (rc)
		{
			case 0:
				/*
				 * We found an entry; fail with EEXIST
				 */
				rc = EEXIST;
				goto cleanup;
			case ENOENT:
				break;
			default:
				/*
				 * Some error from dtSearch().  cleanup and
				 * return
				 */
				error = rc;
				goto cleanup;
		}
	}

	/*
	 * Now we need to validate parameters and check for proper inodes
	 */

	/*
	 * Make sure the source inode has a positive link count
	 */
	if (from_ip->i_nlink <= 0)
	{
		error = EINVAL;
		goto cleanup;
	}

	if ((topar_ip->i_mode & IFMT) != IFDIR)
	{
		error = EINVAL;
		goto cleanup;
	}

	/* Check for the appropriate permissions on the source */

	if (doingdirectory && (frompar_ip->i_number != topar_ip->i_number))
	{
		/*
		 * Account for ".." in new directory.  When source and
		 * destination have the same parent we don't fool with the link
		 * count.
		 */
		if ((nlink_t)topar_ip->i_nlink >= LINK_MAX)
		{
			error = EMLINK;
			goto cleanup;
		}

		/*
		 * If ".." must be changed (i.e. the directory gets a new
		 * parent) then the source directory must not be in the
		 * directory hierarchy above the target, as this would orphan
		 * everything below the source directory.
		 */

		newparent = topar_ip->i_number;
		/* RENAME_LOCK(topar_ip->i_ipmnt); */
		if (error = jfs_checkpath(from_ip, frompar_ip, topar_ip, vfst))
			goto cleanup;
	}

	/*
	 * Check for write-protected media
	 */
	if (isReadOnly(topar_ip))
	{
		error = EROFS;
		goto cleanup;
	}

// BEGIN F226941
	/*
	 * Check to see if there is room in the destination directory
	 */
	blocks_moving = doingdirectory ? DASDUSED(&from_ip->i_DASD) :
					 from_ip->i_nblocks;

	if ((topar_ip != frompar_ip) && (topar_ip->i_dasdlim) &&
	    over_limit(topar_ip->i_dasdlim, blocks_moving, start_here))
	{
		error = ERROR_DISK_FULL;
		goto cleanup;
	}

	orig_fromblocks = frompar_ip->i_nblocks;
	if (topar_ip != frompar_ip)
		orig_toblocks = topar_ip->i_nblocks;
// END F226941

	/*
	 * Perform rename
	 */
	txBegin(topar_ip->i_ipmnt, &tid, 0);

	dname.name = to_name;
	dname.namlen = UniStrlen (to_name);

	if (to_ip != NULL)
	{
		/*
		 * We change the case of the name in place.
		 */
		ino = to_ip->i_number;

		if (error = dtChangeCase(tid, frompar_ip, &dname, &ino,
					 JFS_RENAME))
			goto cleanup;

		imark(topar_ip, ICHG|IUPD|IFSYNC);
	}
	else
	{
		/*
		 * We already know the destination does not exist, so link
		 * source inode as destination.
		 */
		if (error = dtSearch(topar_ip, &dname, &ino, &btstack,
				JFS_CREATE))
		{
			/*
			 * Problem, can't find where to put this guy, or it
			 * already exists!!
			 */
			goto cleanup;
		}

		ino = from_ip->i_number;
		if (error = dtInsert(tid, topar_ip, &dname, &ino, &btstack))
		{
			/*
			 * Failed adding source to destination parent
			 */
			goto cleanup;
		}

		imark(topar_ip, ICHG|IUPD|IFSYNC);

		/* Insert entry for the new file to name cache */
		ncEnter(topar_ip->i_ipimap, topar_ip->i_number, &dname,
			from_ip->i_number, NULL);

		/* Remove source name from source parent and name cache */
		dname.name = from_name;
		dname.namlen = UniStrlen (from_name);
		ino = from_ip->i_number;

		ncDelete(frompar_ip->i_ipimap, frompar_ip->i_number, &dname);

		if (error = dtDelete(tid, frompar_ip, &dname, &ino, JFS_REMOVE))
		{
			/*
		 	* Another unexpected error -- the original file does not
		 	* appear to exist ...
		 	*/
			assert(0);
		}
		imark(frompar_ip, ICHG|IUPD|IFSYNC);

		/*
		 * If this is a directory we need to update the ".." i-number
		 * in the inode.  Also, there is now one fewer ".." referencing
		 * the source parent directory.  Decrement the link count to
		 * the source parent directory for this.
		 */
		if (doingdirectory && newparent)
		{
			/* linelock header of dtree root (containing idotdot)
		 	*/
			tlck = txLock(tid, from_ip,
				      (jbuf_t *)&from_ip->i_bxflag,
				      tlckDTREE|tlckBTROOT);
			dtlck = (dtlock_t *)&tlck->lock;
			ASSERT(dtlck->index == 0);
			lv = (lv_t *)&dtlck->lv[0];
			lv->offset = 0;
			lv->length = 1;
			dtlck->index++;

			((dtroot_t *) &from_ip->i_btroot)->header.idotdot =
					newparent;

			frompar_ip->i_nlink--;
			topar_ip->i_nlink++;
			/* RENAME_UNLOCK(topar_ip->i_ipmnt); */
		}

		/* Set or reset INEWNAME flag as appropriate */
		from_ip->i_mode = (from_ip->i_mode & ~INEWNAME) |
				  (flags & INEWNAME);
		imark(from_ip, ICHG|IFSYNC);			// D231252
		txList[txCount++] = from_ip;			// D231252
	}

// BEGIN F226941
	/*
	 * Modify dasd usage for source and destination directories.
	 * Number of blocks for object being moved didn't change.
	 */
	if (frompar_ip == topar_ip)
	{
		DLIM_UPDATE(tid, frompar_ip, frompar_ip->i_nblocks
			    - orig_fromblocks);
	}
	else
	{
		DLIM_UPDATE(tid, topar_ip, topar_ip->i_nblocks +
			    blocks_moving - orig_toblocks);
		DLIM_UPDATE(tid, frompar_ip, frompar_ip->i_nblocks -
			    (orig_fromblocks + blocks_moving));
	}

// BEGIN D233382
#ifdef _JFS_FASTDASD
	if (from_dlim->num_limits)
	{
		/*
		 * Let's remove any ancestor directories at the beginning of
		 * the lock list that aren't directly involved in the rename.
		 * i_dasdlim will be non-zero for frompar_ip & topar_ip
		 */
		for(i = 0; i < num_locks; i++)
		{
			if (lock_list[i]->i_dasdlim)
			{
				first_locked = i;
				break;
			}
			IWRITE_UNLOCK(lock_list[i]);
		}
		ASSERT(i < num_locks);	// We shouldn't have unlocked them all.
	}
#endif /* _JFS_FASTDASD */
// END D233382

#ifndef _JFS_FASTDASD						// D233382
	if ((from_dlim->flag & DLIM_LOGGED) || (to_dlim->flag & DLIM_LOGGED))
	{
		error = txCommit(tid, num_locks, lock_list, 0);
		from_dlim->flag &= ~DLIM_LOGGED;
		to_dlim->flag &= ~DLIM_LOGGED;
	}
	else
#endif /* _JFS_FASTDASD */					// D233382
// END F226941

	{
		if (frompar_ip != topar_ip)
		{
			/*
			 * Different parent, so we need to add the other parent
			 * to our transaction list
			 */
			txList[txCount++] = topar_ip;
		}

		/* Add source parent directory to transaction list */
		txList[txCount++] = frompar_ip;

		assert(txCount <= 4);
		error = txCommit(tid, txCount, txList, 0);
	}

cleanup:
	/*
	 * Come to this label when all locks have been taken and ready to leave.
	 * error should be set to return value.
	 */
	if (tid != -1) txEnd(tid);

// BEGIN F226941
	if (from_dlim->num_limits)
	{
		frompar_ip->i_dasdlim->flag &= ~DLIM_DIRS_LOCKED;
		frompar_ip->i_dasdlim = 0;
		if (frompar_ip != topar_ip)
		{
			topar_ip->i_dasdlim->flag &= ~DLIM_DIRS_LOCKED;
			topar_ip->i_dasdlim = 0;
		}

#ifdef _JFS_FASTDASD
		for(i = first_locked; i < num_locks; i++)	// D233382
#else
		for(i = 0; i < num_locks; i++)
#endif
			IWRITE_UNLOCK(lock_list[i]);

		if (lock_list != lock_arr)
			xmfree((void *)lock_list, kernel_heap);
	}
	else
// END F226941
	{
		IWRITE_UNLOCK(from_ip);

		IWRITE_UNLOCK(frompar_ip);
		if (frompar_ip != topar_ip)
		{
			IWRITE_UNLOCK(topar_ip);
		}
	}


abortit:
	/*
	 * Come to this label when none of the locks have been taken and ready
	 * to leave.  error should be set to return value.
	 */
	if (to_ip && got_to)
	{
		ICACHE_LOCK();
		iput(to_ip, vfst);
		ICACHE_UNLOCK();
	}
	if (got_from)
	{
		ICACHE_LOCK();
		iput(from_ip, vfsf);
		ICACHE_UNLOCK();
	}

	RENAME_UNLOCK(ipmnt);

	return error;
}
