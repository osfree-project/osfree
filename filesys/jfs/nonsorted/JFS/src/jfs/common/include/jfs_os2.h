/* $Id: jfs_os2.h,v 1.1.1.1 2003/05/21 13:35:54 pasha Exp $ */

/* static char *SCCSID = "@(#)1.9  9/13/99 14:54:41 src/jfs/common/include/jfs_os2.h, sysjfs, w45.fs32, fixbld";
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

#ifndef _H_JFS_OS2
#define _H_JFS_OS2

#include <infoseg.h>						// D230860

/* Include the fundamental JFS definitions. */
#include "jfs_types.h"

/* Include to get some AIX structures and definitions. */
#include "jfs_aixisms.h"

/* Include prototypes for utility functions. */

#ifndef _JFS_FSCK
#include "jfs_util.h"
#endif /* _JFS_FSCK */

#include <string.h>

/*
 * This file contains the definitions of the primary structures used at
 * the interface with the OS/2 Installable File System Mechanism (IFSM).
 * Included are the structure definitions for the file system dependent
 * pieces of the following IFS structures:
 *	Volume Parameter structure
 *	System File Table entry
 *	Current Directory structure
 *	File Search structure
 */

/* The size of these structures is dictated by the IFSM.  Defines for
 * the sizes can be found in fsd.h:
 *	VPDWORKAREASIZE (36 bytes)
 *	SFDWORKAREASIZE (30 bytes)
 *	CDDWORKAREASIZE ( 8 bytes)
 *	FSFSD_WORK_SIZE (24 bytes)
 */


/*	Volume parameter structure - JFS dependent part
 *
 * This structure provides a place to save a pointer to the vfs.
 * The vfs serves as an anchor for various information about an
 * instance of a JFS file system.
 */

struct vpsd {
	struct vfs	*vpd_vfs;	/* ptr to file system struct */
};


/*	System file table entry - JFS dependent part
 *
 * There is a system file table entry for each instance of each open
 * file in the system.  It is the primary means of communication between
 * the IFSM and the file system driver for operations on an open file.
 * Thus the SFT entry is similar to a vnode.  But it is different in that
 * there is only one vnode for an open file, while there are multiple SFT
 * entries for an open file.  For this reason, the JFS will use the SFT
 * only to provide access to the inode.  All per-file information is held
 * exclusively in the inode.
 */

struct sftd {
	struct vnode	*sfd_vp;	/* ptr to vnode */
};


/*	Current directory structure - JFS dependent part
 *
 * The IFSM maintains information about the current directory in each
 * drive for each process.  For every name based operation, the current
 * directory structure is passed down to the file system driver in order
 * to speed the pathname lookup process.  The JFS will keep a pointer to
 * the inode for the current directory in its part of the structure.
 *
 * When a process attempts to change its current directory, the request
 * is also passed down to the file system driver.  The JFS will maintain
 * use counts on the affected directory inodes for that operation.
 */

struct cdsd {
	struct vnode	*cdd_vp;	/* ptr to vnode */
};


/*	File search structure - JFS dependent part
 *
 * Reading a directory may require multiple calls to the file system.
 * The protocol requires that the first call is a "findfirst" operation.
 * That call will, in essence, open the directory.  It will remain open
 * until a corresponding "findclose" operation, and will be used for
 * "findnext" operations.  In addition to an inode pointer, which is 
 * required for any open object in the file system, the file system 
 * driver must keep a copy of the search arguments that are passed on
 * the findfirst call.
 */

struct fsfd {
	ino_t		fsd_inum;	/* inode number for the directory */
	int32		fsd_offset;	/* offset in directory */
	component_t	fsd_pattern;	/* pattern to search for */
	UniChar		*fsd_lastmatch;	/* last name matched */
	union havattr {			/* must-have/may-have attributes */
		uint32	_attr;
		struct {
			uint8	_mayHave;
			uint8	_mustHave;
			uint16	_nada;
		}	_haves;
	}		fsd_havattr;
};

#define	fsd_attr	fsd_havattr._attr
#define mustHave	_haves._mustHave
#define mayHave		_haves._mayHave


/*	Initialization constants
 */

/* Maximum number of inodes in the system is
 * set to 128 per megabyte of real memory.
 */
#define INODES_PER_MB	128

#define MAX_SEMAPHORES	1024*1024	/* Should be plenty */
#define INIT_SEMAPHORES	16 * 1024	/* Maybe too big */


/* Maximum number of pages to transfer in a single request to
 * the swapper file.  The number 8 is chosen because it matches
 * the maximum that the memory manager will request.
 */
#define MAXPGREQ	8

/*
 * ACL enumeration structure
 */
typedef struct enum_instance
{
	struct vfs		*enum_vfsp;
	ino_t			enum_inum;
	int32			enum_pid;
	UniChar			*enum_lastmatch;
	component_t		enum_pattern;			// F226941
	struct enum_instance	*next;
} enum_instance_t;

#endif /* _H_JFS_OS2 */
