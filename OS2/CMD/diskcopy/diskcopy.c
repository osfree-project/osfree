#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define INCL_BASE
#define INCL_DOSDEVIOCTL
#include <os2.h>

/* ------------------------------------------------------------------------ */

#define READ_SOURCE 1
#define COPY_TARGET 2
#define EXIT_DSKCPY -1

#define DSKCPY_ERROR_WRONG_FORMAT 0xffbf
// #define DSKCPY_ERROR_CANT_FORMAT  0xffbe  /* Remove this definition when */
                                             /* the format problem is fixed */

#define BUFSIZE   1024
#define OPENFLAGS (OPEN_FLAGS_DASD | OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE)

  /* Note, for the lockdrive/unlockdrive macros, the global variable _lockCmd
  **  must be accessable and set to zero!
  */
#define lockdrive(hf)   (_DosError = DosDevIOCtl(0L, &_lockCmd, DSK_LOCKDRIVE,   IOCTL_DISK, hf))
#define unlockdrive(hf) (_DosError = DosDevIOCtl(0L, &_lockCmd, DSK_UNLOCKDRIVE, IOCTL_DISK, hf))


/* --------------------------------------------------------------------------
** This structure is used instead of the BIOSPARAMETERBLOCK because the
** definition of that structure in some copies of bsedev.h is wrong!  It
** seems to be missing the BYTE abReserved[6] field.
*/
typedef struct _BSPBLK
  {
  USHORT usBytesPerSector;
  BYTE   bSectorsPerCluster;
  USHORT usReservedSectors;
  BYTE   cFATs;
  USHORT cRootEntries;
  USHORT cSectors;
  BYTE   bMedia;
  USHORT usSectorsPerFAT;
  USHORT usSectorsPerTrack;
  USHORT cHeads;
  ULONG  cHiddenSectors;
  ULONG  cLargeSectors;
  BYTE   abReserved[6];
  USHORT cCylinders;
  BYTE   bDeviceType;
  USHORT fsDeviceAttr;
  } BSPBLK;


/* Global variables ------------------------------------------------------ */

USHORT       _DosError = 0;       /* Most recent error code             */
BYTE         _lockCmd  = 0;       /* Used with [un]lockdrive macros     */
ULONG        _fmtData  = 0L;      /* Used with DSK_FORMATVERIFY         */
                                  /*  (kudos to RMK!)                   */

BSPBLK       sourceParms;         /* Param block for source drive       */
PBYTE        *sourceBuffer;       /* Array of pointers to track buffers */
ULONG        sourceBytes;         /* # bytes on source disk             */
USHORT       sourceTracks;        /* # tracks on source disk            */
USHORT       bytesPerTrack;       /* Bytes per track on source disk     */
PTRACKLAYOUT sourceLayout;        /* Pointer to track layout table      */
USHORT       sizeofLayoutElement; /* Total size of layout table         */
int          gotSource = FALSE;   /* Bool: Source disk has been read    */

BSPBLK       targetParms;         /* Param block for target disk        */


/* Prototypes ------------------------------------------------------------ */

void  copyr(void);
int   query(char *fmt, ... );
HFILE opendrive(char *drive);
int   readsource(HFILE hf);
BYTE  fmttbl_bytessec(USHORT bytesPerSec);
int   writetarget(HFILE hf);
int   bspblkcmp(BSPBLK *blk1, BSPBLK *blk2);
void  *Alloc(unsigned num, unsigned size);
void  errorexit(HFILE hf);
int   dskcpy_menu(int mlevel, char *drive);
int   main(int argc, char **argv);


/* Code ------------------------------------------------------------------ */

  /* -- Logon message --- */
void copyr()
  {

  puts("DskCpy2 -- Alternative disk copier for OS/2");
  puts("Brady Flowers, Mankato, MN\n");
  }



  /* --- Print a formatted string, get a key and return the keycode --- */
