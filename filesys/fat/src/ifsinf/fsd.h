/*static char *SCCSID = "@(#)fsd.h	6.5 91/12/16";*/
/*  fsd.h   - File system driver entry point declarations */

/*  FS_ATTRIBUTE bit field values */

#define FSA_REMOTE  0x00000001	/* Set if REMOTE FSD		      */
#define FSA_UNC	    0x00000002	/* Set if FSD implements UNC support  */
#define FSA_LOCK    0x00000004	/* Set if FSD needs lock notification */
#define FSA_LVL7    0x00000008	/* Set if FSD will accept Level 7     */
     /* DosQueryPathInfo requests - These are requests for the case-  */
     /* perserved "path" of the input file/path name.		      */
#define FSA_PSVR    0x00000010	/* Set if FSD manages Remote NmPipes  */

#define CDDWORKAREASIZE 8
#define SFDWORKAREASIZE 30
#define VPDWORKAREASIZE 36


/* Volume Parameter structures */

#define VPBTEXTLEN 12

struct	vpfsi {
    unsigned long   vpi_vid;		    /* 32 bit volume ID */
    unsigned long   vpi_hDEV;		    /* handle to device driver */
    unsigned short  vpi_bsize;		    /* sector size in bytes */
    unsigned long   vpi_totsec;		    /* total number of sectors */
    unsigned short  vpi_trksec;		    /* sectors / track */
    unsigned short  vpi_nhead;		    /* number of heads */
    char	    vpi_text[VPBTEXTLEN];   /* volume name */
    void far *	    vpi_pDCS;		    /* device capability struc */
    void far *	    vpi_pVCS;		    /* volume characteristics struc */
    unsigned char   vpi_drive;		    /* drive (0=A) */
    unsigned char   vpi_unit;		    /* unit code */
};  /* vpfsi */

/*
 * Predefined volume IDs: [note - keep this list in sync with list in
 * dos/dosinc/vpb.inc!]
 */
/* Unique ID for vpb_ID field for unreadable media. */
#define UNREAD_ID  0x534E4A52L		/* Stored as (bytes) 0x52,4A,4E,53. */

/*
 * Unique ID for vpb_ID field for damaged volume (recognized by IFS but cannot
 * be normally mounted).
 */
#define DAMAGED_ID 0x0L			/* Stored as (bytes) 0,0,0,0. */

/* file system dependent - volume params */
struct	vpfsd {
    char    vpd_work[VPDWORKAREASIZE];	    /* work area */
};  /* vpfsd */


/* Current Directory structures */

struct cdfsi {
    unsigned short  cdi_hVPB;		/* VPB handle for associated device */
    unsigned short  cdi_end;		/* end of root portion of curdir */
    char	    cdi_flags;		/* flags indicating state of cdfsd */
    char	    cdi_curdir[CCHMAXPATH]; /* text of current directory */
};  /* cdfsi */

/* bit values for cdi_flags (state of cdfsd structure */

#define CDI_ISVALID 0x80		/* format is known */
#define CDI_ISROOT  0x40		/* cur dir == root */
#define CDI_ISCURRENT	0x20

struct	cdfsd {
    char    cdd_work[CDDWORKAREASIZE];	    /* work area */
};  /* cdfsd */


/* Per open-instance (System File) structures */

struct	sffsi {
    unsigned long   sfi_mode;	    /* access/sharing mode */
    unsigned short  sfi_hVPB;	    /* volume info. */
    unsigned short  sfi_ctime;	    /* file creation time */
    unsigned short  sfi_cdate;	    /* file creation date */
    unsigned short  sfi_atime;	    /* file access time */
    unsigned short  sfi_adate;	    /* file access date */
    unsigned short  sfi_mtime;	    /* file modification time */
    unsigned short  sfi_mdate;	    /* file modification date */
    unsigned long   sfi_size;	    /* size of file */
    unsigned long   sfi_position;   /* read/write pointer */
/* the following may be of use in sharing checks */
    unsigned short  sfi_UID;	    /* user ID of initial opener */
    unsigned short  sfi_PID;	    /* process ID of initial opener */
    unsigned short  sfi_PDB;	    /* PDB (in 3.x box) of initial opener */
    unsigned short  sfi_selfsfn;    /* system file number of file instance */
    unsigned char   sfi_tstamp;	    /* update/propagate time stamp flags */
				    /* use with ST_Sxxx and ST_Pxxx */
    unsigned short  sfi_type;	    /* use with STYPE_ */
    unsigned long   sfi_pPVDBFil;   /* performance counter data block pointer */
    unsigned char   sfi_DOSattr;    /* DOS file attributes  D/S/A/H/R  */
};  /* sffsi */

