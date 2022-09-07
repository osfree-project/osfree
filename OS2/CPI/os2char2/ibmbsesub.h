/* 
  This is custom bsesub.h file used to develop IBM compatible 32TEXT API.
  Don't use it in another places.

*/

typedef unsigned long HKBD;   /* keyboard handle */
typedef HKBD        * PHKBD;

/*
 * Flags for KbdCharin, KbdGetConsole, and KbdStringIn
 * (IO_PEEKWAIT not available in OS/2 2.x)
 */
#define IO_WAIT               0
#define IO_NOWAIT             1
#define IO_PEEK               2
#define IO_PEEKWAIT           3
#define IO_EXTENDED           4

/*
 * Console Event Kinds (CK_MOUSE, and CK_NOTIFY not in OS/2 2.x)
 */
#define CK_NONE               0
#define CK_KEY                1
#define CK_CHAR               2
#define CK_MOUSE              3
#define CK_NOTIFY             4

/*
 * Translation status for KbdCharin and KbdXlate (fbStatus)
 */
#define KBDTRF_SHIFT_KEY_IN        0x01
#define KBDTRF_NOT_A_CHAR          0x02
#define KBDTRF_CONVERSION_REQUEST  0x20
#define KBDTRF_FINAL_CHAR_IN       0x40
#define KBDTRF_INTERIM_CHAR_IN     0x80

/*
 * Key structure for KbdCharIn and KbdPeek
 * (Unicode character added, change in lengths)
 */
typedef  struct _KBDKEYINFO {
    USHORT   ucUniChar;       /* Unicode char (added field)     */
    USHORT   chChar;          /* Char in current codepage       */
    UCHAR    chScan;          /* Scan code                      */
    UCHAR    fbStatus;        /* Final/Interim bits             */
    USHORT   fsState;         /* Shift state                    */
    USHORT   VKey;            /* Virtual key                    */
    UCHAR    bNlsShift;       /* Extended shift status          */
    UCHAR    resv;            /* Reserved                       */
    ULONG    time;            /* Time stamp                     */
} KBDKEYINFO;
typedef KBDKEYINFO * PKBDKEYINFO;

/*
 * Definitions for usage of bNlsShift
 */
#define  NLSS_NLS1    0x01   /* Fullwidth, National layer   */
#define  NLSS_NLS2    0x02   /* Katakana, JAMO, phonetic    */
#define  NLSS_NLS3    0x04   /* Hiragana, Hangeul, TsangJye */
#define  NLSS_APPL    0x10   /* Application bit             */
#define  NLSS_NLS4    0x40   /* Romanji, HanjaCsr           */
#define  NLSS_KANJI   0x80   /* Kanji, Hanja                */

/*
 * Length structure for KbdStringIn
 */
typedef struct _STRINGINBUF {
    ULONG  cb;           /* Size of input buffer (was ushort) */
    ULONG  cchIn;        /* Number of bytes read (was ushort) */
} STRINGINBUF;
typedef STRINGINBUF * PSTRINGINBUF;

/*
 * Values and layout of hardware ID for KbdGetHWID
 */
#define KBHWID_UNKNOWN        0x0000
#define KBHWID_AT             0x0001
#define KBHWID_ENHANCED       0xab41
#define KBHWID_SMALL          0xab54
#define KBHWID_MFIOLD         0xab85
#define KBHWID_MFI            0xab86
typedef struct _KBDHWID {
    USHORT cb;           /* Size of buffer           */
    USHORT idKbd;        /* Keyboard ID (KBHWID_)    */
    USHORT idSecond;     /* Secondary ID             */
} KBDHWID;
typedef KBDHWID  * PKBDHWID;

/*
 * KBDINFO structure, for KbdSetStatus and KbdGetStatus
 */
#if 0
// Same as in 16bit
typedef struct _KBDINFO {
    USHORT cb;
    USHORT fsMask;
    USHORT chTurnAround;
    USHORT fsInterim;           /* Upper byte is NLS Shift */
    USHORT fsState;
} KBDINFO;
typedef KBDINFO * PKBDINFO;
#endif

/*
 * Mask bits for KBDINFO in KbdSetStatus and KbdGetStatus (fsMask)
 */
