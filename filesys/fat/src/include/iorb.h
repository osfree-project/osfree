/*DDK*************************************************************************/
/*                                                                           */
/* COPYRIGHT    Copyright (C) 1992 IBM Corporation                           */
/*                                                                           */
/*    The following IBM OS/2 source code is provided to you solely for       */
/*    the purpose of assisting you in your development of OS/2 device        */
/*    drivers. You may use this code in accordance with the IBM License      */
/*    Agreement provided in the IBM Developer Connection Device Driver       */
/*    Source Kit for OS/2. This Copyright statement may not be removed.      */
/*                                                                           */
/*****************************************************************************/
/**************************************************************************
 *
 * SOURCE FILE NAME =  IORB.H
 *
 * DESCRIPTIVE NAME =  IORB defines the interface between Device Managers
 *                     and Adapter Device Drivers (ADDs).
 *
 *
 * VERSION = V2.0
 *
 * DATE
 *
 * DESCRIPTION :
 *
 * Purpose:
 *
 *
 *
 *
 * FUNCTIONS  :
 *
 *
 *
 * NOTES      : See ..\OEMSPEC\OEMSPEC.ASC for further details.
 *
 *
 * STRUCTURES
 *
 * EXTERNAL REFERENCES
 *
 *
 *
 * EXTERNAL FUNCTIONS
 *
*/

#ifndef __IORB_H__

/* ASM
if2
*/

#define __IORB_H__

/* ASM
endif
*/

/*
** I/O Request Block (IORB) Structures
*/

/* ASM
;
;       Resolve H2INC references for .INC version of file
;
        include  iorbtype.inc
*/

/* Typedefs to resolve forward references */

typedef struct _IORBH         IORBH;
typedef struct _IORBH         FAR *PIORBH;
typedef struct _IORBH         NEAR *NPIORBH;
typedef struct _IORBH         FAR *PIORB;
typedef struct _IORBH         NEAR *NPIORB;

typedef struct _DEVICETABLE   DEVICETABLE;
typedef struct _DEVICETABLE   FAR *PDEVICETABLE;
typedef struct _DEVICETABLE   NEAR *NPDEVICETABLE;

typedef struct _UNITINFO      UNITINFO;
typedef struct _UNITINFO      FAR *PUNITINFO;
typedef struct _UNITINFO      NEAR *NPUNITINFO;

typedef struct _ADAPTERINFO   ADAPTERINFO;
typedef struct _ADAPTERINFO   FAR *PADAPTERINFO;
typedef struct _ADAPTERINFO   NEAR *NPADAPTERINFO;

typedef struct _GEOMETRY      GEOMETRY;
typedef struct _GEOMETRY      FAR *PGEOMETRY;
typedef struct _GEOMETRY      NEAR *NPGEOMETRY;

typedef struct _SCATGATENTRY  SCATGATENTRY;
typedef struct _SCATGATENTRY  FAR *PSCATGATENTRY;
typedef struct _SCATGATENTRY  NEAR *NPSCATGATENTRY;

typedef struct _RESOURCE_ENTRY RESOURCE_ENTRY;
typedef struct _RESOURCE_ENTRY NEAR *NPRESOURCE_ENTRY;
typedef struct _RESOURCE_ENTRY FAR *PRESOURCE_ENTRY;

/*
** Interface for calling ADD entry point
*/

/* VOID FAR *(ADDEP) (PIORBH); */

/*
** IORB Header
*/

#define DM_WORKSPACE_SIZE       20
#define ADD_WORKSPACE_SIZE      16

typedef struct _IORBH   {               /* IOH */

  USHORT        Length;                 /* IORB Length                */
  USHORT        UnitHandle;             /* Unit Identifier            */
  USHORT        CommandCode;            /* Command Code               */
  USHORT        CommandModifier;        /* Command Modifier           */
  USHORT        RequestControl;         /* Request Control Flags      */
  USHORT        Status;                 /* Status                     */
  USHORT        ErrorCode;              /* Error Code                 */
  ULONG         Timeout;                /* Cmd completion timeout (s) */
  USHORT        StatusBlockLen;         /* Status block length        */
  NPBYTE        pStatusBlock;           /* Status block               */
  USHORT        Reserved_1;             /* Reserved, MBZ              */
  PIORB         pNxtIORB;               /* Pointer to next IORB       */
  PIORB         (FAR *NotifyAddress)(PIORB); /* Notification Address  */
  UCHAR         DMWorkSpace[DM_WORKSPACE_SIZE];   /* For use by DM    */
  UCHAR         ADDWorkSpace[ADD_WORKSPACE_SIZE]; /* For use by ADD   */

} IORBH;

