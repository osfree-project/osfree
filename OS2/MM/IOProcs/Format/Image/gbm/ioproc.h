#define INCL_32                         /* force 32 bit compile */
#define INCL_GPIBITMAPS
#define INCL_DOSFILEMGR
#define INCL_WIN
#define INCL_GPI
#define INCL_PM

/************************************************************************/
/* Put all #includes here                                               */
/************************************************************************/

#include <os2.h>

#define INCL_MMIOOS2
#include <os2me.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gbm.h>

#pragma pack(2)

/****************************************
 * IOProc information structure, used for every file opened
 *    by this IOProc
 ****************************************/
typedef struct _GBMFILESTATUS
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

/**********************
 * Function Declarations
 **********************/
LONG EXPENTRY IOProc_Entry (PVOID  pmmioStr, USHORT usMsg,
                                   LONG   lParam1, LONG lParam2);

#pragma pack()
