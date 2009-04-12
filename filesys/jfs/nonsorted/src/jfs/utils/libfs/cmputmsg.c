/* $Id: cmputmsg.c,v 1.2 2004/03/21 02:43:20 pasha Exp $ */

/*  *** cmputmsg.c ***
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
*/
#define  INCL_BSE
#define  INCL_KBD
#define  INCL_DOS

#include <os2.h>
//#include <bsedos.h>
//#include <bsesub.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "debug.h"


unsigned readinput(int);
unsigned getmessage(char **, unsigned, unsigned, char *);
int com_settbl( void );
int check_dbcs( unsigned char );

#define ERROR_MR_MSG_TOO_LONG  -1
#define ERROR_CM_EOF_REDIRECT  -4
#define DBCSVEC_SIZE            5
#define CASEMAP_SIZE            256

#define OK                0
#define FALSE             0
#define NO_RESPONSE       0               /*   no response                */
#define CR_RESPONSE       1               /*   carriage                   */
#define YES_NO_RESPONSE   2               /*   yes/no                     */
#define STRING_RESPONSE   3               /*   string input               */
#define ANY_KEY_RESPONSE  4               /*   any key                    */
#define ARI_RESPONSE      5               /*   abort/retry/ignore         */

#define YES_ANS           0               /*    Yes                       */
#define NO_ANS            1               /*    No                        */
#define ABORT_ANS         2               /*    Abort                     */
#define RETRY_ANS         3               /*    Retry                     */
#define IGNORE_ANS        4               /*    Ignore                    */
                                          /* flag control                 */
#define ON                1               /*    on                        */
#define OFF               0               /*    off                       */
                                          /* standard device handle       */
#define STDIN_HDL         0               /*    stdin                     */
#define STDOUT_HDL        1               /*    stdout                    */
#define STDERR_HDL        2               /*    stderr                    */

#define INIT_MSG_BUF_SIZE  200            /* initial msg buffer size      */
#define MSGFILENAME "OSO001.MSG"          /* system message filename      */
#define MAX_IVCOUNT     9                 /* maximun no of ivcount ( 9 )  */

#define KBDHANDLE       0                 /* keyboard handle = 0          */
                                          /* reponse char define related  */
#define RESPCHAR_NUM    5                 /*   number of resp char        */
#define YES_RS          0                 /*   pointer - yes              */
#define NO_RS           1                 /*             no               */
#define ABORT_RS        2                 /*             abort            */
#define RETRY_RS        3                 /*             retry            */
#define IGNORE_RS       4                 /*             ignore           */

#define NOT_SHARED      0                 /* not shared option            */
#define WORKBUF_SIZE   241                /* temporary work buffer to     */
                                          /* read response data           */

#define   BUFT_SIZE    512
#define   REAL_MODE      0
#define   PROTECT_MODE   1

static  char  msg_buft[BUFT_SIZE];
static  char  *msg_buf;
static  unsigned short msg_buf_size = INIT_MSG_BUF_SIZE;
static  unsigned long msglength;
static  unsigned short msg_buf_sel;
static  char respchar[RESPCHAR_NUM+1];

static  char init_req = ON;
static  char workbuf[WORKBUF_SIZE];       /* temporary work buffer        */
static  unsigned short bytes_read;        /* number of bytes from DOSREAD */
unsigned long byteswritten;
static  unsigned char machine_mode;
static  short rspswitch;
/* struct  KeyData  KeyBuf; */
static  KBDKEYINFO    KeyBuf;


/* APIRET  APIENTRY  Dos32QueryDBCSEnv( ULONG, PCOUNTRYCODE, PCHAR ); */
/* APIRET  APIENTRY  Dos32MapCase( ULONG, PCOUNTRYCODE, PCHAR ); */

static char DbcsFlags[128];

static struct DbcsEnvVector {
         unsigned char lo_limit;
         unsigned char up_limit;
       } dbcsvec[DBCSVEC_SIZE];

static unsigned char casemap_lo[CASEMAP_SIZE];
static unsigned char casemap_up[CASEMAP_SIZE];

/**********************************************************************/
/*     CMPUTMSG
/**********************************************************************/

unsigned cmputmsg(ivtable, ivcount, msgid, filename,
          devicetype, resptype, respdata, respdata_size)
