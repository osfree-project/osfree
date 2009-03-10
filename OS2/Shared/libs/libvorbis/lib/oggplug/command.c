#define INCL_DOS
#include <os2.h>
#include <process.h>
#include <stdio.h>

#include "format.h"
#include "decoder_plug.h"
#include "plugin.h"

#include "include.h"

ULONG CommandFFWD( PLUGINWORK *pPluginWork, DECODER_PARAMS *pParams )
{
    pPluginWork->ffwd = pParams->ffwd;
    return 0;
}

ULONG CommandREW(  PLUGINWORK *pPluginWork, DECODER_PARAMS *pParams )
{
    pPluginWork->rew = pParams->rew;
    return 0;
}

ULONG CommandJUMPTO(  PLUGINWORK *pPluginWork, DECODER_PARAMS *pParams )
{
    pPluginWork->jumpto = pParams->jumpto;    /* absolute positioning in milliseconds */
    return 0;
}

ULONG CommandSETUP( PLUGINWORK *pPluginWork, DECODER_PARAMS *pParams )
{
    pPluginWork->current_section     = -1;
    pPluginWork->output_play_samples = pParams->output_play_samples;
    pPluginWork->a                   = pParams->a;
    pPluginWork->hevPlaySem          = pParams->playsem;
    pPluginWork->BufferSize          = pParams->audio_buffersize;
    pPluginWork->hwnd                = pParams->hwnd;

    DosPostEventSem( pParams->playsem );

    return 0;
}

ULONG CommandPLAY(  PLUGINWORK *pPluginWork, DECODER_PARAMS *pParams )
{
    ULONG ulTmp;

    if(pPluginWork->fStatus != 0) {
        return 101;
    }
    strncpy( pPluginWork->szFileName, pParams->filename, 4096 );
    DosResetEventSem( pPluginWork->hevThreadA, &ulTmp );
    DosPostEventSem( pPluginWork->hevThreadTrigger );
    DosWaitEventSem( pPluginWork->hevThreadA, 20000 );
    
    return 0;
}

ULONG CommandSTOP(  PLUGINWORK *pPluginWork, DECODER_PARAMS *pParams )
{
    ULONG ulTmp;

    if(pPluginWork->fStatus == 0) {
        return 101;
    }
    DosResetEventSem( pPluginWork->hevThreadA, &ulTmp );
    pPluginWork->fStop = TRUE;
    DosWaitEventSem( pPluginWork->hevThreadA, 20000 );
    return 0;
}

ULONG CommandEQ(  PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams )
{
    return 1;
}

ULONG CommandBUFFER(  PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams )
{
    return 1;
}

ULONG CommandSAVEDATA(  PLUGINWORK *pvPluginWork, DECODER_PARAMS *pParams )
{
    return 1;
}















size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    ULONG ulActual;

    DosRead( (HFILE)datasource, ptr, (size*nmemb), &ulActual );
    return ulActual / size;
}

int seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    ULONG ulMethod;
    ULONG ibActual;
    switch(whence) {
        case SEEK_SET: ulMethod = FILE_BEGIN; break;
        case SEEK_CUR: ulMethod = FILE_CURRENT; break;
        case SEEK_END: ulMethod = FILE_END; break;
        default:
            return 0;
    }
    DosSetFilePtr( (HFILE)datasource, (LONG)(offset & 0xffffffff), ulMethod, &ibActual );
    return 0;
}

int close_func(void *datasource)
{
    DosClose( (HFILE)datasource );
    return 0;
}

long tell_func(void *datasource)
{
    ULONG ibActual;
    DosSetFilePtr( (HFILE)datasource, 0, FILE_CURRENT, &ibActual );

    return ibActual;
}

long Read( PLUGINWORK* pPluginWork, void *pvData, long lSize, long *plSize1 )
{
    unsigned char *pb = (unsigned char *)pvData;
    long lRead = 0;
    *plSize1 = 0;

    while(lSize > 0) {
        if(pPluginWork->lRemain) {
            /**/
            if(pPluginWork->ffwd) {
                int i;
                long lTransSize = min( lSize * 2, pPluginWork->lRemain );
                for(i = 0; i < lTransSize / 4; i++) {
                    ((short *)pb)[i] = ((short *)&pPluginWork->sample_buffer[pPluginWork->lOffset])[i * 2];
                }
                pb                   += lTransSize / 2;
                lSize                -= lTransSize / 2;
                lRead                += lTransSize / 2;

                pPluginWork->lRemain -= lTransSize;
                pPluginWork->lOffset += lTransSize;
                *plSize1             += lTransSize;
            } else {
                long lTransSize = min( lSize, pPluginWork->lRemain );
                memcpy( pb, &pPluginWork->sample_buffer[pPluginWork->lOffset], lTransSize );
                pb                   += lTransSize;
                lSize                -= lTransSize;
                lRead                += lTransSize;

                pPluginWork->lRemain -= lTransSize;
                pPluginWork->lOffset += lTransSize;
                *plSize1             += lTransSize;
            }
        } else {
//            int current_section = -1;
            pPluginWork->lRemain = ov_read( &pPluginWork->vf, pPluginWork->sample_buffer, 576 * pPluginWork->format.channels * 2, 0, 2, 1, &pPluginWork->current_section );
            if(pPluginWork->lRemain < 0) {
                /* ‰½‚©‚µ‚çƒGƒ‰[‚ª‹N‚«‚½ */
                pPluginWork->lRemain = 0;
                continue;
            } else if(pPluginWork->lRemain == 0) {
//                fEOF = 1;
                break;
            }
            pPluginWork->lOffset = 0;
        }
    }
    return lRead;
}