int query(char *fmt, ... )
  {
  int ch;
  va_list args;

  va_start(args, fmt);
  vprintf(fmt, args);
  ch = getch();
  puts("");
  return ch;
  }



  /* --- Open disk drive ---
  **       parameter is asciiz drive specifier, i.e., "a:"
  **       returns handle to open drive or 0, sets/clears _DosError
  */
HFILE opendrive(char *drive)
  {
  USHORT result;
  HFILE  dHandle;

  if ((strlen(drive) != 2) || (drive[1] != ':'))
    _DosError = ERROR_INVALID_DATA;
  else 
    do
      {
      _DosError = DosOpen(drive, &dHandle, &result, 0L, 0, FILE_OPEN, OPENFLAGS, 0L);
      if (_DosError == ERROR_NOT_READY)
        {
        if ((query("Please place a disk in drive %s and strike any key (ESC to end)..", drive)) == 0x1b)
          break;
        }
      else
        break;
      }
    while (TRUE);

  return _DosError ? 0 : dHandle;
  }



  /* --- Read source disk into memory ---
  **       parameter is drive handle as returned from opendrive()
  **       reads the entire source disk into memory allocating as it goes,
  **        when done sourceBuffer points to an array of buffer pointers,
  **        one for each track on the disk and each the the size of a track
  **        in bytes.
  **       sets global variables:
  **        gotSource
  **        sourceBytes
  **        sourceTracks
  **        bytesPerTrack
  **        sizeofLayoutElement
  **        sourceBuffer
  **        sourceLayout
  **        _DosError
  **       returns 0 if success else error code (_DosError)
  */
int readsource(HFILE hf)
  {
  BYTE _parmCmd = 1;
  int  trk, hd, cyl;

  /* If this isn't the first time here, free memory from last time first */
  if (gotSource)
    {
    for (trk = 0; trk < sourceTracks; trk++)
      free(sourceBuffer[trk]);
    free(sourceBuffer);
    free(sourceLayout);
    sourceBuffer = NULL;
    sourceLayout = NULL;
    gotSource = FALSE;
    }

  /* Get source disk parameters */
  _DosError = DosDevIOCtl(&sourceParms, &_parmCmd, DSK_GETDEVICEPARAMS, IOCTL_DISK, hf);
  if (!_DosError)
    {
    /* Set all the informational variables and build a track layout table
    **  for use with the following sector reads.
    */
    sourceBytes   = (ULONG)(sourceParms.usBytesPerSector) *
                    (ULONG)(sourceParms.cSectors);
    sourceTracks  = sourceParms.cSectors         /
                    sourceParms.usSectorsPerTrack;
    bytesPerTrack = sourceParms.usBytesPerSector *
                    sourceParms.usSectorsPerTrack;

    sizeofLayoutElement = sizeof(TRACKLAYOUT)   +
                          ((2 * sizeof(USHORT)) *
                          (sourceParms.usSectorsPerTrack - 1));

    if (sourceLayout = (PTRACKLAYOUT)Alloc(sizeofLayoutElement, sizeof(BYTE)))
      {
      sourceLayout->bCommand = 1;
      sourceLayout->usFirstSector = 0;
      sourceLayout->cSectors = sourceParms.usSectorsPerTrack;
      for (trk = 0; trk < sourceParms.usSectorsPerTrack; trk++)
        {
        sourceLayout->TrackTable[trk].usSectorNumber = trk+1;
        sourceLayout->TrackTable[trk].usSectorSize = sourceParms.usBytesPerSector;
        }
      }
    else
      errorexit(hf);

    /* Allocate the array of BYTE pointers to hold the track data */
    if ((sourceBuffer = (PBYTE *)Alloc(sourceTracks, sizeof(PBYTE))) == NULL)
      errorexit(hf);

    printf("Reading %d cylinders, %d heads, %d sectors, %d bytes per sector\n",
           sourceTracks / sourceParms.cHeads, sourceParms.cHeads,
           sourceParms.usSectorsPerTrack,     sourceParms.usBytesPerSector);

    /* For each track, allocate a buffer and read the sector into it */
    for (trk = 0, cyl = 0; trk < sourceTracks; trk += sourceParms.cHeads, cyl++)
      {
      sourceLayout->usCylinder = cyl;
      for (hd = 0; hd < sourceParms.cHeads; hd++)
        {
        printf("\rCylinder %d, Head %d", cyl, hd);
        sourceLayout->usHead = hd;
        if ((sourceBuffer[trk+hd] = (PBYTE)Alloc(bytesPerTrack, sizeof(BYTE))) == NULL)
          errorexit(hf);
        if (_DosError = DosDevIOCtl(sourceBuffer[trk+hd], sourceLayout, DSK_READTRACK, IOCTL_DISK, hf))
          errorexit(hf);
        }
      }

    puts("\rDone.                 \n");
    gotSource = TRUE;
    }

  return _DosError;
  }



  /* --- Translate bytes per sector into 0-3 code ---
  **       the four sector sizes listed below are alluded to in the OS/2
  **        docs however only 512 byte sectors are allowed under OS/2 1.x
  **       returns the code or -1 and sets _DosError
  */