/*
** IORB CommandCode and CommandModifier Codes.
**      CommandCode prefixed by IOCC and CommandModifier by IOCM.
*/

                                          /*---------------------------*/
                                          /* +----M=Manditory Support  */
                                          /* |    O=Optional Support   */
                                          /* |                         */
                                          /* V     Notes               */
                                          /*---------------------------*/
#define IOCC_CONFIGURATION        0x0001  /*                           */
#define IOCM_GET_DEVICE_TABLE     0x0001  /* M                         */
#define IOCM_COMPLETE_INIT        0x0002  /* O                         */
                                          /*---------------------------*/
#define IOCC_UNIT_CONTROL         0x0002  /*                           */
#define IOCM_ALLOCATE_UNIT        0x0001  /* M                         */
#define IOCM_DEALLOCATE_UNIT      0x0002  /* M                         */
#define IOCM_CHANGE_UNITINFO      0x0003  /* M                         */
                                          /*---------------------------*/
#define IOCC_GEOMETRY             0x0003  /*                           */
#define IOCM_GET_MEDIA_GEOMETRY   0x0001  /* M                         */
#define IOCM_SET_MEDIA_GEOMETRY   0x0002  /* O (M) >1 Media type       */
#define IOCM_GET_DEVICE_GEOMETRY  0x0003  /* M                         */
#define IOCM_SET_LOGICAL_GEOMETRY 0x0004  /* O (M) CHS Addressable     */
                                          /*---------------------------*/
#define IOCC_EXECUTE_IO           0x0004  /*                           */
#define IOCM_READ                 0x0001  /* M                         */
#define IOCM_READ_VERIFY          0x0002  /* M                         */
#define IOCM_READ_PREFETCH        0x0003  /* O                         */
#define IOCM_WRITE                0x0004  /* M                         */
#define IOCM_WRITE_VERIFY         0x0005  /* M                         */
                                          /*---------------------------*/
#define IOCC_FORMAT               0x0005  /*                           */
#define IOCM_FORMAT_MEDIA         0x0001  /* O (M) If HW requires      */
#define IOCM_FORMAT_TRACK         0x0002  /* O (M) If HW requires      */
#define IOCM_FORMAT_PROGRESS      0x0003  /* O                         */
                                          /*---------------------------*/
#define IOCC_UNIT_STATUS          0x0006  /*                           */
#define IOCM_GET_UNIT_STATUS      0x0001  /* M                         */
#define IOCM_GET_CHANGELINE_STATE 0x0002  /* M (O) Fixed Media Only    */
#define IOCM_GET_MEDIA_SENSE      0x0003  /* M                         */
#define IOCM_GET_LOCK_STATUS      0x0004  /* O                         */
                                          /*---------------------------*/
#define IOCC_DEVICE_CONTROL       0x0007  /*                           */
#define IOCM_ABORT                0x0001  /* O (M) SCSI                */
#define IOCM_RESET                0x0002  /* O (M) SCSI                */
#define IOCM_SUSPEND              0x0003  /* O (M) Floppy Driver       */
#define IOCM_RESUME               0x0004  /* O (M) Floppy Driver       */
#define IOCM_LOCK_MEDIA           0x0005  /* M (O) Fixed Media Only    */
#define IOCM_UNLOCK_MEDIA         0x0006  /* M (O) Fixed Media Only    */
#define IOCM_EJECT_MEDIA          0x0007  /* O (M) SCSI & Floppy driver*/
#define IOCM_GET_QUEUE_STATUS     0x0008  /* O (M) ATA/ATAPI Devices   */
                                          /* @V51531                   */
                                          /*---------------------------*/
#define IOCC_ADAPTER_PASSTHRU     0x0008  /*                           */
#define IOCM_EXECUTE_SCB          0x0001  /* O                         */
#define IOCM_EXECUTE_CDB          0x0002  /* O (M) SCSI Adapters       */
#define IOCM_EXECUTE_ATA          0x0003  /* o [001] ATA/ATAPI Devices */  /*@V151345*/
                                          /*---------------------------*/
#define IOCC_RESOURCE             0x0009  /*                           */
#define IOCM_REPORT_RESOURCES     0x0001  /* O (M) ATA/ATAPI Devices   */
                                          /*---------------------------*/

#define MAX_IOCC_COMMAND  IOCC_ADAPTER_PASSTHRU

/*
** Status flags returned in IORBH->Status
*/

#define IORB_DONE                0x0001  /* 1=Done,  0=In progress     */
#define IORB_ERROR               0x0002  /* 1=Error, 0=No error        */
#define IORB_RECOV_ERROR         0x0004  /* Recovered error            */
#define IORB_STATUSBLOCK_AVAIL   0x0008  /* Status Block available     */

/*
** Error Codes returned in IORBH->ErrorCode
*/

#define IOERR_RETRY                     0x8000

