/*===================================================================
 = conversions.c
 =
 = Contains functions to copy/read values and structures to/from
 = the Rexx variable pool.
 =
 = $Log: conversions.c,v $
 = Revision 1.2  2003/12/27 04:38:52  mark
 = Undo the "fix" for the RexxVariablePool() bug and value length. Regina is correct.
 =
 = Revision 1.1.1.1  2002/11/29 23:46:24  mark
 = Importing RxSock 
 =
 = Revision 1.3  1998/10/13 02:12:36  abbott
 = Added c2r_fd_setarray() function needed by SockSelect().
 =
 = Revision 1.2  1998/09/20 23:27:06  abbott
 = Changed to better support WinRexx.  Also added REXXTRANS configuration.
 =
 =
 ===================================================================*/
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#endif

#include "conversions.h"

/*======================================================
 = r2c_SymbIntValueFunc()
 ======================================================*/
int r2c_SymbIntValueFunc(int *intVal, int cvtVal,
                    PRXSTRING rxTstVal, const char *cTstVal) {
   int cTstLen = strlen(cTstVal);
   int isSame = (cTstLen == rxTstVal->strlength)
      && !strncmp(rxTstVal->strptr, cTstVal, cTstLen);

   if (isSame) {
      *intVal = cvtVal;
      return 1;
   }
   return 0;
}

/*======================================================
 = r2c_SymbUshortValueFunc()
 ======================================================*/
int r2c_SymbUshortValueFunc(u_short *intVal, u_short cvtVal,
                    PRXSTRING rxTstVal, const char *cTstVal) {
   int cTstLen = strlen(cTstVal);
   int isSame = (cTstLen == rxTstVal->strlength)
      && !strncmp(rxTstVal->strptr, cTstVal, cTstLen);

   if (isSame) {
      *intVal = cvtVal;
      return 1;
   }
   return 0;
}

/*======================================================
 = r2c_int()
 ======================================================*/
int r2c_int(int *newValue, PRXSTRING rx) {
   char copy[10];  /* Max. length of int we can handle. */

   if (rx->strlength > sizeof(copy) - 1)
      return 0;

   /* Make sure the RXSTRING is null terminated. */
   memcpy(copy, rx->strptr, rx->strlength);
   copy[ rx->strlength ] = 0;

   return sscanf(copy, "%d", newValue);
}

/*======================================================
 = r2c_uint()
 ======================================================*/
int r2c_uint(u_int *newValue, PRXSTRING rx) {
   char copy[10];  /* Max. length of int we can handle. */

   if (rx->strlength > sizeof(copy) - 1)
      return 0;

   /* Make sure the RXSTRING is null terminated. */
   memcpy(copy, rx->strptr, rx->strlength);
   copy[ rx->strlength ] = 0;

   return sscanf(copy, "%u", newValue);
}

/*======================================================
 = r2c_ushort()
 ======================================================*/
int r2c_ushort(u_short *newValue, PRXSTRING rx) {
   int rdVal;
   int rc;
   char copy[10];

   if (rx->strlength > sizeof(copy) - 1)
      return 0;

   /* Make sure the RXSTRING is null terminated. */
   memcpy(copy, rx->strptr, rx->strlength);
   copy[rx->strlength] = 0;

   rc = sscanf(copy, "%d", &rdVal);
   if (rc == 1)
      *newValue = rdVal;
   return rc;
}

/*======================================================
 = r2c_ushort_htons()
 ======================================================*/
int r2c_ushort_htons(u_short *newValue, PRXSTRING rx) {
   int rc;

   rc = r2c_ushort(newValue, rx);
   if (rc == 1)
      *newValue = htons(*newValue);
   return rc;
}

/*=====================================================
 = initStemList()
 = Initializes an array of shvblocks for a stem.
 ====================================================*/
