
/*
 *@@sourcefile lvm.h:
 *      header file for lvm.c. See remarks there.
 *
 *@@include #include "helpers\lvm.h"
 */

/*
 *      Copyright (C) 2000 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef LVM_HEADER_INCLUDED
    #define LVM_HEADER_INCLUDED

    /* An INTEGER number is a whole number, either positive or negative.
       The number appended to the INTEGER key word indicates the number of bits
       used to represent an INTEGER of that type. */
    typedef short int INTEGER16;
    typedef long  int INTEGER32;
    typedef int       INTEGER;    /* Use compiler default. */

    /* In accordance with its mathematical definition, a CARDINAL number is a positive integer >= 0.
       The number appended to the CARDINAL key word indicates the number of bits
       used to represent a CARDINAL of that type. */

    typedef unsigned short int CARDINAL16;
    typedef unsigned long      CARDINAL32;
    typedef unsigned int       CARDINAL;     /* Use compiler default. */

    /* A BYTE is 8 bits of memory with no interpretation attached. */
    // typedef unsigned char BYTE;

    /* A BOOLEAN variable is one which is either TRUE or FALSE. */
    typedef unsigned char  BOOLEAN;
    /* #define TRUE  1
    #define FALSE 0; */

    /* An ADDRESS variable is one which holds the address of a location in memory. */
    // typedef void * ADDRESS;

    /* pSTRING is a pointer to an array of characters. */
    typedef char * pSTRING;

    /* 4 bytes */
    // typedef unsigned long DoubleUSHORT;

    /* 2 bytes */
    // typedef short unsigned int USHORT;

    /* Define a Partition Sector Number.  A Partition Sector Number is relative to the start of a partition.
       The first sector in a partition is PSN 0. */
    typedef unsigned long PSN;

    /* Define a Logical Sector Number.  A Logical Sector Number is relative to the start of a volume.
       The first sector in a volume is LSN 0. */
    typedef unsigned long LSN;

    /* Define a Logical Block Address.  A Logical Block Address is relative to the start of a
       physical device - a disk drive.  The first sector on a disk drive is LBA 0. */
    typedef unsigned long LBA;

    /* ******************************************************************
     *
     *   Partition definitions
     *
     ********************************************************************/

    /*  The disk structure used to create and control partitions on a
        physical disk drive is the partition table.  The partition
        table is itself embedded in two other structures:  the Master
        Boot Record (MBR) and the Extended Boot Record (EBR).  There
        is only one MBR per physical disk drive, and it is always
        located in the first sector of the physical disk drive.
        It contains code and a partition table.  An EBR is similar
        to an MBR except that an EBR generally does not contain code,
        and may appear multiple times on a physical disk drive. */

    /* The following definitions define the format of a partition table and the Master Boot Record (MBR). */
    typedef struct _Partition_Record
    {
        BYTE       Boot_Indicator;    /* 80h = active partition. */
        BYTE       Starting_Head;
        BYTE       Starting_Sector;   /* Bits 0-5 are the sector.  Bits 6 and 7 are the high
                                         order bits of the starting cylinder. */
        BYTE       Starting_Cylinder; /* The cylinder number is a 10 bit value.  The high order
                                         bits of the 10 bit value come from bits 6 & 7 of the
                                         Starting_Sector field.                                */
        BYTE       Format_Indicator;  /* An indicator of the format/operation system on this
                                         partition.                                            */
        BYTE       Ending_Head;
        BYTE       Ending_Sector;
        BYTE       Ending_Cylinder;
        ULONG Sector_Offset;     /* The number of sectors on the disk which are prior to
                                         the start of this partition.                          */
        ULONG Sector_Count;      /* The number of sectors in this partition. */
    } Partition_Record;

    typedef struct _Master_Boot_Record
    {
        BYTE                Reserved[446];
       Partition_Record    Partition_Table[4];
       USHORT                Signature;            /* AA55h in this field indicates that this
                                                    is a valid partition table/MBR.         */
    } Master_Boot_Record;
    typedef Master_Boot_Record  Extended_Boot_Record;

    /* The following is the signature used for a Master Boot Record, an Extended Boot Record, and a Boot Sector. */
    #define MBR_EBR_SIGNATURE  0xAA55

    /* The following list of definitions defines the values of interest for the Format_Indicator in a Partition_Record. */
    #define EBR_INDICATOR                          0x5
    #define UNUSED_INDICATOR                       0x0
    #define IFS_INDICATOR                          0x7
    #define FAT12_INDICATOR                        0x1
    #define FAT16_SMALL_PARTITION_INDICATOR        0x4
    #define FAT16_LARGE_PARTITION_INDICATOR        0x6
    #define BOOT_MANAGER_HIDDEN_PARTITION_FLAG     0x10
    #define LVM_PARTITION_INDICATOR                0x35
    #define BOOT_MANAGER_INDICATOR                 0x0A

    /* ******************************************************************
     *
     *   Drive Letter Assignment Table (DLAT)
     *
     ********************************************************************/

    /*  In addition to the standard structures for controlling the
        partitioning of a physical disk drive, LVM introduces a new
        structure:  the Drive Letter Assignment Table, or DLAT for short.
        The DLAT is used to associate drive letters with volumes, associate
        names with volumes, partitions, and physical disk drives,  indicate
        which volumes and partitions are on the Boot Manager Menu, and to
        hold the information required to turn a partition into a compatibility
        volume.

        Volumes come in two types: Compatibility and LVM.  Compatibility
        volumes are comprised of a single partition, and are usable by older
        versions of OS/2 as well as other operating systems.  LVM volumes
        are not compatible with other operating systems or older versions
        of OS/2.  However, since they do not have to be compatible, they
        can support extra features that compatibility volumes can not,
        such as drive linking and bad block relocation.  The data required
        to implement the various features found on LVM volumes is stored
        within the partitions that comprise the volume.  However, this trick
        can not be employed for compatibility volumes so the data required
        for the features found on compatibility volumes must be stored
        elsewhere: the DLAT.

        DLAT tables are fixed in size, and there is one DLAT for each MBR
        or EBR on a physical disk drive.  The DLAT table resides in an area
        of the disk which is not currently used due to the rules that govern
        the creation of partitions.  Currently, in the DOS/Windows/OS/2
        world, partitions must start on track boundaries.  Extended partitions
        must start on cylinder boundaries.  Since an MBR or EBR occupy one
        sector and must always be the first sector of a track, the remaining
        sectors on a track containing an MBR or EBR are unused.  The DLAT
        table for an MBR/EBR is stored in one of these unused sectors,
        specifically, the last sector on the track.  Thus, for any MBR/EBR,
        the MBR/EBR will be the first sector of a track and its corresponding
        DLAT table will be the last sector of the same track. */

    /* The following definitions define the drive letter assignment table
       used by LVM.
       For each partition table on the disk, there will be a drive letter
       assignment table in the last sector of the track containing the partition
       table. */

    /* NOTE: DLA stands for Drive Letter Assignment. */

    /* Define the signature values to be used for a DLA Table. */
    #define DLA_TABLE_SIGNATURE1  0x424D5202L
    #define DLA_TABLE_SIGNATURE2  0x44464D50L

    /* Define the size of a Partition Name.
       Partition Names are user defined names given to a partition. */
    #define PARTITION_NAME_SIZE  20

    /* Define the size of a volume name.  Volume Names are user defined
       names given to a volume. */
    #define VOLUME_NAME_SIZE  20

    /* Define the size of a disk name.  Disk Names are user defined names
       given to physical disk drives in the system. */
    #define DISK_NAME_SIZE    20

    /* Define the structure of an entry in a DLAT. */
    typedef struct _DLA_Entry
    {
        ULONG  Volume_Serial_Number;                /* The serial number of the volume
                                                            that this partition belongs to.         */
        ULONG  Partition_Serial_Number;             /* The serial number of this partition.    */
        ULONG  Partition_Size;                      /* The size of the partition, in sectors.  */
        LBA         Partition_Start;                     /* The starting sector of the partition.   */
        BOOLEAN     On_Boot_Manager_Menu;                /* Set to TRUE if this volume/partition
                                                            is on the Boot Manager Menu.            */
        BOOLEAN     Installable;                         /* Set to TRUE if this volume is the one
                                                            to install the operating system on.     */
        char        Drive_Letter;                        /* The drive letter assigned to the
                                                            volume, or 0h if the volume is hidden.  */
        BYTE        Reserved;
        char        Volume_Name[VOLUME_NAME_SIZE];       /* The name assigned to the volume by
                                                            the user.                               */
        char        Partition_Name[PARTITION_NAME_SIZE]; /* The name assigned to the partition.     */
    } DLA_Entry;

    /* Define the contents of the sector used to hold a DLAT. */
    typedef struct _DLA_Table_Sector
    {
        ULONG     DLA_Signature1;             /* The magic signature (part 1) of a
                                                      Drive Letter Assignment Table.               */
        ULONG     DLA_Signature2;             /* The magic signature (part 2) of a
                                                      Drive Letter Assignment Table.               */
        ULONG     DLA_CRC;                    /* The 32 bit CRC for this sector.
                                                      Calculated assuming that this
                                                      field and all unused space in
                                                      the sector is 0.                             */
        ULONG     Disk_Serial_Number;         /* The serial number assigned to
                                                      this disk.                                   */
        ULONG     Boot_Disk_Serial_Number;    /* The serial number of the disk used to
                                                      boot the system.  This is for conflict
                                                      resolution when multiple volumes want
                                                      the same drive letter.  Since LVM.EXE
                                                      will not let this situation happen, the
                                                      only way to get this situation is for the
                                                      disk to have been altered by something other
                                                      than LVM.EXE, or if a disk drive has been
                                                      moved from one machine to another.  If the
                                                      drive has been moved, then it should have a
                                                      different Boot_Disk_Serial_Number.  Thus,
                                                      we can tell which disk drive is the "foreign"
                                                      drive and therefore reject its claim for the
                                                      drive letter in question.  If we find that
                                                      all of the claimaints have the same
                                                      Boot_Disk_Serial_Number, then we must assign
                                                      drive letters on a first come, first serve
                                                      basis.                                       */
        CARDINAL32     Install_Flags;              /* Used by the Install program.                 */
        CARDINAL32     Cylinders;                  /* Used by OS2DASD.DMD                          */
        CARDINAL32     Heads_Per_Cylinder;         /* Used by OS2DASD.DMD                          */
        CARDINAL32     Sectors_Per_Track;          /* Used by OS2DASD.DMD                          */
        char           Disk_Name[DISK_NAME_SIZE];  /* The name assigned to the disk containing
                                                      this sector.                                 */
        BOOLEAN        Reboot;                     /* For use by Install.  Used to keep track of
                                                      reboots initiated by install.                */
        BYTE           Reserved[3];                /* Alignment.                                   */
        DLA_Entry      DLA_Array[4];               /* These are the four entries which correspond
                                                      to the entries in the partition table.       */
    } DLA_Table_Sector;

    /* ******************************************************************
     *
     *   LVM Volume Signature Sectors
     *
     ********************************************************************/

    /*  As mentioned earlier, LVM Volumes have extra features that Compatibility
        Volumes do not. The data required to implement these features is stored
        inside of the partitions that comprise the LVM Volume.  Specifically,
        the last sector of each partition that is part of an LVM Volume contains
        an LVM Signature Sector.  The LVM Signature Sector indicates which features
        are active on the volume that the partition is a part of, and where on the
        partition the data for those features may be found.  It also contains a
        duplicate copy of the DLAT information for the partition.  The following
        definitions are used by LVM.DLL for the LVM Signature Sector: */

    /*  The following definitions define the LVM signature sector which will appear
        as the last sector in an LVM partition. */

    #define  LVM_PRIMARY_SIGNATURE   0x4A435332L
    #define  LVM_SECONDARY_SIGNATURE 0x4252444BL

    #define  CURRENT_LVM_MAJOR_VERSION_NUMBER   1        /* Define as appropriate. */
    #define  CURRENT_LVM_MINOR_VERSION_NUMBER   0        /* Define as appropriate. */

    /* The following definitions limit the number of LVM features that
       can be applied to a volume, as well as defining a "NULL"
       feature for use in feature table entries that are not being used.                                                          */
    #define  MAX_FEATURES_PER_VOLUME  10     /* The maximum number of LVM features that can be applied to a volume. */
    #define  NULL_FEATURE              0     /* No feature.  Used in all unused entries of the feature array in the LVM Signature
                                                sector.                                                                           */

    /* The following structure is used to hold the location of the feature
       specific data for LVM features. */
    typedef struct _LVM_Feature_Data
    {
        ULONG     Feature_ID;                            /* The ID of the feature. */
        PSN            Location_Of_Primary_Feature_Data;      /* The PSN of the starting sector of
                                                                 the private data for this feature.*/
        PSN            Location_Of_Secondary_Feature_Data;    /* The PSN of the starting sector of
                                                                 the backup copy of the private
                                                                 data for this feature.            */
        ULONG     Feature_Data_Size;                     /* The number of sectors used by this
                                                                 feature for its private data.     */
        USHORT           Feature_Major_Version_Number;          /* The integer portion of the version
                                                                 number of this feature.           */
        USHORT           Feature_Minor_Version_Number;          /* The decimal portion of the version
                                                                 number of this feature.           */
        BOOLEAN        Feature_Active;                        /* TRUE if this feature is active on
                                                                 this partition/volume, FALSE
                                                                 otherwise.                        */
        BYTE           Reserved[3];                           /* Alignment. */
    } LVM_Feature_Data;


    /* The following structure defines the LVM Signature Sector.  This is the
       last sector of every partition which is part of an LVM volume.  It gives
       vital information about the version of LVM used to create the LVM volume
       that it is a part of, as well as which LVM features (BBR, drive linking,
       etc.) are active on the volume that this partition is a part of. */
