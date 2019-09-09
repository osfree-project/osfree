#define INCL_OS2MM
#define INCL_MMIOOS2
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "mmiovorbis.h"

int main(int argc, char *argv[]) {
	MMIOINFO mmioinfo;
	HMMIO hmmio;
    MCI_OPEN_PARMS openParams;
    PSZ fileName = "test.ogg";
	ULONG ulFlags;
	int rc;
	
	printf("start\n");
    memset (&mmioinfo, 0, sizeof(MMIOINFO));
    mmioinfo.fccIOProc = FOURCC_Vorbis;
//    mmioinfo.fccIOProc = mmioFOURCC('W','A','V','E');
    ulFlags = MMIO_WRITE | MMIO_DENYNONE | MMIO_CREATE;
    mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
 
    hmmio = mmioOpen(fileName, &mmioinfo, ulFlags);
    if (!hmmio) {
       printf("Can't open %s:%ld.\n",fileName,mmioinfo.ulErrorRet);
       return -1;
    } 
    
    memset (&openParams, 0, sizeof(openParams));
    openParams.pszDeviceType = "WaveAudio";
    openParams.pszElementName = (PSZ)hmmio;
    rc = mciSendCommand (0, MCI_OPEN, MCI_WAIT | MCI_OPEN_MMIO,&openParams,0);
    
    if (LOUSHORT(rc) == MCIERR_SUCCESS) {
        USHORT usDeviceID = openParams.usDeviceID;
        MCI_RECORD_PARMS recParams;
  
        memset (&recParams, 0, sizeof(recParams));
        rc = mciSendCommand (usDeviceID, MCI_RECORD,0 , &recParams, 0);
        if (LOUSHORT(rc)== MCIERR_SUCCESS) {
            sleep(2);
            rc = mciSendCommand (usDeviceID, MCI_STOP, MCI_WAIT, 0, 0);
            if (LOUSHORT(rc) == MCIERR_SUCCESS) {
              MCI_SAVE_PARMS saveParams;

              memset (&saveParams, 0, sizeof(saveParams));
              rc = mciSendCommand (usDeviceID, MCI_SAVE, MCI_WAIT,&saveParams , 0);
              if (LOUSHORT(rc) != MCIERR_SUCCESS) {
               printf("save failed:%d.\n",rc);
              }
            } else {
               printf("stop failed:%d.\n",rc);
            }
        } else {
           printf("Can't record:%d.\n",rc);
        } /* endif */

        rc = mciSendCommand (usDeviceID, MCI_CLOSE, MCI_WAIT, 0, 0);
        if (LOUSHORT(rc) != MCIERR_SUCCESS) {
           printf("Can't close audioDevice:%d.\n",rc);
        } /* endif */
    } else {
       printf("Can't open audioDevice:%d.\n",rc);
    }    
    return mmioClose(hmmio, 0);
}

