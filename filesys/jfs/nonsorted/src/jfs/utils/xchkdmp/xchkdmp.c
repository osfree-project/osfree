/* $Id: xchkdmp.c,v 1.2 2004/03/21 02:43:36 pasha Exp $ */

static char *SCCSID = "@(#)1.2  2/9/99 09:08:59 src/jfs/utils/xchkdmp/xchkdmp.c, jfsutil, w45.fs32, 990417.1";
/*
 *
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
 *   COMPONENT_NAME: jfschk
 *
 *      This is a SERVICE-ONLY TOOL.
 *
 *      This routine dumps the contents of a file created by the
 *      service-only tool xchklog.  If no input file name is specified,
 *      a default path and name are used.
 *      
 *      USAGE:  
 *
 *         xchkdmp [-F<input.fil>]
 *
 *         where: o <input.fil> is assumed to be in the present working
 *                  directory unless it is fully qualified
 *                o <input.fil> must be 127 characters or less in length
 *                o if <input.fil> is not specified, the default path
 *                  and name are used:
 *                                         \chkdsk.new
 *
 *      SAMPLE INVOCATIONS:
 *
 *         To dump the contents of the chkdsk service log in \chkdsk.new
 *                     chkdmp
 *
 *         To dump the contents of the chkdsk service log in <pwd>input.fil
 *                     chkdmp -Finput.fil
 * 
 *         To dump the contents of the chkdsk service log in \temp\l970610.log
 *                     chkdmp -f\temp\l970610.log
 *
 *   FUNCTIONS:
 *		main
 *
 *		dump_msg
 *		dump_service_log
 *		final_processing 
 *		fscklog_fill_buffer
 *		initial_processing 
 *		open_infile 
 *		parse_parms 
 *		send_msg
 *
 */

/*
 *       Define the following to avoid including 
 *       a bunch of unnecessary header files
 */
#define _JFS_XCHKDMP

#include "stdio.h"
#include "stdlib.h"

#include <os2.h>  
#include <string.h>
#include <stdlib.h>

#include "jfs_types.h"

#define fsck_highest_msgid_defined 399

/*
 * includes from JFS/Warp chkdsk
 */
#include "fsckmsgc.h"         /* message constants */
#include "fsckmsge.h"         /* message text, all messages, in english */
#include "fscklog.h"          /* fsck service log */


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * record for xchkdmp processing
  *
  */
typedef struct chklog_record {
   char       eyecatcher[8];
   char      *infile_buf_ptr;
   uint32     infile_buf_length;
   uint32     infile_buf_data_len;
   int        ag_blk_size;   
   uint32     highest_msg_num;
   int8       infile_is_open;
   int8       this_buffer_last_message;
   int8       fscklog_last_read;
   int8       infile_name_specified;
   } chklog_record_t;

chklog_record_t  xchklog_record;
chklog_record_t *local_recptr;

#define  default_logfile_name     "\\chkdsk.new"
#define  default_logfile_name_len  11

char     infile_name[128];
FILE *infp;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * input:  fsck service log I/O buffer
  * output: chkdsk extracted service log I/O buffer
  *
  */
char xchklog_buffer[XCHKLOG_BUFSIZE];

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For message processing
  *
  *    values are assigned by the main xchklog routine
  */

char *Vol_Label;
char *msgprms[10] = { NULL, NULL, NULL, NULL, NULL, 
                      NULL, NULL, NULL, NULL, NULL };
