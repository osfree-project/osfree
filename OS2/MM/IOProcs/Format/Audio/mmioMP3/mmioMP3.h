#ifndef MMIOMP3_H
#define MMIOMP3_H

#define INCL_OS2MM
#define INCL_MMIOOS2
#include <os2.h>
#include <os2me.h>

#define FOURCC_MP3 mmioFOURCC('M', 'P', '3', ' ')
#define MP3_COOKIE 0x072D6337

typedef struct _MP3OPTIONS {
int cookie;
long bitrate;
} MP3OPTIONS;

typedef MP3OPTIONS *PMP3OPTIONS;

LONG APIENTRY IOProc_Entry(PVOID pmmioStr, USHORT usMsg, LONG lParam1,
                     LONG lParam2);

#endif