#define KEYBOARD_ECHO_ON                0x0001
#define KEYBOARD_ECHO_OFF               0x0002
#define KEYBOARD_ECHO_SAME              0x0000

#define KEYBOARD_BINARY_MODE            0x0004
#define KEYBOARD_ASCII_MODE             0x0008
#define KEYBOARD_TERM_MODE              0x0200
#define KEYBOARD_SAME_MODE              0x0000

#define KEYBOARD_MODIFY_STATE           0x0010
#define KEYBOARD_MODIFY_INTERIM         0x0020
#define KEYBOARD_MODIFY_TURNAROUND      0x0040
#define KEYBOARD_2B_TURNAROUND          0x0080

#define KEYBOARD_SHIFT_REPORT           0x0100

/*
 * Status bits for KBDINFO in KbdSetStatus and KbdGetStatus (fsState)
 */
#ifndef INCL_DOSDEVIOCTL           /* Also defined in bsedev.h */
    #define KBDSTF_RIGHTSHIFT           0x0001
    #define KBDSTF_LEFTSHIFT            0x0002
    #define KBDSTF_CONTROL              0x0004
    #define KBDSTF_ALT                  0x0008
    #define KBDSTF_SCROLLLOCK_ON        0x0010
    #define KBDSTF_NUMLOCK_ON           0x0020
    #define KBDSTF_CAPSLOCK_ON          0x0040
    #define KBDSTF_INSERT_ON            0x0080
    #define KBDSTF_LEFTCONTROL          0x0100
    #define KBDSTF_LEFTALT              0x0200
    #define KBDSTF_RIGHTCONTROL         0x0400
    #define KBDSTF_RIGHTALT             0x0800
    #define KBDSTF_SCROLLLOCK           0x1000
    #define KBDSTF_NUMLOCK              0x2000
    #define KBDSTF_CAPSLOCK             0x4000
    #define KBDSTF_SYSREQ               0x8000
#endif  /* INCL_DOSDEVIOCTL */

/*
 * Keyboard function prototypes
 */
APIRET APIENTRY  KbdCharIn(PKBDKEYINFO CharData, ULONG Wait, HKBD hkbd);

APIRET APIENTRY  KbdGetConsole(PVOID Data, PULONG Kind, ULONG Flags,
                 HKBD hkbd);

APIRET APIENTRY  KbdFlushBuffer (HKBD hkbd);

APIRET APIENTRY  KbdGetCp (ULONG ulReserved, PUSHORT pidCP, HKBD hkbd);

APIRET APIENTRY  KbdGetHWID (PKBDHWID pkbdhwid, HKBD hkbd);

APIRET APIENTRY  KbdGetLayout(PSZ name, HKBD hkbd);

APIRET APIENTRY  KbdGetLayoutUni(USHORT * name, HKBD hkbd);

APIRET APIENTRY  KbdGetStatus (PKBDINFO pkbdinfo, HKBD hkbd);

APIRET APIENTRY  KbdPeek (PKBDKEYINFO pkbci, HKBD hkbd);

APIRET APIENTRY  KbdSetCp (ULONG ulReserved, USHORT pidCP,  HKBD hkbd);

APIRET APIENTRY  KbdSetLayout(PSZ name, HKBD hkbd);

APIRET APIENTRY  KbdSetLayoutUni(USHORT * name, HKBD hkbd);

APIRET APIENTRY  KbdSetRate (ULONG rate, ULONG delay, HKBD hkbd);

APIRET APIENTRY  KbdSetStatus (PKBDINFO pkbdinfo, HKBD hkbd);

APIRET APIENTRY  KbdStringIn (PCH pch, PSTRINGINBUF pchIn,
                              ULONG Flag, HKBD hkbd);

APIRET APIENTRY  KbdStringInUni (USHORT * pch, PSTRINGINBUF pchIn,
                                 ULONG Flag, HKBD hkbd);

APIRET APIENTRY  KbdXlate (PKBDKEYINFO pkbdtrans, HKBD hkbd);


/*
 *  Video Input Output (Vio) functions
 */

/*
 * Vio handles
 */
