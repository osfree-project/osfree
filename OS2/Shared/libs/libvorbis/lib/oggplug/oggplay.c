/*
   1    decoder_init   exported, shared data
   2    decoder_uninit exported, shared data
   3    decoder_command    exported, shared data
   4    decoder_status exported, shared data
   5    decoder_length exported, shared data
   6    decoder_fileinfo   exported, shared data
   7    decoder_support    exported, shared data
   8    decoder_trackinfo  exported, shared data
   9    decoder_cdinfo exported, shared data
  10    plugin_query   exported, shared data
*/
#define INCL_DOS
#include <os2.h>
#include <math.h>
#include <string.h>

#include "format.h"
#include "decoder_plug.h"
#include "plugin.h"

#include "include.h"


int _System decoder_init( void **ppvPluginWork )
{
    PLUGINWORK *pPluginWork;
    pPluginWork = (void *)malloc( sizeof( PLUGINWORK ) );
    if(pPluginWork == 0) {
        *ppvPluginWork = 0;
        return -1;
    }
    memset( pPluginWork, 0, sizeof( PLUGINWORK ) );
    DosCreateEventSem( NULL, &pPluginWork->hevThreadTrigger, 0UL, FALSE );
    DosCreateEventSem( NULL, &pPluginWork->hevThreadA,       0UL, FALSE );

    pPluginWork->tidThreadDecode = _beginthread( ThreadDecodeMain, NULL, 1024 * 1024, pPluginWork );
    if(pPluginWork->tidThreadDecode == -1) {
        /* can't create thread */
        DosCloseEventSem( pPluginWork->hevThreadTrigger ); pPluginWork->hevThreadTrigger = NULLHANDLE;
        DosCloseEventSem( pPluginWork->hevThreadA       ); pPluginWork->hevThreadA       = NULLHANDLE;
        free( pPluginWork ); pPluginWork = NULL;
        *ppvPluginWork = 0;
        return -1;
    }

    *ppvPluginWork = (void *)pPluginWork;
    pPluginWork->songlength = 0xffffffffUL;
    pPluginWork->fStop = FALSE;
    pPluginWork->fTerminate = FALSE;
//    DosSetPriority( PRTYS_THREAD, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, pPluginWork->tidThreadDecode );

    return 0;
}

BOOL _System decoder_uninit( void *pvPluginWork )
{
    PLUGINWORK *pPluginWork = (PLUGINWORK *)pvPluginWork;
    ULONG ulTmp;

    if (pvPluginWork) {
        pPluginWork->fTerminate = TRUE;

        if (pPluginWork->hevThreadTrigger) {
            DosResetEventSem( pPluginWork->hevThreadA, &ulTmp );
            pPluginWork->fTerminate = TRUE;
            DosPostEventSem( pPluginWork->hevThreadTrigger );
            DosWaitEventSem( pPluginWork->hevThreadA, 20000 );
            DosCloseEventSem( pPluginWork->hevThreadTrigger );
        }

        if (pPluginWork->hevThreadA) {
            DosCloseEventSem( pPluginWork->hevThreadA );
        }
        free( pvPluginWork );
    }
    return 0;
}

static void ThreadDecodeSub( PLUGINWORK *pPluginWork );
void ThreadDecodeMain( void *pv )
{
    while(!((PLUGINWORK *)pv)->fTerminate) {
        ThreadDecodeSub( (PLUGINWORK *)pv );
    }

    DosPostEventSem( ((PLUGINWORK *)pv)->hevThreadA );
}