int16 msgprmidx[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

char message_buffer_0[512];
char *msgs_txt_ptr = &message_buffer_0[0];
ULONG msgs_txt_maxlen = 512;


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */

void dump_msg( int, int );

void dump_service_log( void );

int32 final_processing ( void ) ;

int32  fscklog_fill_buffer ( void );

int32 initial_processing ( int32, char ** );

int32 open_infile ( void );

int32 parse_parms ( int32, char ** );

void send_msg( int, int );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


 /*
  * module return code and possible values
  */

#define XCHKDMP_OK                  0
#define XCHKDMP_FAILED             -1
#define XCHKDMP_CANTOPENINFILE     -2
#define XCHKDMP_MTORBADREAD        -3
#define XCHKDMP_BADFORMAT          -4

int32 rc = XCHKDMP_OK;

/*****************************************************************************
 * NAME: main
 *
 * FUNCTION: Entry point for jfs read aggregate chkdsk service log
 *
 * INTERFACE:  
 *            int32 xchkdmp
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32  main( int32 argc, char **argv )
{
  int32 rc = XCHKDMP_OK;
  int32     Vol_Label_Size;

  /*
   * some basic initializations
   */
  local_recptr = &xchklog_record;
  memset( (void *) infile_name, 0, 128 );

  rc = initial_processing( argc, argv );

  if( rc == XCHKDMP_OK ) {  /* we have a buffer and the device is open */

    dump_service_log();
    }

  if( rc == XCHKDMP_OK ) {  /* no errors yet */
    rc = final_processing();
    }  /* end no errors yet */

  else {  /* something already went wrong */
    final_processing();
    }  /* something already went wrong */

    return( rc );
}                                /* end of main()   */


/*****************************************************************************
 * NAME: dump_msg
 *
 * FUNCTION: Dump the contents of a message record in the extracted 
 *           chkdsk service log.
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
void dump_msg( int  msg_num, int  num_inserts )
{
  int prmidx;

  printf(" message number : %ld\n\r", msg_num );
  for( prmidx=0; prmidx < num_inserts; prmidx++ ) {
    printf("  insert #%d: length: %03d   text: %s\n\r", 
           prmidx, msgprmidx[prmidx], msgprms[prmidx] );
    }
  printf("\n\r");

  return;
}                                        /* end dump_msg() */


/***************************************************************************
 * NAME: dump_service_log
 *
 * FUNCTION:  
 *
 * PARAMETERS:  none
 *
 * NOTES:  o  The first read from the input file is done when the
 *            file is opened.  (This is done so that the eyecatcher
 *            can be read to verify that the input file is actually
 *            an extracted JFS/Warp chkdsk service log.)
 *
 * RETURNS:
 *      success: XCHKDMP_OK
 *      failure: something else
 */
