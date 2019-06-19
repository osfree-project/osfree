#define INCL_OS2MM
#define INCL_MMIOOS2

#ifdef DEBUG
#define INCL_MCIOS2
#endif

#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>

#include <ctype.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>
#include <float.h>
#include "mmioVorbis.h"

#ifdef DEBUG
static FILE *file;
void openDebugFile() {
   char path[CCHMAXPATH];
   char fileName[CCHMAXPATH+11];
   BOOL rc;
   if (!file) {
      rc = mciQuerySysValue(MSV_WORKPATH, path);
      if (rc) {
          sprintf(fileName,"%s\\mmioVorbis%ld.log",path,time(NULL));
	      file = fopen(fileName,"wb");
//	      file = freopen(fileName,"wb",stderr);
      } else {
          exit(-1);
      } /* endif */
   }
}
#endif

#define READNUM 0x52656164
#define WRITENUM 0x57726974

size_t mread(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	if (!ptr || !datasource) return -1;
	return mmioRead((HMMIO)datasource, ptr, nmemb);
}

int mseek(void *datasource, ogg_int64_t offset, int whence) {
 	long rc = mmioSeek((HMMIO)datasource, offset, whence);
#ifdef DEBUG
fprintf(file,"Attempt %ld, %d. Seeked to %d\n",offset,whence,rc);
#endif
 	if (rc < 0) return rc;
 	return 0;
}

int mclose(void *datasource){
	if (!datasource) return -1;
	return mmioClose((HMMIO)datasource, 0);
}

long mtell(void *datasource) {
   	return mmioSeek((HMMIO)datasource, 0, SEEK_CUR);
}

typedef struct _DecInfo {
  int t; /*Always READNUM */
  PVORBISOPTIONS vorbisOptions;
  OggVorbis_File oggfile;
} DecInfo;
    

typedef struct _EncInfo {
  int t; /*Always WRITENUM */
  PVORBISOPTIONS vorbisOptions;
  BOOL headerSet;
  int bitsPerSample;
  ogg_stream_state os; /* take physical pages, weld into a logical
			  stream of packets */
  ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
  ogg_packet       op; /* one raw packet of data for decode */
  
  vorbis_info      vi; /* struct that stores all the static vorbis bitstream
			  settings */
  vorbis_comment   vc; /* struct that stores all the user comments */

  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */
  HMMIO hmmioSS;
} EncInfo;

int oggWrite(EncInfo* encInfo,int close) {
    int total = 0;
#ifdef DEBUG
fprintf(file,"oggwrite\n");
#endif
    /* vorbis does some data preanalysis, then divvies up blocks for
       more involved (potentially parallel) processing.  Get a single
       block for encoding now */

    while(vorbis_analysis_blockout(&encInfo->vd,&encInfo->vb)==1){
      /* analysis */
      vorbis_analysis(&encInfo->vb,NULL);
      vorbis_bitrate_addblock(&encInfo->vb);
      
      while(vorbis_bitrate_flushpacket(&encInfo->vd,&encInfo->op)){
      /* weld the packet into the bitstream */
      ogg_stream_packetin(&encInfo->os,&encInfo->op);

      /* write out pages (if any) */
      while(!ogg_page_eos(&encInfo->og)){
          int result;
#ifdef DEBUG
fprintf(file,"Write a page\n");
#endif
          result=ogg_stream_pageout(&encInfo->os,&encInfo->og);
          if(result==0)break;
          result = mmioWrite(encInfo->hmmioSS, encInfo->og.header,encInfo->og.header_len);
          total += result;
          if (result!=encInfo->og.header_len) return -1;
          result = mmioWrite(encInfo->hmmioSS, encInfo->og.body,encInfo->og.body_len);
          total += result;
          if (result!=encInfo->og.body_len) return -1;
      }
    }
    }
#ifdef DEBUG
fprintf(file,"Wrote %d\n",total);
#endif
    return total;
}

