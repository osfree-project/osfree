/*
 *
 *
 */

#define INLINE _inline
#define _FS_ENTRY far  pascal _loadds
#define _FS_PTR   far *             // This is no longer necessary with MSVC : -Gx- assumes data is FAR, I'll cleanup this later !
#define _FS_RET   int

#pragma pack(1)

/* file system independent - volume parameters */

struct vpfsi {
    unsigned long  vpi_vid;        /* 32 bit volume ID */
    unsigned long  vpi_hDEV;       /* handle to device driver */
    unsigned short vpi_bsize;      /* sector size in bytes */
    unsigned long  vpi_totsec;     /* total number of sectors */
    unsigned short vpi_trksec;     /* sectors / track */
    unsigned short vpi_nhead;      /* number of heads */
    char           vpi_text[12];   /* ASCIIZ volume name */
    void far *     vpi_pDCS;       /* device capability structure */
    void far *     vpi_pVCS;       /* volume characteristics */
    unsigned char  vpi_drive;      /* drive (0=A) */
    unsigned char  vpi_unit;       /* unit code */
};

/* file system dependent - volume parameters */

struct vpfsd {
    char           vpd_work[36];   /* work area */
};

/* file system independent - current directories */

struct cdfsi {
    unsigned short cdi_hVPB;           /* VPB handle for associated device */
    unsigned short cdi_end;            /* offset to root of path */
    char           cdi_flags;          /* FS independent flags */
    char           cdi_curdir[260];    /* text of current directory */
};

/* file system dependent - current directories */

struct cdfsd {
    char           cdd_work[8];        /* work area */
};

/* file system independent - file instance */

struct sffsi {
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
    unsigned char   sfi_tstamp;     /* time stamp flags */
    unsigned short  sfi_type;       /* type of object opened */
    unsigned long   sfi_pPVDBFil;   /* performance counter data block pointer */
    unsigned char   sfi_DOSattr;    /* DOS file attributes D/S/A/H/R */
};

/* file system dependent - file instance */

struct sffsd {
    char            sfd_work[30];   /* work area */
};

/* file system independent - file search parameters */

struct fsfsi {
        unsigned short  fsi_hVPB;       /* volume info. */
};

/* file system dependent - file search parameters */

struct fsfsd {
        char            fsd_work[24];   /* work area */
};

#pragma pack()

/* IFS entry points */
int far pascal _loadds MFS_OPEN(char far *pszName, unsigned long far *pulSize);
int far pascal _loadds MFS_READ(char far *pcData,  unsigned short far *pusLength);

/* IFS helpers */
int far pascal MFSH_INTERR(char far *pcMsg, unsigned short cbMsg);

int far pascal MFSH_SETBOOTDRIVE(unsigned short usDrive);

int far pascal MFSH_SEGFREE(unsigned short usSel);

int far pascal MFSH_PHYSTOVIRT(unsigned long ulAddr,
                               unsigned short usLen,
                               unsigned short far *pusSel);

int far pascal MFSH_UNPHYSTOVIRT(unsigned short usSel);

int far pascal FSH_INTERR(char far *pMsg,
                          unsigned short cbMsg);

void far pascal FSH_GETVOLPARM(unsigned short hVPB,
                               struct vpfsi far * far *ppVPBfsi,
                               struct vpfsd far * far *ppVPBfsd);

int far pascal FSH_DOVOLIO(unsigned short operation,
                           unsigned short fAllowed,
                           unsigned short hVPB,
                           char far *pData,
                           unsigned short far *pcSec,
                           unsigned long iSec);

int far pascal FSH_FINDDUPHVPB(unsigned short hVPB,
                               unsigned short far *phVPB);

int  far pascal FSH_DEVIOCTL  (unsigned short flag,
                               unsigned long hDev,
                               unsigned short sfn,
                               unsigned short cat,
                               unsigned short func,
                               char far *pParm,
                               unsigned short cbParm,
                               char far *pData,
                               unsigned short cbData);

int  far pascal FSH_PROBEBUF  (unsigned short operation,
                               char far *pdata,
                               unsigned short cbData);

int  far pascal FSH_UPPERCASE(char far *szPathName,
                              unsigned short cbPathBuf,
                              char far *pPathBuf);
