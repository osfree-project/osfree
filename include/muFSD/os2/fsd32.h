//
// $Header: /cur/cvsroot/boot/include/muFSD/os2/fsd32.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
// (device drivers and installable file system drivers).
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

#ifndef __fsd32_h
#define __fsd32_h


#define CDDWORKAREASIZE 8
#define SFDWORKAREASIZE 30
#define VPDWORKAREASIZE 36

#define VPBTEXTLEN 12

#include <os2/types.h>

#pragma pack(1)
struct DriverCaps32 {
    USHORT Reserved ;                /* reserved, set to zero                */
    BYTE   VerMajor ;                /* major version of interface supported */
    BYTE   VerMinor ;                /* minor version of interface supported */
    ULONG  Capabilities ;            /* bitfield for driver capabilties      */
    PTR16  Strategy2 ;               /* entry point for strategy-2           */
    PTR16  EndofInt ;                /* entry point for DD_EndOfInt          */
    PTR16  ChgPriority ;             /* entry point for DD_ChgPriority       */
    PTR16  SetRestPos ;              /* entry point for DD_SetRestPos        */
    PTR16  GetBoundary ;             /* entry point for DD_GetBoundary       */
};
#pragma pack()

#pragma pack(1)
struct VolChars32 {
    USHORT VolDescriptor ;           /* see equates below                    */
    USHORT AvgSeekTime ;             /* milliseconds, if unknown, FFFFh      */
    USHORT AvgLatency ;              /* milliseconds, if unknown, FFFFh      */
    USHORT TrackMinBlocks ;          /* blocks on smallest track             */
    USHORT TrackMaxBlocks ;          /* blocks on largest track              */
    USHORT HeadsPerCylinder ;        /* if unknown or not applicable use 1   */
    ULONG  VolCylinderCount ;        /* number of cylinders on volume        */
    ULONG  VolMedianBlock ;          /* block in center of volume for seek   */
    USHORT MaxSGList ;               /* Adapter scatter/gather list limit    */
};
#pragma pack()

#pragma pack(1)
struct  vpfsi32 {
    unsigned long   vpi_vid;                /* 32 bit volume ID */
    unsigned long   vpi_hDEV;               /* handle to device driver */
    unsigned short  vpi_bsize;              /* sector size in bytes */
    unsigned long   vpi_totsec;             /* total number of sectors */
    unsigned short  vpi_trksec;             /* sectors / track */
    unsigned short  vpi_nhead;              /* number of heads */
    char            vpi_text[VPBTEXTLEN];   /* volume name */
 //   struct DriverCaps32 *vpi_pDCS;                /* device capability struc              */
 //   struct VolChars32   *vpi_pVCS;                /* volume characteristics struc         */
    PTR16           vpi_pDCS;               /* device capability struc      [16:16] */
    PTR16           vpi_pVCS;               /* volume characteristics struc [16:16] */
    unsigned char   vpi_drive;              /* drive (0=A) */
    unsigned char   vpi_unit;               /* unit code */
};
#pragma pack()


#pragma pack(1)
union vpfsd32 {
    char    vpd_work[VPDWORKAREASIZE];      /* work area */
    struct {
        struct super_block *sb;
    } u;
};
#pragma pack()

/* Current Directory structures */
#pragma pack(1)
struct cdfsi32 {
    unsigned short  cdi_hVPB;           /* VPB handle for associated device */
    unsigned short  cdi_end;            /* end of root portion of curdir */
    char            cdi_flags;          /* flags indicating state of cdfsd */
    char            cdi_curdir[CCHMAXPATH]; /* text of current directory */
};  /* cdfsi */
#pragma pack()
/* bit values for cdi_flags (state of cdfsd structure */

#define CDI_ISVALID     0x80            /* format is known */
#define CDI_ISROOT      0x40            /* cur dir == root */
#define CDI_ISCURRENT   0x20

#define CURDIREND_INVALID 0xFFFF        /* iCurDirEnd is irrelevant */

