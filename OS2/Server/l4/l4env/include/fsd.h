/*   IFS32 API for osFree
 *   (c) osFree project
 *   author valerius
 */

#ifndef  __FSD_H__
#define  __FSD_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <gcc_os2def.h>

/* time stamping */
#define ST_SCREAT       1         /* stamp creation time */
#define ST_PCREAT       2         /* propagate creation time */
#define ST_SWRITE       4         /* stamp last write time */
#define ST_PWRITE       8         /* propagate last write time */
#define ST_SREAD        16        /* stamp last read time */
#define ST_PREAD        32        /* propagate last */

/* used return types and calling conventions   */
#define FSRET    int
#define FSENTRY  _System

/* file system independent - volume parameters */
struct vpfsi {
    ULONG         vpi_vid;        /* 32 bit volume ID */
    ULONG         vpi_hDEV;       /* handle to device driver */
    USHORT        vpi_bsize;      /* sector size in bytes */
    ULONG         vpi_totsec;     /* total number of sectors */
    USHORT        vpi_trksec;     /* sectors / track */
    USHORT        vpi_nhead;      /* number of heads */
    char          vpi_text[12];   /* ASCIIZ volume name */
    void          *vpi_pDCS;      /* device capability structure */
    void          *vpi_pVCS;      /* volume characteristics */
    UCHAR         vpi_drive;      /* drive (0=A) */
    UCHAR         vpi_unit;       /* unit code */
    USHORT        vpi_flags;      /* flags for memory restrictions */
};

/* file system dependent - volume parameters */
struct vpfsd {
    char          vpd_work[36];   /* work area */
};

/* file system independent - current directories */
struct cdfsi {
    USHORT        cdi_hVPB;        /* VPB handle for associated device */
    USHORT        cdi_end;         /* offset to root of path */
    char          cdi_flags;       /* FS independent flags */
    char          cdi_curdir[CCHMAXPATH]; /* text of current directory */
};

/* bit values for cdi_flags (state of cdfsd structure */
#define CDI_ISVALID 0x80		/* format is known */
#define CDI_ISROOT  0x40		/* cur dir == root */
#define CDI_ISCURRENT	0x20

/* file system dependent - current directories */
struct cdfsd {
    char          cdd_work[8];     /* work area */
};

/* file system independent - file instance */
struct sffsi {
    ULONG         sfi_mode;        /* access/sharing mode */
    USHORT        sfi_hVPB;        /* volume info. */
    USHORT        sfi_ctime;       /* file creation time */
    USHORT        sfi_cdate;       /* file creation date */
    USHORT        sfi_atime;       /* file access time */
    USHORT        sfi_adate;       /* file access date */
    USHORT        sfi_mtime;       /* file modification time */
    USHORT        sfi_mdate;       /* file modification date */
    ULONG         sfi_size;        /* size of file */
    ULONG         sfi_position;    /* read/write pointer */

/* the following may be of use in sharing checks */

    USHORT        sfi_UID;         /* user ID of initial opener */
    USHORT        sfi_PID;         /* process ID of initial opener */
    USHORT        sfi_PDB;         /* PDB (in 3.x box) of initial opener */
    USHORT        sfi_selfsfn;     /* system file number of file instance */
    UCHAR         sfi_tstamp;      /* time stamp flags */
    USHORT        sfi_type;        /* type of object opened */
    ULONG         sfi_pPVDBFil;    /* performance counter data block pointer */
    UCHAR         sfi_DOSattr;     /* DOS file attributes D/S/A/H/R */
    LONGLONG        sfi_sizel;     /* size of file */
    LONGLONG        sfi_positionl; /* read/write pointer */
};

/* file system dependent - file instance */
struct sffsd {
    char            sfd_work[30];  /* work area */
};

/* file system independent - file search parameters */
struct fsfsi {
        USHORT      fsi_hVPB;      /* volume info. */
};

/* file system dependent - file search parameters */
struct fsfsd {
        char        fsd_work[24];  /* work area */
};


/* file system service routines */
FSRET FSENTRY FS_ALLOCATEPAGESPACE(struct sffsi *psffsi,
                                   struct sffsd *psffsd,
                                   ULONG ulsize,
                                   USHORT ulWantContig);

FSRET FSENTRY FS_ATTACH(USHORT flag,
                        char *pDev,
                        struct vpfsd *pvpfsd,
                        struct cdfsd *pcdfsd,
                        char *pParm,
                        USHORT *pLen);

struct filelock {
    ULONG FileOffset;   /* offset where the lock/unlock begins  */
    ULONG RangeLength;  /* length of region locked/unlocked     */
}

FSRET FSENTRY FS_CANCELLOCKREQUEST(struct sffsi *psffsi,
                                   struct sffsd *psffsd,
                                   struct filelock *pLockRange);

