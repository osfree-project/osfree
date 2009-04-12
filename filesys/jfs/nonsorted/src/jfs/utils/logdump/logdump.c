/* $Id: logdump.c,v 1.2 2004/03/21 02:43:14 pasha Exp $ */

static char *SCCSID = "@(#)1.4  3/4/99 07:22:22 src/jfs/utils/logdump/logdump.c, jfsutil, w45.fs32, 990417.1";
/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   MODULE_NAME:		logdump.c
 *
 *   COMPONENT_NAME: 	jfsutil
 *
 *
 *   FUNCTIONS:
 *
 *      This  *** SERVICE-ONLY ***  tool dumps the contents of
 *      the journal log on the specified JFS partition into 
 *      "\JFSLOG.DMP".
 *
 *
 */

#include <stdio.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_xtree.h"
#include "jfs_logmgr.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_debug.h"
#include "jfs_cntl.h"
#include "logredo.h"
#include "devices.h"
#include "debug.h"
#include "fsckmsge.h"


#define LOGDMP_OK 	0
#define LOGDMP_FAILED	-1

#define  FULLLOG  -1
#define  CURRLOG   1


int32  jfs_logdump( char *, int32, int32 );
int32 open_device_read ( char * );
int32 parse_parms ( int32, char ** );


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * Device information.
  *
  *     values are assigned when (if) the device is opened.
  */
HFILE   Dev_IOPort;
uint32  Dev_blksize;
int32   Dev_SectorSize;

struct DPB Datap;

TRACKLAYOUT *ptrklay;   /* pointer to TRACKLAYOUT.
                         * The tracktable is initialized during 
                         * device open processing and is updated
                         * in device read processing
                         */
char  *Vol_Label;

int device_is_open = 0;
int32 dump_all = 0;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For message processing in called routines
  *
  */

// UniChar uni_message_parm[64];
// UniChar *uni_msgprm = uni_message_parm;

char message_parm_0[64];
char message_parm_1[64];
char message_parm_2[64];
char message_parm_3[64];
char message_parm_4[64];
char message_parm_5[64];
char message_parm_6[64];
char message_parm_7[64];
char message_parm_8[64];
char message_parm_9[64];

char *msgprms[10] = { message_parm_0, message_parm_1, message_parm_2,
                      message_parm_3, message_parm_4, message_parm_5,
                      message_parm_6, message_parm_7, message_parm_8,
                      message_parm_9 };

int16 msgprmidx[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

char message_buffer_0[256];
char message_buffer_1[1024];
char message_buffer_2[1024];
char *terse_msg_ptr =   &message_buffer_0[0];
char *verbose_msg_ptr = &message_buffer_1[0];
char *msgs_txt_ptr =    &message_buffer_2[0];
ULONG msgs_txt_maxlen = 1024;

extern char *MsgText[];

extern void fsck_send_msg( int, int, int );		/* defined in fsckmsg.c */
extern int alloc_wrksp( uint32, int, int, void ** );	/* defined in fsckwsp.c */

/****************************************************************************
 *
 * NAME: main
 *
 * FUNCTION: call jfs_logdmp()
 *
 * INTERFACE:  
 *            jfs_logdump [-a] <volume specifier> 
 *		
 *		where -a => dump entire contents of log instead of just 
 *				committed transactions since last synch
 *				point.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32  main( int32 argc, char **argv )
{
  int32 rc = LOGDMP_OK;
  int32     Vol_Label_Size;

  rc = parse_parms( argc, argv );  /* parse the parms and record
                                    * them in the aggregate wsp record
                                    */

  if( rc == LOGDMP_OK ) {  /* parms are good */

    rc = open_device_read ( Vol_Label );

    if( rc == LOGDMP_OK ) {
      rc = jfs_logdump( Vol_Label, Dev_IOPort, dump_all  );
      }

        /*
         * Close the device
         */
    if( device_is_open ) {
      rc = DosClose( Dev_IOPort );
      free(ptrklay);
      }

    }  /* end parms are good */

    return( rc );
}                                /* end of main()   */

/* ======================================================================*/


/*****************************************************************************
 * NAME: open_device_read
 *
 * FUNCTION:  Open the specified device for read access.
 *
 * PARAMETERS:
 *      Device  - input - the device specification
 *
 * NOTES:
 *
 * RETURNS:
 *      success: LOGDMP_OK
 *      failure: something else
 */