#define IOERR_CMD                       0x0100
#define IOERR_CMD_NOT_SUPPORTED         IOERR_CMD+1
#define IOERR_CMD_SYNTAX                IOERR_CMD+2
#define IOERR_CMD_SGLIST_BAD            IOERR_CMD+3
#define IOERR_CMD_SW_RESOURCE           IOERR_CMD+IOERR_RETRY+4
#define IOERR_CMD_ABORTED               IOERR_CMD+5
#define IOERR_CMD_ADD_SOFTWARE_FAILURE  IOERR_CMD+6
#define IOERR_CMD_OS_SOFTWARE_FAILURE   IOERR_CMD+7

#define IOERR_UNIT                      0x0200
#define IOERR_UNIT_NOT_ALLOCATED        IOERR_UNIT+1
#define IOERR_UNIT_ALLOCATED            IOERR_UNIT+2
#define IOERR_UNIT_NOT_READY            IOERR_UNIT+3
#define IOERR_UNIT_PWR_OFF              IOERR_UNIT+4

#define IOERR_RBA                       0x0300
#define IOERR_RBA_ADDRESSING_ERROR      IOERR_RBA+IOERR_RETRY+1
#define IOERR_RBA_LIMIT                 IOERR_RBA+2
#define IOERR_RBA_CRC_ERROR             IOERR_RBA+IOERR_RETRY+3

#define IOERR_MEDIA                     0x0400
#define IOERR_MEDIA_NOT_FORMATTED       IOERR_MEDIA+1
#define IOERR_MEDIA_NOT_SUPPORTED       IOERR_MEDIA+2
#define IOERR_MEDIA_WRITE_PROTECT       IOERR_MEDIA+3
#define IOERR_MEDIA_CHANGED             IOERR_MEDIA+4
#define IOERR_MEDIA_NOT_PRESENT         IOERR_MEDIA+5

#define IOERR_ADAPTER                   0x0500
#define IOERR_ADAPTER_HOSTBUSCHECK      IOERR_ADAPTER+1
#define IOERR_ADAPTER_DEVICEBUSCHECK    IOERR_ADAPTER+IOERR_RETRY+2
#define IOERR_ADAPTER_OVERRUN           IOERR_ADAPTER+IOERR_RETRY+3
#define IOERR_ADAPTER_UNDERRUN          IOERR_ADAPTER+IOERR_RETRY+4
#define IOERR_ADAPTER_DIAGFAIL          IOERR_ADAPTER+5
#define IOERR_ADAPTER_TIMEOUT           IOERR_ADAPTER+IOERR_RETRY+6
#define IOERR_ADAPTER_DEVICE_TIMEOUT    IOERR_ADAPTER+IOERR_RETRY+7
#define IOERR_ADAPTER_REQ_NOT_SUPPORTED IOERR_ADAPTER+8
#define IOERR_ADAPTER_REFER_TO_STATUS   IOERR_ADAPTER+9
#define IOERR_ADAPTER_NONSPECIFIC       IOERR_ADAPTER+10


#define IOERR_DEVICE                    0x0600
#define IOERR_DEVICE_DEVICEBUSCHECK     IOERR_DEVICE+IOERR_RETRY+1
#define IOERR_DEVICE_REQ_NOT_SUPPORTED  IOERR_DEVICE+2
#define IOERR_DEVICE_DIAGFAIL           IOERR_DEVICE+3
#define IOERR_DEVICE_BUSY               IOERR_DEVICE+IOERR_RETRY+4
#define IOERR_DEVICE_OVERRUN            IOERR_DEVICE+IOERR_RETRY+5
#define IOERR_DEVICE_UNDERRUN           IOERR_DEVICE+IOERR_RETRY+6
#define IOERR_DEVICE_RESET              IOERR_DEVICE+7
#define IOERR_DEVICE_NONSPECIFIC        IOERR_DEVICE+8
#define IOERR_DEVICE_ULTRA_CRC          IOERR_DEVICE+IOERR_RETRY+9  /*@V179942*/


/*
** Request Control flags in IORBH->RequestControl
*/

#define  IORB_ASYNC_POST        0x0001   /* Asynchronous post enabled  */
#define  IORB_CHAIN             0x0002   /* IORB Chain Link enabled    */
#define  IORB_CHS_ADDRESSING    0x0004   /* CHS fmt addr in RBA Field  */
#define  IORB_REQ_STATUSBLOCK   0x0008   /* Obtain Status Block Data   */
#define  IORB_DISABLE_RETRY     0x0010   /* Disable retries in ADD     */


/*
** ADAPTER CONFIGURATION IORB         (for IOCC_CONFIGURATION)
*/

