#define INCL_DOSERRORS
#define INCL_OS2MM
#define INCL_MMIOOS2

#ifdef DEBUG
#define INCL_MCIOS2
#endif

#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>
#include <mad.h>

#ifdef DEBUG
#include <time.h>
#endif

#include "mmioMP3.h"
#include "xing.h"

#define BUFSIZE 16384
#define BYTES_PER_SAMPLE 2
#define BITS_PER_SAMPLE 8*BYTES_PER_SAMPLE

#ifdef DEBUG
static FILE *file;
void openDebugFile() {
   char path[CCHMAXPATH];
   char fileName[CCHMAXPATH+11];
   BOOL rc;
   if (!file) {
      rc = mciQuerySysValue(MSV_WORKPATH, path);
      if (rc) {
          sprintf(fileName,"%s\\mmiomp3%ld.log",path,time(NULL));
	      file = fopen(fileName,"wb");
      } else {
          exit(-1);
      } /* endif */
   }
}
#endif

/* Code from Chris Wohlgemuth. Modified */

BOOL cwGetStringFromEA(PSZ chrFileName,  PSZ pKey, char * chrBuffer, ULONG ulBuffSize) {
  LONG rc;                             /* Ret code                   */
  UCHAR       geabuff[300];            /* buffer for GEA             */
  PVOID       fealist;                 /* fealist buffer             */
  EAOP2       eaop;                    /* eaop structure             */
  PGEA2       pgea;                    /* pgea structure             */
  PFEA2       pfea;                    /* pfea structure             */
  HFILE       handle;                  /* file handle                */
  ULONG       act;                     /* open action                */
  ULONG ulSize;
  USHORT     *ptrUs;
  USHORT us;

  if ((rc = DosOpen(chrFileName, &handle, &act,
      0L, 0, OPEN_ACTION_OPEN_IF_EXISTS,
      OPEN_ACCESS_READONLY + OPEN_SHARE_DENYNONE +
      OPEN_FLAGS_FAIL_ON_ERROR + OPEN_FLAGS_WRITE_THROUGH,NULL))!=NO_ERROR) {
#ifdef DEBUG
  fprintf(file, "open %s for ea failed\n", chrFileName);
#endif
    return FALSE;
  }                                    /* get the file status info   */

  fealist= calloc(0x00010000L,1);
  if (0 == fealist) { return FALSE; }

                                       /* FEA and GEA lists          */
  eaop.fpGEA2List = (PGEA2LIST)geabuff;
  eaop.fpFEA2List = (PFEA2LIST)fealist;
  eaop.oError = 0;                     /* no error occurred yet      */
  pgea = &eaop.fpGEA2List->list[0];    /* point to first GEA         */
  eaop.fpGEA2List->cbList = sizeof(ULONG) + sizeof(GEA2) +
    strlen(pKey);
  eaop.fpFEA2List->cbList = (ULONG)0xffff;

                                       /* fill in the EA name length */
  pgea->cbName = (BYTE)strlen(pKey);
  strcpy(pgea->szName, pKey);          /* fill in the name           */
  pgea->oNextEntryOffset = 0;          /* fill in the next offset    */
                                       /* read the extended attribute*/
  rc = DosQueryFileInfo(handle, 3, (PSZ)&eaop, sizeof(EAOP2));
  DosClose(handle);                    /* close the file             */
  if (eaop.fpFEA2List->cbList <= sizeof(ULONG))
    rc = ERROR_EAS_NOT_SUPPORTED;      /* this is error also         */

  if (rc) {                            /* failure?                   */
	free(fealist);
    return FALSE;
  }

  pfea = &(eaop.fpFEA2List->list[0]);  /* point to the first FEA     */

  ptrUs=(USHORT*)((PSZ)pfea->szName+(pfea->cbName+1));
  if(*ptrUs!=0xfffd) {
    /* Not an ASCII EA */
	free(fealist);
    return FALSE;
  }
  ptrUs++; /* Move to length */
  ulSize=(ULONG)*ptrUs;

  ptrUs++; /* Move to value */
  if(ulSize==0) {
    /* Not an ASCII EA */
	free(fealist);
    return FALSE;
  }
  //  HlpWriteToTrapLog("Length: %x, Value: %s\n", ulSize, (char*)ptrUs);
  memset(chrBuffer,0,ulBuffSize);
  memcpy(chrBuffer, (char*)ptrUs, (ulSize>=ulBuffSize ? ulBuffSize-1 : ulSize));
  free(fealist);
  return TRUE;
}

