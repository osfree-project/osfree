/* $Id: message.c,v 1.1.1.1 2003/05/21 13:42:24 pasha Exp $ */

/**********************************************************************/
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
*/
/*                                                                    */
/*====================================================================*/
/*  message.c  - message functions for JFS Utilities                  */
/* -------------------------------------------------------------------*/
/*  DESCRIPTION:                                                      */
/*                                                                    */
/*   message_user function for common siple messaging                 */
/*                                                                    */
/*====================================================================*/
/*  REVISION HISTORY                                                  */
/*  ------------------------------------------------------------------*/
/**********************************************************************/

#define INCL_32
#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include <stdio.h>
#include <ctype.h>
#include "message.h"
#include "debug.h"


/* Global declarations */
int          pm_front_end = 0;
static char *message_file_name[2] = { "OSO001.MSG",
                                      "JFS.MSG"    };

/*  *** message_user -
 *
 *  This function is
 *
 *  message_user (mesg_no, param, param_cnt, device, response, msg_file)
 *
 *  ENTRY    mesg_no   - index into the array of messages
 *           param     - list of pointers to parameters in the message
 *           param_cnt - count of parameters in the param variable
 *           device    - where to write to (STDOUT, STDERR)
 *           response  - what kind of response is needed from user
 *           msg_file  - OSO_MSG/JFS_MSG which message file to use
 *
 *  EXIT     Returns pointer to buffer contain the user's response
 *           or NULL if no response needed.
 *
 *  CALLS
 */

char *message_user(unsigned mesg_no,
                   unsigned char  **param,
                   unsigned param_cnt,
                   unsigned device,
                   unsigned response,
                   int      msg_file)
{
  int      i;
  unsigned data_len;
  unsigned rc;
  char     data_buf[24];

  DBG_TRACE(("enter:  message_user(%d)\n",mesg_no));

  data_len = 24;

  if (pm_front_end)
  {
    printf ("\nMSG_%u", mesg_no);
    if (param_cnt != 0)
    {
      for (i = 0; i < param_cnt; i++)
      {
        printf (",%s", * (param + i));
      }
    }
    printf ("\n");
    fflush (stdout);
  }

  if (response == NO_RESPONSE)
    data_len = 0;

  rc = cmputmsg(param, param_cnt, mesg_no, message_file_name[msg_file],
                device, response, data_buf, data_len);
  if (rc != 0)
    DBG_ERROR(("message_user: I don't know what to say... \n"));

  DBG_TRACE(("exit:  message_user(%s)\n",data_buf));

  if (response == NO_RESPONSE)
    return(NULL);
  else
    return(data_buf);
}