FSRET FSENTRY FS_CHDIR(USHORT flag,
                       struct cdsfi *pcdfsi,
                       struct cdfsd *pcdfsd,
                       char *pDir,
                       USHORT iCurDirEnd);

FSRET FSENTRY FS_CHGFILEPTR(struct sffsi *psffsi,
                            struct sffsd *psffsd,
                            long offset,
                            USHORT type,
                            USHORT IOflag);

FSRET FSENTRY FS_CLOSE(USHORT type,
                       USHORT IOflag,
                       struct sffsi *psffsi,
                       struct sffsd *psffsd);

FSRET FSENTRY FS_COMMIT(USHORT type,
                        USHORT IOflag,
                        struct sffsi *psffsi,
                        struct sffsd *psffsd);

FSRET FSENTRY FS_COPY(USHORT flag,
                      struct cdfsi *pcdfsi,
                      struct cdfsd *pcdfsd,
                      char *pSrc,
                      USHORT iSrcCurDirEnd,
                      char *pDst,
                      USHORT iDstCurDirEnd,
                      USHORT nameType);

FSRET FSENTRY FS_DELETE(struct cdfsi *pcdfsi,
                        struct cdfsd *pcdfsd,
                        char *pFile,
                        USHORT iCurDirEnd);

struct PageCmd {
    UCHAR Cmd;           /* Cmd Code (Read,Write,Verify)       */
    UCHAR Priority;      /* Same values as for req packets     */
    UCHAR Status;        /* Status byte                        */
    UCHAR Error;         /* I24 error code                     */
    ULONG Addr;          /* Physical(0:32) or Virtual(16:16)   */
    ULONG FileOffset;    /* Byte Offset in page file  */
}

struct PageCmdHeader {
    UCHAR  InFlags;      /* Input Flags                        */
    UCHAR  OutFlags;     /* Output Flags - must be 0 on entry  */
    UCHAR  OpCount;      /* Number of operations               */
    UCHAR  Pad;          /* Pad for DWORD alignment            */
    ULONG  Reserved1;    /* Currently Unused                   */
    ULONG  Reserved2;    /* Currently Unused                   */
    ULONG  Reserved3;    /* Currently Unused                   */
    struct PageCmd PageCmdList;  /* Currently Unused           */
}

FSRET FSENTRY FS_DOPAGEIO(struct sffsi *psffsi,
                          struct sffsd *psffsd,
                          struct PageCmdHeader *pList);

void  FSENTRY FS_EXIT(USHORT uid,
                      USHORT pid,
                      USHORT pdb);

FSRET FSENTRY FS_FILEATTRIBUTE(USHORT flag,
                               struct cdfsi *pcdfsi,
                               struct cdfsd *pcdfsd,
                               char *pName,
                               USHORT iCurDirEnd,
                               USHORT *pAttr);

FSRET FSENTRY FS_FILEINFO(USHORT flag,
                          struct sffsi *psffsi,
                          struct sffsd *psffsd,
                          USHORT level,
                          char *pData,
                          USHORT cbData,
                          USHORT IOflag);

FSRET FSENTRY FS_FILEIO (struct sffsi *psffsi,
                         struct sffsd *psffsd,
                         char *pCmdList,
                         USHORT cbCmdList,
                         USHORT *poError,
                         USHORT IOflag);

FSRET FSENTRY FS_FILELOCKS(struct sffsi *psffsi,
                           struct sffsd *psffsd,
                           struct filelock *pUnLockRange,
                           struct filelock *pLockRange,
                           ULONG timeout,
                           ULONG flags);

FSRET FSENTRY FS_FINDCLOSE(struct fsfsi *pfsfsi,
                           struct fsfsd *pfsfsd);

FSRET FSENTRY FS_FINDFIRST(struct cdfsi *pcdfsi,
                           struct cdfsd *pcdfsd,
                           char *pName,
                           USHORT iCurDirEnd,
                           USHORT attr,
                           struct fsfsi *pfsfsi,
                           struct fsfsd *pfsfsd,
                           char *pData,
                           USHORT cbData,
                           USHORT *pcMatch,
                           USHORT level,
                           USHORT flags);

FSRET FSENTRY FS_FINDFROMNAME(struct fsfsi *pfsfsi,
                              struct fsfsd *pfsfsd,
                              char *pData,
                              USHORT cbData,
                              USHORT *pcMatch,
                              USHORT level,
                              ULONG position,
                              char *pName,
                              USHORT flags);

FSRET FSENTRY FS_FINDNEXT(struct fsfsi *pfsfsi,
                          struct fsfsd *pfsfsd,
                          char *pData,
                          USHORT cbData,
                          USHORT *pcMatch,
                          USHORT level,
                          USHORT flags);

