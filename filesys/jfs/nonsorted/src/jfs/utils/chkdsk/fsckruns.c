/* $Id: fsckruns.c,v 1.2 2004/03/21 02:43:28 pasha Exp $ */

static char *SCCSID = "@(#)1.4  4/7/99 08:25:11 src/jfs/utils/chkdsk/fsckruns.c, jfschk, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		fsckruns.c
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *   FUNCTIONS:
 *              fsck_hbeat
 *              fsck_hbeat_start
 *              fsck_hbeat_stop
 *	
 *
 */

#define INCL_DOSPROCESS

/*
 * defines and includes common among the xfsck modules
 */
#include "xfsckint.h"

extern char *MsgText[];

char hb_message_buffer[256];
char *hb_msg_ptr = &hb_message_buffer[0];

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For managing the heartbeat thread
  *
  * defined in xchkdsk.c
  *
  */
void _System fsck_hbeat ( ULONG );

static TID		tidHbeatThread = 0;	
static PFNTHREAD	pfnHbeat = &fsck_hbeat;	
int volatile 		exit_flag = 0;

/*****************************************************************************
 * NAME: fsck_hbeat
 *
 * FUNCTION:	
 *
 * PARAMETERS:  none
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
void _System fsck_hbeat ( ULONG current_heartbeat )
{
    while( exit_flag == 0 ) {
	
       DosSleep( 500); 	/* sleep for a half second */
	
	switch( current_heartbeat ) {
	    case 0:
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT0] );
		current_heartbeat = 1;
		break;
	    case 1:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT1] );
		current_heartbeat = 2;
		break;
	    case 2:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT2] );
		current_heartbeat = 3;
		break;
	    case 3:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT3] );
		current_heartbeat = 4;
		break;
	    case 4:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT4] );
		current_heartbeat = 5;
		break;
	    case 5:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT5] );
		current_heartbeat = 6;
		break;
	    case 6:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT6] );
		current_heartbeat = 7;
		break;
	    case 7:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT7] );
		current_heartbeat = 8;
		break;
	    case 8:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT8] );
		current_heartbeat = 9;
		break;
	    case 9:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT7] );
		current_heartbeat = 10;
		break;
	    case 10:             
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT6] );
		current_heartbeat = 11;
		break;
	    case 11:             
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT5] );
		current_heartbeat = 12;
		break;
	    case 12:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT4] );
		current_heartbeat = 13;
		break;
	    case 13:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT3] );
		current_heartbeat = 14;
		break;
	    case 14:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT2] );
		current_heartbeat = 15;
		break;
	    case 15:               
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT1] );
		current_heartbeat = 0;
		break;
	    default:
		sprintf( hb_msg_ptr, MsgText[fsck_HEARTBEAT0] );
		current_heartbeat = 1;
	    }

	DosPutMessage( 1, strlen(hb_msg_ptr), hb_msg_ptr );
	fflush( stdout );
	
	}  /* end while */

  exit_flag = 0;
	
  return;
}                            /* end of fsck_hbeat()  */
	
	
/*****************************************************************************
 * NAME: fsck_hbeat_start
 *
 * FUNCTION:	
 *
 * PARAMETERS:  none
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
void fsck_hbeat_start ( )
{
  APIRET   rc;
  ULONG     ulThreadParm = 0;
	
  rc = DosCreateThread( &tidHbeatThread, pfnHbeat, ulThreadParm, 
                                  CREATE_READY | STACK_COMMITTED, 8192L );
	
  if( rc != NO_ERROR ) {
    tidHbeatThread = 0;
    }
	
  return;
}                            /* end of fsck_hbeat_start()  */
	
	
/*****************************************************************************
 * NAME: fsck_hbeat_stop
 *
 * FUNCTION:	
 *
 * PARAMETERS:  none
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
void fsck_hbeat_stop ( )
{
  APIRET   rc;
  int	idx, done = 0;

  exit_flag = -1;
	
	/*
	 * try 5 times to kill the heartbeat thread
	 */
  while( exit_flag != 0 ) {
	DosSleep(0);
	}
	
  return;
}                            /* end of fsck_hbeat_stop()  */
