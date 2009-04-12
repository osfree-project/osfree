/* $Id: jfs_readdir.c,v 1.1 2000/04/21 10:58:15 ktk Exp $ */

static char *SCCSID = "@(#)1.26.1.2  12/2/99 11:13:36 src/jfs/ifs/jfs_readdir.c, sysjfs, w45.fs32, fixbld";
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
 * Module: jfs_readdir.c
 */

/*
 * Change History :
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#define INCL_LONGLONG
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */

#include "mmph.h"
#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#ifdef _JFS_OS2
#include "jfs_dirent.h"						// F226941
#endif /* _JFS_OS2 */
#include "jfs_dtree.h"
#include "jfs_ea.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

#include <uni_inln.h>
int32 jfs_strfromUCS(char *, UniChar *, int32, int32);

extern	pool_t	*dirent_pool;

/* forward reference
 */
int32	get_fileinfo(struct inode *, char **, uint32,
			 UniChar *, uint32, union havattr, uint32, EAOP *,
			 unsigned short);

/* external references
 */
void	Net_To_Dos_Date (uint32, uint16 *, uint16 *);

int32 jfs_GetLevel3Info(inode_t *ip, GEALIST *pgeal, FEALIST *pfeal,
		ULONG *oError);

/*
 * NAME:	readdir
 *
 * FUNCTION:	read directory according to specifications
 *		in directory search structure
 *
 * PARAMETER:
 *
 * RETURN:	EINVAL	- if not a directory
 *		errors from subroutines
 *
 * note:
 * N.B. directory file offset encodes (directory page number,
 * entry index number), and shold NOT be interpreted/modified
 * by caller (lseek()) except that intial offset set to 0.
 *
 * no guarantees can be made that the exact offset
 * requested can be found if directory has been updated
 * by other threads between consecutive readdir()s.
 * transfer length of zero signals start offset beyond eof.
 *
 * unused space in the directory are not returned to the user,
 * i.e., more than requested size may have to be read
 * from directory to fill the user's buffer.
 */
readdir(
	struct vnode	*dvp,		/* inode of directory being read */
	struct fsfd	*fsfp,		/* directory search information */
	char 		*ubuf,		/* user's data area */
	uint32		ubytes,		/* size of user's data area */
	uint32		*matchcnt,	/* count of entries returned */
	uint32		level,		/* level of output struct */
	uint32		flags,		/* offsets needed in output? */
	EAOP		*eaopp,		/* pointer to EAOP */
	struct ucred	*crp)
{	
	int32 	rc = 0;	
	int32 	ReturnCode = NO_ERROR;	
	inode_t	*dip;			/* directory inode */
	inode_t	*ip;			/* object inode */
	uint32	matches;		/* output matches found */
	uint32	dtmatches;		/* matches found per dtFind call */
	uint32	position;		/* offsets in output */
	uint32	count;			/* output buffer count */
	int32 	tbytes;			/* byte count in dirent buffer */
	struct dirent *dbuf;		/* dirent buffer */
	struct dirent *dbufp;		/* dirent buffer */
	uint32 ffhdsize;		/* size of ffbuf header */
	component_t lastmatch;		/* pointer to last matching entry */
	char *ffbuf;			/* output buffer pointer */
	char *nxbuf;			/* output buffer pointer */
	char *bufp;			/* output buffer pointer */
	
   MMPHPrereaddir();      /* MMPH Performance Hook */

	/* set state from search structure
	 */
	dip = VP2IP(dvp);
	position = flags & FF_GETPOS;

	/* validate request */
	if (ubytes == 0)
	{
		rc = EINVAL;
		goto readdir_Exit;
	}

 	/* continuous read of empty directory ? */
	if (fsfp->fsd_offset == -1)
	{
		rc = ERROR_NO_MORE_FILES;
		goto readdir_Exit;
	}

	dbuf = (struct dirent *)allocpool(dirent_pool, 0);	// D228565
	if (dbuf == 0)						// D228565
	{
		rc = ENOMEM;
		goto readdir_Exit;
	}

        /* set up variable to manipulate output buffer pointers
         * based on level.
         */
        if (level == 1)
                ffhdsize = FFBUFHD;
        else if (level == 11)
                ffhdsize = FFBUFHD3L;
        else if (level < 11)
                ffhdsize = FFBUFHD2;
        else
                ffhdsize = FFBUFHD4L;
	if (position)
		ffhdsize += sizeof(uint32);