typedef unsigned long   HVIO;
typedef HVIO        *   PHVIO;

/*
 * VioCreatePS format constants
 */
#define FORMAT_CGA      0x0001
#define FORMAT_VGA      0x0001
#define FORMAT_4BYTE    0x0003
#define FORMAT_EXTEND   0x0003
#define FORMAT_UNICODE  0x0002

/*
 * options for VioQueryFonts
 */
//#define VQF_PUBLIC      0x0001L
//#define VQF_PRIVATE     0x0002L
//#define VQF_ALL         0x0003L

/*
 * Cursor structure for VioSetCurType and VioGetCurType()
 */
#if 0
// Same as in 16-bit
typedef struct _VIOCURSORINFO {
    USHORT   yStart;
    USHORT   cEnd;
    USHORT   cx;
    USHORT   attr;
} VIOCURSORINFO;
typedef VIOCURSORINFO * PVIOCURSORINFO;
#endif

/*
 * Mode structure for VioSetMode and VioGetMode
 * (field added for 32 bit alignment)
 */
typedef  struct _VIOMODEINFO     /* viomi */
        {
        USHORT cb;            /* structure length */
        UCHAR  fbType;        /* mode type */
        UCHAR  color;         /* number of colors */
        USHORT col;           /* number of columns */
        USHORT row;           /* number of rows */
        USHORT hres;          /* horizontal size in pels */
        USHORT vres;          /* vertical size in pels */
        UCHAR  fmt_ID;        /* format of attributes */
        UCHAR  attrib;        /* length of attributes */
        USHORT resvd;         /* Reserved   (added field) */
        ULONG  buf_addr;
        ULONG  buf_length;
        ULONG  full_length;
        ULONG  partial_length;
        PCH    ext_data_addr;
        } VIOMODEINFO;
typedef VIOMODEINFO *PVIOMODEINFO;


/*
 * VIOMODEINFO.fbType constants
 */
//#define VGMT_OTHER            1
//#define VGMT_COLOR            1
//#define VGMT_GRAPHICS         2
//#define VGMT_DISABLEBURST     4
//#define VGMT_NOTBIOS          8

/*
 * VIOMODEINFO.color constants
 */
//#define COLORS_2              1
//#define COLORS_4              2
//#define COLORS_16             4
//#define COLORS_256            8
//#define COLORS_64K           16
//#define COLORS_16M           24

/*
 * Return types for VioCheckCharType
 */
#define VCC_SBCSCHAR          0
#define VCC_DBCSFULLCHAR      1
#define VCC_DBCS1STHALF       2
#define VCC_DBCS2NDHALF       3

/*
 * Values for graphics mode screen synchronization
 */
#define VSRWI_SAVEANDREDRAW   0
#define VSRWI_REDRAW          1

#define VSRWN_SAVE            0
#define VSRWN_REDRAW          1

#define UNDOI_GETOWNER        0
#define UNDOI_RELEASEOWNER    1

#define UNDOK_ERRORCODE       0
#define UNDOK_TERMINATE       1

#define LOCKIO_NOWAIT         0
#define LOCKIO_WAIT           1

#define LOCK_SUCCESS          0
#define LOCK_FAIL             1

/*
 * Values for VioSetAnsi and VioGetAnsi
 */
//#define ANSI_ON               1
//#define ANSI_OFF              0

/*
 * Values for VioPopup
 */
//#define VMWR_POPUP            0
//#define VMWN_POPUP            0
//#define VP_NOWAIT             0
//#define VP_WAIT               1
//#define VP_OPAQUE             0
//#define VP_TRANSPARENT        2

/*
 * VIOCONFIGINFO.adapter constants
 */
//#define DISPLAY_MONOCHROME    0
//#define DISPLAY_CGA           1
//#define DISPLAY_EGA           2
//#define DISPLAY_VGA           3
//#define DISPLAY_8514A         7
//#define DISPLAY_IAA           8
//#define DISPLAY_XGA           9
//#define DISPLAY_SVGA         10
//#define DISPLAY_NONVGA       12

/*
 * VIOCONFIGINFO.display constants
 */
