/* $Id: helpers.c,v 1.1.1.1 2003/05/21 13:42:26 pasha Exp $ */

static char *SCCSID = "@(#)1.1  3/1/99 09:34:22 src/jfs/utils/logdump/helpers.c, jfsutil, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		helpers.c 
 *
 *		 	no-frills substitutes for chkdsk routines used 
 *			by logredo modules used outside chkdsk 
 *
 *   COMPONENT_NAME: 	jfslib
 *
 *
 *   FUNCTIONS:
 *		alloc_wrksp
 *		fsck_send_msg
 *
 */

#define INCL_DOSMISC

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jfs_types.h"
#include "fsckmsgc.h"

typedef int    retcode_t;
typedef uint32   reg_idx_t;

#define STDOUT_HANDLE  1
#define STDERR_HANDLE  2

extern int16 MsgProtocol[][4];

extern char *Vol_Label;

extern char *terse_msg_ptr;
extern char *verbose_msg_ptr;

extern char *MsgText[];

extern char *msgprms[];
extern int16 msgprmidx[];

extern ULONG msgs_txt_maxlen;


/****************************************************************************
 * NAME: alloc_wrksp
 *
 * FUNCTION:  Allocates and initializes (to guarantee the storage is backed)
 *            dynamic storage for the caller.
 *
 * PARAMETERS:
 *      length         - input - the number of bytes of storage which are needed
 *      dynstg_object  - input - a constant (see xfsck.h) identifying the purpose
 *                               for which the storage is needed (Used in error 
 *                               message if the request cannot be satisfied.
 *      addr_wrksp_ptr - input - the address of a variable in which this routine
 *                               will return the address of the dynamic storage
 *                               allocated for the caller
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
retcode_t alloc_wrksp ( reg_idx_t   length,
                        int         dynstg_object,
                        int         for_logredo,
                        void      **addr_wrksp_ptr )
{
  retcode_t awsp_rc = 0;
  char *extent_addr = NULL;
  reg_idx_t min_length;

  *addr_wrksp_ptr = NULL;      /* initialize return value */
  min_length = ((length + 7) / 4) * 4;  /* round up to an 4 byte boundary */

  *addr_wrksp_ptr = (char *) malloc(min_length);

 return( awsp_rc );
}                                    /* end alloc_wrksp()    */


/*****************************************************************************
 * NAME: fsck_send_msg
 *
 * FUNCTION: Issue an fsck message, depending on the message's protocol
 *           according to the fsck message arrays.
 *
 * PARAMETERS:
 *      ?                 - input -
 *      ?                 - returned -
 *
 * NOTES:  none
 *
 * RETURNS:
 *	nothing
 */
void fsck_send_msg( int    msg_num, 
                    int    terse_msg_num, 
                    int    num_inserts )
{
  int  prmidx;

  memset( (void *) verbose_msg_ptr, '\0', msgs_txt_maxlen );

  prmidx = 0;
  while( prmidx < num_inserts ) {
    if( msgprmidx[prmidx] != 0 ) {  /* variable text insert */
      msgprms[prmidx] = MsgText[ msgprmidx[prmidx] ];
      }  /* end variable text insert */
    prmidx += 1;
    }

  sprintf( verbose_msg_ptr, MsgText[ msg_num ], 
           msgprms[0], msgprms[1], msgprms[2], msgprms[3], msgprms[4], 
           msgprms[5], msgprms[6], msgprms[7], msgprms[8], msgprms[9]  );

  DosPutMessage( STDOUT_HANDLE, strlen(verbose_msg_ptr), verbose_msg_ptr );

  return;
}                                        /* end fsck_send_msg() */

