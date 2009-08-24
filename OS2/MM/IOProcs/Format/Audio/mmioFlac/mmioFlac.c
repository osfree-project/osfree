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
#include <FLAC/seekable_stream_decoder.h>
#include <FLAC/seekable_stream_encoder.h>
#include <FLAC/ordinals.h>
#include <FLAC/format.h>
#include "mmioFLAC.h"

#ifdef DEBUG
static FILE *file;
void openDebugFile() {
   char path[CCHMAXPATH];
   char fileName[CCHMAXPATH+11];
   BOOL rc;
   if (!file) {
      rc = mciQuerySysValue(MSV_WORKPATH, path);
      if (rc) {
          sprintf(fileName,"%s\\mmioFLAC%ld.log",path,time(NULL));
	      file = fopen(fileName,"wb");
//	      file = freopen(fileName,"wb",file);
      } else {
          exit(-1);
      } /* endif */
   }
}
#endif

#define READNUM 0xA64EFDAA
#define WRITENUM 0xDF5FC57F

typedef struct _Mdata {
   	int t; /*Always READNUM */
    HMMIO hmmio;
    FLAC__SeekableStreamDecoder* decoder;
    FLAC__bool eof;
    unsigned sample_rate;
    unsigned channels;
    unsigned bits_per_sample;
    FLAC__uint64 total_samples;
    LONG byteAt;
    signed char *buffer;
    long bufat;
    long bufsize;
    size_t bufend;
} Mdata;

typedef struct _Mencode {
   	int t; /*Always WRITENUM */
    HMMIO hmmio;
    FLAC__SeekableStreamEncoder* encoder;
} Mencode;


FLAC__SeekableStreamDecoderReadStatus mread
    (const FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[],
     unsigned *bytes, void *client_data)
{
    LONG rc;
    Mdata *mdata = client_data;
	if (!buffer || !bytes || !mdata) return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR;
	rc = mmioRead(mdata->hmmio, buffer, *bytes);
#ifdef DEBUG
			fprintf(file,"callback read bytes:%d rc:%ld\n",*bytes,rc);
#endif
    if (rc < 0) {
       return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR;
    } else {
       if (rc < *bytes) {
            mdata->eof = true;
       } /* endif */
       *bytes = rc;
       return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK;
    } /* endif */
}

FLAC__SeekableStreamDecoderSeekStatus mseek
    (const FLAC__SeekableStreamDecoder *decoder, 
    FLAC__uint64 absolute_byte_offset, void *client_data)
{
    LONG rc;
    Mdata *mdata = client_data;
	if (!mdata) return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;
 	rc = mmioSeek(mdata->hmmio, absolute_byte_offset, SEEK_SET);
#ifdef DEBUG
			fprintf(file,"callback seek rc:%ld\n",rc);
#endif
    if (rc < 0) {
       return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;
    } else {
       mdata->eof = (rc != absolute_byte_offset);
       return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK;
    } /* endif */
}

FLAC__SeekableStreamDecoderTellStatus mtell
    (const FLAC__SeekableStreamDecoder *decoder, 
    FLAC__uint64 *absolute_byte_offset, void *client_data)
{
   LONG rc;
   Mdata *mdata = client_data;
   if (!absolute_byte_offset || !mdata) { return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_ERROR; }

   rc = mmioSeek(mdata->hmmio, 0, SEEK_CUR);
#ifdef DEBUG
			fprintf(file,"callback tell rc:%ld\n",rc);
#endif
   if (rc < 0) {
       return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_ERROR;
    } else {
       *absolute_byte_offset = rc;
       return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK;
    } /* endif */
}

