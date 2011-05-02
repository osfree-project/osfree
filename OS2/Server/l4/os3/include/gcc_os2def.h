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
#ifndef __WIN32__
#define MAKELONG(l, h)   ((LONG)MAKEULONG(l, h))
#endif
#define MAKEUSHORT(l, h) (((USHORT)(l)) | ((USHORT)(h)) << 8)
#define MAKESHORT(l, h)  ((SHORT)MAKEUSHORT(l, h))

#ifndef __WIN32__
#define LOBYTE(w)   LOUCHAR(w)
#define HIBYTE(w)   HIUCHAR(w)
#endif
#define LOUCHAR(w)  ((UCHAR)(w))
#define HIUCHAR(w)  ((UCHAR)(((USHORT)(w) >> 8) & 0xff))
#define LOUSHORT(l) ((USHORT)((ULONG)l))
#define HIUSHORT(l) ((USHORT)(((ULONG)(l) >> 16) & 0xffff))

//#define NULLHANDLE  ((LHANDLE)0)

#define MAKEERRORID(sev, error) (ERRORID)(MAKEULONG((error), (sev)))
#define ERRORIDERROR(errid)     (LOUSHORT(errid))
#define ERRORIDSEV(errid)       (HIUSHORT(errid))

#define SEVERITY_NOERROR       0x0000
#define SEVERITY_WARNING       0x0004
#ifndef __WIN32__
#define SEVERITY_ERROR         0x0008
#endif
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

#define CDECL __attribute__((__cdecl__))

#ifdef __OS2__
#define EXPENTRY  _System
#define APIENTRY  _System
#else
#ifdef L4API_l4v2
 #define APIENTRY
#else
#endif
#endif

#ifdef L4API_l4v2
#define ERROR_INVALID_PARAMETER 87
#define NO_ERROR                0
#define ERROR_ENVVAR_NOT_FOUND	203
#define PAG_READ		0x0001
#define PAG_WRITE		0x0002
#define PAG_EXECUTE		0x0004
#define PAG_GUARD		0x0008
#define PAG_COMMIT		0x0010
#define OBJ_TILE                0x0040
#endif

#define CHAR     char
#define SHORT    short
#define LONG     long
#define INT      int
#define VOID     void
#define LONGLONG long long

typedef unsigned long   APIRET;
typedef unsigned short  APIRET16;

typedef unsigned char   UCHAR, *PUCHAR;
#ifndef BYTE_DEFINED
#define BYTE_DEFINED
typedef char            BYTE, *PBYTE, *NPBYTE;
#endif
typedef unsigned short  USHORT, *PUSHORT;
typedef unsigned int    UINT, *PUINT;
typedef unsigned long   ULONG, *PULONG;

// Från os2medef.h
//#ifndef __WORDBYTEDWORD__
  #define __WORDBYTEDWORD__
  #ifndef WORD_DEFINED
    #define WORD_DEFINED
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

#ifndef __WIN32__
typedef unsigned long BOOL, *PBOOL;
#endif
typedef unsigned LONG BOOL32, *PBOOL32;

typedef CHAR     *PCHAR;
typedef SHORT    *PSHORT;
typedef INT      *PINT;
typedef LONG     *PLONG;
typedef LONGLONG *PLONGLONG;
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

#ifndef __WIN32__
typedef LHANDLE HFILE, *PHFILE;
typedef LHANDLE HMODULE, *PHMODULE;
#endif
//typedef LHANDLE PID, *PPID;
//typedef LHANDLE TID, *PTID;
typedef ULONG   HEV,  *PHEV;
typedef ULONG   HMTX, *PHMTX;
typedef ULONG   HMUX, *PHMUX;
typedef VOID    *HSEM;
typedef HSEM    *PHSEM;
typedef USHORT  SGID;

typedef ULONG   ERRORID, *PERRORID;

#ifndef CCHMAXPATH
#define CCHMAXPATH          260
#define CCHMAXPATHCOMP      256
#endif

typedef struct _COUNTRYCODE {
  ULONG country;
  ULONG codepage;
} COUNTRYCODE, *PCOUNTRYCODE;

/* extended attribute structures */

typedef struct _GEA {
  BYTE   cbName;
  CHAR   szName[1];
} GEA, *PGEA;

typedef struct _GEALIST {
  ULONG  cbList;
  GEA    list[1];
} GEALIST, *PGEALIST;

typedef struct _FEA {
  BYTE   fEA;
  BYTE   cbName;
  USHORT cbValue;
} FEA, *PFEA;

#define FEA_NEEDEA 0x80

typedef struct _FEALIST {
  ULONG  cbList;
  FEA    list[1];
} FEALIST, *PFEALIST;

#pragma pack(1)

typedef struct _FEA2 {
  ULONG   oNextEntryOffset;
  BYTE    fEA;
  BYTE    cbName;
  USHORT  cbValue;
  CHAR    szName[1];
} FEA2, *PFEA2;

typedef struct _FEA2LIST {
  ULONG   cbList;
  FEA2    list[1];
} FEA2LIST, *PFEA2LIST;

typedef struct _GEA2 {
  ULONG   oNextEntryOffset;
  BYTE    cbName;
  CHAR    szName;
} GEA2, *PGEA2;

typedef struct _GEA2LIST {
  ULONG   cbList;
  GEA2    list[1];
} GEA2LIST, *PGEA2LIST;

typedef struct _EAOP2 {
  PGEA2LIST  fpGEA2List;
  PFEA2LIST  fpFEA2List;
  ULONG      oError;
} EAOP2, *PEAOP2;

#pragma pack()

#if 0
#ifndef __OS2__

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
#endif

#endif