static void initStemList(SHVBLOCK list[], int len, int shvcode,
                  RXSTRING *stem, char *names[],
                  char namespace[][256],
                  char valuespace[][256],
                  int  valuelens[]) {
   int i;

   for (i = 0; i < len; i++) {
      list[i].shvnext = &list[i + 1];
      list[i].shvcode = shvcode;
      list[i].shvname.strptr = namespace[i];
      strncpy(namespace[i], stem->strptr, stem->strlength);
      strcpy(namespace[i] + stem->strlength, names[i]);
      list[i].shvnamelen = list[i].shvname.strlength
         = strlen(list[i].shvname.strptr);
      list[i].shvvalue.strptr = valuespace[i];
      list[i].shvvalue.strlength = valuelens[i];
      list[i].shvvaluelen = valuelens[i];
   }

   list[len-1].shvnext = NULL;
}

/*======================================================
 = r2c_dotAddress()
 ======================================================*/
int r2c_dotAddress(unsigned long *newValue, PRXSTRING rx) {
   char copy[20];

   if (rx->strlength > sizeof(copy) - 1)
      return 0;

   /* Make sure rx is null terminated */
   memcpy(copy, rx->strptr, rx->strlength);
   copy[rx->strlength] = 0;

   *newValue = inet_addr(copy);
   return (*newValue != INADDR_NONE);
}

/*======================================================
 = r2c_sockaddr_in()
 ======================================================*/
int r2c_sockaddr_in(SOCKADDR_IN *newValue, PRXSTRING rx) {
   SHVBLOCK query[3];
   char *names[3] = { "FAMILY", "PORT", "ADDR" };
   char namespace[3][256];
   char valuespace[3][256];
   int  valuelens[3] = { 256, 256, 256 };
   int  rc;

   /* Initialize the list of shvblocks. */
   initStemList(query, 3, RXSHV_SYFET, rx, names, namespace,
                valuespace, valuelens);

   /* Look up the Rexx variables. */
   rc = RexxVariablePool(query);

   /*
    * We have to "fix" the value of all shvvalue.strlength(s)
    * because the REXX API documentation says that shvvalue.strlength
    * is set to the length of shvvalue.strptr, but OS/2 REXX is
    * broken and most other implementors followed the broken OS/2
    * REXX!
    * All implementations set the length of shvvalue.strptr in
    * shvvaluelen, except when shvret == RXSHV_TRUNC
   for (rc = 0; rc < 3; rc++) {
      if (query[rc].shvret != RXSHV_TRUNC)
         query[rc].shvvalue.strlength = query[rc].shvvaluelen;
   }
    */

   /* Zero out the sin_zero of newValue. */
   memset(newValue->sin_zero, 0, sizeof(newValue->sin_zero));

   return
      (
       r2c_SymbUshortValue(&newValue->sin_family, &query[0].shvvalue,
                           AF_INET) &&
       r2c_ushort_htons(&newValue->sin_port, &query[1].shvvalue) &&
       (r2c_SymbIntValue((int *)&newValue->sin_addr, &query[2].shvvalue,
                         INADDR_ANY) ||
        r2c_dotAddress((unsigned long *)&newValue->sin_addr,
                       &query[2].shvvalue)));
}

/*======================================================
 = c2r_sockaddr_in()
 ======================================================*/
int c2r_sockaddr_in(SOCKADDR_IN const *value, PRXSTRING rx) {
   SHVBLOCK query[3];
   char *names[3] = { "FAMILY", "PORT", "ADDR" };
   char namespace[3][256];
   char valuespace[3][256];
   int  valuelens[3] = { 0, 0, 0 };
   int  rc;

   /* Set the values for the stems. */
   /* Only family known is AF_INET */
   if (value->sin_family != AF_INET) {
      return 0;
   }

   valuelens[0] = sprintf(valuespace[0], "%d", value->sin_family);
#if 0
   /* OS/2 seems to return a numeric value for sin_family. */
   strcpy(valuespace[0], "AF_INET");
   valuelens[0] = 7;
#endif

   valuelens[1] = sprintf(valuespace[1], "%d", ntohs(value->sin_port));
   valuelens[2] = sprintf(valuespace[2], "%s", inet_ntoa(value->sin_addr));
      
   /* Initialize the list of shvblocks. */
   initStemList(query, 3, RXSHV_SYSET, rx, names, namespace,
                valuespace, valuelens);

   /* Set the Rexx variables. */
   rc = RexxVariablePool(query);

   return ((rc == RXSHV_OK) || (rc == RXSHV_NEWV));
}

