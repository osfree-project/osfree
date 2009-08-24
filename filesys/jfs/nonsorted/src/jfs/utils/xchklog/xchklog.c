/* $Id: xchklog.c,v 1.2 2004/03/21 02:43:36 pasha Exp $ */

static char *SCCSID = "@(#)1.5  2/9/99 09:09:04 src/jfs/utils/xchklog/xchklog.c, jfsutil, w45.fs32, 990417.1";
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
 *      This tool provides a way for the customer to extract a log
 *      from the chkdsk workspace on the partition into a file which
 *      can be sent to IBM service.
 *
 *      This routine extracts the contents of the specified (or implied)
 *      chkdsk service log on the specified device.  The output is written
 *      to a file (name specified or defaulted).  
 *
 *      USAGE:  
 *
 *         xchklog [-L[:N|:P]] [-F<output.fil>] Device
 *
 *         where: o <output.fil> will be created in the present working
 *                  directory unless it is fully qualified
 *                o <output.fil> must be 127 characters or less in length
 *
 *      SAMPLE INVOCATIONS:
 *
 *         To extract the most recent log on f: into \chkdsk.new         
 *                     chklog f:
 *
 *         To extract the most recent log on d: into <pwd>output.fil
 *                     chklog d: -L -Foutput.fil
 * 
 *         To extract the most recent log on f: into \temp\l970610.log
 *                     chklog f: -l:N -f\temp\l970610.log
 *
 *         To extract the prior log on g: into \chkdsk.old
 *                     chklog g: -L:p
 *
 *   FUNCTIONS:
 *		main
 *
 *		extract_service_log
 *		final_processing
 *		fscklog_fill_buffer
 *		initial_processing
 *		open_device_read
 *		open_outfile
 *		parse_parms
 *		readwrite_device
 *		record_msg
 *		send_msg
 *		validate_super
 *		validate_superblock
 *
 *
 */

/*
 * includes from JFS/Warp chkdsk
 */
#include "xfsck.h"            /* chkdsk constants */
#include "fsckwsp.h"          /* chkdsk workspace mappings */
#include "fsckmsgc.h"         /* message constants */
#include "fsckmsge.h"         /* message text, all messages, in english */
#include "fsckmsgp.h"         /* message processing protocols           */


#define fsck_READ  1
#define fsck_WRITE 2

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * record for xchklog processing
  *
  */
typedef struct chklog_record {
   char          eyecatcher[8];
   agg_byteidx_t ondev_wsp_byte_length;
   agg_byteidx_t ondev_wsp_byte_offset;
   reg_idx_t     ondev_wsp_fsblk_length;
   fsblkidx_t    ondev_wsp_fsblk_offset;
   reg_idx_t     ondev_fscklog_fsblk_length;
   fsblkidx_t    ondev_fscklog_fsblk_offset;
   agg_byteidx_t ondev_fscklog_byte_length;
   agg_byteidx_t ondev_fscklog_byte_offset;
   char         *infile_buf_ptr;
   reg_idx_t     infile_buf_length;
   reg_idx_t     infile_buf_data_len;
   agg_byteidx_t infile_agg_offset;
   agg_byteidx_t infile_log_offset;
   logent_hdptr  last_msghdr;
   char         *outfile_buf_ptr;
   reg_idx_t     outfile_buf_length;
   reg_idx_t     outfile_buf_data_len;
   int           ag_blk_size;   
   int8          which_log;
   int8          device_is_open;
   int8          outfile_is_open;
   int8          this_buffer_last_message;
   int8          fscklog_last_read;
   int8          outfile_name_specified;
   } chklog_record_t;

chklog_record_t  xchklog_record;
chklog_record_t *local_recptr;

#define  NEWLOG  1
#define  logfile_new  "chkdsk.new"
#define  OLDLOG  2
#define  logfile_old  "chkdsk.old"
#define  default_logfile_name_len  10
char     outfile_name[128];

FILE *outfp;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * superblock I/O buffer and pointer
  *
  */
struct superblock  aggr_superblock;
struct superblock *sb_ptr;


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * input:  fsck service log I/O buffer
  * output: chkdsk extracted service log I/O buffer
  *
  */
char fscklog_buffer[FSCKLOG_BUFSIZE];
char xchklog_buffer[XCHKLOG_BUFSIZE];

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For message processing
  *
  *    values are assigned by the main xchklog routine
  */

char *Vol_Label;

char message_parm_0[MAXPARMLEN];
char message_parm_1[MAXPARMLEN];
char message_parm_2[MAXPARMLEN];
char message_parm_3[MAXPARMLEN];
char message_parm_4[MAXPARMLEN];
char message_parm_5[MAXPARMLEN];
char message_parm_6[MAXPARMLEN];
char message_parm_7[MAXPARMLEN];
char message_parm_8[MAXPARMLEN];
char message_parm_9[MAXPARMLEN];

char *msgprms[10] = { message_parm_0, message_parm_1, message_parm_2,
                      message_parm_3, message_parm_4, message_parm_5,
                      message_parm_6, message_parm_7, message_parm_8,
                      message_parm_9 };

int16 msgprmidx[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

char message_buffer_0[512];
char *msgs_txt_ptr = &message_buffer_0[0];
ULONG msgs_txt_maxlen = 512;

DATETIME fsck_DateTime = {0};

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

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */

void      extract_service_log( void );

retcode_t final_processing ( void ) ;

retcode_t fscklog_fill_buffer ( void );

retcode_t initial_processing ( int32, char ** );

retcode_t open_device_read ( char * );

retcode_t open_outfile ( void );

retcode_t parse_parms ( int32, char ** );

retcode_t readwrite_device( agg_byteidx_t, uint32, uint32 *,
                            void *, int );

void record_msg( int, int );

void send_msg( int, int );

retcode_t validate_super ( int );

retcode_t validate_superblock ( void );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


 /*
  * module return code and possible values
  */

#define XCHKLOG_OK                  0
#define XCHKLOG_CANTOPENOUTFILE    -1

retcode_t rc = XCHKLOG_OK;

/*****************************************************************************
 * NAME: main
 *
 * FUNCTION: Entry point for jfs read aggregate chkdsk service log
 *
 * INTERFACE:  
 *            int32 xchklog <volume specifier>
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
retcode_t  main( int32 argc, char **argv )
{
  retcode_t rc = XCHKLOG_OK;
  int32     Vol_Label_Size;

  /*
   * some basic initializations
   */
  sb_ptr = &aggr_superblock;
  local_recptr = &xchklog_record;
  memset( (void *) outfile_name, 0, 128 );

  rc = initial_processing( argc, argv );

  if( rc == XCHKLOG_OK ) {  /* we have a buffer and the device is open */

    extract_service_log();
    }

  if( rc == XCHKLOG_OK ) {  /* no errors yet */
    rc = final_processing();
    }  /* end no errors yet */

  else {  /* something already went wrong */
    final_processing();
    }  /* something already went wrong */

    return( rc );
}                                /* end of main()   */

