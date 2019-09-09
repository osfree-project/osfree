#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <os2medef.h>
#include <mmioos2.h>
#include "gbm.h"

#pragma pack(2)

static char     pszGIFExt  [] = "GIF";

typedef RGB FAR *PRGB;


/****************************************
 * IOProc information structure, used for every file opened
 *    by this IOProc
 ****************************************/
typedef struct _GIFFILESTATUS
{
  GBM gbm;
  int ft;  /* GBM index for the proc to use */
  int fHandleGBM;

  PBYTE   lpRGBBuf;               /* 24-bit RGB Buf for translated data */
  ULONG   ulRGBTotalBytes;        /* Length of 24-bit RGBBuf       */

  LONG    lImgBytePos;            /* Current pos in RGB buf        */

  ULONG   ulImgTotalBytes;
  BOOL    bSetHeader;             /* TRUE if header set in WRITE mode*/

  MMIMAGEHEADER   mmImgHdr;       /* Standard image header         */
} GIFFILESTATUS;
typedef GIFFILESTATUS FAR *PGIFFILESTATUS;

#define     MMOTION_HEADER_SIZE         sizeof (MMOTIONHEADER)
#define     ONE_BLOCK                   6L
#define     FOURCC_GIF                 mmioFOURCC('G', 'I', 'F', 'C')

/* RC file defines */

#define     HEX_FOURCC_GIF               0x434D4547L

#define     MMOTION_IOPROC_NAME_TABLE   7000
#define     MMOTION_NLS_CHARSET_INFO    7500

#ifndef BITT_NONE
#define BITT_NONE 0
#define BI_NONE   0
#endif

/**********************
 * Function Declarations
 **********************/
LONG EXPENTRY IOProc_Entry (PVOID  pmmioStr, USHORT usMsg,
                                   LONG   lParam1, LONG lParam2);

#pragma pack()