static LONG APIENTRY IOProc_Entry2(PVOID pmmioStr, USHORT usMsg, LONG lParam1,
                     LONG lParam2) {
	PMMIOINFO pmmioinfo = (PMMIOINFO)pmmioStr;
	switch (usMsg) {
	case MMIOM_OPEN:
		{	
     		HMMIO hmmioSS;
     		MMIOINFO mmioinfoSS;
	    	PSZ pszFileName = (char *)lParam1;
   	 		if (!pmmioinfo) return MMIO_ERROR;
  	  		if ((pmmioinfo->ulFlags & MMIO_READWRITE)) {
#ifdef DEBUG
			fprintf(file,"ReadWrite - requested.\n");
#endif
   	  		   return MMIO_ERROR;
  	  		}
			if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) &&
			    !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return MMIO_ERROR;
			if (!pmmioinfo->fccChildIOProc) {
				FOURCC fccFileStorageSystem;
				if (pmmioinfo->ulFlags & MMIO_CREATE) {
       				if (mmioDetermineSSIOProc(pszFileName, pmmioinfo, &fccFileStorageSystem, NULL)) {
              			fccFileStorageSystem = FOURCC_DOS;
              		}
       			} else {
	        		if (mmioIdentifyStorageSystem(pszFileName, pmmioinfo, &fccFileStorageSystem)) {
 		           		return MMIO_ERROR;
    	        	}
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
			
			hmmioSS = mmioOpen (pszFileName, &mmioinfoSS, mmioinfoSS.ulFlags);
			if (pmmioinfo->ulFlags & MMIO_DELETE) {
      			if (!hmmioSS) {
            		pmmioinfo->ulErrorRet = MMIOERR_DELETE_FAILED;
            		return MMIO_ERROR;
            	}
            	else return MMIO_SUCCESS;
			}
   			if (!hmmioSS) return MMIO_ERROR;
   			if (pmmioinfo->ulFlags & MMIO_READ) {
 			   DecInfo *decInfo = (DecInfo *)malloc(sizeof(DecInfo));
#ifdef DEBUG
			fprintf(file,"File Read: %s\n",pszFileName);
#endif
        		if (!decInfo) {
  	          		mmioClose(hmmioSS, 0);
            		return MMIO_ERROR;
	            }
				decInfo->t = READNUM;
			    decInfo->vorbisOptions = pmmioinfo->pExtraInfoStruct;
 	      		pmmioinfo->pExtraInfoStruct = (PVOID)decInfo;
  	     		{
            		ov_callbacks cb;
   	         		cb.read_func = mread;
            		cb.seek_func = mseek;
            		cb.close_func = mclose;
            		cb.tell_func = mtell;
	       			if(0 != ov_open_callbacks((void *)hmmioSS, &decInfo->oggfile, 0, 0, cb)) {
           	  			free(decInfo);
	            		mmioClose(hmmioSS, 0);
 	           			return MMIO_ERROR;
             		}
            	}
#ifdef DEBUG
				fprintf(file,"Open successfull\n");
#endif
				return MMIO_SUCCESS;
			} else if (pmmioinfo->ulFlags & MMIO_WRITE) {
	       		EncInfo *encInfo = (EncInfo *)malloc(sizeof(EncInfo));
#ifdef DEBUG
			fprintf(file,"File Write: %s\n",pszFileName);
#endif
	       		if (!encInfo) {
  	          		mmioClose(hmmioSS, 0);
            		return MMIO_ERROR;
                }
	       		memset(encInfo, 0, sizeof(EncInfo));
	       		encInfo->t = WRITENUM;
	       		encInfo->hmmioSS = hmmioSS;
				encInfo->vorbisOptions = (PVORBISOPTIONS)pmmioinfo->pExtraInfoStruct;
 	      		pmmioinfo->pExtraInfoStruct = (PVOID)encInfo;
 	      		return MMIO_SUCCESS;
      		}
#ifdef DEBUG
			fprintf(file,"File not read nor write: %s\n",pszFileName);
#endif
      		return MMIO_ERROR;
		}
	    break;                                                 
	case MMIOM_READ: {
		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct || !lParam1) return MMIO_ERROR;
		
		if (!pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) {
     		return MMIO_ERROR;
//     		return mmioRead (ogginfo->hmmioSS, (PVOID) lParam1, (ULONG) lParam2);
     	} else {
			OggVorbis_File *oggfile;
			long rc = 0;
			int current_section;
			long total = 0;
			
			oggfile = &((DecInfo *)pmmioinfo->pExtraInfoStruct)->oggfile;
			if (READNUM != ((DecInfo *)pmmioinfo->pExtraInfoStruct)->t) return MMIO_ERROR;
			while (lParam2 > 0) {
	         	rc = ov_read(oggfile, (char *)lParam1, (int)lParam2, 0, 2, 1,  &current_section);
	         	if (rc < 0) {
#ifdef DEBUG
fprintf(file, "Read failed once\n");
#endif
				continue;
				}
				if (rc <= 0) break;
				lParam2 -= rc;
				lParam1 += rc;
				total += rc;
			}
#ifdef DEBUG
fprintf(file,"Read rc:%ld total:%ld\n",rc,total);
#endif
         	if (rc < 0) return MMIO_ERROR;
         	return total;
        }
   	}
	break;
	case MMIOM_SEEK: {
        LONG lPosDesired;
		OggVorbis_File *oggfile;
    	vorbis_info *vi;
		
		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct) return MMIO_ERROR;
		if (!pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) return MMIO_ERROR;    	
		
		oggfile = &((DecInfo *)pmmioinfo->pExtraInfoStruct)->oggfile;
		if (READNUM != ((DecInfo *)pmmioinfo->pExtraInfoStruct)->t) return MMIO_ERROR;
		vi = ov_info(oggfile, -1);
		if (!vi) return MMIO_ERROR;
	
		if (SEEK_SET == lParam2) {
     		lPosDesired = lParam1/(2*vi->channels);
        } else if (SEEK_CUR == lParam2) {
            if (0 == lParam1) {
               return ov_pcm_tell(oggfile)*2*vi->channels;
            } /* endif */
           	lPosDesired = ov_pcm_tell(oggfile) + lParam1/(2*vi->channels);
        } else if (SEEK_END == lParam2) {
           	lPosDesired = ov_pcm_total(oggfile,-1) + lParam1/(2*vi->channels);
        } else {
           return MMIO_ERROR;
        }
#ifdef DEBUG
fprintf(file,"Seek to %ld by %d\n",lPosDesired, lParam2);
#endif
		if (ov_pcm_seek(oggfile, lPosDesired) < 0) return MMIO_ERROR;

    	return lPosDesired*2*vi->channels;
	}
    break;

	case MMIOM_CLOSE: {
		int rc;
#ifdef DEBUG
       	    fprintf(file,"start CLOSE\n");
#endif		
		if (!pmmioinfo) return MMIO_ERROR;
		
		if (pmmioinfo->pExtraInfoStruct) {
     		DecInfo *decInfo = (DecInfo *)pmmioinfo->pExtraInfoStruct;
#ifdef DEBUG
       	    fprintf(file,"ready CLOSE\n");
#endif		
			if (READNUM == decInfo->t) {
#ifdef DEBUG
       	    fprintf(file,"read CLOSE\n");
#endif		
     			ov_clear(&decInfo->oggfile);
     			free(decInfo);
     			decInfo = 0;
                rc=MMIO_SUCCESS;
     		} else if (WRITENUM == decInfo->t) {
   	           EncInfo *encInfo = (EncInfo*)pmmioinfo->pExtraInfoStruct;
#ifdef DEBUG
       	    fprintf(file,"write CLOSE\n");
#endif		
			   if (encInfo->headerSet) {
	               vorbis_analysis_wrote(&encInfo->vd,0);
 	               rc = oggWrite(encInfo,1);
	               ogg_stream_clear(&encInfo->os);
	               vorbis_block_clear(&encInfo->vb);
	               vorbis_dsp_clear(&encInfo->vd);
 	               vorbis_comment_clear(&encInfo->vc);
	               vorbis_info_clear(&encInfo->vi);
               }
   	           mmioClose(encInfo->hmmioSS, 0);
  	           free(encInfo);
 	           encInfo = 0;
               rc = MMIO_SUCCESS;
	        } else rc = MMIO_ERROR;
     		pmmioinfo->pExtraInfoStruct = 0;
#ifdef DEBUG
       	    fprintf(file,"CLOSE\n");
#endif		
			return rc;
        }
     	return MMIO_ERROR;
    }
    break;
	case MMIOM_IDENTIFYFILE: {
    	unsigned char buf[4];
		HMMIO hmmioTemp;
		ULONG ulTempFlags = MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY;
		LONG rc = MMIO_ERROR;

		if (!lParam1 && !lParam2) return MMIO_ERROR;
    	hmmioTemp = (HMMIO)lParam2;
    	if (!hmmioTemp) {
        	hmmioTemp = mmioOpen((PSZ)lParam1, NULL, ulTempFlags);
        }

		if (hmmioTemp) {
			rc = mmioRead(hmmioTemp, buf, 4);
			if (rc == 4 && buf[0] == 'O' && buf[1] == 'g' &&
					buf[2] == 'g' && buf[3] == 'S') {
				rc = MMIO_SUCCESS;
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
    	pmmformatinfo->fccIOProc = FOURCC_Vorbis;
    	pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
    	pmmformatinfo->ulMediaType = MMIO_MEDIATYPE_AUDIO;
    	pmmformatinfo->ulFlags = MMIO_CANREADTRANSLATED 
    	   | MMIO_CANWRITETRANSLATED
           | MMIO_CANSEEKTRANSLATED;
    	strcpy(pmmformatinfo->szDefaultFormatExt, "OGG");
    	pmmformatinfo->ulCodePage = 0;
    	pmmformatinfo->ulLanguage = 0;
    	pmmformatinfo->lNameLength = 21;
    	return MMIO_SUCCESS;
    }
    break;
	case MMIOM_GETFORMATNAME:
		if (lParam2 > 21) { 
     		strcpy((PSZ)lParam1, "Ogg Vorbis");
     		return MMIO_SUCCESS;
     	} else return MMIO_ERROR;
    break;
    case MMIOM_QUERYHEADERLENGTH: return (sizeof (MMAUDIOHEADER));
    break;
	case MMIOM_GETHEADER: {
		OggVorbis_File *oggfile;
		DecInfo *decInfo;
    	PMMAUDIOHEADER mmaudioheader;
    	vorbis_info *vi;
    	
		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct) return 0;
		if (!(pmmioinfo->ulFlags & MMIO_READ)) return 0;
		decInfo = (DecInfo *)pmmioinfo->pExtraInfoStruct;
		oggfile = &decInfo->oggfile;
		if (READNUM != decInfo->t){
            pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return 0;
        }
		
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

#ifdef DEBUG
		fprintf(file,"THERE\n");
#endif

		vi = ov_info(oggfile, -1);
		if (!vi) {
            pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return 0;
        }

		mmaudioheader->ulContentType = MMIO_MIDI_UNKNOWN;
		mmaudioheader->ulMediaType = MMIO_MEDIATYPE_AUDIO;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usFormatTag=DATATYPE_WAVEFORM;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels = vi->channels;
		mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec = vi->rate;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample = 16;
		mmaudioheader->mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec=
			mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels * 
			mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec *
			mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample / 8;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usBlockAlign=
			mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels * 
			mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample / 8;
		mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS=
			(unsigned long)(ov_time_total(oggfile, -1)*1000.0);
		mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes=
		    (unsigned long)ov_pcm_total(oggfile, -1)*
		    mmaudioheader->mmXWAVHeader.WAVEHeader.usBlockAlign;

#ifdef DEBUG
		fprintf(file,"time: %ld size: %ld rate: %ld\n",
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS,
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes,
			mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec);
	
#endif
		if (0 != decInfo->vorbisOptions && VORBIS_COOKIE == decInfo->vorbisOptions->cookie) {
			decInfo->vorbisOptions->nominal_bitrate = ov_bitrate(oggfile, -1);
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.pAdditionalInformation = decInfo->vorbisOptions;
        }
		return (sizeof (MMAUDIOHEADER));
    }
    break;

	case MMIOM_SETHEADER: {
    	EncInfo *encInfo;
    	PMMAUDIOHEADER mmaudioheader;
        int totalout = 0;
        int rc;

		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct) return 0;
		encInfo = (EncInfo*)pmmioinfo->pExtraInfoStruct;
		if (WRITENUM != encInfo->t) return 0;
#ifdef DEBUG
fprintf(file,"write header: %x, %x, %x\n",!(pmmioinfo->ulFlags & MMIO_WRITE),
	encInfo->headerSet, (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) && 
		        !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)));