/* from libid3tag/parse.c (modified, Source: madlib) */
unsigned long id3_parse_syncsafe(char const *ptr, unsigned int bytes)
{
  unsigned long value = 0;

  switch (bytes) {

  case 5:
    value = (value << 4) | (*(ptr)++ & 0x0f);
  case 4:
    value = (value << 7) | (*(ptr)++ & 0x7f);
    value = (value << 7) | (*(ptr)++ & 0x7f);
    value = (value << 7) | (*(ptr)++ & 0x7f);
    value = (value << 7) | (*(ptr)++ & 0x7f);
  }

  return value;
}

BOOL skipId3v2(HMMIO hmmio) {
	unsigned char *buf = (unsigned char *)malloc(10);
	long rc2;
	if (buf) {
		rc2 = mmioRead(hmmio, buf, 10);
		if (rc2 >= 10) {
            if (buf[0]=='I' && buf[1]=='D' && buf[2]=='3' &&
                buf[3]!=0xFF && buf[4]!=0xFF && (buf[6] & 0x80)==0 &&
                (buf[7] & 0x80)==0 && (buf[8] & 0x80)==0 && (buf[9] & 0x80)==0) {
                unsigned long headerlength = id3_parse_syncsafe(&(buf[6]),4);
                if ((buf[5] & 0x10)==0) {
                    headerlength += 10;
                } else {
                    headerlength += 20;
                }
                rc2 = mmioSeek(hmmio, headerlength-rc2, SEEK_CUR);
			} else {
      			rc2 = mmioSeek(hmmio, -10, SEEK_CUR);
			}
#ifdef DEBUG
fprintf(file, "skip id3v2 went to :%ld\n", rc2);
#endif
			return (rc2 > 0);
		}
	}
	return FALSE;
}

typedef struct _MP3INFO {
	HMMIO hmmioSS;
  struct {
    struct mad_stream stream;
    struct mad_frame frame;
    struct mad_synth synth;
  } sync;
  
    BOOL local;
 	unsigned char buf[BUFSIZE];
 	int bufend;
 	unsigned char out[4];
 	int outsize;
	int ptr;
	long datastart;
	long datasize;
	unsigned long location;
	unsigned long totalFrames;

	unsigned long vbr;
	unsigned int frames;
	unsigned long bestvbr;
	unsigned int bestframes;

    PMP3OPTIONS mp3Options;

	BOOL haveHeader;
    struct mad_header firstHeader;
} MP3INFO;

void reInit(MP3INFO *mp3info) {
	mp3info->outsize = 0;
	mp3info->ptr = -1;
	mp3info->bufend = 0;
	mp3info->vbr = 0;
	mp3info->frames = 0;
  	mad_stream_init(&mp3info->sync.stream);
 	mad_frame_init(&mp3info->sync.frame);
	mad_synth_init(&mp3info->sync.synth);   			
}

LONG totalBytes(MP3INFO *mp3info) {
    if (mp3info->haveHeader) {
       return mp3info->totalFrames*
       		32*MAD_NSBSAMPLES(&(mp3info->firstHeader))*
       		MAD_NCHANNELS(&(mp3info->firstHeader))*
       		BYTES_PER_SAMPLE;
    } else {
       	return 0;
    } /* endif */
}

LONG CodedBytesToPCMBytes(MP3INFO *mp3info, LONG codedBytes) {
	unsigned long scale;   	
	unsigned long bitrate = mp3info->bestvbr/mp3info->bestframes;
	USHORT channels = MAD_NCHANNELS(&(mp3info->firstHeader));
	ULONG samplesPerSec = mp3info->firstHeader.samplerate;
	USHORT bitsPerSample = BITS_PER_SAMPLE;

	scale = (channels * samplesPerSec * bitsPerSample)/1000;
	return (codedBytes/bitrate)*scale;
}

LONG PCMBytesToCodedBytes(MP3INFO *mp3info, LONG PCMBytes) {
	unsigned long scale;   	
	unsigned long bitrate = mp3info->bestvbr/mp3info->bestframes;
	USHORT channels = MAD_NCHANNELS(&(mp3info->firstHeader));
	ULONG samplesPerSec = mp3info->firstHeader.samplerate;
	USHORT bitsPerSample = BITS_PER_SAMPLE;

	scale = (channels * samplesPerSec * bitsPerSample)/1000;
//	if ((PCMBytes >> 16) > 0) 
		return (PCMBytes/scale)*bitrate;
//	else 
//		return (PCMBytes*bitrate)/scale;
}

