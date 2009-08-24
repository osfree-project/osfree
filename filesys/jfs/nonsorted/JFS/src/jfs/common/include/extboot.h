/* $Id: extboot.h,v 1.1.1.1 2003/05/21 13:35:36 pasha Exp $ */

/* static char *SCCSID = "@(#)1.2  7/30/98 14:04:05 src/jfs/common/include/extboot.h, sysjfs, w45.fs32, 990417.1";
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
/* 
 * We need to duplicate the BPB structure defined in sysbloks.h.  Extended_BPB
 * is identical except without the six-byte reserved pad at the end of the
 * structure
 */

/* bios parameter block */

struct Extended_BPB {			     /* bios parameter block		 */
  unsigned short bytes_per_sector;   /* sector size		      2	 */
  unsigned char sectors_per_cluster; /* sectors per allocation unit   1	 */
  unsigned short reserved_sectors;   /* number of reserved sectors    2	 */
  unsigned char nbr_fats;	     /* number of fats		      1	 */
  unsigned short root_entries;	     /* number of directory entries   2	 */
  unsigned short total_sectors;	     /* number of sectors	      2	 */
  unsigned char media_type;	     /* fatid byte		      1	 */
  unsigned short sectors_per_fat;    /* sectors in a copy of the FAT  2	 */
  unsigned short sectors_per_track;  /* number of sectors per track   2	 */
  unsigned short number_of_heads;    /* number of heads		      2	 */
  unsigned long hidden_sectors;      /* number of hidden sectors      4	 */
  unsigned long large_total_sectors; /* large total sectors	      4	 */
};				     /*	    total byte size = 25	 */

/* Extended Boot Structure */

struct Extended_Boot {
	unsigned char		Boot_jmp[3];
	unsigned char		Boot_OEM[8];
	struct Extended_BPB	Boot_BPB;
	unsigned char		Boot_DriveNumber;
	unsigned char		Boot_CurrentHead;
	unsigned char		Boot_Sig;	/* 41 indicates extended boot */
	unsigned char		Boot_Serial[4];
	unsigned char		Boot_Vol_Label[11];
	unsigned char		Boot_System_ID[8];
};

#define EXT_BOOT_SIG 41

#pragma pack()