/* ======================================================================*/


/***************************************************************************
 * NAME: extract_service_log
 *
 * FUNCTION:  
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: XCHKLOG_OK
 *      failure: something else
 */
void extract_service_log( )
{
  retcode_t dsl_rc = XCHKLOG_OK;
  int       prmidx;
  char     *log_entry;
  int       log_entry_pos;
  int       bytes_left;
  logent_hdptr      hdptr;
  logent_prmtype    prmtype;
  logent_prmval     prmval;
  logent_prmstr_ptr prmlit;

  local_recptr->fscklog_last_read = 0;

  while( (!local_recptr->fscklog_last_read) && 
         (dsl_rc == XCHKLOG_OK)                   ) {

    dsl_rc = fscklog_fill_buffer();
    local_recptr->this_buffer_last_message = 0;
    while( (!local_recptr->this_buffer_last_message) && 
           (dsl_rc == XCHKLOG_OK)                         ) {

      hdptr = (logent_hdptr) 
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
              (hdptr->msg_num > fsck_highest_msgid_defined)   ) { /*
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
              prmtype = (int8 *) &(log_entry[log_entry_pos]);
              log_entry_pos += hdptr->num_parms * sizeof(int8);
              prmval = (int16 *) &(prmtype[hdptr->num_parms]);
              log_entry_pos += hdptr->num_parms * sizeof(int16);
              prmlit = (char *) &(log_entry[log_entry_pos]);

              prmidx = 0;
              while( prmidx < hdptr->num_parms ) {
                if( prmtype[prmidx] == fscklog_var_text ) {  /* 
                            * variable text insert 
                            */
                  msgprmidx[prmidx] = prmval[prmidx];
                  }  /* end variable text insert */

                else  {  /* string literal insert */
                  msgprmidx[prmidx] = 0;
                  msgprms[prmidx] = prmlit;
                  prmlit = (char *) 
                           ((int32) prmlit + (int32) prmval[prmidx]);
                  }  /* end string literal insert */
                prmidx += 1;
                }  /* end while prmidx */
              }  /* end there are parms */

               /* 
                * record the message in the output file
                */
            record_msg( hdptr->msg_num, hdptr->num_parms );
            }  /* end the the record looks ok */

             /* 
              * set up for the next record 
              */
          local_recptr->infile_buf_data_len += hdptr->entry_length;
if( hdptr->msg_num == fsck_SESSEND ) {
  local_recptr->this_buffer_last_message = -1;
  }
          if( local_recptr->infile_buf_data_len >= 
                       local_recptr->infile_buf_length ) {
            local_recptr->this_buffer_last_message = -1;
            }
          }  /* end else the log record length looks ok */
        }  /* end else this entry_length != 0 */
      }  /* end while !this_buffer_last_message */
    }  /* end while !fscklog_last_read  */

  return;
}                              /* end of extract_service_log ()  */


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
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t  final_processing ( )
{
  retcode_t pf_rc = XCHKLOG_OK;
  retcode_t of_rc = XCHKLOG_OK;

    /*
     * Close the device
     */
  if( local_recptr->device_is_open ) {

    pf_rc = DosClose( Dev_IOPort );

    free(ptrklay);
    }

    /*
     * Close the output file
     */
  if( local_recptr->outfile_is_open ) {
    /*
     * flush the buffer if necessary
     */
    if( local_recptr->outfile_buf_data_len != 0 ) {
      fwrite( (const void *) (local_recptr->outfile_buf_ptr),
              sizeof( char ), 
              local_recptr->outfile_buf_length,
              outfp
             );
      }
    of_rc = fclose( outfp );
    }

  return( pf_rc );
}                              /* end of final_processing ()  */