static LONG fillStream(MP3INFO *mp3info) {
	LONG rc;
	int extrasize = 0;

	if (mp3info->sync.stream.next_frame >= mp3info->buf &&
	    mp3info->sync.stream.next_frame < mp3info->buf + mp3info->bufend)
	{
    	extrasize = mp3info->buf + mp3info->bufend - mp3info->sync.stream.next_frame;
#ifdef DEBUG
//fprintf(file, "Moving Memory: %d\n", extrasize);
#endif
		memmove(mp3info->buf,mp3info->sync.stream.next_frame, extrasize);
	}
	rc = mmioRead(mp3info->hmmioSS, mp3info->buf + extrasize, BUFSIZE-extrasize);
	if (rc > 0) {
    	mp3info->bufend = rc + extrasize;
		mad_stream_buffer(&(mp3info->sync.stream), mp3info->buf, mp3info->bufend );
	} else {
    	mp3info->bufend = 0;
    }
	return rc;
}

static LONG getHeader(MP3INFO *mp3info) {
	int rc;
	do {
#ifdef DEBUG
fprintf(file, "Get Header\n");
#endif
		rc = mad_frame_decode(&mp3info->sync.frame, &mp3info->sync.stream);
		if (-1 == rc) {
#ifdef DEBUG
fprintf(file, "err: %s\n", mad_stream_errorstr(&mp3info->sync.stream));
#endif
			if (MAD_ERROR_BUFLEN == mp3info->sync.stream.error ||
			    MAD_ERROR_BUFPTR == mp3info->sync.stream.error) {
           		if (0 == fillStream(mp3info)) return MMIO_ERROR;
   			} else if(!MAD_RECOVERABLE(mp3info->sync.stream.error)) {
          		return MMIO_ERROR;
        	} 
        } else {
             mp3info->haveHeader = TRUE;
             mp3info->firstHeader = mp3info->sync.frame.header;
			 {
      			struct xing xng;
				xing_init(&xng);
				if (0==xing_parse(&xng, mp3info->sync.stream.anc_ptr, mp3info->sync.stream.anc_bitlen)) {
		            mp3info->totalFrames=xng.frames;
#ifdef DEBUG
fprintf(file, "xing: %ld\n", mp3info->totalFrames);
#endif
       			}
 			 }
 		 	 mp3info->vbr += mp3info->sync.frame.header.bitrate/1000;
  		 	 mp3info->frames++;
  			 mad_synth_frame(&mp3info->sync.synth, &mp3info->sync.frame);
		     mp3info->ptr = 0;
        }
  	} while (-1 == rc);
  	if (0 == mp3info->bestframes) {
       mp3info->bestframes = 1;
       mp3info->bestvbr = mp3info->sync.frame.header.bitrate/1000;
    }
  	return MMIO_SUCCESS;
}

static LONG scanStartToEnd(MP3INFO *mp3info) {
    int rc;
	mmioSeek(mp3info->hmmioSS, 0L, SEEK_SET);
	mp3info->totalFrames = 0;
	while(1) {
  		rc = mad_header_decode(&mp3info->sync.frame.header, &mp3info->sync.stream);
		if (-1 == rc) {
			if (MAD_ERROR_BUFLEN == mp3info->sync.stream.error ||
			    MAD_ERROR_BUFPTR == mp3info->sync.stream.error) {
          	    rc = fillStream(mp3info);	
           		if (0 == rc) {
#ifdef DEBUG
fprintf(file,"bitrate about %ld\n",mp3info->vbr/mp3info->frames);
#endif
					if (mp3info->frames > mp3info->bestframes) {
						mp3info->bestframes = mp3info->frames;
						mp3info->bestvbr = mp3info->vbr;
					}
           			return MMIO_SUCCESS;
           		}
           		if (rc < 0) {
#ifdef DEBUG
fprintf(file, "seekToEnd: fill error: %d\n", rc);
#endif
					return rc;
                }
   			} else if(!MAD_RECOVERABLE(mp3info->sync.stream.error)) {
#ifdef DEBUG
fprintf(file, "seekToEnd: err: %s\n", mad_stream_errorstr(&mp3info->sync.stream));
#endif
          		return MMIO_ERROR;
        	}
        } else {
            mp3info->totalFrames++;
			mp3info->vbr += mp3info->sync.frame.header.bitrate/1000;
			mp3info->frames++;
		}
  	};
}