        ffbuf = ubuf;
	count = 0;
	matches = *matchcnt;
	*matchcnt = 0;

	while ((*matchcnt < matches) && (rc == 0))
	{
		IREAD_LOCK(dip);

		/* directory became void when last link was removed */
		if ((dip->i_nlink == 0) || ((dip->i_mode & IFMT) != IFDIR))
		{
			IREAD_UNLOCK(dip);
			freepool(dirent_pool, (caddr_t *)dbuf);
			rc = ENOTDIR;
			goto readdir_Exit;
		}

		/* fill a directory buffer.
	 	* read on-disk structure (struct ldtentry_t) and
	 	* translate into readdir() structure (struct dirent).
	 	*/
		tbytes = 0;
		dtmatches = matches - *matchcnt;
		dbufp = dbuf;					// D228565
		rc = dtFind(dip, &fsfp->fsd_pattern, fsfp->fsd_lastmatch,
			&fsfp->fsd_offset, &dtmatches, PSIZE, &tbytes, dbufp);
		IREAD_UNLOCK(dip);
		if (rc)
		{
			freepool(dirent_pool, (caddr_t *)dbuf);
			goto readdir_Exit;
		}

        	/* copy translate buffer to user FileFindBuf buffer */

		while ((*matchcnt < matches) && (ReturnCode == NO_ERROR))
        	{
			uint32 namlen;

			/* translation buffer empty? */
			if (tbytes == 0)
				break;

			/* get size of next name */
			namlen = dbufp->d_namlen;

			/* user buffer full?
		 	* the +1 here is to allow for the null character
		 	* terminating the name string.
		 	*/
			if ((count + ffhdsize + namlen + 1) > ubytes)
			{
				rc = ERROR_BUFFER_OVERFLOW;
				break;
			}

			/* get the inode for the file
		 	*/
			ICACHE_LOCK();
			rc = iget(dvp->v_vfsp, dbufp->d_ino, &ip, 0);
			ICACHE_UNLOCK();

			if (rc)
				goto try_next;

			nxbuf = ffbuf;


			/* fill in file search info for files that have
			 * the proper attributes; ignore others.
			 */
			rc = get_fileinfo(ip, &nxbuf, ubytes, dbufp->d_name,
					  namlen, fsfp->fsd_havattr, level,
					  eaopp, flags);

			if ((rc == ERROR_BUFFER_OVERFLOW) && (*matchcnt == 0) &&
			    ((level == FIL_QUERYEASFROMLIST) ||
			     (level == FIL_QUERYEASFROMLISTL)))
			{
				/* Can't fit EA in buffer, try without
				 * getting EA
				 */

				if (level == FIL_QUERYEASFROMLIST)
					level = FIL_QUERYEASIZE;
				else
					level = FIL_QUERYEASIZEL;

				ReturnCode = ERROR_EAS_DIDNT_FIT;
				rc = get_fileinfo(ip, &nxbuf, ubytes,
						  dbufp->d_name, namlen,
						  fsfp->fsd_havattr, level,
						  eaopp, flags);
			}
			/* release the inode */
			jfs_rele(IP2VP(ip));

			if (rc == 0)
			{
				/* set offset if requested */
				if (position)
				{
					rc = KernCopyOut(ffbuf,
							 &dbufp->d_offset,
							 sizeof(int32));
					if (rc)
					{
						/* This is very unlikely to
						 * happen! */
						ASSERT(0);
						break;
					}
				}

				/* update output buffer count */
				count += nxbuf - ffbuf;

				/* move to next entry in output buffer */
				ffbuf = nxbuf;

				/* update match count */
				*matchcnt += 1;
			}
	 		else if (rc != -1)
	 			break;
		
try_next:
			/* rc == -1 indicates no attribute match,
			 * just keep going.
			 */
			rc = 0;

			/* save name for next call setup */
			lastmatch.name = dbufp->d_name;
			lastmatch.namlen = namlen;

			/* update dirent buffer count */
			tbytes -= dbufp->d_reclen;

			/* move to next entry in dirent buffer */
			dbufp = (struct dirent *)
					((caddr_t)dbufp + dbufp->d_reclen);
		}
		/* We don't want to continue if ReturnCode = ERROR_EAS_DIDNT_FIT
		 */
		if (rc == 0)
			rc = ReturnCode;

		/* set return code for end of directory with no matches */
		if (fsfp->fsd_offset == -1)
			rc = ERROR_NO_MORE_FILES;
		else if ((rc == 0) || (rc == ERROR_EAS_DIDNT_FIT))
		{
			/* save last matching name for next call */
			UniStrncpy(fsfp->fsd_lastmatch,lastmatch.name,
				   lastmatch.namlen);
			fsfp->fsd_lastmatch[lastmatch.namlen] = '\0';
		}
	}