#pragma pack(1)
union cdfsd32 {
    char cdd_work[CDDWORKAREASIZE];
    struct {
        int          is_valid;
        struct file *p_file;
    } u;
};
#pragma pack()

/* Per open-instance (System File) structures */
#pragma pack(1)
struct  sffsi32 {
    unsigned long   sfi_mode;       /* access/sharing mode */
    unsigned short  sfi_hVPB;       /* volume info. */
    unsigned short  sfi_ctime;      /* file creation time */
    unsigned short  sfi_cdate;      /* file creation date */
    unsigned short  sfi_atime;      /* file access time */
    unsigned short  sfi_adate;      /* file access date */
    unsigned short  sfi_mtime;      /* file modification time */
    unsigned short  sfi_mdate;      /* file modification date */
    unsigned long   sfi_size;       /* size of file */
    unsigned long   sfi_position;   /* read/write pointer */
/* the following may be of use in sharing checks */
    unsigned short  sfi_UID;        /* user ID of initial opener */
    unsigned short  sfi_PID;        /* process ID of initial opener */
    unsigned short  sfi_PDB;        /* PDB (in 3.x box) of initial opener */
    unsigned short  sfi_selfsfn;    /* system file number of file instance */
    unsigned char   sfi_tstamp;     /* update/propagate time stamp flags */
                                    /* use with ST_Sxxx and ST_Pxxx */
    unsigned short  sfi_type;       /* use with STYPE_ */
    unsigned long   sfi_pPVDBFil;   /* performance counter data block pointer */
    unsigned char   sfi_DOSattr;    /* DOS file attributes  D/S/A/H/R  */
};  /* sffsi */
#pragma pack()
/* sfi_tstamps flags */
#define ST_SCREAT  1    /* stamp creation time */
#define ST_PCREAT  2    /* propagate creation time */
#define ST_SWRITE  4    /* stamp last write time */
#define ST_PWRITE  8    /* propagate last write time */
#define ST_SREAD  16    /* stamp last read time */
#define ST_PREAD  32    /* propagate last read time */

/* sfi_type flags */
#define STYPE_FILE      0       /* file */
#define STYPE_DEVICE    1       /* device */
#define STYPE_NMPIPE    2       /* named pipe */
#define STYPE_FCB       4       /* fcb sft */

/* file system dependent - file instance */
#pragma pack(1)
union sffsd32 {
    char         sfd_work[SFDWORKAREASIZE];         /* work area */
    struct file *f;
};  /* sffsd */
#pragma pack()

/* file system independent - file search parameters */
#pragma pack(1)
struct fsfsi32 {
        unsigned short  fsi_hVPB;       /* volume info. */
};  /* fsfsi */
#pragma pack()

/* file system dependent - file search parameters */
#define FSFSD_WORK_SIZE     24
#pragma pack(1)
struct fsfsd32 {
        char            fsd_work[FSFSD_WORK_SIZE];      /* work area */
};  /* fsfsd */
#pragma pack()

/*****
 *
 * Declarations for FS_DOPAGEIO entry point.
 *
 */
#pragma pack(1)
    struct PageCmd {
        unsigned char Cmd;          /* Cmd code for Read, Write, WriteVerify */
        unsigned char Priority;     /* Same values as for request packets */
        unsigned char Status;       /* Status byte */
        unsigned char Error;        /* I24 error code */
        unsigned long Addr;         /* Physical (32 bit) or Virtual (16:16) */
        unsigned long FileOffset;   /* Byte offset in page file */
                                    /* (on page boundary) */
    };
    struct PageCmdHeader {
        unsigned char InFlags;          /* Input flags */
        unsigned char OutFlags;         /* Output flags - must be 0 on entry */
        unsigned char OpCount;          /* Number of operations */
        unsigned char Pad;              /* Preserve DWORD alignment */
        unsigned long Reserved1;        /* unused */
        unsigned long Reserved2;        /* unused */
        unsigned long Reserved3;        /* unused */
        struct PageCmd PageCmdList [1]; /* The actual commands */
    };