char *ivtable[];                         /* table of varbs to insert    */
unsigned ivcount;                        /* number of variables         */
unsigned msgid;                          /* id of the message           */
char *filename;                          /* message filename strig      */
unsigned devicetype;                     /* output device type          */
unsigned resptype;                       /* type of response            */
char *respdata;                          /* response data area          */
unsigned respdata_size;                  /* response data area size     */
{
  int rc = OK;                           /* function return code        */
  unsigned drc = 0;                      /* extra return code varb      */
  int i, j;                              /* work                        */
  char   temp[10];

  if (devicetype == 0)
       devicetype = STDOUT_HDL;
    else
       devicetype = STDERR_HDL;

  if ( init_req == ON )
  {
     rc = getmessage( ivtable, 0, 0, MSGFILENAME);
     if ( rc != OK)
     {
       DBG_ERROR(("getmessage failed to find %s, rc = %d\n",MSGFILENAME,rc));
       return(rc);
     }

     i = j = 0;
     while (i < RESPCHAR_NUM)  {
        respchar[i] = msg_buft[j];
        i++;
        j += 2;
     }

    // debug
     memset(temp, '\0', 10);
     strncpy( temp, respchar, 5);


//   rc=DOSGETMACHINEMODE( &machine_mode );
//   if ( rc != OK ) {
//      printf ("DosGetMessage Error, rc = %d\n", rc );
//      return(rc);
//   }

     init_req = OFF;                      /*  reset init request         */
//   rc = com_settbl();                   /* DBCS initialization */
//   if (rc ) {
//     printf("Error with com_settbl, rc =%d\n", rc );
//     return( rc );
//   }

  }

  rc = getmessage(ivtable,ivcount, msgid, filename);
  if ( rc != OK ) {
     DBG_ERROR(("get message failed on getting msg from msgfile, rc =%d\n",rc));
     return(rc);
  }

  rc = DosPutMessage( devicetype, msglength, msg_buft);
  if ( rc != OK) {
     DBG_ERROR(("put msg failed, rc = %d\n",rc));
     return(rc);
  }

  rspswitch = ON;
  while ( rspswitch != OFF ) {
    switch (resptype) {

       case NO_RESPONSE:
         rspswitch = OFF;
         break;

       case CR_RESPONSE:
         if ((rc=readinput(resptype)) != OK)
             return(rc);
         workbuf[bytes_read - 2] = '\0';
         rspswitch = OFF;
         break;

       case YES_NO_RESPONSE:
       case ARI_RESPONSE:
         if ((rc=readinput(resptype))!= OK)
             return(rc);
         workbuf[bytes_read - 2] = '\0';
         /* com_strupr(workbuf); */
         strupr(workbuf);
         if (*workbuf == respchar[YES_RS])  {
             respdata[0] = YES_ANS;
             rspswitch = OFF;
             break;
         }
         if (*workbuf == respchar[NO_RS])   {
             respdata[0] = NO_ANS;
             rspswitch = OFF;
             break;
          }
          if (*workbuf == respchar[ABORT_RS])  {
             respdata[0] = ABORT_ANS;
             rspswitch = OFF;
             break;
          }
          if (*workbuf == respchar[RETRY_RS])  {
             respdata[0] = RETRY_ANS;
             rspswitch = OFF;
             break;
          }
          if (*workbuf == respchar[IGNORE_RS])  {
             respdata[0] = IGNORE_ANS;
             rspswitch = OFF;
             break;
          }

         rc = DosPutMessage( (unsigned short)devicetype, msglength, (char FAR *)msg_buft);
         if ( rc != OK) {
            DBG_ERROR(("DosPutMessage failed in unexpected response, rc = %d\n", rc));
            return(rc);
         }

         break;

       case STRING_RESPONSE:
         if ((rc=readinput(resptype)) != OK)
            return(rc);
         workbuf[bytes_read - 2] = '\0';
         strncpy(respdata, workbuf, respdata_size);
         respdata[respdata_size - 1] = '\0';
         rspswitch = OFF;
         break;

      case ANY_KEY_RESPONSE:
         rc = KbdFlushBuffer(KBDHANDLE);
         if ( rc != OK) {
            DBG_ERROR(("KbdFlushBuffer error, rc = %d\n", rc));
            return(rc);
         }

         rc = KbdCharIn ( &KeyBuf, 0, 0 );

         DosWrite( (devicetype != 0) ? 2:1, "\r\n", 2, &byteswritten);

         rspswitch = OFF;
         break;

       default:
        return (ERROR_MR_MSG_TOO_LONG);

    }  // end switch

  } // end while

  return(OK);
}

/**********************************************************************/
/*     GETMESSAGE
/**********************************************************************/