//#define MONITOR_MONOCHROME    0
//#define MONITOR_COLOR         1
//#define MONITOR_ENHANCED      2
//#define MONITOR_8503          3
//#define MONITOR_851X_COLOR    4
//#define MONITOR_8514          9
//#define MONITOR_FLATPANEL    10
//#define MONITOR_LCD640       10
//#define MONITOR_8507_8604    11
//#define MONITOR_8515         12
//#define MONITOR_9515         15
//#define MONITOR_9517         17
//#define MONITOR_9518         18
//#define MONITOR_OEM          19
//#define MONITOR_LCD800       21
//#define MONITOR_LCD1024      22

/*
 * Configuration structure for VioGetConfig
 * (all fields changed to ULONG)
 */
typedef  struct _VIOCONFIGINFO {
    ULONG   cb;              /* Size of data      */
    ULONG   adapter;         /* Adapter type      */
    ULONG   display;         /* Display type      */
    ULONG   cbMemory;        /* Adapter memory    */
    ULONG   Configuration;   /* Which config      */
    ULONG   Version;         /* Driver version    */
    ULONG   Flags;           /*                   */
    ULONG   HWBufferSize;    /* State save size   */
    ULONG   FullSaveSize;    /* Buffer save size  */
    ULONG   PartSaveSize;    /* Partial save size */
    ULONG   VideoID;         /* Adapter ID        */
    ULONG   VideoData;       /* Adapter user data */
} VIOCONFIGINFO;
typedef VIOCONFIGINFO * PVIOCONFIGINFO;

#define VIO_CONFIG_CURRENT    0
#define VIO_CONFIG_PRIMARY    1
#define VIO_CONFIG_SECONDARY  2

/*
 *  Structures for VioSetState and VioGetState
 */
#if 0
// Same as in 16bit
typedef struct _VIOPALSTATE {
    USHORT  cb;
    USHORT  type;
    USHORT  iFirst;
    USHORT  acolor[1];
} VIOPALSTATE;
typedef VIOPALSTATE * PVIOPALSTATE;

typedef struct _VIOOVERSCAN {
    USHORT  cb;
    USHORT  type;
    USHORT  color;
} VIOOVERSCAN;
typedef VIOOVERSCAN * PVIOOVERSCAN;

typedef struct _VIOINTENSITY {
    USHORT  cb;
    USHORT  type;
    USHORT  fs;
} VIOINTENSITY;
typedef VIOINTENSITY * PVIOINTENSITY;


typedef struct _VIOCOLORREG {
    USHORT  cb;
    USHORT  type;
    USHORT  firstcolorreg;
    USHORT  numcolorregs;
    PCH     colorregaddr;
} VIOCOLORREG;
typedef VIOCOLORREG * PVIOCOLORREG;

typedef struct _VIOSETULINELOC {
    USHORT  cb;
    USHORT  type;
    USHORT  scanline;
} VIOSETULINELOC;
typedef VIOSETULINELOC * PVIOSETULINELOC;

typedef struct _VIOSETTARGET {
    USHORT  cb;
    USHORT  type;
    USHORT  defaultalgorithm;
} VIOSETTARGET;
typedef VIOSETTARGET * PVIOSETTARGET;
#endif

/*
 * Vio function prototypes
 */
APIRET APIENTRY  VioAssociate(ULONG hdc, HVIO hvps);

APIRET APIENTRY  VioCharWidthUni(USHORT uch);

APIRET APIENTRY  VioCheckCharType (PULONG pType, ULONG ulRow,
                                   ULONG ulColumn, HVIO hvio);

// @todo
//APIRET APIENTRY  VioCreateLogFont(PFATTRS pfat, ULONG lcid, PSTR8 pstr8Name,
//                                 HVIO hvps);

APIRET APIENTRY  VioCreatePS(PHVIO phvps, ULONG Rows, ULONG Columns,
                            ULONG Format, ULONG AttrBytes, HVIO hvps);

APIRET APIENTRY  VioDeleteSetId(ULONG lcid, HVIO hvps);

APIRET APIENTRY  VioDestroyPS(HVIO hvps);

APIRET APIENTRY  VioEndPopUp (HVIO hvio);

