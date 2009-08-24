/* $Id: strat2.h,v 1.1.1.1 2003/05/21 13:36:00 pasha Exp $ */

/*static char *SCCSID = "@(#)strat2.h	6.3 91/04/29";*/
/*static char *SCCSID = "@(#)strat2.h	6.3 91/04/29";*/
/****************************************************************************\
*                                                                            *
* Strat2.inc - definitions for HPFS strategy-2 request list support 10/3/89  *
*                                                                            *
\****************************************************************************/


/*
 ***************************
 ** Request List Structure *
 ***************************
 *
 * Request Lists have the following format:
 *
 *       ==================================
 *       | BYTE 16   Request List Header  |
 *       | -------------------------------|
 *       | ARRAY     Requests             |
 *       ==================================
 *
 * Where the Request List Header has the following structure:
 */
typedef struct _Req_List_Header { /* RLH */
        ULONG  Count;             /* number of requests in Req List        */
        PVOID  Notify_Address;    /* 16:16 address of notification routine */
        USHORT Request_Control;   /* bitfield of flags defined below       */
        BYTE   Block_Dev_Unit;    /* logical unit number of volume         */
        BYTE   Lst_Status;        /* overall status for Req List           */
        ULONG  y_Done_Count;      /* count of request completed (internal) */
        ULONG  y_PhysAddr;        /* 32 bit phys addr of RLH    (internal) */
} Req_List_Header;

/*
 * this redefines some fields for use by DISK01
*/
typedef struct _Req_List_Header_1 { /* RLH */
        USHORT Short_Count;     /* number of requests in Req List           */
        USHORT Dummy1;          /* reserved must be zero                    */
        PVOID  Dummy2;          /* 16:16 address of notification routine    */
        USHORT Dummy3;          /* bitfield of flags defined below          */
        BYTE   Dummy4;          /* logical unit number of volume            */
        BYTE   Dummy5;          /* overall status for Req List              */
        USHORT Count_Done;      /* count of request completed (internal)    */
        USHORT Queued;          /* number of RH that caused queuing (int)   */
        USHORT Next_RH;         /* address of next RH to process (internal) */
        USHORT Current_RH;      /* address of RH currently in process (int) */
} Req_List_Header_1;

/*
 * List Request Control bit masks  (RLH_Request_Control)
 */
#define RLH_Req_From_PB 0x0001 /* Request came directly from Pinball */
#define RLH_Single_Req  0x0002 /* Single request in list */
#define RLH_Exe_Req_Seq 0x0004 /* Requests to be executed in sequence */
#define RLH_Abort_Err   0x0008 /* Abort on error */
#define RLH_Notify_Err  0x0010 /* Notify immediately on error */
#define RLH_Notify_Done 0x0020 /* Notify on completion */

/*
 * List Status bit masks (RLH_Lst_Status)
 *       - low nibble indicates completion status of reqs
 */
#define RLH_No_Req_Queued       0x00 /* No requests queued */
#define RLH_Req_Not_Queued      0x01 /* Some, but not all, requests queued */
#define RLH_All_Req_Queued      0x02 /* All requests queued */
#define RLH_All_Req_Done        0x04 /* All requests done or aborted */
#define RLH_Seq_In_Process      0x08 /* Requests being processed in sequence */
#define RLH_Abort_pendings      0x08 /* Abort list processing in progress */

/*
 * List Status bit masks (RLH_Lst_Status)
 *       - high nibble indicates error status of reqs in list
 */
#define RLH_No_Error            0x00 /* No error */
#define RLH_Rec_Error           0x10 /* Recoverable error has occurred */
#define RLH_Unrec_Error         0x20 /* Unrecoverable error has occurred */
#define RLH_Unrec_Error_Retry   0x30 /* Unrecoverable error after retry */

/*
 * Individual requests within the request list have the following format:
 *
 *       ===============================
 *       | BYTE 32   Request Header    |
 *       | ----------------------------|
 *       | BYTE      Command-Specific  |
 *       ===============================
 *
 * Where the Request Header has the following structure:
 */
typedef struct _Req_Header { /* RH */
        USHORT Length;          /* offset of the next request           */
        BYTE   Old_Command;     /* reserved, always 1Ch, same offset    */
                                /* as command code in OS/2 req header   */
        BYTE   Command_Code;    /* Pinball command request code         */
        ULONG  Head_Offset;     /* offset from begin of Req List Header */
        BYTE   Req_Control;     /* control flags bits defined below     */
        BYTE   Priority;        /* Priority of request defined below    */
        BYTE   Status;          /* status bitfield defined below        */
        BYTE   Error_Code;      /* Pinball errors defined below         */
        PVOID  Notify_Address;  /* 16:16 address called when done       */
        ULONG  Hint_Pointer;    /* 16:16 pointer to req packet in list  */
        ULONG  Waiting;         /* Waiting queue link pointer (internal)*/
        ULONG  FT_Orig_Pkt;     /* 32b ptr to original request(internal)*/
        ULONG  Physical;        /* 32 bit phys addr of RLH    (internal)*/
} Req_Header;