#if 0
    typedef struct _LVM_Signature_Sector
    {
        ULONG        LVM_Signature1;                             /* The first part of the
                                                                         magic LVM signature. */
        ULONG        LVM_Signature2;                             /* The second part of
                                                                         the magic LVM
                                                                         signature.           */
        ULONG        Signature_Sector_CRC;                       /* 32 bit CRC for this
                                                                         sector.  Calculated
                                                                         using 0 for this
                                                                         field.               */
        ULONG        Partition_Serial_Number;                    /* The LVM assigned
                                                                         serial number for this
                                                                         partition.           */
        LBA               Partition_Start;                            /* LBA of the first
                                                                         sector of this
                                                                         partition.           */
        LBA               Partition_End;                              /* LBA of the last sector
                                                                         of this partition.   */
        ULONG        Partition_Sector_Count;                     /* The number of sectors
                                                                         in this partition.   */
        ULONG        LVM_Reserved_Sector_Count;                  /* The number of sectors
                                                                         reserved for use by
                                                                         LVM.                 */
        ULONG        Partition_Size_To_Report_To_User;           /* The size of the
                                                                         partition as the user
                                                                         sees it - i.e. (the
                                                                         actual size of the
                                                                         partition - LVM
                                                                         reserved sectors)
                                                                         rounded to a track
                                                                         boundary.            */
        ULONG        Boot_Disk_Serial_Number;                    /* The serial number of
                                                                         the boot disk for the
                                                                         system.  If the system
                                                                         contains Boot Manager,
                                                                         then this is the
                                                                         serial number of the
                                                                         disk containing the
                                                                         active copy of Boot
                                                                         Manager.             */
        ULONG        Volume_Serial_Number;                       /* The serial number of
                                                                         the volume that this
                                                                         partition belongs to.*/
        CARDINAL32        Fake_EBR_Location;                          /* The location, on disk,
                                                                         of a Fake EBR, if one
                                                                         has been allocated.  */
        USHORT              LVM_Major_Version_Number;                   /* Major version number
                                                                         of the LVM that
                                                                         created this
                                                                         partition.           */
        USHORT              LVM_Minor_Version_Number;                   /* Minor version number
                                                                         of the LVM that
                                                                         created this
                                                                         partition.           */
        char              Partition_Name[PARTITION_NAME_SIZE];        /* User defined partition
                                                                         name.                */
        char              Volume_Name[VOLUME_NAME_SIZE];              /* The name of the volume
                                                                         that this partition
                                                                         belongs to.          */
        LVM_Feature_Data  LVM_Feature_Array[MAX_FEATURES_PER_VOLUME]; /* The feature array.
                                                                         This indicates which
                                                                         LVM features, if any,
                                                                         are active on this
                                                                         volume and what order
                                                                         they should be applied
                                                                         in.                  */
        char              Drive_Letter;                               /* The drive letter
                                                                         assigned to the volume
                                                                         that this partition is
                                                                         part of.             */
        BOOLEAN           Fake_EBR_Allocated;                         /* If TRUE, then a fake
                                                                         EBR has been
                                                                         allocated.           */
        char              Comment[COMMENT_SIZE];                      /* User comment.        */
        /* The remainder of the sector is reserved for future use and should be all zero or
           else the CRC will not come out correctly.                                          */
    } LVM_Signature_Sector;