FLAC__SeekableStreamDecoderLengthStatus mlength
    (const FLAC__SeekableStreamDecoder *decoder, 
    FLAC__uint64 *stream_length, void *client_data)
{
   LONG rc1,rc2;
   Mdata *mdata = client_data;
   if (!stream_length || !mdata) { return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_ERROR; }

#ifdef DEBUG
			fprintf(file,"callback length\n");
#endif
   rc1 = mmioSeek(mdata->hmmio, 0, SEEK_CUR);
   if (rc1 < 0) {
       return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_ERROR;
   }

   rc2 = mmioSeek(mdata->hmmio, 0, SEEK_END);
   if (rc2 < 0) {
       return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_ERROR;
   }

   if (mmioSeek(mdata->hmmio, rc1, SEEK_SET) < 0) {
       return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_ERROR;
   }

   *stream_length = rc2;
   return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool meof
    (const FLAC__SeekableStreamDecoder *decoder, 
    void *client_data)
{
   Mdata *mdata = client_data;
   if (!mdata) { return true; }
#ifdef DEBUG
			fprintf(file,"callback eof %d:\n",mdata->eof);
#endif
   return (mdata->eof);
}

FLAC__StreamDecoderWriteStatus mwrite
    (const FLAC__SeekableStreamDecoder *decoder, 
    const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
   Mdata *mdata = client_data;
   unsigned i,j;
   size_t newend;

   if (!mdata) { return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;} 

   newend = frame->header.blocksize*frame->header.channels*(frame->header.bits_per_sample/8);
   if (mdata->bufend < newend) {
        if (mdata->buffer) {
            free(mdata->buffer);
            mdata->buffer = 0;
        } /* endif */
        mdata->bufend=newend;
   } /* endif */

   if (!mdata->buffer) {
        mdata->buffer = malloc(mdata->bufend);
        if (!mdata->buffer) { return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;} 
   } /* endif */

#ifdef DEBUG
			fprintf(file,"callback write %ld\n",newend);
			fprintf(file,"size_t: %ld\n",sizeof(size_t));
     fprintf(file,"heap: %d\n",_heapchk());
#endif
   mdata->bufsize=0;
   for(i=0;i<frame->header.blocksize;i++) {
       for(j=0;j<frame->header.channels;j++) {
           mdata->buffer[mdata->bufsize] = (buffer[j][i] & 0xFF);
           if (mdata->bits_per_sample <= 8) {
              mdata->buffer[mdata->bufsize]+=1<<(mdata->bits_per_sample-1);
           }
           mdata->bufsize++;
           if (mdata->bits_per_sample > 8) {
               mdata->buffer[mdata->bufsize] = ((buffer[j][i] & 0xFF00) >> 8);
               mdata->bufsize++;
           } /* endif */
           if (mdata->bits_per_sample > 16) {
               mdata->buffer[mdata->bufsize] = ((buffer[j][i] & 0xFF0000) >> 8);
               mdata->bufsize++;
           } /* endif */
           if (mdata->bits_per_sample > 24) {
               mdata->buffer[mdata->bufsize] = ((buffer[j][i] & 0xFF000000) >> 8);
               mdata->bufsize++;
           } /* endif */
       }
   }
   mdata->bufat=0;
#ifdef DEBUG
     fprintf(file,"write post-heap: %d\n",_heapchk());
#endif
   
   return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void mmetadata
    (const FLAC__SeekableStreamDecoder *decoder, 
    const FLAC__StreamMetadata *metadata, void *client_data)
{ 
    if (!decoder || !metadata || !client_data) return;
#ifdef DEBUG
			fprintf(file,"callback metadata\n");
#endif
    if (FLAC__METADATA_TYPE_STREAMINFO == metadata->type) {
        Mdata *mdata = (Mdata *)client_data;
        mdata->sample_rate = metadata->data.stream_info.sample_rate;
        mdata->channels = metadata->data.stream_info.channels;
        mdata->bits_per_sample = metadata->data.stream_info.bits_per_sample;
        mdata->total_samples = metadata->data.stream_info.total_samples;
    } /* endif */
}

void merror(const FLAC__SeekableStreamDecoder *decoder, 
    FLAC__StreamDecoderErrorStatus status, void *client_data)
{
#ifdef DEBUG
  fprintf(file, "merror: %d\n",status);
     fprintf(file,"heap: %d\n",_heapchk());
#endif
 return; 
}

FLAC__StreamEncoderWriteStatus encodeWrite
	(const FLAC__SeekableStreamEncoder *encoder, const FLAC__byte buffer[], 
	 unsigned bytes, unsigned samples, unsigned current_frame, void *client_data)
{
    LONG rc;
    Mencode *mencode = client_data;
	if (!buffer || !mencode) return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR;
	rc = mmioWrite(mencode->hmmio, buffer, bytes);
#ifdef DEBUG
			fprintf(file,"callback write bytes:%d rc:%ld\n",bytes,rc);
#endif
    if (rc != bytes) {
       return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
    } else {
       return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    } /* endif */
}

FLAC__SeekableStreamEncoderSeekStatus encodeSeek
	(const FLAC__SeekableStreamEncoder *encoder, 
	 FLAC__uint64 absolute_byte_offset, void *client_data) {
    LONG rc;
    Mencode *mencode = client_data;
	if (!mencode) return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR;
	rc = mmioSeek(mencode->hmmio, absolute_byte_offset, SEEK_SET);
#ifdef DEBUG
			fprintf(file,"encode seek callback:%lld rc:%ld\n",absolute_byte_offset,rc);
#endif
    if (absolute_byte_offset != rc) {
       return FLAC__SEEKABLE_STREAM_ENCODER_SEEK_STATUS_ERROR;
    } else {
       return FLAC__SEEKABLE_STREAM_ENCODER_SEEK_STATUS_OK;
    } /* endif */
}

LONG APIENTRY IOProc_Entry(PVOID pmmioStr, USHORT usMsg, LONG lParam1,
                     LONG lParam2) {
	PMMIOINFO pmmioinfo = (PMMIOINFO)pmmioStr;
#ifdef DEBUG
     openDebugFile();
     fprintf(file,"command: %x\n",usMsg);
     fprintf(file,"heap: %d\n",_heapchk());
     fflush(file);
#endif
	switch (usMsg) {
	case MMIOM_OPEN:
		{	
     		HMMIO hmmioSS;
     		MMIOINFO mmioinfoSS;
	    	PSZ pszFileName = (char *)lParam1;
   	 		if (!pmmioinfo) return MMIO_ERROR;
            if (!(pmmioinfo->ulFlags & MMIO_READ || pmmioinfo->ulFlags & MMIO_WRITE)) return MMIO_ERROR;
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
			if (mmioinfoSS.ulFlags & MMIO_WRITE) {
				mmioinfoSS.ulFlags ^= MMIO_WRITE;
				mmioinfoSS.ulFlags |= MMIO_READWRITE;
      		}
			
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
	     		Mdata *mdata;
#ifdef DEBUG
			fprintf(file,"File Read: %s\n",pszFileName);
#endif
	       		mdata = (Mdata *) calloc(1,sizeof(Mdata));
 	      		if (!mdata) {
  	          		mmioClose(hmmioSS, 0);
            		return MMIO_ERROR;
	            }
				mdata->t = READNUM;
	            mdata->hmmio = hmmioSS;
 	      		pmmioinfo->pExtraInfoStruct = (PVOID)mdata;
  	     		{
                    FLAC__bool rc = true;
            		mdata->decoder = FLAC__seekable_stream_decoder_new();
	       			if(!mdata->decoder) {
           	  			free(mdata);
	            		mmioClose(hmmioSS, 0);
 	           			return MMIO_ERROR;
             		}
   	         		if (!FLAC__seekable_stream_decoder_set_read_callback(mdata->decoder, mread)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_decoder_set_seek_callback(mdata->decoder, mseek)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_decoder_set_tell_callback(mdata->decoder, mtell)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_decoder_set_length_callback(mdata->decoder, mlength)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_decoder_set_eof_callback(mdata->decoder, meof)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_decoder_set_write_callback(mdata->decoder, mwrite)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_decoder_set_metadata_callback(mdata->decoder, mmetadata)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_decoder_set_error_callback(mdata->decoder, merror)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_decoder_set_client_data(mdata->decoder,mdata)) {
                        rc = false; } else
   	         		if (FLAC__SEEKABLE_STREAM_DECODER_OK != FLAC__seekable_stream_decoder_init(mdata->decoder)) {
                        rc = false; } else
#ifdef DEBUG
			fprintf(file,"open here rc:%d\n",rc);
#endif
                    if (!FLAC__seekable_stream_decoder_process_until_end_of_metadata(mdata->decoder))  { 
                        rc = false; }
	       			if(!rc) {
                        FLAC__seekable_stream_decoder_delete (mdata->decoder);
           	  			free(mdata);
	            		mmioClose(hmmioSS, 0);
 	           			return MMIO_ERROR;
             		}
            	}
#ifdef DEBUG
				fprintf(file,"Open successfull\n");
#endif
				return MMIO_SUCCESS;
			} else if (pmmioinfo->ulFlags & MMIO_WRITE) {
	     		Mencode *mencode;
#ifdef DEBUG
			fprintf(file,"File Write: %s\n",pszFileName);
#endif
	       		mencode = (Mencode *) calloc(1,sizeof(Mencode));
 	      		if (!mencode) {
  	          		mmioClose(hmmioSS, 0);
            		return MMIO_ERROR;
	            }
				mencode->t = WRITENUM;
	            mencode->hmmio = hmmioSS;
 	      		pmmioinfo->pExtraInfoStruct = (PVOID)mencode;
  	     		{
                    FLAC__bool rc = true;
            		mencode->encoder = FLAC__seekable_stream_encoder_new();
	       			if(!mencode->encoder) {
           	  			free(mencode);
	            		mmioClose(hmmioSS, 0);
 	           			return MMIO_ERROR;
             		}
   	         		if (!FLAC__seekable_stream_encoder_set_write_callback(mencode->encoder,encodeWrite )) {
                       rc = false; } else
   	         		if (!FLAC__seekable_stream_encoder_set_seek_callback(mencode->encoder,encodeSeek)) {
                        rc = false; } else
   	         		if (!FLAC__seekable_stream_encoder_set_client_data(mencode->encoder,mencode)) {
                        rc = false; } else
//		if (FLAC__SEEKABLE_STREAM_ENCODER_OK != FLAC__seekable_stream_encoder_init(mencode->encoder))
//  			return MMIO_ERROR;
#ifdef DEBUG
			fprintf(file,"open here rc:%d\n",rc);
#endif
	       			if(!rc) {
                        FLAC__seekable_stream_encoder_delete (mencode->encoder);
           	  			free(mencode);
	            		mmioClose(hmmioSS, 0);
 	           			return MMIO_ERROR;
             		}
            	}
#ifdef DEBUG
				fprintf(file,"Open successfull\n");
#endif
				return MMIO_SUCCESS;
      		}
      		return MMIO_ERROR;
		}
	    break;                                                 
	case MMIOM_READ: {
		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct || !lParam1) return MMIO_ERROR;
		
		if (!pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) {
     		return MMIO_ERROR;
     	} else {
    		Mdata *mdata;
            LONG total = 0;
    
      		mdata = (Mdata*)pmmioinfo->pExtraInfoStruct;
			if (READNUM != mdata->t) return MMIO_ERROR;

            while (1) {
                if (total==lParam2) {
#ifdef DEBUG
			fprintf(file,"decoded all %ld %ld %ld\n",total, mdata->bufat, mdata->bufsize);
     fprintf(file,"heap: %d\n",_heapchk());
#endif
                    return total;
                } /* endif */
                if (mdata->bufsize==mdata->bufat) {
                    FLAC__bool rc;
#ifdef DEBUG
			fprintf(file,"process single, %d\n",FLAC__seekable_stream_decoder_get_state(mdata->decoder));
     fprintf(file,"heap: %d\n",_heapchk());
#endif
                    rc = FLAC__seekable_stream_decoder_process_single(mdata->decoder);
#ifdef DEBUG
     fprintf(file,"post-heap: %d\n",_heapchk());
#endif
                    if (false==rc) {
#ifdef DEBUG
			fprintf(file,"unexpected state:%d\n",FLAC__seekable_stream_decoder_get_state(mdata->decoder));
#endif
                        if (FLAC__STREAM_DECODER_END_OF_STREAM==FLAC__seekable_stream_decoder_get_state(mdata->decoder)) {
                            return total;
                        } /* endif */
                        return MMIO_ERROR;
                    } /* endif */
                } /* endif */
                if (mdata->bufsize==mdata->bufat) {
#ifdef DEBUG
			fprintf(file,"failed to get more data.\n");
#endif
                    return total;
                }
                if (lParam2-total >= mdata->bufsize-mdata->bufat) {
                    memcpy((PSZ)lParam1+total,mdata->buffer+mdata->bufat,mdata->bufsize-mdata->bufat);
                    total+=mdata->bufsize-mdata->bufat;
                    mdata->byteAt+=mdata->bufsize-mdata->bufat;
                    mdata->bufat=mdata->bufsize;
                } else {
                    memcpy((PSZ)lParam1+total,mdata->buffer+mdata->bufat,lParam2-total);
                    mdata->bufat+=lParam2-total;
                    mdata->byteAt+=lParam2-total;
                    total=lParam2;
                }
            } /* endwhile */
        }
   	}
	break;
	case MMIOM_SEEK: {
    		Mdata *mdata;
            LONG posDesired;
            FLAC__bool rc;

    		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct) return MMIO_ERROR;
    		if (!pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) return MMIO_ERROR;    	
    
      		mdata = (Mdata*)pmmioinfo->pExtraInfoStruct;
			if (READNUM != mdata->t) return MMIO_ERROR;
            
            if (SEEK_END == lParam2) {
                posDesired = mdata->total_samples-1;
            } else if (SEEK_CUR == lParam2) {
                posDesired = mdata->byteAt/(mdata->bits_per_sample*mdata->channels/8);
            } else {
                posDesired = 0;
            }

            posDesired += lParam1/(mdata->bits_per_sample*mdata->channels/8);

            rc = FLAC__seekable_stream_decoder_seek_absolute (mdata->decoder, posDesired);

            if (rc) {
                mdata->byteAt=posDesired*(mdata->bits_per_sample*mdata->channels/8);
                mdata->bufsize = 0;
                mdata->bufat = 0;
                return mdata->byteAt;
            } else {
                return MMIO_ERROR;
            }
	}
    break;

	case MMIOM_CLOSE: {
		Mdata *mdata;
        HMMIO hmmio;
		if (!pmmioinfo) return MMIO_ERROR;
		
  		mdata = (Mdata*)pmmioinfo->pExtraInfoStruct;
		if (mdata && READNUM==mdata->t) {
            hmmio = mdata->hmmio;
#ifdef DEBUG
       	    fprintf(file,"pre CLOSE\n");
#endif		
            if (mdata->decoder) {
//                FLAC__seekable_stream_decoder_finish (mdata->decoder);
                FLAC__seekable_stream_decoder_delete (mdata->decoder);
            }
#ifdef DEBUG
       	    fprintf(file,"mid CLOSE\n");
#endif		
            if (mdata->buffer) free(mdata->buffer);
            mdata->buffer = 0;
           	free(mdata);
            pmmioinfo->pExtraInfoStruct = 0;
            mdata = 0;
#ifdef DEBUG
       	    fprintf(file,"write CLOSE\n");
#endif		
	        return mmioClose(hmmio, 0);
        }
		if (mdata && WRITENUM==mdata->t) {
			Mencode *mencode;
	  		mencode = (Mencode*)pmmioinfo->pExtraInfoStruct;
            hmmio = mencode->hmmio;
#ifdef DEBUG
       	    fprintf(file,"pre CLOSE\n");
			mencode->t=0;
#endif		
            if (mencode->encoder) {
//                FLAC__seekable_stream_encoder_finish (mencode->encoder);
                FLAC__seekable_stream_encoder_delete (mencode->encoder);
            }
#ifdef DEBUG
       	    fprintf(file,"mid CLOSE\n");
#endif		
            pmmioinfo->pExtraInfoStruct = 0;
            mencode = 0;
#ifdef DEBUG
       	    fprintf(file,"write CLOSE\n");
#endif		
	        return mmioClose(hmmio, 0);
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
			if (rc == 4 && buf[0] == 'f' && buf[1] == 'L' &&
					buf[2] == 'a' && buf[3] == 'C') {
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
    	pmmformatinfo->fccIOProc = FOURCC_FLAC;
    	pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
    	pmmformatinfo->ulMediaType = MMIO_MEDIATYPE_AUDIO;
    	pmmformatinfo->ulFlags = MMIO_CANREADTRANSLATED | MMIO_CANWRITETRANSLATED
           | MMIO_CANSEEKTRANSLATED;
    	strcpy(pmmformatinfo->szDefaultFormatExt, "FLA");
    	pmmformatinfo->ulCodePage = 0;
    	pmmformatinfo->ulLanguage = 0;
    	pmmformatinfo->lNameLength = 6;
    	return MMIO_SUCCESS;
    }
    break;
	case MMIOM_GETFORMATNAME:
		if (lParam2 > 6) { 
     		strcpy((PSZ)lParam1, "FLAC");
     		return MMIO_SUCCESS;
     	} else return MMIO_ERROR;
    break;
    case MMIOM_QUERYHEADERLENGTH: return (sizeof (MMAUDIOHEADER));
    break;
	case MMIOM_GETHEADER: {
		Mdata *mdata;
    	PMMAUDIOHEADER mmaudioheader;
    	
		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct) return 0;
		if (!(pmmioinfo->ulFlags & MMIO_READ)) return 0;
		mdata = (Mdata*)pmmioinfo->pExtraInfoStruct;
        if (READNUM != mdata->t) return MMIO_ERROR;
		if (!mdata->decoder) {
            pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return 0;
        }
		
		if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) &&
		    !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return 0;
		mmaudioheader = (PMMAUDIOHEADER)lParam1;

		if (sizeof(MMAUDIOHEADER) > lParam2) {
            pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
			return 0;
		}
		mmaudioheader->ulContentType = MMIO_MIDI_UNKNOWN;
		mmaudioheader->ulMediaType = MMIO_MEDIATYPE_AUDIO;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usFormatTag=DATATYPE_WAVEFORM;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels = mdata->channels;
		mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec = mdata->sample_rate;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample =  mdata->bits_per_sample;
		mmaudioheader->mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec=
			mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels * 
			mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec *
			mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample / 8;
		mmaudioheader->mmXWAVHeader.WAVEHeader.usBlockAlign=
			mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels * 
			mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample / 8;
		mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes=
            mdata->total_samples*
            mmaudioheader->mmXWAVHeader.WAVEHeader.usBlockAlign;
        mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS=
            (mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes)/
            (mmaudioheader->mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec/1000);

#ifdef DEBUG
		fprintf(file,"time: %ld size: %ld rate: %ld\n",
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS,
			mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes,
			mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec);
#endif
		return (sizeof (MMAUDIOHEADER));
    }
	case MMIOM_SETHEADER: {
		Mencode *mencode;
    	PMMAUDIOHEADER mmaudioheader;
		int rc;
    	
		if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct) return 0;
		if (!(pmmioinfo->ulFlags & MMIO_WRITE)) return 0;
		mencode = (Mencode*)pmmioinfo->pExtraInfoStruct;
        if (WRITENUM != mencode->t) return MMIO_ERROR;
		if (!mencode->encoder) {
            pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return 0;
        }
		
		if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) &&
		    !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return 0;
		mmaudioheader = (PMMAUDIOHEADER)lParam1;

		if (sizeof(MMAUDIOHEADER) > lParam2 || sizeof(MMAUDIOHEADER) > mmaudioheader->ulHeaderLength) {
            pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
			return 0;
		}
