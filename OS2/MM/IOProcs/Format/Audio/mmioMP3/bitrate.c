/* Public Domain.  Author: Russell O'Connor */

/* This code illustrates how to use the MMIOMP3 audio IO Proc
    to determine the bitrate of an MP3 file */

#define INCL_OS2MM
#define INCL_MMIOOS2
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include "mmioMP3.h"

char buf[131072];

int main(int argc, char *argv[])
{
      MMIOINFO mmIn;
	  MP3OPTIONS mp3opt;
      HMMIO in;
      int result = 0;
      LONG rc;

      memset(&mmIn, 0, sizeof(mmIn));

      if (2 != argc) {
         printf("usage: %s <filename_input>",argv[0]);
		 return -2;
      } /* endif */
      
	  mp3opt.cookie = MP3_COOKIE;
      mmIn.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
      mmIn.pExtraInfoStruct = &mp3opt;
      in = mmioOpen(argv[1], &mmIn, MMIO_READ | MMIO_DENYNONE);

      if (in) {
         ULONG headerLength;
         rc = mmioQueryHeaderLength(in, &headerLength, 0, 0);
         if (MMIO_SUCCESS==rc && headerLength == sizeof(MMAUDIOHEADER)) {
           MMAUDIOHEADER header;
           LONG bytesRead = 0;
         
           memset(&header, 0, sizeof(header));
           rc = mmioGetHeader(in, &header, sizeof(header), &bytesRead, 0, 0);
           if (MMIO_SUCCESS==rc) {
			 PMP3OPTIONS returnedOpts = (PMP3OPTIONS)header.mmXWAVHeader.XWAVHeaderInfo.pAdditionalInformation;
             /* could use mp3opt directly here */
             /* it should be the case that returnedOpts = mp3opt */
			 if (0 != returnedOpts && MP3_COOKIE == returnedOpts->cookie && 0 != returnedOpts->bitrate) {
                printf("bitrate: %ld bits per second\n",returnedOpts->bitrate);
			 } else {
                printf("couldn't access bitrate information\n");
				result = -1;
             } /* endif */
         } else {
            printf("Can't read input header.\n");
   		    result = -1;
         }
        } else {
            printf("Can't read input header length.\n");
			result = -1;
        } /* endif */
       mmioClose(in, 0);
      } else {
         printf("Can't open %s:%ld.\n",argv[1],mmIn.ulErrorRet);
         result = -1;
      }

      return result;
}
