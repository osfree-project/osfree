/* $Id: jfs_finfo.c,v 1.1 2000/04/21 10:58:09 ktk Exp $ */

static char *SCCSID = "@(#)1.24  11/1/99 13:09:05 src/jfs/ifs/jfs_finfo.c, sysjfs, w45.fs32, fixbld";
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
 */

/*
 * Change History :
 *
 */

/*
 * File Status Information for the Journalled File System
 *
 * Overall Design --
 *
 * Format:
 *
 *   OS/2 provides for three status retrieval requests (4 when you include
 *   the "return all EAs request") and two status modification requests.
 *
 *   The relevant structures for querying and modifying file status
 *   information are:
 *
 *   FILESTATUS -- basic file information, such as dates and times.
 *   FILESTATUS2 -- same as FILESTATUS, but also includes size of EA list
 *   FEALIST -- the extended attribute list structure on disk
 *
 * Operations:
 *
 *   There are four OS/2 functions for querying and modifying file status
 *   information.  They are DosQueryFileInfo, DosQueryPathInfo, DosSetFileInfo
 *   and DosSetPathInfo.  These four functions are converted by the IFSM into
 *   the two function fs_fileinfo() and fs_pathinfo().  These two functions
 *   in turn call the specific JFS functions for querying and setting the
 *   various pieces of file status information.
 *
 *   FILESTATUS structure:
 *
 *     jfs_GetLevel1Info() and jfs_SetLevel1Info() are used to get and set
 *     the most basic of file information.
 *
 *   FILESTATUS2 structure:
 *
 *     jfs_GetLevel2Info() will return all of the information returned by
 *     jfs_GetLevel1Info() with the addition of a cbList field which gives
 *     the number of bytes of EA data which are associated with the file.
 *
 *     There is no mechanism for setting the FILESTATUS2 information since
 *     the value of cbList is controlled by the actual size of the EA data.
 *
 *   FEALIST data:
 *
 *     jfs_GetLevel3Info() and jfs_SetLevel2Info() (yes, this is OS/2
 *     stupidity and I picked these names on purpose to drill into my head
 *     that the level numbers DON'T MATCH!  I probably need to rename these
 *     functions more descriptively now that I am unconfused ...) are used
 *     to get and set (respectively) EA data.
 *
 *     There is an additional "level 4" DosQueryFileInfo/DosQueryPathInfo
 *     request which is special-cased thru jfs_GetLevel3Info().  This level
 *     is not externally documented ...
 *
 * Storage:
 *
 *     The information returned by these calls is either stored in the
 *     i-node (FILESTATUS and FILESYSTEM2) or as a single extent (for EA
 *     lists) on disk.
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_LONGLONG

#define _cdecl __cdecl

#include <os2.h>

#include "jfs_os2.h"
#include "jfs_inode.h"
#include "jfs_dirent.h"
#include "jfs_cntl.h"
#include "jfs_debug.h"
#include "jfs_txnmgr.h"
#include "jfs_ea.h"
#include "jfs_dasdlim.h"					// F226941
#include "jfs_proto.h"
#include "jfs_lock.h"

extern void Net_To_Dos_Date (ULONG, PFDATE, PFTIME);
extern uint32 Dos_To_Net_Date (FDATE, FTIME);
extern int32 Validate_Dos_Date_Time(FDATE, FTIME);

/*
 * jfs_setLevel1Info -- modify the FILESTATUS data for an inode
 */