int32 open_device_read ( char    *Device )
{
  int32    openrd_rc = 0;
  int32    close_rc = 0;
  ULONG    Action;
  int      i;
  unsigned char CommandInfo = 0;
  unsigned long ParmLen = sizeof(CommandInfo);
  unsigned long DataLen = sizeof(struct DPB);
  USHORT ulSecsPerTrk;
  struct DPB *tmp_dpb = &Datap;
  TRACKLAYOUT *tmplayout;

  openrd_rc = DosOpen(Device, &Dev_IOPort, &Action, 0, 0,
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                      OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0);

  if( openrd_rc != NO_ERROR ) {
    printf("LOG_DUMP: DosOpen failed with return code = %ld\n\r", openrd_rc );
    openrd_rc = LOGDMP_FAILED;
    }

  else {  /* now open for read, allow read/write */

        /*
         * Get the device parameters
         */
    openrd_rc = DosDevIOCtl(Dev_IOPort, IOCTL_DISK,
                            DSK_GETDEVICEPARAMS, &CommandInfo,
                            sizeof(CommandInfo), &ParmLen, &Datap,
                            sizeof(struct DPB), &DataLen);

    if( openrd_rc != NO_ERROR ) {
      printf("LOG_DUMP: DSK_GETDEVICEPARAMS failed with return code = %ld\n\r", openrd_rc );
      openrd_rc = LOGDMP_FAILED;
      }

    else {
      Dev_SectorSize = Datap.dev_bpb.bytes_per_sector;
      Dev_blksize = Dev_SectorSize;
      ulSecsPerTrk = Datap.dev_bpb.sectors_per_track;
      ptrklay = (TRACKLAYOUT *)malloc(sizeof(TRACKLAYOUT) + 4 * ulSecsPerTrk );
 
      if( ptrklay == NULL ) {
        printf("LOG_DUMP: Can't allocate storage for device information\n\r" );
        openrd_rc = LOGDMP_FAILED;
        }
      else {
        for( i = 0; i < ulSecsPerTrk; i++ ) {
          ptrklay->TrackTable[i].usSectorSize = Dev_SectorSize;
          ptrklay->TrackTable[i].usSectorNumber = i + 1;
          }
        device_is_open = -1;
        }
      }
    }   /* end successful open for read, allow read/write */

  return( openrd_rc );
}                             /* end of open_device_read ( ) */


/*****************************************************************************
 * NAME: parse_parms
 *
 * FUNCTION:  Parse the invocation parameters.  If any unrecognized
 *            parameters are detected, or if any required parameter is
 *            omitted, issue a message and exit.
 *
 * PARAMETERS:  as specified to main()     
 *
 * NOTES:
 *
 * RETURNS:
 *      success: LOGDMP_OK
 *      failure: something else
 */
int32 parse_parms ( int32 argc, char **argv )
{
  int32 pp_rc = LOGDMP_OK;
  int       i = 0;
  int       log_parm_seen = 0;
  int       bad_log_parm = 0;
  char      *lvolume = NULL;
  char      *argp;

  for (i = 1; i < argc; i++) {  /* for all parms on command line */
    argp = argv[i];

    if (*argp == '/' || *argp == '-') {  /* leading / or - */
        argp++;
        if (*argp == 'a' || *argp == 'A') {  /* debug */
              /*
               * Dump entire log Option
               */
            dump_all = -1;
            }  
        else { 
              /* 
               * unrecognized keyword parm 
               */
            printf( "LOGDUMP:  unrecognized keyword detected:   %s\n\r", argp );
            return( LOGDMP_FAILED );
            }
        }  /* end leading / or - */

    else if (argp[1] == ':') {  /* 2nd char is : */
        lvolume = argp;
        }  /* end 2nd char is : */

    else {  /* unrecognized parm */
        printf( "LOGDUMP:  unsupported parameter detected:   %s\n\r", argp );
        return( LOGDMP_FAILED );
        }  /* end unrecognized parm */
    }  /* end for all parms on command line */

    if (lvolume == NULL) {  /* no device specified */
        printf( "LOGDUMP:  volume must be specified\n\r" );
        return( LOGDMP_FAILED );
        }  /* end no device specified */
    else {  /* got a device */
        Vol_Label = lvolume;
        }  /* end got a device */

  return( pp_rc );
}                            /* end of parse_parms()  */