typedef struct _IORB_CONFIGURATION   {  /* IOCFG */

  IORBH            iorbh;               /* IORB Header                  */
  DEVICETABLE far *pDeviceTable;        /* far ptr to adapter dev table */
  USHORT           DeviceTableLen;      /* Length of adapter dev table  */

} IORB_CONFIGURATION, FAR *PIORB_CONFIGURATION, NEAR *NPIORB_CONFIGURATION;

/* Adapter device table returned by GET_DEVICE_TABLE                      */

typedef struct _DEVICETABLE  {            /* IODT */

  UCHAR         ADDLevelMajor;            /* ADD Major Support Level       */
  UCHAR         ADDLevelMinor;            /* ADD Minor Support Level       */
  USHORT        ADDHandle;                /* ADD Handle                    */
  USHORT        TotalAdapters;            /* Number of adapters supported  */
  NPADAPTERINFO pAdapter[1];              /* Array of adapter info pointers*/

} DEVICETABLE, FAR *PDEVICETABLE;

/*
** Current ADD Level for DEVICETABLE->AddLevelMajor, AddLevelMinor
*/

#define ADD_LEVEL_MAJOR         0x01
#define ADD_LEVEL_MINOR         0x00


typedef struct _UNITINFO   {           /* IOUI */

  USHORT    AdapterIndex;              /* nth Adapter this driver     */
  USHORT    UnitIndex;                 /* nth Unit on this card       */
  USHORT    UnitFlags;                 /* Unit Flags                  */
  USHORT    Reserved;                  /* Reserved                    */
  USHORT    UnitHandle;                /* Assigned by ADD or Filter   */
  USHORT    FilterADDHandle;           /* Handle of Filter ADD  0=None*/
  USHORT    UnitType;                  /* Unit type                   */
  USHORT    QueuingCount;              /* Recommended number to queue */
  UCHAR     UnitSCSITargetID;          /* SCSI Target ID  (SCSI Only) */
  UCHAR     UnitSCSILUN;               /* SCSI Log.  Unit (SCSI Only) */

} UNITINFO;


/*
** Unit Flags for UNITINFO->UnitFlags
*/

#define UF_REMOVABLE     0x0001         /* Media can be removed            */
#define UF_CHANGELINE    0x0002         /* ChangeLine Supported            */
#define UF_PREFETCH      0x0004         /* Supports prefetch reads         */
#define UF_A_DRIVE       0x0008         /* Manages A:                      */
#define UF_B_DRIVE       0x0010         /* Manages B:                      */
#define UF_NODASD_SUPT   0x0020         /* Suppress DASD Mgr support       */
#define UF_NOSCSI_SUPT   0x0040         /* Suppress SCSI Mgr support       */
#define UF_DEFECTIVE     0x0080         /* Device is defective             */
#define UF_FORCE         0x0100         /* Device presense is forced       *//*@V155162*/
#define UF_NOTPRESENT    0x0200         /* Device is not present now       *//*@V155162*/
#define UF_LARGE_FLOPPY  0x0400         /* Treat removable as Large Floppy *//*@V189001*/


/*
** Unit Types for UNITINFO->UnitType
*/

#define  UIB_TYPE_DISK      0x0000     /* All Direct Access Devices        */
#define  UIB_TYPE_TAPE      0x0001     /* Sequencial Access Devices        */
#define  UIB_TYPE_PRINTER   0x0002     /* Printer Device                   */
#define  UIB_TYPE_PROCESSOR 0x0003     /* Processor type device            */
#define  UIB_TYPE_WORM      0x0004     /* Write Once Read Many Device      */
#define  UIB_TYPE_CDROM     0x0005     /* CD ROM Device                    */
#define  UIB_TYPE_SCANNER   0x0006     /* Scanner Device                   */
#define  UIB_TYPE_OPTICAL_MEMORY 0x0007 /* some Optical disk               */
#define  UIB_TYPE_CHANGER   0x0008     /* Changer device e.g. juke box     */
#define  UIB_TYPE_COMM      0x0009     /* Communication devices            */
#define  UIB_TYPE_ATAPI     0x000A     /* Unspecific ATAPI protocol device */
#define  UIB_TYPE_UNKNOWN   0x001F     /* Unknown or faked device          */ /*@V149971*/


typedef struct _ADAPTERINFO   {           /* IOAI */

  UCHAR            AdapterName[17];       /* Adapter Name ASCIIZ string    */
  UCHAR            Reserved;              /*                               */
  USHORT           AdapterUnits;          /* Number of units this adapter  */
  USHORT           AdapterDevBus;         /* Bus Type - Adapter to Device  */
  UCHAR            AdapterIOAccess;       /* I/O Type - Adapter to Host    */
  UCHAR            AdapterHostBus;        /* Bus Type - Adapter to Host    */
  UCHAR            AdapterSCSITargetID;   /* Adapter SCSI Target ID        */
  UCHAR            AdapterSCSILUN;        /* Adapter SCSI LUN              */
  USHORT           AdapterFlags;
  USHORT           MaxHWSGList;           /* Max HW S/G List Entries       */
  ULONG            MaxCDBTransferLength;  /* Max data length for CDBs      */
  UNITINFO         UnitInfo[1];           /* Unit info for each unit       */

} ADAPTERINFO;