void dump_service_log( )
{
  int32 dsl_rc = XCHKDMP_OK;
  int       prmidx;
  char     *log_entry;
  int       log_entry_pos;
  int       bytes_left;
  chklogent_hdptr      hdptr;
  chklogent_prmlen     prmlen;
  chklogent_prmstr_ptr prmlit;

  local_recptr->fscklog_last_read = 0;

  while( (!local_recptr->fscklog_last_read) && 
         (dsl_rc == XCHKDMP_OK)                   ) {

    local_recptr->this_buffer_last_message = 0;
    while( (!local_recptr->this_buffer_last_message) && 
           (dsl_rc == XCHKDMP_OK)                         ) {

      hdptr = (chklogent_hdptr) 
                 ( (int32) local_recptr->infile_buf_ptr + 
                   (int32) local_recptr->infile_buf_data_len
                  );
      if( hdptr->entry_length == 0 ) {  /* no more entries in this buffer */
        local_recptr->this_buffer_last_message = -1;
        }  /* end no more entries in this buffer */

      else {  /* this entry_length != 0 */
        log_entry = (char *) hdptr;
        log_entry_pos = sizeof(fscklog_entry_hdr_t);

            /*
             * Each FSCKLOG_BUFSIZE boundary starts a new
             * log record.  Otherwise, we depend on a valid
             * entry_length to locate the next message.
             */
        bytes_left = local_recptr->infile_buf_length -
                     local_recptr->infile_buf_data_len;
        if( (bytes_left < 0 )           ||
            (hdptr->entry_length < 0 )  ||
            (hdptr->entry_length > bytes_left)   ) { /*
                          * this isn't a valid log record
                          * length
                          */
          send_msg( fsck_CHKLOGINVALRCD, 0 );
          local_recptr->this_buffer_last_message = -1;
          }  /* end this isn't a valid log record length */
        else {  /* the log record length looks ok */
          if( (hdptr->msg_num < 0)                          || 
              (hdptr->msg_num > local_recptr->highest_msg_num) ) { /*
                          * this isn't a valid log record
                          */
            send_msg( fsck_CHKLOGINVALRCD, 0 );
            }  /* end this isn't a valid log record */
          else if( (hdptr->num_parms < 0) ||
                   (hdptr->num_parms > 10)  )  {  /* invalid number of parms */
            send_msg( fsck_CHKLOGINVALRCD, 0 );
            }  /* end invalid number of parms */

          else {  /* the record looks ok */

               /* 
                * set up for message inserts
                */
            if( hdptr->num_parms != 0 ) {  /* there are parms */
              prmlen = (int16 *) &(log_entry[log_entry_pos]);
              log_entry_pos += hdptr->num_parms * sizeof(int16);
              prmlit = (char *) &(log_entry[log_entry_pos]);

              for( prmidx = 0; prmidx < hdptr->num_parms; prmidx++ ) {
                msgprmidx[prmidx] = prmlen[prmidx];
                msgprms[prmidx] = prmlit;
                prmlit = (char *) 
                         ((int32) prmlit + (int32) prmlen[prmidx]);
                }  /* end for prmidx */
              }  /* end there are parms */

               /* 
                * dump the message record
                */
            dump_msg( hdptr->msg_num, hdptr->num_parms );
            }  /* end the the record looks ok */

             /* 
              * set up for the next record 
              */
          local_recptr->infile_buf_data_len += hdptr->entry_length;
          if( local_recptr->infile_buf_data_len >= 
                       local_recptr->infile_buf_length ) {
            local_recptr->this_buffer_last_message = -1;
            }
          }  /* end else the log record length looks ok */
        }  /* end else this entry_length != 0 */
      }  /* end while !this_buffer_last_message */

          /*
           * read some more from the input file 
           */
    if( (!local_recptr->fscklog_last_read) && 
         (dsl_rc == XCHKDMP_OK)                   ) {
      dsl_rc = fscklog_fill_buffer();
      }
    }  /* end while !fscklog_last_read  */

  return;
}                              /* end of dump_service_log ()  */


/***************************************************************************
 * NAME: final_processing
 *
 * FUNCTION:  If processing read/write, replicate the superblock and the 
 *            aggregate inode structures (i.e., the Aggregate Inode Map
 *            and the Aggregate Inode Table).
 *
 *            Notify the user about various things.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: XCHKDMP_OK
 *      failure: something else
 */
int32  final_processing ( )
{
  int32 pf_rc = XCHKDMP_OK;
  int32 of_rc = XCHKDMP_OK;

    /*
     * Close the input file
     */
  if( local_recptr->infile_is_open ) {
    pf_rc = fclose( infp );
    }

  return( pf_rc );
}                              /* end of final_processing ()  */


/****************************************************************************
 * NAME: fscklog_fill_buffer
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            and if the in-aggregate fsck log is not full, write the
 *            contents of the current fscklog buffer into the in-aggregate
 *            fsck log.
 *
 * PARAMETERS:  none
 *
 * NOTES:  o Unlike most _put_ routines in this module, fscklog_put_buffer
 *           actually writes to the device.  This is done because the fsck
 *           log contains information which provides crucial serviceability 
 *           should the fsck session be interrupted.
 *
 *         o Errors here are recorded in the control page of the fsck 
 *           in-aggregate workspace but never affect other fsck processing.
 *
 * RETURNS:
 *      success: XCHKDMP_OK
 *      failure: something else
 */
