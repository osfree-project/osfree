#ifndef INCL_INCLUDE_H
#define INCL_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vorbis/vorbisfile.h"

#ifndef min
#define min(x,y) (x < y ? x : y)
#endif

typedef struct {
    HEV hevThreadTrigger; /* ThreadTrigger用 */
    HEV hevThreadA;
    TID tidThreadDecode;

    char szFileName[4096];

    HEV hevPlaySem;
    ULONG fStatus;
    ULONG songlength; /* 曲の長さ */
    ULONG BufferSize;
    HWND  hwnd;
    BOOL  fStop;      /* Flags stop playback of current file */
    BOOL  fTerminate; /* Flags termination of decode thread */
    int jumpto;     /* absolute positioning in milliseconds */
    int ffwd;       /* 1 = start ffwd, 0 = end ffwd */
    int rew;        /* 1 = start rew, 0 = end rew */

    int current_section;
    OggVorbis_File vf;
    char sample_buffer[576 * 2 * 2 * 2]; /* ｏｇｇ読み込みバッファ */
    long lRemain; /* ｏｇｇ読み込みバッファの残り */
    long lOffset; /* ｏｇｇ読み込みバッファの位置 */
    char sample_buffer1[64 * 1024];
//    int num_channels;
    FORMAT_INFO format;
    int (* _System output_play_samples)(void *a, FORMAT_INFO *format, char *buf, int len, int posmarker);
    void *a;
} PLUGINWORK;

extern ULONG CommandPLAY(  PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );
extern ULONG CommandSTOP(  PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );
extern ULONG CommandFFWD(  PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );
extern ULONG CommandREW(  PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );
extern ULONG CommandJUMPTO(  PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );
extern ULONG CommandSETUP( PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );
extern ULONG CommandEQ( PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );
extern ULONG CommandBUFFER( PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );
extern ULONG CommandSAVEDATA( PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams );

extern long Read( PLUGINWORK* pPluginWork, void *pvData, long lSize, long *lSize1 );
extern size_t read_func( void *ptr, size_t size, size_t nmemb, void *datasource );
extern int seek_func(  void *datasource, ogg_int64_t offset, int whence );
extern int close_func( void *datasource );
extern long tell_func( void *datasource );

extern void ThreadDecodeMain( void * );

#endif