#pragma pack()

    /* FSD_DoPageIO InFlags values */
    #define PGIO_FI_ORDER   0x01    /* Force Order of operations */

    /* FSD_DoPageIO OutFlags values */
    #define PGIO_FO_DONE    0x01    /* Operation done */
    #define PGIO_FO_ERROR   0x02    /* Operation failed */

    /* FSD_DoPageIO Status values */
    #define PGIO_ATTEMPTED  0x0f    /* Operation attempted */
    #define PGIO_FAILED     0xf0    /* Operation failed */




/*****
 *
 * union and structures for FS_FSCTL
 */
/* pArgType == 1, FileHandle directed case */
#pragma pack(1)
struct SF32 {
        PTR16 psffsi;
        PTR16 psffsd;
};
#pragma pack()

/* pArgType == 2, PathName directed case */
#pragma pack(1)
struct CD32 {
        PTR16          pcdfsi;
        PTR16          pcdfsd;
        PTR16          pPath;
        unsigned short iCurDirEnd;
};
#pragma pack()

#pragma pack(1)
union argdat32 {
    /* pArgType == 1, FileHandle directed case */
    struct SF32 sf;

    /* pArgType == 2, PathName directed case */
    struct CD32 cd;

    /* pArgType == 3, FSD Name directed case */
    /* nothing */
};  /* argdat */
#pragma pack()


#pragma pack(1)
struct fs32_allocatepagespace_parms {
    unsigned long ulWantContig;   /* contiguous chunk size       */
    unsigned long ulSize;         /* new size                    */
    PTR16 psffsd;                 /* ptr to fs dependent SFT     */
    PTR16 psffsi;                 /* ptr to fs independent SFT   */
};
#pragma pack()
int FS32ENTRY fs32_allocatepagespace(struct fs32_allocatepagespace_parms *parms);


#pragma pack(1)
struct fs32_chdir_parms {
    unsigned short      iCurDirEnd;
    PTR16               pDir;
    PTR16               pcdfsd;
    PTR16               pcdfsi;
    unsigned short      flag;
};
#pragma pack()
int FS32ENTRY fs32_chdir(struct fs32_chdir_parms *parms);

/* values for flag in FS_CHDIR */
#define CD_EXPLICIT             0x00
#define CD_VERIFY               0x01
#define CD_FREE                 0x02

#pragma pack(1)
struct fs32_chgfileptr_parms {
    unsigned short IOflag;
    unsigned short type;
    long           offset;
    PTR16          psffsd;
    PTR16          psffsi;
};
#pragma pack()
int FS32ENTRY fs32_chgfileptr(struct fs32_chgfileptr_parms *parms);

/* values for type in FS_CHGFILEPTR */
#define CFP_RELBEGIN            0x00
#define CFP_RELCUR              0x01
#define CFP_RELEND              0x02

#pragma pack(1)
struct fs32_close_parms {
    PTR16 psffsd;
    PTR16 psffsi;
    unsigned short IOflag;
    unsigned short type;
};
#pragma pack()
int FS32ENTRY fs32_close(struct fs32_close_parms *parms);

#define FS_CL_ORDINARY  0
                /* ordinary close of file */
#define FS_CL_FORPROC   1
                /* final close of file for the process */
#define FS_CL_FORSYS    2
                /* final close of file for the system (for all processes) */

#pragma pack(1)
struct fs32_commit_parms {
    PTR16 psffsd;
    PTR16 psffsi;
    unsigned short IOflag;
    unsigned short type;
};
#pragma pack()
int FS32ENTRY fs32_commit(struct fs32_commit_parms *parms);

/* values for commit type */
#define FS_COMMIT_ONE   1
        /* commit for a single file */
#define FS_COMMIT_ALL   2
        /* commit due to buf reset - for all files */