int32  fscklog_fill_buffer ( )
{
  int32 flfb_rc = XCHKDMP_OK;
  uint32 bytes_read = 0;

  bytes_read = fread( (void *) local_recptr->infile_buf_ptr,
                      sizeof(char),
                      XCHKLOG_BUFSIZE,
                      infp
                     );

  if( bytes_read != XCHKLOG_BUFSIZE ) {
    if( feof(infp) ) {
      local_recptr->fscklog_last_read = -1;
      }
    }  /* end else didn't get all we asked for but not at end... */

      /*
       * We want to reset the buffer no matter what.
       * It is useful to keep going because the next section may be
       * readable. 
       */
  local_recptr->infile_buf_data_len = 0;

  return( flfb_rc );
}                              /* end of fscklog_fill_buffer ()  */


/*****************************************************************************
 * NAME: initial_processing
 *
 * FUNCTION: Parse and verify invocation parameters.  
 *           Open the device and verify that it contains a JFS file system.  
 *           Read the chkdsk workspace control page.
 *           Calculate interesting aggregate offsets.
 *
 *
 * PARAMETERS:  as specified to main()     
 *
 * NOTES:
 *
 * RETURNS:
 *      success: XCHKDMP_OK
 *      failure: something else
 */
int32 initial_processing ( int32 argc, char **argv )
{
  int32  pi_rc = XCHKDMP_OK;

  /*
   * Initialize the xchklog control block
   */
  memset ( local_recptr, 0, sizeof(xchklog_record) );
  memcpy( (void *) &(local_recptr->eyecatcher), (void *) "chklog  ", 8 );
  local_recptr->infile_buf_length = XCHKLOG_BUFSIZE;
  local_recptr->infile_buf_ptr = xchklog_buffer;
  local_recptr->highest_msg_num = fsck_highest_msgid_defined + 
                                  JFSCHKLOG_FIRSTMSGNUM;
  /*
   * Process the parameters given by the user
   */
  pi_rc = parse_parms( argc, argv );  /* parse the parms and record
                                       * them in the aggregate wsp record
                                       */

  if( pi_rc == XCHKDMP_OK ) {  /* the parms are good */
    pi_rc = open_infile();
    }  /* end parms are good */

  return( pi_rc );
}                              /* end of initial_processing ()  */


/*****************************************************************************
 * NAME: open_infile
 *
 * FUNCTION:  Open the input file.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: XCHKDMP_OK
 *      failure: something else
 */
int32 open_infile ( )
{
  int32    openif_rc = 0;
  uint32   bytes_read = 0;
  int      eyecatcher_incorrect = 0;

  if( !local_recptr->infile_name_specified ) {  /* no input file name specified */
    memcpy( (void *) infile_name, 
            (void *) default_logfile_name, 
            default_logfile_name_len 
          );
    }  /* end no input file name specified */

  msgprms[0] = infile_name;
  msgprmidx[0] = 0;

  infp = fopen( infile_name, "r");
  if( infp == NULL ) {  /* input file open failed */
    openif_rc = XCHKDMP_CANTOPENINFILE;
    send_msg( fsck_XCHKDMPOPNFAIL, 1 );
    }  /* end input file open failed */
  else {  /* input file is open */
    local_recptr->infile_is_open = -1;
    }  /* end else input file is open */

  if( local_recptr->infile_is_open ) {  /* so far so good */
         /*
          * read in the first buffer of data
          */
    bytes_read = fread( (void *) local_recptr->infile_buf_ptr,
                        sizeof(char),
                        XCHKLOG_BUFSIZE,
                        infp
                       );
    if( bytes_read != XCHKLOG_BUFSIZE ) {  /*
                   * there should be at least this much
                   */
      openif_rc = XCHKDMP_MTORBADREAD;
      send_msg( fsck_XCHKDMPMTORBADREAD, 1 );
      }
    else {  /* read was successful */
      eyecatcher_incorrect = 
               memcmp( (void *) local_recptr->infile_buf_ptr,
                       (void *) jfs_chklog_eyecatcher,
                        16 );       /* returns 0 if equal */
      if( eyecatcher_incorrect) {  /* incorrect 1st 16 chars */
        openif_rc = XCHKDMP_BADFORMAT;
        send_msg( fsck_XCHKDMPBADFORMAT, 1 );
        }  /* end incorrect 1st 16 chars */
      else {
        local_recptr->infile_buf_data_len = 16;
        }
      }  /* end else read was successful */
    }  /* end so far so good */

  return( openif_rc );
}                             /* end of open_infile ( ) */


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
 *      success: XCHKDMP_OK
 *      failure: something else
 */