static void ThreadDecodeSub( PLUGINWORK *pPluginWork )
{
    static ov_callbacks callbacks = {read_func, seek_func, close_func, tell_func};
    ULONG ulTmp;
    HFILE stream;
    ULONG ulAction;

    pPluginWork->jumpto     = -1;
    pPluginWork->fStop      = FALSE;
    pPluginWork->fTerminate = FALSE;
    pPluginWork->ffwd       = FALSE;
    pPluginWork->rew        = FALSE;
    pPluginWork->lRemain    = 0;
    pPluginWork->lOffset    = 0;

    DosWaitEventSem( pPluginWork->hevThreadTrigger, SEM_INDEFINITE_WAIT );
    DosResetEventSem( pPluginWork->hevThreadTrigger, &ulTmp );

    if(pPluginWork->fTerminate) {
        return;
    }

    pPluginWork->fStatus = DECODER_STARTING;

    DosOpen( pPluginWork->szFileName, &stream, &ulAction, 0, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0 );
    {
        double posmarker = 0.0;
        vorbis_info *vi = NULL;
        int BytesPerSec;

        // The ov_open() function performs full stream detection and machine
        // initialization.  If it returns 0, the stream *is* Vorbis and we're
        // fully ready to decode.
        if (ov_open_callbacks( (void *)stream, &pPluginWork->vf, NULL, 0, callbacks) < 0) {
            close_func( (void *)stream );
            return;
        }
        vi = ov_info( &pPluginWork->vf, -1);
        pPluginWork->format.size = sizeof(FORMAT_INFO);
        pPluginWork->format.samplerate = vi->rate;
        pPluginWork->format.channels = vi->channels;
        pPluginWork->format.bits = 16;
        pPluginWork->format.format = WAVE_FORMAT_PCM;
        BytesPerSec = vi->rate * vi->channels * 2;

        /* 曲の時間(ms) */
        pPluginWork->songlength = (int)(ov_time_total(&pPluginWork->vf, -1) * 1000.0);

        DosResetEventSem( pPluginWork->hevPlaySem, &ulTmp );
        DosPostEventSem( pPluginWork->hevThreadA );

        pPluginWork->fStatus = DECODER_PLAYING;

        //seekneeded = -1;
        while(!pPluginWork->fStop && !pPluginWork->fTerminate) {
            long read_size, read_size1;
            if(pPluginWork->jumpto != -1) {
                ov_time_seek( &pPluginWork->vf, pPluginWork->jumpto / 1000 );
                posmarker = (double)(pPluginWork->jumpto / 1000) * BytesPerSec;
                pPluginWork->lOffset = 0;
                pPluginWork->lRemain = 0;
                pPluginWork->jumpto = -1;
                WinPostMsg( pPluginWork->hwnd, WM_SEEKSTOP, 0, 0 );
            }
#if 0
            if(pPluginWork->rew) {
                posmarker -= 1 * BytesPerSec; /* 2s */
                ov_time_seek( &pPluginWork->vf, (int)(posmarker / BytesPerSec * 1000.0) );
                pPluginWork->lOffset = 0;
                pPluginWork->lRemain = 0;
//                pPluginWork->jumpto = -1;
//                WinPostMsg( pPluginWork->hwnd, WM_SEEKSTOP, 0, 0 );
            }
#endif
            read_size = Read( pPluginWork, pPluginWork->sample_buffer1, pPluginWork->BufferSize, &read_size1 );
            if(read_size < pPluginWork->BufferSize) {
                if(read_size == 0) {
                    break;
                }
                read_size = pPluginWork->BufferSize;
                memset( &pPluginWork->sample_buffer1[read_size], 0, pPluginWork->BufferSize -  read_size );
            }
            pPluginWork->output_play_samples( pPluginWork->a, &pPluginWork->format, pPluginWork->sample_buffer1, read_size, (int)(posmarker / BytesPerSec * 1000.0) );
            posmarker += (double)read_size1;
        }

        ov_clear( &pPluginWork->vf );
        WinPostMsg( pPluginWork->hwnd, WM_PLAYSTOP, 0, 0 );
        pPluginWork->fStatus = 0;
        close_func( (void *)stream );
        if(pPluginWork->fStop) {
            DosPostEventSem( pPluginWork->hevThreadA );
        }
        DosPostEventSem( pPluginWork->hevPlaySem );
    }
}

ULONG _System decoder_command( void *pvPluginWork, ULONG msg, DECODER_PARAMS *pParams )
{
    switch( msg ) {
        case DECODER_PLAY:
            return CommandPLAY( (PLUGINWORK *)pvPluginWork, pParams );

        case DECODER_STOP:
            return CommandSTOP( (PLUGINWORK *)pvPluginWork, pParams );

        case DECODER_FFWD:
            return CommandFFWD( (PLUGINWORK *)pvPluginWork, pParams );

        case DECODER_REW:
            return CommandREW( (PLUGINWORK *)pvPluginWork, pParams );

        case DECODER_JUMPTO:
            return CommandJUMPTO( (PLUGINWORK *)pvPluginWork, pParams );

        case DECODER_SETUP:
            return CommandSETUP( (PLUGINWORK *)pvPluginWork, pParams );

        case DECODER_EQ:
            return CommandEQ( (PLUGINWORK *)pvPluginWork, pParams );

        case DECODER_BUFFER:
            return CommandBUFFER( (PLUGINWORK *)pvPluginWork, pParams );

        case DECODER_SAVEDATA:
            return CommandSAVEDATA( (PLUGINWORK *)pvPluginWork, pParams );
    }
    return 1; /* command unsupported */
}