/* sfi_tstamps flags */
#define ST_SCREAT  1	/* stamp creation time */
#define ST_PCREAT  2	/* propagate creation time */
#define ST_SWRITE  4	/* stamp last write time */
#define ST_PWRITE  8	/* propagate last write time */
#define ST_SREAD  16	/* stamp last read time */
#define ST_PREAD  32	/* propagate last read time */

/* sfi_type flags */
#define STYPE_FILE	0	/* file */
#define STYPE_DEVICE	1	/* device */
#define STYPE_NMPIPE	2	/* named pipe */
#define STYPE_FCB	4	/* fcb sft */

/* file system dependent - file instance */
struct	sffsd {
    char    sfd_work[SFDWORKAREASIZE];	    /* work area */
};  /* sffsd */


/* file system independent - file search parameters */
struct fsfsi {
	unsigned short	fsi_hVPB;	/* volume info. */
};  /* fsfsi */

/* file system dependent - file search parameters */
#define FSFSD_WORK_SIZE	    24
struct fsfsd {
	char		fsd_work[FSFSD_WORK_SIZE];	/* work area */
};  /* fsfsd */

/* file system dependent - device information */
struct devfsd {
	unsigned long	FSDRef; /* Reference obtained from FSD during ATTACH */
};  /* devfsd */

/*****
 *
 * union and structures for FS_FSCTL
 */
/* pArgType == 1, FileHandle directed case */
/* XLATOFF */
struct SF {
	struct sffsi far * psffsi;
	struct sffsd far * psffsd;
};  /* SF */
/* XLATON */

/* pArgType == 2, PathName directed case */
struct CD {
	struct cdfsi far * pcdfsi;
	struct cdfsd far * pcdfsd;
	char far *	   pPath;
	unsigned short	   iCurDirEnd;
};  /* CD */

/* XLATOFF */
union argdat {
    /* pArgType == 1, FileHandle directed case */
    struct SF sf;

    /* pArgType == 2, PathName directed case */
    struct CD cd;

    /* pArgType == 3, FSD Name directed case */
    /* nothing */
};  /* argdat */
/* XLATON */


