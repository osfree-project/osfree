//
// $Header: /cur/cvsroot/boot/include/muFSD/os2/os2misc.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
// access your Linux ext2fs partitions as normal drive letters.
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#ifndef __OS2MISC_H
#define __OS2MISC_H

#pragma pack(1)


// 
// Extended Boot record structure and extended BPB (from the IBM IFS document)
//
struct Extended_BPB {
    unsigned short BytePerSector;
    unsigned char SectorPerCluster;
    unsigned short ReservedSectors;
    unsigned char NumberOfFats;
    unsigned short RootEntries;
    unsigned short TotalSectors;
    unsigned char MediaDescriptor;
    unsigned short SectorsPerFat;
    unsigned short SectorsPerTrack;
    unsigned short Heads;
    unsigned long HiddenSectors;
    unsigned long Ext_TotalSectors;
};

struct Extended_Boot {
    unsigned char Boot_jmp[3];
    unsigned char Boot_OEM[8];
    struct Extended_BPB Boot_BPB;
    unsigned char Boot_DriveNumber;
    unsigned char Boot_CurrentHead;
    unsigned char Boot_Sig;
    unsigned long Boot_Serial;
    unsigned char Boot_Vol_Label[11];
    unsigned char Boot_System_ID[8];
};

struct boot_sector {
    struct Extended_Boot boot;
    char                 pad[512 - sizeof(struct Extended_Boot) - sizeof(unsigned short)];
    unsigned short       sig;       // 0xAA55
};

#pragma pack()

//
// OS/2 infosegs
//
extern struct InfoSegGDT *pSysInfoSeg;
extern struct InfoSegLDT *pLocInfoSeg;

//
// Some command line options ...
//
extern int Read_Write;

/*
 * Skip drive letter
 */
INLINE char *skip_drive(char *path) {
    char *pathname = path;
    if (pathname[0]                                        &&
        (((pathname[0] >= 'A') && (pathname[0] <= 'Z')) ||
         ((pathname[0] >= 'a') && (pathname[0] <= 'z')))   &&
        pathname[1]                                        &&   
        (pathname[1] == ':') ) {
        pathname += 2;
    }
    return pathname;
}
#endif