/*****************************************************************************
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
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t  fscklog_fill_buffer ( )
{
  retcode_t flfb_rc = XCHKLOG_OK;
  retcode_t io_rc = XCHKLOG_OK;
  uint32 bytes_read = 0;
  uint32 log_bytes_left;
  int32 num_log_errors;
  fscklog_error_t *log_error_recptr;

  io_rc = readwrite_device( local_recptr->infile_agg_offset,
                            local_recptr->infile_buf_length,
                            &bytes_read,
                            (void *) local_recptr->infile_buf_ptr,
                            fsck_READ
                             );

  if( (io_rc != XCHKLOG_OK) ||
      (bytes_read != (uint32) local_recptr->infile_buf_length) )  {  /*
                               * write failed or didn't read correct 
                               * number of bytes 
                               */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = fsck_metadata;
    msgprms[1] = Vol_Label;
    msgprmidx[1] = 0;
    send_msg( fsck_URCVWRT, 2 );

    sprintf( message_parm_0, "%d", FSCK_BADREAD_FSCKLOG );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%d", io_rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%d", fsck_READ );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    sprintf( message_parm_3, "%lld", local_recptr->infile_agg_offset );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    sprintf( message_parm_4, "%ld", local_recptr->infile_buf_length );
    msgprms[4] = message_parm_4;
    msgprmidx[4] = 0;
    sprintf( message_parm_5, "%ld", bytes_read );
    msgprms[5] = message_parm_5;
    msgprmidx[5] = 0;
    send_msg( fsck_ERRONLOG, 6 );
    }  /* end write failed or didn't write correct number of bytes */

      /*
       * We want to reset the buffer no matter what.
       * It is useful to keep going because the next section may be
       * readable. 
       */
  local_recptr->infile_agg_offset += local_recptr->infile_buf_length;
  local_recptr->infile_log_offset += local_recptr->infile_buf_length;
  local_recptr->infile_buf_data_len = 0;

  log_bytes_left = (local_recptr->ondev_fscklog_byte_length / 2) -
                   local_recptr->infile_log_offset;
  if( log_bytes_left < local_recptr->infile_buf_length ) {  /* 
                                * this is the last one
                                * 
                                */
    local_recptr->fscklog_last_read = -1;
    }  /* end this read was the last one for the log */

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
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t initial_processing ( int32 argc, char **argv )
{
  retcode_t  pi_rc = XCHKLOG_OK;

  /*
   * Initialize the xchklog control block
   */
  memset ( local_recptr, 0, sizeof(xchklog_record) );
  memcpy( (void *) &(local_recptr->eyecatcher), (void *) "chklog  ", 8 );
  local_recptr->infile_buf_length = FSCKLOG_BUFSIZE;
  local_recptr->infile_buf_ptr = fscklog_buffer;
  local_recptr->outfile_buf_length = XCHKLOG_BUFSIZE;
  local_recptr->outfile_buf_ptr = xchklog_buffer;

  /*
   * Process the parameters given by the user
   */
  pi_rc = parse_parms( argc, argv );  /* parse the parms and record
                                       * them in the aggregate wsp record
                                       */

  if( pi_rc == XCHKLOG_OK ) {  /* the parms are good */
     /*
      * Open the device and verify that it contains a valid JFS aggregate
      * If it does, check/repair the superblock.
      */
    pi_rc = open_device_read ( Vol_Label );

    if( pi_rc != XCHKLOG_OK ) {  /*device open failed */
      send_msg( fsck_CNTRESUPB, 0 );
      }  /* end device open failed */

    else {  /* device is open */

      local_recptr->device_is_open = 1;

      pi_rc = validate_superblock();

      if(pi_rc == XCHKLOG_OK) {    /* a valid superblock */

             /*
              * add some stuff to the local record which is based on
              * superblock fields
              */
        local_recptr->ondev_fscklog_byte_length = 
                             sb_ptr->s_fsckloglen * sb_ptr->s_bsize; /*
                                      * length of the on-device fsck
                                      * service log
                                      */
        local_recptr->ondev_fscklog_fsblk_length = sb_ptr->s_fsckloglen; /*
                                      * length of the on-device fsck
                                      * service log
                                      */
        local_recptr->ondev_wsp_fsblk_length = 
                                lengthPXD( &(sb_ptr->s_fsckpxd) ) - 
                                local_recptr->ondev_fscklog_fsblk_length; /*
                                      * length of the on-device fsck
                                      * workspace
                                      */
        local_recptr->ondev_wsp_byte_length =
                   local_recptr->ondev_wsp_fsblk_length * sb_ptr->s_bsize; /*
                                      * length of the on-device fsck
                                      * workspace
                                      */
        local_recptr->ondev_wsp_fsblk_offset =
                               addressPXD( &(sb_ptr->s_fsckpxd) ); /*
                                      * aggregate block offset of
                                      * the on-device fsck workspace
                                      */
        local_recptr->ondev_wsp_byte_offset =
                   local_recptr->ondev_wsp_fsblk_offset * sb_ptr->s_bsize; /*
                                      * byte offset of the on-device
                                      * fsck workspace
                                      */
        local_recptr->ondev_fscklog_fsblk_offset =
                               local_recptr->ondev_wsp_fsblk_offset + 
                               local_recptr->ondev_wsp_fsblk_length; /*
                                      * aggregate block offset of
                                      * the on-device fsck workspace
                                      */
        local_recptr->ondev_fscklog_byte_offset =
                               local_recptr->ondev_wsp_byte_offset + 
                               local_recptr->ondev_wsp_byte_length; /*
                                      * byte offset of the on-device
                                      * fsck workspace
                                      */
              /* 
               * The offsets now assume the most recent log is 1st in the
               * aggregate fsck service log space.  Adjust if needed.
               */
        if( local_recptr->which_log == NEWLOG ) {  /* most recent wanted */
          if( sb_ptr->s_fscklog == 2 ) {  /* the 2nd is most recent */
            local_recptr->ondev_fscklog_fsblk_offset +=
                                   local_recptr->ondev_fscklog_fsblk_length / 2;
            local_recptr->ondev_fscklog_byte_offset += 
                                   local_recptr->ondev_fscklog_byte_length / 2;
            }  /* end the 2nd is most recent */
          }  /* end most recent wanted */
        else {  /* previous log wanted */
          if( sb_ptr->s_fscklog != 2 ) {  /* the 2nd is not most recent */
            local_recptr->ondev_fscklog_fsblk_offset +=
                                   local_recptr->ondev_fscklog_fsblk_length / 2;
            local_recptr->ondev_fscklog_byte_offset += 
                                   local_recptr->ondev_fscklog_byte_length / 2;
            }  /* end the 2nd is not most recent */
          }  /* end else previous log wanted */

        local_recptr->infile_agg_offset = local_recptr->ondev_fscklog_byte_offset;

        pi_rc = open_outfile();

        }  /* end a valid superblock */
      }  /* end device is open */
    }  /* end parms are good */

  return( pi_rc );
}                              /* end of initial_processing ()  */


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
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t open_device_read ( char    *Device )
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

  if( openrd_rc == NO_ERROR ) {  /* now open for read, allow read/write */

        /*
         * Get the device parameters
         */
    openrd_rc = DosDevIOCtl(Dev_IOPort, IOCTL_DISK,
                            DSK_GETDEVICEPARAMS, &CommandInfo,
                            sizeof(CommandInfo), &ParmLen, &Datap,
                            sizeof(struct DPB), &DataLen);

    if( openrd_rc == NO_ERROR ) {
      Dev_SectorSize = Datap.dev_bpb.bytes_per_sector;
      Dev_blksize = Dev_SectorSize;
      ulSecsPerTrk = Datap.dev_bpb.sectors_per_track;
      ptrklay = (TRACKLAYOUT *)malloc(sizeof(TRACKLAYOUT) + 4 * ulSecsPerTrk );
 
      if( ptrklay == NULL ) {
        openrd_rc = FSCK_FAILED_DYNSTG_EXHAUST9;
        }
      else {
        for( i = 0; i < ulSecsPerTrk; i++ ) {
          ptrklay->TrackTable[i].usSectorSize = Dev_SectorSize;
          ptrklay->TrackTable[i].usSectorNumber = i + 1;
          }
        }
      }
    }   /* end successful open for read, allow read/write */

  return( openrd_rc );
}                             /* end of open_device_read ( ) */


