#ifndef MMIOFLAC_H
#define MMIOFLAC_H

#define INCL_OS2MM
#define INCL_MMIOOS2
#include <os2.h>
#include <os2me.h>

#define FOURCC_FLAC mmioFOURCC('f', 'L', 'a', 'C')

LONG APIENTRY IOProc_Entry(PVOID pmmioStr, USHORT usMsg, LONG lParam1,
                     LONG lParam2);

#endif