/*======================================================
 = c2r_hostent()
 ======================================================*/
int c2r_hostent(struct hostent const *value, PRXSTRING rx) {
   SHVBLOCK query[3];
   char *names[3] = { "NAME", "ADDRTYPE", "ADDR" };
   char namespace[3][256];
   char valuespace[3][256];
   int  valuelens[3] = { 0, 0, 0 };
   int  rc;
   int  count = 0;
   char **aliases;
   int  curLen;
   char  **addrs;

   /* Only known addrtype is PF_INET/AF_INET */
   if (value->h_addrtype != PF_INET) {
      return 0;
   }

   /* Set the values for the stems. */
   valuelens[0] = sprintf(valuespace[0], "%s", value->h_name);
   /* Next value is AF_INET because that's what OS/2 RXSOCK produces. */
   valuelens[1] = sprintf(valuespace[1], "%s", "AF_INET");
   valuelens[2] = sprintf(valuespace[2], "%s",
                          inet_ntoa(*(struct in_addr *)value->h_addr_list[0]));
      
   /* Initialize the list of shvblocks. */
   initStemList(query, 3, RXSHV_SYSET, rx, names, namespace,
                valuespace, valuelens);

   /* Set the Rexx variables. */
   rc = RexxVariablePool(query);

   /* Now convert the non-static portion. */
   /* Use query[0].  It has been initialized somewhat */
   /* by the call to initStemList */
   query[0].shvnext = 0;
   namespace[0][rx->strlength] = 0;
   strcat(namespace[0], "ALIAS.");
   curLen = rx->strlength + 6;      /* Current length of namespace[0] */

   /* Convert the list of alises. */
   aliases = value->h_aliases;
   while (*aliases != 0) {
      char numAl[10];  /* Assume fewer than 1000000000 aliases. */
      count++;

      sprintf(numAl, "%d", count);
      namespace[0][curLen] = 0;   /* Remove last index. */
      strcat(namespace[0], numAl);
      query[0].shvname.strlength = strlen(namespace[0]);

      query[0].shvvalue.strlength
         = sprintf(valuespace[0], "%s", *aliases);

      rc = RexxVariablePool(query);
      aliases++;
   }

   /* Now need to set alias.0 */
   namespace[0][curLen] = 0;   /* Remove last index. */
   strcat(namespace[0], "0");
   query[0].shvname.strlength = strlen(namespace[0]);
   query[0].shvvalue.strlength
      = sprintf(valuespace[0], "%d", count);
   rc = RexxVariablePool(query);
   
   /* Convert the list of addresses. */
   namespace[0][rx->strlength] = 0;
   strcat(namespace[0], "ADDR.");
   curLen = rx->strlength + 5;      /* Current length of namespace[0] */
   count = 0;
   addrs = value->h_addr_list;

   while (*addrs != 0) {
      char numAl[10];  /* Assume fewer than 1000000000 addrs. */
      count++;

      sprintf(numAl, "%d", count);
      namespace[0][curLen] = 0;   /* Remove last index. */
      strcat(namespace[0], numAl);
      query[0].shvname.strlength = strlen(namespace[0]);

      query[0].shvvalue.strlength
         = sprintf(valuespace[0], "%s", inet_ntoa(*(struct in_addr *)*addrs));

      rc = RexxVariablePool(query);
      addrs++;
   }
   
   /* Now need to set addr.0 */
   namespace[0][curLen] = 0;   /* Remove last index. */
   strcat(namespace[0], "0");
   query[0].shvname.strlength = strlen(namespace[0]);
   query[0].shvvalue.strlength
      = sprintf(valuespace[0], "%d", count);
   rc = RexxVariablePool(query);

   return ((rc == RXSHV_OK) || (rc == RXSHV_NEWV));
}