/* ASM
RHL_Waiting   equ   RH_Waiting + size Req_List_Header
RH_pIORB      equ   word ptr RH_Physical ; pointer back to IORB on queue
*/

/*
 * Pinball Length flag
 */
#define RH_LAST_REQ     0xFFFF  /* value of Length if last req in list */


/*
 * Pinball Command Prefix  (RH_Old_Command)
 */
#define PB_REQ_LIST     0x1C /* pinball request list */

/*
 * Pinball Command Codes (RH_Command_Code)
 */
#define PB_READ_X     0x1E /* pinball read */
#define PB_WRITE_X    0x1F /* pinball write */
#define PB_WRITEV_X   0x20 /* pinball write/verify */
#define PB_PREFETCH_X 0x21 /* pinball prefetch read */

/*
 * Pinball Status (RH_Status) - low nibble
 */
#define RH_NOT_QUEUED   0x00 /* not yet queued */
#define RH_QUEUED       0x01 /* queued and waiting */
#define RH_PROCESSING   0x02 /* in process */
#define RH_DONE         0x04 /* done */

/*
 * Pinball Status (RH_Status) - high nibble
 */
#define RH_NO_ERROR             0x00 /* No error                            */
#define RH_RECOV_ERROR          0x10 /* A recoverable error has occurred    */
#define RH_UNREC_ERROR          0x20 /* An unrecoverable error has occurred */
#define RH_UNREC_ERROR_RETRY    0x30 /* An unrecoverable error with retry   */
#define RH_ABORTED              0x40 /* The request was aborted             */

/*
 * Pinball Error Codes - see error.inc, device driver error codes
 *
 * Pinball Request Control bit masks (RH_Req_Control)
 */
#define RH_PB_REQUEST       0x01 /* Request came directly from Pinball */
#define RH_NOTIFY_ERROR     0x10 /* Notify on Error */
#define RH_NOTIFY_DONE      0x20 /* Notify on completion */

/*
 * Priorities
 */
#define PRIO_PREFETCH           0x00    /* Prefetch requests. */
#define PRIO_LAZY_WRITE         0x01    /* Lazy writer. */
#define PRIO_PAGER_READ_AHEAD   0x02    /* Read ahead, low priority pager I/O */
#define PRIO_BACKGROUND_USER    0x04    /* Background synchronous user I/O. */
#define PRIO_FOREGROUND_USER    0x08    /* Foreground synchronous user I/O. */
#define PRIO_PAGER_HIGH         0x10    /* High priority pager I/O. */
#define PRIO_URGENT             0x80    /* Urgent (e.g. power fail). */

/*
 * Pinball Read/Write request list entry
 */
typedef struct _PB_Read_Write { /* PB */
        Req_Header  RqHdr;      /* request header */
        ULONG  Start_Block;     /* start block for data transfer */
        ULONG  Block_Count;     /* number of blocks to transfer */
        ULONG  Blocks_Xferred;  /* number of blocks transferred */
        USHORT RW_Flags;        /* command specific control flags */
        USHORT SG_Desc_Count;   /* number of SG descriptors */
        USHORT SG_Desc_Count2;  /* number of SG descriptors   (internal) */
        USHORT reserved;        /* reserved for dd alignment  (internal) */
} PB_Read_Write;

/*
 * Pinball Read/Write flags (PB_RW_Flags)
 */
#define RW_Cache_WriteThru  0x0001 /* Cache write thru */
#define RW_Cache_Req        0x0002 /* Cache the request */



/* ASM

PB_SG_Array_Offset              equ size PB_Read_Write

*/

typedef struct _SG_Descriptor { /* SG */
        PVOID  BufferPtr;       /* 32 bit physcial pointer to the buffer */
        ULONG  BufferSize;      /* size of the buffer in bytes */
} SG_Descriptor;

typedef SG_Descriptor SGENTRY;
typedef SGENTRY FAR *PSGENTRY;

