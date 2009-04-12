/* $Id: fsckmsgs.c,v 1.1.1.1 2003/05/21 13:40:04 pasha Exp $ */

static char *SCCSID = "@(#)1.12  6/17/98 09:17:57 src/jfs/utils/chkdsk/fsckmsgs.c, jfschk, w45.fs32, 990417.1";
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
*/
/*
 *   COMPONENT_NAME: jfschk
 *
 *   FUNCTIONS: 
 *               fsck_send_msg
 *
 *               fsck_record_msg
 *               fsck_send_debug_msg
 *
 */

/*
 * defines and includes common among the xfsck modules
 */
#include "xfsckint.h"

/*
 * local includes
 */
#include "xchkdsk.h"
#include "fsckwsp.h"
#include "fsckmsgc.h"


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsck aggregate info structure pointer
  *
  *      defined in xchkdsk.c
  */
extern struct fsck_agg_record *agg_recptr;


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For message processing
  */
#define HPFSMSGFILE  "OSO001.MSG"
#define JFSMSGFILE   "JFS.MSG"
#define STDOUT_HANDLE  1
#define STDERR_HANDLE  2

 /*
  *      defined in xchkdsk.c
  */
extern int16 MsgProtocol[][4];

extern char *Vol_Label;

extern char *terse_msg_ptr;
extern char *verbose_msg_ptr;

extern char *MsgText[];

extern char *msgprms[];
extern int16 msgprmidx[];

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For query response processing
  *
  *      defined in xchkdsk.c
  */
extern char  *msgs_txt_ptr;
extern ULONG  msgs_txt_maxlen;

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */
void fsck_record_msg ( int, int );

void fsck_send_debug_msg ( int, int );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */

/*****************************************************************************
 * NAME: fsck_record_msg
 *
 * FUNCTION:
 *
 * PARAMETERS:
 *      ?                 - input -
 *      ?                 - returned -
 *
 * NOTES:
 *
 * RETURNS:
 *	nothing
 */
void fsck_record_msg( int mess_num, int num_parms )
{
  int prmidx;
  char log_entry[512];
  int entry_length = sizeof(fscklog_entry_hdr_t);
  logent_hdptr      hdptr;
  logent_prmtype    prmtype;
  logent_prmval     prmval;
  logent_prmstr_ptr prmlit;
  int32             buffer_bytes_left;
  char             *buf_entry_ptr;

  if( (!agg_recptr->fscklog_full) && 
      (agg_recptr->fscklog_buf_allocated) &&
      (!agg_recptr->fscklog_buf_alloc_err) ) {  /* logging is active */

    hdptr = (logent_hdptr) log_entry;

    hdptr->msg_num = mess_num;
    hdptr->num_parms = num_parms;

    if( num_parms != 0 ) {
      prmtype = (int8 *) &(log_entry[entry_length]);
      entry_length += num_parms * sizeof(int8);
      prmval = (int16 *) &(prmtype[num_parms]);
      entry_length += num_parms * sizeof(int16);
      prmlit = (char *) &(log_entry[entry_length]);

      prmidx = 0;
      while( prmidx < num_parms ) {
        if( msgprmidx[prmidx] != 0 ) {  /* variable text insert */
          prmtype[prmidx] = fscklog_var_text;
          prmval[prmidx] = msgprmidx[prmidx];
          }  /* end variable text insert */

        else  {  /* string literal insert */
          prmtype[prmidx] = fscklog_literal;
          prmval[prmidx] = strlen(msgprms[prmidx]);
          strncpy( prmlit, msgprms[prmidx], prmval[prmidx] );
          entry_length += prmval[prmidx];
             /*
              * add a null terminator to the literal
              */
          log_entry[entry_length] = '\0';
          prmval[prmidx] += 1;
          entry_length += 1;
             /*
              * set up for the next literal
              */
          prmlit = (char *) &(log_entry[entry_length]);
          }  /* end string literal insert */

        prmidx += 1;
        }
      }
  
    entry_length = ((entry_length + 3) / 4 ) * 4;  /* 
                          * pad to the next 4 byte boundary
                          */
    hdptr->entry_length = entry_length;
    buffer_bytes_left = agg_recptr->fscklog_buf_length - 
                        agg_recptr->fscklog_buf_data_len;
    if( buffer_bytes_left < entry_length ) {
      agg_recptr->fscklog_last_msghdr->entry_length += buffer_bytes_left;
      fscklog_put_buffer();      
      memset( (void *) (agg_recptr->fscklog_buf_ptr), 
               0, 
               agg_recptr->fscklog_buf_length);  /*
                         * clear the buffer 
                         */
      }

    if( ! agg_recptr->fscklog_full ) {  
      buf_entry_ptr = (char *) ( (int32) agg_recptr->fscklog_buf_ptr + 
                                 (int32) agg_recptr->fscklog_buf_data_len
                                );
      memcpy( (void *) buf_entry_ptr, 
                (void *) hdptr,
                entry_length );

      agg_recptr->fscklog_last_msghdr = 
                          (struct fscklog_entry_hdr *) buf_entry_ptr;
      agg_recptr->fscklog_buf_data_len += entry_length;
      }
    }  /* end logging is active */

    return;
}                                        /* end fsck_record_msg() */