/*
** Adapter Flags for ADAPTERINFO->AdapterFlags
*/

#define AF_16M                 0x0001  /* Supports >16M addresses        */
#define AF_IBM_SCB             0x0002  /* Supports IBM SCB commands      */
#define AF_HW_SCATGAT          0x0004  /* Supports scatter/gather in HW  */
#define AF_CHS_ADDRESSING      0x0008  /* Supports CHS addressing in HW  */
#define AF_ASSOCIATED_DEVBUS   0x0010  /* Supports >1 Device Bus         */


/*
** Adapter-to-Device protocol for ADAPTERINFO->AdapterDevBus
*/

#define  AI_DEVBUS_OTHER        0x0000
#define  AI_DEVBUS_ST506        0x0001 /* ST-506 CAM-I                     */
#define  AI_DEVBUS_ST506_II     0x0002 /* ST-506 CAM-II                    */
#define  AI_DEVBUS_ESDI         0x0003 /* ESDI                             */
#define  AI_DEVBUS_FLOPPY       0x0004 /* Diskette                         */
#define  AI_DEVBUS_SCSI_1       0x0005
#define  AI_DEVBUS_SCSI_2       0x0006
#define  AI_DEVBUS_SCSI_3       0x0007
#define  AI_DEVBUS_NONSCSI_CDROM 0x0008 /* Non-scsi CD-ROM interface bus    */

/*
** Note: A one of the following BUS WIDTH indicators should be
**       or'd with the above field.
*/

#define  AI_DEVBUS_FAST_SCSI    0x0100
#define  AI_DEVBUS_8BIT         0x0200
#define  AI_DEVBUS_16BIT        0x0400
#define  AI_DEVBUS_32BIT        0x0800


/*
** Adapter-to-Device protocol for ADAPTERINFO->AdapterIOAccess
*/

#define  AI_IOACCESS_OTHER      0x00
#define  AI_IOACCESS_BUS_MASTER 0x01
#define  AI_IOACCESS_PIO        0x02
#define  AI_IOACCESS_DMA_SLAVE  0x04
#define  AI_IOACCESS_MEMORY_MAP 0x08


/*
** Adapter-to-Host bus type for ADAPTERINFO->AdapterHostBus
*/

#define  AI_HOSTBUS_OTHER       0x00
#define  AI_HOSTBUS_ISA         0x01
#define  AI_HOSTBUS_EISA        0x02
#define  AI_HOSTBUS_uCHNL       0x03
#define  AI_HOSTBUS_UNKNOWN     0x0f

/*
** Note: A one of the following BUS WIDTH indicators should be
**       or'd with the above field.
*/

#define  AI_BUSWIDTH_8BIT       0x10
#define  AI_BUSWIDTH_16BIT      0x20
#define  AI_BUSWIDTH_32BIT      0x30
#define  AI_BUSWIDTH_64BIT      0x40
#define  AI_BUSWIDTH_UNKNOWN    0xf0


/*
** UNIT CONTROL IORB                  (for IOCC_UNIT_CONTROL)
*/

typedef struct _IORB_UNIT_CONTROL  {     /* IOUC */

  IORBH            iorbh;                /* IORB Header                  */
  USHORT           Flags;                /*                              */
  PUNITINFO        pUnitInfo;            /* For: IOCM_CHANGE_UNITINFO    */
  USHORT           UnitInfoLen;          /* Length of UnitInfo structure */

} IORB_UNIT_CONTROL, FAR *PIORB_UNIT_CONTROL, NEAR *NPIORB_UNIT_CONTROL;

/*
** DASD GEOMETRY IORB                      (for IOCC_GEOMETRY)
*/

typedef struct _IORB_GEOMETRY  {         /* IOGG */

  IORBH            iorbh;                /* IORB Header                  */
  PGEOMETRY        pGeometry;            /* far ptr to GEOMETRY block    */
  USHORT           GeometryLen;          /* Length of GEOMETRY block     */

} IORB_GEOMETRY, FAR *PIORB_GEOMETRY, NEAR *NPIORB_GEOMETRY;

typedef struct _GEOMETRY  {              /* IOG */

  ULONG    TotalSectors;
  USHORT   BytesPerSector;
  USHORT   Reserved;
  USHORT   NumHeads;
  ULONG    TotalCylinders;
  USHORT   SectorsPerTrack;

} GEOMETRY, FAR *PGEOMETRY, NEAR *NPGEOMETRY;