APIRET APIENTRY  VioGetAnsi (PULONG pfAnsi, HVIO hvio);

APIRET APIENTRY  VioGetBuf  (PULONG pLVB, PULONG pcbLVB, HVIO hvio);

APIRET APIENTRY  VioGetConfig (ULONG ulConfigId, PVIOCONFIGINFO pvioin,
                                   HVIO hvio);

APIRET APIENTRY  VioGetCp (ULONG ulReserved, PUSHORT pIdCodePage, HVIO hvio);

APIRET APIENTRY  VioGetCurPos (PULONG pusRow, PULONG pusColumn, HVIO hvio);

APIRET APIENTRY  VioGetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio);

APIRET APIENTRY  VioGetDeviceCellSize(PULONG Height, PULONG Width,
                                     HVIO hvps);

APIRET APIENTRY  VioGetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio);

APIRET APIENTRY  VioGetOrigin(PULONG Row, PULONG Column, HVIO hvps);

APIRET APIENTRY  VioGetState (PVOID pState, HVIO hvio);

APIRET APIENTRY  VioModeUndo (ULONG ulOwnerInd, ULONG ulKillInd,
                                  ULONG ulReserved);

APIRET APIENTRY  VioModeWait (ULONG ulReqType, PULONG pNotifyType,
                                  ULONG ulReserved);

APIRET APIENTRY  VioPopUp (PULONG pfWait, HVIO hvio);

APIRET APIENTRY  VioPrtSc (HVIO hvio);

APIRET APIENTRY  VioPrtScToggle (HVIO hvio);

// @todo
//APIRET APIENTRY  VioQueryFonts(PULONG Remfonts, PFONTMETRICS Metrics,
//                              ULONG MetricsLength, PULONG Fonts,
//                              PSZ Facename, ULONG Options, HVIO hvps);

// @todo
//APIRET APIENTRY  VioQueryFontsUni(PULONG Remfonts, PFONTMETRICS Metrics,
//                              ULONG MetricsLength, PULONG Fonts,
//                              USHORT * Facename, ULONG Options, HVIO hvps);

APIRET APIENTRY  VioQuerySetIds(PULONG lcids, PSTR8 Names,
                               PULONG Types, ULONG count, HVIO hvps);

APIRET APIENTRY  VioReadCellStr (PCH pchCellStr, PULONG pcb, ULONG ulRow,
                                 ULONG ulColumn, HVIO hvio);

APIRET APIENTRY  VioReadCharStr (PCH pchCellStr, PULONG pcb, ULONG ulRow,
                                 ULONG ulColumn, HVIO hvio);

APIRET APIENTRY  VioSaveRedrawUndo (ULONG ulOwnerInd, ULONG ulKillInd,
                                       ULONG ulReserved);

APIRET APIENTRY  VioSaveRedrawWait (ULONG ulRedrawInd, PULONG pNotifyType,
                                       ULONG ulReserved);

APIRET APIENTRY  VioScrLock (ULONG fWait, PUCHAR pfNotLocked, HVIO hvio);

APIRET APIENTRY  VioScrollDown (ULONG ulTopRow, ULONG ulLeftCol,
                              ULONG ulBotRow, ULONG ulRightCol,
                              ULONG cbLines,  PBYTE pCell, HVIO hvio);

APIRET APIENTRY  VioScrollLeft (ULONG ulTopRow, ULONG ulLeftCol,
                              ULONG ulBotRow, ULONG ulRightCol,
                              ULONG cbCol, PBYTE pCell, HVIO hvio);

APIRET APIENTRY  VioScrollRight (ULONG ulTopRow, ULONG ulLeftCol,
                              ULONG ulBotRow, ULONG ulRightCol,
                              ULONG cbCol, PBYTE pCell, HVIO hvio);

APIRET APIENTRY  VioScrollUp (ULONG ulTopRow, ULONG ulLeftCol,
                              ULONG ulBotRow, ULONG ulRightCol,
                              ULONG cbLines, PBYTE pCell, HVIO hvio);

APIRET APIENTRY  VioScrUnLock (HVIO hvio);

APIRET APIENTRY  VioSetAnsi (ULONG fAnsi, HVIO hvio);

