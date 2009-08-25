/* debug formats */

#define DBGTYPE_32CV  '1'  /* 32bit CodeView */
#define DBGTYPE_AIX   '2'  /* AIX */
#define DBGTYPE_16CV  '3'  /* 16bit CodeView */
#define DBGTYPE_32IBM '4'  /* 32bit IBM Debug */


#define    HLL    0x4C48                /* debug style value "HL"            */

//---------

/* HLL Header for DBGTYPE_32IBM */
typedef struct _HLLHdr
{
    UCHAR   achSignature[4];    /* LX Debug Info Signature, 'NB04' */
    ULONG   ulDirOffset;       /* Offset to the HLL Directory.
                                * (Relative to start of this header.) */
} HLLHDR, *PHLLHDR;


//---------

/* HLL Dictionary entry */

typedef struct _hlldirentry
{
 USHORT  SubSectType;
 USHORT  ModIndex;
 ULONG   SubSectOff;
 ULONG   SubSectLen;
} HLLDIRENTRY, *PHLLDIRENTRY;

typedef struct _oldhlldirentry
{
 USHORT  SubSectType;
 USHORT  ModIndex;
 ULONG   SubSectOff;
 USHORT  SubSectLen;
} OLDHLLDIRENTRY, *POLDHLLDIRENTRY;

//---------

/* HLL Directory for DBGTYPE_32IBM */
typedef struct _HLLDirectory
{
    USHORT       cb;              /* Size of this struct (minus aEntries)
                      offset of aEntries relative to start of this structure. */
    USHORT       cbEntry;     /* Size of the directory entries (HLLDIRENTRY). */
    ULONG        cEntries;       /* Count of directory entires. */
    HLLDIRENTRY  aEntries[1];    /* Directory. */
} HLLDIR, *PHLLDIR;

/* HLL Directory for other debug types */
typedef struct _OldHLLDirectory
{
  USHORT cEntries;             /* count of directory entries */
  OLDHLLDIRENTRY aEntries[1]; /* Directory */
} OLDHLLDIR, *POLDHLLDIR;

//---------


/*
 * HLL Directory entry types.
 */
#define HLL_DE_MODULES          0x0101  /* Filename */
#define HLL_DE_PUBLICS          0x0102  /* Public symbols */
#define HLL_DE_TYPES            0x0103  /* Types */
#define HLL_DE_SYMBOLS          0x0104  /* Symbols */
#define HLL_DE_LIBRARIES        0x0106  /* Libraries */
#define HLL_DE_SRCLINES         0x0105  /* Line numbers - (IBM C/2 1.1) */
#define HLL_DE_SRCLNSEG         0x0109  /* Line numbers - (MSC 6.00) */
#define HLL_DE_IBMSRC           0x010B  /* Line numbers - (IBM HLL) */


//-----
typedef struct _MOD
{
 ULONG              FileName;
 ULONG              FileNameLen;
 ULONG              Publics;
 ULONG              PubLen;
 ULONG              TypeDefs;
 ULONG              TypeLen;
 ULONG              Symbols;
 ULONG              SymLen;
 ULONG              LineNums;
 ULONG              LineNumsLen;

 struct
 {
  UCHAR  Pubs;
  UCHAR  Syms;
  UCHAR  Lins;
  UCHAR  Typs;
 } DbgFormatFlags;
}MODULE,*PMODULE;

/*****************************************************************************/
/* - Public symbol types                                                     */
/*****************************************************************************/
#define    TYPE_PUB_16  1
#define    TYPE_PUB_32  2


/*
 * HLL Segment
 */
typedef struct _HLL04SegInfo
{
    USHORT  usObject;          /* LX Object number. */
    ULONG   ulOffset;          /* Offset into the load image. */
    ULONG   ulLength;          /* Object length. */
} HLL04SEGINFO, *PHLL04SEGINFO;


/*
 * HLL Module (file)
 */