fprintf(file,"flag: %x, trans: %x\n",pmmioinfo->ulFlags,pmmioinfo->ulTranslate);
#endif
		if (/*!(pmmioinfo->ulFlags & MMIO_WRITE) ||*/ encInfo->headerSet
		    || (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) && 
		        !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))) return 0;
		if (!lParam1) {
     		pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
     		return 0;
     	}
     	mmaudioheader = (PMMAUDIOHEADER)lParam1;
     	if (lParam2 != sizeof(MMAUDIOHEADER)) {
         	pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
         	return 0;
        }
		  /********** Encode setup ************/
	if (0 != mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample%8) {
      /* Bit-rate must be multiple of 8 */
      return 0;
    }
	encInfo->bitsPerSample=mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample;
#ifdef DEBUG
fprintf(file,"ready to write header: ");
#endif
    vorbis_info_init(&encInfo->vi);
	if (0 == encInfo->vorbisOptions || VORBIS_COOKIE != encInfo->vorbisOptions->cookie) {
#ifdef DEBUG
fprintf(file,"default quality 0.3\n");
#endif
	    rc = vorbis_encode_init_vbr(&encInfo->vi,
	  	mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels,
	  	mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec, 0.3);
    } else {
#ifdef DEBUG
fprintf(file,"bitsPerSample: %d channels: %d samplesPerSec: %ld min: %ld nominal: %ld max: %ld\n",
	encInfo->bitsPerSample,
	  	mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels,
	  	mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec,
	  	encInfo->vorbisOptions->max_bitrate,
	  	encInfo->vorbisOptions->nominal_bitrate,
	  	encInfo->vorbisOptions->min_bitrate);
#endif
	    rc = vorbis_encode_init(&encInfo->vi,
	  	mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels,
	  	mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec,
	  	encInfo->vorbisOptions->max_bitrate,
	  	encInfo->vorbisOptions->nominal_bitrate,
	  	encInfo->vorbisOptions->min_bitrate);
    }

    if (rc) {
#ifdef DEBUG
fprintf(file,"encodeInit failed: %d\n",rc);
#endif
       return 0;
    } /* endif */

  /* add a comment */
  vorbis_comment_init(&encInfo->vc);
  vorbis_comment_add_tag(&encInfo->vc,"ENCODER","mmioVorbis");

  /* set up the analysis state and auxiliary encoding storage */
  vorbis_analysis_init(&encInfo->vd,&encInfo->vi);
  vorbis_block_init(&encInfo->vd,&encInfo->vb);

  /* set up our packet->stream encoder */
  /* pick a random serial number; that way we can more likely build
     chained streams just by concatenation */
  srand(time(NULL));
  ogg_stream_init(&encInfo->os,rand());

  {
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&encInfo->vd,&encInfo->vc,&header,&header_comm,&header_code);
    ogg_stream_packetin(&encInfo->os,&header); /* automatically placed in its own
					 page */
    ogg_stream_packetin(&encInfo->os,&header_comm);
    ogg_stream_packetin(&encInfo->os,&header_code);
	while(1){
		int result=ogg_stream_flush(&encInfo->os,&encInfo->og);
		if(result==0)break;
		result = mmioWrite(encInfo->hmmioSS, encInfo->og.header,encInfo->og.header_len);
        totalout += result;
		if (result!=encInfo->og.header_len) {
	               ogg_stream_clear(&encInfo->os);
	               vorbis_block_clear(&encInfo->vb);
	               vorbis_dsp_clear(&encInfo->vd);
 	               vorbis_comment_clear(&encInfo->vc);
	               vorbis_info_clear(&encInfo->vi);
			return 0;
        }
		result = mmioWrite(encInfo->hmmioSS, encInfo->og.body,encInfo->og.body_len);
        totalout += result;
		if (result!=encInfo->og.body_len) {
	               ogg_stream_clear(&encInfo->os);
	               vorbis_block_clear(&encInfo->vb);
	               vorbis_dsp_clear(&encInfo->vd);
 	               vorbis_comment_clear(&encInfo->vc);
	               vorbis_info_clear(&encInfo->vi);
			return 0;
		}
	}
  }
  		encInfo->headerSet = 1;
  		return totalout;
    }
    break;
	case MMIOM_WRITE: {
    	EncInfo *encInfo;
		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct) return MMIO_ERROR;
		encInfo = (EncInfo*)pmmioinfo->pExtraInfoStruct;
		if (WRITENUM != encInfo->t) return MMIO_ERROR;
		if (!encInfo->headerSet) return MMIO_ERROR;
		if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) {
     		if (!lParam1) return MMIO_ERROR;
     		return mmioWrite(encInfo->hmmioSS, (PVOID)lParam1, lParam2);
     	} else {
            if (lParam2 == 0) {
                vorbis_analysis_wrote(&encInfo->vd,0);
            } else {
                long i;
				int j, k;
                int num;
				float denom = 1;
                signed char *readbuffer = (signed char *)lParam1;
                /* data to encode */
                /* expose the buffer to submit data */
				int sampleSize = encInfo->vi.channels*encInfo->bitsPerSample/8;
				int samples = lParam2/sampleSize;
	            float **buffer=vorbis_analysis_buffer(&encInfo->vd, samples);
#ifdef DEBUG
fprintf(file,"write: %ld\n",lParam2);
#endif

  	            /* :TODO: Work with buffers not a multiple of sampleSize*/
                if (lParam2 % sampleSize != 0) {
#ifdef DEBUG
fprintf(file,"Bad Write Buffer Size\n");
#endif
                    return MMIO_ERROR;
                }
                for(i=0;i<samples;i++){
                   for (j=0;j<encInfo->vi.channels;j++) {
                    num=0; denom=0.5;
					if (encInfo->bitsPerSample<=8) {
        				buffer[j][i]=(float)((unsigned char)(readbuffer[i*sampleSize])-
							(1<<(encInfo->bitsPerSample-1)))/
							(float)(1<<(encInfo->bitsPerSample-1));
					} else {
						for (k=encInfo->bitsPerSample/8;k>0;k--) {
							if (k==encInfo->bitsPerSample/8) {
								num=(readbuffer[i*sampleSize+k-1]);
							} else {
								num=(num<<8)|((0xff)&(int)(readbuffer[i*sampleSize+k-1]));
    		    			}
							denom *= 256.0;
						}
 	                   buffer[j][i]=((float)num)/denom;
					}
                   } /* endfor */
                }
                vorbis_analysis_wrote(&encInfo->vd,i);
            } /* endif */
            if (oggWrite(encInfo,0)>=0) {
                return lParam2;
            } else {
                return MMIO_ERROR;
            } /* endif */
        }
    }
    break;
#ifdef DEBUG
	case MMIOM_TEMPCHANGE: {
        return MMIO_SUCCESS;
    }
    break;
#endif
	}
#ifdef DEBUG	
	fprintf(file,"unexpected command: %x\n",usMsg);
#endif
	return MMIOERR_UNSUPPORTED_MESSAGE;
}

LONG APIENTRY IOProc_Entry(PVOID pmmioStr, USHORT usMsg, LONG lParam1,
                     LONG lParam2) {
    unsigned cw;
    LONG rc;

#ifdef DEBUG	
	openDebugFile();
	fprintf(file,"command: %x\n",usMsg);
	fflush(file);
#endif

    cw = _control87(MCW_EM,MCW_EM);
    rc = IOProc_Entry2(pmmioStr, usMsg, lParam1, lParam2);
//    cw = _control87(cw,MCW_EM);
    return rc;
}
