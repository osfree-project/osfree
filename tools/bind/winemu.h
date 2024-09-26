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
#define HANDLE WORD
#endif

#ifndef HGLOBAL
#define HGLOBAL HANDLE
#endif

#define WINAPI far pascal
#define HFILE WORD
#define UINT unsigned int
#define LONG long
#define LPVOID void far *
#define LPCSTR const char far *
#define LPSTR char far *

#define GMEM_FIXED          0x0000
#define GMEM_ZEROINIT       0x0040

#define OF_READ 0x0000
#define SEEK_SET 0

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
char far *  GlobalLock(HGLOBAL h);
HFILE _lopen(LPCSTR lpPathName, int iReadWrite);
UINT _lread(HFILE  hFile, LPVOID lpBuffer, UINT uBytes);
HFILE _lclose(HFILE hFile);
LONG _llseek( HFILE hFile, LONG lOffset, int nOrigin );
LPSTR WINAPI lstrcpy( LPSTR s, LPCSTR t );
int WINAPI lstrlen( LPCSTR s );
//void WINAPI         hmemcpy( void __huge *, const void __huge *, long );