typedef struct _HLL04Module
{
    HLL04SEGINFO      SegInfo0;           /* Segment info entry 0. */
    USHORT  overlay;            /* unused. */
    USHORT  iLib;               /* Library number it came from. */
    UCHAR   cSegInfo;           /* Number of segment info entries. */
    UCHAR   pad;                /* 1 byte padding. */
    USHORT  usDebugStyle;       /* Debug style -'HL' */
    UCHAR   chVerMinor;         /* HLL version - minor number. */
    UCHAR   chVerMajor;         /* HLL version - major number. */
    UCHAR   cchName;            /* Filename length. */
    UCHAR   achName[1];         /* Filename. (*) */
    /* HLLSEGINFO      aSegInfo[] */    /* Array of segment info, starting with entry 1. (Starts at achName[cchName]) */
} HLL04MODULE, *PHLL04MODULE;

/*****************************************************************************/
/* - Public symbol records.                                                  */
/*****************************************************************************/
/*
 * HLL Public Symbols
 */
typedef struct _HLLPublicSymbol
{
    ULONG   ulOffset;        /* 32-bit offset (into the LX object) of the symbol location. */
    USHORT  usObject;        /* LX Object number. */
    USHORT  usType;          /* Symbol type index (into the type info data). */
    UCHAR   cchNameLen;      /* Size of name. */
    UCHAR   achName[1];      /* Name (*) */
} HLLPUBLICSYM32, *PHLLPUBLICSYM32;




//--- 

/*****************************************************************************/
/* Flags for the line number sections.                                       */
/*****************************************************************************/
#define    TYPE109_16     1             /* 16 bit MS/(C211 linked link386)   */
#define    TYPE109_32     2             /* 32 bit MSC.                       */
#define    TYPE105        3             /* IBM C211                          */
#define    TYPE10B_HL01   4             /* IBM HLL Level 1                   */
#define    TYPE10B_HL02   5             /* IBM HLL Level 2                   */
#define    TYPE10B_HL03   6             /* IBM HLL Level 3                   */
#define    TYPE10B_HL04   7             /* IBM HLL Level 4                   */

/*****************************************************************************/
/* Flags for the symbols sections.                                           */
/*****************************************************************************/
#define    TYPE104_C211   1
#define    TYPE104_C600   2
#define    TYPE104_HL01   3
#define    TYPE104_HL02   4
#define    TYPE104_CL386  5
#define    TYPE104_HL03   6
#define    TYPE104_HL04   7

/*****************************************************************************/
/* Flags for the types sections.                                             */
/*****************************************************************************/
#define    TYPE103_C211   1
#define    TYPE103_C600   2
#define    TYPE103_HL01   3
#define    TYPE103_HL02   4
#define    TYPE103_CL386  5
#define    TYPE103_HL03   6
#define    TYPE103_HL04   7




//----

/* Line Numbers HL03 */

typedef struct _firstentryhl03
{
 USHORT LineNumber;
 UCHAR  EntryType;
 UCHAR  Reserved;
 USHORT NumEntries;
 USHORT SegNum;
 ULONG  FileNameTableSize;
}HL03FIRSTENTRY,*PHL03FIRSTENTRY;

typedef struct _linenumbertableentryhl03
{
 USHORT LineNumber;
 USHORT SourceFileIndex;
 ULONG  Offset;
}LINE_NUMBER_TABLE_ENTRY_HL03,*PLINE_NUMBER_TABLE_ENTRY_HL03;

typedef struct _filenametableentryhl03
{
 ULONG  StartOfSource;
 ULONG  NumberOfSourceRecords;
 ULONG  NumberOfSourceFiles;
 UCHAR chNameLen;
}FILE_NAME_TABLE_ENTRY_HL03,*PFILE_NAME_TABLE_ENTRY_HL03;



//-- line numbers 109_32

typedef struct _109_32tableheader
{
 USHORT SegNum;
 USHORT NumEntries;
}FIRST_ENTRY_109_32, *PFIRST_ENTRY_109_32;

typedef struct _109_32tableentry
{
 USHORT LineNumber;
 ULONG  Offset;
}LINE_NUMBER_TABLE_ENTRY_109_32,*PLINE_NUMBER_TABLE_ENTRY_109_32;