int32
jfs_SetLevel1Info (
        inode_t         *ip,
        FILESTATUS      *fsts,
        int32           size)
{
        int32   rc;

        /* check required buffer size */
        /* this is hardcoded to the size of FILESTATUS, but due to padding */
        /* we need to limit to 0x16 */
        if (size < 0x16)
                return ERROR_INSUFFICIENT_BUFFER;

	rc = Validate_Dos_Date_Time(fsts->fdateCreation, fsts->ftimeCreation);
	if (rc)
		return rc;
	rc = Validate_Dos_Date_Time(fsts->fdateLastAccess, fsts->ftimeLastAccess);
	if (rc)
		return rc;
	rc = Validate_Dos_Date_Time(fsts->fdateLastWrite, fsts->ftimeLastWrite);
	if (rc)
		return rc;

	/* If inode is a directory, FILE_DIRECTORY may be set, but will be
	 * ignored.
	 */
	if ((ip->i_mode & IFMT) == IFDIR)
		fsts->attrFile &= ~FILE_DIRECTORY;

	/* Ignore FILE_NEWNAME
	 */
	fsts->attrFile &= ~FILE_NEWNAME;

	if( ((fsts->attrFile << ATTRSHIFT) & ~IRASH ) != 0 ) 
		return ERROR_INVALID_PARAMETER;

        IWRITE_LOCK(ip);

        /* set time stamps */
        if (*(USHORT *)&fsts->fdateCreation ||
            *(USHORT *)&fsts->ftimeCreation)
                ip->i_otime.tv_sec = Dos_To_Net_Date (fsts->fdateCreation,
                                                      fsts->ftimeCreation);

        if (*(USHORT *)&fsts->fdateLastAccess ||
            *(USHORT *)&fsts->ftimeLastAccess)
                ip->i_atime.tv_sec = Dos_To_Net_Date (fsts->fdateLastAccess,
                                                      fsts->ftimeLastAccess);

        if (*(USHORT *)&fsts->fdateLastWrite ||
            *(USHORT *)&fsts->ftimeLastWrite)
                ip->i_mtime.tv_sec = Dos_To_Net_Date (fsts->fdateLastWrite,
                                                      fsts->ftimeLastWrite);

        /* set file attributes */
        ip->i_mode = (ip->i_mode & ~IRASH) |
                ((fsts->attrFile << ATTRSHIFT) & IRASH);

        /* set flag here instead of calling imark() so dates aren't changed */
        ip->i_flag |= ICHG;

        /* Commit changes */

        IWRITE_UNLOCK(ip);

        return rc;
}

/*
 * jfs_setLevel11Info -- modify the FILESTATUS3L data for an inode
 */

int32
jfs_SetLevel11Info (
        inode_t         *ip,
        FILESTATUS3L    *fst3L,
        int32           size)
{
        inode_t *iplist[1];
        int32   rc;

        /* check required buffer size */
        if (size < sizeof(FILESTATUS3L))
                return ERROR_INSUFFICIENT_BUFFER;

	rc = Validate_Dos_Date_Time(fst3L->fdateCreation, fst3L->ftimeCreation);
	if (rc)
		return rc;
	rc = Validate_Dos_Date_Time(fst3L->fdateLastAccess, fst3L->ftimeLastAccess);
	if (rc)
		return rc;
	rc = Validate_Dos_Date_Time(fst3L->fdateLastWrite, fst3L->ftimeLastWrite);
	if (rc)
		return rc;

	/* If inode is a directory, FILE_DIRECTORY may be set, but will be
	 * ignored.
	 */
	if ((ip->i_mode & IFMT) == IFDIR)
		fst3L->attrFile &= ~FILE_DIRECTORY;

	/* Ignore FILE_NEWNAME
	 */
	fst3L->attrFile &= ~FILE_NEWNAME;

	if( ((fst3L->attrFile << ATTRSHIFT) & ~IRASH ) != 0 ) 
		return ERROR_INVALID_PARAMETER;

        IWRITE_LOCK(ip);

        /* set time stamps */
        if (*(USHORT *)&fst3L->fdateCreation ||
            *(USHORT *)&fst3L->ftimeCreation)
                ip->i_otime.tv_sec = Dos_To_Net_Date (fst3L->fdateCreation,
                                                      fst3L->ftimeCreation);

        if (*(USHORT *)&fst3L->fdateLastAccess ||
            *(USHORT *)&fst3L->ftimeLastAccess)
                ip->i_atime.tv_sec = Dos_To_Net_Date (fst3L->fdateLastAccess,
                                                      fst3L->ftimeLastAccess);

        if (*(USHORT *)&fst3L->fdateLastWrite ||
            *(USHORT *)&fst3L->ftimeLastWrite)
                ip->i_mtime.tv_sec = Dos_To_Net_Date (fst3L->fdateLastWrite,
                                                      fst3L->ftimeLastWrite);

        /* set file attributes */
        ip->i_mode = (ip->i_mode & ~IRASH) |
                ((fst3L->attrFile << ATTRSHIFT) & IRASH);

        /* set flag here instead of calling imark() so dates aren't changed */
        ip->i_flag |= ICHG;

        /* Commit changes */
        iplist[0] = ip;

        IWRITE_UNLOCK(ip);

        return rc;
}