#ifdef DEBUG
fprintf(file,"begin write header channels: %d bps: %d rate:%ld estimate: %ld.\n",
mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels,
mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample,
mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec,
mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
	(mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels*mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample/8));
#endif

		if (!FLAC__seekable_stream_encoder_set_streamable_subset(mencode->encoder, 
				false)) 
			return 0;
		if (!FLAC__seekable_stream_encoder_set_channels(mencode->encoder, 
				mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels)) 
			return 0;
		if (!FLAC__seekable_stream_encoder_set_bits_per_sample(mencode->encoder, 
				mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample)) 
			return 0;
		if (!FLAC__seekable_stream_encoder_set_sample_rate(mencode->encoder, 
				mmaudioheader->mmXWAVHeader.WAVEHeader.ulSamplesPerSec)) 
			return 0;
#if 0
		if (!FLAC__seekable_stream_encoder_set_total_samples_estimate(mencode->encoder, 
				mmaudioheader->mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
					(mmaudioheader->mmXWAVHeader.WAVEHeader.usChannels*mmaudioheader->mmXWAVHeader.WAVEHeader.usBitsPerSample/8))) 
			return 0;
#endif

#ifdef DEBUG
fprintf(file,"header set %d.\n",FLAC__seekable_stream_encoder_get_state(mencode->encoder));
#endif
		rc = FLAC__seekable_stream_encoder_init(mencode->encoder);