int32 parse_parms ( int32 argc, char **argv )
{
  int32 pp_rc = XCHKDMP_OK;
  int       i = 0;
  int       log_parm_seen = 0;
  int       bad_log_parm = 0;
  char      *lvolume = NULL;
  char      *argp;
  int       infile_name_len = 0;

  for (i = 1; i < argc; i++) {  /* for all parms on command line */
    argp = argv[i];

    if (*argp == '/' || *argp == '-') {  /* leading / or - */
      argp++;

      if (*argp == 'f' || *argp == 'F')  {  /* input file specifier */

        argp++;

        if( *argp == '\0' ) {  /* nothing follows the F|f */
                /*
                 * take the default output file name
                 */
          local_recptr->infile_name_specified = 0;
          }  /* end nothing follows the F|f */

        else {  /* a file name specified */
          local_recptr->infile_name_specified = -1;
          infile_name_len = strlen( argp );
          if( infile_name_len > 128 ) {  /* filename too long */
            send_msg( fsck_XCHKDMPBADFNAME, 0 );
            return( XCHKDMP_FAILED );
            }  /* end filename too long */
          else {  /* go with the specified file name */
            memcpy( (void *) infile_name, 
                    (void *) argp, 
                    infile_name_len 
                   );
            }  /* end else go with the specified file name */
          }  /* end else a file name specified */
        }  /* end output file specifier */

      else {  /* unrecognized keyword parm */
        msgprms[0] = argp;
        msgprmidx[0] = 0;
        send_msg( fsck_PRMUNSUPPENUM, 1 );
        send_msg( fsck_XCHKDMPUSAGE, 0 );
        return( XCHKDMP_FAILED );
        }  /* end unrecognized keyword parm */
      }  /* end leading / or - */

    else {  /* unrecognized parm */
      msgprms[0] = argp;
      msgprmidx[0] = 0;
      send_msg( fsck_PRMUNSUPPENUM, 1 );
      send_msg( fsck_XCHKDMPUSAGE, 1 );
      return( XCHKDMP_FAILED );
      }  /* end unrecognized parm */
    }  /* end for all parms on command line */

  return( pp_rc );
}                            /* end of parse_parms()  */


/*****************************************************************************
 * NAME: send_msg
 *
 * FUNCTION: Issue an fsck message, depending on the message's protocol
 *           according to the fsck message arrays (above).
 *
 * PARAMETERS:
 *      ?                 - input -
 *      ?                 - returned -
 *
 * NOTES:  Any message designated fsck_debug is english-only, and
 *         is issued in english, even if it is sent to stdout.
 *        
 *         All other messages are local-language (in the nls context)
 *         and, when sent to stdout or stderr, are issued in the 
 *         local language.  
 *
 *
 * RETURNS:
 *	nothing
 */
void send_msg( int  msg_num, int  num_inserts )
{
  int prmidx;

  sprintf( msgs_txt_ptr, MsgText[ msg_num ], 
           msgprms[0], msgprms[1], msgprms[2], msgprms[3], msgprms[4], 
           msgprms[5], msgprms[6], msgprms[7], msgprms[8], msgprms[9]  );

  fprintf( stdout, msgs_txt_ptr );

  return;
}                                        /* end send_msg() */