/*
 * jfs_GetLevel1Info -- return the FILESTATUS information for an inode
 */

int32
jfs_GetLevel1Info (
        inode_t         *ip,
        FILESTATUS      *fsts,
        int32           size)
{
          /* this is hardcoded to the size of FILESTATUS, but due to padding */
          /* we need to limit to 0x16 */
          if (size < 0x16)
                return ERROR_BUFFER_OVERFLOW;

        /* get time stamps */
        Net_To_Dos_Date(ip->i_otime.tv_sec,
                &fsts->fdateCreation, &fsts->ftimeCreation);
        Net_To_Dos_Date(ip->i_atime.tv_sec,
                &fsts->fdateLastAccess, &fsts->ftimeLastAccess);
        Net_To_Dos_Date(ip->i_mtime.tv_sec,
                &fsts->fdateLastWrite, &fsts->ftimeLastWrite);

        /* get file size */
	if (ip->i_size > 0x7fffffff)
	{
		if ((ip->i_mode & IFMT) == IFDIR)
			fsts->cbFile = 0;
		else
        		fsts->cbFile = 1;

        	fsts->cbFileAlloc = 1;
	}
	else
	{
		if ((ip->i_mode & IFMT) == IFDIR)
			fsts->cbFile = 0;
		else
        		fsts->cbFile = ip->i_size;

        	fsts->cbFileAlloc = ip->i_nblocks << ip->i_ipmnt->i_l2bsize;
	}

        /* get file attributes */
        fsts->attrFile = (ip->i_mode >> ATTRSHIFT) & ~FILE_NEWNAME;

        return 0;
}

/*
 * jfs_GetLevel11Info -- return the FILESTATUS3L information for an inode
 */

int32
jfs_GetLevel11Info (
        inode_t         *ip,
        FILESTATUS3L    *fst3L,
        int32           size)
{
          if (size < sizeof(FILESTATUS3L))
                return ERROR_BUFFER_OVERFLOW;

        /* get time stamps */
        Net_To_Dos_Date(ip->i_otime.tv_sec,
                &fst3L->fdateCreation, &fst3L->ftimeCreation);
        Net_To_Dos_Date(ip->i_atime.tv_sec,
                &fst3L->fdateLastAccess, &fst3L->ftimeLastAccess);
        Net_To_Dos_Date(ip->i_mtime.tv_sec,
                &fst3L->fdateLastWrite, &fst3L->ftimeLastWrite);

        /* get file size */
	if ((ip->i_mode & IFMT) == IFDIR)
		fst3L->cbFile = 0;
	else
        	fst3L->cbFile = ip->i_size;

        fst3L->cbFileAlloc = ip->i_nblocks << ip->i_ipmnt->i_l2bsize;

        /* get file attributes */
        fst3L->attrFile = (ip->i_mode >> ATTRSHIFT) & ~FILE_NEWNAME;

        return 0;
}

/*
 * jfs_SetLevel2Info -- modify extended attributes for an inode
 */

