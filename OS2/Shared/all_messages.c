/*!
   $Id: all_messages.c,v 1.1.1.1 2003/10/04 08:36:16 prokushev Exp $

   @file all_messages.c

   @brief utility functions related to retrieving messages,
   shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>

   @author Henk Kelder
   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

/* C standard library headers */
#include <string.h>
#include <stdio.h>

#define  INCL_DOSMISC
#include <all_shared.h> /* comand line tools' shared functions and defines */

#define BUFSIZE 300 /*!< buffer size for arguments table - see prepareArgTable */

/*!
   Gets system error text (description)

   @param ulRc    Api Return code, for which description is requested

   @return Pointer to PSZ containing error message or information that it
           couldn't be retrieved

   @todo  NLS support
*/
PSZ all_GetSystemErrorMessage(ULONG ulRc)
{
  APIRET  rc;
  CHAR    szErrorMessage[1000];
#ifdef __386__
  ULONG   cbErrorMessage = 0;
#else
  USHORT  cbErrorMessage = 0;
#endif

  rc=DosGetMessage(NULL, 0,szErrorMessage, sizeof(szErrorMessage),
                  ulRc,
                  "OSO001.MSG",        // default OS/2 message file
                  &cbErrorMessage);

  switch (rc)
  {
    case NO_ERROR: /* message retrieved, ok... */
      break;
    case ERROR_FILE_NOT_FOUND :
            sprintf(szErrorMessage, "SYS%04u (Message file not found!)", ulRc);
      break;
    default:
            sprintf(szErrorMessage, "SYS%04u (Error %d while retrieving message text!)", ulRc, rc);
  };

  return (szErrorMessage);
};

/*!
   Gets system error explaination )

   @param ulRc    Api Return code, for which explaination is requested

   @return Pointer to PSZ containing explaination or information that it
           couldn't be retrieved
*/
PSZ FAR all_GetSystemErrorHelp(ULONG ulRc)
{
  APIRET  rc;
  CHAR    szErrorMessage[4000];
#ifdef __386__
  ULONG   cbErrorMessage = 0;
#else
  USHORT  cbErrorMessage = 0;
#endif

  rc=DosGetMessage(NULL, 0,szErrorMessage, sizeof(szErrorMessage),
                  ulRc,
                  "OSO001H.MSG",        // default OS/2 message file
                  &cbErrorMessage);

  if (rc!=0) return all_GetSystemErrorMessage(rc);

  return (szErrorMessage);
};

/*!
  Prepares argument tables for message (*.msg) processing

  @param ulParams     number of parameters to be processed, max allowed
                      value is 9
  @param pszArgTable  table of PSZs where prepared parameters will be saved
                      note that function internally handles memory allocation
                      for these PSZs
  @param ...          variable arguments, for every message parameter do:
                        - char *fmt - format specifier for parameter - the same
                                      as for printf
                        - values to put in parameter
                       there is max 9 arguments for each message parameter
                       allowed
*/
void __cdecl all_prepareArgTable(ULONG ulParams,PSZ pszArgTable[],...)
{
  va_list args;
  va_start(args,pszArgTable);
  all_vprepareArgTable(ulParams,pszArgTable,args);
  va_end(args);
};

/*!
  Prepares argument tables for message (*.msg) processing - va_list version

  @param ulParams     number of parameters to be processed, max allowed
                      value is 9
  @param pszArgTable  table of PSZs where prepared parameters will be saved
                      note that function internally handles memory allocation
                      for these PSZs
  @param args         opened va_list (for variable number of parameters)
*/
void __cdecl all_vprepareArgTable(ULONG ulParams,PSZ pszArgTable[],va_list vlArgs)
{
  char *fmt;        /* format string */
  int paramCount;   /* parameter count for specified format string */
  void *pvArgTable[9]; /* pointers for arguments table */
  CHAR buffer[BUFSIZE];
  ULONG i,j;


  /* for every argument.. */
  for (i=0;i<ulParams;i++)
  {
   fmt=va_arg(vlArgs,char*); /* get format specifier */

   paramCount=0; /* for a while there is no arguments for this format */

   /* search format string, count argument number and record argument pointers
      in pvArgTable */
   for (j=0;j<strlen(fmt);j++)
   {
    if (fmt[j]=='%') /* found argument specifier */
    {
      if (fmt[j+1]!='%') /* if it's not just to print '%' */
        pvArgTable[paramCount++]=va_arg(vlArgs,void *); /* we record position, and increase
                                         parameter count */
      else j++; /* or we just skip to next field */
    };
   }; /* END: for (j=0;j<strlen(fmt);j++) */

   memset(buffer,0,BUFSIZE);

   switch (paramCount)
   {
     case 1: sprintf(buffer,fmt,pvArgTable[0]);
     break;
     case 2: sprintf(buffer,fmt,pvArgTable[0],pvArgTable[1]);
     break;
     case 3: sprintf(buffer,fmt,pvArgTable[0],pvArgTable[1],
                        pvArgTable[2]);
     break;
     case 4: sprintf(buffer,fmt,pvArgTable[0],pvArgTable[1],
                        pvArgTable[2],pvArgTable[3]);
     break;
     case 5: sprintf(buffer,fmt,pvArgTable[0],pvArgTable[1],
                        pvArgTable[2],pvArgTable[3],
                        pvArgTable[4]);
     break;
     case 6: sprintf(buffer,fmt,pvArgTable[0],pvArgTable[0],
                        pvArgTable[2],pvArgTable[3],
                        pvArgTable[4],pvArgTable[5]);
     break;
     case 7: sprintf(buffer,fmt,pvArgTable[0],pvArgTable[1],
                        pvArgTable[2],pvArgTable[3],
                        pvArgTable[4],pvArgTable[5],
                        pvArgTable[6]);
     break;
     case 8: sprintf(buffer,fmt,pvArgTable[0],pvArgTable[1],
                        pvArgTable[2],pvArgTable[3],
                        pvArgTable[4],pvArgTable[5],
                        pvArgTable[6],pvArgTable[7]);
     break;
     case 9: sprintf(buffer,fmt,pvArgTable[0],pvArgTable[1],
                        pvArgTable[2],pvArgTable[3],
                        pvArgTable[4],pvArgTable[5],
                        pvArgTable[6],pvArgTable[7],
                        pvArgTable[8]);
     break;
   };

  pszArgTable[i]=(PSZ)calloc(strlen(buffer)+1,1);
#ifdef __386__
  strcpy(pszArgTable[i],buffer);
#else
  _fstrcpy(pszArgTable[i],buffer);
#endif

  }; //END: for (i=0;i<n;i++)

};
