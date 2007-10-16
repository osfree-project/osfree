/*
    OS/2 typedefs and constants for 32-bit development.
*/


#ifndef __OS2DEF_H__
#define __OS2DEF_H__

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define FAR
#define NEAR

#define FIELDOFFSET(type, field)   ((SHORT)&(((type *)0)->field))

#define MAKEULONG(l, h)  ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define MAKELONG(l, h)   ((LONG)MAKEULONG(l, h))
#define MAKEUSHORT(l, h) (((USHORT)(l)) | ((USHORT)(h)) << 8)
#define MAKESHORT(l, h)  ((SHORT)MAKEUSHORT(l, h))

#define LOBYTE(w)   LOUCHAR(w)
#define HIBYTE(w)   HIUCHAR(w)
#define LOUCHAR(w)  ((UCHAR)(w))
#define HIUCHAR(w)  ((UCHAR)(((USHORT)(w) >> 8) & 0xff))
#define LOUSHORT(l) ((USHORT)((ULONG)l))
#define HIUSHORT(l) ((USHORT)(((ULONG)(l) >> 16) & 0xffff))

#define NULLHANDLE  ((LHANDLE)0)

#define MAKEERRORID(sev, error) (ERRORID)(MAKEULONG((error), (sev)))
#define ERRORIDERROR(errid)     (LOUSHORT(errid))
#define ERRORIDSEV(errid)       (HIUSHORT(errid))

#define SEVERITY_NOERROR       0x0000
#define SEVERITY_WARNING       0x0004
#define SEVERITY_ERROR         0x0008
#define SEVERITY_SEVERE        0x000C
#define SEVERITY_UNRECOVERABLE 0x0010

#define WINERR_BASE 0x1000
#define GPIERR_BASE 0x2000
#define DEVERR_BASE 0x3000
#define SPLERR_BASE 0x4000

#define ICON_FILE     1
#define ICON_RESOURCE 2
#define ICON_DATA     3
#define ICON_CLEAR    4

#define APIENTRY16 _Far16 _Pascal
#define PASCAL16   _Far16 _Pascal

#define EXPENTRY  _System
#define APIENTRY  _System

#define CHAR     char
#define SHORT    short
#define LONG     long
#define INT      int
#define VOID     void


typedef unsigned long   APIRET;
typedef unsigned short  APIRET16;

typedef unsigned char   UCHAR, *PUCHAR;
typedef char            BYTE, *PBYTE, *NPBYTE;
typedef unsigned short  USHORT, *PUSHORT;
typedef unsigned int    UINT, *PUINT;
typedef unsigned long   ULONG, *PULONG;

// Från os2medef.h
//#ifndef __WORDBYTEDWORD__
  #define __WORDBYTEDWORD__
  #ifndef WORD
    typedef USHORT WORD;
  #endif
  #ifndef DWORD
    typedef ULONG  DWORD;
  #endif
//#endif

typedef unsigned short  SHANDLE;
typedef unsigned long   LHANDLE;

typedef char *PSZ;
typedef char *PCH;
typedef const char *PCSZ;

typedef unsigned long BOOL, *PBOOL;
typedef unsigned LONG BOOL32, *PBOOL32;

typedef CHAR     *PCHAR;
typedef SHORT    *PSHORT;
typedef INT      *PINT;
typedef LONG     *PLONG;
/*
typedef CHAR     * _Seg16 PCHAR16;
typedef UCHAR    * _Seg16 PUCHAR16;
*/
typedef VOID     *PVOID;
typedef PVOID    *PPVOID;
/*  Problematiska för gcc.
typedef int (APIENTRY _PFN)();
typedef _PFN  *PFN;
*/
typedef USHORT  SEL, *PSEL;

typedef LHANDLE HFILE, *PHFILE;
typedef LHANDLE HMODULE, *PHMODULE;
typedef LHANDLE PID, *PPID;
typedef LHANDLE TID, *PTID;
typedef ULONG   HEV,  *PHEV;
typedef ULONG   HMTX, *PHMTX;
typedef ULONG   HMUX, *PHMUX;
typedef VOID    *HSEM;
typedef HSEM    *PHSEM;
typedef USHORT  SGID;

typedef ULONG   ERRORID, *PERRORID;

typedef struct _QWORD {
    ULONG   ulLo;
    ULONG   ulHi;
} QWORD, *PQWORD;

typedef struct _ICONINFO {
    ULONG   cb;
    ULONG   fFormat;
    PSZ     pszFileName;
    HMODULE hmod;
    ULONG   resid;
    ULONG   cbIconData;
    PVOID   pIconData;
} ICONINFO, *PICONINFO;

#endif