/*
** Scatter/Gather List Entry
*/

typedef struct _SCATGATENTRY  {      /* IOSG */

  ULONG         ppXferBuf;           /* Physical pointer to transfer buffer */
  ULONG         XferBufLen;          /* Length of transfer buffer           */

} SCATGATENTRY, FAR *PSCATGATENTRY, NEAR *NPSCATGATENTRY;

#define MAXSGLISTSIZE   (sizeof(SCATGATENTRY)) * 16

/*
** EXECUTE_IO IORB                    (for IOCC_EXECUTE_IO)
*/

typedef struct _IORB_EXECUTEIO  {            /* IOXIO */

  IORBH         iorbh;                       /* IORB Header                */
  USHORT        cSGList;                     /* Count of S/G list elements */
  PSCATGATENTRY pSGList;                     /* far ptr to S/G List        */
  ULONG         ppSGList;                    /* physical addr of  S/G List */
  ULONG         RBA;                         /* RBA Starting Address       */
  USHORT        BlockCount;                  /* Block Count                */
  USHORT        BlocksXferred;               /* Block Done Count           */
  USHORT        BlockSize;                   /* Size of a block in bytes   */
  USHORT        Flags;

} IORB_EXECUTEIO, FAR *PIORB_EXECUTEIO, NEAR *NPIORB_EXECUTEIO;


/*
** CHS Direct Addressing (Overlays RBA field)
*/

typedef struct _CHS_ADDR  {                  /* IOCHS */

  USHORT        Cylinder;
  UCHAR         Head;
  UCHAR         Sector;

} CHS_ADDR, FAR *PCHS_ADDR, NEAR *NPCHS_ADDR;


/*
** IORB specific flags for IORB_EXECUTE_IO->Flags
*/

#define  XIO_DISABLE_HW_WRITE_CACHE      0x0001
#define  XIO_DISABLE_HW_READ_CACHE       0x0002


/*
** FORMAT IORB                        (for IOCC_FORMAT)
*/


typedef struct _IORB_FORMAT  {               /* IOFMT */

  IORBH         iorbh;                       /* IORB Header                */
  USHORT        cSGList;                     /* Count of S/G list elements */
  PSCATGATENTRY pSGList;                     /* far ptr to S/G List        */
  ULONG         ppSGList;                    /* physical addr of  S/G List */
  USHORT        FormatCmdLen;                /*                            */
  PBYTE         pFormatCmd;                  /* SCSI CDB or Track Fmt Cmd  */
  UCHAR         Reserved_1[8];               /* Reserved, must not be modified*/

} IORB_FORMAT, FAR *PIORB_FORMAT, NEAR *NPIORB_FORMAT;


typedef struct _FORMAT_CMD_TRACK {           /* FMCT  */

  USHORT        Flags;
  ULONG         RBA;
  USHORT        cTrackEntries;
  USHORT        FmtGapLength;                                        /*@V80754*/
  USHORT        BlockSize;                                           /*@V82940*/

} FORMAT_CMD_TRACK, FAR *PFORMAT_CMD_TRACK, NEAR *NPFORMAT_CMD_TRACK;


/*
** Flags for FORMAT_CMD_TRACK->Flags
*/

#define FF_VERIFY       0x0001          /* Verify sectors after formatting */
#define FF_FMTGAPLEN    0x0002          /* InterSector GAP (bits)          */ /*V80754*/


/*
** ADAPTER PASS THROUGH IORB           (for IOCC_ADAPTER_PASSTHRU)
*/

typedef struct _IORB_ADAPTER_PASSTHRU  {  /* IOPT */

  IORBH         iorbh;                       /* IORB Header                */
  USHORT        cSGList;                     /* Count of S/G list elements */
  PSCATGATENTRY pSGList;                     /* far ptr to S/G List        */
  ULONG         ppSGLIST;                    /* physical addr of  S/G List */
  USHORT        ControllerCmdLen;
  PBYTE         pControllerCmd;
  ULONG         ppSCB;              /* phys ptr to SCB for SCB_PASSTHRU    */
  USHORT        Flags;

} IORB_ADAPTER_PASSTHRU, FAR *PIORB_ADAPTER_PASSTHRU, NEAR *NPIORB_ADAPTER_PASSTHRU;

/*
** IORB specific flags for IORB_ADAPTER_PASSTHRU->Flags
**
** Note: These flags apply to IOCM_EXECUTE_CDB
*/

#define PT_DIRECTION_IN     0x0001           /* Data xfer to host adapter  */


/*
** UNIT STATUS IORB                   (for IOCC_UNIT_STATUS)
*/

typedef struct _IORB_UNIT_STATUS  {        /* IOUS */

  IORBH      iorbh;
  USHORT     UnitStatus;

} IORB_UNIT_STATUS, FAR *PIORB_UNIT_STATUS, NEAR *NPIORB_UNIT_STATUS;