/*
;; DriverCaps is the the Driver Capabilities structure.  This structure is
;; used by FSD's to determine the specific features supported by the
;; device driver.
*/
typedef struct _DriverCaps { /* GDC_DD */
    USHORT Reserved ;                /* 0 reserved, set to zero                */
    BYTE   VerMajor ;                /* 2 major version of interface supported */
    BYTE   VerMinor ;                /* 3 minor version of interface supported */
    ULONG  Capabilities ;            /* 4 bitfield for driver capabilties      */
    PVOID  Strategy2 ;               /* 8 entry point for strategy-2           */
    PVOID  EndofInt ;                /* c entry point for DD_EndOfInt          */
    PVOID  ChgPriority ;             /* 10 entry point for DD_ChgPriority       */
    PVOID  SetRestPos ;              /* 14 entry point for DD_SetRestPos        */
    PVOID  GetBoundary ;             /* 18 entry point for DD_GetBoundary       */
/* KSO: new strat 3 */
    PVOID  Strategy3;                /* 1c entry point for strategy-3 */
    } DriverCaps ;

typedef DriverCaps FAR * P_DriverCaps ;

/*
;; Driver Capabilites bit mask (GDC_DD_Capabilities)
*/
#define GDC_DD_Read2      0x00000001  /* Read2 supported with DMA hardware     */
#define GDC_DD_DMA_Word   0x00000002  /* DMA on word-aligned buffers supported */
#define GDC_DD_DMA_Byte   0x00000006  /* DMA on byte-aligned buffers supported */
#define GDC_DD_Mirror     0x00000008  /* Disk Mirroring supported by driver  */
#define GDC_DD_Duplex     0x00000010  /* Disk Duplexing supported by driver  */
#define GDC_DD_No_Block   0x00000020  /* Driver does not block in Strategy 2 */
#define GDC_DD_16M        0x00000040  /* >16M memory supported               */
/* KSO: new strat 3 */
#define GDC_DD_STRAT3     0x00000080

/*
;; VolChars is the volume charactieristics table.  This table describes the
;; physical characteristics of the logical volume, under the simplifying (but
;; not necessarily true) assumption that this is the only volume on the
;; physical drive.
*/
typedef struct _VolChars { /* VC */
    USHORT VolDescriptor ;           /* see equates below                    */
    USHORT AvgSeekTime ;             /* milliseconds, if unknown, FFFFh      */
    USHORT AvgLatency ;              /* milliseconds, if unknown, FFFFh      */
    USHORT TrackMinBlocks ;          /* blocks on smallest track             */
    USHORT TrackMaxBlocks ;          /* blocks on largest track              */
    USHORT HeadsPerCylinder ;        /* if unknown or not applicable use 1   */
    ULONG  VolCylinderCount ;        /* number of cylinders on volume        */
    ULONG  VolMedianBlock ;          /* block in center of volume for seek   */
    USHORT MaxSGList ;               /* Adapter scatter/gather list limit    */
    } VolChars ;

typedef VolChars FAR * P_VolChars ;

/*
;; Volume Descriptor bit masks (VC_VolDescriptor)
*/
#define VC_REMOVABLE_MEDIA    0x0001  /* Volume resides on removable media   */
#define VC_READ_ONLY          0x0002  /* Volume is read-only                 */
#define VC_RAM_DISK           0x0004  /* Seek time independant of position   */
#define VC_HWCACHE            0x0008  /* Outboard cache supported            */
#define VC_SCB                0x0010  /* SCB protocol supported              */
#define VC_PREFETCH           0x0020  /* Prefetch read supported             */


/*
 * The rest of this file is not needed by standard HPFS
 */

/* ASM
;
;
;***************************
;* Queue Linkage Structure *
;***************************
;
; This structure is private to the driver
;
; Queue linkage will be allocated from the local data segment. A word
;       pointer will point to the next link structure in the queue and a
;       double word will serve as a far pointer to the actual request.
;
; The linkage provided in the elements of the Pinball request array will
;       point back to the queue linkage to fascilitate removal of a request
;       if a request list was cancelled or a request's priority changed.
;
NUMQLINKS = 50                          ; number of QLink elements
;
QLink struc
        QLinkNext               dw ?    ; pointer to next queue link
        QLinkPriority           db ?    ; request priority
        QLinkFlags              db ?    ; pinball indicator
        QLinkRequest            dd ?    ; far pointer to pinball request
        QLinkStartBlock         dd ?    ; disk I/O start block for sort key
        QLinkReqHandle          dw ?    ; used when notifying FT_Done
        QLinkPartNumber         db ?    ; FT partition number
        QLinkDevUnit            db ?    ; device unit number from packet
        QLinkRelBlock           dd ?    ; FT relative start block
QLink ends
;
; QLinkFlags Values
;
        QL_OS2          equ     00h     ; this is standard OS/2 I/O
        QL_Pinball      equ     01h     ; this is HPFS386 I/O
        QL_HPFS386      equ     01h     ; this is HPFS386 I/O
        FT_AUX_PKT      equ     02h     ; this is an auxilliary packet
        FT_MIRROR_IO    equ     04h     ; mirrored I/O
;
; end of assembly section
*/