#ifdef DEBUG
fprintf(file,"initalized %d : %d.\n", rc, FLAC__seekable_stream_encoder_get_stream_encoder_state(mencode->encoder));
#endif
		if (FLAC__SEEKABLE_STREAM_ENCODER_OK != rc) {
  			return 0;
		}
		return (sizeof (MMAUDIOHEADER));
    }
	case MMIOM_WRITE: {
    	  Mencode *mencode;
          if (!pmmioinfo || !pmmioinfo->pExtraInfoStruct) return MMIO_ERROR;
          mencode = (Mencode*)pmmioinfo->pExtraInfoStruct;
          if (WRITENUM != mencode->t) return MMIO_ERROR;
          if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) {
               if (!lParam1) return MMIO_ERROR;
               return mmioWrite(mencode->hmmio, (PVOID)lParam1, lParam2);
          } else {
             	int sampleSize =  
	FLAC__seekable_stream_encoder_get_bits_per_sample(mencode->encoder)/8;
				int samples;
				int i, j;
				FLAC__int32   	  *buf = 0;
                /* :TODO: Work with buffers not a multiple of sampleSize*/
                if (lParam2 % sampleSize != 0) {
#ifdef DEBUG
fprintf(file,"Bad Write Buffer Size, %ld : %d\n",lParam2, sampleSize);
#endif
                    return MMIO_ERROR;
                }
				samples = lParam2/sampleSize;
				buf = (FLAC__int32*)malloc(sizeof(FLAC__int32)*lParam2);
				if (0==buf) return MMIO_ERROR;
#ifdef DEBUG
fprintf(file,"buffer allocated %ld\n", lParam2);
#endif
				if (FLAC__seekable_stream_encoder_get_bits_per_sample(mencode->encoder)<=8) {
					for(i=0; i<lParam2; i++){
 	      				buf[i]=((unsigned char *)lParam1)[i]-(1<<(FLAC__seekable_stream_encoder_get_bits_per_sample(mencode->encoder)-1));
  	     			}
				} else {
					for(i=0; i<samples; i++){
        				buf[i]=((signed char *)lParam1)[i*sampleSize+sampleSize-1];
        				for (j=sampleSize-2; j>=0; j--) {
               				buf[i]<<=8;
	 	      				buf[i]|=((unsigned char *)lParam1)[i*sampleSize+j];
						}
  	     			}
				}
#ifdef DEBUG
fprintf(file,"Ready to write %d samples.\n",samples);
#endif
			if (!FLAC__seekable_stream_encoder_process_interleaved(mencode->encoder, buf, samples/FLAC__seekable_stream_encoder_get_channels(mencode->encoder))) {
      			free(buf);
				return MMIO_ERROR;
			} else {
      			free(buf);
      			return lParam2;
      		}
          }
    }
    break;
	}
#ifdef DEBUG	
	fprintf(file,"unexpected command: %x\n",usMsg);
#endif
	return MMIOERR_UNSUPPORTED_MESSAGE;
}

