#ifndef  _REQPKTH_
#define  _REQPKTH_

#ifdef __IBMC__
   #define FAR
   #define near
#endif

typedef struct _RPH  FAR *PRPH;

#pragma pack(1)

typedef struct _RPH  {                  /* RPH */

  UCHAR         Len;
  UCHAR         Unit;
  UCHAR         Cmd;
  USHORT        Status;
  UCHAR         Flags;
  UCHAR         Reserved_1[3];
  PRPH          Link;
} RPH;

typedef struct _RP_GENIOCTL  {          /* RPGIO */

  RPH           rph;
  UCHAR         Category;
  UCHAR         Function;
  PUCHAR        ParmPacket;
  PUCHAR        DataPacket;
  USHORT        sfn;
} RP_GENIOCTL, FAR *PRP_GENIOCTL;

typedef struct _RPINIT  {

  RPH           rph;
  UCHAR         Unit;
  PFN           DevHlpEP;
  PSZ           InitArgs;
  UCHAR         DriveNum;
} RPINITIN, FAR *PRPINITIN;

typedef struct _BPB  {                  /* BPB */

  USHORT        BytesPerSector;
  UCHAR         SectorsPerCluster;
  USHORT        ReservedSectors;
  UCHAR         NumFATs;
  USHORT        MaxDirEntries;
  USHORT        TotalSectors;
  UCHAR         MediaType;
  USHORT        NumFATSectors;
  USHORT        SectorsPerTrack;
  USHORT        NumHeads;
  ULONG         HiddenSectors;
  ULONG         BigTotalSectors;
  UCHAR         Reserved_1[6];
} BPB, FAR *PBPB, *NPBPB;

typedef BPB    near *BPBS[];         /* An array of NEAR BPB Pointers */
typedef BPBS   FAR  *PBPBS;          /* A pointer to the above array  */





typedef struct _RPINITOUT  {            /* RPINO */

  RPH           rph;
  UCHAR         Unit;
  USHORT        CodeEnd;
  USHORT        DataEnd;
  PBPBS         BPBArray;
  USHORT        Status;
} RPINITOUT, FAR *PRPINITOUT;

#pragma pack()
#endif

