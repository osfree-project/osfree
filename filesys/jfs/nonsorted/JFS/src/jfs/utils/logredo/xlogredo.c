/* $Id: xlogredo.c,v 1.1.1.1 2003/05/21 13:42:28 pasha Exp $ */

static char *SCCSID = "@(#)1.6  3/1/99 09:32:48 src/jfs/utils/logredo/xlogredo.c, jfsutil, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		xlogredo.c
 *
 *   COMPONENT_NAME: 	jfsutil
 *
 *
 *   FUNCTIONS:
 *
*/

#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSDATETIME
#define INCL_DOSEXCEPTIONS

#include <os2.h>  
#include <builtin.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jfs_types.h"
#include <jfs_aixisms.h>
#include "devices.h"
#include "fsckmsge.h"

/* Global variables
 */

/* external references **
extern	char *optarg;
extern	int optind;
extern errno;
*/

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


/*
 *	main()
 */
main(
int argc,
char **argv)
{
	int32 inc, rc;
	int32 fd, Blksize;
	uint32 Action;
	int32 dflag = 0;
	caddr_t dev_name;
	extern int32 jfs_logredo(caddr_t, int32, int32 );

	char *prog;           /* program name    */

	prog = argv[0];

	if (argc < 2)
	{	fprintf (stderr, "Usage: %s [-n] filename\n", prog);
		return -1;
	}
	dev_name = *++argv;

	rc = ujfs_open_device(dev_name, (unsigned long *)&fd, &Blksize,
			      RDWR_EXCL);
	if ( rc != 0 )
	{
		fprintf(stderr,"open file system %s failed rc = %d\n", 
					dev_name, rc);
		return(-1);
	}

	/*
	 * logredo it 
	 */
	
	if (rc = jfs_logredo(dev_name, fd, 0))
	{	
		fprintf (stderr, "Failure replaying log: %d\n",rc);
		return -1;
	}
	return 0;
}