/*======================================================
 = r2c_fd_setarray()
 ======================================================*/
int r2c_fd_setarray(fd_set *ary, int *fdin, PRXSTRING rx, int *aryLen) {
   int rAryLen;
   char aryVal[256];   /* Should be plenty. */
   char result[256];
   SHVBLOCK shv;
   int rc;
   int i,curr_fd;
   int max_fd=0;

   FD_ZERO(ary);

   strncpy(aryVal, rx->strptr, rx->strlength);
   aryVal[rx->strlength] = '0';
   aryVal[rx->strlength+1] = 0;
   make_upper(aryVal);

   shv.shvnext = 0;
   shv.shvcode = RXSHV_SYFET;
   shv.shvname.strptr = aryVal;
   shv.shvname.strlength = rx->strlength + 1;
   shv.shvvalue.strptr = result;
   shv.shvvaluelen = sizeof(result);

   rc = RexxVariablePool(&shv);

   if (shv.shvret == RXSHV_TRUNC) {
      shv.shvvalue.strptr[shv.shvvalue.strlength] = 0;
   }
   else
      shv.shvvalue.strptr[shv.shvvaluelen] = 0;
   rAryLen = atoi(result);

   if (rAryLen > *aryLen) {
      /* Rexx array too large.  Punt. */
      return 0;
   }

   for (i = 0; i < rAryLen; i++) {
      shv.shvname.strlength
         = shv.shvnamelen
         = sprintf(shv.shvname.strptr + rx->strlength, "%d", i + 1)
           + rx->strlength;
      shv.shvvaluelen = sizeof(result);
      rc = RexxVariablePool(&shv);
      result[shv.shvvalue.strlength] = 0;
      curr_fd = atoi(result);
      FD_SET(curr_fd, ary);
      *fdin = curr_fd;
      if (curr_fd > max_fd)
         max_fd = curr_fd;
      fdin++;
   }

   *aryLen = rAryLen;
   return max_fd;
}

/*======================================================
 = c2r_fd_setarray()
 ======================================================*/
int c2r_fd_setarray(fd_set *ary, int *fdin, PRXSTRING rx,int numin) {
   int rAryLen;
   char rxName[256];   /* Should be plenty. */
   char rxVal[256];
   SHVBLOCK shv;
   int rc;
   int i;

   strncpy(rxName, rx->strptr, rx->strlength);
   rxName[rx->strlength] = '0';
   rxName[rx->strlength + 1] = 0;
   make_upper(rxName);

   shv.shvnext = 0;
   shv.shvcode = RXSHV_SYSET;
   shv.shvname.strptr = rxName;

   shv.shvvalue.strptr = rxVal;

   /* Loop through the fd_set to find all of the FDs that are set. */
   rAryLen = 0;
#ifdef __WIN32__
   for (i = 0; i < ary->fd_count; i++) {
      rAryLen++;
      shv.shvname.strlength
         = sprintf(rxName + rx->strlength, "%d", rAryLen)
         + rx->strlength;

      shv.shvvaluelen
         = shv.shvvalue.strlength
         = sprintf(rxVal, "%d", ary->fd_array[i]);

      rc = RexxVariablePool(&shv);
   }
#else
   for (i = 0; i < numin; i++) {
      if ( FD_ISSET( fdin[i], ary ) ) {
         rAryLen++;
         shv.shvname.strlength
            = sprintf(rxName + rx->strlength, "%d", rAryLen)
            + rx->strlength;
         shv.shvvaluelen
            = shv.shvvalue.strlength
            = sprintf(rxVal, "%d", fdin[i]);

         rc = RexxVariablePool(&shv);
      }
   }
#endif

   /* Set the 0th element of the Rexx array to the size. */
   shv.shvname.strlength
      = sprintf(rxName + rx->strlength, "%d", 0)
      + rx->strlength;
   
   shv.shvvaluelen
      = shv.shvvalue.strlength
      = sprintf(rxVal, "%d", rAryLen);

   rc = RexxVariablePool(&shv);

   return 1;
}

