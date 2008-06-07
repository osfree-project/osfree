/* Public Domain.  Auther: Russell O'Connor */

/* Converts anything to (Vorbis/FLAC) */

#define INCL_OS2MM
#define INCL_MMIOOS2
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include "mmioVorbis.h"

char buf[131072];
//char buf[4096];

int main(int argc, char *argv[])
{
      MMIOINFO mmIn, mmOut;
	  VORBISOPTIONS vorbisOpt;
      HMMIO in, out;
      int result = 0;
      LONG rc;

      memset(&mmIn, 0, sizeof(mmIn));
      memset(&mmOut, 0, sizeof(mmOut));
      memset(&vorbisOpt, 0, sizeof(vorbisOpt));

      if (6 != argc  && 3 != argc) {
         printf("ogg usage: %s <filename_input> <filename_output> <min_bitrate> <nominal_bitrate> <max_bitrate>\n",argv[0]);
         printf("or: %s <filename_input> <filename_output> -1 <nominal_bitrate> -1\n",argv[0]);
         printf("or: %s <filename_input> <filename_output> <min_bitrate> -1 <max_bitrate>\n",argv[0]);
         printf("bitrate is measured in bits per second, use values such as 160000.\n");
 	 return -2;
      } /* endif */

      mmIn.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
      in = mmioOpen(argv[1], &mmIn, MMIO_READ | MMIO_DENYNONE);

      if (in) {
       mmOut.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
       mmOut.fccIOProc=mmioFOURCC('O','G','G','S');
	   if (argc >= 6) {
		   vorbisOpt.cookie = VORBIS_COOKIE;
		   vorbisOpt.min_bitrate = atol(argv[3]);
  	 	   vorbisOpt.nominal_bitrate = atol(argv[4]);
   	 	   vorbisOpt.max_bitrate = atol(argv[5]);
		   mmOut.pExtraInfoStruct = &vorbisOpt;
	   }
       out = mmioOpen(argv[2], &mmOut, MMIO_WRITE | MMIO_CREATE | MMIO_DENYNONE);
      
       if (out) {
        LONG headerLength = 0;

        rc = mmioQueryHeaderLength(in, &headerLength, 0, 0);
        if (MMIO_SUCCESS==rc && headerLength > 0) {
         void *header = malloc(headerLength);
         LONG bytesRead = 0;
         
         memset(header, 0, headerLength);
         rc = mmioGetHeader(in, header, headerLength, &bytesRead, 0, 0);
         if (MMIO_SUCCESS==rc) {
          LONG bytesWrite = 0;

          rc = mmioSetHeader(out, header, headerLength, &bytesWrite, 0,0);
          if (MMIO_SUCCESS==rc) {
           LONG nRead, nWrite;

           while(1) {
            nRead = mmioRead(in, buf, sizeof(buf));
            if (nRead<=0) {
               if (nRead<0) {
                 printf("read failed.\n");
               }
               break;
            } /* endif */
            nWrite = mmioWrite(out, buf, nRead);
            if (nWrite != nRead) {
                printf("write failed.\n");
            } /* endif */
            printf("-");
           }
          } else {
             printf("Can't write output header.\n");
			 result = -1;
          } /* endif */

         } else {
            printf("Can't read input header.\n");
			 result = -1;
         }
         free(header);
        } else {
            printf("Can't read input header length.\n");
  		    result = -1;
        } /* endif */

        mmioClose(out, 0);
     
       } else {
         printf("Can't open %s:%ld.\n",argv[2],mmOut.ulErrorRet);
         result = -1;
       } 
     
      mmioClose(in, 0);
      
      } else {
         printf("Can't open %s:%ld.\n",argv[1],mmIn.ulErrorRet);
         result = -1;
      }

      return result;
}
