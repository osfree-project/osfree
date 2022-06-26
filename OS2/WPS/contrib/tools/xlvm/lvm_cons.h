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
* Module: lvm_cons.h
*/

/*
* Change History:
*
*/

/*
* Description:  This module defines constants used throughout the LVM code.
*
*/

/* XLATOFF */
#ifndef LVM_CONSTANTS_H_INCLUDED

#define LVM_CONSTANTS_H_INCLUDED 1
/* XLATON */

/* The number of bytes in a sector on the disk. */
#define BYTES_PER_SECTOR  512

/* The maximum number of cylinders, heads, and sectors that a partition table entry can accomodate.

Cylinders are numbered 0 - 1023, for a maximum of 1024 cylinders.
Heads are numbered 0 - 255, for a maximum of 256 heads.
Sectors are numbered 1 - 63, for a maximum of 63 sectors per track.                                */
#define MAX_CYLINDERS 1024
#define MAX_HEADS     256
#define MAX_SECTORS   63

/* The following define the values used to indicate that a partition table entry is for an EBR, not a partition. */
#define EBR_BOOT_INDICATOR     0
#define EBR_FORMAT_INDICATOR   5

/* The following define is used as the default Format_Indicator for new non-primary partitions. */
#define NEW_LOGICAL_DRIVE_FORMAT_INDICATOR   0x6

/* The following define is used as the default Format_Indicator for a new non-active primary partitions. */
#define NEW_PRIMARY_PARTITION_FORMAT_INDICATOR   0x16

/* The following define is used as the default Format_Indicator for a new active primary partition. */
#define NEW_ACTIVE_PRIMARY_PARTITION_FORMAT_INDICATOR  0x06

/* The following define is used to hold the value of the Boot_Indicator for active partitions. */
#define ACTIVE_PARTITION   0x80

/* Define the size of a Partition Name.  Partition Names are user defined names given to a partition. */
#define PARTITION_NAME_SIZE  20

/* Define the size of a volume name.  Volume Names are user defined names given to a volume. */
#define VOLUME_NAME_SIZE  20

/* Define the size of a disk name.  Disk Names are user defined names given to physical disk drives in the system. */
#define DISK_NAME_SIZE    20

/* The name of the filesystem in use on a partition.  This name may be up to 12 ( + NULL terminator) characters long. */
#define FILESYSTEM_NAME_SIZE 20

/* The comment field is reserved but is not currently used.  This is for future expansion and use. */
#define COMMENT_SIZE 81

/* Define the minimum number of sectors to reserve on the disk for Boot Manager. */
#define BOOT_MANAGER_SIZE   2048

/* XLATOFF */
#endif
/* XLATON */
