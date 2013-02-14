#ifndef __COPIED_DECL_H__

/* Datatype declarations from Open Watcom without functions declarations. */
/*#define INCL_KBD
hkbd
pkbdinfo
pkbdhwid
pkbdtrans
KBDKEYINFO
*/

#ifdef DO_VIO

typedef SHANDLE  HVIO;
typedef HVIO     *PHVIO;
/* typedef SHANDLE  HDC;*/
typedef SHANDLE  HVPS;
typedef HVPS  *PHVPS;

/*typedef char *PCH; */
/* PCCH ?? */
typedef char *PCCH; 

typedef _Packed struct _VIOCONFIGINFO {
    USHORT  cb;
    USHORT  adapter;
    USHORT  display;
    ULONG   cbMemory;
    USHORT  Configuration;
    USHORT  VDHVersion;
    USHORT  Flags;
    ULONG   HWBufferSize;
    ULONG   FullSaveSize;
    ULONG   PartSaveSize;
    USHORT  EMAdaptersOFF;
    USHORT  EMDisplaysOFF;
} VIOCONFIGINFO, *PVIOCONFIGINFO;

typedef struct _VIOCURSORINFO {
    USHORT   yStart;
    USHORT   cEnd;
    USHORT   cx;
    USHORT   attr;
} VIOCURSORINFO, *PVIOCURSORINFO;

typedef _Packed struct _VIOFONTINFO {
    USHORT  cb;
    USHORT  type;
    USHORT  cxCell;
    USHORT  cyCell;
    PVOID16 pbData;
    USHORT  cbData;
} VIOFONTINFO, *PVIOFONTINFO;

typedef _Packed struct _VIOMODEINFO {
    USHORT cb;
    UCHAR  fbType;
    UCHAR  color;
    USHORT col;
    USHORT row;
    USHORT hres;
    USHORT vres;
    UCHAR  fmt_ID;
    UCHAR  attrib;
    ULONG  buf_addr;
    ULONG  buf_length;
    ULONG  full_length;
    ULONG  partial_length;
    PCH    ext_data_addr;
} VIOMODEINFO, *PVIOMODEINFO;

typedef struct _VIOPHYSBUF {
    PBYTE    pBuf;
    ULONG    cb;
    SEL      asel[1];
} VIOPHYSBUF, *PVIOPHYSBUF;

#endif

#ifdef DO_KBD


typedef SHANDLE HKBD;
typedef HKBD    *PHKBD;

typedef struct _STRINGINBUF {
    USHORT cb;
    USHORT cchIn;
} STRINGINBUF, *PSTRINGINBUF;

#pragma pack(2)

typedef struct _KBDKEYINFO {
    UCHAR  chChar;
    UCHAR  chScan;
    UCHAR  fbStatus;
    UCHAR  bNlsShift;
    USHORT fsState;
    ULONG  time;
}KBDKEYINFO, *PKBDKEYINFO;

typedef struct _KBDTRANS {
    UCHAR  chChar;
    UCHAR  chScan;
    UCHAR  fbStatus;
    UCHAR  bNlsShift;
    USHORT fsState;
    ULONG  time;
    USHORT fsDD;
    USHORT fsXlate;
    USHORT fsShift;
    USHORT sZero;
} KBDTRANS, *PKBDTRANS;

#pragma pack()

typedef struct _KBDHWID {
    USHORT length;
    USHORT kbd_id;
    USHORT reserved1;
    USHORT reserved2;
}KBDHWID, *PKBDHWID;

typedef struct _KBDINFO {
    USHORT cb;
    USHORT fsMask;
    USHORT chTurnAround;
    USHORT fsInterim;
    USHORT fsState;
} KBDINFO, *PKBDINFO;


#endif

#ifdef DO_MOU

typedef SHANDLE  HMOU;
typedef HMOU     *PHMOU;

typedef struct _MOUQUEINFO {
    USHORT cEvents;
    USHORT cmaxEvents;
} MOUQUEINFO, *PMOUQUEINFO;


typedef struct _PTRLOC {
    USHORT row;
    USHORT col;
} PTRLOC, *PPTRLOC;

typedef struct _PTRSHAPE {
    USHORT cb;
    USHORT col;
    USHORT row;
    USHORT colHot;
    USHORT rowHot;
} PTRSHAPE, *PPTRSHAPE;

typedef struct _SCALEFACT {
    USHORT rowScale;
    USHORT colScale;
} SCALEFACT, *PSCALEFACT;

typedef _Packed struct _MOUEVENTINFO {
    USHORT fs;
    ULONG  time;
    SHORT  row;
    SHORT  col;
} MOUEVENTINFO, *PMOUEVENTINFO;

typedef struct _NOPTRRECT {
    USHORT row;
    USHORT col;
    USHORT cRow;
    USHORT cCol;
} NOPTRRECT, *PNOPTRRECT;


/* Unknown declarations, put ints in them for now? */
typedef int        THRESHOLD;
typedef THRESHOLD *PTHRESHOLD;


#endif


#ifdef DO_DOSMONITORS

typedef SHANDLE  HMONITOR;
typedef HMONITOR *PHMONITOR;

#endif

#endif