#pragma pack(1)
struct fs32_dopageio_parms {
    PTR16 pPageCmdList;     /* ptr to list of page commands */
    PTR16 psffsd;           /* ptr to fs dependent SFT      */
    PTR16 psffsi;           /* ptr to fs independent SFT    */
};
#pragma pack()
int FS32ENTRY fs32_dopageio(struct fs32_dopageio_parms *parms);

#pragma pack(1)
struct fs32_delete_parms {
    unsigned short      iCurDirEnd;
    PTR16               pFile;
    PTR16               pcdfsd;
    PTR16               pcdfsi;
};
#pragma pack()
int FS32ENTRY fs32_delete(struct fs32_delete_parms *parms);

#pragma pack(1)
struct fs32_exit_parms {
    unsigned short  pdb;
    unsigned short  pid;
    unsigned short  uid;
};
#pragma pack()
int FS32ENTRY fs32_exit(struct fs32_exit_parms *parms);

#pragma pack(1)
struct fs32_fileattribute_parms {
    PTR16           pAttr;
    unsigned short  iCurDirEnd;
    PTR16           pName;
    PTR16           pcdfsd;
    PTR16           pcdfsi;
    unsigned short  flag;
};
#pragma pack()
int FS32ENTRY fs32_fileattribute(struct fs32_fileattribute_parms *parms);

/* values for flag in FS_FILEATTRIBUTE */
#define FA_RETRIEVE             0x00
#define FA_SET                  0x01

#pragma pack(1)
struct fs32_fileinfo_parms {
    unsigned short  IOflag;
    unsigned short  cbData;
    PTR16           pData;
    unsigned short  level;
    PTR16           psffsd;
    PTR16           psffsi;
    unsigned short  flag;
};
#pragma pack()
int FS32ENTRY fs32_fileinfo(struct fs32_fileinfo_parms *parms);

/* values for flag in FS_FILEINFO */
#define FI_RETRIEVE             0x00
#define FI_SET                  0x01

#define FIL_QUERYALLEAS       4      // Level 4, query full EAS set
#define FIL_LEVEL7            7      // Level 7, return case preserved path name
#define FILE_NONFAT     0x0040       // File is non 8.3 compliant

#pragma pack(1)
struct fs32_findclose_parms {
    PTR16           pfsfsd;
    PTR16           pfsfsi;
};
#pragma pack()
int FS32ENTRY fs32_findclose(struct fs32_findclose_parms *parms);

/* values for flag in FS_FindFirst, FS_FindFromName, FS_FindNext */
#define     FF_NOPOS            0x00
#define     FF_GETPOS           0X01

#pragma pack(1)
struct fs32_findfirst_parms {
    unsigned short  flags;
    unsigned short  level;
    PTR16           pcMatch;
    unsigned short  cbData;
    PTR16           pData;
    PTR16           pfsfsd;
    PTR16           pfsfsi;
    unsigned short  attr;
    unsigned short  iCurDirEnd;
    PTR16           pName;
    PTR16           pcdfsd;
    PTR16           pcdfsi;
};
#pragma pack()
int FS32ENTRY fs32_findfirst(struct fs32_findfirst_parms *parms);

#pragma pack(1)
struct fs32_findfromname_parms {
    unsigned short  flags;
    PTR16           pName;
    unsigned long   position;
    unsigned short  level;
    PTR16           pcMatch;
    unsigned short  cbData;
    PTR16           pData;
    PTR16           pfsfsd;
    PTR16           pfsfsi;
};
#pragma pack()
int FS32ENTRY fs32_findfromname(struct fs32_findfromname_parms *parms);

#pragma pack(1)
struct fs32_findnext_parms {
    unsigned short  flags;
    unsigned short  level;
    PTR16           pcMatch;
    unsigned short  cbData;
    PTR16           pData;
    PTR16           pfsfsd;
    PTR16           pfsfsi;
};
#pragma pack()
int FS32ENTRY fs32_findnext(struct fs32_findnext_parms *parms);