/*****************************************************************************
 * NAME: open_outfile
 *
 * FUNCTION:  Open the output file.
 *
 * PARAMETERS:
 *      Device  - input - the device specification
 *
 * NOTES:
 *
 * RETURNS:
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t open_outfile ( )
{
  int32    openof_rc = 0;
  int32    close_rc = 0;
  char     eyecatcher[16];
  
  if( !local_recptr->outfile_name_specified ) {  /* no output file name given */
    outfile_name[0] = '\\';
    if( local_recptr->which_log == NEWLOG ) {  /* most recent wanted */
      memcpy( (void *) &(outfile_name[1]), 
              (void *) logfile_new, 
              default_logfile_name_len 
            );
      }  /* end most recent wanted */
    else {  /* prior log wanted */
      memcpy( (void *) &(outfile_name[1]), 
              (void *) logfile_old, 
              default_logfile_name_len 
            );
      }  /* end prior log wanted */
    }  /* end no output file name given */

  msgprms[0] = outfile_name;
  msgprmidx[0] = 0;

  outfp = fopen( outfile_name, "w");
  if( outfp == NULL ) {  /* output file open failed */
    openof_rc = XCHKLOG_CANTOPENOUTFILE;
    send_msg( fsck_XCHKLOGOPNFAIL, 1 );
    }  /* end output file open failed */
  else {  /* output file is open */
    local_recptr->outfile_is_open = -1;

       /*
        * write the eyecatcher into the output buffer
        */
    memcpy( (void *) (local_recptr->outfile_buf_ptr), 
            (void *) jfs_chklog_eyecatcher, 
            16
           );
    local_recptr->outfile_buf_data_len = 16;

       /*
        * announce the output file name
        */
    if( local_recptr->which_log == NEWLOG ) {  /* most recent */
      send_msg( fsck_XCHKLOGNEW, 1 );
      }  /* end most recent */
    else {  /* previous */
      send_msg( fsck_XCHKLOGOLD, 1 );
      }  /* end else previous */
    }  /* end else output file is open */

  return( openof_rc );
}                             /* end of open_outfile ( ) */


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
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t parse_parms ( int32 argc, char **argv )
{
  retcode_t pp_rc = XCHKLOG_OK;
  int       i = 0;
  int       log_parm_seen = 0;
  int       bad_log_parm = 0;
  char      *lvolume = NULL;
  char      *argp;
  int       outname_len = 0;

  for (i = 1; i < argc; i++) {  /* for all parms on command line */
    argp = argv[i];

    if (*argp == '/' || *argp == '-') {  /* leading / or - */
      argp++;

      if (*argp == 'f' || *argp == 'F')  {  /* output file specifier */

        argp++;

        if( *argp == '\0' ) {  /* nothing follows the F|f */
                /*
                 * take the default output file name
                 */
          local_recptr->outfile_name_specified = 0;
          }  /* end nothing follows the F|f */

        else {  /* a file name specified */
          local_recptr->outfile_name_specified = -1;
          outname_len = strlen( argp );
          if( outname_len > 128 ) {  /* filename too long */
            send_msg( fsck_XCHKLOGBADFNAME, 0 );
            return( FSCK_FAILED );
            }  /* end filename too long */
          else {  /* go with the specified file name */
            memcpy( (void *) outfile_name, 
                    (void *) argp, 
                    outname_len 
                   );
            }  /* end else go with the specified file name */
          }  /* end else a file name specified */
        }  /* end output file specifier */

      else if (*argp == 'l' || *argp == 'L')  {  /* log selector */

        argp++;
        log_parm_seen = -1;

        if( *argp == '\0' ) {  /* nothing follows the L|l */
                /*
                 * selects most recent log 
                 */
          local_recptr->which_log = NEWLOG; 
          }  /* end nothing follows the F|f */

        else if( *argp == ':' ) {  /* a log selector value */
          argp++;
          if( *(argp + 1) == '\0' ) {  /* a single char follows the colon */
            if( (*argp == 'n') || (*argp == 'N') ) {  /* selects most recent log */
              local_recptr->which_log = NEWLOG; 
              }  /* end selects most recent log */
            else if( (*argp == 'p') || (*argp == 'P') ) {  /* selects previous log */
              local_recptr->which_log = OLDLOG; 
              }  /* end selects previous log */
            else {  /* given log selector is not valid */
              bad_log_parm = -1;
              }  /* end given log selector is not valid */
            }  /* end a single char follows the colon */
          else {  /* given log selector is not valid */
            bad_log_parm = -1;
            }  /* end given level number is not valid */
          }  /* end a level number will be given */
        else {  /* something not valid follows the L|l */
          bad_log_parm = -1;
          }  /* end something not valid follows the L|l */

        if( bad_log_parm ) {  /* don't know what they want */
          msgprms[0] = argv[i];
          msgprmidx[0] = 0;
          send_msg( fsck_PRMUNRECOPTION, 1 );
          return( FSCK_FAILED );
          }  /* end don't know what they want */

        }  /* end log selector */

      else {  /* unrecognized keyword parm */
        msgprms[0] = argp;
        msgprmidx[0] = 0;
        send_msg( fsck_PRMUNSUPPENUM, 1 );
        send_msg( fsck_XCHKLOGUSAGE, 0 );
        return( FSCK_FAILED );
        }  /* end unrecognized keyword parm */
      }  /* end leading / or - */

    else if (argp[1] == ':') {  /* 2nd char is : */
      lvolume = argp;
      }  /* end 2nd char is : */

    else {  /* unrecognized parm */
      msgprms[0] = argp;
      msgprmidx[0] = 0;
      send_msg( fsck_PRMUNSUPPENUM, 1 );
      send_msg( fsck_XCHKLOGUSAGE, 1 );
      return( FSCK_FAILED );
      }  /* end unrecognized parm */
    }  /* end for all parms on command line */

  if (lvolume == NULL) {  /* no device specified */
    send_msg( fsck_XCHKLOGNOVOL, 0 );
    send_msg( fsck_XCHKLOGUSAGE, 0 );
    return( FSCK_FAILED );
    }  /* end no device specified */
  else {  /* got a device */
    Vol_Label = lvolume;
    }  /* end got a device */

  if( !log_parm_seen ) {  /* no log selected */
    local_recptr->which_log = NEWLOG; 
    }  /* end no log selected */

  if( !bad_log_parm ) {  /* a log is selected */
    if( local_recptr->which_log == NEWLOG ) {
      send_msg( fsck_CHKLOGNEW, 0 ); 
      }
    else {
      send_msg( fsck_CHKLOGOLD, 0 ); 
      }
    }  /* end no log selected */


  return( pp_rc );
}                            /* end of parse_parms()  */