APIRET APIENTRY  VioSetCp (ULONG ulReserved, USHORT idCodePage, HVIO hvio);

APIRET APIENTRY  VioSetCurPos (ULONG  ulRow,  ULONG  ulColumn,  HVIO hvio);

APIRET APIENTRY  VioSetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio);

APIRET APIENTRY  VioSetDeviceCellSize(ULONG Height, ULONG Width, HVIO hvps);

APIRET APIENTRY  VioSetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio);

APIRET APIENTRY  VioSetOrigin(ULONG Row, ULONG Column, HVIO hvps);

APIRET APIENTRY  VioSetState (PVOID pState, HVIO hvio);

APIRET APIENTRY  VioShowBuf (ULONG offLVB, ULONG cb, HVIO hvio);

APIRET APIENTRY  VioShowPS(ULONG Depth, ULONG Width, ULONG Cell, HVIO hvps);

APIRET APIENTRY  VioWrtCellStr (PCH pchCellStr, ULONG cb, ULONG ulRow,
                                ULONG ulColumn, HVIO hvio);

APIRET  APIENTRY VioWrtCharStr (PCH pchStr, ULONG cb, ULONG ulRow,
                                ULONG ulColumn, HVIO hvio);

APIRET APIENTRY  VioWrtCharStrAttr (PCH pch, ULONG cb, ULONG ulRow,
                                   ULONG ulColumn, PBYTE pAttr, HVIO hvio);

APIRET APIENTRY  VioWrtNAttr (PBYTE pAttr, ULONG cb, ULONG ulRow,
                              ULONG ulColumn, HVIO hvio);

APIRET APIENTRY  VioWrtNCell (PBYTE pCell, ULONG cb, ULONG ulRow,
                              ULONG ulColumn, HVIO hvio);

APIRET APIENTRY  VioWrtNChar (PCH pchChar, ULONG cb, ULONG ulRow,
                              ULONG ulColumn, HVIO hvio);

APIRET APIENTRY  VioWrtTTY   (PCH pch, ULONG cb, HVIO hvio);

APIRET APIENTRY  VioWrtTTYUni (USHORT * pch, ULONG cb, HVIO hvio);


/*
 * Mouse (Mou) functions
 *
 * Notes:
 *     The mouse handle is always required to be zero, as there is no longer
 *     an open an close.
 */

typedef unsigned long   HMOU;
typedef HMOU        *   PHMOU;

APIRET APIENTRY  MouFlushQue (HMOU hmou);

#define MHK_BUTTON1                0x0001
#define MHK_BUTTON2                0x0002
#define MHK_BUTTON3                0x0004

/*
 * Structure for MouGetPtrPos and MouSetPtrPos
 */
typedef struct _PTRLOC {
    ULONG  row;               /* was USHORT */
    ULONG  col;               /* was USHORT */
} PTRLOC;
typedef PTRLOC * PPTRLOC;

/*
 * structure for MouRemovePtr
 * (field lengths increased)
 */
typedef struct _NOPTRRECT {
    ULONG  row;
    ULONG  col;
    ULONG  cRow;
    ULONG  cCol;
} NOPTRRECT;
typedef NOPTRRECT * PNOPTRRECT;

#if 0
// same in 16bit
typedef struct _THRESHOLD {   /* threshold */
    USHORT Length;            /* Length Field            */
    USHORT Level1;            /* First movement level    */
    USHORT Lev1Mult;          /* First level multiplier  */
    USHORT Level2;            /* Second movement level   */
    USHORT lev2Mult;          /* Second level multiplier */
} THRESHOLD;
typedef THRESHOLD * PTHRESHOLD;


/*
 * Pointer shape function for MouGetPtrShape and MouSetPtrShape
 */
typedef struct _PTRSHAPE {
    USHORT cb;                /* Structure length */
    USHORT col;               /* Horizontal size */
    USHORT row;               /* Vertical size */
    USHORT colHot;            /* Horizontal hot spot */
    USHORT rowHot;            /* Vertical hot spot */
} PTRSHAPE;
typedef PTRSHAPE * PPTRSHAPE;
#endif