FSRET FSENTRY FS_FINDNOTIFYCLOSE(USHORT handle);

FSRET FSENTRY FS_FINDNOTIFYFIRST(struct cdfsi *pcdfsi,
                                 struct cdfsd *pcdfsd,
                                 char *pName,
                                 USHORT iCurDirEnd,
                                 USHORT attr,
                                 USHORT *pHandle,
                                 char *pData,
                                 USHORT cbData,
                                 USHORT *pMatch,
                                 USHORT level,
                                 ULONG timeout);

FSRET FSENTRY FS_FINDNOTIFYNEXT(USHORT handle,
                                char *pData,
                                USHORT cbData,
                                USHORT *pcMatch,
                                USHORT level,
                                ULONG timeout);

FSRET FSENTRY FS_FLUSHBUF(USHORT hVPB,
                          USHORT flag);

union argdat {

    /* pArgType = 1, FileHandle directed case */

    struct sf {
        struct sffsi far * psffsi;
        struct sffsd far * psffsd;
    };

    /* pArgType = 2, Pathname directed case */

    struct cd {
        struct cdfsi far * pcdfsi;
        struct cdfsd far * pcdfsd;
        char far *         pPath;
        unsigned short     iCurDirEnd;
    };

    /* pArgType = 3, FSD Name directed case */
    /* pArgDat is Null                      */
};

FSRET FSENTRY FS_FSCTL(union argdat *pArgDat,
                       USHORT iArgType,
                       USHORT func,
                       char *pParm,
                       USHORT lenParm,
                       USHORT *plenParmIO,
                       char *pData,
                       USHORT lenData,
                       USHORT *plenDataIO);

FSRET FSENTRY FS_FSINFO(USHORT flag,
                        USHORT hVPB,
                        char *pData,
                        USHORT cbData,
                        USHORT level);

FSRET FSENTRY FS_INIT(char *szParm
                      ULONG DevHelp,
                      ULONG *pMiniFSD);

FSRET FSENTRY FS_IOCTL(struct sffsi *psffsi,
                       struct sffsd *psffsd,
                       USHORT cat,
                       USHORT func,
                       char *pParm,
                       USHORT lenMaxParm,
                       USHORT *plenInOutParm,
                       char *pData,
                       USHORT lenMaxData,
                       USHORT *plenInOutData);

FSRET FSENTRY FS_MKDIR(struct cdfsi *pcdfsi,
                       struct cdfsd *pcdfsd,
                       char *pName,
                       USHORT iCurDirEnd,
                       char *pEABuf,
                       USHORT flags);

FSRET FSENTRY FS_MOUNT(USHORT flag,
                       struct vpfsi *pvpfsi,
                       struct vpfsd *pvpfsd,
                       USHORT hVPB,
                       char *pBoot);

FSRET FSENTRY FS_MOVE(struct cdfsi *pcdfsi,
                      struct cdfsd *pcdfsd,
                      char *pSrc,
                      USHORT iSrcCurDirEnd,
                      char *pDst,
                      USHORT iDstCurDirEnd,
                      USHORT flags);

FSRET FSENTRY FS_NEWSIZE(struct sffsi *psffsi,
                         struct sffsd *psffsd,
                         ULONG len,
                         USHORT IOflag);

#define NMP_GetPHandState    0x21
#define NMP_SetPHandState    0x01
#define NMP_PipeQInfo        0x22
#define NMP_PeekPipe         0x23
#define NMP_ConnectPipe      0x24
#define NMP_DisconnectPipe   0x25
#define NMP_TransactPipe     0x26
#define NMP_ReadRaw          0x11
#define NMP_WriteRaw         0x31
#define NMP_WaitPipe         0x53
#define NMP_CallPipe         0x54
#define NMP_QNmPipeSemState  0x58

/* Get/SetPhandState parameter block */
struct phs_param {
    short phs_len;
    short phs_dlen;
    short phs_pmode; /* pipe mode set or returned */
};

/* DosQNmPipeInfo parameter block */
struct npi_param {
    short npi_len;
    short npi_dlen;
    short npi_level; /* information level desired */
};

/* DosRawReadNmPipe parameters */
/* data is buffer addr         */
struct npr_param {
    short npr_len;
    short npr_dlen;
    short npr_nbyt; /* number of bytes read */
};


/* DosRawWriteNmPipe parameters */
/* data is buffer addr          */
struct npw_param {
    short npw_len;
    short npw_dlen;
    short npw_nbyt; /* number of bytes written */
};

/* NPipeWait parameters */
struct npq_param {
    short npq_len;
    short npq_dlen;
    long npq_timeo;  /* time-out in milliseconds */
    short npq_prio;  /* priority of caller       */
};