/*****************************************************************************
 * NAME: readwrite_device
 *
 * FUNCTION:  Read data from or write data to the device on which the 
 *            aggregate resides.
 *
 * PARAMETERS:
 *      dev_offset           - input - the offset, in bytes, into the aggregate
 *                                     of the data to read or to which to write
 *                                     the data.
 *      requested_data_size  - input - the number of bytes requested
 *      actual_data_size     - input - pointer to a variable in which to return
 *                                     the number of bytes actually read or
 *                                     written
 *      data_buffer          - input - the address of the buffer in which to
 *                                     put the data or from which to write
 *                                     the data
 *      mode                 - input - { fsck_READ | fsck_WRITE }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t readwrite_device( agg_byteidx_t  dev_offset,
                            uint32         requested_data_size,
                            uint32        *actual_data_size,
                            void          *data_buffer,
                            int            mode
                           )
{
  retcode_t rwdb_rc = XCHKLOG_OK;
  uint32 actual_seek_addr;
  uint32 actual_bytes_moved = 0;
  uint32 requested_bytes_moved;

  ULONG ulPhys_sector;
  ULONG ulNumSectors;
  ULONG ulSectorsPerCylinder;
  USHORT ulSecsPerTrk;
  USHORT numSecs, sCylinder, sHead, sFirstSec;
  uint32 ulbytesPsec;
  unsigned long	ParmLen ;
  struct DPB *Pdpb;
  TRACKLAYOUT *tmp_layout;
  char *ptr;

  *actual_data_size = 0;

  Pdpb = &Datap;
  ulbytesPsec = Pdpb->dev_bpb.bytes_per_sector;

  if ( (dev_offset % ulbytesPsec) || (requested_data_size % ulbytesPsec ) ) {
    rwdb_rc = FSCK_FAILED_SEEK;
    }

  else {  /* offset seems ok */
    tmp_layout = ptrklay;
    ulSecsPerTrk =  Pdpb->dev_bpb.sectors_per_track;
    ParmLen = sizeof(TRACKLAYOUT);
    ulPhys_sector = dev_offset / Pdpb->dev_bpb.bytes_per_sector;
    ulPhys_sector += Pdpb->dev_bpb.hidden_sectors;
    ulSectorsPerCylinder = ulSecsPerTrk * Pdpb->dev_bpb.number_of_heads;  
    sCylinder = (USHORT)((ulPhys_sector) / ulSectorsPerCylinder);

    if ( sCylinder > Pdpb->number_of_tracks )  {
      rwdb_rc = FSCK_FAILED_BADSEEK;
      }

    else {  /* cylinder number seems ok */

      ptr = (char *)data_buffer;
      ulNumSectors = requested_data_size / Pdpb->dev_bpb.bytes_per_sector;
      sHead = (USHORT)((ulPhys_sector % ulSectorsPerCylinder) / ulSecsPerTrk);
      sFirstSec = (USHORT)
                  ((ulPhys_sector % ulSectorsPerCylinder) % ulSecsPerTrk);
      ptrklay->bCommand = 0;
      ptrklay->usCylinder = sCylinder;

      while ( ptrklay->usCylinder < Pdpb->number_of_tracks ) {

        ptrklay->usHead = sHead;

        while ( ptrklay->usHead  < Pdpb->dev_bpb.number_of_heads )  {

          ptrklay->usFirstSector =  sFirstSec;
          numSecs = ulSecsPerTrk - ptrklay->usFirstSector;
          ptrklay->cSectors = (numSecs > ulNumSectors)  ? ulNumSectors 
                                                        : numSecs;
          requested_bytes_moved = ptrklay->cSectors * ulbytesPsec;

          switch( mode ) {
            case fsck_READ:
              rwdb_rc = DosDevIOCtl( Dev_IOPort, IOCTL_DISK, DSK_READTRACK, 
                                    (PVOID)ptrklay, ParmLen, &ParmLen, 
                                    (void *)ptr, 
                                    (ULONG )(requested_bytes_moved),   
                                    (PULONG) &actual_bytes_moved );
              break;

            case fsck_WRITE:
              rwdb_rc = DosDevIOCtl( Dev_IOPort, IOCTL_DISK, DSK_WRITETRACK,
                                     (PVOID)ptrklay, ParmLen, &ParmLen, 
                                     (void *)ptr, 
                                     (ULONG)(requested_bytes_moved),
                                     (PULONG) &actual_bytes_moved );
              break;

            default:  /* it's a bad call */
              rwdb_rc = FSCK_INTERNAL_ERROR_3;
	      break;
            }  /* end it's a bad call */

          if( rwdb_rc != XCHKLOG_OK ) {
            return rwdb_rc;
            }

          *actual_data_size += actual_bytes_moved;

          ulNumSectors -= ptrklay->cSectors;

          if ( ulNumSectors == 0 )
            goto outloop;
          else {
            ptr += *actual_data_size;
            ptrklay->usHead++;
            sFirstSec = 0;     /* for the next track, starting from sector 0 */
            }  /* end else */
          }  /* end while */

	ptrklay->usCylinder++;
	sHead = 0;  /* for the next cylinder, starting from head 0 */ 
        }  /* end while */

      if ( ulNumSectors > 0 ) {
        rwdb_rc =  FSCK_INTERNAL_ERROR_53;
        }
      }  /* end else cylinder number seems ok */
    }  /* end else offset seems ok */