/*****
 *
 * Union and structures for FS_NMPIPE
 *
 */

    /* Get/SetPHandState parameter block */
    struct	phs_param {
	short	phs_len;
	short	phs_dlen;
	short	phs_pmode;	/* pipe mode set or returned */
    };


    /* DosQNmPipeInfo parameter block,
     * data is info. buffer addr */
    struct	npi_param {
	short	npi_len;
	short	npi_dlen;
	short	npi_level;	/* information level desired */
    };


    /* DosRawReadNmPipe parameters,
     * data is buffer addr */
    struct	npr_param {
	short	npr_len;
	short	npr_dlen;
	short	npr_nbyt;	/* number of bytes read */
    };

    /* DosRawWriteNmPipe parameters,
     * data is buffer addr */
    struct	npw_param {
	short	npw_len;
	short	npw_dlen;
	short	npw_nbyt;	/* number of bytes written */
    };

    /* NPipeWait parameters */
    struct	npq_param {
	short	npq_len;
	short	npq_dlen;
	long	npq_timeo;	/* timeout in milliseconds */
	short	npq_prio;	/* priority of caller */
    };

    /* DosCallNmPipe parameters,
     * data is in-buffer addr */
    struct	npx_param {
	short		npx_len;
	unsigned short	npx_ilen;   /* length of in-buffer */
	char far	*npx_obuf;  /* pointer to out-buffer */
	unsigned short	npx_olen;   /* length of out-buffer */
	unsigned short	npx_nbyt;   /* number of bytes read */
	long		npx_timeo;  /* timeout in milliseconds */
    };

    /* PeekPipe parameters, data is buffer addr */
    struct	npp_param {
	short		npp_len;
	unsigned short	npp_dlen;
	unsigned short	npp_nbyt;   /* number of bytes read */
	unsigned short	npp_avl0;   /* bytes left in pipe */
	unsigned short	npp_avl1;   /* bytes left in current msg */
	unsigned short	npp_state;  /* pipe state */
    };

    /* DosTransactNmPipe parameters,
     * data is in-buffer addr */
    struct	npt_param {
	short		npt_len;
	unsigned short	npt_ilen;   /* length of in-buffer */
	char far	*npt_obuf;  /* pointer to out-buffer */
	unsigned short	npt_olen;   /* length of out-buffer */
	unsigned short	npt_nbyt;   /* number of bytes read */
    };

    /* QNmpipeSemState parameter block,
     * data is user data buffer */
    struct	qnps_param {
	unsigned short	qnps_len;   /* length of parameter block */
	unsigned short	qnps_dlen;  /* length of supplied data block */
	long		qnps_semh;  /* system semaphore handle */
	unsigned short	qnps_nbyt;  /* number of bytes returned */
    };

    /* ConnectPipe parameter block, no data block */
    struct	npc_param {
	unsigned short	npc_len;    /* length of parameter block */
	unsigned short	npc_dlen;   /* length of data block */
    };

    /* DisconnectPipe parameter block, no data block */
    struct	npd_param {
	unsigned short	npd_len;    /* length of parameter block */
	unsigned short	npd_dlen;   /* length of data block */
    };

    /* XLATOFF */
    union npoper {
	struct phs_param    phs;
	struct npi_param    npi;
	struct npr_param    npr;
	struct npw_param    npw;
	struct npq_param    npq;
	struct npx_param    npx;
	struct npp_param    npp;
	struct npt_param    npt;
	struct qnps_param   qnps;
	struct npc_param    npc;
	struct npd_param    npd;
    };	/* npoper */
/* XLATON */


/*****
 *
 * Declarations for FS_DOPAGEIO entry point.
 *
 */

    struct PageCmd {
	unsigned char Cmd;	    /* Cmd code for Read, Write, WriteVerify */
	unsigned char Priority;	    /* Same values as for request packets */
	unsigned char Status;	    /* Status byte */
	unsigned char Error;	    /* I24 error code */
	unsigned long Addr;	    /* Physical (32 bit) or Virtual (16:16) */
	unsigned long FileOffset;   /* Byte offset in page file */
				    /* (on page boundary) */
    };

    /* XLATOFF */
    struct PageCmdHeader {
	unsigned char InFlags;		/* Input flags */
	unsigned char OutFlags;		/* Output flags - must be 0 on entry */
	unsigned char OpCount;		/* Number of operations */
	unsigned char Pad;		/* Preserve DWORD alignment */
	unsigned long Reserved1;	/* unused */
	unsigned long Reserved2;	/* unused */
	unsigned long Reserved3;	/* unused */
	struct PageCmd PageCmdList [1]; /* The actual commands */
    };
    /* XLATON */

    /* FSD_DoPageIO InFlags values */
    #define PGIO_FI_ORDER   0x01    /* Force Order of operations */

    /* FSD_DoPageIO OutFlags values */
    #define PGIO_FO_DONE    0x01    /* Operation done */
    #define PGIO_FO_ERROR   0x02    /* Operation failed */

    /* FSD_DoPageIO Status values */
    #define PGIO_ATTEMPTED  0x0f    /* Operation attempted */
    #define PGIO_FAILED	    0xf0    /* Operation failed */

/*****
 *
 * Declarations for the FSD entry points.
 *
 */

/* bit values for the IOflag parameter in various FS_ entry points */
#define IOFL_WRITETHRU	    0x10	/* Write through bit	*/
#define IOFL_NOCACHE	    0x20	/* No Cache bit		*/