/*======================================================
 = setRexxVar()
 ======================================================*/
int setRexxVar(PRXSTRING rx, char *buf, int len) {
   SHVBLOCK shv;
   int rc;
   int rcode = 0;
   char varName[MAXVARNAMELEN + 1];

   if (rx->strlength != 0) {
      shv.shvnext = 0;
      shv.shvcode = RXSHV_SYSET;
      shv.shvname = *rx;
      shv.shvvalue.strptr = buf;
      shv.shvvalue.strlength = len;
      
      /* make sure the variable name is uppercase. */
      memcpy(varName, rx->strptr, rx->strlength);
      varName[rx->strlength] = 0;

      make_upper(varName);

      shv.shvname.strptr = varName;

      rc = RexxVariablePool(&shv);

      rcode = ((rc == RXSHV_OK) || (rc == RXSHV_NEWV)) ? 1 : 0;
   }
   return rcode;
}

/*======================================================
 = r2c_sockopt_option()
 ======================================================*/
int r2c_sockopt_option(int *value, PRXSTRING rx) {
   return r2c_SymbIntValue(value, rx, SO_BROADCAST)
      || r2c_SymbIntValue(value, rx, SO_DEBUG)
      || r2c_SymbIntValue(value, rx, SO_DONTROUTE)
      || r2c_SymbIntValue(value, rx, SO_ERROR)
      || r2c_SymbIntValue(value, rx, SO_KEEPALIVE)
      || r2c_SymbIntValue(value, rx, SO_LINGER)
      || r2c_SymbIntValue(value, rx, SO_OOBINLINE)
      || r2c_SymbIntValue(value, rx, SO_RCVBUF)
#if 0
      || r2c_SymbIntValue(value, rx, SO_RCVLOWAT)
      || r2c_SymbIntValue(value, rx, SO_RCVTIMEO)
#endif
      || r2c_SymbIntValue(value, rx, SO_REUSEADDR)
      || r2c_SymbIntValue(value, rx, SO_SNDBUF)
#if 0
      || r2c_SymbIntValue(value, rx, SO_SNDLOWAT)
      || r2c_SymbIntValue(value, rx, SO_SNDTIMEO)
#endif
      || r2c_SymbIntValue(value, rx, SO_TYPE)
#if 0
      || r2c_SymbIntValue(value, rx, SO_USELOOPBACK)
#endif
      || 0;
}

/*======================================================
 = r2c_recv_flags()
 ======================================================*/
int r2c_recv_flags(int *value, PRXSTRING rx) {
   char *flagsstr;
   int flags = 0;
   char copy[200];

   if (rx->strlength > sizeof(copy) - 1)
      return 0;

   /* Make sure rx->strptr is null terminated. */
   memcpy(copy, rx->strptr, rx->strlength);
   copy[rx->strlength] = 0;

   /* Valid flags are MSG_OOB and MSG_PEEK */
   flagsstr = strtok(copy, " ");
   while (flagsstr != 0) {
      if (!strcmp(flagsstr, "MSG_OOB")) {
         flags |= MSG_OOB;
      }
      else if (!strcmp(flagsstr, "MSG_PEEK")) {
         flags |= MSG_PEEK;
      }
      
      flagsstr = strtok(0, " ");
   }

   *value = flags;
   return 1;
}

/*======================================================
 = r2c_2_ints()
 ======================================================*/
int r2c_2_ints(int *newValue1, int *newValue2, PRXSTRING rx) {
   int numCvt;
   char copy[100];

   /* Make sure NULL terminated. */
   if (rx->strlength > sizeof(copy) - 1)
      return 0;

   memcpy(copy, rx->strptr, rx->strlength);
   copy[rx->strlength] = 0;
   numCvt = sscanf(copy, "%d %d", newValue1, newValue2);

   return (numCvt == 2 ? 1 : 0);
}