outloop:

  return( rwdb_rc );
}                    /* end of readwrite_device ()       */


/*****************************************************************************
 * NAME: record_msg
 *
 * FUNCTION: Record an fsck service log message in the output file, 
 *           formatted for the common chkdsk service log display tool.
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
 *         Regardless of the language used to send a message to 
 *         stdout and stderr, messages are ALWAYS LOGGED IN ENGLISH.
 *
 * RETURNS:
 *	nothing
 */
void record_msg( int mess_num, int num_parms )
{
  int prmidx;
  char log_entry[4096];
  int entry_length = sizeof(chklog_entry_hdr_t);
  chklogent_hdptr      hdptr;
  chklogent_prmlen     prmlen;
  chklogent_prmstr_ptr prmlit;
  int32  buffer_bytes_left;

      /* 
       * lookup any inserts not specified as literals
       */
  memset( (void *) msgs_txt_ptr, '\0', msgs_txt_maxlen );
  prmidx = 0;
  while( prmidx < num_parms ) {
    if( msgprmidx[prmidx] != 0 ) {  /* variable text insert */
      msgprms[prmidx] = MsgText[ msgprmidx[prmidx] ];
      }  /* end variable text insert */
    prmidx += 1;
    }

  hdptr = (chklogent_hdptr) log_entry;

  hdptr->msg_num = mess_num + JFSCHKLOG_FIRSTMSGNUM;
  hdptr->num_parms = num_parms;

  if( num_parms != 0 ) {  /* there are inserts */
    prmlen = (int16 *) &(log_entry[entry_length]);
    entry_length += num_parms * sizeof(int16);
    prmlit = (char *) &(log_entry[entry_length]);

    prmidx = 0;
    while( prmidx < num_parms ) {
      prmlen[prmidx] = strlen(msgprms[prmidx]);
      strncpy( prmlit, msgprms[prmidx], prmlen[prmidx] );
      entry_length += prmlen[prmidx];
           /*
            * add a null terminator to the literal
            */
      log_entry[entry_length] = '\0';
      prmlen[prmidx] += 1;
      entry_length += 1;
           /*
            * set up for the next literal
            */
      prmlit = (char *) &(log_entry[entry_length]);

      prmidx += 1;
      }  /* end while */
    }  /* end there are inserts */

  entry_length = ((entry_length + 3) / 4 ) * 4;  /*
                         * round the length up so the next
                         * entry will start on a doubleword
                         * (4 byte) boundary
                         */

  hdptr->entry_length = entry_length; 

  buffer_bytes_left = local_recptr->outfile_buf_length -
                      local_recptr->outfile_buf_data_len;
  if( buffer_bytes_left < entry_length ) {  /* won't fit */
    local_recptr->last_msghdr->entry_length += buffer_bytes_left;
    fwrite( (const void *) (local_recptr->outfile_buf_ptr),
            sizeof( char ), 
            local_recptr->outfile_buf_length,
            outfp
           );
    memset( (void *) (local_recptr->outfile_buf_ptr),
             0, 
             local_recptr->outfile_buf_length );  /*
                         * clear the buffer 
                         */
    local_recptr->outfile_buf_data_len = 0;
    }  /* end won't fit */

  local_recptr->last_msghdr = (struct fscklog_entry_hdr *)
                                 ( (int32) local_recptr->outfile_buf_ptr +
                                   (int32) local_recptr->outfile_buf_data_len 
                                  );
  memcpy( (void *) local_recptr->last_msghdr,
          (void *) hdptr,
          entry_length  );

  local_recptr->outfile_buf_data_len += entry_length;

  return;
}                                        /* end record_msg() */


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

      /* 
       * assemble the message 
       */
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

      /*
       * issue the message in the local language if it has been 
       * translated, otherwise issue it in english
       */
  if( MsgProtocol[msg_num][MP_STREAM] == fsck_out ) {
    fprintf( stdout, msgs_txt_ptr );
    }
  else {
    fprintf( stderr, msgs_txt_ptr );
    }

  return;
}                                        /* end send_msg() */