/* XLATOFF */
int far pascal
FS_ALLOCATEPAGESPACE(
    struct sffsi far *psffsi,	    /* ptr to fs independent SFT */
    struct sffsd far *psffsd,	    /* ptr to fs dependent SFT	 */
    unsigned long     lSize,	    /* new size			 */
    unsigned long     lWantContig   /* contiguous chunk size	 */
);

int far pascal
FS_ATTACH(
    unsigned short,		/* flag		*/
    char far *,			/* pDev		*/
    void far *,			/* if remote drive
				     struct vpfsd far *
				   else if remote device
				     null ptr (0L)    */
    void far *,			/* if remote drive
				     struct cdfsd far *
				   else
				     struct devfsd far * */
    char far *,			/* pParm	*/
    unsigned short far *	/* pLen		*/
);
/* XLATON */

/* values for flag in FS_ATTACH */
#define FSA_ATTACH		0x00
#define FSA_DETACH		0x01
#define FSA_ATTACH_INFO		0x02

/* XLATOFF */
int far pascal
FS_CANCELLOCKREQUEST(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    void far *			/* pLockRang	*/
);

int far pascal
FS_CHDIR(
    unsigned short,		/* flag		*/
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pDir		*/
    unsigned short		/* iCurDirEnd	*/
);
/* XLATON */

/* values for flag in FS_CHDIR */
#define CD_EXPLICIT		0x00
#define CD_VERIFY		0x01
#define CD_FREE			0x02

/* XLATOFF */
int far pascal
FS_CHGFILEPTR(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    long,			/* offset	*/
    unsigned short,		/* type		*/
    unsigned short		/* IOflag	*/
);

/* values for type in FS_CHGFILEPTR */
#define CFP_RELBEGIN		0x00
#define CFP_RELCUR		0x01
#define CFP_RELEND		0x02

int far pascal
FS_CLOSE(
    unsigned short,		/* close type	*/
    unsigned short,		/* IOflag	*/
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *		/* psffsd	*/
);
/* XLATON */

#define FS_CL_ORDINARY	0
		/* ordinary close of file */
#define FS_CL_FORPROC	1
		/* final close of file for the process */
#define FS_CL_FORSYS	2
		/* final close of file for the system (for all processes) */

/* XLATOFF */
int far pascal
FS_COMMIT(
    unsigned short,		/* commit type	*/
    unsigned short,		/* IOflag	*/
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *		/* psffsd	*/
);
/* XLATON */

/* values for commit type */
#define FS_COMMIT_ONE	1
	/* commit for a single file */
#define FS_COMMIT_ALL	2
	/* commit due to buf reset - for all files */

/* XLATOFF */
int far pascal
FS_COPY(
    unsigned short,		/* copy mode	*/
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* source name	*/
    unsigned short,		/* iSrcCurrDirEnd	*/
    char far *,			/* pDst		*/
    unsigned short,		/* iDstCurrDirEnd	*/
    unsigned short		/* nameType (flags)	*/
);

int far pascal
FS_DELETE(
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pFile	*/
    unsigned short		/* iCurDirEnd	*/
);

int far pascal
FS_DOPAGEIO(
    struct sffsi far *	       psffsi,	    /* ptr to fs independent SFT    */
    struct sffsd far *	       psffsd,	    /* ptr to fs dependent SFT	    */
    struct PageCmdHeader far * pPageCmdList /* ptr to list of page commands */
);

void far pascal
FS_EXIT(
    unsigned short,		/* uid		*/
    unsigned short,		/* pid		*/
    unsigned short		/* pdb		*/
);

int far pascal
FS_FILEATTRIBUTE(
    unsigned short,		/* flag		*/
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pName	*/
    unsigned short,		/* iCurDirEnd	*/
    unsigned short far *	/* pAttr	*/
);
/* XLATON */

/* values for flag in FS_FILEATTRIBUTE */
#define FA_RETRIEVE		0x00
#define FA_SET			0x01

/* XLATOFF */
int far pascal
FS_FILEINFO(
    unsigned short,		/* flag		*/
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    unsigned short,		/* level	*/
    char far *,			/* pData	*/
    unsigned short,		/* cbData	*/
    unsigned short		/* IOflag	*/
);
/* XLATON */