/* DosCallNmPipe parameters */
/* data is in-buffer addr   */
struct npx_param {
    short npx_len;
    unsigned short npx_ilen;  /* length of in-buffer      */
    char far * npx_obuf;      /* pointer to out-buffer    */
    unsigned short npx_ilen;  /* length of out-buffer     */
    unsigned short npx_nbyt;  /* number of bytes read     */
    long npx_timeo;           /* time-out in milliseconds */
};

/* PeekPipe parameters, data is buffer addr */
struct npp_param {
    short npp_len;
    unsigned short npp_dlen;
    unsigned short npp_nbyt;   /* number of bytes read      */
    unsigned short npp_av10;   /* bytes left in pipe        */
    unsigned short npp_av11;   /* bytes left in current msg */
    unsigned short npp_state;  /* pipe state                */
};

/* DosTransactNmPipe parameters */
/* data is in-buffer addr       */
struct npt_param {
    short npt_len;
    unsigned short npt_ilen;  /* length of in-buffer   */
    char far * npt_obuf;      /* pointer to out-buffer */
    unsigned short npt_olen;  /* length of out-buffer  */
    unsigned short npt_nbyt;  /* number of bytes read  */
};

/* QNmPipeSemState parameter block */ /* data is user data buffer */
struct qnps_param {
    unsigned short qnps_len;  /* length of parameter block      */
    unsigned short qnps_dlen;  /* length of supplied data block */
    long qnps_semh;            /* system semaphore handle       */
    unsigned short qnps_nbyt;  /* number of bytes returned      */
};

/* ConnectPipe parameter block, no data block */
struct npc_param {
    unsigned short npc_len;   /* length of parameter block */
    unsigned short npc_dlen;  /* length of data block      */
};

/* DisconnectPipe parameter block, no data block */
struct npd_param {
    unsigned short npd_len;   /* length of parameter block */
    unsigned short npd_dlen;  /* length of data block      */
}

union npoper {
    struct phs_param phs;
    struct npi_param npi;
    struct npr_param npr;
    struct npw_param npw;
    struct npq_param npq;
    struct npx_param npx;
    struct npp_param npp;
    struct npt_param npt;
    struct qnps_param qnps;
    struct npc_param npc;
    struct npd_param npd;
};

FSRET FSENTRY FS_NMPIPE(struct sffsi *psffsi,
                        struct sffsd *psffsd,
                        USHORT OpType,
                        union npoper *pOpRec,
                        char pData,
                        char *pName);

FSRET FSENTRY FS_OPENCREATE(struct cdfsi *pcdfsi,
                            struct cdfsd *pcdfsd,
                            char *pName,
                            USHORT iCurDirEnd,
                            struct sffsi *psffsi,
                            struct sffsd *psffsd,
                            ULONG ulOpenMode,
                            USHORT usOpenFlag,
                            USHORT *pusAction,
                            USHORT usAttr,
                            char *pcEABuf,
                            USHORT *pfgenflag);

FSRET FSENTRY FS_OPENPAGEFILE(unsigned long *pFlag,
                              unsigned long *pcMaxReq,
                              char *pName,
                              struct sffsi *psffsi,
                              struct sffsd *psffsi,
                              USHORT usOpenMode,
                              USHORT usOpenFlag,
                              USHORT usAttr,
                              ULONG Reserved);

FSRET FSENTRY FS_PATHINFO(unsigned short flag,
                          struct cdfsi *pcdfsi,
                          struct cdfsd *pcdfsd,
                          char *pName,
                          USHORT iCurDirEnd,
                          USHORT level,
                          char *pData,
                          USHORT cbData);

FSRET FSENTRY FS_PROCESSNAME(char *pNameBuf);

FSRET FSENTRY FS_READ(struct sffsi *psffsi,
                      struct sffsd *psffsd,
                      char *pData,
                      unsigned short *pLen,
                      USHORT IOflag);

FSRET FSENTRY FS_RMDIR(struct cdfsi *pcdfsi,
                       struct cdfsd *pcdfsd,
                       char *pName,
                       USHORT iCurDirEnd);

FSRET FSENTRY FS_SETSWAP(struct sffsi *psffsi,
                         struct sffsd *psffsd);

FSRET FSENTRY FS_SHUTDOWN(USHORT type,
                          ULONG reserved);

FSRET FSENTRY FS_VERIFYUNCNAME(USHORT flag,
                               char *pName);

FSRET FSENTRY FS_WRITE(struct sffsi *psffsi,
                       struct sffsd *psffsd,
                       char *pData,
                       unsigned short *pLen,
                       USHORT IOflag);

#ifdef __cplusplus
  }
#endif

#endif
