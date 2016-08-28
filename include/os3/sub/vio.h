/*****************************************************************************
    vio.h
    (C) 2004-2010 osFree project

    WARNING! Automaticaly generated file! Don't edit it manually!
*****************************************************************************/

#ifndef __VIO_H__
#define __VIO_H__

#ifdef __cplusplus
   extern "C" {
#endif

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef EXPENTRY
#define EXPENTRY  _System
#endif

#ifndef APIENTRY
#define APIENTRY  _System
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

typedef USHORT HVIO;
typedef HVIO * PHVIO;
#define FORMAT_CGA 0x0001
#define FORMAT_VGA 0x0001
#define FORMAT_4BYTE 0x0003
#define FORMAT_EXTEND 0x0003
#define FORMAT_UNICODE 0x0002
#define VQF_PUBLIC 0x0001
#define VQF_PRIVATE 0x0002
#define VQF_ALL 0x0003
typedef struct _VIOCURSORINFO {
USHORT yStart;
USHORT cEnd;
USHORT cx;
USHORT attr;
} VIOCURSORINFO;
typedef VIOCURSORINFO * PVIOCURSORINFO;
typedef struct _VIOMODEINFO {
USHORT cb;
UCHAR fbType;
UCHAR color;
USHORT col;
USHORT row;
USHORT hres;
USHORT vres;
UCHAR fmt_ID;
UCHAR attrib;
USHORT resvd;
ULONG buf_addr;
ULONG buf_length;
ULONG full_length;
ULONG partial_length;
PCH ext_data_addr;
} VIOMODEINFO;
typedef VIOMODEINFO * PVIOMODEINFO;
#define VGMT_OTHER 1
#define VGMT_COLOR 1
#define VGMT_GRAPHICS 2
#define VGMT_DISABLEBURST 4
#define VGMT_NOTBIOS 8
#define COLORS_2 1
#define COLORS_4 2
#define COLORS_16 4
#define COLORS_256 8
#define COLORS_64K 16
#define COLORS_16M 24
#define VCC_SBCSCHAR 0
#define VCC_DBCSFULLCHAR 1
#define VCC_DBCS1STHALF 2
#define VCC_DBCS2NDHALF 3
#define VSRWI_SAVEANDREDRAW 0
#define VSRWI_REDRAW 1
#define VSRWN_SAVE 0
#define VSRWN_REDRAW 1
#define UNDOI_GETOWNER 0
#define UNDOI_RELEASEOWNER 1
#define UNDOK_ERRORCODE 0
#define UNDOK_TERMINATE 1
#define LOCKIO_NOWAIT 0
#define LOCKIO_WAIT 1
#define LOCK_SUCCESS 0
#define LOCK_FAIL 1
#define ANSI_ON 1
#define ANSI_OFF 0
#define VMWR_POPUP 0
#define VMWN_POPUP 0
#define VP_NOWAIT 0
#define VP_WAIT 1
#define VP_OPAQUE 0
#define VP_TRANSPARENT 2
#define DISPLAY_MONOCHROME 0
#define DISPLAY_CGA 1
#define DISPLAY_EGA 2
#define DISPLAY_VGA 3
#define DISPLAY_8514A 7
#define DISPLAY_IAA 8
#define DISPLAY_XGA 9
#define DISPLAY_SVGA 10
#define DISPLAY_NONVGA 12
#define MONITOR_MONOCHROME 0
#define MONITOR_COLOR 1
#define MONITOR_ENHANCED 2
#define MONITOR_8503 3
#define MONITOR_851X_COLOR 4
#define MONITOR_8514 9
#define MONITOR_FLATPANEL 10
#define MONITOR_LCD640 10
#define MONITOR_8507_8604 11
#define MONITOR_8515 12
#define MONITOR_9515 15
#define MONITOR_9517 17
#define MONITOR_9518 18
#define MONITOR_OEM 19
#define MONITOR_LCD800 21
#define MONITOR_LCD1024 22
typedef struct _VIOCONFIGINFO {
ULONG cb;
ULONG adapter;
ULONG display;
ULONG cbMemory;
ULONG Configuration;
ULONG Version;
ULONG Flags;
ULONG HWBufferSize;
ULONG FullSaveSize;
ULONG PartSaveSize;
ULONG VideoID;
ULONG VideoData;
} VIOCONFIGINFO;
typedef VIOCONFIGINFO * PVIOCONFIGINFO;
#define VIO_CONFIG_CURRENT 0
#define VIO_CONFIG_PRIMARY 1
#define VIO_CONFIG_SECONDARY 2
typedef struct _VIOPALSTATE {
USHORT cb;
USHORT type;
USHORT iFirst;
USHORT acolor[1];
} VIOPALSTATE;
typedef VIOPALSTATE * PVIOPALSTATE;
typedef struct _VIOOVERSCAN {
USHORT cb;
USHORT type;
USHORT color;
} VIOOVERSCAN;
typedef VIOOVERSCAN * PVIOOVERSCAN;
typedef struct _VIOINTENSITY {
USHORT cb;
USHORT type;
USHORT fs;
} VIOINTENSITY;
typedef VIOINTENSITY * PVIOINTENSITY;
typedef struct _VIOCOLORREG {
USHORT cb;
USHORT type;
USHORT firstcolorreg;
USHORT numcolorregs;
PCH colorregaddr;
} VIOCOLORREG;
typedef VIOCOLORREG * PVIOCOLORREG;
typedef struct _VIOSETULINELOC {
USHORT cb;
USHORT type;
USHORT scanline;
} VIOSETULINELOC;
typedef VIOSETULINELOC * PVIOSETULINELOC;
typedef struct _VIOSETTARGET {
USHORT cb;
USHORT type;
USHORT defaultalgorithm;
} VIOSETTARGET;
typedef VIOSETTARGET * PVIOSETTARGET;
#define VR_VIOGETCURPOS 0x00000001
#define VR_VIOGETCURTYPE 0x00000002
#define VR_VIOGETMODE 0x00000004
#define VR_VIOGETBUF 0x00000008
#define VR_VIOGETPHYSBUF 0x00000010
#define VR_VIOSETCURPOS 0x00000020
#define VR_VIOSETCURTYPE 0x00000040
#define VR_VIOSETMODE 0x00000080
#define VR_VIOSHOWBUF 0x00000100
#define VR_VIOREADCHARSTR 0x00000200
#define VR_VIOREADCELLSTR 0x00000400
#define VR_VIOWRTNCHAR 0x00000800
#define VR_VIOWRTNATTR 0x00001000
#define VR_VIOWRTNCELL 0x00002000
#define VR_VIOWRTTTY 0x00004000
#define VR_VIOWRTCHARSTR 0x00008000
#define VR_VIOWRTCHARSTRATT 0x00010000
#define VR_VIOWRTCELLSTR 0x00020000
#define VR_VIOSCROLLUP 0x00040000
#define VR_VIOSCROLLDN 0x00080000
#define VR_VIOSCROLLLF 0x00100000
#define VR_VIOSCROLLRT 0x00200000
#define VR_VIOSETANSI 0x00400000
#define VR_VIOGETANSI 0x00800000
#define VR_VIOPRTSC 0x01000000
#define VR_VIOSCRLOCK 0x02000000
#define VR_VIOSCRUNLOCK 0x04000000
#define VR_VIOSAVREDRAWWAIT 0x08000000
#define VR_VIOSAVREDRAWUNDO 0x10000000
#define VR_VIOPOPUP 0x20000000
#define VR_VIOENDPOPUP 0x40000000
#define VR_VIOPRTSCTOGGLE 0x80000000
#define VR_VIOMODEWAIT 0x00000001
#define VR_VIOMODEUNDO 0x00000002
#define VR_VIOGETFONT 0x00000004
#define VR_VIOGETCONFIG 0x00000008
#define VR_VIOSETCP 0x00000010
#define VR_VIOGETCP 0x00000020
#define VR_VIOSETFONT 0x00000040
#define VR_VIOGETSTATE 0x00000080
#define VR_VIOSETSTATE 0x00000100
typedef struct _VIOPHYSBUF {
PBYTE pBuf;
ULONG cb;
SEL asel[1];
} VIOPHYSBUF;
typedef VIOPHYSBUF * PVIOPHYSBUF;
typedef struct _VIOFONTINFO {
USHORT cb;
USHORT type;
USHORT cxCell;
USHORT cyCell;
PVOID16 pbData;
USHORT cbData;
} VIOFONTINFO;
typedef VIOFONTINFO * PVIOFONTINFO;
#define VGFI_GETCURFONT 0
#define VGFI_GETROMFONT 1
USHORT APIENTRY VioDeRegister(VOID);
USHORT APIENTRY VioRegister(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2);
USHORT APIENTRY VioGlobalReg(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2, const USHORT usReturn);
USHORT APIENTRY VioGetAnsi(USHORT * Ansi, const HVIO Handle);
USHORT APIENTRY VioGetCp(const USHORT Reserved, USHORT * IdCodePage, const HVIO Handle);
USHORT APIENTRY VioGetCurPos(USHORT * Row, USHORT * Column, const HVIO Handle);
USHORT APIENTRY VioGetCurType(VIOCURSORINFO * CursorInfo, const HVIO Handle);
USHORT APIENTRY VioGetMode(VIOMODEINFO * ModeInfo, const HVIO Handle);
USHORT APIENTRY VioReadCellStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY VioReadCharStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY VioScrollDown(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY VioScrollDn(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY VioScrollLeft(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY VioScrollLf(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY VioScrollRight(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY VioScrollRt(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY VioScrollUp(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY VioSetAnsi(const USHORT Ansi, const HVIO Handle);
USHORT APIENTRY VioSetCp(const USHORT Reserved, const USHORT IdCodePage, const HVIO Handle);
USHORT APIENTRY VioSetCurPos(const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY VioSetCurType(const PVIOCURSORINFO CursorInfo, const HVIO Handle);
USHORT APIENTRY VioSetMode(const PVIOMODEINFO ModeInfo, const HVIO hvio);
USHORT APIENTRY VioWrtCellStr(const PCHAR CellStr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY VioWrtCharStr(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY VioWrtCharStrAttr(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const PBYTE pAttr, const HVIO Handle);
USHORT APIENTRY VioWrtCharStrAtt(const PCH pch, const USHORT cb, const USHORT usRow, const USHORT usColumn, const PBYTE pAttr, const HVIO hvio);
USHORT APIENTRY VioWrtNAttr(const PBYTE Attr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY VioWrtNCell(const PBYTE Cell, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY VioWrtNChar(const PCHAR Char, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY VioWrtTTY(const PCHAR Str, const USHORT Count, const HVIO Handle);
USHORT APIENTRY VioGetState(const PVOID pState, const HVIO Handle);
USHORT APIENTRY VioSetState(const PVOID pState, const HVIO Handle);
USHORT APIENTRY VioGetConfig(const USHORT ConfigId, VIOCONFIGINFO * vioin, const HVIO hvio);
USHORT APIENTRY VioPopUp(USHORT * Options, const HVIO VioHandle);
USHORT APIENTRY VioEndPopUp(const HVIO VioHandle);
USHORT APIENTRY VioGetPhysBuf(VIOPHYSBUF * pvioPhysBuf, const USHORT usReserved);
USHORT APIENTRY VioPrtSc(const HVIO hvio);
USHORT APIENTRY VioPrtScToggle(const HVIO hvio);
USHORT APIENTRY VioShowBuf(const USHORT offLVB, const USHORT cb, const HVIO hvio);
USHORT APIENTRY VioScrLock(const USHORT fWait, UCHAR * pfNotLocked, const HVIO hvio);
USHORT APIENTRY VioScrUnLock(const HVIO hvio);
USHORT APIENTRY VioSavRedrawWait(const USHORT usRedrawInd, USHORT * pNotifyType, const USHORT usReserved);
USHORT APIENTRY VioSavRedrawUndo(const USHORT usOwnerInd, const USHORT usKillInd, const USHORT usReserved);
USHORT APIENTRY VioModeWait(const USHORT usReqType, USHORT * pNotifyType, const USHORT usReserved);
USHORT APIENTRY VioModeUndo(const USHORT usOwnerInd, const USHORT usKillInd, const USHORT usReserved);
USHORT APIENTRY VioGetFont(VIOFONTINFO * pviofi, const HVIO hvio);
USHORT APIENTRY VioSetFont(VIOFONTINFO * pviofi, const HVIO hvio);
USHORT APIENTRY VioGetBuf(ULONG * pLVB, USHORT * pcbLVB, const HVIO hvio);
USHORT APIENTRY VioCheckCharType(USHORT * pType, const USHORT usRow, const USHORT usColumn, const HVIO hvio);

#ifdef __cplusplus
   }
#endif

#endif /* __VIO_H__ */