static LONG seekToLocation(MP3INFO *mp3info, unsigned long location) {
    int rc;
    int bytesPerFrame, alignment;
    unsigned long frames;
    int remainder;
    unsigned long curFrame = 0;
    
	if (!mp3info->haveHeader) {
#ifdef DEBUG
fprintf(file, "seek: no header\n");
#endif
		return MMIO_ERROR;
	}
    alignment = MAD_NCHANNELS(&(mp3info->firstHeader))*
            BYTES_PER_SAMPLE;
    bytesPerFrame = 32*MAD_NSBSAMPLES(&(mp3info->firstHeader))*
            alignment;
    frames = location/bytesPerFrame;
    remainder = location%bytesPerFrame;
	mmioSeek(mp3info->hmmioSS, mp3info->datastart, SEEK_SET);
#ifdef DEBUG
fprintf(file, "seek request: %ld\n",location);
#endif
	while(curFrame+29 < frames) {
  		rc = mad_header_decode(&mp3info->sync.frame.header, &mp3info->sync.stream);
		if (-1 == rc) {
			if (MAD_ERROR_BUFLEN == mp3info->sync.stream.error ||
			    MAD_ERROR_BUFPTR == mp3info->sync.stream.error) {
          	    rc = fillStream(mp3info);	
           		if (rc <= 0) {
#ifdef DEBUG
fprintf(file, "seekToLocation: rcerr: %d\n", rc);
#endif
           			return MMIO_ERROR;
           		}
   			} else if(!MAD_RECOVERABLE(mp3info->sync.stream.error)) {
#ifdef DEBUG
fprintf(file, "seekToLocation: err: %s\n", mad_stream_errorstr(&mp3info->sync.stream));
#endif
          		return MMIO_ERROR;
        	}
        } else {
            curFrame++;
			mp3info->vbr += mp3info->sync.frame.header.bitrate/1000;
			mp3info->frames++;
		}
  	};
#ifdef DEBUG
fprintf(file, "seek: almost there\n");
#endif
	while(curFrame < frames) {
  		rc = mad_header_decode(&mp3info->sync.frame.header, &mp3info->sync.stream);
		if (-1 == rc) {
			if (MAD_ERROR_BUFLEN == mp3info->sync.stream.error ||
			    MAD_ERROR_BUFPTR == mp3info->sync.stream.error) {
          	    rc = fillStream(mp3info);	
           		if (rc <= 0) {
#ifdef DEBUG
fprintf(file, "seekToLocation: rcerr: %d\n", rc);
#endif
           			return MMIO_ERROR;
           		}
   			} else if(!MAD_RECOVERABLE(mp3info->sync.stream.error)) {
#ifdef DEBUG
fprintf(file, "seekToLocation: err: %s\n", mad_stream_errorstr(&mp3info->sync.stream));
#endif
          		return MMIO_ERROR;
        	}
        } else {
            mad_frame_decode(&mp3info->sync.frame, &mp3info->sync.stream);
            curFrame++;
			mp3info->vbr += mp3info->sync.frame.header.bitrate/1000;
			mp3info->frames++;
		}
	};
#ifdef DEBUG
fprintf(file, "seek: really close now\n");
#endif
    if (curFrame > 0) {
		mad_synth_frame(&mp3info->sync.synth, &mp3info->sync.frame);
    } /* endif */
    while (1) {
		rc = mad_frame_decode(&mp3info->sync.frame, &mp3info->sync.stream);
		if (-1 == rc) {
			if (MAD_ERROR_BUFLEN == mp3info->sync.stream.error ||
			    MAD_ERROR_BUFPTR == mp3info->sync.stream.error) {
  	     	    rc = fillStream(mp3info);
  	     	    if (rc == 0 && curFrame>=mp3info->totalFrames) return bytesPerFrame*curFrame;
           		if (rc <= 0) {
#ifdef DEBUG
fprintf(file, "cur: %ld, total :%ld\n", curFrame, mp3info->totalFrames);
fprintf(file, "seekToLocation: rcerr: %d\n", rc);
#endif
           			return MMIO_ERROR;
           		}
   			} else if(!MAD_RECOVERABLE(mp3info->sync.stream.error)) {
#ifdef DEBUG
fprintf(file, "seekToLocation: err: %s\n", mad_stream_errorstr(&mp3info->sync.stream));
#endif
          		return MMIO_ERROR;
        	}
        } else {
			mad_synth_frame(&mp3info->sync.synth, &mp3info->sync.frame);
#ifdef DEBUG
if (0 != remainder%(BYTES_PER_SAMPLE*MAD_NCHANNELS(&(mp3info->firstHeader)))) {
	fprintf(file, "seek: not aligned\n");
} else {
	fprintf(file, "seek: made it\n");
} /* endif */
#endif
			remainder = (remainder/alignment)*alignment;
		    mp3info->ptr = (remainder/alignment)*alignment;
		    mp3info->location = remainder+bytesPerFrame*curFrame;
		    return mp3info->location;
        }
    }
}

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