/*****************************************************************************
 * NAME: fsck_send_debug_msg
 *
 * FUNCTION: Issue an fsck message to stdout in English.
 *
 * PARAMETERS:
 *      ?                 - input -
 *      ?                 - returned -
 *
 * NOTES:    none
 *
 * RETURNS:  none
 */
void fsck_send_debug_msg( int msg_num, int num_inserts )
{
  int  prmidx;

      /* 
       * if the messaging level is debug, issue the message in English
       */
  memset( (void *) verbose_msg_ptr, '\0', msgs_txt_maxlen );
  memset( (void *) msgs_txt_ptr, '\0', msgs_txt_maxlen );
  prmidx = 0;
  while( prmidx < num_inserts ) {
    if( msgprmidx[prmidx] != 0 ) {  /* variable text insert */
      msgprms[prmidx] = MsgText[ msgprmidx[prmidx] ];
      }  /* end variable text insert */
    prmidx += 1;
    }
  sprintf( msgs_txt_ptr, MsgText[ msg_num ], 
           msgprms[0], msgprms[1], msgprms[2], msgprms[3], msgprms[4], 
           msgprms[5], msgprms[6], msgprms[7], msgprms[8], msgprms[9]  );
  sprintf( verbose_msg_ptr, "%s%s", "(chklog) ", msgs_txt_ptr ); 

  if( MsgProtocol[msg_num][MP_STREAM] == fsck_out ) {
    DosPutMessage( STDOUT_HANDLE, strlen(verbose_msg_ptr), verbose_msg_ptr );
      }
  else {
    DosPutMessage( STDERR_HANDLE, strlen(verbose_msg_ptr), verbose_msg_ptr );
    }

  return;
}                                               /* end fsck_send_debug_msg() */


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
  int prmidx;
  ULONG mess_length = 0;
  ULONG mess_number;
  ULONG insert_number;
  ULONG terse_mess_number;
  char *msg_file;
  int msgfile_idx;
  int msg_stream;

  if( MsgProtocol[msg_num][MP_MSGFILE] == jfs_msgfile ) {
    msg_file =JFSMSGFILE;
    msgfile_idx = JFS_MSG;
    mess_number = msg_num + fsck_msgid_offset;
    }
  else if( MsgProtocol[msg_num][MP_MSGFILE] == hpfs_msgfile ) {   
    msg_file = HPFSMSGFILE;
    msgfile_idx = OSO_MSG;
    mess_number = MsgProtocol[msg_num][MP_HPFSMSGNUM];
    }
  if( MsgProtocol[msg_num][MP_STREAM] == fsck_out ) {
    msg_stream = STDOUT_CODE;
    }
  else {
    msg_stream = STDERR_CODE;
    }

    /*
     * Now see if we're messaging at the terse level and the
     * given message has a terse alternate
     *
     * Note that any message designated fsck_terse is local_language
     * for nls.
     *
     * Note that terse messages never have inserts
     */
  if( (agg_recptr->effective_msg_level == fsck_terse) &&
      (terse_msg_num != 0)                               ) {

     /*
      * Issue the terse message in the local language
      */
    memset( (void *) msgs_txt_ptr, '\0', msgs_txt_maxlen );

    terse_mess_number = terse_msg_num + fsck_msgid_offset;

    message_user( terse_msg_num, NULL, 0, 
                  msg_stream, NO_RESPONSE, msgfile_idx );
    }

     /*
      * otherwise see if
      *      - effective messaging level is >= the level of the given message 
      * or  - this is an autocheck message and processing in autocheck mode
      */
  else if( (agg_recptr->effective_msg_level >= 
                                        MsgProtocol[msg_num][MP_MSGLVL])      || 
             ((MsgProtocol[msg_num][MP_MSGLVL] == fsck_autochk)     &&
              (agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK])     )    ) {
      /* 
       * if the message has a local language translation, 
       * assemble it and issue it in the local language.
       */
    if( MsgProtocol[msg_num][MP_MSGFILE] != no_msgfile ) {  /* translate */
      memset( (void *) msgs_txt_ptr, '\0', msgs_txt_maxlen );
      prmidx = 0;
         /*
          * translated insert text ALWAYS comes from the JFS message file
          */
      while( prmidx < num_inserts ) {
        if( msgprmidx[prmidx] != 0 ) {  /* variable text insert */
          insert_number = msgprmidx[prmidx] + fsck_msgid_offset;
          DosGetMessage( NULL, 0, msgprms[prmidx], MAXPARMLEN, 
                         insert_number, JFSMSGFILE, &mess_length );
          msgprms[prmidx][mess_length - 2] = '\0';        /* strip off the CRLF */
          }  /* end variable text insert */
        prmidx += 1;
       }

      message_user( mess_number, msgprms, num_inserts, 
                    msg_stream, NO_RESPONSE, msgfile_idx );
      }  /* end translate */
    }

     /*
      * Log the message  
      */
  fsck_record_msg( msg_num, num_inserts );        

     /*
      * Issue the message in (English) if appropriate
      */
  if( agg_recptr->effective_msg_level == fsck_debug ) {  /* messaging debug */
    fsck_send_debug_msg( msg_num, num_inserts );        
    }  /* end messaging debug */

    return;
}                                        /* end fsck_send_msg() */
