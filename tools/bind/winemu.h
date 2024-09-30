#ifndef __DOS__
#include "stdio.h"
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef HANDLE
#ifndef __DOS__
#define HANDLE void *
#else
#define HANDLE WORD
#endif
#endif

#ifndef HGLOBAL
#define HGLOBAL HANDLE
#endif

#ifndef __DOS__
#define FAR
#define FP_SEG(__p) (void *)(__p)
#define FP_OFF(__p) 0
#undef MK_FP
#define MK_FP(__s,__o) (void *)((char *)(__s)+(WORD)(__o))
#else
#define FAR far
#endif

#define WINAPI FAR pascal

#ifndef __DOS__
#define HFILE FILE *
#else
#define HFILE WORD
#endif

#define UINT unsigned int
#define LONG long
#define LPVOID void FAR *
#define LPCSTR const char FAR *
#define LPSTR char FAR *

#define GMEM_FIXED          0x0000
#define GMEM_ZEROINIT       0x0040

#define OF_READ 0x0000
#define SEEK_SET 0
#define HFILE_ERROR (-1)
#pragma pack( push, 1 )

typedef struct
{
    WORD first; /* ordinal */
    WORD last;  /* ordinal */
    WORD next;  /* bundle */
} ET_BUNDLE;

typedef struct tagOFSTRUCT {
    BYTE    cBytes;
    BYTE    fFixedDisk;
    unsigned int	nErrCode;
    BYTE    reserved[4];
    char    szPathName[128];
} OFSTRUCT;

#pragma pack( pop )

#define Dos3Call __asm { int 21h }
HGLOBAL WINAPI GlobalAlloc(WORD flags, DWORD size);
char FAR *  WINAPI GlobalLock(HGLOBAL h);
HFILE WINAPI _lopen(LPCSTR lpPathName, int iReadWrite);
UINT WINAPI _lread(HFILE  hFile, LPVOID lpBuffer, UINT uBytes);
HFILE WINAPI _lclose(HFILE hFile);
LONG WINAPI _llseek( HFILE hFile, LONG lOffset, int nOrigin );
LPSTR WINAPI lstrcpy( LPSTR s, LPCSTR t );
int WINAPI lstrlen( LPCSTR s );
//void WINAPI         hmemcpy( void __huge *, const void __huge *, long );