unsigned getmessage(ivtable, ivcount, msgid, filename)
  char *ivtable[];                            /* table of variables to insert */
  unsigned ivcount;                          /* number of variables          */
  unsigned msgid;                            /* id of the message            */
  char *filename;                            /* message filename strig       */
  {
     unsigned i;
     int rc,rc_s;
     unsigned char        *ptr16;
     unsigned char        *ptr32;
     char                 *ivtable_16[9];

     char **x;

     char  *table[2];
     UCHAR  field0[10];
     UCHAR  field1[10];

     strcpy(field0, "ZERO" );
     strcpy(field1, "ONE" );
     table[0] = field0;
     table[1] = field1;

     x = ivtable;

//   while (i=0, i < ivcount && i < MAX_IVCOUNT, i++) {
//      ptr32 = ivtable[i];
//      ptr16 = ptr32;
//      ivtable_16[i] = ptr16;
//   }

     rc = DosGetMessage( x, ivcount, msg_buft, BUFT_SIZE, msgid, filename, &msglength);
     rc_s = rc;

     switch(rc_s) {
        case OK:
          if (msglength == msg_buf_size)
             msg_buft[msglength-1] = 0;
          else
             msg_buft[msglength] = 0;
          return(rc_s);
        case ERROR_MR_MSG_TOO_LONG:
          return(rc);
        default:
          rc = DosPutMessage( STDERR_HDL, msglength, msg_buft);
          if ( rc ) {
             return(rc);
          }
          return(rc_s);
       }
  }

/**********************************************************************/
/*     READINPUT
/**********************************************************************/
unsigned readinput(resptype)
int resptype;
{
  int rc;                                  /* return code                  */
  char  char_avail_flag;                   /* relate to KBDCHARIN          */
  char  kbdin_flag = OFF;                  /* relate to KBDCHARIN          */
  int   readloop;
/*************************************/
/*                                   */
/*  for DOSQHANDLETYPE               */
/*                                   */
/*************************************/
  unsigned long handletype;  /* stdin handletype for DOSQHANDTYPE  */
  unsigned long out_handletype;   /* stdout handletype for DOSQHANDTYPE  */
  unsigned long flagword;         /* flagword   for DOSQHANDTYPE  */
  #define FILEHANDLE     0   /* file systme handle type      */
  #define DEVICEHANDLE   1   /* device handle type           */
  #define PIPEHANDLE     2   /* pipe handle type      DCR74  */

/*************************************/
/*                                   */
/*  for DOSREAD                      */
/*                                   */
/*************************************/
  char inputchar;                   /* char input from DOSREAD      */
  unsigned long num_read_byte;      /* num. of actually read bytes  */
  #define ONE_BYTE  1               /* number of bytes to read at   */
                                    /*   one time                   */
  char carriage_detect = OFF;       /* when CR is detected          */
                                    /*   set to ON                  */
  #define CR_CHAR  0x0d             /* Carriage Return Code         */
  #define NL_CHAR  0x0a             /* New Line (line feed) Code    */

/*************************************/
/*                                   */
/* for DBCS char checking            */
/*                                   */
/*************************************/

  #define DBCS_LEAD_BYTE   1
  char dbcs_being_processed = OFF;    /* DBCS process flag (init OFF) */

/*************************************/
/*                                   */
/* If in Protect mode, get handletype*/
/* of current Standard Input         */
/* (handle 0).                       */
/*                                   */
/* In real mode, DOSQHANDLETYPE is   */
/* is not supported. Set handletype  */
/* to DEVICEHANDLE (= 1)             */
/*                                   */
/*************************************/

    rc = DosQHandType( STDIN_HDL,&handletype,&flagword);
    if ( rc != OK )
       return(rc);


    rc = DosQHandType( STDOUT_HDL, &out_handletype, &flagword);
    if ( rc != OK )
       return(rc);

    bytes_read = 0;

/*************************************/
/*                                   */
/* In protect mode,  if input being  */
/* redirected to a file, kbd buffer  */
/* is not flushed. In real mode, kbd */
/* buffer is always flushed even     */
/* if input being redirected.        */
/*                                   */
/*************************************/

    handletype = handletype & 0x00FF;
    out_handletype = out_handletype & 0x00FF;
    if (handletype == DEVICEHANDLE)    {
       rc = KbdFlushBuffer(KBDHANDLE);
       if ( rc != OK)
            return(rc);
    }

/*************************************/
/*                                   */
/* read one byte from std input dev  */
/*                                   */
/*************************************/

  readloop = 1;

  while ( readloop ) {
    if ((handletype == DEVICEHANDLE) && (out_handletype == FILEHANDLE)) {
       kbdin_flag = ON;
       rc = KbdCharIn ( &KeyBuf, 0, 0 );
       if ( rc != OK)
         return(rc);

       char_avail_flag = (KeyBuf.fbStatus & 0x40);
       if (char_avail_flag == FALSE)
          return(ERROR_CM_EOF_REDIRECT);

       bytes_read ++;
       workbuf[bytes_read-1] = KeyBuf.chChar; /* put input char in buffer */
    }
    else {
       rc = DosRead( STDIN_HDL, &inputchar, ONE_BYTE, &num_read_byte);

       if ( rc != OK)
         return(rc);

       if (num_read_byte == 0)
          return(ERROR_CM_EOF_REDIRECT);

       bytes_read += (short)num_read_byte;
       workbuf[bytes_read-1] = inputchar;

    }  // end if-else

    if( dbcs_being_processed == OFF) {
       if( check_dbcs(inputchar)== DBCS_LEAD_BYTE) {
           dbcs_being_processed = ON;           /*    get tailing character.    */
       }
    }

    /*****************************************************************/
    /* Check if input is Carriage return(0D) or New line (0A)        */
    /* or EOF(1A).  if 0D, 0A are received, terminate reading data.  */
    /* if EOF (1A) is received and requested response type is Yes/No,*/
    /* return No response as default.                                */
    /*****************************************************************/
    if (kbdin_flag) {
      switch(KeyBuf.chChar) {

        case CR_CHAR:
          carriage_detect = ON;
          if ((handletype==FILEHANDLE) || (out_handletype==FILEHANDLE)) {
              workbuf[bytes_read++] = NL_CHAR;
              workbuf[bytes_read]   = 0;
              DosWrite(STDOUT_HDL, workbuf, bytes_read, &byteswritten);         /*;BC016;*/
          }
          return(OK);

        default:
          carriage_detect = OFF;
       } // end switch
    }
    else   {
      switch(inputchar) {

        case CR_CHAR:
          carriage_detect = ON;
          if (!kbdin_flag)
             break;

        case NL_CHAR:
         /*******************************************************************/
         /* CR(0x0d),NL(0x0a) character sets are found, terminate reading   */
         /* data and return normally. If input is redirected to file,       */
         /* write input data to standard output.  (only for protect mode).  */
         /*******************************************************************/
         if (carriage_detect == ON) {
           if ((handletype==FILEHANDLE) || (out_handletype==FILEHANDLE)) {
               workbuf[bytes_read] = 0;
               DosWrite(STDOUT_HDL, workbuf, bytes_read, &byteswritten);
           }
           readloop = 0;
           return(OK);
         }

        default:
          carriage_detect = OFF;
      } // end switch
    } // end if-else

  } // end while loop
  return( rc );
}

