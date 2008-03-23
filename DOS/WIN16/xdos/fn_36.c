/*    
	fn_36.c	1.26
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

**
** 	Return disk free space (int 21 Fn 36h)
**
**	get free disk space...
**	(needs to be moved into mfs code, to handle floppy driver...
**	can be put into mfs_disk.c for generic handling...
*/

#include "windows.h"
#include "Log.h"
#include "fn_36.h"
#include "platform.h"
#include <sys/types.h>

#if defined(TWIN_HASSTATVFS)
#include <sys/statvfs.h>

#elif defined(TWIN_HASSTATFS)
#include <sys/statfs.h>

#else
#include <sys/vfs.h>
#endif

extern int get_path(int, char *, int);

#define		DOSMAXVALUE	0x7fff
#define		DOSMINBLKSIZE	1024
#define		DOSCLUSTER	(8 * 512)

unsigned int
disk_free(
		int		drive,
		unsigned int	*clusters,	/* Number of free clusters */
		unsigned int	*driveclusters,	/* Number of total lcusters on disk */
		unsigned int	*sectorsize,	/* Bytes per sector */
		unsigned int	*sectorsper	/* Sectors per cluster */
	 )
{
	register unsigned int	blkpcluster;
	register unsigned long	nclusters;
	unsigned int		bsize;
	unsigned long		nfree;
	unsigned long		ntotal;
	char 			file[256];

	/* this gets us the mapped drive... */
	if(get_path(drive,file,256)) 
	{
		return -1;
	}

#if defined(NETWARE)
	{
		char server[49];
		char volume[17];
		char dirs[256];
		int volumeNumber;
		VOLUME_INFO vi;
		GetAvailableUserDiskSpace(file, (LONG*)clusters);
		ParsePath(file, server, volume, dirs);
		GetVolumeNumber(volume, &volumeNumber);
		GetVolumeStatistics(GetDefaultFileServerID(),
			volumeNumber,
			sizeof(VOLUME_INFO),
			&vi);
		*sectorsize = 512;
		*sectorsper = vi.sectorsPerBlock;
		*driveclusters = vi.totalBlocks;
	}
#else
	{
	#if defined(TWIN_HASSTATVFS)
		struct statvfs sbuf;
	#else
		struct statfs sbuf;
	#endif

	#if defined(TWIN_HASSTATFS)
		if(statfs(file, &sbuf, sizeof(struct statfs), 0) < 0)
			return -1;
	#elif defined(TWIN_HASSTATVFS)
		if (statvfs(file, &sbuf) < 0)
			return -1;
	#else
		if(statfs(file, &sbuf) < 0 )
			return -1;
	#endif

	bsize = (unsigned long)sbuf.f_bsize;
	nfree =  (unsigned long)sbuf.f_bfree;
	ntotal = (unsigned long)sbuf.f_blocks;

	if (((blkpcluster = (DOSCLUSTER / bsize)) * bsize) != DOSCLUSTER)
		blkpcluster = 1;		/* If not a multiple of 512 */

	nclusters = ntotal / blkpcluster;

	while ((nclusters > DOSMAXVALUE)
		&& ((blkpcluster * bsize) < (DOSMAXVALUE / 2))) 
	{
		blkpcluster *= 2;
		nclusters /= 2;
	}

	*driveclusters = (nclusters > DOSMAXVALUE) ? DOSMAXVALUE : nclusters;

	nclusters = nfree / blkpcluster;

	*clusters = (nclusters > DOSMAXVALUE) ? DOSMAXVALUE : nclusters;
	*sectorsize = bsize;
	*sectorsper = blkpcluster;
	}
#endif	/* NETWARE */

	return 0;

}