/* values for flag in FS_FILEINFO */
#define FI_RETRIEVE		0x00
#define FI_SET			0x01

/* XLATOFF */
int far pascal
FS_FILEIO(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    char far *,			/* cbCmdList	*/
    unsigned short,		/* pCmdLen	*/
    unsigned short far *,	/* poError	*/
    unsigned short		/* IOflag	*/
);

int far pascal
FS_FILELOCKS(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    void far *,			/* pUnLockRange */
    void far *,			/* pLockRange	*/
    unsigned long,		/* timeout	*/
    unsigned long		/* flags	*/
);

int far pascal
FS_FINDCLOSE(
    struct fsfsi far *,		/* pfsfsi	*/
    struct fsfsd far *		/* pfsfsd	*/
);
/* XLATON */

/* values for flag in FS_FindFirst, FS_FindFromName, FS_FindNext */
#define	    FF_NOPOS		0x00
#define	    FF_GETPOS		0X01


/* XLATOFF */
int far pascal
FS_FINDFIRST(
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pName	*/
    unsigned short,		/* iCurDirEnd	*/
    unsigned short,		/* attr		*/
    struct fsfsi far *,		/* pfsfsi	*/
    struct fsfsd far *,		/* pfsfsd	*/
    char far *,			/* pData	*/
    unsigned short,		/* cbData	*/
    unsigned short far *,	/* pcMatch	*/
    unsigned short,		/* level	*/
    unsigned short		/* flags	*/
);

int far pascal
FS_FINDFROMNAME(
    struct fsfsi far *,		/* pfsfsi	*/
    struct fsfsd far *,		/* pfsfsd	*/
    char far *,			/* pData	*/
    unsigned short,		/* cbData	*/
    unsigned short far *,	/* pcMatch	*/
    unsigned short,		/* level	*/
    unsigned long,		/* position	*/
    char far *,			/* pName	*/
    unsigned short		/* flags	*/
);

int far pascal
FS_FINDNEXT(
    struct fsfsi far *,		/* pfsfsi	*/
    struct fsfsd far *,		/* pfsfsd	*/
    char far *,			/* pData	*/
    unsigned short,		/* cbData	*/
    unsigned short far *,	/* pcMatch	*/
    unsigned short,		/* level	*/
    unsigned short		/* flag		*/
);

int far pascal
FS_FINDNOTIFYCLOSE(
    unsigned short		/* handle	*/
);

int far pascal
FS_FINDNOTIFYFIRST(
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pName	*/
    unsigned short,		/* iCurDirEnd	*/
    unsigned short,		/* attr		*/
    unsigned short far *,	/* pHandle	*/
    char far *,			/* pData	*/
    unsigned short,		/* cbData	*/
    unsigned short far *,	/* pcMatch	*/
    unsigned short,		/* level	*/
    unsigned long		/* timeout	*/
);

int far pascal
FS_FINDNOTIFYNEXT(
    unsigned short,		/* handle	*/
    char far *,			/* pData	*/
    unsigned short,		/* cbData	*/
    unsigned short far *,	/* pcMatch	*/
    unsigned short,		/* infolevel	*/
    unsigned long		/* timeout	*/
);

int far pascal
FS_FLUSHBUF(
    unsigned short,		/* hVPB		*/
    unsigned short		/* flag		*/
);
/* XLATON */

/* values for flag in FS_FLUSH */
#define FLUSH_RETAIN		0x00
#define FLUSH_DISCARD		0x01

/* XLATOFF */
int far pascal
FS_FSCTL(
    union argdat far *,		/* pArgdat	*/
    unsigned short,		/* iArgType	*/
    unsigned short,		/* func		*/
    char far *,			/* pParm	*/
    unsigned short,		/* lenParm	*/
    unsigned short far *,	/* plenParmOut	*/
    char far *,			/* pData	*/
    unsigned short,		/* lenData	*/
    unsigned short far *	/* plenDataOut	*/
);
/* XLATON */