/*****************************************************************************
 * NAME: validate_super
 *
 * FUNCTION:  This routine validates the JFS superblock currently in the
 *            buffer.  If any problem is detected, the which_superblock
 *            input parm is used to tailor the message issued to notify
 *            the user.
 *
 * PARAMETERS:
 *      which_super  - input - specifies the superblock on which is in the
 *                             buffer { fsck_primary | fsck_secondary }
 *
 * NOTES: 
 *
 * RETURNS:
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t validate_super ( int which_super )
{
  retcode_t vs_rc = 0;               /* assume the superblock is ok */
  retcode_t ujfs_rc = 0;
  agg_byteidx_t bytes_on_device, hidden_bytes_on_device;
  agg_byteidx_t s_size_inbytes, sum_inbytes;
  fsblkidx_t agg_blks_in_aggreg, agg_blks_on_device, dev_blks_on_device;
  fsblkidx_t fsck_start_from_pxd, fsck_blkmap_start_blks;
  reg_idx_t fsck_length_from_pxd, fsck_blkmap_size_blks, fsck_blkmap_size_pages;
  fsblkidx_t jlog_start_from_pxd;
  reg_idx_t jlog_length_from_pxd;
  int32 agl2size;
  uint32 expected_flag = JFS_OS2|JFS_GROUPCOMMIT|JFS_INLINELOG;
  uint32 agsize;
  int bad_bsize = 0;

  if( memcmp(sb_ptr->s_magic, JFS_MAGIC,
               sizeof(sb_ptr->s_magic)) != 0 ) {
    vs_rc = FSCK_BADSBMGC;           /* problem detected */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = which_super;
    send_msg( fsck_BADSBMGC, 1 );
    }  /* end if bad magic number */

  else if( sb_ptr->s_version != JFS_VERSION ) {
    vs_rc = FSCK_BADSBVRSN;          /* problem detected */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = which_super;
    send_msg( fsck_BADSBVRSN, 1 );
    }  /* end if bad version number */

  else  {  /* the magic number and version number are correct so it
            * probably is a JFS superblock with the format we are expecting
            */
    ujfs_rc = ujfs_get_dev_size( Dev_IOPort, 
                                 &bytes_on_device,
                                 &hidden_bytes_on_device );   /*
                                       * get physical device size
                                       */
    dev_blks_on_device = bytes_on_device / Dev_blksize;

    if( sb_ptr->s_pbsize != Dev_blksize ) {
      vs_rc = FSCK_BADSBOTHR1;       /* problem detected */
      msgprms[0] = "1";
      msgprmidx[0] = 0;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = which_super;
      send_msg( fsck_BADSBOTHR, 2 );
      }  /* end if physical blocksize is incorrect */

    if( sb_ptr->s_l2pbsize != log2shift( Dev_blksize ) ) {
      vs_rc = FSCK_BADSBOTHR2;       /* problem detected */
      msgprms[0] = "2";
      msgprmidx[0] = 0;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = which_super;
      send_msg( fsck_BADSBOTHR, 2 );
      }  /* end if log 2 of physical blocksize is incorrect */

    if( !inrange( sb_ptr->s_bsize, 512, 4096 ) ) {
      bad_bsize = -1;
      vs_rc = FSCK_BADSBOTHR3;       /* problem detected */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = which_super;
      send_msg( fsck_BADSBBLSIZ, 1 );
      }  /* end if the filesystem block size is not an integral power of 2
            * and between 512 and 4096
            */

    else {  /* else the filesystem block size is a legal value */

      if( sb_ptr->s_l2bsize != log2shift( sb_ptr->s_bsize ) ) {
        vs_rc = FSCK_BADSBOTHR4;     /* problem detected */
        msgprms[0] = "4";
        msgprmidx[0] = 0;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = which_super;
        send_msg( fsck_BADSBOTHR, 2 );
        }  /* end if log2 of block size is incorrect */

      if( sb_ptr->s_l2bfactor !=
          log2shift( sb_ptr->s_bsize / Dev_blksize ) ) {
        vs_rc = FSCK_BADSBOTHR5;     /* problem detected */
        msgprms[0] = "5";
        msgprmidx[0] = 0;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = which_super;
        send_msg( fsck_BADSBOTHR, 2 );
        }  /* end if log2 of the ratio between logical and physical
            * blocks is incorrect
            */

      if( sb_ptr->s_bsize < Dev_blksize ) {
        bad_bsize = -1;
        vs_rc = FSCK_BLSIZLTLVBLSIZ; /* problem detected */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_super;
        send_msg( fsck_BLSIZLTLVBLSIZ, 1 );
        }  /* end if filesystem block size is less than device block size */
      }  /* end else the filesystem block size is an acceptable value */

    if( !bad_bsize ) {
      agg_blks_on_device = bytes_on_device / sb_ptr->s_bsize;
      }

    if( sb_ptr->s_size > dev_blks_on_device ) {
      vs_rc = FSCK_BADSBFSSIZ;       /* problem detected */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = which_super;
      send_msg( fsck_BADSBFSSIZ, 1 );
      }  /* end if size in phys blocks is bigger than whole device */

#ifdef	_JFS_DFS_LFS
    s_size_inbytes = sb_ptr->s_size * Dev_blksize;
    sum_inbytes = (int64)(sb_ptr->totalUsable * 1024) +
                  (int64)(sb_ptr->minFree * 1024);
    if( (sum_inbytes > s_size_inbytes) ||
        ( (s_size_inbytes - sum_inbytes) >= 1024 ) 
      ) { /* the sum is greater or the difference is at least 1K */
      vs_rc = FSCK_BADBLKCTTTL;      /* problem detected */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = which_super;
      send_msg( fsck_BADBLKCTTTL, 1 );
      }  /* end the sum is greater or the difference is at least 1 K */
#endif	/* _JFS_DFS_LFS */
	
      if( (sb_ptr->s_flag&expected_flag)!= expected_flag ) {
        vs_rc = FSCK_BADSBOTHR6;     /* problem detected */
        msgprms[0] = "6";
        msgprmidx[0] = 0;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = which_super;
        send_msg( fsck_BADSBOTHR, 2 );
        }  /* end if allocation group is too small (must be at least 32M) */

    if( sb_ptr->s_agsize < (1 << L2BPERDMAP) ) {
      vs_rc = FSCK_BADSBAGSIZ;       /* problem detected */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = which_super;
      send_msg( fsck_BADSBAGSIZ, 1 );
      }  /* end if allocation group is too small (must be at least 32M) */

    else {  /* else the alloc group size is possibly correct */
      agg_blks_in_aggreg = sb_ptr->s_size * sb_ptr->s_pbsize / sb_ptr->s_bsize;
      agl2size = ujfs_getagl2size( agg_blks_in_aggreg, sb_ptr->s_bsize );
      agsize = (int64) 1 << agl2size; /* get the allocation group size */
      if( sb_ptr->s_agsize != agsize ) {
        vs_rc = FSCK_BADAGFSSIZ;     /* problem detected */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_super;
        send_msg( fsck_BADSBAGSIZ, 1 );
        }  /* end if size in phys blocks is bigger than whole device */
      }  /* end else the alloc group size is possibly correct */

    if( !vs_rc ) {  /* no problems detected in this superblock yet */
        /*
         * check out the fsck in-aggregate workspace 
         */
      fsck_length_from_pxd = lengthPXD(&(sb_ptr->s_fsckpxd));
      fsck_start_from_pxd = addressPXD(&(sb_ptr->s_fsckpxd));

      agg_blks_in_aggreg =  fsck_length_from_pxd +
                     (sb_ptr->s_size * sb_ptr->s_pbsize / sb_ptr->s_bsize);

      if( agg_blks_in_aggreg > agg_blks_on_device ) {  /* wsp length is bad */
        vs_rc = FSCK_BADSBFWSL1;      /* problem detected */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_super;
        send_msg( fsck_BADSBFWSL1, 1 );
        }  /* end wsp length is bad */

      else {  /* wsp length is plausible */
        fsck_blkmap_size_pages =
              ((agg_blks_in_aggreg + (BITSPERPAGE - 1)) / BITSPERPAGE) + 1 + 50;
        fsck_blkmap_size_blks =
                  (fsck_blkmap_size_pages << L2PSIZE) / sb_ptr->s_bsize; /*
                                      * size in aggregate blocks
                                      */
        fsck_blkmap_start_blks = agg_blks_in_aggreg - fsck_blkmap_size_blks; /*
                                      * aggregate block offset of the fsck
                                      * workspace in the aggregate.
                                      */
        if( fsck_length_from_pxd != fsck_blkmap_size_blks ) { /*
                                      * length of fsck in-aggregate workspace
                                      * is incorrect
                                      */
          vs_rc = FSCK_BADSBFWSL;      /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          send_msg( fsck_BADSBFWSL, 1 );
          }  /* end length of fsck in-aggregate workspace is incorrect */

        if( fsck_start_from_pxd != fsck_blkmap_start_blks ) { /*
                                      * address of fsck in-aggregate workspace
                                      * is incorrect
                                      */
          vs_rc = FSCK_BADSBFWSA;      /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          send_msg( fsck_BADSBFWSA, 1 );
          }  /* end address of fsck in-aggregate workspace is incorrect */
        }  /* end else wsp length is plausible */
      }  /* end no problems detected in this superblock yet */

    if( !vs_rc ) {  /* no problems detected in this superblock yet */
        /*
         * check out the in-aggregate journal log
         *
         * if there is one it starts at the end of the fsck
         * in-aggregate workspace.
         */
      jlog_length_from_pxd = lengthPXD(&(sb_ptr->s_logpxd));
      jlog_start_from_pxd = addressPXD(&(sb_ptr->s_logpxd));

      if( jlog_start_from_pxd != 0 ) {  /* there's one in there */

        if( jlog_start_from_pxd != agg_blks_in_aggreg ) { /*
                                      * address of in-aggregate journal log
                                      * is incorrect
                                      */
          vs_rc = FSCK_BADSBFJLA;      /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          send_msg( fsck_BADSBFJLA, 1 );
          }  /* end address of in-aggregate journal log is incorrect */

        agg_blks_in_aggreg +=  jlog_length_from_pxd;
        if( agg_blks_in_aggreg > agg_blks_on_device ) {  /* log length is bad */
          vs_rc = FSCK_BADSBFJLL;      /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          send_msg( fsck_BADSBFJLL, 1);
          }  /* end log length is bad */
        }  /* end there's one in there */
      }  /* end no problems detected in this superblock yet */

    }  /* end else the magic number and version number are correct so it
        * probably is a JFS superblock with the format we are expecting
        */

  return( vs_rc );
}                              /* end of validate_super ()  */