/* ASM

;***********************************
;* GetDeviceSupport Request Packet *
;***********************************

*/
/* ASM
;
;; define the device driver request packet for the get device
;; capabilities (hex 1d) command
;
Pkt_1d      struc
                db  PktHeadSize dup (?) ; device driver request packet header.
                db  3 dup (?)           ; reserved.
Pkt_1d_DCS_Addr dd  ?                   ; pointer to dcs area in 16:16 format.
Pkt_1d_VCS_Addr dd  ?                   ; pointer to vcs area in 16:16 format.
Pkt_1d      ends

access_type_write equ   1
access_type_read  equ   0


;
ERROR_INT13_BAD_COMMAND         EQU     01h
ERROR_INT13_DRIVE_NOT_READY     EQU     80h
;
;***********************************************************
;* IOCTL Packet for IOPD_SB - Set Simulated Bad sectors *  *
;* NOTE: THIS IS AN INTERNAL IOCTL FOR DEBUG PURPOSES ONLY *
;***********************************************************

IOPD_SB             equ 04Bh    ; category 9 function 4Bh
SSB_MAXCNT          equ 012h    ; only allow 18 simulated bad sectors
;* SSB commands
SSB_SET             equ 0
SSB_CLEAR           equ 1
SSB_CLEARALL        equ 2
SSB_LIST            equ 3

IOCTLSETSBAD struc
    ssb_cmd         db  ?
    ssb_cnt         db  ?       ; on return will be
    ssb_sec         dd  ?       ; sector to be marked bad/ cleared
IOCTLSETSBAD ends

;* cnt field returns # of slots clear on a SET cmd and # of slots used
;* on a clear cmd.

;PBTEST equ 1   ; for Pinball testing, enabling simbad code

;
; this next routine is two macros for spin-locking
; critical sections. In the first macro a semaphore
; is tested for availability. It is tested in such
; a way start after the first test the data should
; be in the cache and there should be no more memory
; accesses. Once it is available we try to grab it
; and if someone beats us to it, we go back to
; checking the semaphore in the cache.
; The second routine simply writes a zero to the
; semaphore to clear it (and enables interrupts).
;
STARTCRITICAL macro semaphore
        push    ax
        mov     al, 1
@@:     sti
        cmp     semaphore, 0    ; Q: semaphore available?
        jnz     @B              ;    N: try again
        cli
        xchg    semaphore, al   ; this is where we actually set it
        or      al,al           ; Q: did we set it?
        jnz     @B              ;   N: start over again
        pop     ax
        endm

X_STARTCRITICAL macro semaphore
        push    ax
        mov     al, 1
@@:     xchg    semaphore, al
        or      al,al           ; Q: did we set it?
        jnz     @B              ;   N: start over again
        pop     ax
        endm

ENDCRITICAL macro semaphore
        mov     semaphore, 0
        sti
        endm

X_ENDCRITICAL macro semaphore
        mov     semaphore, 0
        endm

; State Trace Buffer Pointer Mask
        ST_BufferPtr_Mask   equ 0FFFh       ; only 4k state trace buffer
;       STATETRACE          equ 1           ; enable state trace debugging

; SimBad
        CACHETEST           equ 1           ; enable Bad Sector Simulation

; Minimum MaxSgList
; If the scsi card can't handle at least 16 s/g entries then (for fat)
; volumes feed it strat1.
   FAT_MIN_MAX_SGLIST equ 16


; FAT strat2 request packet. Note that the SG field is pre-set for 60 pages.
; The worst case is around 18 pages. But we need more for the Lazy Writer
; which will not require more SG_Descriptior but just a few more RB_Read_Write.
; If we add more PB_Read_Writes then the old FAT strategy2 would not work, so
; for now we fake it out.

fats2   struc
fs2_rlh db      size Req_List_Header dup(?)
fs2_rh  db      size PB_Read_Write dup(?)
fs2_sg  db 60 * size SG_Descriptor dup(?)
fs2_sem dd      ?
fats2   ends

; Used by FAT as the max size of a strat2 req pkt

S2_RW_RQPKT_SIZE  EQU   size fats2
*/