/* values for iArgType in FS_FSCTL */
#define FSCTL_ARG_FILEINSTANCE	0x01
#define FSCTL_ARG_CURDIR	0x02
#define FSCTL_ARG_NULL		0x03

/* values for func in FS_FSCTL */
#define FSCTL_FUNC_NONE		0x00
#define FSCTL_FUNC_NEW_INFO	0x01
#define FSCTL_FUNC_EASIZE	0x02

/* XLATOFF */
int far pascal
FS_FSINFO(
    unsigned short,		/* flag		*/
    unsigned short,		/* hVPB		*/
    char far *,			/* pData	*/
    unsigned short,		/* cbData	*/
    unsigned short		/* level	*/
);
/* XLATON */

/* values for flag in FS_FSINFO */
#define INFO_RETREIVE		0x00
#define INFO_SET		0x01

/* XLATOFF */
int far pascal
FS_INIT(
    char far *,			/* szParm	*/
    unsigned long,		/* pDevHlp	*/
    unsigned long far *		/* pMiniFSD	*/
);

int far pascal
FS_IOCTL(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    unsigned short,		/* cat		*/
    unsigned short,		/* func		*/
    char far *,			/* pParm	*/
    unsigned short,		/* lenParm	*/
    unsigned far *,		/* pParmLenInOut */
    char far *,			/* pData	*/
    unsigned short,		/* lenData	*/
    unsigned far *		/* pDataLenInOut */
);

int far pascal
FS_MKDIR(
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pName	*/
    unsigned short,		/* iCurDirEnd	*/
    char far *,			/* pEABuf	*/
    unsigned short		/* flags	*/
);

int far pascal
FS_MOUNT(
    unsigned short,		/* flag		*/
    struct vpfsi far *,		/* pvpfsi	*/
    struct vpfsd far *,		/* pvpfsd	*/
    unsigned short,		/* hVPB		*/
    char far *			/* pBoot	*/
);
/* XLATON */

/* values for flag in FS_MOUNT */
#define MOUNT_MOUNT		0x00
#define MOUNT_VOL_REMOVED	0x01
#define MOUNT_RELEASE		0x02
#define MOUNT_ACCEPT		0x03

/* XLATOFF */
int far pascal
FS_MOVE(
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pSrc		*/
    unsigned short,		/* iSrcCurDirEnd*/
    char far *,			/* pDst		*/
    unsigned short,		/* iDstCurDirEnd*/
    unsigned short		/* flags	*/
);

int far pascal
FS_NEWSIZE(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    unsigned long,		/* len		*/
    unsigned short		/* IOflag	*/
);


int far pascal
FS_NMPIPE(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    unsigned short,		/* OpType	*/
    union npoper far *,		/* pOpRec	*/
    char far *,			/* pData	*/
    char far *			/* pName	*/
);
/* XLATON */

/* Values for OpType in FS_NMPIPE   */

#define	    NMP_GetPHandState	    0x21
#define	    NMP_SetPHandState	    0x01
#define	    NMP_PipeQInfo	    0x22
#define	    NMP_PeekPipe	    0x23
#define	    NMP_ConnectPipe	    0x24
#define	    NMP_DisconnectPipe	    0x25
#define	    NMP_TransactPipe	    0x26
#define	    NMP_READRAW		    0x11
#define	    NMP_WRITERAW	    0x31
#define	    NMP_WAITPIPE	    0x53
#define	    NMP_CALLPIPE	    0x54
#define	    NMP_QNmPipeSemState	    0x58


/* XLATOFF */
int far pascal
FS_OPENCREATE(
    struct cdfsi far *,		/* pcdfsi		*/
    void far *,			/* if remote device
				     struct devfsd far *
				   else
				     struct cdfsd far * */
    char far *,			/* pName		*/
    unsigned short,		/* iCurDirEnd		*/
    struct sffsi far *,		/* psffsi		*/
    struct sffsd far *,		/* psffsd		*/
    unsigned long,		/* fhandflag/openmode	*/
    unsigned short,		/* openflag		*/
    unsigned short far *,	/* pAction		*/
    unsigned short,		/* attr			*/
    char far *,			/* pEABuf		*/
    unsigned short far *	/* pfgenFlag		*/
);
/* XLATON */