/*****************************************************************************
 * NAME: validate_superblock
 *
 * FUNCTION:  Verify that the primary superblock is valid.  
 *            If not, verify that the secondary superblock is valid.
 *
 * PARAMETERS:  none
 *
 * NOTES:  If this routine returns XCHKLOG_OK then the superblock
 *         I/O buffer contains a valid superblock.
 *
 * RETURNS:
 *      success: XCHKLOG_OK
 *      failure: something else
 */
retcode_t validate_superblock ( )
{
  retcode_t vsb_rc = XCHKLOG_OK;
  retcode_t intermed_rc = XCHKLOG_OK;
  UniChar *uni_result_ptr;
  int primary_sb_bad = 1;           /* assume a problem with primary */
  int secondary_sb_bad = 1;         /* assume a problem with secondary */
  int which_sb;

  vsb_rc = ujfs_get_superblk(Dev_IOPort, sb_ptr, 1); /* get primary  */

  if( vsb_rc != XCHKLOG_OK ) { /* if read primary fails */
    send_msg( fsck_CNTRESUPP, 0 );
    }
  else {   /* got primary superblock */
    which_sb = fsck_primary;
    primary_sb_bad = validate_super( fsck_primary );
    }

  if( primary_sb_bad ) {  /* can't use the primary superblock */
    send_msg( fsck_SBBADP, 0 );

    vsb_rc = ujfs_get_superblk(Dev_IOPort, sb_ptr, 0); /* get 2ndary */

    if( vsb_rc != XCHKLOG_OK ) {
      send_msg( fsck_CNTRESUPS, 0 );
      }
    else {    /* got secondary superblock */
      which_sb = fsck_secondary;
      secondary_sb_bad = validate_super( fsck_secondary );
      }

    if( !secondary_sb_bad ) {  /* secondary is ok */
      vsb_rc = XCHKLOG_OK;
      }
    else {
      send_msg( fsck_SBBADS, 0 );
      }
    }  /* end can't use the primary superblock */

  if( (!primary_sb_bad) || (!secondary_sb_bad) )  {   /*
                                      * the buffer holds a valid superblock
                                      */   
    local_recptr->ag_blk_size = sb_ptr->s_bsize; /* aggregate block size */

    msgprms[0] = message_parm_0;
    msgprmidx[0] = which_sb;
    send_msg( fsck_XCHKLOGSBOK, 1 );
    }

  return( vsb_rc );
}                              /* end of validate_superblock ()  */