LONG APIENTRY IOProc_Entry(PVOID pmmioStr, USHORT usMsg, LONG lParam1,
                     LONG lParam2) {
	PMMIOINFO pmmioinfo = (PMMIOINFO)pmmioStr;
#ifdef DEBUG	
	openDebugFile();
	fprintf(file,"command: %x\n",usMsg);
	fflush(file);
#endif
	switch (usMsg) {
	case MMIOM_OPEN:
		{	
     		MP3INFO *mp3info;
     		HMMIO hmmioSS;
     		MMIOINFO mmioinfoSS;
	    	PSZ pszFileName = (char *)lParam1;
   	 		if (!pmmioinfo) return MMIO_ERROR;
  	  		if (!(pmmioinfo->ulFlags & MMIO_READ)) return MMIO_ERROR;
			if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) &&
			    !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return MMIO_ERROR;
			if (!pmmioinfo->fccChildIOProc) {
				FOURCC fccFileStorageSystem;
        		if (mmioIdentifyStorageSystem(pszFileName, pmmioinfo, &fccFileStorageSystem)) {
            		return MMIO_ERROR;
            	}
            	if (!fccFileStorageSystem) {
             	  	return MMIO_ERROR;
            	} else {
             	  	pmmioinfo->fccChildIOProc = fccFileStorageSystem;
            	} /* endif */
			}
			memmove(&mmioinfoSS, pmmioinfo, sizeof(MMIOINFO));
			mmioinfoSS.pIOProc = NULL;
			mmioinfoSS.fccIOProc = pmmioinfo->fccChildIOProc;
			
			mmioinfoSS.ulFlags |= MMIO_NOIDENTIFY;
			
#ifdef DEBUG
			fprintf(file,"File: %s\n",pszFileName);
#endif
			hmmioSS = mmioOpen (pszFileName, &mmioinfoSS, mmioinfoSS.ulFlags);
			if (pmmioinfo->ulFlags & MMIO_DELETE) {
      			if (!hmmioSS) {
            		pmmioinfo->ulErrorRet = MMIOERR_DELETE_FAILED;
            		return MMIO_ERROR;
            	}
            	else return MMIO_SUCCESS;
			}
   			if (!hmmioSS) return MMIO_ERROR;
   			
       		mp3info = (MP3INFO *) calloc(1,sizeof(MP3INFO));
       		if (!mp3info) {
            	mmioClose(hmmioSS, 0);
            	return MMIO_ERROR;
            }
            
			mp3info->mp3Options = (PMP3OPTIONS)(pmmioinfo->pExtraInfoStruct);
			mp3info->local = (mmioinfoSS.fccIOProc == mmioFOURCC('D','O','S',' '));
#ifdef DEBUG
if (mp3info->local) {
	fprintf(file, "local file\n");
} else {
	fprintf(file, "remote file\n");
} /* endif */
#endif

	
			/* Get length of buffer */
			mp3info->datasize = mmioSeek(hmmioSS, 0L, SEEK_END);
			if (mp3info->datasize < 0) mp3info->datasize = 0;
			
       		pmmioinfo->pExtraInfoStruct = (PVOID)mp3info;
       		mp3info->hmmioSS = hmmioSS;
       		
			reInit(mp3info);
			
#if (0)
			/* Calculate bitrate */
			if (mp3info->local) {
					scanStartToEnd(mp3info);
      			if (mp3info->mp3Options) {
					scanStartToEnd(mp3info);
				}
				{ 
   					char timeInMsStr[30];
					if (cwGetStringFromEA(pszFileName,  "MMPLAYTIMEMS", timeInMsStr, 30)) {
#ifdef DEBUG
			fprintf(file,"read time %s from ea\n",timeInMsStr);
#endif
        				mp3info->timeInMS = atol(timeInMsStr);
        			} else {
#ifdef DEBUG
			fprintf(file,"read from ea failed.\n",timeInMsStr);
#endif
						scanStartToEnd(mp3info);
              		}
                }
			}
#endif

			mmioSeek(hmmioSS, 0L, SEEK_SET);
			skipId3v2(hmmioSS);
			mp3info->datastart = mmioSeek(hmmioSS,0L,SEEK_CUR);
			mp3info->location = 0;
#ifdef DEBUG
			fprintf(file,"Open successfull: %ld\n",mp3info->sync.stream.next_frame-mp3info->buf);
#endif

			return MMIO_SUCCESS;
		}
	    break;                                                 
	case MMIOM_READ: {
		MP3INFO *mp3info;
		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct || !lParam1) return MMIO_ERROR;
		
		mp3info = (MP3INFO*)pmmioinfo->pExtraInfoStruct;
		
		if (!pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) {
     		if (!lParam1)
     			return MMIO_ERROR;
     		return mmioRead (mp3info->hmmioSS, (PVOID) lParam1, (ULONG) lParam2);
     	} else {
         	int total;
         	
			if (!mp3info->haveHeader) {
	         	LONG rc;
				rc=getHeader(mp3info);
				if (MMIO_SUCCESS != rc) return 0;
	     		if (!mp3info->haveHeader) return MMIO_ERROR;
	     	}

			total = 0;
  			  while(1) {
			if (mp3info->ptr < 0) {
  				int rc;
				do {
#ifdef DEBUG
//               	fprintf(file,"Read\n");
#endif
 	 				rc = mad_frame_decode(&mp3info->sync.frame, &mp3info->sync.stream);
					if (-1 == rc) {
#ifdef DEBUG
               	fprintf(file,"Bad Read\n");
#endif
						if (MAD_ERROR_BUFLEN == mp3info->sync.stream.error ||
						    MAD_ERROR_BUFPTR == mp3info->sync.stream.error) {
             				int rc = fillStream(mp3info);
		  	         		if (0 == rc) {
								mp3info->location += total;
#ifdef DEBUG
fprintf(file, "done, location: %ld\n",mp3info->location);
#endif
		  	         			return total;
		  	         		}
		  	         		if (rc < 0) {
#ifdef DEBUG
fprintf(file, "fill error: %d\n", rc);
#endif
		  	         			return rc;
		  	         		}
            			} else if(!MAD_RECOVERABLE(mp3info->sync.stream.error)) {
#ifdef DEBUG
fprintf(file, "0 err: %s\n", mad_stream_errorstr(&mp3info->sync.stream));
#endif
    			      		return MMIO_ERROR;
      			    	}
		      	  }
			 	} while (-1 == rc);
			 	mp3info->vbr += mp3info->sync.frame.header.bitrate/1000;
			 	mp3info->frames++;
#ifdef DEBUG
//               	fprintf(file,"bitrate about %ld\n",mp3info->vbr/mp3info->frames);
#endif
				mad_synth_frame(&mp3info->sync.synth, &mp3info->sync.frame);
				mp3info->ptr = 0;
			}
			
			while (1) {
				signed int sample;
				int sampleSize = (mp3info->sync.synth.pcm.channels == 2)?4:2;
				while (mp3info->outsize > 0) {
					if (total >= lParam2) {
						mp3info->location += total;
#ifdef DEBUG
fprintf(file, "location: %ld\n",mp3info->location);
#endif
						return total;
					}
					mp3info->outsize--;
					*((unsigned char *)lParam1) = mp3info->out[mp3info->outsize];
					lParam1++;
					total++;
				}

				if (mp3info->ptr >= mp3info->sync.synth.pcm.length) {
       				mp3info->ptr = -1;
					break;
				}
				
				/* output sample(s) in 16-bit signed little-endian PCM */

			    sample = scale(mp3info->sync.synth.pcm.samples[0][mp3info->ptr]);
   				if (mp3info->sync.synth.pcm.channels == 2) {
				    mp3info->out[3] = (sample >> 0) & 0xff;
				    mp3info->out[2] = (sample >> 8) & 0xff;
				} else {
				    mp3info->out[1] = (sample >> 0) & 0xff;
				    mp3info->out[0] = (sample >> 8) & 0xff;
       			}

   				if (mp3info->sync.synth.pcm.channels == 2) {
				    sample = scale(mp3info->sync.synth.pcm.samples[1][mp3info->ptr]);
				    mp3info->out[1] = (sample >> 0) & 0xff;
				    mp3info->out[0] = (sample >> 8) & 0xff;
				}
				mp3info->outsize = sampleSize;
				mp3info->ptr++;
      		}
      		  }
        }
   	}
	break;
	case MMIOM_SEEK: {
        LONG lNewPos;
        LONG lPosDesired;
        SHORT sSeekMode;
        MP3INFO *mp3info;
        LONG totalbytes;
        
    	if (!pmmioinfo) return MMIO_ERROR;
		if (!pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) return MMIO_ERROR;    	
		mp3info = (MP3INFO*)pmmioinfo->pExtraInfoStruct;
		if (!mp3info) return MMIO_ERROR;

		if (0 == mp3info->bestvbr) {
	    	LONG rc;
			rc=getHeader(mp3info);
			if (MMIO_SUCCESS != rc) return rc;
 	    	if (0 == mp3info->bestvbr) return MMIO_ERROR;
     	}
     	
		if (mp3info->frames > mp3info->bestframes) {
     		mp3info->bestframes = mp3info->frames;
     		mp3info->bestvbr = mp3info->vbr;
    	}

        lPosDesired = lParam1;
		sSeekMode = (SHORT)lParam2;
#ifdef DEBUG
		fprintf(file,"Seek %ld, Mode: %d\n",lPosDesired,sSeekMode);
#endif	
		mad_stream_finish(&mp3info->sync.stream);
		mad_frame_finish(&mp3info->sync.frame);
		mad_synth_finish(&mp3info->sync.synth);
		reInit(mp3info);
		
        totalbytes = totalBytes(mp3info);
        if (totalbytes <= 0 && SEEK_END==sSeekMode) {
           	return MMIO_ERROR;
        } /* endif */
		if (mp3info->local) {
     		long target;
     		switch (sSeekMode) {
			  case SEEK_SET:
			    target = lPosDesired;
			    break;
			  case SEEK_END:
				target = totalbytes+lPosDesired;
			    break;
			  case SEEK_CUR:
			    target = mp3info->location+lPosDesired;
			    break;
			  default:
			    return MMIO_ERROR;
            }
            if (target < 0) {
                return MMIO_ERROR;
//            } else if (target >= totalbytes) {
//               	target = totalbytes;
            } /* endif */
		    return seekToLocation(mp3info, target);
        } else {
		    lNewPos = mmioSeek(mp3info->hmmioSS,
 	          PCMBytesToCodedBytes(mp3info,lPosDesired),
  	         sSeekMode);
			if (lNewPos < 0) return lNewPos;
			if (lNewPos > mp3info->datasize) return MMIO_ERROR;
#ifdef DEBUG
			fprintf(file,"File Seeked\n");
#endif
			mp3info->location = lNewPos;
		
			fillStream(mp3info);
		
			mp3info->sync.stream.sync = 0;
			mad_frame_mute(&mp3info->sync.frame);
			return CodedBytesToPCMBytes(mp3info, lNewPos);
		}
	}
    break;

	case MMIOM_CLOSE: {
		MP3INFO *mp3info;
		HMMIO hmmioSS;
		if (!pmmioinfo) return MMIO_ERROR;
		
		mp3info = (MP3INFO*)pmmioinfo->pExtraInfoStruct;
		if (mp3info) {
     		hmmioSS = mp3info->hmmioSS;
   			mad_stream_finish(&mp3info->sync.stream);
   			mad_frame_finish(&mp3info->sync.frame);
   			mad_synth_finish(&mp3info->sync.synth);
 		    mp3info->haveHeader = FALSE;
     		free (mp3info);
			pmmioinfo->pExtraInfoStruct = 0;
			mp3info = 0;
#ifdef DEBUG
       	    fprintf(file,"CLOSE\n");
#endif
	       	return mmioClose(hmmioSS, 0);
     	}
     	return MMIO_ERROR;
    }
    break;
	case MMIOM_IDENTIFYFILE: {
    	unsigned char *buf;
		HMMIO hmmioTemp;
		ULONG ulTempFlags = MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY;
		LONG rc = MMIO_ERROR;
        LONG rc2;
#ifdef NOIDENTIFY
		return MMIO_SUCCESS;
#endif
    	if (!lParam1 && !lParam2) return MMIO_ERROR;
    	hmmioTemp = (HMMIO)lParam2;
    	if (!hmmioTemp) {
        	hmmioTemp = mmioOpen((PSZ)lParam1, NULL, ulTempFlags);
        }

		if (hmmioTemp) {
			buf = (unsigned char *)malloc(BUFSIZE);
			if (buf) {
				skipId3v2(hmmioTemp);            
                rc2 = mmioRead(hmmioTemp, buf, BUFSIZE);
                if (rc2 >= 0) {
       				struct mad_stream *stream;
   	    			struct mad_frame *frame;
       				stream = (struct mad_stream *)malloc(sizeof(struct mad_stream));
       				frame = (struct mad_frame *)malloc(sizeof(struct mad_frame));
 					if (stream && frame) {
    				  	mad_stream_init(stream);
			         	mad_frame_init(frame);
       					mad_stream_buffer(stream, buf, rc2);
       					do {
#ifdef DEBUG
fprintf(file,"next: %ld, end: %ld\n", stream->next_frame, stream->bufend);
#endif
               				rc2 = mad_header_decode(&frame->header, stream);
#ifdef DEBUG
fprintf(file,"identify: %s\n", mad_stream_errorstr(stream));
#endif
               			} while (-1 == rc2 && MAD_RECOVERABLE(stream->error));
               			if (rc2==0) rc2 = mad_header_decode(&frame->header, stream);
#ifdef DEBUG
fprintf(file,"identify rc2: %ld\n", rc2);
#endif
               			rc = (0 == rc2)?MMIO_SUCCESS:MMIO_ERROR;
         	  			mad_stream_finish(stream);
 	 	 			    mad_frame_finish(frame);
     	 			}
     	 			if (stream) {
 	 	    			free(stream);
     	 				stream = 0;
 	 	    		}
     	 			if (frame) {
           				free(frame);
           				frame = 0;
           			}
                }
	     		free(buf);
			}
			if (!lParam2) mmioClose(hmmioTemp, 0);
		}
		return rc;
	}
    break;
	case MMIOM_GETFORMATINFO: {
    	PMMFORMATINFO pmmformatinfo;
    	pmmformatinfo = (PMMFORMATINFO)lParam1;
    	pmmformatinfo->ulStructLen = sizeof(MMFORMATINFO);
    	pmmformatinfo->fccIOProc = FOURCC_MP3;
    	pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
    	pmmformatinfo->ulMediaType = MMIO_MEDIATYPE_AUDIO;
    	pmmformatinfo->ulFlags = MMIO_CANREADTRANSLATED | MMIO_CANSEEKTRANSLATED; // | MMIO_CANREADUNTRANSLATED;
    	strcpy(pmmformatinfo->szDefaultFormatExt, ".MP3");
    	pmmformatinfo->ulCodePage = 0;
    	pmmformatinfo->ulLanguage = 0;
    	pmmformatinfo->lNameLength = 21;
    	return MMIO_SUCCESS;
    }
    break;
	case MMIOM_GETFORMATNAME:
		if (lParam2 > 21) { 
     		strcpy((PSZ)lParam1, "MP3AUDIO");
     		return MMIO_SUCCESS;
     	} else return MMIO_ERROR;
    break;
    case MMIOM_QUERYHEADERLENGTH: return (sizeof (MMAUDIOHEADER));
    break;
	case MMIOM_GETHEADER: {
    	MP3INFO *mp3info;
    	PMMAUDIOHEADER mmaudioheader;
    	
		if (!pmmioinfo) return MMIO_ERROR;
		mp3info = (MP3INFO*)pmmioinfo->pExtraInfoStruct;
		
		if (!(pmmioinfo->ulFlags & MMIO_READ)) return 0;
		if (!mp3info || !mp3info->hmmioSS) return 0;
#ifdef DEBUG
		fprintf(file,"HERE\n");
#endif
		if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) &&
		    !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return 0;
		mmaudioheader = (MMAUDIOHEADER *)lParam1;
		if (sizeof(MMAUDIOHEADER) > lParam2) {
            pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
			return 0;
		}

		if (!mp3info->haveHeader) {
	    	LONG rc;
			rc=getHeader(mp3info);
			if (MMIO_SUCCESS != rc) {
                pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
                return 0;
            }
 	    	if (!mp3info->haveHeader) {
                pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
                return 0;
            }
 	    }