ULONG _System decoder_status( void *pvPluginWork )
{
    return ((PLUGINWORK*)pvPluginWork)->fStatus;
}

ULONG _System decoder_length(void *pvPluginWork)
{
    if(((PLUGINWORK *)pvPluginWork)->fStatus != DECODER_PLAYING) {
        return 0xFFFFFFFFUL;
    }
    return ((PLUGINWORK *)pvPluginWork)->songlength;
}

ULONG _System decoder_fileinfo(char *pszFilename, DECODER_INFO *pInfo)
{
    if(pInfo == NULL) {
        return 200;
    }
    {
        HFILE stream;
        ULONG rc;
        OggVorbis_File vf;
        vorbis_info *vi = NULL;
        ov_callbacks callbacks = {read_func, seek_func, close_func, tell_func};
        if (pszFilename != NULL && pszFilename[0] != 0) {
            ULONG ulAction;
            rc = DosOpen( pszFilename, &stream, &ulAction, 0, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0 );

            if (rc != 0) {
                return 100;
            }

            // The ov_open() function performs full stream detection and machine
            // initialization.  If it returns 0, the stream *is* Vorbis and we're
            // fully ready to decode.
            if (ov_open_callbacks( (void *)stream, &vf, NULL, 0, callbacks) < 0) {
                DosClose( stream );
                return 200;
            }
            /* 曲の時間(ms) */
            pInfo->songlength = (int)(ov_time_total(&vf, -1) * 1000.0);
            vi = ov_info( &vf, -1);

            pInfo->mpeg = 0;
            pInfo->layer = 0;
            pInfo->numchannels = 2;

            /* 曲データの始まるファイル先頭からの位置(byte) */
            /* 多分、MAC Binary用かな? */
            pInfo->junklength = 0;
#if 1
    {
        vorbis_comment *comment;
        comment = ov_comment( &vf, -1 );
        if(comment)
        {
            char *title, *artist;
            pInfo->bitrate = ov_bitrate( &vf, -1 );
            sprintf( pInfo->tech_info, "%5.1fkbs, %4.1fkHz, %s", (float)pInfo->bitrate / 1000.0, (float)vi->rate / 1000.0, (vi->channels == 1) ? "Mono" : "Stereo" );
            title = vorbis_comment_query(  comment, "title",  0 );
            if(title) strncpy( pInfo->title, title, 128 );
            artist = vorbis_comment_query( comment, "artist", 0 );
            if(artist) strncpy( pInfo->artist, artist, 128 );
//            strcpy( pInfo->album,    "album" );
//            strcpy( pInfo->year,    "year" );
//            strcpy( pInfo->comment,    "comment" );
//            strcpy( pInfo->genre,    "genre" );
        }
    }
#endif
#if 0
   /* general technical information string */
   char tech_info[128];

   /* song information */
   char title[128];
   char artist[128];
   char album[128];
   char year[128];
   char comment[128];
   char genre[128];
#endif


            pInfo->format.size = sizeof(FORMAT_INFO);
            pInfo->format.samplerate = vi->rate;
            pInfo->format.channels = vi->channels;
            pInfo->format.bits = 16;
            pInfo->bitrate = ov_bitrate( &vf, -1 ) / 1000;
            pInfo->format.format = WAVE_FORMAT_PCM;
            if(vi->channels > 2) /* We can't handle this */
            {
                ov_clear( &vf );
                DosClose( stream );
                return 200;
            }

            // once the ov_open() succeeds, the file belongs to vorbisfile.
            // ov_clear() will close it.
            ov_clear(&vf);
            DosClose( stream );
        } else {
            return 200;
        }
    }
    return 0;
}

ULONG _System decoder_support(char *fileext[], int *size)
{
    if(size) {
        if(fileext && *size >=1) {
            strcpy( fileext[0], "*.OGG" );
        }
        *size = 1;
    }
    return DECODER_FILENAME;
}


ULONG _System decoder_trackinfo(char *drive, int track, DECODER_INFO *info)
{
    return 200; /* decoder can't play that */
}

ULONG _System decoder_cdinfo(char *drive, DECODER_CDINFO *info)
{
    return 100; /* error reading file */
}

int _System plugin_query( PPLUGIN_QUERYPARAM param )
{
    param->type         = PLUGIN_DECODER;
    param->author       = "Sofiya";
    param->desc         = "Ogg Play 1.03";
    param->configurable = FALSE; /* Toggles plugin configurability via PM123 Properties dialog */
    return (int)param;
}
