#ifndef MMIOVORBIS_H
#define MMIOVORBIS_H

#define INCL_OS2MM
#define INCL_MMIOOS2
#include <os2.h>
#include <os2me.h>
#include <vorbis/vorbisenc.h>

#define FOURCC_Vorbis mmioFOURCC('O', 'G', 'G', 'S')
#define VORBIS_COOKIE 0x99E59D59

typedef struct _VORBISOPTIONS {
int cookie;
long nominal_bitrate;
long max_bitrate;
long min_bitrate;
} VORBISOPTIONS;

typedef VORBISOPTIONS *PVORBISOPTIONS;

typedef struct _OggStreamHeader {
long serialnumber;
ogg_int64_t raw_total;
ogg_int64_t pcm_total;
double time_total;
} OggStreamHeader;

LONG APIENTRY IOProc_Entry(PVOID pmmioStr, USHORT usMsg, LONG lParam1,
                     LONG lParam2);

#endif