BYTE fmttbl_bytessec(USHORT bytesPerSec)
  {

  _DosError = NO_ERROR;
  switch (bytesPerSec)
    {
    case 128:  return 0;
    case 256:  return 1;
    case 512:  return 2;
    case 1024: return 3;
    }
  _DosError = ERROR_BAD_FORMAT;
  return -1;
  }



  /* --- write information read by readsource() onto target disk ---
  **       parameter is drive handle as returned by opendrive()
  **       checks the target disk, if it's the same format as the source
  **        or not formatted at all, write the information contained in
  **        sourceBuffer formatting if neccessary.
  **       returns 0 if successful else errorcode (_DosError)
  **
  **  -------------------------------------------------------------------
  **    NOTE: THIS ROUTINE, AS WRITTEN WORKS FINE IF IT DOESN'T HAVE TO
  **          FORMAT THE DISK.  IT FAILS ON THE FIRST CALL TO DosDevIOCtl
  **          WITH THE DSK_FORMATVERIFY PARAMETER.
  **  -------------------------------------------------------------------
  */
int writetarget(HFILE hf)
  {
  BYTE         _parmCmd = 1;
  PTRACKFORMAT trkfmt;
  USHORT       sizeofTrkfmt;
  int          i, trk, hd, cyl, needFormat = FALSE;

  /* Get target disk parameters */
  _DosError = DosDevIOCtl(&targetParms, &_parmCmd, DSK_GETDEVICEPARAMS, IOCTL_DISK, hf);

  if (_DosError == ERROR_READ_FAULT)
    {
#if defined (DSKCPY_ERROR_CANT_FORMAT)
    _DosError = DSKCPY_ERROR_CANT_FORMAT;
#else
    /* If the disk needs formatting we build a format table for it based
    **  on the source disk.
    */
    needFormat = TRUE;
    _DosError = 0;
    sizeofTrkfmt = sizeof(TRACKFORMAT) +
                    ((4 * sizeof(BYTE)) *
                    (sourceParms.usSectorsPerTrack - 1));
    if ((trkfmt = (PTRACKFORMAT)Alloc(sizeofTrkfmt, sizeof(BYTE))) == NULL)
      errorexit(hf);
    trkfmt->bCommand = 1;
    trkfmt->cSectors = sourceParms.usSectorsPerTrack;
    for (trk = 0; trk < trkfmt->cSectors; trk++)
      {
      trkfmt->FormatTable[trk].idSector = (BYTE)(trk+1);
      trkfmt->FormatTable[trk].bBytesSector = fmttbl_bytessec(sourceParms.usBytesPerSector);
      }
#endif
    }
  else if (!_DosError)
    /* Else if no other error, make sure that the target disk is the same
    **  format as the source.
    */
    if (bspblkcmp(&sourceParms, &targetParms))
      _DosError = DSKCPY_ERROR_WRONG_FORMAT;


  if (!_DosError)
    {
    printf("Writing %d cylinders, %d heads, %d sectors, %d bytes per sector\n",
           sourceTracks / sourceParms.cHeads, sourceParms.cHeads,
           sourceParms.usSectorsPerTrack,     sourceParms.usBytesPerSector);
    if (needFormat)
      puts("Formatting while copying.");

    for (trk = 0, cyl = 0; trk < sourceTracks; trk += sourceParms.cHeads, cyl++)
      {
      sourceLayout->usCylinder = cyl;
      for (hd = 0; hd < sourceParms.cHeads; hd++)
        {
        printf("\rCylinder %d, Head %d", cyl, hd);
        sourceLayout->usHead = hd;
        if (needFormat)
          {
          trkfmt->usHead = hd;
          trkfmt->usCylinder = cyl;
          for (i = 0; i < trkfmt->cSectors; i++)
            {
            trkfmt->FormatTable[i].bHead = (BYTE)hd;
            trkfmt->FormatTable[i].bCylinder = (BYTE)cyl;
            }

#if defined (DEBUG)
    puts("");
    printf("bCommand   %d\n", trkfmt->bCommand);
    printf("usHead     %d\n", trkfmt->usHead);
    printf("usCylinder %d\n", trkfmt->usCylinder);
    printf("usReserved %d\n", trkfmt->usReserved);
    printf("cSectors   %d\n", trkfmt->cSectors);
    getch();
    for (i = 0; i < trkfmt->cSectors; i++)
      {
      printf(" ft[%d].bCylinder    %d\n", i, trkfmt->FormatTable[i].bCylinder);
      printf(" ft[%d].bHead        %d\n", i, trkfmt->FormatTable[i].bHead);
      printf(" ft[%d].idSector     %d\n", i, trkfmt->FormatTable[i].idSector);
      printf(" ft[%d].bBytesSector %d\n", i, trkfmt->FormatTable[i].bBytesSector);
      getch();
      }
    printf("Ready.");
    getch();
    puts("");
#endif

          if (_DosError = DosDevIOCtl(&_fmtData, trkfmt, DSK_FORMATVERIFY, IOCTL_DISK, hf))
            errorexit(hf);
          }
        if (_DosError = DosDevIOCtl(sourceBuffer[trk+hd], sourceLayout, DSK_WRITETRACK, IOCTL_DISK, hf))
          errorexit(hf);
        }
      }

    puts("\rDone.                 \n");
    if (needFormat) free(trkfmt);
    }

  return _DosError;
  }



  /* --- compare two BSPBLK structures ---
  **       returns 0 if both are the same except for possibly the
  **        abReserved field, else returns non-zero.
  */