#pragma pack(1)
struct fs32_flushbuf_parms {
    unsigned short flag;
    unsigned short hVPB;
};
#pragma pack()
int FS32ENTRY fs32_flushbuf(struct fs32_flushbuf_parms *parms);

/* values for flag in FS_FLUSH */
#define FLUSH_RETAIN            0x00
#define FLUSH_DISCARD           0x01

#pragma pack(1)
struct fs32_fsctl_parms {
    PTR16          plenDataOut;
    unsigned short lenData;
    PTR16          pData;
    PTR16          plenParmOut;
    unsigned short lenParm;
    PTR16          pParm;
    unsigned short func;
    unsigned short iArgType;
    PTR16          pArgdat;
};
#pragma pack()
int FS32ENTRY fs32_fsctl(struct fs32_fsctl_parms *parms);


/* values for iArgType in FS_FSCTL */
#define FSCTL_ARG_FILEINSTANCE  0x01
#define FSCTL_ARG_CURDIR        0x02
#define FSCTL_ARG_NULL          0x03

/* values for func in FS_FSCTL */
#define FSCTL_FUNC_NONE         0x00
#define FSCTL_FUNC_NEW_INFO     0x01
#define FSCTL_FUNC_EASIZE       0x02

#pragma pack(1)
struct fs32_fsinfo_parms {
    unsigned short level;
    unsigned short cbData;
    PTR16          pData;
    unsigned short hVPB;
    unsigned short flag;
};
#pragma pack()
int FS32ENTRY fs32_fsinfo(struct fs32_fsinfo_parms *parms);

/* values for flag in FS_FSINFO */
#define INFO_RETREIVE           0x00
#define INFO_SET                0x01

#pragma pack(1)
struct fs32_init_parms {
    PTR16 pMiniFSD;
    PTR16 DevHelp;
    PTR16 cmdline;
};
#pragma pack()
int FS32INIT fs32_init(struct fs32_init_parms *parms);

#pragma pack(1)
struct fs32_ioctl_parms {
    PTR16          pDataLenInOut;
    unsigned short lenData;
    PTR16          pData;
    PTR16          pParmLenInOut;
    unsigned short lenParm;
    PTR16          pParm;
    unsigned short func;
    unsigned short cat;
    PTR16           psffsd;
    PTR16           psffsi;
};
#pragma pack()
int FS32ENTRY fs32_ioctl(struct fs32_ioctl_parms *parms);

#pragma pack(1)
struct fs32_mkdir_parms {
    unsigned short flags;
    PTR16          pEABuf;
    unsigned short iCurDirEnd;
    PTR16          pName;
    PTR16          pcdfsd;
    PTR16          pcdfsi;
};
#pragma pack()
int FS32ENTRY fs32_mkdir(struct fs32_mkdir_parms *parms);


#pragma pack(1)
struct fs32_mount_parms {
    PTR16          pBoot;
    unsigned short hVPB;
    PTR16          pvpfsd;
    PTR16          pvpfsi;
    unsigned short flag;
};
#pragma pack()
int FS32ENTRY fs32_mount(struct fs32_mount_parms *parms);

/* values for flag in FS_MOUNT */
#define MOUNT_MOUNT             0x00
#define MOUNT_VOL_REMOVED       0x01
#define MOUNT_RELEASE           0x02
#define MOUNT_ACCEPT            0x03

#pragma pack(1)
struct fs32_move_parms {
    unsigned short flag;
    unsigned short iDstCurDirEnd;
    PTR16          pDst;
    unsigned short iSrcCurDirEnd;
    PTR16          pSrc;
    PTR16          pcdfsd;
    PTR16          pcdfsi;
};
#pragma pack()
int FS32ENTRY fs32_move(struct fs32_move_parms *parms);