/*
** Unit Status for IORB_UNIT_STATUS->UnitStatus
**
** Note: These flags apply to IOCM_GET_UNIT_STATUS
*/

#define US_READY                0x0001          /* Unit ready          */
#define US_POWER                0x0002          /* Unit powered on     */
#define US_DEFECTIVE            0x0004          /* Unit operational    */

/*
** Unit Status for IORB_UNIT_STATUS->UnitStatus
**
** Note: These flags apply to IOCM_GET_CHANGELINE_STATE
*/

#define US_CHANGELINE_ACTIVE    0x0001          /* ChangeLine State    */


/*
** Unit Status for IORB_UNIT_STATUS->UnitStatus
**
** Note: These flags apply to IOCM_GET_MEDIA_SENSE
*/

#define US_MEDIA_UNKNOWN        0x0000          /* Unable to determine media */
#define US_MEDIA_720KB          0x0001          /* 720KB in 3.5" drive       */
#define US_MEDIA_144MB          0x0002          /* 1.44MB in 3.5" drive      */
#define US_MEDIA_288MB          0x0003          /* 2.88MB in 3.5" drive      */
#define US_MEDIA_LARGE_FLOPPY   0x0004          /* > 2.88 Floppies   190435  */

/*
** Unit Status for IORB_UNIT_STATUS->UnitStatus
**
** Note: These flags apply to IOCM_GET_LOCK_STATE
*/

#define US_LOCKED               0x0001          /* Unit locked - @V51531 */


/*
** DEVICE CONTROL IORB                (for IOCC_DEVICE_CONTROL
*/


typedef struct _IORB_DEVICE_CONTROL   {   /* IODC */

  IORBH      iorbh;                       /* IORB Header                */
  USHORT     Flags;
  USHORT     Reserved;
  USHORT     QueueStatus;
  USHORT     (FAR *IRQHandlerAddress)();                             /*@V93531*/
              /* IRQ Handler while suspended */

} IORB_DEVICE_CONTROL, FAR *PIORB_DEVICE_CONTROL, NEAR *NPIORB_DEVICE_CONTROL;


/*
** IORB specific flags for IORB_DEVICE_CONTROL->Flags
**
** Note: These flags apply to IOCM_SUSPEND
*/

#define DC_SUSPEND_DEFERRED      0x0000         /* Suspend after device idle */
#define DC_SUSPEND_IMMEDIATE     0x0001         /* Suspend after current req */
#define DC_SUSPEND_IRQADDR_VALID 0x0002  /* Suspend and notify IRQ *//*@V93531*/

#define DC_Queue_Empty          0                                    /*@V93531*/

#define MAX_IORB_SIZE   128

/*
** RESOURCE IORB                   (for IOCC_RESOURCE)
*/

typedef struct _IORB_RESOURCE  {        /* IORS */

  IORBH           iorbh;
  USHORT          Flags;
  USHORT          ResourceEntryLen;
  PRESOURCE_ENTRY pResourceEntry;

} IORB_RESOURCE, FAR *PIORB_RESOURCE, NEAR *NPIORB_RESOURCE;

typedef struct _DMA_ENTRY     NEAR *NPDMA_ENTRY;
typedef struct _DMA_ENTRY     FAR *PDMA_ENTRY;
typedef struct _IRQ_ENTRY     NEAR *NPIRQ_ENTRY;
typedef struct _IRQ_ENTRY     FAR *PIRQ_ENTRY;
typedef struct _PORT_ENTRY    NEAR *NPPORT_ENTRY;
typedef struct _PORT_ENTRY    FAR *PPORT_ENTRY;
typedef struct _MEMORY_ENTRY  NEAR *NPMEMORY_ENTRY;
typedef struct _MEMORY_ENTRY  FAR *PMEMORY_ENTRY;
typedef struct _PCI_IRQ_ENTRY NEAR *NPPCI_IRQ_ENTRY;
typedef struct _PCI_IRQ_ENTRY FAR *PPCI_IRQ_ENTRY;
typedef struct _UNIT_ENTRY    NEAR *NPUNIT_ENTRY;
typedef struct _UNIT_ENTRY    FAR *PUNIT_ENTRY;