int bspblkcmp(BSPBLK *blk1, BSPBLK *blk2)
  {
  BSPBLK tmp1, tmp2;

  tmp1 = *blk1;
  tmp2 = *blk2;
  memset(tmp1.abReserved, 0, 6);
  memset(tmp2.abReserved, 0, 6);
  return memcmp(&tmp1, &tmp2, sizeof(BSPBLK));
  }



  /* --- calloc type routine ---
  **       sets _DosError to ERROR_NOT_ENOUGH_MEMORY upon failure
  */
void *Alloc(unsigned num, unsigned size)
  {
  void *rVal;

  _DosError = NO_ERROR;
  if ((rVal = calloc(num, size)) == NULL)
    _DosError = ERROR_NOT_ENOUGH_MEMORY;

  return rVal;
  }



  /* --- error handler ---
  **       parameter is disk handle as returned from opendrive()
  **       prints system error message if possible, closes the disk
  **        handle if neccessary, gets a key stroke, and exits.
  */
void errorexit(HFILE hf)
  {
  USHORT cbBuf;
  CHAR   *msgBuf;

  if (_DosError == DSKCPY_ERROR_WRONG_FORMAT)
    {
    /* Special handling for this non-fatal error */
    fprintf(stderr, "\nThe TARGET disk is not the correct format!");
    fprintf(stderr, "\nStrike any key to return to menu..");
    getch();
    fprintf(stderr, "\n\n");
    return;
    }

#if defined (DSKCPY_ERROR_CANT_FORMAT)
  if (_DosError == DSKCPY_ERROR_CANT_FORMAT)
    {
    /* Special handling for this non-fatal error */
    fprintf(stderr, "\nThe TARGET disk must be preformatted!");
    fprintf(stderr, "\nStrike any key to return to menu..");
    getch();
    fprintf(stderr, "\n\n");
    return;
    }
#endif

  puts("");
  if ((msgBuf = (PCHAR)calloc(BUFSIZE, sizeof(CHAR))) != NULL)
    {
    DosGetMessage(NULL, 0, msgBuf, BUFSIZE, _DosError, "oso001.msg", &cbBuf);
    fputs(msgBuf, stderr);
    free(msgBuf);
    }
  else
    fprintf(stderr, "SYS%04d: error text unavailable\n", _DosError);

  if (hf) DosClose(hf);
  fprintf(stderr, "Strike any key to exit..");
  getch();
  fprintf(stderr, "\n");
  DosExit(EXIT_PROCESS, _DosError);
  }



  /* --- _very_ simpleminded menu ---
  **       parameters are: mlevel: TRUE if source has been read
  **                       drive: asciiz drive specifier, i.e., "a:"
  **       returns READ_SOURCE, COPY_TARGET, or EXIT_DSKCPY
  */