/*
 * Device status for MouGetDevStatus and MouSetDevStatus
 */
#define MOUSE_QUEUEBUSY        0x0001
#define MOUSE_BLOCKREAD        0x0002
#define MOUSE_FLUSH            0x0004
#define MOUSE_UNSUPPORTED_MODE 0x0008
#define MOUSE_DISABLED         0x0100
#define MOUSE_MICKEYS          0x0200

/*
 * Wait constants for MouReadEventQue
 */
//#define MOU_NOWAIT             0
//#define MOU_WAIT               1

/*
 * structure for MouReadEventQue()
 * (all fields lengthened)
 */
typedef  struct _MOUEVENTINFO {
    ULONG  fs;                 /* Mouse state (MOUSE_ ) */
    LONG   row;                /* Horizontal position */
    LONG   col;                /* Vertical position */
    ULONG  time;               /* Timestamp */
} MOUEVENTINFO;
typedef MOUEVENTINFO *PMOUEVENTINFO;

/*
 * structure for MouGetNumQueEl()
 * (all fields lengthened)
 */
typedef struct _MOUQUEINFO {
    ULONG  cEvents;
    ULONG  cmaxEvents;
} MOUQUEINFO;
typedef MOUQUEINFO *PMOUQUEINFO;

/*
 * Event mask constants for MouGetEventMask and MouSetEventMask
 */
#define MOUSE_MOTION                 0x0001
#define MOUSE_MOTION_WITH_BN1_DOWN   0x0002
#define MOUSE_BN1_DOWN               0x0004
#define MOUSE_MOTION_WITH_BN2_DOWN   0x0008
#define MOUSE_BN2_DOWN               0x0010
#define MOUSE_MOTION_WITH_BN3_DOWN   0x0020
#define MOUSE_BN3_DOWN               0x0040

/*
 * Structure for MouGetScaleFact and MouSetScaleFact
 */
typedef  struct _SCALEFACT {
    ULONG  rowScale;
    ULONG  colScale;
} SCALEFACT;
typedef SCALEFACT * PSCALEFACT;

/*
 * Status for MouSetDevStatus
 */
#define MOU_NODRAW                 0x0001
#define MOU_DRAW                   0x0000
#define MOU_MICKEYS                0x0002
#define MOU_PELS                   0x0000

/*
 * Mou function prototypes
 */

APIRET APIENTRY  MouDrawPtr (HMOU hmou);

APIRET APIENTRY  MouFlushQue (HMOU hmou);

APIRET APIENTRY  MouGetDevStatus (PULONG  DevStat, HMOU hmou);

APIRET APIENTRY  MouGetEventMask (PULONG  EventMask, HMOU hmou);

APIRET APIENTRY  MouGetNumButtons (PULONG  Buttons, HMOU hmou);

APIRET APIENTRY  MouGetNumMickeys (PULONG  Mickeys, HMOU hmou);

APIRET APIENTRY  MouGetNumQueEl (PMOUQUEINFO QElements, HMOU hmou);

APIRET APIENTRY  MouGetPtrPos (PPTRLOC PtrLoc, HMOU hmou);

APIRET APIENTRY  MouGetPtrShape (PVOID PtrBuf, PPTRSHAPE PtrInfo, HMOU hmou);

APIRET APIENTRY  MouGetScaleFact (PSCALEFACT MouScale, HMOU hmou);

APIRET APIENTRY  MouReadEventQue (PMOUEVENTINFO EventMask, PULONG Wait,
                                  HMOU hmou);

APIRET APIENTRY  MouRemovePtr (PNOPTRRECT MouseRect, HMOU hmou);

APIRET APIENTRY  MouSetDevStatus (PULONG  DevStat, HMOU hmou);

APIRET APIENTRY  MouSetEventMask (PULONG  EventMask, HMOU hmou);

APIRET APIENTRY  MouSetPtrPos (PPTRLOC PtrLoc, HMOU hmou);

APIRET APIENTRY  MouSetPtrShape (PBYTE PtrBuf, PPTRSHAPE PtrInfo, HMOU hmou);

APIRET APIENTRY  MouSetScaleFact (PSCALEFACT Scale, HMOU hmou);