typedef struct _RESOURCE_ENTRY {        /* RE */

  USHORT           Max_Resource_Entry;
  USHORT           cDMA_Entries;   /* Number of DMA entries                  */
  NPDMA_ENTRY      npDMA_Entry;    /* Near pointer to DMA resource entry     */
                                   /* array                                  */
  USHORT           cIRQ_Entries;   /* Number of IRQ entries                  */
  NPIRQ_ENTRY      npIRQ_Entry;    /* Near pointer to IRQ resource entry     */
                                   /* array                                  */
  USHORT           cPort_Entries;  /* Number of Port entries                 */
  NPPORT_ENTRY     npPort_Entry;   /* Near pointer to Port Range resource    */
                                   /* entry array                            */
  USHORT           cMemory_Entries;/* Number of Memory entries               */
  NPMEMORY_ENTRY   npMemory_Entry; /* Near pointer to Memory map resource    */
                                   /* entry array                            */
  USHORT           cPCI_IRQ_Entries;/* Number of PCI IRQ entries             */
  NPIRQ_ENTRY      npPCI_IRQ_Entry;/* Near pointer to PCI IRQ resource entry */

  ULONG            Reserved[4];    /* Reserved (Must be zero)                */

} RESOURCE_ENTRY;

typedef struct _RESOURCE_ENTRY  NEAR *NPRESOURCE_ENTRY;
typedef struct _RESOURCE_ENTRY  FAR *PRESOURCE_ENTRY;

/* Max_Resource_Entry will have one of these values. */
#define RE_DMA     1
#define RE_IRQ     2
#define RE_PORT    3
#define RE_MEMORY  4
#define RE_PCI_IRQ 5

/* Global Resource Entry Flags */
#define RE_ADAPTER_RESOURCE  0x1000  /* Resource is shared by units on an    */
                                     /* adapter                              */
#define RE_SYSTEM_RESOURCE   0x2000  /* Resource is shared by multiple       */
                                     /* adapters                             */
#define RE_RESOURCE_CONFLICT 0x4000  /* Indicates resource is in conflict    */
                                     /* with another unit's resources        */

typedef struct _DMA_ENTRY {          /* DE */
  USHORT     DMA_Flags;              /* DMA Flags                            */
  USHORT     DMA_Width;              /* DMA Width                            */
  USHORT     DMA_Type;               /* DMA Type                             */
  USHORT     DMA_Channel;            /* DMA Channel                          */
  USHORT     DMA_Arbitration_Level;  /* uChannel only                        */
} DMA_ENTRY;

/* Resource entry DMA flags */
#define RE_DMA_BUS_MASTER          0x0001 /* set for Bus Master              */

/* Resource entry DMA width values */
#define RE_DMA_WIDTH_8_BIT              1 /* set if 8 bit                    */
#define RE_DMA_WIDTH_16_BIT             2 /* set if 16 bit                   */

/* Resource entry DMA types */
#define RE_DMA_TYPE_A                   1 /* Type A DMA                      */
#define RE_DMA_TYPE_B                   2 /* Type B DMA                      */
#define RE_DMA_TYPE_F                   3 /* Type F DMA                      */

typedef struct _IRQ_ENTRY {        /* IE */
  USHORT          IRQ_Flags;       /* IRQ Flags                              */
  USHORT          IRQ_Value;       /* IRQ Line                               */
} IRQ_ENTRY;

/* Resource entry IRQ flags */
#define IRQ_RISING_EDGE_TRIGGERED  0x0001 /* ISA Standard                    */
#define IRQ_FALLING_EDGE_TRIGGERED 0x0002
#define IRQ_LOW_LEVEL_TRIGGERED    0x0004
#define IRQ_HIGH_LEVEL_TRIGGERED   0x0008


typedef struct _PORT_ENTRY{        /* PE */
  USHORT          Port_Flags;      /* Port Flags                             */
  USHORT          StartPort;       /* Start port of port range               */
  USHORT          cPorts;          /* Number of Byte size ports in range     */
} PORT_ENTRY;

typedef struct _MEMORY_ENTRY{      /* ME */
   USHORT         Memory_Flags;    /* Memory Flags                           */
   USHORT         Memory_Width;    /* Memory Width                           */
   ULONG          StartMemory;     /* Start address of memory range          */
   USHORT         cMemory;         /* Number of bytes in Memory range        */
} MEMORY_ENTRY;

/* Resource entry Memory flags */
#define MEMORY_WRITEABLE           0x0001  /* memory is writable             */
#define MEMORY_CACHEABLE           0x0002  /* read cacheable, write-through  */
#define MEMORY_SHADOWABLE          0x0004  /* memory is shadowable           */
#define MEMORY_ROM                 0x0008  /* memory is an expansion ROM     */

/* Resource entry Memory width values */
#define MEMORY_8_BIT                    1  /* 8 bit memory                   */
#define MEMORY_16_BIT                   2  /* 16 bit memory                  */

typedef struct _PCI_IRQ_ENTRY{     /* PCI */
   USHORT         PCI_IRQ_Flags;   /* PCI IRQ Flags                          */
   USHORT         PCI_IRQ_Value;   /* PCI IRQ Line                           */
} PCI_IRQ_ENTRY;

#endif /* __IORB_H__ */