int dskcpy_menu(int mlevel, char *drive)
  {
  int ch;

  printf(" 1. Read SOURCE disk into memory from drive %s\n", drive);
  if (mlevel)
    printf(" 2. Copy memory image to TARGET disk in drive %s\n", drive);
  puts(" Q. Exit DskCpy2\n");
  printf("Enter Choice --> ");
  do
    switch (ch = getch())
      {
      case 'R':
      case 'r':
      case '1':
        puts("Read\n");
        ch = READ_SOURCE;
        break;

      case 'C':
      case 'c':
      case '2':
        if (mlevel)
          {
          puts("Copy\n");
          ch = COPY_TARGET;
          }
        else
          ch = 0;
        break;

      case 'Q':
      case 'q':
      case '\x1b':
        puts("Exit\n");
        ch = EXIT_DSKCPY;
        break;

      default:
        ch = 0;
      }
  while (!ch);
  return ch;
  }



int main(int argc, char **argv)
  {
  HFILE dHandle;
  char  *drive = "a:";
  int   choice;

  copyr();
  if ((argc > 2) || ((argc == 2) && (argv[1][1] != ':')))
    {
    fputs("usage: dskcpy2 drive_letter:", stderr);
    exit(1);
    }

  if (argc == 2)
    drive = argv[1];

  DosError(HARDERROR_DISABLE);
  do
    {
    choice = dskcpy_menu(gotSource, drive);
    switch (choice)
      {
      case READ_SOURCE:
        query("Place SOURCE disk in drive %s and strike any key when ready..", drive);
        if ((dHandle = opendrive(drive)) == 0) errorexit(dHandle);
        if (lockdrive(dHandle))                errorexit(dHandle);
        if (readsource(dHandle))               errorexit(dHandle);
        if (unlockdrive(dHandle))              errorexit(dHandle);
        DosClose(dHandle);
        break;
      case COPY_TARGET:
        query("Place TARGET disk in drive %s and strike any key when ready..", drive);
        if ((dHandle = opendrive(drive)) == 0) errorexit(dHandle);
        if (lockdrive(dHandle))                errorexit(dHandle);
        if (writetarget(dHandle))              errorexit(dHandle);
        if (unlockdrive(dHandle))              errorexit(dHandle);
        DosClose(dHandle);
        break;
      default:
        break;
      }
    }
  while (choice != EXIT_DSKCPY);

  return _DosError;
  }