#pragma pack(1)
struct fs32_newsize_parms {
    unsigned short IOflag;
    unsigned long len;
    PTR16          psffsd;
    PTR16          psffsi;
};
#pragma pack()
int FS32ENTRY fs32_newsize(struct fs32_newsize_parms *parms);

#pragma pack(1)
struct fs32_opencreate_parms {
    PTR16          pfgenflag;
    PTR16          pEABuf;
    unsigned short attr;
    PTR16          pAction;
    unsigned short openflag;
    unsigned long  ulOpenMode;
    PTR16          psffsd;
    PTR16          psffsi;
    unsigned short iCurDirEnd;
    PTR16          pName;
    PTR16          pcdfsd;
    PTR16          pcdfsi;
};
#pragma pack()
int FS32ENTRY fs32_opencreate(struct fs32_opencreate_parms *parms);

#pragma pack(1)
struct fs32_openpagefile_parms {
    unsigned long  Reserved; /* reserved, must be zero        */
    unsigned short Attr;     /* file attribute                */
    unsigned short OpenFlag; /* open flag for action          */
    unsigned short OpenMode; /* sharing, ...                  */
    PTR16 psffsd;            /* ptr to fs dependent SFT       */
    PTR16 psffsi;            /* ptr to fs independent SFT     */
    PTR16 pName;             /* name of paging file           */
    PTR16 pcMaxReq;          /* max # of reqs packed in list  */
    PTR16 pFlags;            /* pointer to Flags              */
};
#pragma pack()
int FS32ENTRY fs32_openpagefile(struct fs32_openpagefile_parms *parms);

/* values for *pFlags in FS_OPENPAGEFILE */
#define PGIO_FIRSTOPEN 0x00000001   /* first open of page file        */
#define PGIO_PADDR     0x00004000   /* physical addresses required    */
#define PGIO_VADDR     0x00008000   /* 16:16 virtual address required */

#pragma pack(1)
struct fs32_pathinfo_parms {
    unsigned short cbData;
    PTR16          pData;
    unsigned short level;
    unsigned short iCurDirEnd;
    PTR16          pName;
    PTR16          pcdfsd;
    PTR16          pcdfsi;
    unsigned short flag;
};
#pragma pack()
int FS32ENTRY fs32_pathinfo(struct fs32_pathinfo_parms *parms);

/* values for flag in FS_PATHINFO */
#define PI_RETRIEVE             0x00
#define PI_SET                  0x01

#pragma pack(1)
struct fs32_read_parms {
    unsigned short IOflag;
    PTR16          pLen;
    PTR16          pData;
    PTR16          psffsd;
    PTR16          psffsi;
};
#pragma pack()
int FS32ENTRY fs32_read(struct fs32_read_parms *parms);

#pragma pack(1)
struct fs32_rmdir_parms {
    unsigned short iCurDirEnd;
    PTR16          pName;
    PTR16          pcdfsd;
    PTR16          pcdfsi;
};
#pragma pack()
int FS32ENTRY fs32_rmdir(struct fs32_rmdir_parms *parms);

#pragma pack(1)
struct fs32_shutdown_parms {
    unsigned long  ulReserved;
    unsigned short usType;
};
#pragma pack()
int FS32ENTRY fs32_shutdown(struct fs32_shutdown_parms *parms);

/* values for usType in FS_SHUTDOWN */
#define SD_BEGIN                0x00
#define SD_COMPLETE             0x01

#pragma pack(1)
struct fs32_write_parms {
    unsigned short IOflag;
    PTR16          pLen;
    PTR16          pData;
    PTR16          psffsd;
    PTR16          psffsi;
};
#pragma pack()
int FS32ENTRY fs32_write(struct fs32_write_parms *parms);

typedef struct _EAOP16 {      /* eaop */
    PTR16 fpGEAList;          /* general EA list */
    PTR16 fpFEAList;          /* full EA list */
    ULONG oError;
} EAOP16;
typedef EAOP16* PEAOP16;




#endif /* __fsd32_h */