int32
jfs_SetLevel2Info (
        inode_t         *ip,
        EAOP            *peaop,
        int32           size)
{
	ULONG	cbList;
        FEALIST * pfeal;
        FEALIST * old;
        FEALIST * new;
        FEALIST *allocp;
        dxd_t   dxd;
	int64	orig_nblocks;					// F226941
        int     rc;
        int     no_eas = 1;
        int     newSize;
        int32   allocsize = 0;
        int32   tid;

        /* validate the EAOP structure to insure all of the fields exist. */
        if (size < sizeof *peaop)
                return ERROR_INSUFFICIENT_BUFFER;

	/* Get the lenghth of the attribute list
	 */
	rc = KernCopyIn(&cbList, &peaop->fpFEAList->cbList, sizeof(cbList));
	if (rc != NO_ERROR)
		return rc;

        /* Get all of the buffer we will need in one call to prevent deadlocks
         * We can't determine the resulting size until we have both lists in
         * memory to look at, but we can't do that until we have the memory
         * allocated.  So, we know the resulting list can't be bigger than the
         * original list and the one to add, and we need the space to read the
         * original list and the one to add into memory.
         */

	/* If i_ea.flag == 0, size is meaningless.  In this case, we'll set
	 * it to zero.
	 */
	if (ip->i_ea.flag == 0)
		ip->i_ea.size = 0;

        allocsize = (ip->i_ea.size + cbList) << 1;
        allocp = jfs_EABufAlloc(allocsize);
        if (allocp == NULL)
                return ERROR_EA_LIST_TOO_LONG;

        old = (ip->i_ea.size == 0) ? NULL : allocp;
        pfeal = (FEALIST *)((char *)allocp + ip->i_ea.size);
        new = (FEALIST *)((char *)pfeal + cbList);

        /* Copy the FEAList from the user's address space into pinned memory.
         * We will also convert any lowercase letters to uppercase if needed.
         */
        copyin((char *)peaop->fpFEAList, (char *)pfeal, cbList);

	if (rc = jfs_ValidateFEAList(pfeal, &peaop->oError))
	{
		jfs_EABufFree((char *) allocp, allocsize);
                return rc;
	}

        if (rc = jfs_ConvertFEAListNames(pfeal, &peaop->oError))
	{
		jfs_EABufFree((char *) allocp, allocsize);
                return rc;
	}

        /* now go get the EA into a buffer
         */
        if (ip->i_ea.size)
        {
                if (rc = jfs_ReadEA (ip, old))
                {
                        jfs_EABufFree((char *) allocp, allocsize);
                        return rc;
                }
        }

	orig_nblocks = ip->i_nblocks;				// F226941

        jfs_MergeFEALists (new, old, pfeal);

        /* write the updated FEALIST to disk
         */
        rc = jfs_WriteEA (ip, new, &dxd);

	if (!rc)
	{
		if (ip->i_ea.flag & DXD_EXTENT)
			/* free old EA pages from cache */
			bmExtentInvalidate(ip, addressDXD(&ip->i_ea),
				lengthDXD(&ip->i_ea));

// BEGIN D230860
#ifndef _JFS_FASTDASD						// D233382
		/* BUGBUG - ip->i_nblocks has not been updated yet! */
		if (ip->i_dasdlim && (ip->i_nblocks != orig_nblocks))
		{
			/*
			 * All inodes must be locked before beginning
			 * transaction.
			 */
			if ((ip->i_dasdlim->flag & DLIM_DIRS_LOCKED) == 0)
				dasd_write_lock(0, ip->i_dasdlim);

			txBegin (ip->i_ipmnt, &tid, 0);
		}
		else
#endif /* _JFS_FASTDASD */					// D233382 
			tid = 0;
// END D230860

        	/* record the EA change and update the inode */

        	txEA (tid, ip, &ip->i_ea, &dxd);

        	ip->i_nblocks = ip->i_nblocks + lengthDXD(&dxd) -
				lengthDXD(&ip->i_ea);
        	ip->i_ea = dxd;

        	imark (ip, ICHG);

		DLIM_UPDATE(tid, ip, ip->i_nblocks - orig_nblocks); // F226941
#ifndef _JFS_FASTDASD						// D233382
		if (tid)					// D230860
		{						// D230860
			rc = DLIM_TXCOMMIT(tid, ip, 0);		// F226941
			txEnd(tid);
		}						// D230860
#endif /* _JFS_FASTDASD */					// D233382
	}

        jfs_EABufFree((char *)allocp, allocsize);

        return rc;
}

/*
 * jfs_GetLevel2Info -- return FILESTATUS2 information to the caller
 */