#ifdef DEBUG
		fprintf(file,"THERE\n");
#endif
		
		mmaudioheader->ulContentType = MMIO_MIDI_UNKNOWN;
		mmaudioheader->ulMediaType = MMIO_MEDIATYPE_AUDIO;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usFormatTag=DATATYPE_WAVEFORM;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels = MAD_NCHANNELS(&(mp3info->firstHeader));
		mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec = mp3info->firstHeader.samplerate;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample = BITS_PER_SAMPLE;
		mmaudioheader->mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec=
			mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels * 
			mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec *
			mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample / 8;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usBlockAlign=
			mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels * 
			mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample / 8;
		if (mp3info->sync.frame.header.bitrate > 0) {
			if (totalBytes(mp3info)) {
					mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes=
						totalBytes(mp3info);
					mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS=
						(mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes)/
						(mmaudioheader->mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec/1000);
			} else {
				mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS=
					(mp3info->datasize-mp3info->datastart)/((mp3info->sync.frame.header.bitrate)/8000);
				mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes=
			 	    (mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS/10)*
			  	    (mmaudioheader->mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec/100);
			} /* endif */
		} else {
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS= 0;
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes= 0;
     	}

#ifdef DEBUG
		fprintf(file,"time: %ld size: %ld bytes/sec: %ld\n",
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS,
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes,
			mmaudioheader->mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec);
	
#endif                                              
	
		if (0 != mp3info->mp3Options && MP3_COOKIE == mp3info->mp3Options->cookie) {
			if (mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes && totalBytes(mp3info)) {
      				mp3info->mp3Options->bitrate = ((long long)mp3info->datasize-mp3info->datastart)*mmaudioheader->mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec*8/
						mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes;
			} else {
      			mp3info->mp3Options->bitrate = mp3info->sync.frame.header.bitrate;
			} /* endif */
		    mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.pAdditionalInformation = mp3info->mp3Options;
        }
		return (sizeof (MMAUDIOHEADER));
    }
    break;
	}
#ifdef DEBUG	
	fprintf(file,"unexpected command: %x\n",usMsg);
#endif
	return MMIOERR_UNSUPPORTED_MESSAGE;
}

