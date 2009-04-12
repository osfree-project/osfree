/* $Id: sysbloks.h,v 1.1.1.1 2003/05/21 13:36:00 pasha Exp $ */

/* static char *SCCSID = "@(#)1.3  9/23/98 08:36:38 src/jfs/common/include/sysbloks.h, sysjfs, w45.fs32, 990417.1";
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
#pragma pack(1)

/* bios parameter block */

struct BPB {			     /* bios parameter block		 */
  unsigned short bytes_per_sector;   /* sector size		      2	 */
  unsigned char sectors_per_cluster; /* sectors per allocation unit   1	 */
  unsigned short reserved_sectors;   /* number of reserved sectors    2	 */
  unsigned char nbr_fats;	     /* number of fats		      1	 */
  unsigned short root_entries;	     /* number of directory entries   2	 */
  unsigned short total_sectors;	     /* number of sectors	      2	 */
  char media_type;		     /* fatid byte		      1	 */
  unsigned short sectors_per_fat;    /* sectors in a copy of the FAT  2	 */
  unsigned short sectors_per_track;  /* number of sectors per track   2	 */
  unsigned short number_of_heads;    /* number of heads		      2	 */
  unsigned long hidden_sectors;      /* number of hidden sectors      4	 */
  unsigned long large_total_sectors; /* large total sectors	      4	 */
  char reserved_3[6];		     /* 6 reserved bytes	      6	 */
};				     /*	    total byte size = 31	 */

/* device parameter block */

struct DPB {			     /* device parameter block		 */
  struct BPB dev_bpb;		     /* 31 byte extended bpb		 */
  unsigned short number_of_tracks;   /* number of tracks		 */
  char device_type;		     /* device type	   see DT_	 */
  unsigned short device_attributes;  /* device attributes  see DA_	 */
};

#define DT_FLOPPY_LD   0	     /* 48 TPI low density diskette	 */
#define DT_FLOPPY_HD   1	     /* 96 TPI high density diskette	 */
#define DT_SMALL_DRIVE 2	     /* 3 1/2 inch drive		 */
#define DT_8IN_SD      3	     /* 8 inch single density floppy	 */
#define DT_8IN_DD      4	     /* 8 inch double density floppy	 */
#define DT_HARD_DISK   5	     /* hard disk drive			 */
#define DT_TAPE_DRIVE  6	     /* tape drive			 */
#define DT_OTHER       7	     /* unknown device type		 */

#define DA_REMOVABLE   0x0001	     /* removable media flag		 */
#define DA_CHANGELINE  0x0002	     /* detects when it has been changed */
#define DA_ABOVE16MB   0x0004	     /* supports physical memory > 16 MB */

/* directory entry */

struct DIR_ENTRY {		     /* directory entry, 32 bytes	 */
  char file_name[8];		     /*	 0 -  7				 */
  char ext_name[3];		     /*	 8 - 10				 */
  char attribute;		     /* 11				 */
  char reserved[10];		     /* 12 - 21				 */
  unsigned short time;		     /* 22 - 23				 */
  unsigned short date;		     /* 24 - 25				 */
  unsigned short first_cluster;	     /* 26 - 27				 */
  unsigned long file_size;	     /* 28 - 31				 */
};

/* fat table entry */

union FAT {			     /* 512 byte fat sector		 */
  unsigned short fat16[256];	     /* 16 bit fat entries		 */
  char fat8[512];		     /* fat as a byte stream		 */
};
#pragma pack()