	/* claim success if we return any entries */
	if (*matchcnt != 0)
		rc = ReturnCode;

	freepool(dirent_pool, (caddr_t *)dbuf);

readdir_Exit:

   MMPHPostreaddir();     /* MMPH Performance Hook */
	return rc;
}

int32
get_fileinfo(struct inode	*ip,
	     char		**ubuf,
	     uint32		bufsize,
	     UniChar		*name,
	     uint32		namlen,
	     union  havattr	attr,
	     uint32		level,
	     EAOP		*eaopp,
	     unsigned short	flags)
{
	char  c;
	int32 rc = 0;
	int32 mode;
	FEALIST *feal;
	int32 fixed_size;

/* This is the file name in the format dictated by FileFindBuf.  It is
 * split out to allow the same mapping for copying into the various formats.
 */
	struct _namestr {
		uint8	string_len;
		char	file_name[1];
	} *namestr;

	struct FindBufFixed ffbuf, *pffbuf;

	pffbuf = (flags & FF_GETPOS) ?
		(struct FindBufFixed *)(*ubuf + sizeof(int32)) :
		(struct FindBufFixed *)*ubuf;

	/* assure that the information returned represents a consistent
	 * view of the inode by locking out updates to it.
	 */
	IREAD_LOCK(ip);

	/* attribute filtering:
	 * file must have attributes defined in mustHave;
	 * file must not have attributes not defined in mayHave.
	 */
	mode = ip->i_mode >> ATTRSHIFT;
	if (((mode & attr.mustHave) != attr.mustHave) ||
	   ((mode & (attr.mayHave|FILE_READONLY|FILE_ARCHIVED)) != mode))
	{
		IREAD_UNLOCK(ip);
		return -1;
	}

	/* copy the attributes and the name to the user's buffer
	 */
	Net_To_Dos_Date(ip->i_otime.tv_sec,
		(uint16 *)&ffbuf.fdateCreation,
		(uint16 *)&ffbuf.ftimeCreation);
	Net_To_Dos_Date(ip->i_atime.tv_sec,
		(uint16 *)&ffbuf.fdateLastAccess,
		(uint16 *)&ffbuf.ftimeLastAccess);
	Net_To_Dos_Date(ip->i_mtime.tv_sec,
		(uint16 *)&ffbuf.fdateLastWrite,
		(uint16 *)&ffbuf.ftimeLastWrite);

	if (level > FIL_QUERYEASFROMLIST)
	{
		if ((ip->i_mode & IFMT) == IFDIR)
			ffbuf.cbFile = 0;
		else
			ffbuf.cbFile = ip->i_size;

		ffbuf.cbFileAlloc = ip->i_nblocks << ip->i_ipmnt->i_l2bsize;
		ffbuf.attrFile = mode & ~FILE_NEWNAME;
		fixed_size = sizeof(ffbuf);
	}
	else
	{
		FILEFINDBUF *pffb = (FILEFINDBUF *)&ffbuf;
		if (ip->i_size > 0x7fffffff)
		{
			if ((ip->i_mode & IFMT) == IFDIR)
				pffb->cbFile = 0;
			else
				pffb->cbFile = 1;

			pffb->cbFileAlloc = 1;
		}
		else
		{
			if ((ip->i_mode & IFMT) == IFDIR)
				pffb->cbFile = 0;
			else
				pffb->cbFile = ip->i_size;

			pffb->cbFileAlloc =
				ip->i_nblocks << ip->i_ipmnt->i_l2bsize;
		}
		pffb->attrFile = mode & ~FILE_NEWNAME;
		fixed_size = 22;
	}

	if (KernCopyOut(pffbuf, &ffbuf, fixed_size))
	{
		IREAD_UNLOCK(ip);
		return ERROR_BUFFER_OVERFLOW;
	}

	switch (level)
	{
	case FIL_STANDARD:
		namestr = (struct _namestr *)
			  &((FILEFINDBUF *)pffbuf)->cchName;
		break;
	
	case FIL_STANDARDL:
		/* The -4 takes into account oNextEntryOffset */
		namestr = (struct _namestr *)
			  &((FILEFINDBUF3L *)((uint32)pffbuf-4))->cchName;
		break;
	
	case FIL_QUERYEASIZE:
		{
			FILEFINDBUF2 *f2b;
			ULONG	size;

			f2b = (FILEFINDBUF2 *)pffbuf;

			/* set EA list length */
			size = ip->i_ea.flag ? ip->i_ea.size : 0;
			if (KernCopyOut(&f2b->cbList, &size,
			   		sizeof(f2b->cbList)) != NO_ERROR)
			{
				IREAD_UNLOCK(ip);
				return ERROR_BUFFER_OVERFLOW;
			}
			/* address the name fields */
			namestr = (struct _namestr *)&f2b->cchName;
		}
		break;
	
	case FIL_QUERYEASIZEL:
		{
			FILEFINDBUF4L *f4L;
			ULONG	size;

			/* The -4 takes into account oNextEntryOffset */
			f4L = (FILEFINDBUF4L *)((uint32)pffbuf-4);

			/* set EA list length */
			size = ip->i_ea.flag ? ip->i_ea.size : 0;
			if (KernCopyOut(&f4L->cbList, &size,
			   		sizeof(f4L->cbList)) != NO_ERROR)
			{
				IREAD_UNLOCK(ip);
				return ERROR_BUFFER_OVERFLOW;
			}
			/* address the name fields */
			namestr = (struct _namestr *)&f4L->cchName;
		}
		break;
	
	case FIL_QUERYEASFROMLIST:
	case FIL_QUERYEASFROMLISTL:
		{
			ULONG cbList;
			if (level == FIL_QUERYEASFROMLIST)
				feal = (FEALIST *)((char *)pffbuf + 22);
			else	/* FIL_QUERYEASFROMLISTL */
				feal = (FEALIST *)((char *)pffbuf +
					sizeof(struct FindBufFixed));

			/* fill in EAs - only use up to amount of space not
			 * being used by the FixedBuffer stuff and the name
			 *
			 * We don't know for certain the length of the name
			 * until it has been converted from Unicode.  We will
			 * check for overflow again when we do the conversion.
			 */
			cbList = bufsize - (FFBUFHD + namlen + 1);
			if (KernCopyOut(&feal->cbList, &cbList, sizeof(cbList))
			    != NO_ERROR)
			{
				IREAD_UNLOCK(ip);
				return ERROR_BUFFER_OVERFLOW;
			}
			rc = jfs_GetLevel3Info(ip, eaopp->fpGEAList, feal,
				&(eaopp->oError));

			if (rc)
			{
				IREAD_UNLOCK(ip);
				return rc;
			}

			if (KernCopyIn(&cbList, &feal->cbList, sizeof(cbList))
			    != NO_ERROR)
			{
				/* Very unlikely */
				ASSERT(0);
				IREAD_UNLOCK(ip);
				return ERROR_BUFFER_OVERFLOW;
			}

			/* address the name fields */
			namestr = (struct _namestr *) ((char *)feal + cbList);
		}
		break;

	default:
		panic("Bad info level");
		return -1;
	}

	IREAD_UNLOCK(ip);

	/* If called by DOS process, convert name to upper case
	 */
	if ((attr.mayHave & FILE_NEWNAME) == 0)
		UniStrupr(name);

	/* copy entry name to user's buffer */
	namlen = jfs_strfromUCS(namestr->file_name, name,
				bufsize - ((uint32)namestr - (uint32)*ubuf)
					- sizeof(namestr->string_len), namlen);
	if (namlen == -1)
		return ERROR_BUFFER_OVERFLOW;

	rc = KernCopyOut(&namestr->string_len, &namlen,
			 sizeof(namestr->string_len));
	if (rc != NO_ERROR)
	{
		/* Very unlikely */
		ASSERT(0);
		return ERROR_BUFFER_OVERFLOW;
	}

	/* set pointer in output buffer */
	*ubuf = (char *)namestr + namlen + sizeof(namestr->string_len) + 1;

	return 0;
}