#endif

    /* ******************************************************************
     *
     *   Boot Manager
     *
     ********************************************************************/

    /*  In addition to dealing with partitions and volumes, LVM.DLL must also
        deal with Boot Manager.  Boot Manager has several data areas which must
        be modified by LVM.DLL when LVM.DLL installs Boot Manager, or when it
        upgrades an existing Boot Manager installation.  The structures which
        define these areas are listed below: */

    /* Structure used to hold the values for INT13 calls to access the disk. */
    typedef struct _INT13_Record
    {
        unsigned short      AX; /* AH = 02, always.  AL = number of sectors to read. */
        unsigned short      CX; /* CH = top 8 bits of cylinder number.  CL bits 0 - 5 = sector number,
                                   bits 6 and 7  are high order bits of cylinder number. */
        unsigned short      DX; /* DH = head number.  DL = drive number.  Note: Bit 7 must always be set.*/
    } INT13_Record;

    #define INT13_TABLE_SIZE   11

    /* Boot Manager Boot Structure. */
    typedef struct _Boot_Manager_Boot_Record
    {
        unsigned char           Boot_jmp[3];
        unsigned char           Boot_OEM[8];
        struct Extended_BPB     Boot_BPB;
        unsigned char           Boot_DriveNumber;
        unsigned char           Boot_CurrentHead;
        unsigned char           Boot_Sig;                     /* 41 indicates extended boot */
        unsigned char           Boot_Serial[4];
        unsigned char           Boot_Vol_Label[11];
        unsigned char           Boot_System_ID[8];
        unsigned long           Reserved1;                     /* _SectorBase */
        unsigned short          Reserved2;                     /* CurrentTrack */
        unsigned char           Reserved3;                     /* CurrentSector */
        unsigned short          Reserved4;                     /* SectorCount */
        unsigned long           Reserved5;                     /* lsnSaveChild */
        unsigned char           BootPathDrive;
        unsigned char           BootPathHead;
        unsigned char           BootPathSector;                /* Bits 0 - 5 are sector,
                                                                  bits 6 and 7 are high order
                                                                  bits of Cylinder.          */
        unsigned char           BootPathCylinder;              /* Lower 8 bits of cylinder.  */
        INT13_Record            INT13_Table[INT13_TABLE_SIZE];
    } Boot_Manager_Boot_Record;

    #define BOOT_MANAGER_PATH_SECTOR_OFFSET 1

    /* Boot Manager Alias entry. */
    #define ALIAS_NAME_LENGTH 8
    typedef struct _Alias_Entry
    {
        unsigned char  Reserved[4];
        unsigned char  Name[ALIAS_NAME_LENGTH];
    } Alias_Entry;

    /* The next structure defines the Boot Manager BootPath record. */
    #define MAX_ALIAS_ENTRIES  6
    typedef struct _Boot_Path_Record
    {
        unsigned char    Drive;
        unsigned char    Head;
        unsigned char    Sector;
        unsigned char    Cylinder;
        unsigned short   Migration_Flag;  /* ??? */
        unsigned short   TimeOut;         /* Time out value in 1/18 of a second increments. */
        unsigned char    Boot_Index;
        unsigned char    Advanced_Mode;  /* If 0, then Boot Manager operates in Normal Mode.
                                            If 1, then Boot Manager operates in advanced mode. */
        unsigned char    Immediate_Boot_Drive_Letter;
        unsigned char    Reboot_Flag;
        unsigned char    Reserved[4];
        Alias_Entry      Alias_Array[MAX_ALIAS_ENTRIES];
    } Boot_Path_Record;

    #define DEFAULT_ALIAS_ENTRY   0
    #define LAST_ALIAS_BOOTED     5
    #define IMMEDIATE_BOOT_ALIAS  4

    /* Boot Manager Alias Table Information.
       The Alias Table is a two dimensional array of structures.
       The array is 24 by 4, and is composed of Alias_Table_Entry structures.
       It is used to hold the Boot Manager name of any primary
       partitions residing on the first 24 drives in the system. */

    #define ALIAS_TABLE_SECTOR_OFFSET    3
    #define SECTORS_PER_ALIAS_TABLE      3
    #define ALIAS_TABLE_DRIVE_LIMIT     24
    #define ALIAS_TABLE_PARTITION_LIMIT  4

    /* The following structure is used in the creation of the
       Boot Manager Alias Table. */
    typedef struct _Alias_Table_Entry
    {
        unsigned char  Drive;                   /* BIOS Drive ID of the partition this entry
                                                   represents.                                    */

        /* Head, Sector, and Cylinder are the CHS address of the partition this entry represents. */

        unsigned char  Head;
        unsigned char  Sector;
        unsigned char  Cylinder;

        unsigned char  Bootable;                 /* ?? Assumed to be 0 if not bootable. */
        char           Name[ALIAS_NAME_LENGTH];  /* Boot Manager name for the partition represented
                                                    by this entry. */

        /* Padding. */
        unsigned char  Reserved[3];
    } Alias_Table_Entry;

    /* ******************************************************************
     *
     *   LVM API declarations
     *
     ********************************************************************/

    /* The following are invariant for a disk drive. */
    typedef struct _Drive_Control_Record
    {
        CARDINAL32   Drive_Number;        /* OS/2 Drive Number for this drive. */
        CARDINAL32   Drive_Size;          /* The total number of sectors on the drive. */
        ULONG   Drive_Serial_Number; /* The serial number assigned to this drive.
                                             For info. purposes only. */
        PVOID      Drive_Handle;        /* Handle used for operations on the disk that
                                             this record corresponds to. */
        CARDINAL32   Cylinder_Count;      /* The number of cylinders on the drive. */
        CARDINAL32   Heads_Per_Cylinder;  /* The number of heads per cylinder for this drive. */
        CARDINAL32   Sectors_Per_Track;   /* The number of sectors per track for this drive. */
        BOOLEAN      Drive_Is_PRM;        /* Set to TRUE if this drive is a PRM. */
        BYTE         Reserved[3];         /* Alignment. */
    } Drive_Control_Record;

    /* The following structure is returned by the Get_Drive_Control_Data function. */
    typedef struct _Drive_Control_Array
    {
        Drive_Control_Record *   Drive_Control_Data;       /* An array of drive control records. */
        CARDINAL32               Count;                    /* The number of entries in the array
                                                              of drive control records. */
    } Drive_Control_Array;


    /* The following structure defines the information that can be changed for a specific disk drive. */
    typedef struct _Drive_Information_Record
    {
        CARDINAL32   Total_Available_Sectors;        /* The number of sectors on the disk
                                                        which are not currently assigned to
                                                        a partition. */
        CARDINAL32   Largest_Free_Block_Of_Sectors;  /* The number of sectors in the largest
                                                        contiguous block of available sectors.  */
        BOOLEAN      Corrupt_Partition_Table;        /* If TRUE, then the partitioning
                                                        information found on the drive is
                                                        incorrect! */
        BOOLEAN      Unusable;                       /* If TRUE, the drive's MBR is not
                                                        accessible and the drive can not be
                                                        partitioned. */
        BOOLEAN      IO_Error;                       /* If TRUE, then the last I/O operation
                                                        on this drive failed! */
        BYTE         Reserved;
        char         Drive_Name[DISK_NAME_SIZE];     /* User assigned name for this disk
                                                        drive. */
    } Drive_Information_Record;

    typedef struct _Partition_Information_Record
    {
        PVOID      Partition_Handle;                      /* The handle used to perform
                                                               operations on this partition. */
        PVOID      Volume_Handle;                         /* If this partition is part
                                                               of a volume, this will be the
                                                               handle of the volume.  If
                                                               this partition is NOT
                                                               part of a volume, then
                                                               this handle will be 0.        */
        PVOID      Drive_Handle;                          /* The handle for the drive
                                                               this partition resides on. */
        ULONG   Partition_Serial_Number;               /* The serial number assigned
                                                               to this partition.         */
        CARDINAL32   Partition_Start;                       /* The LBA of the first
                                                               sector of the partition. */
        CARDINAL32   True_Partition_Size;                   /* The total number of
                                                               sectors comprising the partition. */
        CARDINAL32   Usable_Partition_Size;                 /* The size of the partition
                                                               as reported to the IFSM.  This is the
                                                               size of the partition less
                                                               any LVM overhead.                  */
        CARDINAL32   Boot_Limit;                            /* The maximum number of
                                                               sectors from this block
                                                               of free space that can be
                                                               used to create a bootable
                                                               partition if you allocate
                                                               from the beginning of the block
                                                               of free space.             */
        BOOLEAN      Spanned_Volume;                        /* TRUE if this partition is
                                                               part of a multi-partition
                                                               volume.                          */
        BOOLEAN      Primary_Partition;                     /* True or False.  Any
                                                               non-zero value here indicates
                                                               that this partition is
                                                               a primary partition.  Zero
                                                               here indicates that this
                                                               partition is a "logical drive"
                                                               - i.e. it resides inside of
                                                               an extended partition. */
        BYTE         Active_Flag;                           /* 80 = Partition is marked
                                                                    as being active.
                                                                0 = Partition is not
                                                                    active.                   */
        BYTE         OS_Flag;                               /* This field is from the
                                                               partition table.  It is
                                                               known as the OS flag, the
                                                               Partition Type Field,
                                                               Filesystem Type, and
                                                               various other names.

                                                               Values of interest

                                                               If this field is: (values
                                                               are in hex)

                                                               07 = The partition is a
                                                                    compatibility
                                                                    partition formatted
                                                                    for use with an
                                                                    installable
                                                                    filesystem, such as
                                                                    HPFS or JFS.
                                                               00 = Unformatted partition
                                                               01 = FAT12 filesystem is
                                                                    in use on this
                                                                    partition.
                                                               04 = FAT16 filesystem is
                                                                    in use on this
                                                                    partition.
                                                               0A = OS/2 Boot Manager
                                                                    Partition
                                                               35 = LVM partition
                                                               06 = OS/2 FAT16 partition    */
        BYTE         Partition_Type;                        /* 0 = Free Space
                                                               1 = LVM Partition (Part of
                                                                   an LVM Volume.)
                                                               2 = Compatibility Partition
                                                               All other values are reserved
                                                               for future use. */
        BYTE         Partition_Status;                      /* 0 = Free Space
                                                               1 = In Use - i.e. already
                                                                    assigned to a volume.
                                                               2 = Available - i.e. not
                                                                   currently assigned
                                                                   to a volume. */
        BOOLEAN      On_Boot_Manager_Menu;                  /* Set to TRUE if this
                                                               partition is not part of
                                                               a Volume yet is on the
                                                               Boot Manager Menu.       */
        BYTE         Reserved;                              /* Alignment. */
        char         Volume_Drive_Letter;                   /* The drive letter assigned
                                                               to the volume that this
                                                                partition is a part of. */
        char         Drive_Name[DISK_NAME_SIZE];            /* User assigned name for
                                                                this disk drive. */
        char         File_System_Name[FILESYSTEM_NAME_SIZE];/* The name of the filesystem
                                                               in use on this partition,
                                                               if it is known. */
        char         Partition_Name[PARTITION_NAME_SIZE];   /* The user assigned name for
                                                               this partition. */
        char         Volume_Name[VOLUME_NAME_SIZE];         /* If this partition is part
                                                               of a volume, then this
                                                               will be the name of the
                                                               volume that this partition
                                                               is a part of.  If this record
                                                               represents free space,
                                                               then the Volume_Name will be
                                                               "FS xx", where xx is a unique
                                                               numeric ID generated by
                                                               LVM.DLL.  Otherwise it
                                                               will be an empty string.     */
    } Partition_Information_Record;

    /* The following defines are for use with the Partition_Type field in the Partition_Information_Record. */
    #define FREE_SPACE_PARTITION     0
    #define LVM_PARTITION            1
    #define COMPATIBILITY_PARTITION  2

    /* The following defines are for use with the Partition_Status field in the Partition_Information_Record. */
    #define PARTITION_IS_IN_USE      1
    #define PARTITION_IS_AVAILABLE   2
    #define PARTITION_IS_FREE_SPACE  0


    /* The following structure is returned by various functions in the LVM Engine. */
    typedef struct _Partition_Information_Array
    {
        Partition_Information_Record * Partition_Array; /* An array of Partition_Information_Records. */
        CARDINAL32                     Count;           /* The number of entries in the Partition_Array. */
    } Partition_Information_Array;

    /* The following items are invariant for a volume. */
    typedef struct _Volume_Control_Record
    {
        ULONG Volume_Serial_Number;            /* The serial number assigned to this volume. */
        PVOID    Volume_Handle;                   /* The handle used to perform operations on this volume. */
        BOOLEAN    Compatibility_Volume;            /* TRUE indicates that this volume is compatible with older versions of OS/2.
                                                       FALSE indicates that this is an LVM specific volume and can not be used without OS2LVM.DMD. */
        BOOLEAN    On_PRM;                          /* Set to TRUE if this volume resides on a PRM.  Set to FALSE otherwise. */
        BYTE       Reserved[2];                     /* Alignment. */
    } Volume_Control_Record;

    /* The following structure is returned by the Get_Volume_Control_Data function. */
    typedef struct _Volume_Control_Array
    {
        Volume_Control_Record *  Volume_Control_Data;      /* An array of volume control records. */
        CARDINAL32               Count;                    /* The number of entries in the array of volume control records. */
    } Volume_Control_Array;

    /* The following information about a volume can (and often does) vary. */
    typedef struct _Volume_Information_Record
    {
        CARDINAL32 Volume_Size;                           /* The number of sectors comprising the volume. */
        CARDINAL32 Partition_Count;                       /* The number of partitions which comprise this volume. */
        CARDINAL32 Drive_Letter_Conflict;                 /* 0 indicates that the drive letter preference for this volume is unique.
                                                             1 indicates that the drive letter preference for this volume
                                                               is not unique, but this volume got its preferred drive letter anyway.
                                                             2 indicates that the drive letter preference for this volume
                                                               is not unique, and this volume did NOT get its preferred drive letter.
                                                             4 indicates that this volume is currently "hidden" - i.e. it has
                                                               no drive letter preference at the current time.                        */
        BOOLEAN    Compatibility_Volume;                  /* TRUE if this is for a compatibility volume, FALSE otherwise. */
        BOOLEAN    Bootable;                              /* Set to TRUE if this volume appears on the Boot Manager menu, or if it is
                                                             a compatibility volume and its corresponding partition is the first active
                                                             primary partition on the first drive.                                         */
        char       Drive_Letter_Preference;               /* The drive letter that this volume desires to be. */
        BYTE       Status;                                /* 0 = None.
                                                             1 = Bootable
                                                             2 = Startable
                                                             3 = Installable.           */
        char       Volume_Name[VOLUME_NAME_SIZE];         /* The user assigned name for this volume. */
        char       File_System_Name[FILESYSTEM_NAME_SIZE];/* The name of the filesystem in use on this partition, if it is known. */
    } Volume_Information_Record;

    /* The following structure defines an item on the Boot Manager Menu. */
    typedef struct _Boot_Manager_Menu_Item
    {
        PVOID     Handle;            /* A Volume or Partition handle. */
        BOOLEAN     Volume;            /* If TRUE, then Handle is the handle of a Volume.  Otherwise, Handle is the handle of a partition. */
    } Boot_Manager_Menu_Item;

    /* The following structure is used to get a list of the items on the
       partition manager menu. */
    typedef struct _Boot_Manager_Menu
    {
        Boot_Manager_Menu_Item *  Menu_Items;
        CARDINAL32                Count;
    } Boot_Manager_Menu;

    /* The following preprocessor directives define the operations that
       can be performed on a partition, volume, or a block of free space.
       These definitions represent bits in a 32 bit value returned by the
       Get_Valid_Options function. */

    #define CREATE_PRIMARY_PARTITION           1
    #define CREATE_LOGICAL_DRIVE               2
    #define DELETE_PARTITION                   4
    #define SET_ACTIVE_PRIMARY                 8
    #define SET_PARTITION_ACTIVE              0x10
    #define SET_PARTITION_INACTIVE            0x20
    #define SET_STARTABLE                     0x40
    #define INSTALL_BOOT_MANAGER              0x80
    #define REMOVE_BOOT_MANAGER               0x100
    #define SET_BOOT_MANAGER_DEFAULTS         0x200
    #define ADD_TO_BOOT_MANAGER_MENU          0x400
    #define REMOVE_FROM_BOOT_MANAGER_MENU     0x800
    #define DELETE_VOLUME                     0x1000
    #define HIDE_VOLUME                       0x2000
    #define EXPAND_VOLUME                     0x4000
    #define SET_VOLUME_INSTALLABLE            0x8000
    #define ASSIGN_DRIVE_LETTER               0x10000
    #define CAN_BOOT_PRIMARY                  0x20000      /* If a primary is created from this block of free space, then it can be made bootable. */
    #define CAN_BOOT_LOGICAL                  0x40000      /* If a logical drive is created from this block of free space, then OS/2 can boot from it by adding it to the boot manager menu. */
    #define CAN_SET_NAME                      0x80000

    /* The following enumeration defines the allocation strategies used
       by the Create_Partition function. */
    typedef enum _Allocation_Algorithm
    {
        Automatic,               /* Let LVM decide which block of free space to use to create the partition. */
        Best_Fit,                /* Use the block of free space which is closest in size to the partition being created. */
        First_Fit,               /* Use the first block of free space on the disk which is large enough to hold a partition of the specified size. */
        Last_Fit,                /* Use the last block of free space on the disk which is large enough to hold a partition of the specified size. */
        From_Largest,            /* Find the largest block of free space and allocate the partition from that block of free space. */
        From_Smallest,           /* Find the smallest block of free space that can accommodate a partition of the size specified. */
        All                      /* Turn the specified drive or block of free space into a single partition. */
    } Allocation_Algorithm;

    /* Error codes returned by the LVM Engine. */
    #define LVM_ENGINE_NO_ERROR                           0
    #define LVM_ENGINE_OUT_OF_MEMORY                      1
    #define LVM_ENGINE_IO_ERROR                           2
    #define LVM_ENGINE_BAD_HANDLE                         3
    #define LVM_ENGINE_INTERNAL_ERROR                     4
    #define LVM_ENGINE_ALREADY_OPEN                       5
    #define LVM_ENGINE_NOT_OPEN                           6
    #define LVM_ENGINE_NAME_TOO_BIG                       7
    #define LVM_ENGINE_OPERATION_NOT_ALLOWED              8
    #define LVM_ENGINE_DRIVE_OPEN_FAILURE                 9
    #define LVM_ENGINE_BAD_PARTITION                     10
    #define LVM_ENGINE_CAN_NOT_MAKE_PRIMARY_PARTITION    11
    #define LVM_ENGINE_TOO_MANY_PRIMARY_PARTITIONS       12
    #define LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE        13
    #define LVM_ENGINE_REQUESTED_SIZE_TOO_BIG            14
    #define LVM_ENGINE_1024_CYLINDER_LIMIT               15
    #define LVM_ENGINE_PARTITION_ALIGNMENT_ERROR         16
    #define LVM_ENGINE_REQUESTED_SIZE_TOO_SMALL          17
    #define LVM_ENGINE_NOT_ENOUGH_FREE_SPACE             18
    #define LVM_ENGINE_BAD_ALLOCATION_ALGORITHM          19
    #define LVM_ENGINE_DUPLICATE_NAME                    20
    #define LVM_ENGINE_BAD_NAME                          21
    #define LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE       22
    #define LVM_ENGINE_NO_DRIVES_FOUND                   23
    #define LVM_ENGINE_WRONG_VOLUME_TYPE                 24
    #define LVM_ENGINE_VOLUME_TOO_SMALL                  25
    #define LVM_ENGINE_BOOT_MANAGER_ALREADY_INSTALLED    26
    #define LVM_ENGINE_BOOT_MANAGER_NOT_FOUND            27
    #define LVM_ENGINE_INVALID_PARAMETER                 28
    #define LVM_ENGINE_BAD_FEATURE_SET                   29
    #define LVM_ENGINE_TOO_MANY_PARTITIONS_SPECIFIED     30
    #define LVM_ENGINE_LVM_PARTITIONS_NOT_BOOTABLE       31
    #define LVM_ENGINE_PARTITION_ALREADY_IN_USE          32
    #define LVM_ENGINE_SELECTED_PARTITION_NOT_BOOTABLE   33
    #define LVM_ENGINE_VOLUME_NOT_FOUND                  34
    #define LVM_ENGINE_DRIVE_NOT_FOUND                   35
    #define LVM_ENGINE_PARTITION_NOT_FOUND               36
    #define LVM_ENGINE_TOO_MANY_FEATURES_ACTIVE          37
    #define LVM_ENGINE_PARTITION_TOO_SMALL               38
    #define LVM_ENGINE_MAX_PARTITIONS_ALREADY_IN_USE     39
    #define LVM_ENGINE_IO_REQUEST_OUT_OF_RANGE           40
    #define LVM_ENGINE_SPECIFIED_PARTITION_NOT_STARTABLE 41
    #define LVM_ENGINE_SELECTED_VOLUME_NOT_STARTABLE     42
    #define LVM_ENGINE_EXTENDFS_FAILED                   43
    #define LVM_ENGINE_REBOOT_REQUIRED                   44

    /* ******************************************************************
     *
     *   LVM API prototypes
     *
     ********************************************************************/

    /* LVM.DLL exports both 16 bit and 32 bit functions.
       The 16 bit functions are just wrappers for the 32 bit functions.
       Their purpose is to provide 16 bit entry points for 16 bit components
       using LVM.DLL (such as Base Install).
       They basically thunk their input arguments and then call the
       corresponding 32 bit function.  The following 32 bit functions are
       defined by LVM.DLL for use by other components: */

    /* ******************************************************************
     *
     *   Functions relating to the LVM Engine itself
     *
     ********************************************************************/

    /*
     *@@ Open_LVM_Engine:
     *      opens the LVM Engine and readies it for use.
     *
     *      Input:
     *      -- BOOLEAN Ignore_CHS:
     *              If TRUE, then the LVM engine will not check the CHS values in the
     *              MBR/EBR partition tables for validity.  This is useful if there
     *              are drive geometry problems, such as the drive was partitioned and
     *              formatted with one geometry and then moved to a different machine
     *              which uses a different geometry for the drive.  This would cause
     *              the starting and ending CHS values in the partition tables to
     *              be inconsistent with the size and partition offset entries in the
     *              partition tables.  Setting Ignore_CHS to TRUE will disable the
     *              LVM Engine's CHS consistency checks, thereby allowing the drive
     *              to be partitioned.
     *      -- CARDINAL32 * Error_Code:
     *              The address of a CARDINAL32 in which to store an error code,
     *              should an error occur.
     *
     *      Error Handling:
     *
     *      If this function aborts with an error, all memory allocated during the course
     *      of this function will be released.  Disk read errors will be reported to the
     *      user via pop-up error messages.  Disk read errors will only cause this
     *      function to abort if none of the disk drives in the system could be
     *      successfully read.
     *
     *      Side Effects:
     *
     *      The LVM Engine will be initialized.  The partition tables for all OS2DASD
     *      controlled disk drives will be read into memory.  Memory will be allocated for
     *      the data structures used by the LVM Engine.
     */

    void _System Open_LVM_Engine(BOOLEAN Ignore_CHS,
                                 CARDINAL32* Error_Code);

    /*
     *@@ Commit_Changes:
     *      saves any changes made to the partitioning information
     *      of the OS2DASD controlled disk drives in the system.
     *
     *      Output:
     *
     *      The function return value will be TRUE if all of the
     *      partitioning/volume changes made were successfully
     *      written to disk.  Also, *Error_Code will be 0 if no
     *      errors occur.
     *
     *      If an error occurs, then the furnction return value
     *      will be FALSE and *Error_Code will contain a non-zero
     *      error code.
     *
     *      Error Handling:
     *
     *      If an error occurs, the function return value
     *      will be false and *Error_Code will be > 0.
     *
     *      Disk read and write errors will be indicated by setting
     *      the IO_Error field of the Drive_Information_Record to TRUE.
     *      Thus, if the function return value is FALSE, and *Error_Code
     *      indicates an I/O error, the caller of this function should
     *      call the Get_Drive_Status function on each drive to determine
     *      which drives had I/O errors.
     *
     *      If a read or write error occurs, then the engine may not
     *      have been able to create a partition or volume.  Thus,
     *      the caller may want to refresh all partition and volume
     *      data to see what the engine was and was not able to create.
     *
     *      Side Effects:
     *
     *      The partitioning information of the disk drives in the system
     *      may be altered.
     */

    BOOLEAN _System Commit_Changes(CARDINAL32* Error_Code);


    /*
     *@@ Close_LVM_Engine:
     *      closes the LVM Engine and frees any memory held by the
     *      LVM Engine.
     */

    void _System Close_LVM_Engine ( void );


    /* ******************************************************************
     *
     *   Functions relating to Drives
     *
     ********************************************************************/

    /*
     *@@ Get_Drive_Control_Data:
     *      This function returns an array of Drive_Control_Records.
     *      These records provide important information about the
     *      drives in the system and provide the handles required to
     *      access them.
     *
     *      Output:
     *
     *      A Drive_Control_Array structure is returned.  If no
     *      errors occur, Drive_Control_Data will be non-NULL,
     *      Count will be greater than zero, and *Error_Code will
     *      be 0.
     *
     *      Error Handling:
     *
     *      If an error occurs, the Drive_Control_Array returned
     *      by this function will have NULL for Drive_Control_Data,
     *      and 0 for Count. *Error_Code will be greater than 0.
     *
     *      Notes:
     *
     *      The caller becomes responsible for the memory allocated
     *      for the array of Drive_Control_Records pointed to by
     *      Drive_Control_Data pointer in the Drive_Control_Array
     *      structure returned by this function.  The caller should
     *      free this memory when they are done using it.
     */

    Drive_Control_Array _System Get_Drive_Control_Data( CARDINAL32 * Error_Code );

    /*
     *@@ Get_Drive_Status:
     *      returns the Drive_Information_Record for the drive
     *      specified by Drive_Handle.
     *
     *      Input:
     *
     *      -- PVOID Drive_Handle: The handle of the drive to use.
     *                    Drive handles are obtained through the
     *                    Get_Drive_Control_Data function.
     *
     *      Output:
     *
     *      This function returns the Drive_Information_Record for
     *      the drive associated with the specified Drive_Handle.
     *      If no errors occur, *Error_Code will be set to 0.  If an
     *      error does occur, then *Error_Code will be non-zero.
     *
     *      Error Handling:
     *
     *      If an error occurs, then *Error_Code will be non-zero.
     */

    Drive_Information_Record _System Get_Drive_Status( PVOID Drive_Handle, CARDINAL32 * Error_Code );

    /* ******************************************************************
     *
     *   Functions relating to Partitions
     *
     ********************************************************************/

    /*
     *@@ Get_Partitions:
     *      returns an array of partitions associated with the
     *      object specified by Handle.
     *
     *      Input:
     *
     *      -- PVOID Handle: This is the handle of a drive or volume.
     *             Drive handles are obtained through the
     *             Get_Drive_Control_Data function.  Volume
     *             handles are obtained through the
     *             Get_Volume_Control_Data function.
     *
     *      Output:
     *
     *      This function returns a structure.  The structure has
     *      two components: an array of partition information
     *      records and the number of entries in the array.  If
     *      Handle is the handle of a disk drive, then the returned
     *      array will contain a partition information record for
     *      each partition and block of free space on that drive.
     *      If Handle is the handle of a volume, then the returned
     *      array will contain a partition information record for
     *      each partition which is part of the specified volume.
     *
     *      If no errors occur, then *Error_Code will be 0.  If an
     *      error does occur, then *Error_Code will be non-zero.
     *
     *      Error Handling:
     *
     *      Any memory allocated for the return value will be freed.
     *      The Partition_Information_Array returned by this function
     *      will contain a NULL pointer for Partition_Array, and have a
     *      Count of 0.  *Error_Code will be non-zero.
     *
     *      If Handle is non-NULL and is invalid, a trap is likely.
     *
     *      Side Effects:
     *
     *      Memory will be allocated to hold the array returned by this
     *      function.
     *
     *      Notes:
     *
     *      The caller becomes responsible for the memory allocated
     *      for the array of Partition_Information_Records pointed
     *      to by Partition_Array pointer in the
     *      Partition_Information_Array structure returned by this
     *      function.  The caller should free this memory when they
     *      are done using it.
     */

    Partition_Information_Array _System Get_Partitions( PVOID Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Get_Partition_Handle:
     *      returns the handle of the partition whose serial number
     *      matches the one provided.
     *
     *      Input:
     *
     *      -- CARDINAL32 Serial_Number: This is the serial number to
     *          look for.  If a partition with a matching serial number
     *          is found, its handle will be returned.
     *
     *      Output:
     *
     *      If a partition with a matching serial number is found,
     *      then the function return value will be the handle
     *      of the partition found.  If no matching partition is
     *      found, then the function return value will be NULL.
     *
     *      Error Handling:
     *
     *      If no errors occur, *Error_Code will be LVM_ENGINE_NO_ERROR.
     *      If an error occurs, then *Error_Code will be a non-zero error
     *      code.
     */

    PVOID _System Get_Partition_Handle( CARDINAL32 Serial_Number, CARDINAL32 * Error_Code );

    /*
     *@@ Get_Partition_Information:
     *      returns the Partition_Information_Record for the partition
     *      specified by Partition_Handle.
     *
     *      Input:
     *
     *      -- PVOID Partition_Handle:
     *          The handle associated with the partition for which the
     *          Partition_Information_Record is desired.
     *
     *      Output:
     *      A Partition_Information_Record is returned.  If there
     *      is no error, then *Error_Code will be 0.  If an error
     *      occurs, *Error_Code will be non-zero.
     *
     *      Error Handling:
     *      If the Partition_Handle is not a valid handle, a trap
     *      could result.  If it is a handle for something other than
     *      a partition, an error code will be returned in *Error_Code.
     */

    Partition_Information_Record  _System Get_Partition_Information( PVOID Partition_Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Create_Partition:
     *      creates a partition on a disk drive.
     *
     *      Input:
     *
     *      --  PVOID Handle: The handle of a disk drive or a block
     *          of free space.
     *
     *      --  CARDINAL32 Size: The size, in sectors, of the
     *          partition to create.
     *
     *      --  char Name[]: The name to give to the newly created
     *          partition.
     *
     *      --  Allocation_Algorithm algorithm: If Handle is a drive,
     *          then the engine will find a block of free space to use
     *          to create the partition.  This tells the engine which
     *          memory management algorithm to use.
     *
     *      --  BOOLEAN Bootable: If TRUE, then the engine will
     *          only create the partition if it can be booted from.
     *          If Primary_Partition is FALSE, then it is assumed that
     *          OS/2 is the operating system that will be booted.
     *
     *      --  BOOLEAN Primary_Partition: If TRUE, then the engine
     *          will create a primary partition. If FALSE, then the
     *          engine will create a logical drive.
     *
     *      --  BOOLEAN Allocate_From_Start: If TRUE, then the engine
     *          will allocate the new partition from the beginning of
     *          the selected block of free space.  If FALSE, then the
     *          partition will be allocated from the end of the selected
     *          block of free space.
     *
     *      Output:
     *
     *      The function return value will be the handle of the partition
     *      created.  If the partition could not be created, then NULL will
     *      be returned. *Error_Code will be 0 if the partition was created.
     *      *Error_Code will be > 0 if the partition could not be created.
     *
     *      Error Handling:
     *
     *      If the partition can not be created, then any memory allocated
     *      by this function will be freed and the partitioning of the disk in
     *      question will be unchanged.
     *
     *      If Handle is not a valid handle, then a trap may result.
     *
     *      If Handle represents a partition or volume, then the function
     *      will abort and set *Error_Code to a non-zero value.
     *
     *      Side Effects:
     *
     *      A partition may be created on a disk drive.
     */

    PVOID _System Create_Partition( PVOID               Handle,
                                      CARDINAL32            Size,
                                      char                  Name[ PARTITION_NAME_SIZE ],
                                      Allocation_Algorithm  algorithm,
                                      BOOLEAN               Bootable,
                                      BOOLEAN               Primary_Partition,
                                      BOOLEAN               Allocate_From_Start,
                                      CARDINAL32 *          Error_Code
                                    );

    /*
     *@@ Delete_Partition:
     *      deletes the partition specified by Partition_Handle.
     *
     *      Input:
     *
     *      --  PVOID Partition_Handle: The handle associated with the*
     *          partition to be deleted.
     *
     *      Output:
     *      *Error_Code will be 0 if the partition was deleted successfully.
     *      *Error_Code will be > 0 if the partition could not be deleted.
     *
     *      Error Handling:
     *      If the partition can not be deleted, then *Error_Code will be > 0.
     *
     *      If Partition_Handle is not a valid handle, a trap may result.
     *
     *      If Partition_Handle is a volume or drive handle, then this
     *      function will abort and set *Error_Code to a non-zero value.
     *
     *      Side Effects:
     *      A partition on a disk drive may be deleted.
     *
     *      Notes:  A partition can not be deleted if it is part of a
     *      volume!
     */

    void _System Delete_Partition( PVOID Partition_Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Set_Active_Flag:
     *      sets the Active Flag field for a partition.
     *
     *      Input:
     *
     *      --  PVOID Partition_Handle: The handle of the partition
     *          whose Active Flag is to be set.
     *
     *      --  BYTE Active_Flag: The new value for the Active Flag.
     *
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the Active Flag was successfully set,
     *      otherwise *Error_Code will contain a non-zero error code
     *      indicating what went wrong.
     *
     *      Error Handling:
     *
     *      If the Active Flag can not be set, this function will abort
     *      without changing any disk structures.
     *
     *      If Partition_Handle is not a valid handle, a trap may result.
     *
     *      If Partition_Handle is a volume or drive handle, then this
     *      function will abort and set *Error_Code to a non-zero value.
     *
     *      Side Effects:
     *
     *      The Active Flag for a partition may be modified.
     */

    void _System Set_Active_Flag ( PVOID      Partition_Handle,
                                   BYTE         Active_Flag,
                                   CARDINAL32 * Error_Code
                                 );

    /*
     *@@ Set_OS_Flag:
     *      sets the OS Flag field for a partition.  This field
     *      is typically used to indicate the filesystem used on
     *      the partition, which generally gives an indication of
     *      which OS is using that partition.
     *
     *      Input:
     *
     *      --  PVOID Partition_Handle: The handle of the partition
     *          whose Active Flag is to be set.
     *      --  BYTE OS_Flag - The new value for the OS Flag.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the OS Flag was successfully
     *      set, otherwise *Error_Code will contain a non-zero error
     *      code indicating what went wrong.
     *
     *      Error Handling:
     *
     *      If the OS Flag can not be set, this function will abort
     *      without changing any disk structures.
     *
     *      If Partition_Handle is not a valid handle, a
     *      trap may result.
     *
     *      If Partition_Handle is a volume or drive handle,
     *      then this function will abort and set
     *      *Error_Code to a non-zero value.
     *
     *      Side Effects:
     *
     *      The OS Flag for a partition may be modified.
     */

    void _System Set_OS_Flag ( PVOID      Partition_Handle,
                               BYTE         OS_Flag,
                               CARDINAL32 * Error_Code
                             );

    /* ******************************************************************
     *
     *   Functions relating to Volumes
     *
     ********************************************************************/

    /*
     *@@ Get_Volume_Control_Data:
     *      returns a structure containing an array of Volume_Control_Records.
     *      These records contain information about volumes which is
     *      invariant - i.e. will not change for as long as the volume exists.
     *      One of the items in the Volume_Control_Record is the handle for
     *      the volume.  This handle must be used on all accesses to the volume.
     *
     *
     *      Output:
     *
     *      A Volume_Control_Array structure is returned.
     *
     *      If there are no errors, then the Volume_Control_Data
     *      pointer in the Volume_Control_Array will be non-NULL,
     *      the Count field of the Volume_Control_Array will be
     *      >= 0, and *Error_Code will be 0.
     *
     *      If an error does occur, then the Volume_Control_Data
     *      pointer in the the Volume_Control_Array will be NULL,
     *      the Count field of the Volume_Control_Array will be 0,
     *      and *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If an error occurs, then any memory allocated by this
     *      function will be freed.
     *
     *      Side Effects:
     *      Memory for the returned array is allocated.
     *
     *      Notes:  The caller becomes responsible for the memory allocated
     *      for the array of Volume_Control_Records pointed to by
     *      Volume_Control_Data pointer in the Volume_Control_Array
     *      structure returned by this function.  The caller should
     *      free this memory when they are done using it.
     */

    Volume_Control_Array _System Get_Volume_Control_Data( CARDINAL32 * Error_Code );


    /*
     *@@ Get_Volume_Information:
     *      returns the Volume_Information_Record for the volume
     *      associated with Volume_Handle.
     *
     *      Input:
     *
     *      --  PVOID Volume_Handle: The handle of the volume about
     *          which information is desired.
     *
     *      Output: This function returns a Volume_Information_Record.
     *
     *      If this function is successful, then *Error_Code will be 0.
     *
     *      If this function fails, then *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If Volume_Handle is not a valid handle, a trap will be likely.
     *      If Volume_Handle is a drive or partition handle, *Error_Code
     *      will be > 0.
     */

    Volume_Information_Record _System Get_Volume_Information( PVOID Volume_Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Create_Volume:
     *      creates a volume from a list of partitions.  The partitions
     *      are specified by their corresponding handles.
     *
     *      Input:
     *      --  char Name[]: The name to assign to the newly created volume.
     *
     *      --  BOOLEAN Create_LVM_Volume: If TRUE, then an LVM volume is
     *          created, otherwise a compatibility volume is created.
     *
     *      --  BOOLEAN Bootable: If TRUE, the volume will not be created
     *          unless OS/2 can be booted from it.
     *
     *      --  char Drive_Letter_Preference: This is the drive letter to
     *          use for accessing the newly created volume.
     *
     *      --  CARDINAL32 FeaturesToUse: This is currently reserved for
     *          future use and should always be set to 0.
     *
     *      --  CARDINAL32 Partition_Count: The number of partitions to
     *          link together to form the volume being created.
     *
     *      --  PVOID Partition_Handles[]: An array of partition handles
     *          with one entry for each partition that is to become part
     *          of the volume being created.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the volume was created. *Error_Code will
     *      be > 0 if the volume could not be created.
     *
     *      Error Handling:
     *
     *      If any of the handles in the partition handles array is not valid,
     *      then a trap is likely.  If Partition_Count is greater than the
     *      number of entries in the partition handles array, then a trap is
     *      likely.  If any of the handles in the partition array are not
     *      partition handles, then *Error_Code will be > 0.  If the volume
     *      can NOT be created, then *Error_Code will be > 0 and any
     *      memory allocated by this function will be freed. If the volume
     *      can NOT be created, then the existing partition/volume structure
     *      of the disk will be unchanged.
     *
     *      Side Effects:
     *
     *      A volume may be created.
     */

    void _System Create_Volume( char         Name[VOLUME_NAME_SIZE],
                                BOOLEAN      Create_LVM_Volume,
                                BOOLEAN      Bootable,
                                char         Drive_Letter_Preference,
                                CARDINAL32   FeaturesToUse,
                                CARDINAL32   Partition_Count,
                                PVOID      Partition_Handles[],
                                CARDINAL32 * Error_Code
                              );

    /*
     *@@ Delete_Volume:
     *      deletes the volume specified by Volume_Handle.
     *
     *      Input:
     *
     *      -- PVOID Volume_Handle: The handle of the volume to
     *                         delete.  All partitions which are
     *                         part of the specified volume will
     *                         be deleted also.
     *
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the volume and its partitions
     *      are successfully deleted.  Otherwise, *Error_Code will
     *      be > 0.
     *
     *      Error Handling:
     *
     *      *Error_Code will be > 0 if an error occurs.  If the
     *      volume or any of its partitions can not be deleted,
     *      then any changes made by this function will be undone.
     *
     *      If Volume_Handle is not a valid handle, a trap may result.
     *
     *      If Volume_Handle is a partition or drive handle, then
     *      this function will abort and set *Error_Code to a non-zero value.
     *
     *      Side Effects:
     *
     *      A volume and its partitions may be deleted. System memory
     *      may be freed as the internal structures used to track the
     *      deleted volume are no longer required.
     */

    void _System Delete_Volume( PVOID Volume_Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Hide_Volume:
     *      "hides" a volume from OS/2 by removing its drive letter
     *      assignment.  Without a drive letter assignment, OS/2 can not
     *      access (or "see") the  volume.
     *
     *      Input:
     *
     *      -- PVOID Volume_Handle: The handle of the volume to hide.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the volume was successfully
     *      hidden.  If the volume could not be hidden, then
     *      *Error_Code will be > 0.
     *
     *      Error Handling:
     *      *Error_Code will be > 0 if the volume can not be hidden.
     *      If the volume can not be hidden, then nothing will be altered.
     *
     *      If Volume_Handle is not a valid handle, a trap may result.
     *
     *      If Volume_Handle is a partition or drive handle, then this
     *      function will abort and set *Error_Code to a non-zero value.
     */

    void _System Hide_Volume( PVOID Volume_Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Expand_Volume:
     *      expands an existing volume by linking additional partitions to it.
     *
     *      Input:
     *
     *      --  PVOID Volume_Handle: The handle of the volume to be
     *          expanded.
     *
     *      --  CARDINAL32 Partition_Count - The number of partitions or
     *          volumes to be added to the volume being expanded.
     *
     *      -- PVOID Partition_Handles[] - An array of handles.  Each
     *          handle in the array is the handle of a partition which
     *          is to be added to the volume being expanded.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the volume is successfully expanded.
     *      If the volume can not be expanded, *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If the volume can not be expanded, the state of the volume
     *      is unchanged and any memory allocated by this function is freed.
     *
     *      If Volume_Handle is not a valid handle, a trap may result.
     *
     *      If Volume_Handle is a partition or drive handle, then this
     *      function will abort and set *Error_Code to a non-zero value.
     *
     *      If any of the partition handles in the Partition_handles
     *      array are not valid handles, then a trap may result.
     *
     *      If any of the partition handles in the Partition_Handles
     *      array are actually drive handles, then this function will
     *      abort and set *Error_Code to a non-zero value.
     *
     *      If Partition_Count is greater than the number of entries in
     *      the Partition_Handles array, a trap may result.
     *
     *      Side Effects:
     *
     *      A volume may be expanded.  If the volume is expanded using
     *      another volume, the partitions on the second volume will be
     *      linked to those of the first volume and all data on the second
     *      volume will be lost.
     */

    void _System Expand_Volume ( PVOID         Volume_Handle,
                                 CARDINAL32      Partition_Count,
                                 PVOID         Partition_Handles[],
                                 CARDINAL32 *    Error_Code
                               );

    /*
     *@@ Assign_Drive_Letter:
     *      assigns a drive letter to a volume.
     *
     *      Input:
     *
     *      --  PVOID Volume_Handle: The handle of the volume which
     *          is to have its assigned drive letter changed.
     *
     *      --  char New_Drive_Preference: The new drive letter to
     *          assign to the volume.
     *
     *      Output:
     *      *Error_Code will be 0 if the drive letter was assigned
     *      successfully; otherwise *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If the drive letter assignment can not be made, the
     *      volume will not be altered.
     *
     *      If Volume_Handle is not a valid handle, a trap may result.
     *
     *      If Volume_Handle is a partition or drive handle, then
     *      this function will abort and set *Error_Code to a non-zero
     *      value.
     *
     *      Side Effects:
     *
     *      A volume may have its drive letter assignment changed.
     *
     *      Notes: If the drive letter being assigned is already in use by
     *      volume which does not lie on removable media, then the
     *      drive assignment will NOT be made.
     */

    void _System Assign_Drive_Letter( PVOID      Volume_Handle,
                                      char         New_Drive_Preference,
                                      CARDINAL32 * Error_Code
                                    );

    /*
     *@@ Set_Installable:
     *      marks a volume as being the volume to install OS/2 on.
     *
     *      Input:
     *
     *      --  PVOID Volume_Handle: The handle of the volume to which
     *          OS/2 should be installed.
     *
     *      Output:
     *
     *      If the volume is successfully marked as installable,
     *      *Error_Code will be 0; otherwise *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If Volume_Handle is not a valid handle, a trap may result.
     *
     *      If Volume_Handle is a partition or drive handle, then this
     *      function will abort and set *Error_Code to a non-zero value.
     *
     *      Side Effects:
     *
     *      The specified volume may be marked as installable.
     */

    void _System Set_Installable ( PVOID Volume_Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Get_Installable_Volume:
     *      returns the volume currently marked as installable.
     *
     *      Output:
     *
     *      If a volume is marked installable, its information will be
     *      returned and *Error_Code will be LVM_ENGINE_NO_ERROR.
     *      If there is no volume marked installable, then
     *      *Error_Code will be > 0.
     */

    Volume_Information_Record _System Get_Installable_Volume ( CARDINAL32 * Error_Code );

    /* ******************************************************************
     *
     *   Functions relating to Partitions, Drives, and Volumes
     *
     ********************************************************************/

    /*
     *@@ Set_Name:
     *      sets the name of a volume, drive, or partition.
     *
     *      Input:
     *
     *      --  PVOID Handle: The handle of the drive, partition, or
     *          volume which is to have its name set.
     *
     *      --  char New_Name[]: The new name for the drive/partition/volume.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the name is set as specified.
     *      If the name can not be set, *Error_Code will be > 0.
     *
     *      Error Handling:
     *      If the name can not be set, then drive/volume/partition is
     *      not modified.
     *
     *      If Handle is not a valid handle, a trap may result.
     *
     *      Side Effects: A drive/volume/partition may have its name set.
     *
     */

    void _System Set_Name ( PVOID      Handle,
                            char         New_Name[],
                            CARDINAL32 * Error_Code
                          );

    /*
     *@@ Set_Startable:
     *      sets the specified volume or partition startable.
     *      If a volume is specified, it must be a compatibility
     *      volume whose partition is a primary partition on the
     *      first drive.  If a partition is specified, it must be
     *      a primary partition on the first drive in the system.
     *
     *      Input:
     *
     *      --  PVOID Handle: The handle of the partition or volume
     *          which is to be set startable.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the specified volume or
     *      partition was set startable. If the name can not be set,
     *      *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If the volume or partition could not be set startable,
     *      then nothing in the system is changed.
     *
     *      If Handle is not a valid handle, a trap may result.
     *
     *      Side Effects:
     *
     *      Any other partition or volume which is marked startable
     *      will have its startable flag cleared.
     */

    void _System Set_Startable ( PVOID      Handle,
                                 CARDINAL32 * Error_Code
                               );

    /*
     *@@ Get_Valid_Options:
     *      returns a bitmap where each bit in the bitmap
     *      corresponds to a possible operation that the LVM
     *      Engine can perform.  Those bits which are 1 represent
     *      operations which can be performed on the item specified
     *      by Handle.  Those bits which are 0 are not allowed on the
     *      item specified by Handle.
     *
     *      Input:
     *
     *      --  PVOID Handle: This is any valid drive, volume, or
     *          partition handle.
     *
     *      Output:
     *
     *      A bitmap indicating which operations are valid on the
     *      item specified by Handle.
     *
     *      If no errors occur, *Error_Code will be 0, otherwise
     *      *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If Handle is not valid, a trap will be likely.
     *
     *      Notes:
     *
     *      The values of the various bits in the bitmap returned
     *      by this function are defined near the beginning of this
     *      file, immediately after all of the structure
     *      definitions.
     */

    CARDINAL32 _System Get_Valid_Options( PVOID Handle, CARDINAL32 * Error_Code );

    /* ******************************************************************
     *
     *   Functions relating to Boot Manager
     *
     ********************************************************************/

    /*
     *@@ Boot_Manager_Is_Installed:
     *      indicates whether or not Boot Manager is installed
     *      on the first or second hard drives in the system.
     *
     *      Output:
     *
     *      TRUE is returned if Boot Manager is found.
     *      FALSE is returned if Boot Manager is not found or if an
     *      error occurs.
     *      *Error_Code will be 0 if no errors occur; otherwise it
     *      will be > 0.
     */

    BOOLEAN _System Boot_Manager_Is_Installed( CARDINAL32 * Error_Code);

    /*
     *@@ Add_To_Boot_Manager:
     *      adds the volume/partition to the Boot Manager menu.
     *
     *      Input:
     *
     *      --  PVOID Handle: The handle of a partition or volume that
     *          is to be added to the Boot Manager menu.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the partition or volume was
     *      successfully added to the Boot Manager menu; otherwise
     *      *Error_Code will be > 0.
     *
     *      Error Handling:
     *      If the partition/volume can not be added to the Boot
     *      Manager menu, no action is taken and *Error_Code will
     *      contain a non-zero error code.
     *
     *      If Handle is not a valid handle, a trap may
     *      result.
     *
     *      If Handle represents a drive, then this function will
     *      abort and set *Error_Code to a non-zero value.
     *
     *      Side Effects:
     *
     *      The Boot Manager menu may be altered.
     */

    void _System Add_To_Boot_Manager ( PVOID Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Remove_From_Boot_Manager:
     *      removes the specified partition or volume from the
     *      Boot Manager menu.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the partition or volume was
     *      successfully removed to the Boot Manager menu;
     *      otherwise *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If Handle is not a valid handle, a trap may result.
     *
     *      If Handle represents a drive, or if Handle represents
     *      a volume or partition which is NOT on the boot manager
     *      menu, then this function will abort and set *Error_Code
     *      to a non-zero value.
     *
     *      Side Effects:
     *
     *      The Boot Manager menu may be altered.
     */

    void _System Remove_From_Boot_Manager ( PVOID Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Get_Boot_Manager_Menu:
     *      returns an array containing the handles of the partitions
     *      and volumes appearing on the Boot Manager menu.
     *
     *      Output:
     *
     *      The function returns a Boot_Manager_Menu structure.
     *      This structure contains two items: a pointer to an array
     *      of Boot_Manager_Menu_Items and a count of how many items
     *      are in the array.  Each Boot_Manager_Menu_Item contains
     *      a handle and a BOOLEAN variable to indicate whether the
     *      handle is for a partition or a volume.
     *
     *      If this function is successful, then *Error_Code will
     *      be 0.
     *
     *      If an error occurs, the Count field in the Boot_Manager_Menu
     *      will be 0 and the corresponding pointer will be NULL.  *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If an error occurs, *Error_Code will be > 0. Any memory
     *      allocated by this function will be freed.
     */

    Boot_Manager_Menu  _System Get_Boot_Manager_Menu ( CARDINAL32 * Error_Code);

    /*
     *@@ Install_Boot_Manager:
     *      this function installs Boot Manager.  It can be used
     *      to replace an existing Boot Manager as well.
     *
     *      Input:
     *
     *      --  CARDINAL32 Drive_Number: The number of the drive to
     *          install Boot Manager on.  Must be 1 or 2.
     *
     *      Output:
     *
     *      If this function is successful, then *Error_Code will be 0;
     *      otherwise it will be > 0.
     *
     *      Error Handling:
     *      If an error occurs, *Error_Code will be set to a non-zero
     *      value.  Depending upon the error, it is possible that the
     *      Boot Manager partition can be left in an unusuable state
     *      (such as for a write error).
     *
     *      Side Effects:
     *
     *      Boot Manager may be installed on drive 1 or 2. The MBR for
     *      drive 1 may be altered.
     */

    void _System Install_Boot_Manager ( CARDINAL32   Drive_Number, CARDINAL32 * Error_Code );

    /*
     *@@ Remove_Boot_Manager:
     *      removes Boot Manager from the system.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if Boot Manager was successfully
     *      removed from the system; otherwise *Error_Code will
     *      be 0.
     *
     *      Error Handling:
     *
     *      If an error occurs, *Error_Code will be > 0.
     *
     *      Side Effects:
     *      Boot Manager will be removed from the system.
     */

    void _System Remove_Boot_Manager( CARDINAL32 * Error_Code );

    /*
     *@@ Set_Boot_Manager_Options:
     *      sets the Boot Managers Options.  The options that can
     *      be set are: whether or not the time-out timer is
     *      active, how long the timer-out is, the partition to
     *      boot by default, and whether or not Boot Manager should
     *      display its menu using default mode or advanced mode.
     *
     *      Input:
     *
     *      --  PVOID Handle - The handle of the partition or volume
     *          to boot if the time-out timer is active and the time-out
     *          value is reached.
     *
     *      --  BOOLEAN Timer_Active - If TRUE, then the time-out timer
     *          is active.
     *
     *      --  CARDINAL32 Time_Out_Value - If the time-out timer is
     *          active, this is the time-out value, in seconds.
     *
     *      --  BOOLEAN Advanced_Mode - If TRUE, then Boot Manager will
     *          operate in advanced mode.  If FALSE, then normal mode
     *          will be in effect.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if no errors occur.  If an error
     *      does occur, then *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If an error occurs, no changes will be made to Boot Manager
     *      and *Error_Code will be set a non-zero error code.
     *
     *      Side Effects:
     *
     *      Boot Manager may be modified.
     */

    void _System Set_Boot_Manager_Options( PVOID      Handle,
                                           BOOLEAN      Timer_Active,
                                           CARDINAL32   Time_Out_Value,
                                           BOOLEAN      Advanced_Mode,
                                           CARDINAL32 * Error_Code
                                         );

    /*
     *@@ Get_Boot_Manager_Options:
     *      returns the current Boot Manager settings for the
     *      various Boot Manager options.
     *
     *      Input:
     *
     *      --  PVOID * Handle - The handle for the default boot
     *          volume or partition.
     *
     *      --  BOOLEAN * Handle_Is_Volume - If TRUE, then Handle
     *          represents a volume.  If FALSE, then Handle
     *          represents a partition.
     *
     *      --  BOOLEAN * Timer_Active - If TRUE, then the time-out
     *          timer is active.  If FALSE, then the time-out timer
     *          is not active.
     *
     *      --  CARDINAL32 * Time_Out_Value - If the time-out timer
     *          is active, then this is the number of seconds that
     *          Boot Manager will wait for user input before booting
     *          the default volume/partition.
     *
     *      --  BOOLEAN * Advanced_Mode - If TRUE, the Boot Manager is
     *          operating in advanced mode.  If FALSE, then Boot Manager
     *          is operating in normal mode.
     *
     *      Output:
     *
     *      *Handle, *Handle_Is_Volume, *Timer_Active, *Time_out_value,
     *      *Advanced_Mode, and *Error_Code are all set by this function.
     *      If there are no errors, then *Error_Code will be set to 0.
     *      If any errors occur, then *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If any of the parameters are invalid, then a trap is likely.
     *      If Boot Manager is not installed, then *Error_Code will be > 0.
     */

    void _System Get_Boot_Manager_Options( PVOID    *  Handle,
                                           BOOLEAN    *  Handle_Is_Volume,
                                           BOOLEAN    *  Timer_Active,
                                           CARDINAL32 *  Time_Out_Value,
                                           BOOLEAN    *  Advanced_Mode,
                                           CARDINAL32 * Error_Code
                                         );

    /* ******************************************************************
     *
     *   Other Functions
     *
     ********************************************************************/

    /*
     *@@ Free_Engine_Memory:
     *      frees a memory object created by LVM.DLL and returned
     *      to a user of LVM.DLL.
     *
     *      Input:
     *
     *      --  PVOID Object: The address of the memory object to
     *          free.  This could be the Drive_Control_Data field of
     *          a Drive_Control_Record, the Partition_Array field of
     *          a Partition_Information_Array structure, or any other
     *          dynamically allocated memory object created by LVM.DLL
     *          and returned by a function in LVM.DLL.
     *
     *      Notes:
     *
     *      A trap or exception could occur if a bad address is passed
     *      into this function.
     */

    void _System Free_Engine_Memory( PVOID Object );

    /*
     *@@ New_MBR:
     *      this function lays down a new MBR on the specified drive.
     *
     *      Input:
     *
     *      --  PVOID Drive_Handle - The handle of the drive on which
     *          the new MBR is to be placed.
     *
     *      Output:
     *
     *      *Error_Code will be 0 if the new MBR was successfully
     *      placed on the specified drive.  If the operation failed
     *      for any reason, then *Error_Code will contain a non-zero
     *      error code.
     *
     *      Error Handling:
     *
     *      If an error occurs, then the existing MBR is not altered
     *      and *Error_Code will be > 0.
     *
     *      Side Effects:
     *
     *      A new MBR may be placed on the specified drive.
     */

    void _System New_MBR( PVOID Drive_Handle, CARDINAL32 * Error_Code );

    /*
     *@@ Get_Available_Drive_Letters:
     *      returns a bitmap indicating which drive letters are
     *      available for use.
     *
     *      Output:
     *
     *      This function returns a bitmap of the available drive
     *      letters.  If this function is successful, then
     *      *Error_Code will be set to 0.  Otherwise, *Error_Code
     *      will be > 0 and the bitmap returned will have all bits
     *      set to 0.
     *
     *      Error Handling:
     *
     *      If an error occurs, *Error_Code will be > 0.
     *
     *      Notes:
     *      A drive letter is available if it is not associated
     *      with a volume located on a disk drive controlled
     *      by OS2DASD.
     */

    CARDINAL32 _System Get_Available_Drive_Letters ( CARDINAL32 * Error_Code ) ;

    /*
     *@@ Reboot_Required:
     *      this function indicates whether or not any changes
     *      were made to the partitioning of the disks in the
     *      system which would require a reboot to make functional.
     *
     *      Output:
     *
     *      The function return value will be TRUE if the system
     *      must be rebooted as a result of disk partitioning
     *      changes.
     */

    BOOLEAN _System Reboot_Required ( void );

    /*
     *@@ Set_Reboot_Flag:
     *      this function sets the Reboot Flag.  The Reboot
     *      Flag is a special flag on the boot disk used by
     *      the install program to keep track of whether or
     *      not the system was just rebooted.  It is used by
     *      the various phases of install.
     *
     *      Input:
     *
     *      --  BOOLEAN Reboot: The new value for the Reboot Flag.
     *          If TRUE, then the reboot flag will be set. If
     *          FALSE, then the reboot flag will be cleared.
     *
     *      Output:
     *
     *      *Error_Code will be set to 0 if there are no errors.
     *      *Error_Code will be > 0 if an error occurs.
     *
     *      Error Handling:
     *
     *      If an error occurs, then the value of the Reboot
     *      Flag will be unchanged.
     *
     *      Side Effects:
     *      The value of the Reboot Flag may be changed.
     */

    void _System Set_Reboot_Flag( BOOLEAN Reboot, CARDINAL32 * Error_Code );

    /*
     *@@ Get_Reboot_Flag:
     *      this function returns the value of the Reboot Flag.
     *      The Reboot Flag is a special flag on the boot disk
     *      used by the install program to keep track of whether
     *      or not the system was just rebooted.  It is used by
     *      the various phases of install.
     *
     *      Output:
     *
     *      The function return value will be TRUE if no errors
     *      occur and the Reboot Flag is set.  *Error_Code will be
     *      0 under these conditions.  If an error occurs, the
     *      function return value will be FALSE and *Error_Code
     *      will be > 0.
     *
     *      Error Handling:
     *
     *      If an error occurs, *Error_Code will be > 0. The value
     *      of the reboot flag will be unchanged.
     */

    BOOLEAN _System Get_Reboot_Flag( CARDINAL32 * Error_Code );

    /*
     *@@ Set_Install_Flags:
     *      this function sets the value of the Install Flags.
     *      The Install Flags reside in a 32 bit field in the
     *      LVM dataspace.  These flags are not used by LVM,
     *      thereby leaving Install free to use them for whatever it wants.
     *
     *      Input:
     *
     *      --  CARDINAL32 Install_Flags: The new value for the Install
     *          Flags.
     *
     *      Output:
     *
     *      *Error_Code will be set to 0 if there are no errors.
     *      *Error_Code will be > 0 if an error occurs.
     *
     *      Error Handling:
     *
     *      If an error occurs, then the value of the Install Flags
     *      will be unchanged.
     *
     *      Side Effects:
     *
     *      The value of the Install Flags may be changed.
     */

    void _System Set_Install_Flags( CARDINAL32 Install_Flags, CARDINAL32 * Error_Code );

    /*
     *@@ Get_Install_Flags:
     *      returns the value of the Install Flags.  The Install
     *      Flags reside in a 32 bit field in the LVM dataspace.
     *      These flags are not used by LVM, thereby leaving Install
     *      free to use them for whatever it wants.
     *
     *      Output:
     *
     *      The function returns the current value of the Install
     *      Flags stored in the LVM Dataspace.
     *      *Error_Code will be LVM_ENGINE_NO_ERROR if the function
     *      is successful.  If an error occurs, the function will
     *      return 0 and *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If an error occurs, *Error_Code will be > 0.
     */

    CARDINAL32 _System Get_Install_Flags( CARDINAL32 * Error_Code );

    /*
     *@@ Set_Min_Install_Size:
     *      this function tells the LVM Engine how big a
     *      partition/volume must be in order for it to marked
     *      installable.  If this function is not used to set the
     *      minimum size for an installable partition/volume, the
     *      LVM Engine will use a default value of 300 MB.
     *
     *      Input:
     *
     *      --  CARDINAL32 Min_Sectors: The minimum size, in sectors,
     *          that a partition must be in order for it to be marked
     *          as installable.
     */

    void _System Set_Min_Install_Size ( CARDINAL32  Min_Sectors );

    /*
     *@@ Set_Free_Space_Threshold:
     *      this function tells the LVM Engine not to report
     *      blocks of free space which are less than the size
     *      specified.  The engine defaults to not reporting
     *      blocks of free space which are smaller than 2048
     *      sectors (1 MB).
     *
     *      Input:
     *
     *      --  CARDINAL32 Min_Sectors: The minimum size, in sectors,
     *          that a block of free space must be in order for the
     *          LVM engine to report it.
     */

    void _System Set_Free_Space_Threshold( CARDINAL32  Min_Sectors );

    /*
     *@@ Read_Sectors:
     *      reads one or more sectors from the specified drive and
     *      places the data read in Buffer.
     *
     *      Input:
     *
     *      --  CARDINAL32 Drive_Number: The number of the hard drive
     *          to read from.  The drives in the system are numbered
     *          from 1 to n, where n is the total number of hard drives
     *          in the system.
     *
     *      --  LBA Starting_Sector: The first sector to read from.
     *
     *      --  CARDINAL32 Sectors_To_Read : The number of sectors to
     *          read into memory.
     *
     *      --  PVOID Buffer : The location to put the data read into.
     *
     *      Output:
     *
     *      If successful, then the data read will be placed in memory
     *      starting at Buffer, and *Error will be LVM_ENGINE_NO_ERROR.
     *
     *      If unsuccessful, then *Error will be > 0 and the contents
     *      of memory starting at Buffer is undefined.
     *
     *      Error Handling:
     *
     *      *Error will be > 0 if an error occurs.
     *
     *      Side Effects:
     *
     *      Data may be read into memory starting at Buffer.
     */

    void _System Read_Sectors ( CARDINAL32          Drive_Number,
                                LBA                 Starting_Sector,
                                CARDINAL32          Sectors_To_Read,
                                PVOID             Buffer,
                                CARDINAL32 *        Error);

    /*
     *@@ Write_Sectors:
     *      writes data from memory to one or more sectors on the
     *      specified drive.
     *
     *      Input:
     *
     *      --  CARDINAL32 Drive_Number: The number of the hard drive to
     *          write to.  The drives in the system are numbered from 1
     *          to n, where n is the total number of hard drives in the system.
     *
     *      --  LBA Starting_Sector : The first sector to write to.
     *
     *      --  CARDINAL32 Sectors_To_Read : The number of sectors to
     *          be written.
     *
     *      --  PVOID Buffer : The location of the data to be written
     *          to disk.
     *
     *      Output:
     *
     *      If successful, then the data at Buffer will be placed on the
     *      disk starting at the sector specified, and *Error will be
     *      LVM_ENGINE_NO_ERROR.
     *      If unsuccessful, then *Error will be > 0 and the contents
     *      of the disk starting at sector Starting_Sector is undefined.
     *
     *      Error Handling:
     *
     *      *Error will be > 0 if an error occurs.
     *
     *      Side Effects:
     *
     *      Data may be written to disk.
     */

    void _System Write_Sectors ( CARDINAL32          Drive_Number,
                                 LBA                 Starting_Sector,
                                 CARDINAL32          Sectors_To_Write,
                                 PVOID             Buffer,
                                 CARDINAL32 *        Error);

    /*
     *@@ Start_Logging:
     *      enables the LVM Engine logging.  Once enabled, the LVM
     *      Engine logging function will log all LVM Engine activity
     *      to the specified log file. The data is logged in a binary
     *      format for compactness and speed.
     *
     *      Input:
     *
     *      --  char * Filename: The filename of the file to use as the
     *          log file.
     *
     *      Output:
     *
     *      If the logging file was successfully created, then
     *      *Error_Code will be 0.  If the log file could not be
     *      created, then *Error_Code will be > 0.
     *
     *      Error Handling:
     *
     *      If the log file can not be created, then *Error_Code will
     *      be > 0.
     *
     *      Side Effects:
     *
     *      A file may be created/opened for logging of LVM Engine actions.
     */

    void _System Start_Logging( char * Filename, CARDINAL32 * Error_Code );

    /*
     *@@ Stop_Logging:
     *      this function ends LVM Engine logging and closes the log file.
     *
     *      Output: *Error_Code will be 0 if this function completes
     *      successfully; otherwise it will be > 0.
     *
     *      Error Handling: If the log file is not currently opened, or if
     *      the close operation fails on the log file, then
     *      *Error_Code will be > 0.
     *
     *      Side Effects:  The log file may be closed.
     */

    void _System Stop_Logging ( CARDINAL32 * Error_Code );

    /*
     *@@ Export_Configuration:
     *      this function creates a file containing all of the LVM
     *      commands necessary to recreate all partitions and volumes
     *      of this machine on another machine.
     *
     *      Input:
     *
     *      --  char * Filename: The pathname of the file to be created.
     *
     *      Output:
     *
     *      A file containing LVM commands.  *Error_Code will be 0
     *      if this function completed successfully; otherwise
     *      *Error_code will be > 0.
     *
     *      Error Handling:
     *
     *      If the output file can not be created, or if there is
     *      some other error, then *Error_Code will be > 0.
     *
     *      Side Effects:  A file may be created.
     */

    void _System Export_Configuration( char * Filename, CARDINAL32 * Error_Code );
        /* Creates a file containing LVM.EXE commands that can be used to replicate
        the partitioning of the current machine on another machine. */


#endif

#if __cplusplus
}
#endif