#define FOC_NEEDEAS	0x1	/*there are need eas for this file */

/* XLATOFF */
int far pascal
FS_OPENPAGEFILE (
    unsigned long far *pFlags,	    /* pointer to Flags		      */
    unsigned long far *pcMaxReq,    /* max # of reqs packed in list   */
    char far *	       pName,	    /* name of paging file	      */
    struct sffsi far * psffsi,	    /* ptr to fs independent SFT      */
    struct sffsd far * psffsd,	    /* ptr to fs dependent SFT	      */
    unsigned short     OpenMode,    /* sharing, ...		      */
    unsigned short     OpenFlag,    /* open flag for action	      */
    unsigned short     Attr,	    /* file attribute		      */
    unsigned long      Reserved	    /* reserved, must be zero	      */
);
/* XLATON */

/* values for *pFlags in FS_OPENPAGEFILE */
#define PGIO_FIRSTOPEN 0x00000001   /* first open of page file	      */
#define PGIO_PADDR     0x00004000   /* physical addresses required    */
#define PGIO_VADDR     0x00008000   /* 16:16 virtual address required */

/* XLATOFF */
int far pascal
FS_PATHINFO(
    unsigned short,		/* flag		*/
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pName	*/
    unsigned short,		/* iCurDirEnd	*/
    unsigned short,		/* level	*/
    char far *,			/* pData	*/
    unsigned short		/* cbData	*/
);
/* XLATON */

/* values for flag in FS_PATHINFO */
#define PI_RETRIEVE		0x00
#define PI_SET			0x01

/* XLATOFF */
int far pascal
FS_PROCESSNAME(
    char far *			/* pNameBuf	*/
);

int far pascal
FS_READ(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    char far *,			/* pData	*/
    unsigned short far *,	/* pLen		*/
    unsigned short		/* IOflag	*/
);

int far pascal
FS_RMDIR(
    struct cdfsi far *,		/* pcdfsi	*/
    struct cdfsd far *,		/* pcdfsd	*/
    char far *,			/* pName	*/
    unsigned short		/* iCurDirEnd	*/
);

int far pascal
FS_SETSWAP(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *		/* psffsd	*/
);

int far pascal
FS_SHUTDOWN(
    unsigned short,		/* usType	*/
    unsigned long		/* ulReserved	*/
);
/* XLATON */

/* values for usType in FS_SHUTDOWN */
#define SD_BEGIN		0x00
#define SD_COMPLETE		0x01

/* XLATOFF */

int far pascal
FS_VERIFYUNCNAME(
    unsigned short,		/* flag		*/
    char far *			/* pName	*/
);


/* XLATON */
/* values for flag in FS_PATHINFO */
/* These values depend on code in GetUNCFSDFromPath */
/* Don't change values without also changing code */
#define VUN_PASS1		 0x0000
#define VUN_PASS2		 0x0001
#define ERROR_UNC_PATH_NOT_FOUND 0x0003	  /* equals ERROR_PATH_NOT_FOUND */

/* XLATOFF */

int far pascal
FS_WRITE(
    struct sffsi far *,		/* psffsi	*/
    struct sffsd far *,		/* psffsd	*/
    char far *,			/* pData	*/
    unsigned short far *,	/* pLen		*/
    unsigned short		/* IOflag	*/
);

int far pascal
MFS_CHGFILEPTR(
    unsigned long,		/* offset	*/
    unsigned short		/* type		*/
);

int far pascal
MFS_CLOSE(
    void
);


int far pascal
MFS_INIT(
    void far *,			/* bootdata	*/
    char far *,			/* number io	*/
    long far *,			/* vectorripl	*/
    void far *,			/* bpb		*/
    unsigned long far *,	/* pMiniFSD	*/
    unsigned long far *		/* dump address */
);

int far pascal
MFS_OPEN(
    char far *,			/* name		*/
    unsigned long far *		/* size		*/
);

int far pascal
MFS_READ(
    char far *,			/* data		*/
    unsigned short far *	/* length	*/
);

int far pascal
MFS_TERM(
    void
);
/* XLATON */
