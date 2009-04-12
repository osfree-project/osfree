/* $Id: mbr.h,v 1.1.1.1 2003/05/21 13:42:24 pasha Exp $ */

/* @(#)10       1.2  src/jfs/utils/libfs/mbr.h, jfslib, w45.fs32, 990417.1 7/29/98 16:51:40  */
/*
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
 *   COMPONENT_NAME: cmdufs
 *
 *   FUNCTIONS: none
 *
 */
#ifndef H_MBR
#define H_MBR

/* XLATOFF */
#pragma pack(2)
/* XLATON */
/*
 * Structures which define what the master boot record looks like.
 * They are used to find the partition on disk.
 */
struct part {
    UCHAR       bootind;        /* 0x80 means partition is bootable */
    UCHAR       starthead;      /* head number of partition start */
    UCHAR       startsect;      /* sector number */
    UCHAR       startcyl;       /* cylinder number */
    UCHAR       systind;        /* partition ID */
    UCHAR       endhead;        /* head number of partition end */
    UCHAR       endsect;        /* sector number */
    UCHAR       endcyl;         /* cylinder number */
    ULONG       lsn;            /* number of sectors before partition */
    ULONG       nsects;         /* number of sectors in partition */
};

struct mbr {
    UCHAR       code[0x1be];    /* boot record code and data */
    struct part ptbl[4];        /* the partition table */
    USHORT      sig;            /* special signature */
};

/*
 * Partition table ID for IFS partitions.
 */
#define F12_PART        1
#define F16_PART        4
#define BIG_PART        6
#define IFS_PART        7
#define HIDDEN_PART_MASK 0x10                            //201823

/* XLATOFF */
#pragma pack()
/* XLATON */

#endif  /* H_MBR */
