/* static char *SCCSID = "@(#)1.1  7/30/96 11:01:11 src/jfs/common/include/extboot.h, sysjfs, c.jfs, fixbld";*/
#pragma pack(1)
/*
 * We need to duplicate the BPB structure defined in sysbloks.h.  Extended_BPB
 * is identical except without the six-byte reserved pad at the end of the
 * structure
 */
 /*
 *  12/12/01  d_267590 dimir - added support of new FAT16 partition
 *                           of type 0x0E (the same as type 0x06) and
 *                           new extended partition of type 0x0F
 *                           (the same as type 0x05). Partitions of
 *                           these types are located beyond 7.875GB limit.
 */


/* bios parameter block */

struct Extended_BPB {                /* bios parameter block             */
  unsigned short bytes_per_sector;   /* sector size                   2  */
  unsigned char sectors_per_cluster; /* sectors per allocation unit   1  */
  unsigned short reserved_sectors;   /* number of reserved sectors    2  */
  unsigned char nbr_fats;            /* number of fats                1  */
  unsigned short root_entries;       /* number of directory entries   2  */
  unsigned short total_sectors;      /* number of sectors             2  */
  unsigned char media_type;          /* fatid byte                    1  */
  unsigned short sectors_per_fat;    /* sectors in a copy of the FAT  2  */
  unsigned short sectors_per_track;  /* number of sectors per track   2  */
  unsigned short number_of_heads;    /* number of heads               2  */
  unsigned long hidden_sectors;      /* number of hidden sectors      4  */
  unsigned long large_total_sectors; /* large total sectors           4  */
};                                   /*     total byte size = 25         */

/* Extended Boot Structure */

struct Extended_Boot 
{
        unsigned char           Boot_jmp[3];
        unsigned char           Boot_OEM[8];
        struct Extended_BPB     Boot_BPB;
        unsigned char           Boot_DriveNumber;
        unsigned char           Boot_CurrentHead;
        unsigned char           Boot_Sig;       /* 41 indicates extended boot */
        unsigned char           Boot_Serial[4];
        unsigned char           Boot_Vol_Label[11];
        unsigned char           Boot_System_ID[8];
};

#define EXT_BOOT_SIG 41


/* Structure used to hold the values for INT13 calls to access the disk. */
typedef struct _INT13_Record 
{
   unsigned short      AX;  /* AH = 02, always.  AL = number of sectors to read. */
   unsigned short      CX;  /* CH = top 8 bits of cylinder number.  CL bits 0 - 5 = sector number, bits 6 and 7  are high order bits of cylinder number. */
   unsigned short      DX;  /* DH = head number.  DL = drive number.  Note: Bit 7 must always be set. */
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
   unsigned char           Boot_Sig;       /* 41 indicates extended boot */
   unsigned char           Boot_Serial[4];
   unsigned char           Boot_Vol_Label[11];
   unsigned char           Boot_System_ID[8];
   unsigned long           Reserved1;         /* _SectorBase */
   unsigned short          Reserved2;         /* CurrentTrack */
   unsigned char           Reserved3;         /* CurrentSector */
   unsigned short          Reserved4;         /* SectorCount */
   unsigned long           Reserved5;         /* lsnSaveChild */
   unsigned char           BootPathDrive;
   unsigned char           BootPathHead;
   unsigned char           BootPathSector;    /* Bits 0 - 5 are sector, bits 6 and 7 are high order bits of Cylinder. */
   unsigned char           BootPathCylinder;  /* Lower 8 bits of cylinder. */
   INT13_Record            INT13_Table[INT13_TABLE_SIZE];
} Boot_Manager_Boot_Record;

#define BOOT_MANAGER_PATH_SECTOR_OFFSET 1


/* Boot Manager Alias entry. */
// should be using bootpath.h instead of these defines
#define ALIAS_NAME_LENGTH 8
typedef struct _Alias_Entry {
                              unsigned char  Reserved[4];
                              unsigned char  Name[ALIAS_NAME_LENGTH];
                            } Alias_Entry;

/* New wide Boot Manager Alias entry. 206211*/
#define ALIAS_NAME_LENGTH_2 20
typedef struct _Alias_Entry_2 {
      unsigned char  Valid_Length;    // LVM should set to 20
      unsigned char  Reserved[3];
      unsigned char  Name[ALIAS_NAME_LENGTH_2];
 } Alias_Entry_2;

/* The next structure defines the Boot Manager BootPath record. */
#define MAX_ALIAS_ENTRIES  6
typedef struct _Boot_Path_Record {
                                   unsigned char    Drive;
                                   unsigned char    Head;
                                   unsigned char    Sector;
                                   unsigned char    Cylinder;
                                   unsigned short   Migration_Flag;  /* ??? */
                                   unsigned short   TimeOut;         /* Time out value in 1/18 of a second increments. */
                                   unsigned char    Boot_Index;
                                   unsigned char    Advanced_Mode;  /* If 0, then Boot Manager operates in Normal Mode.  If 1, then Boot Manager operates in advanced mode. */
                                   unsigned char    Immediate_Boot_Drive_Letter;
                                   unsigned char    Reboot_Flag;
                                   unsigned char    Reserved[4];
                                   Alias_Entry      Alias_Array[MAX_ALIAS_ENTRIES];
     Alias_Entry_2    Alias_Array_2[MAX_ALIAS_ENTRIES];
                                 } Boot_Path_Record;

#define DEFAULT_ALIAS_ENTRY   0
#define LAST_ALIAS_BOOTED     5
#define IMMEDIATE_BOOT_ALIAS  4


/* Boot Manager Alias Table Information.  The Alias Table is a two dimensional array of structures.  The array is
   24 by 4, and is composed of Alias_Table_Entry structures.  It is used to hold the Boot Manager name of any primary
   partitions residing on the first 24 drives in the system.                                                           */
#define ALIAS_TABLE_SECTOR_OFFSET    3
#define SECTORS_PER_ALIAS_TABLE      3
#define ALIAS_TABLE_DRIVE_LIMIT     24
#define ALIAS_TABLE_PARTITION_LIMIT  4

#define PART_78GB_LIM 8064UL                  //d_267590 Number of Mbytes for 7.8Gb limit

/* The following structure is used in the creation of the Boot Manager Alias Table. */
typedef struct _Alias_Table_Entry {
                                    unsigned char  Drive;                   /* BIOS Drive ID of the partition this entry represents. */

                                    /* Head, Sector, and Cylinder are the CHS address of the partition this entry represents. */

                                    unsigned char  Head;
                                    unsigned char  Sector;
                                    unsigned char  Cylinder;

                                    unsigned char  Bootable;                 /* ?? Assumed to be 0 if not bootable. */
                                    char           Name[ALIAS_NAME_LENGTH];  /* Boot Manager name for the partition represented by this entry. */

                                    /* Padding. */
                                    unsigned char  Reserved[3];
                                  } Alias_Table_Entry;


#pragma pack()
