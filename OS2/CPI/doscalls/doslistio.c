#include <os2.h>

#include <stdio.h>

APIRET  APIENTRY        DosListIO(ULONG ulCmdMode,
                                  ULONG ulNumentries,
                                  PLISTIO pListIO)
{
  PLISTIOL pListIOL;


  return 0;

//  return DosListIO(ulCmdMode,
//                   ulNumentries,
//                   pListIOL);
}

#if 0

typedef struct _LISTIO_CB {
    HFILE hFile;
    ULONG CmdFlag;
    LONG  Offset;
    PVOID pBuffer;
    ULONG NumBytes;
    ULONG Actual;
    ULONG RetCode;
    ULONG Reserved;
    ULONG Reserved2[3];
    ULONG Reserved3[2];
} LISTIO, *PLISTIO;

typedef struct _LISTIO_CBL {
    HFILE    hFile;
    ULONG    CmdFlag;
    LONGLONG Offset;
    PVOID    pBuffer;
    ULONG    NumBytes;
    ULONG    Actual;
    ULONG    RetCode;
    ULONG    Reserved;
    ULONG    Reserved2[3];
    ULONG    Reserved3[2];
} LISTIOL, *PLISTIOL;

#endif
