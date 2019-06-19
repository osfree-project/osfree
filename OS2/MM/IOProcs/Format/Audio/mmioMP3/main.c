#define INCL_OS2MM
#define INCL_MMIOOS2
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "mmiomp3.h"

int main(int argc, char *argv[]) {
	PMMIOPROC pmmioprocMp3IOProc;
	char retmsg[1024];
	char reterr[1024];
	int rc;
	
	printf("start?\n");


/*
	mciSendString("open d:\\sounds\\relmorch.wav alias wav1 wait", retmsg, 1024, 0, 0);
	printf("%s\n",retmsg);
	mciSendString("play wav1 wait", retmsg, 1024, 0, 0);
	printf("%s\n",retmsg);
*/

/*
	pmmioprocMp3IOProc = mmioInstallIOProc(FOURCC_MP3, IOProc_Entry, MMIO_INSTALLPROC);
	printf("MMIO PROC Installed at %d\n",(unsigned int)pmmioprocMp3IOProc);
	printf("vs %d\n",(unsigned int)IOProc_Entry);
*/

/*
	{
    	USHORT usDeviceID;
		{
	    	MCI_OPEN_PARMS mciopenparms;
			memset(&mciopenparms, 0, sizeof(mciopenparms));
   			mciopenparms.pszDeviceType="waveaudio";

			rc = mciSendCommand(0, MCI_OPEN, MCI_WAIT, (PVOID) &mciopenparms,0);
			mciGetErrorString(rc, reterr, 1024);
			printf("%s\n",reterr);
			usDeviceID = mciopenparms.usDeviceID;
		}
		{
     		MCI_LOAD_PARMS mciloadparms;
	   		memset(&mciloadparms, 0, sizeof(mciloadparms));
     		{
		   		MMIOINFO mmioinfoSS;
 		  		memset(&mmioinfoSS, 0, sizeof(mmioinfoSS));
		   		mmioinfoSS.fccIOProc = FOURCC_MP3;
		   		mmioinfoSS.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
  			  	mciloadparms.pszElementName = (PSZ)mmioOpen(argv[1], &mmioinfoSS, MMIO_READ);
				printf("open okay\n");
			}
			rc = mciSendCommand(usDeviceID , MCI_LOAD, MCI_WAIT | MCI_OPEN_MMIO | MCI_READONLY, (PVOID) &mciloadparms, 0);
			mciGetErrorString(rc, reterr, 1024);
			printf("%s\n",reterr);
		}
		{
     		MCI_PLAY_PARMS mciplayparms;
	   		memset(&mciplayparms, 0, sizeof(mciplayparms));
			rc = mciSendCommand(usDeviceID , MCI_PLAY, MCI_WAIT, (PVOID) &mciplayparms, 0);
			mciGetErrorString(rc, reterr, 1024);
			printf("%s\n",reterr);
		}
	}
*/

//	rc = mciSendString("open waveaudio alias wave wait", retmsg, 1024, 0, 0);
	rc = mciSendString("open H:\\MMIOMP3\\0.2\\TEST.MP3  type WAVEAUDIO alias wave wait", retmsg, 1024, 0, 0);
	mciGetErrorString(rc, reterr, 1024);
	printf("%s : %s\n",retmsg,reterr);
//	rc = mciSendString("load wave d:\\sounds\\relmorch.wav wait", retmsg, 1024, 0, 0);
//	rc = mciSendString("load wave h:\\mpg123\\59r\\mpglib\\test.mp3 wait", retmsg, 1024, 0, 0);
	mciGetErrorString(rc, reterr, 1024);
	printf("%s : %s\n",retmsg,reterr);
	rc = mciSendString("play wave wait", retmsg, 1024, 0, 0);
	printf("DonePlaying\n");
	mciGetErrorString(rc, reterr, 1024);
	printf("%s : %s\n",retmsg,reterr);


	return 0;
}

