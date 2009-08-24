/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, 2001 Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   If you have any questions, comments, suggestions, or fixes please
   email me at: imre.leber@worldonline.be

 */

#ifndef BOOT_H_
#define BOOT_H_

/* The boot sector struct */
struct FAT32Specific
{ 
   unsigned long  SectorsPerFat;  /* Sectors per FAT (32 bit)                 */
   unsigned short ExtendedFlags;  /* Bits 0-3 -- Zero-based number of active FAT. 
                                                 Only valid if mirroring is disabled.
                                     Bits 4-6 -- Reserved.    
                                     Bit 7 -- 0 means the FAT is mirrored at runtime into all FATs.
                                           -- 1 means only one FAT is active; it is the one referenced
                                              in bits 0-3.
                                     Bits 8-15 -- Reserved.*/
   unsigned short FSVersion;      /* File system version, MUST be 0.  */
   unsigned long  RootCluster;    /* First cluster of ROOT directory. */                                    
   unsigned short FSInfo;         /* Sector number of FSINFO structure in the
                                     reserved area of the FAT32 volume. Usually 1.*/
   unsigned short BackupBoot;     /* Sector of boot sector backup         */
   unsigned char  Reserved[12];   /* Should be all 0's                    */
   unsigned char DriveNumber;     /* BIOS drive number                        */
   unsigned char Reserved1;       /* Used by Windows NT (must be 0)           */
   unsigned char Signature;       /* Indicates wether the following fields 
                                     are present                              */  
   unsigned long VolumeID;        /* Disk serial number                       */
   unsigned char VolumeLabel[11]; /* Volume label                             */
   unsigned char FSType[8];       /* Informational: "FAT12", "FAT16", "FAT32" */
                                  /* Has nothing to do with FAT type
                                     determination                            */
   unsigned char UnUsed[420];
   unsigned short LastTwoBytes;   /* Value of the last two bytes if
                                     BYTESPERSECTOR == 512                   */
};

#if sizeof(struct FAT32Specific) != 476
#error Wrong struct FAT32Specific
#endif

struct FAT1216Specific
{
   unsigned char DriveNumber;    /* BIOS drive number                        */
   unsigned char Reserved;       /* Used by Windows NT (must be 0)           */
   unsigned char Signature;      /* Indicates wether the following fields 
                                    are present                              */  
   unsigned char SerialNumber[4];/* Disk serial number                       */
   unsigned char VolumeLabel[11];/* Volume label                             */
   unsigned char FSType[8];      /* Informational: "FAT12", "FAT16", "FAT32" */       
   
   unsigned char UnUsed[448];

   unsigned short LastTwoBytes;  /* Value of the last two bytes if
                                    BYTESPERSECTOR == 512                   */
};

#if sizeof(struct FAT1216Specific) != 476
#error Wrong struct FAT1216Specific
#endif

union FATSpecific
{
   struct FAT32Specific spc32;
   struct FAT1216Specific spc1216;
};

struct BootSectorStruct
{
   char     Jump[3];                 /* Jump instruction in boot routine. */
   char     Identification[8];       /* Identification code.              */
   unsigned short BytesPerSector;    /* bytes per sector.                 */
   unsigned char  SectorsPerCluster; /* sectors per cluster.              */
   unsigned short ReservedSectors;   /* number of reserved sectors.       */
   unsigned char  Fats;              /* number of fats.                   */
   unsigned short NumberOfFiles;     /* number of files or directories in */
                                     /* the root directory.               */
   unsigned short NumberOfSectors;   /* number of sectors in the volume.  */
   unsigned char  descriptor;        /* media descriptor.                 */
   unsigned short SectorsPerFat;     /* number of sectors per fat.        */
   unsigned short SectorsPerTrack;   /* sectors per track.                */
   unsigned short Heads;             /* number of read/write heads.       */
   unsigned long  HiddenSectors;     /* number of hidden sectors in the 
                                        partition table                   */
   unsigned long  NumberOfSectors32; /* Number of sectors if the total
                                        number of sectors > 0xffff, or
                                        if this is FAT 32                 */
   union FATSpecific fs;             /* Fields that are different for
                                        FAT 12/16 and FAT32               */
};

#if sizeof(struct BootSectorStruct) != 512
#error WRONG BOOT SECTOR STRUCT LENGTH
#endif

/* A few simple defines */
#define BYTESPERSECTOR    512
#define ENTRIESPERSECTOR  (BYTESPERSECTOR / 32)

/* This is not used in diskcopy, intended for defrag 
   int ReadBootSector(RDWRHandle handle, struct BootSectorStruct* buffer);
   int WriteBootSector(RDWRHandle handle, struct BootSectorStruct* buffer);

   unsigned char  GetSectorsPerCluster(RDWRHandle handle);
   unsigned short GetReservedSectors(RDWRHandle handle);
   unsigned char  GetNumberOfFats(RDWRHandle handle);
   unsigned short GetNumberOfRootEntries(RDWRHandle handle);
   unsigned char  GetMediaDescriptor(RDWRHandle handle);
   unsigned short GetNumberOfSectors(RDWRHandle handle);
   unsigned short GetSectorsPerFat(RDWRHandle handle);
   unsigned short GetSectorsPerTrack(RDWRHandle handle);
   unsigned short GetReadWriteHeads(RDWRHandle handle);
   unsigned short GetClustersInDataArea(RDWRHandle handle);
 */

#endif