/**********************************************************************/
/*     CHECK_DBCS
/**********************************************************************/

int check_dbcs(c)
unsigned char c;
{
  unsigned short temp;

  temp = c;
  temp = (unsigned short)(temp & 0x00FF);

  if ( temp < 0x0080 )
      return( 0 );

  return( DbcsFlags[ temp & 0x7F ] );
}

/**********************************************************************/
/*     COM_SETTBL
/**********************************************************************/

int com_settbl()
{
   unsigned int i, j;
   unsigned char *strptr;
   int rc;

   static int setup_flag = OFF;

   static COUNTRYCODE current_country;

  if (setup_flag == ON)
     return(OK);

  rc = DosQueryDBCSEnv( (ULONG)DBCSVEC_SIZE*2,&current_country,(PCHAR)&dbcsvec );
  if( rc != OK ) {
     setup_flag = ON;
     dbcsvec[0].lo_limit = 0;
     dbcsvec[0].up_limit = 0;
     return(rc);
  }

  for (i = 0; i < DBCSVEC_SIZE; i++) {
     if (!dbcsvec[i].lo_limit)
         break;

     dbcsvec[i].up_limit &= 0x7f;
     for (j = (dbcsvec[i].lo_limit & 0x7f); j <= dbcsvec[i].up_limit; j++)
         DbcsFlags[j] = 1;
  }

  for (i = 0; i < CASEMAP_SIZE; i++) {
    casemap_lo[i] =
    casemap_up[i] = (unsigned char)i;
  }

  rc = DosMapCase( CASEMAP_SIZE, &current_country, casemap_up );
  if ( rc != OK ) {
     strupr(casemap_up);
     return(rc);
  }

  strptr = casemap_up;

  for(i=0; i<0x80; i++, strptr++) {
    if (i!=*strptr)
      casemap_lo[*strptr]=(unsigned char)i;
  }
  setup_flag = ON;

  return(OK);
}

