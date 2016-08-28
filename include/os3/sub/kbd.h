/*****************************************************************************
    kbd.h
    (C) 2004-2010 osFree project

    WARNING! Automaticaly generated file! Don't edit it manually!
*****************************************************************************/

#ifndef __KBD_H__
#define __KBD_H__

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

typedef USHORT HKBD;
typedef HKBD * PHKBD;
USHORT APIENTRY KbdRegister(const PSZ pszModName, const PSZ pszEntryPt, const ULONG FunMask);
#define KR_KBDCHARIN 0x00000001
#define KR_KBDPEEK 0x00000002
#define KR_KBDFLUSHBUFFER 0x00000004
#define KR_KBDGETSTATUS 0x00000008
#define KR_KBDSETSTATUS 0x00000010
#define KR_KBDSTRINGIN 0x00000020
#define KR_KBDOPEN 0x00000040
#define KR_KBDCLOSE 0x00000080
#define KR_KBDGETFOCUS 0x00000100
#define KR_KBDFREEFOCUS 0x00000200
#define KR_KBDGETCP 0x00000400
#define KR_KBDSETCP 0x00000800
#define KR_KBDXLATE 0x00001000
#define KR_KBDSETCUSTXT 0x00002000
#define IO_WAIT 0
#define IO_NOWAIT 1
USHORT APIENTRY KbdDeRegister(VOID);
typedef struct _KBDKEYINFO {
UCHAR chChar;
UCHAR chScan;
UCHAR fbStatus;
UCHAR bNlsShift;
USHORT fsState;
ULONG time;
} KBDKEYINFO;
typedef KBDKEYINFO * PKBDKEYINFO;
USHORT APIENTRY KbdCharIn(KBDKEYINFO * pkbci, const USHORT fWait, const HKBD hkbd);
USHORT APIENTRY KbdPeek(KBDKEYINFO * pkbci, const HKBD hkbd);
typedef struct _STRINGINBUF {
USHORT cb;
USHORT cchIn;
} STRINGINBUF;
typedef STRINGINBUF * PSTRINGINBUF;
USHORT APIENTRY KbdStringIn(CHAR * pch, STRINGINBUF * pchIn, const USHORT fsWait, const HKBD hkbd);
USHORT APIENTRY KbdFlushBuffer(const HKBD hkbd);
#define KEYBOARD_ECHO_ON 0x0001
#define KEYBOARD_ECHO_OFF 0x0002
#define KEYBOARD_BINARY_MODE 0x0004
#define KEYBOARD_ASCII_MODE 0x0008
#define KEYBOARD_MODIFY_STATE 0x0010
#define KEYBOARD_MODIFY_INTERIM 0x0020
#define KEYBOARD_MODIFY_TURNAROUND 0x0040
#define KEYBOARD_2B_TURNAROUND 0x0080
#define KEYBOARD_SHIFT_REPORT 0x0100
#define KBDSTF_RIGHTSHIFT 0x0001
#define KBDSTF_LEFTSHIFT 0x0002
#define KBDSTF_CONTROL 0x0004
#define KBDSTF_ALT 0x0008
#define KBDSTF_SCROLLLOCK_ON 0x0010
#define KBDSTF_NUMLOCK_ON 0x0020
#define KBDSTF_CAPSLOCK_ON 0x0040
#define KBDSTF_INSERT_ON 0x0080
#define KBDSTF_LEFTCONTROL 0x0100
#define KBDSTF_LEFTALT 0x0200
#define KBDSTF_RIGHTCONTROL 0x0400
#define KBDSTF_RIGHTALT 0x0800
#define KBDSTF_SCROLLLOCK 0x1000
#define KBDSTF_NUMLOCK 0x2000
#define KBDSTF_CAPSLOCK 0x4000
#define KBDSTF_SYSREQ 0x8000
typedef struct _KBDINFO {
USHORT cb;
USHORT fsMask;
USHORT chTurnAround;
USHORT fsInterim;
USHORT fsState;
} KBDINFO;
typedef KBDINFO * PKBDINFO;
USHORT APIENTRY KbdSetStatus(const PKBDINFO pkbdinfo, const HKBD hkbd);
USHORT APIENTRY KbdGetStatus(KBDINFO * pkbdinfo, const HKBD hdbd);
USHORT APIENTRY KbdSetCp(const USHORT usReserved, const USHORT pidCP, const HKBD hdbd);
USHORT APIENTRY KbdGetCp(const ULONG ulReserved, USHORT * pidCP, const HKBD hkbd);
USHORT APIENTRY KbdOpen(PHKBD * hkbd);
USHORT APIENTRY KbdClose(const HKBD hkbd);
USHORT APIENTRY KbdGetFocus(const USHORT fWait, const HKBD hkbd);
USHORT APIENTRY KbdFreeFocus(const HKBD hkbd);
USHORT APIENTRY KbdSynch(const USHORT fsWait);
USHORT APIENTRY KbdSetFgnd(VOID);
typedef struct _KBDHWID {
USHORT cb;
USHORT idKbd;
USHORT usReserved1;
USHORT usReserved2;
} KBDHWID;
typedef KBDHWID * PKBDHWID;
USHORT APIENTRY KbdGetHWID(PKBDHWID * kbdhwid, const HKBD hkbd);
USHORT APIENTRY KbdSetHWID(const PKBDHWID pkbdhwid, const HKBD hkbd);
#define KBDTRF_SHIFT_KEY_IN 0x01
#define KBDTRF_CONVERSION_REQUEST 0x20
#define KBDTRF_FINAL_CHAR_IN 0x40
#define KBDTRF_INTERIM_CHAR_IN 0x80
typedef struct _KBDTRANS {
UCHAR chChar;
UCHAR chScan;
UCHAR fbStatus;
UCHAR bNlsShift;
USHORT fsState;
ULONG time;
USHORT fsDD;
USHORT fsXlate;
USHORT fsShift;
USHORT sZero;
} KBDTRANS;
typedef KBDTRANS * PKBDTRANS;
USHORT APIENTRY KbdXlate(const PKBDTRANS pkbdtrans, const HKBD hkbd);
USHORT APIENTRY KbdSetCustXt(const PUSHORT usCodePage, const HKBD hkbd);

#ifdef __cplusplus
   }
#endif

#endif /* __KBD_H__ */
