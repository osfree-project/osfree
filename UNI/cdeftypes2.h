#ifndef __CDEFTYPES2_H__
#define __CDEFTYPES2_H__
#define CDEFTYPES2_INCLUDED

/* types tricks */

typedef CHAR     * _Seg16 PCHAR16;
typedef UCHAR    * _Seg16 PUCHAR16;

typedef struct _ICONINFO {
    ULONG   cb;
    ULONG   fFormat;
    PSZ     pszFileName;
    HMODULE hmod;
    ULONG   resid;
    ULONG   cbIconData;
    PVOID   pIconData;
} ICONINFO, *PICONINFO;

typedef struct _ACCEL {
    USHORT fs;
    USHORT key;
    USHORT cmd;
} ACCEL, *PACCEL;

typedef struct _QWORD {
    ULONG   ulLo;
    ULONG   ulHi;
} QWORD, *PQWORD;

typedef VOID     * _Seg16 PVOID16;


typedef int (APIENTRY _PFN)();
typedef _PFN  *PFN;


#ifndef LONGLONG_INCLUDED
#define LONGLONG_INCLUDED 1

#ifndef INCL_LONGLONG
typedef struct _LONGLONG {
    ULONG ulLo;
    LONG  ulHi;
} LONGLONG, *PLONGLONG;

typedef struct _ULONGLONG {
    ULONG ulLo;
    ULONG ulHi;
} ULONGLONG, *PULONGLONG;
#else
typedef long long LONGLONG, *PLONGLONG;
typedef unsigned long long ULONGLONG, *PULONGLONG;
#endif

#define MAKEULONG(l, h)  ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define MAKELONG(l, h)   ((LONG)MAKEULONG(l, h))
#define MAKEUSHORT(l, h) (((USHORT)(l)) | ((USHORT)(h)) << 8)
#define MAKESHORT(l, h)  ((SHORT)MAKEUSHORT(l, h))

#endif

#endif