int32
jfs_GetLevel2Info (
        inode_t         *ip,
        FILESTATUS2     *fst2,
        int32           size)
{
        int     rc;

        /* check the buffer size */
        /* this is hardcoded to the size of FILESTATUS2, but due to padding */
        /* we need to limit to 0x1a */
        if (size < 0x1A)
                return ERROR_BUFFER_OVERFLOW;

        /* FILESTATUS2 info is the same as FILESTATUS info except for the
         * addition of cbList at the end of the structure.
         */
        if (rc = jfs_GetLevel1Info (ip, (FILESTATUS *) fst2, size))
                return rc;

        fst2->cbList = ip->i_ea.flag ? ip->i_ea.size : sizeof(ULONG);

        return 0;
}

/*
 * jfs_GetLevel12Info -- return FILESTATUS2 information to the caller
 */

int32
jfs_GetLevel12Info (
        inode_t         *ip,
        FILESTATUS4L    *fst4L,
        int32           size)
{
        int     rc;

        /* check the buffer size */
        if (size < sizeof(FILESTATUS4L))
                return ERROR_BUFFER_OVERFLOW;

        /* FILESTATUS4L info is the same as FILESTATUS3L info except for the
         * addition of cbList at the end of the structure.
         */
        if (rc = jfs_GetLevel11Info (ip, (FILESTATUS3L *) fst4L, size))
                return rc;

        fst4L->cbList = ip->i_ea.flag ? ip->i_ea.size : 0;

        return 0;
}

/*
 * jfs_GetLevel3Info -- return requested list of FEA elements
 *
 *	pgeal & pfeal are user buffers.  We need to make sure we don't fault.
 *	(In other words, access via KernCopyIn & KernCopyOut.)
 */
int32
jfs_GetLevel3Info (
        inode_t         *ip,
        GEALIST         *pgeal,
        FEALIST         *pfeal,
        ULONG           *oError)
{
	ULONG	cbList;
        FEALIST * old = 0;
        int16   rc;
        GEALIST *pgeal_copy = NULL;
        FEALIST *allocp;
        int32   allocsize = 0;
        int     no_eas = 1;

	/* If i_ea.flag == 0, size is meaningless.  In this case, we'll set
	 * it to zero.
	 */
	if (ip->i_ea.flag == 0)
		ip->i_ea.size = 0;

        allocsize = (ip->i_ea.size) ? ip->i_ea.size : sizeof(FEALIST);

        /* a GEALIST will be used (if present) to return only specific
         * EAs for this file.  if there is no GEALIST, all EAs will be
         * returned to the caller.
         */
        if (pgeal)
        {
                /* Copy the GEAList from the user's address space into pinned
                 * memory.  We will also convert any lowercase letters to
                 * uppercase if needed.
                 */
		rc = KernCopyIn(&cbList, &pgeal->cbList, sizeof (cbList));
		if (rc != NO_ERROR)
			return rc;
                allocsize += cbList;
        	allocp = jfs_EABufAlloc(allocsize);
                pgeal_copy = (GEALIST *)allocp;
                rc = KernCopyIn((char *)pgeal_copy, (char *)pgeal, cbList);
		if (rc == NO_ERROR)
                	rc = jfs_ConvertGEAListNames(pgeal_copy, oError);
		if (rc != NO_ERROR)
                {
                        jfs_EABufFree((char *) allocp, allocsize);
                        return rc;
                }
		old = (FEALIST *)((char *)allocp + cbList);
        }
	else
        	old = allocp = jfs_EABufAlloc(allocsize);

        /* locate the storage for the FEALIST which is being scanned and
         * copy the requested extended attributes to the user's buffer.
         */
        if (ip->i_ea.size)
        {
                if (rc = jfs_ReadEA (ip, old))
                {
                        jfs_EABufFree((char *) allocp, allocsize);
                        return rc;
                }
        }
        else
        {
                old->cbList = sizeof old->cbList;
        }

        rc = jfs_GEAListToFEAList(old, pfeal, pgeal_copy, oError);

        jfs_EABufFree ((char *) allocp, allocsize);

        return rc;
}
