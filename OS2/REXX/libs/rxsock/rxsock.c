/*===================================================================
 = rxsock.c
 =
 = Contains the Rexx SAA API RXSOCK interface routines.  See the array
 = rxfuncs[] below for a complete list of functions available to a
 = Rexx program.
 =
 = $Log: rxsock.c,v $
 = Revision 1.5  2003/12/27 10:36:34  mark
 = Update version details to 1.4
 =
 = Revision 1.4  2003/12/27 06:51:44  mark
 = Remove debugging code.
 =
 = Revision 1.3  2003/12/27 04:39:20  mark
 = Changes for new common source format.
 =
 = Revision 1.2  2003/01/22 03:33:57  mark
 = Changes for "standard" package structure
 =
 = Revision 1.1.1.1  2002/11/29 23:46:24  mark
 = Importing RxSock 
 =
 = Revision 1.4  1998/11/11 02:24:34  abbott
 = Changed RxsockVersion number.  Now 1.2.
 =
 = Revision 1.3  1998/10/13 02:14:37  abbott
 = SockSelect() was not properly updating the lists of sockets.
 =
 = Revision 1.2  1998/09/20 23:24:57  abbott
 = Changed to better support WinRexx.  Also added REXXTRANS configuration.
 =
 =
 ===================================================================*/

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#endif

#if defined(__EMX__) && defined(USE_OS2REXX)
# define INCL_DOS
# define INCL_WIN
# define INCL_GPI
# include "rxpack.h"
#else
# include "rxpack.h"
#endif

#if defined(__WATCOMC__)
# include <alloca.h>
#endif

#include "conversions.h"

#define RXSOCK_VERSION "1.4.0"
#define RXSOCK_DATE "30 December 2003"
#define LIBNAME "RXSOCK"

static char *RxPackageName = "rxsock";

#ifndef MAXHOSTNAMELEN
#  define MAXHOSTNAMELEN 256
#endif

RexxFunctionHandler SockAccept;
RexxFunctionHandler SockBind;
RexxFunctionHandler SockClose;
RexxFunctionHandler SockConnect;
RexxFunctionHandler SockDropFuncs;
RexxFunctionHandler SockLoadFuncs;
RexxFunctionHandler SockGetHostByAddr;
RexxFunctionHandler SockGetHostByName;
RexxFunctionHandler SockGetHostId;
RexxFunctionHandler SockGetPeerName;
RexxFunctionHandler SockGetSockName;
RexxFunctionHandler SockGetSockOpt;
RexxFunctionHandler SockInit;
RexxFunctionHandler SockIoctl;
RexxFunctionHandler SockListen;
RexxFunctionHandler SockPSock_Errno;
RexxFunctionHandler SockRecv;
RexxFunctionHandler SockRecvFrom;
RexxFunctionHandler SockSelect;
RexxFunctionHandler SockSend;
RexxFunctionHandler SockSendTo;
RexxFunctionHandler SockSetSockOpt;
RexxFunctionHandler SockShutDown;
RexxFunctionHandler SockSock_Errno;
RexxFunctionHandler SockSocket;
RexxFunctionHandler SockSoClose;
RexxFunctionHandler SockVersion;
RexxFunctionHandler SockVariable;
RexxFunctionHandler RxsockVersion;
RexxFunctionHandler SockVariable;

/********************************************************************
 * rxfuncs[] is an array of names of functions that are made
 * available to a Rexx program.
 ********************************************************************/
RexxFunction RxSockFunctions[] = {
 { "SOCKACCEPT",       SockAccept,         "SockAccept"        , 1 },
 { "SOCKBIND",         SockBind,           "SockBind"          , 1 },
 { "SOCKCLOSE",        SockClose,          "SockClose"         , 1 },
 { "SOCKCONNECT",      SockConnect,        "SockConnect"       , 1 },
 { "SOCKDROPFUNCS",    SockDropFuncs,      "SockDropFuncs"     , 1 },
 { "SOCKGETHOSTBYADDR",SockGetHostByAddr,  "SockGetHostByAddr" , 1 },
 { "SOCKGETHOSTBYNAME",SockGetHostByName,  "SockGetHostByName" , 1 },
 { "SOCKGETHOSTID",    SockGetHostId,      "SockGetHostId"     , 1 },
 { "SOCKGETPEERNAME",  SockGetPeerName,    "SockGetPeerName"   , 1 },
 { "SOCKGETSOCKNAME",  SockGetSockName,    "SockGetSockName"   , 1 },
 { "SOCKGETSOCKOPT",   SockGetSockOpt,     "SockGetSockOpt"    , 1 },
 { "SOCKINIT",         SockInit,           "SockInit"          , 1 },
 { "SOCKIOCTL",        SockIoctl,          "SockIoctl"         , 1 },
 { "SOCKLISTEN",       SockListen,         "SockListen"        , 1 },
 { "SOCKPSOCK_ERRNO",  SockPSock_Errno,    "SockPSock_Errno"   , 1 },
 { "SOCKRECV",         SockRecv,           "SockRecv"          , 1 },
 { "SOCKRECVFROM",     SockRecvFrom,       "SockRecvFrom"      , 1 },
 { "SOCKSELECT",       SockSelect,         "SockSelect"        , 1 },
 { "SOCKSEND",         SockSend,           "SockSend"          , 1 },
 { "SOCKSENDTO",       SockSendTo,         "SockSendTo"        , 1 },
 { "SOCKSETSOCKOPT",   SockSetSockOpt,     "SockSetSockOpt"    , 1 },
 { "SOCKSHUTDOWN",     SockShutDown,       "SockShutDown"      , 1 },
 { "SOCKSOCK_ERRNO",   SockSock_Errno,     "SockSock_Errno"    , 1 },
 { "SOCKSOCKET",       SockSocket,         "SockSocket"        , 1 },
 { "SOCKSOCLOSE",      SockSoClose,        "SockSoClose"       , 1 },
 { "SOCKVERSION",      SockVersion,        "SockVersion"       , 1 },
 { "RXSOCKVERSION",    RxsockVersion,      "RxsockVersion"     , 1 },
 { "SOCKVARIABLE",     SockVariable,       "SockVariable"      , 1 },
 { "SOCKLOADFUNCS",    SockLoadFuncs,      "SockLoadFuncs"     , 0 },
 { NULL, NULL, NULL, 0 }
};

static RXSOCKDATA RxSockData = { NULL };

/********************************************************************
 * socksNotInitted is nonzero until the socket interface has been
 * initialized.
 ********************************************************************/
static int socksNotInitted = 1;

/********************************************************************
 * lastSockErrno holds the error code after the last socket
 * operation.
 ********************************************************************/
static int lastSockErrno = 0;
#ifdef __WIN32__
/*******************************************************************
 * translateError() will eventually convert Windows error codes into
 * OS/2 equivalents.
 *
 * For right now, does nothing.
 ******************************************************************/
# define translateError(x) (x)

# define UPDATE_ERRNO (lastSockErrno = translateError(WSAGetLastError()))
# define ZERO_ERRNO   (lastSockErrno = 0)
#else
# define UPDATE_ERRNO (lastSockErrno = errno)
# define ZERO_ERRNO   (lastSockErrno = 0)
#endif

static int initializeSockets( void );

/*
 * These functions used by loader.c to obtain package-specific info
 */
int RxSockInitialiser( RxPackageGlobalDataDef *MyGlob )
{
   RxSockData.RxPackageGlobalData = MyGlob;
   return initializeSockets();
}

int RxSockTerminator( RxPackageGlobalDataDef *MyGlob )
{
   /*
    * Shouldn't there be a call to:
    * WSACleanup(); for __WIN32__ ?
    */
   return 0;
}

RexxSubcomHandler *getRxSockSubcomHandler( void )
{
   return NULL;
}

RexxExitHandler *getRxSockInitHandler( void )
{
   return NULL;
}

RexxFunction *getRxSockFunctions( void )
{
   return RxSockFunctions;
}

PackageInitialiser *getRxSockInitialiser( void )
{
   return RxSockInitialiser;
}

PackageTerminator *getRxSockTerminator( void )
{
   return RxSockTerminator;
}

/********************************************************************
 * initializeSockets()
 * Initialize the socket interface.  Returns 0 if properly
 * initialized.
 ********************************************************************/
static int initializeSockets() {
#ifdef __WIN32__
   WORD vr = MAKEWORD(1, 1);
   WSADATA data;
   int rcode;

   socksNotInitted = 0;
   rcode = (WSAStartup(vr, &data) != 0);

   UPDATE_ERRNO;

   return rcode;
#else
   socksNotInitted = 0;
   ZERO_ERRNO;
   return 0;
#endif
}

#define CHECK_INIT if (socksNotInitted) if (initializeSockets()) return 40

/********************************************************************
 * SockAccept()
 *    Calls the C accept() function on the socket.
 *
 * Rexx call syntax:
 *    rc = SockAccept(socket [, address])
 ********************************************************************/
RFH_RETURN_TYPE SockAccept(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv, 
                         RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* 1 or 2 args. */
   if (argc == 1 || argc == 2) {
      u_int sockno;

      /* 1st parm is socket. */
      if (r2c_uint(&sockno, &argv[0])) {
         struct sockaddr *sa = 0;
         struct sockaddr_in sai;
         int sai_len = sizeof(sai);
         u_int rcode;

         rc = 0;
         /* 2nd parm, if avail, is ptr to sockaddr_in to receive */
         /* address of connected socket.                         */
         if (argc == 2) {
            sa = (struct sockaddr *)&sai;
         }

         /* Ready to accept() */
         rcode = accept(sockno, sa, &sai_len);
         UPDATE_ERRNO;
         retstr->strlength
            = sprintf(retstr->strptr, "%d", rcode);

         /* If rcode wasn't INVALID_SOCKET and 2 parms, return */
         /* the address.                                       */
         if (rcode != INVALID_SOCKET && argc == 2) {
            char stemName[MAXSTEMNAMELEN + 1];
            RXSTRING stem;

            stem = argv[1];

            memcpy(stemName, argv[1].strptr, argv[1].strlength);
            stemName[ argv[1].strlength ] = 0;
            
            make_upper(stemName);
            stem.strptr = stemName;

            c2r_sockaddr_in(&sai, &stem);
         }
      }
   }

   return rc;
}

/********************************************************************
 * SockBind()
 *    Calls the C bind() function on the socket.
 *
 * Rexx call syntax:
 *    rc = SockBind(socket, address)
 ********************************************************************/
RFH_RETURN_TYPE SockBind(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                         RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   if (argc == 2) {
      struct sockaddr_in sin;

      if (r2c_sockaddr_in(&sin, &argv[1])) {
         u_int sockno;
         if (r2c_uint(&sockno, &argv[0])) {
            int rcode;
            rcode = bind(sockno, (LPSOCKADDR)&sin, sizeof(sin));
            UPDATE_ERRNO;

            retstr->strlength
               = sprintf(retstr->strptr, "%d", rcode);
                         
            rc = 0;
         }
      }

      /* OS/2 RXSOCK seems to simply return -1 if parameters were */
      /* bad.                                                     */
      if (rc != 0) {
         rc = 0;
         strcpy(retstr->strptr, "-1");
         retstr->strlength = 2;
      }
   }

   return rc;
}

/********************************************************************
 * SockClose()
 *    Calls the C function to close the socket.
 *
 * Rexx call syntax:
 *    rc = SockClose(socket)
 ********************************************************************/
RFH_RETURN_TYPE SockClose(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                          RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;
   u_int sockno;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   if ( my_checkparam(  RxSockData.RxPackageGlobalData, name, argc, 1, 1 ) )
      return rc;

   /* Exactly 1 argument */

   if (r2c_uint(&sockno, &argv[0])) {
      int rcode;

      rcode = closesocket(sockno);
      UPDATE_ERRNO;

      return RxReturnNumber( RxSockData.RxPackageGlobalData, retstr, rcode );
   }

   return rc;
}

/********************************************************************
 * SockConnect()
 *    Calls the C function connect()
 *
 * Rexx call syntax:
 *    rc = SockConnect(socket, address)
 ********************************************************************/
RFH_RETURN_TYPE SockConnect(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                            RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

#if 0
   /* Some interpreters are not NULL terminating their strings. */
   {
      int i;

      for (i = 0; i < argc; i++) {
         argv[i].strptr[ argv[i].strlength ] = 0;
      }
   }
#endif

   if (argc == 2) {
      u_int sock;
      struct sockaddr_in sockaddr;

      if (r2c_uint(&sock, &argv[0])) {
         /*
          * OS/2 RXSOCK returns Rexx error 40 when the socket is
          * invalid but just returns -1 if the addr parameter
          * is invalid.
          */
         rc = 0;
         strcpy(retstr->strptr, "-1");
         retstr->strlength = 2;

         if (r2c_sockaddr_in(&sockaddr, &argv[1])) {
            int ret = connect(sock, (struct sockaddr *)&sockaddr,
                              sizeof(struct sockaddr_in));
            UPDATE_ERRNO;

            if (ret == 0) {
               /* Successfull connect. */
               *retstr->strptr = '0';
               retstr->strlength = 1;
            }
         }
      }
   }

   return rc;
}

/********************************************************************
 * SockDropFuncs()
 *    Drops all the functions here, making them unavailable to Rexx.
 *
 * Rexx call syntax:
 *    call SockDropFuncs
 *       RC - Null string.
 ********************************************************************/
RFH_RETURN_TYPE SockDropFuncs(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                              RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   ULONG rc=0;
   int unload=0;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );
   if ( my_checkparam( RxSockData.RxPackageGlobalData, name, argc, 0, 1 ) )
      return( 1 );
   if ( argv[0].strlength == 6
   &&   memcmpi( argv[0].strptr, "UNLOAD", 6 ) == 0 )
      unload = 1;
   (void)TermRxPackage( &RxSockData.RxPackageGlobalData, NULL, RxSockFunctions, RxPackageName, unload );
   return RxReturnNumber( NULL, retstr, rc );
}

/********************************************************************
 * SockGetHostByAddr()
 *    Calls C function gethostbyaddr()
 *
 * Rexx call syntax:
 *    rc = SockGetHostByAddr(address, 'host.' [, domain])
 ********************************************************************/
RFH_RETURN_TYPE SockGetHostByAddr(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                                  RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* 2 or 3 parameters. */
   if (argc == 2 || argc == 3) {
      int domain;

      /* Default to returning 0 (error) */
      rc = 0;
      *retstr->strptr = '0';
      retstr->strlength = 1;

      /* 3rd parameter muse be AF_INET, if specified. */
      if ((argc == 2)
          || r2c_SymbIntValue(&domain, &argv[2], AF_INET)) {
         long inetAddr;

         if (r2c_dotAddress(&inetAddr, &argv[0])) {
            /* Have the inet address.  OK to call gethostbyaddr() */
            struct hostent *h;

            h = gethostbyaddr((char *)&inetAddr, 4, PF_INET);
            UPDATE_ERRNO;
            if (h != 0) {
               char stemName[MAXSTEMNAMELEN + 1];
               RXSTRING stem;

               stem = argv[1];
               memcpy(stemName, argv[1].strptr, argv[1].strlength);
               stemName[ argv[1].strlength ] = 0;

               make_upper(stemName);
               stem.strptr = stemName;

               if (c2r_hostent(h, &stem)) {
                  *retstr->strptr = '1';
               }
            }
         }
      }
      
   }

   return rc;
}

/********************************************************************
 * SockGetHostByName()
 *    Calls C function gethostbyname()
 *
 * Rexx call syntax:
 *    rc = SockGetHostByName(name, 'host.')
 ********************************************************************/
RFH_RETURN_TYPE SockGetHostByName(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                                  RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   if ((argc == 2) && (argv[1].strlength <= MAXSTEMNAMELEN)) {
      char copy[MAXHOSTNAMELEN + 1];
      struct hostent *h;

      memcpy(copy, argv[0].strptr, argv[0].strlength);
      copy[ argv[0].strlength ] = 0;

      h = gethostbyname(copy);

      /*
       * OS/2 RXSOCK always has the socket errno set to 0 after a call
       * to SockGetHostByName (even if it fails).  I have no idea why.
       */
      ZERO_ERRNO;
      *retstr->strptr = '0';
      retstr->strlength = 1;

      if (h != 0) {
         char stemName[MAXSTEMNAMELEN + 1];
         RXSTRING stem;

         stem = argv[1];

         memcpy(stemName, argv[1].strptr, argv[1].strlength);
         stemName[ argv[1].strlength ] = 0;

         stem.strptr = stemName;

         /* The stem should be uppercase. */
         make_upper(stemName);
         if (c2r_hostent(h, &stem)) {
            *retstr->strptr = '1';
         }
      }

      /*---------------------------------------------------------
       - OS/2's RXSOCK exits with a return status of 0, without
       - setting any values for the stem.  Do the same thing here.
       ---------------------------------------------------------*/
      rc = 0;
   }

   return rc;
}

/********************************************************************
 * SockGetHostId()
 *    Calls C function gethostid()
 *
 * NOTE: Windows doesn't appear to have a gethostid() function.  It
 * must be simulated.
 *
 * Rexx call syntax:
 *    rc = SockGetHostId()
 ********************************************************************/
#ifndef HAVE_GETHOSTID
static u_long gethostid() {
   char hostname[256];
   struct hostent *host;

   gethostname(hostname, sizeof(hostname));
   host = gethostbyname(hostname);

   return (host ? *(u_long *)host->h_addr_list[0] : inet_addr("127.0.0.1"));
}
#endif

RFH_RETURN_TYPE SockGetHostId(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                              RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   int rc = 40;
   u_long hostId;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   hostId  = gethostid();
   UPDATE_ERRNO;

   /* No args. */
   /* OS/2 RXSOCK doesn't check for bad # of args. */
   rc = 0;
   
   retstr->strlength
      = sprintf(retstr->strptr, inet_ntoa(*(struct in_addr *)&hostId));

   return rc;
}

/********************************************************************
 * SockGetPeerName()
 *    Calls C function getpeername()
 *
 * Rexx call syntax:
 *    rc = SockGetPeerName(socket, 'addr.!')
 ********************************************************************/
RFH_RETURN_TYPE SockGetPeerName(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                                RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;
   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* Exactly 2 args. */
   if (argc == 2) {
      /* 1st arg is socket. */
      u_int sockno;

      if (r2c_uint(&sockno, &argv[0])) {
         /* 2nd arg a stem in which to return the socket address. */
         struct sockaddr_in sa;
         int    salen = sizeof(sa);
         int    rcode;

         rc = 0;
         rcode = getpeername(sockno, (struct sockaddr *)&sa, &salen);
         UPDATE_ERRNO;
         if (rcode == 0) {
            char stemName[MAXSTEMNAMELEN + 1];
            RXSTRING stem;

            stem = argv[1];
            memcpy(stemName, argv[1].strptr, argv[1].strlength);
            stemName[ argv[1].strlength ] = 0;
            
            /* Successfull call. */
            make_upper(stemName);

            stem.strptr = stemName;

            c2r_sockaddr_in(&sa, &stem);
         }

         retstr->strlength
            = sprintf(retstr->strptr, "%d", rcode);
      }
   }

   return rc;
}

/********************************************************************
 * SockGetSockName()
 *    Calls the C function getsockname().
 *
 * Rexx call syntax:
 *    rc = SockGetSockName(socket, 'addr.!')
 ********************************************************************/
RFH_RETURN_TYPE SockGetSockName(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                                RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* Exactly 2 args. */
   if (argc == 2) {
      /* 1st arg is socket */
      u_int sockno;

      if (r2c_uint(&sockno, &argv[0])) {
         /* Next arg is stem in which to put the socket address. */
         struct sockaddr_in sa;
         int salen = sizeof(sa);
         int rcode;

         rc = 0;

         rcode = getsockname(sockno, (struct sockaddr *)&sa, &salen);
         UPDATE_ERRNO;
         if (rcode == 0) {
            char stemName[MAXSTEMNAMELEN + 1];
            RXSTRING stem;

            stem = argv[1];

            memcpy(stemName, argv[1].strptr, argv[1].strlength);
            stemName[argv[1].strlength] = 0;

            stem.strptr = stemName;

            /* Successfull call. */
            make_upper(stemName);
            c2r_sockaddr_in(&sa, &stem);
         }
         
         retstr->strlength
            = sprintf(retstr->strptr, "%d", rcode);
      }
   }

   return rc;
}

/********************************************************************
 * SockGetSockOpt()
 *    Calls the C function getsockopt().
 *
 * Rexx call syntax:
 *    rc = SockGetSockOpt(socket, level, optVar, 'optVal')
 ********************************************************************/
RFH_RETURN_TYPE SockGetSockOpt(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                               RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* Exactly 4 parameters */
   if (argc == 4) {
      /* 1st arg is socket */
      u_int sockno;

      if (r2c_uint(&sockno, &argv[0])) {
         /* 2nd arg must be the string SOL_SOCKET */
         int level;

         if (r2c_SymbIntValue(&level, &argv[1], SOL_SOCKET)) {
            /* 3rd arg is the option to query. */
            int sockopt;

            /* Never return error 40 at this point. */
            rc = 0;

            if (r2c_sockopt_option(&sockopt, &argv[2])) {
               /* Last arg is variable to store the result. */
               union {
                  struct linger linger;
                  int    intOpt;
               } optVal;

               int optValLen = sizeof(optVal);
               int rcode;

               rcode = getsockopt(sockno, level, sockopt,
                                  (char *)&optVal, &optValLen);
               UPDATE_ERRNO;

               retstr->strlength
                  = sprintf(retstr->strptr, "%d", rcode);

               /* Convert the returned value. */
               /* All are int's except SO_LINGER and SO_TYPE */
               if (sockopt == SO_LINGER) {
                  /* Result should be 'l_onoff l_linger' */
                  char outbuf[256];
                  int outbuflen;

                  outbuflen
                     = sprintf(outbuf, "%d %d",
                               optVal.linger.l_onoff,
                               optVal.linger.l_linger);
                  setRexxVar(&argv[3], outbuf, outbuflen);
               }
               else if (sockopt == SO_TYPE) {
                  /* Result should be STREAM, DGRAM, or RAW */
                  switch (optVal.intOpt) {
                     case SOCK_STREAM: {
                        setRexxVar(&argv[3], "STREAM", 6);
                     }
                     break;

                     case SOCK_DGRAM: {
                        setRexxVar(&argv[3], "DGRAM", 5);
                     }
                     break;

                     case SOCK_RAW: {
                        setRexxVar(&argv[3], "RAW", 3);
                     }
                     break;

                     default: {
                        char outbuf[10];
                        int  outbuflen;

                        outbuflen = sprintf(outbuf, "%d",
                                            optVal.intOpt);
                        setRexxVar(&argv[3], outbuf, outbuflen);
                     }
                     break;
                  } /* switch */
               } /* else if SO_TYPE */
               else {
                  /* Integer value. */
                  char outbuf[10];
                  int  outbuflen;

                  outbuflen = sprintf(outbuf, "%d", optVal.intOpt);
                  setRexxVar(&argv[3], outbuf, outbuflen);
               }
            } /* if r2c_sockopt */
            else {
               /* OS/2 sets optVal to 0 for unknown options. */
               setRexxVar(&argv[3], "0", 1);
            }
         }
      }
   }

   return rc;
}

/********************************************************************
 * SockInit()
 *    Initializes the socket interface.
 *
 * Rexx call syntax:
 *    rc = SockInit()
 ********************************************************************/
RFH_RETURN_TYPE SockInit(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                         RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* No args. */
   if (argc == 0) {
      rc = 0;

      retstr->strlength = 1;
      *retstr->strptr = (initializeSockets() ? '1' : '0');
   }

   return rc;
}

/********************************************************************
 * SockIoctl()
 *    Calls ioctl() on the socket.
 *
 * Rexx call syntax:
 *    rc = SockIoctl(socket, ioctlCmd, ioctlData)
 ********************************************************************/
RFH_RETURN_TYPE SockIoctl(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                          RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;
   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* Exactly 3 args. */
   if (argc == 3) {
      u_int sockno;

      /* 1st arg is socket. */
      if (r2c_uint(&sockno, &argv[0])) {
         /* 2nd arg is ioctl command. */
         long ioctlCmd;
         int rcode = -1;

         rc = 0;
         if (r2c_SymbLongValue(&ioctlCmd, &argv[1], FIONBIO)
             || r2c_SymbLongValue(&ioctlCmd, &argv[1], FIONREAD)) {
            /* See which ioctl we are using. */
            if (ioctlCmd == FIONBIO) {
               u_long ioctlVal;

               if (r2c_ulong(&ioctlVal, &argv[2])) {
                  rcode = ioctlsocket(sockno, ioctlCmd, &ioctlVal);
                  UPDATE_ERRNO;
               }
            }
            else {  /* ioctlCmd == FIONREAD */
               u_long numRd;
               char   numRdStr[10];
               int    numRdStrLen;

               rcode = ioctlsocket(sockno, ioctlCmd, &numRd);
               numRdStrLen = sprintf(numRdStr, "%ld", numRd);
               setRexxVar(&argv[2], numRdStr, numRdStrLen);
            }
         }

         retstr->strlength = sprintf(retstr->strptr, "%d", rcode);
      }
   }

   return rc;
}

/********************************************************************
 * SockListen()
 *    Calls C function listen()
 *
 * Rexx call syntax:
 *    rc = SockListen(socket, backlog)
 ********************************************************************/
RFH_RETURN_TYPE SockListen(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                           RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* Exactly 2 args */
   if (argc == 2) {
      u_int sockno;

      /* Get the socket. */
      if (r2c_uint(&sockno, &argv[0])) {
         int backlog;

         if (r2c_int(&backlog, &argv[1])) {
            int rcode;

            rc = 0;
            rcode = listen(sockno, backlog);
            UPDATE_ERRNO;
            retstr->strlength
               = sprintf(retstr->strptr, "%d", rcode);
         }

         /* OS/2 returns error 40 if either parameter was not */
         /* numeric.  Do the same.                            */
      }
   }

   return rc;
}

/********************************************************************
 * SockLoadFuncs()
 *    Makes all the functions here available to Rexx.
 *
 * Rexx call syntax:
 *    call SockLoadFuncs
 *       RC - Null string.
 ********************************************************************/
RFH_RETURN_TYPE SockLoadFuncs(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                              RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   int rc = 0L;

#if defined(DYNAMIC_LIBRARY)
   RxSockData.RxPackageGlobalData = InitRxPackage( RxSockData.RxPackageGlobalData, RxSockInitialiser, &rc );
   if ( !QueryRxFunction( RxSockData.RxPackageGlobalData, "SOCKBIND" ) )
   {
      /* 
       * Register all external functions
       */
      if ( !rc )
      {
         rc = RegisterRxFunctions( RxSockData.RxPackageGlobalData, RxSockFunctions, RxPackageName );
      }
   }
#endif
   return RxReturnNumber( RxSockData.RxPackageGlobalData, retstr, rc );
}

/********************************************************************
 * SockPSock_Errno()
 *    Calls the C function psock_errno()
 *
 * Rexx call syntax:
 *    rc = SockPSock_Errno([message])
 ********************************************************************/
void psock_errno(char const *msg) {
   typedef struct {
      int         sockErrno;
      char const *errmsg;
   } psock_conv;

#ifdef __WIN32__
   static psock_conv errTbl[] = {
      { 0,        "No error" },
      { WSAEINTR, "Interrupted system call" },
      { WSAEBADF, "Bad file number" },
      { WSAEACCES, "Permission denied" },
      { WSAEFAULT, "Bad address" },
      { WSAEINVAL, "Invalid argument" },
      { WSAEMFILE, "Too many files/sockets open" },
      { WSAEWOULDBLOCK, "Blocking operation on non-blocking socket" },
      { WSAEINPROGRESS, "Operation now in progress" },
      { WSAEALREADY, "Operation already in progress" },
      { WSAENOTSOCK, "Socket operation on non-socket" },
      { WSAEDESTADDRREQ, "Destination address required" },
      { WSAEMSGSIZE, "Message too long" },
      { WSAEPROTOTYPE, "Protocol wrong type for socket" },
      { WSAENOPROTOOPT, "Option not supported by protocol" },
      { WSAEPROTONOSUPPORT, "Protocol not supported" },
      { WSAESOCKTNOSUPPORT, "Socket type not supported" },
      { WSAEOPNOTSUPP, "Operation not supported on socket" },
      { WSAEPFNOSUPPORT, "Protocol family not supported" },
      { WSAEAFNOSUPPORT, "Address family not supported by protocol family" },
      { WSAEADDRINUSE, "Address already in use" },
      { WSAEADDRNOTAVAIL, "Can't assigned requested address" },
      { WSAENETDOWN, "Network is down" },
      { WSAENETUNREACH, "Network is unreachable" },
      { WSAENETRESET, "Network dropped connection on reset" },
      { WSAECONNABORTED, "Software caused connection abort" },
      { WSAECONNRESET, "Connection reset by peer" },
      { WSAENOBUFS, "No buffer space available" },
      { WSAEISCONN, "Socket is already connected" },
      { WSAENOTCONN, "Socket is not connected" },
      { WSAESHUTDOWN, "Can't send after socket shutdown" },
      { WSAETOOMANYREFS, "Too many references: can't splice" },
      { WSAETIMEDOUT, "Connection timed out" },
      { WSAECONNREFUSED, "Connection refused" },
      { WSAELOOP, "Too many levels of symbolic links" },
      { WSAENAMETOOLONG, "File name too long" },
      { WSAEHOSTDOWN, "Host is down" },
      { WSAEHOSTUNREACH, "No route to host" },
      { WSAENOTEMPTY, "Directory not empty" },
      { WSAEPROCLIM, "WSAEPROCLIM" },
      { WSAEUSERS, "WSAEUSERS" },
      { WSAEDQUOT, "WSAEDQUOT" },
      { WSAESTALE, "WSAESTALE" },
      { WSAEREMOTE, "WSAEREMOTE" },
      { WSAEDISCON, "Other end disconnected" },
      { WSASYSNOTREADY, "Underlying network subsystem not ready" },
      { WSAVERNOTSUPPORTED, "Cannot find usefull WINSOCK" },
      { WSANOTINITIALISED, "Internal RXSOCK error WSANOTINITIALIZED" }
   };
#endif

   int i;

   if (msg && *msg) {
      fprintf(stderr, "%s: ", msg);
   }

#ifdef __WIN32__
   /* Search for the errno in errTbl */
   i = 0;
   while ((i < sizeof(errTbl) / sizeof(*errTbl))
          && (errTbl[i].sockErrno != lastSockErrno)) {
      i++;
   }

   if (i == sizeof(errTbl) / sizeof(*errTbl)) {
      /* Not found in table. */
      fprintf(stderr, "Error code %d\n", lastSockErrno);
   }
   else {
      fprintf(stderr, "%s\n", errTbl[i].errmsg);
   }
#else
   fprintf(stderr,"%s\n", strerror( lastSockErrno ) );
#endif
}

RFH_RETURN_TYPE SockPSock_Errno(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                                RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;
   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* 0 or 1 args. */
   if (argc == 0 || argc == 1) {
      rc = 0;

      retstr->strlength = 0;

      if (argc == 0 || argv[0].strlength == 0) {
         psock_errno(0);
      }
      else {
         char *copy = alloca(argv[0].strlength + 1);
         memcpy(copy, argv[0].strptr, argv[0].strlength);
         copy[argv[0].strlength] = 0;
         psock_errno(copy);
      }
   }

   return rc;
}

/********************************************************************
 * SockRecv()
 *    Calls the C recv() function.
 *
 * NOTE: Not MT-Safe since it calls strtok().
 *
 * Rexx call syntax:
 *    rc = SockRecv(socket, var, len [, flags])
 ********************************************************************/
RFH_RETURN_TYPE SockRecv(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                         RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   if (argc == 3 || argc == 4) {
      u_int sockno;

      if (r2c_uint(&sockno, &argv[0])) {
         int len;

         /*
          * OS/2 RXSOCK exits with error 40 (Inavlid call) if length
          * parameter is <= 0 (or not a number).
          */
         if (r2c_int(&len, &argv[2]) && (len > 0)) {
            int flags = 0;
            char buf[512];     /* OS/2 RXSOCK maxes out at 512 */
            int  maxLen = min(len, 512);
            int  numRd;

            rc = 0;

            /* Check for flags. */
            if (argc == 4) {
               r2c_recv_flags(&flags, &argv[3]);
            }

            numRd = recv(sockno, buf, maxLen, flags);
            UPDATE_ERRNO;

            retstr->strlength
               = sprintf(retstr->strptr, "%d", numRd);

            if (numRd < 0)
               numRd = 0;

            setRexxVar(&argv[1], buf, numRd);
         }
      }
   }

   return rc;
}

/********************************************************************
 * SockRecvFrom()
 *    Calls the C recvfrom() function.
 *
 * Rexx call syntax:
 *    rc = SockRecvFrom(socket, 'buf', maxBytes [, flags], 'clntAddr.!')
 ********************************************************************/
RFH_RETURN_TYPE SockRecvFrom(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                             RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;
   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* 4 or 5 args. */
   if (argc == 4 || argc == 5) {
      /* 1st arg is socket. */
      u_int sockno;

      if (r2c_uint(&sockno, &argv[0])) {
         /* 3rd arg is max # of bytes to read. */
         int maxBytes;

         if (r2c_int(&maxBytes, &argv[2])) {
            /* If 5 args, 4th is flags and 5th is client stem. */
            /* Otherwise, 4th is client stem.                  */
            int flags = 0;
            struct sockaddr_in from;
            int fromLen = sizeof(from);
            int rcode;
            char buf[512]; /* OS/2 RXSOCK maxes out at 512 */
            int  maxLen = min(maxBytes, 512);
            PRXSTRING clnt = &argv[3];

            rc = 0;

            if (argc == 5) {
               r2c_recv_flags(&flags, &argv[3]);
               clnt = &argv[4];
            }

            /* Ready to call recvfrom() */
            rcode = recvfrom(sockno, buf, maxLen, flags,
                             (struct sockaddr *)&from, &fromLen);
            UPDATE_ERRNO;

            retstr->strlength = sprintf(retstr->strptr, "%d", rcode);

            /* Fill the buf and client stem variables. */
            setRexxVar(&argv[1], buf, rcode);
            {
               char stemName[MAXSTEMNAMELEN + 1];
               RXSTRING stem;

               stem = *clnt;

               memcpy(stemName, clnt->strptr, clnt->strlength);
               stemName[ clnt->strlength ] = 0;
               
               make_upper(stemName);

               stem.strptr = stemName;
               c2r_sockaddr_in(&from, &stem);
            }
         }
      }
   }

   return rc;
}

/********************************************************************
 * SockSelect()
 *    Calls the C select() function.
 *
 * *** Max size for any of the fd_set's is 64. ***
 * *** THIS CALL MAY VARY SLIGHTLY FROM OS/2 RXSOCK's BEHAVIOUR. ***
 * *** SEE COMMENTS IN CODE BELOW FOR EXPLANATION.               ***
 *
 * Rexx call syntax:
 *    rc = SockSelect(reads, writes, excepts [, timeoutsecs])
 ********************************************************************/
RFH_RETURN_TYPE SockSelect(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                           RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;
   int max_fd=0;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   if (argc == 3 || argc == 4) {
      fd_set fds[3];
      fd_set *fdsp[3] = { 0, 0, 0 };
      int fdin[3][FD_SETSIZE];
      int numin[3];
      int i;
      struct timeval tim;
      struct timeval *timp = 0;
      int anyFds = 0;    /* Flag for: Are any non-empty fds specified? */

      /*
       * Never return 'Invalid call' if number of parameters is
       * correct.
       */
      rc = 0;

      /* Read the fd_sets */
      for (i = 0; i < 3; i++) {
         if (argv[i].strlength != 0) {
            /* This set was specified.  Convert it. */
            int curr_max = 0;

            numin[i] = FD_SETSIZE;
            curr_max = r2c_fd_setarray(&fds[i], (int *)&fdin[i], &argv[i], &numin[i]);
            if (curr_max == -1) {
               /*
                * Too many sockets specified.  Can't do much other
                * than simply exit.
                * NOTE:
                *   This may be different that OS/2 RXSOCK's behavior.
                *   OS/2 RXSOCK's max sockets and its behavior when
                *   that maximum is exceeded is not documented and I
                *   did not try to determine them experimentaly.
                */
               strcpy(retstr->strptr, "-1");
               retstr->strlength = 2;
               return 0;
            }
            else {
               /*
                * If numin[i] == 0, we are not using any sockets for this
                * fd_set.  To get the same behaviour as OS/2's RXSOCK
                * we can't set fdsp[i] to an empty fd_set, we must
                * instead use the NULL pointer. (WinSock will return
                * immediately, OS/2 doesn't).
                */
               if (curr_max > max_fd)
                  max_fd = curr_max;
               if (numin[i] != 0) {
                  fdsp[i] = &fds[i];
                  anyFds = 1;
               }
            }
         }
      }

      /* Read the timeout if it was specified. */
      if (argc == 4) {
         int secs;

         /*
          * OS/2's RXSOCK seems to consider an invalid timeval to be
          * be the same as specifying 0.
          */
         if (!r2c_int(&secs, &argv[3])) {
            secs = 0;
         }

         timp = &tim;
         tim.tv_sec = secs;
         tim.tv_usec = 0;
      }

      /* We are finally ready to make the call to select. */
      /*
       * WinSock select() returns immediatly if all fds are
       * unspecified or empty.  OS/2's RXSOCK waits for timeout
       * to expire before returning (this is documented behavior in
       * RXSOCK.INF).  Simulate OS/2 RXSOCK's behavior here.
       */
      if (anyFds) {
         int rcode;

         rcode = select(max_fd+1, fdsp[0], fdsp[1], fdsp[2], timp);
         UPDATE_ERRNO;

         retstr->strlength
            = sprintf(retstr->strptr, "%d", rcode);

         /* Now we have to copy the fd_array back to the Rexx arrays. */
         for (i = 0; i < 3; i++) {
#ifdef WIN32
            if ((fdsp[i] != 0)
                && (c2r_fd_setarray(fdsp[i], fdin[i], &argv[i], numin[i]) == 0)) {
#else
            if ((c2r_fd_setarray(fdsp[i], fdin[i], &argv[i], numin[i]) == 0)) {
#endif
               /* Conversion error.  exit. */
               strcpy(retstr->strptr, "-1");
               retstr->strlength = 2;
               return 0;
            }
         }
      }
      else {
         /* Return 0. */
         *retstr->strptr = '0';
         retstr->strlength = 1;

         if (timp) {
#if defined(_MSC_VER)
            Sleep(timp->tv_sec*1000);
#else
            sleep(timp->tv_sec);
#endif
         }

         /* No socket error if we get here. */
         ZERO_ERRNO;
      }
   }

   return rc;
}

/********************************************************************
 * SockSend()
 *    Calls the C send() function to send data on a connected socket.
 *
 * *** NOT MT_SAFE since it calls strtok() ***
 *
 * Rexx call syntax:
 *    rc = SockSend(socket, data [, flags])
 ********************************************************************/
RFH_RETURN_TYPE SockSend(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                         RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   if ((argc == 2) || (argc == 3)) {
      u_int sockno;

      if (r2c_uint(&sockno, &argv[0])) {
         /* Check for flags. */
         int flags = 0;
         int rcode;

         if (argc == 3) {
            char *flagsstr = strtok(argv[2].strptr, " ");
            while (flagsstr != 0) {
               if (!strcmp(flagsstr, "MSG_OOB")) {
                  flags |= MSG_OOB;
               }
               else if (!strcmp(flagsstr, "MSG_DONTROUTE")) {
                  flags |= MSG_DONTROUTE;
               }

               flagsstr = strtok(0, " ");
            }
         }

         rcode = send(sockno, argv[1].strptr, argv[1].strlength,
                      flags);

         UPDATE_ERRNO;
         retstr->strlength
            = sprintf(retstr->strptr, "%d", rcode);
                      
         rc = 0;
      }
   }

   return rc;
}

/********************************************************************
 * SockSendTo()
 *    Calls the C function sendto().
 *
 * Rexx call syntax:
 *    rc = SockSendTo(socket, data, [, flags] , 'address.!')
 ********************************************************************/
RFH_RETURN_TYPE SockSendTo(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                           RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* 3 or 4 params. */
   if (argc == 3 || argc == 4) {
      /* 1 param is socket. */
      u_int sockno;

      /*
       * OS/2 RXSOCK returns error 40 if sockno is not a number or if
       * data has length 0.
       */
      if (r2c_uint(&sockno, &argv[0]) && (argv[1].strlength > 0)) {
         /* 2nd param is data to send. */
         /* If 4 params, 3rd is flags. */
         /* Last param is socket address. */
         int flags = 0;
         struct sockaddr_in sa;
         int rcode;

         rc = 0;

         if (argc == 4) {
            r2c_SymbIntValue(&flags, &argv[2], MSG_DONTROUTE);
            r2c_sockaddr_in(&sa, &argv[3]);
         }
         else {
            r2c_sockaddr_in(&sa, &argv[2]);
         }

         rcode = sendto(sockno, argv[1].strptr, argv[1].strlength,
                        flags, (struct sockaddr *)&sa, sizeof(sa));
         UPDATE_ERRNO;

         retstr->strlength = sprintf(retstr->strptr, "%d", rcode);
      }
   }

   return rc;
}

/********************************************************************
 * SockSetSockOpt()
 *    Calls the C function setsockopt().
 *
 * Rexx call syntax:
 *    rc = SockSetSockOpt(socket, level, optVar, optVal)
 ********************************************************************/
RFH_RETURN_TYPE SockSetSockOpt(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                               RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* Exactly 4 parameters */
   if (argc == 4) {
      /* 1st arg is socket */
      u_int sockno;

      if (r2c_uint(&sockno, &argv[0])) {
         /* 2nd arg must be the string SOL_SOCKET */
         int level;

         if (r2c_SymbIntValue(&level, &argv[1], SOL_SOCKET)) {
            /* 3rd arg is the option to set. */
            int sockopt;

            /* Never return error 40 at this point. */
            rc = 0;

            if (r2c_sockopt_option(&sockopt, &argv[2])) {
               /* Last arg is value to set. */
               /* Usually integer, only exception is SO_LINGER */
               /* (SO_TYPE is not allowed for setsockopt())    */
               int rcode = -1;

               if (sockopt != SO_LINGER) {
                  int optVal;

                  if (r2c_int(&optVal, &argv[3])) {
                     rcode = setsockopt(sockno, level, sockopt,
                                        (char *)&optVal,
                                        sizeof(optVal));
                     UPDATE_ERRNO;
                  }
               }
               else {
                  /* SO_LINGER.  Break into 2 ints. */
                  int l_onoff;
                  int l_linger; 
                  struct linger linger;

                  if (r2c_2_ints(&l_onoff, &l_linger,
                                 &argv[3])) {
                     linger.l_onoff = l_onoff;
                     linger.l_linger = l_linger;
                     rcode = setsockopt(sockno, level, sockopt,
                                        (char *)&linger,
                                        sizeof(linger));
                     UPDATE_ERRNO;
                  }
               }

               retstr->strlength
                  = sprintf(retstr->strptr, "%d", rcode);
            } /* if r2c_sockopt_option */
            else {
               retstr->strlength = 2;
               strcpy(retstr->strptr, "-1");
            }
         }
      }
   }

   return rc;
}

/********************************************************************
 * SockShutDown()
 *    Calls the C function shutdown()
 *
 * Rexx call syntax:
 *    rc = SockShutDown(socket, how)
 ********************************************************************/
RFH_RETURN_TYPE SockShutDown(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                             RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* Exactly 2 parameters. */
   if (argc == 2) {
      /* 1st arg is the socket number. */
      u_int sockno;

      /* OS/2 RXSOCK returns error 40 if either are is non-numeric. */
      if (r2c_uint(&sockno, &argv[0])) {
         /* 2nd arg is how to shutdown. */
         int how;

         if (r2c_int(&how, &argv[1])) {
            int rcode;

            rc = 0;

            rcode = shutdown(sockno, how);
            UPDATE_ERRNO;
            retstr->strlength = sprintf(retstr->strptr, "%d", rcode);
         }
      }
   }

   return rc;
}

/********************************************************************
 * SockSock_Errno()
 *    Returns the error code of the last socket operation.
 *
 * Rexx call syntax:
 *    rc = SockSock_Errno()
 ********************************************************************/
RFH_RETURN_TYPE SockSock_Errno(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                               RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;
   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* No args but OS/2 RXSOCK doesn't check for the number of args */
   rc = 0;

   retstr->strlength = sprintf(retstr->strptr, "%d", lastSockErrno);

   return rc;
}

/********************************************************************
 * SockSocket()
 *    Calls the C socket() function to create a socket.
 *
 * Rexx call syntax:
 *    rc = SockSocket(domain, type, protocol)
 ********************************************************************/
RFH_RETURN_TYPE SockSocket(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                           RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;

   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* Make sure exactly 3 args. */
   if (argc == 3) {
      int domain;

      /* make sure domain is "AF_INET" */
      if (r2c_SymbIntValue(&domain, &argv[0], AF_INET)) {
         int type;
         /* Convert type parameter. */
         if (r2c_SymbIntValue(&type, &argv[1], SOCK_STREAM)
             || r2c_SymbIntValue(&type, &argv[1], SOCK_DGRAM)
             || r2c_SymbIntValue(&type, &argv[1], SOCK_RAW)) {
            /* Type is OK.  Convert protocol. */
            int protocol;
            if (r2c_SymbIntValue(&protocol, &argv[2], IPPROTO_UDP)
                || r2c_SymbIntValue(&protocol, &argv[2], IPPROTO_TCP)
                || r2c_SymbIntValue(&protocol, &argv[2], 0)) {
               /* Everything is OK. Call socket() */
               u_int sockNum;

               sockNum = socket(domain, type, protocol);
               UPDATE_ERRNO;
               retstr->strlength = sprintf(retstr->strptr, "%d", sockNum);
               rc = 0;
            }
         }
      }
   }

   return rc;
}

/********************************************************************
 * SockSoClose()
 *    Equivalent to SockClose()
 *
 * Rexx call syntax:
 *    rc = SockSoClose(socket)
 ********************************************************************/
RFH_RETURN_TYPE SockSoClose(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                            RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {

   return SockClose(name, argc, argv, queuename, retstr);
}

/********************************************************************
 * SockVariable()
 *    Set/get internal variable stuff
 *
 * Rexx call syntax:
 *    rc = SockVariable(name[,value])
 ********************************************************************/
RFH_RETURN_TYPE SockVariable(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv, 
                             RFH_ARG3_TYPE stck, RFH_ARG4_TYPE retstr)
{
   ULONG rc = 0L;
   char buf[50];

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );
   if ( my_checkparam(  RxSockData.RxPackageGlobalData, name, argc, 1, 2 ) )
      return( 40 );
   if ( argv[0].strlength == 5 && memcmpi( "DEBUG", argv[0].strptr, argv[0].strlength ) == 0 )
   {
      if ( argc == 1 )
      {
         sprintf( buf, "%d", RxGetRunFlags( RxSockData.RxPackageGlobalData ) );
         return RxReturnString( RxSockData.RxPackageGlobalData, retstr, buf );
      }
      else
      {
         if ( StrToInt( &argv[1], (ULONG *)&rc ) == -1 )
            return RxReturnString( RxSockData.RxPackageGlobalData, retstr, "ERROR: Invalid DEBUG value. Cannot set variable; DEBUG" );
         RxSetRunFlags ( RxSockData.RxPackageGlobalData, rc );
         return RxReturnNumber( RxSockData.RxPackageGlobalData, retstr, 0 );
      }
   }
   else if ( argv[0].strlength == 7 && memcmpi( "VERSION", argv[0].strptr, argv[0].strlength ) == 0 )
   {
      if ( argc == 1 )
      {
         sprintf( buf, "%s %s %s", RxPackageName, RXSOCK_VERSION, RXSOCK_DATE );
         return RxReturnString( RxSockData.RxPackageGlobalData, retstr, buf );
      }
      else
         return RxReturnString( RxSockData.RxPackageGlobalData, retstr, "ERROR: Cannot set variable; VERSION" );
   }
   else if ( argv[0].strlength == 9 && memcmpi( "DEBUGFILE", argv[0].strptr, argv[0].strlength ) == 0 )
   {
      if ( argc == 1 )
         return RxReturnString( RxSockData.RxPackageGlobalData, retstr, RxGetTraceFile( RxSockData.RxPackageGlobalData ) );
      else
      {
         rc = RxSetTraceFile( RxSockData.RxPackageGlobalData, (char *)argv[1].strptr );
         return RxReturnNumber( RxSockData.RxPackageGlobalData, retstr, rc );
      }
   }
   sprintf( buf, "ERROR: Invalid variable; %s", argv[0].strptr );
   return RxReturnString( RxSockData.RxPackageGlobalData, retstr, buf );
}

/********************************************************************
 * SockVersion()
 *    Returns 2.0
 *
 * Rexx call syntax:
 *    v = SockVersion()
 ********************************************************************/
RFH_RETURN_TYPE SockVersion(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                            RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   APIRET rc = 40;
   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* No args */
   /* OS/2 RXSOCK doesn't checks for bad # of args */
   return RxReturnString( RxSockData.RxPackageGlobalData, retstr, "2.0" ) ;
}

/********************************************************************
 * SockVersion()
 *    Returns the current version of RXSOCK.DLL.
 *
 * Rexx call syntax:
 *    v = RxsockVersion()
 ********************************************************************/
RFH_RETURN_TYPE RxsockVersion(RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc, RFH_ARG2_TYPE argv,
                            RFH_ARG3_TYPE queuename, RFH_ARG4_TYPE retstr) {
   CHECK_INIT;

   RxSockData.RxPackageGlobalData = FunctionPrologue( RxSockData.RxPackageGlobalData, NULL, (char *)name, argc, argv );

   /* No args */
   /* OS/2 RXSOCK doesn't checks for bad # of args */

   return RxReturnString( RxSockData.RxPackageGlobalData, retstr, RXSOCK_VERSION ) ;
}

/*
 * The following functions are used in rxpackage.c
 */

/*-----------------------------------------------------------------------------
 * Print a usage message.
 *----------------------------------------------------------------------------*/
void RxSockUsage

#ifdef HAVE_PROTO
   (void)
#else
   ()
#endif

{
   (void)fprintf(stderr,
      "\nVersion: %s %s %s\n\nUsage:   %s [-h]\n         %s [-idvf<trace file>] [Rexx program name]\n\n",
      RxPackageName,
      RXSOCK_VERSION,
      RXSOCK_DATE,
      RxPackageName,
      RxPackageName);
   exit( 1 );
}

#if defined(USE_REXX6000)
/*
 * This function is used as the entry point for the REXX/6000
 * interpreter
 * If you change this table, don't forget to change the table at the
 * start of this file.
 */
USHORT InitFunc( RXFUNCBLOCK **FuncBlock )
{
   static RXFUNCBLOCK funcarray[] =
   {
      { "SOCKACCEPT",       SockAccept,         NULL                 },
      { "SOCKBIND",         SockBind,           NULL                 },
      { "SOCKCLOSE",        SockClose,          NULL                 },
      { "SOCKCONNECT",      SockConnect,        NULL                 },
      { "SOCKDROPFUNCS",    SockDropFuncs,      NULL                 },
      { "SOCKGETHOSTBYADDR",SockGetHostByAddr,  NULL                 },
      { "SOCKGETHOSTBYNAME",SockGetHostByName,  NULL                 },
      { "SOCKGETHOSTID",    SockGetHostId,      NULL                 },
      { "SOCKGETPEERNAME",  SockGetPeerName,    NULL                 },
      { "SOCKGETSOCKNAME",  SockGetSockName,    NULL                 },
      { "SOCKGETSOCKOPT",   SockGetSockOpt,     NULL                 },
      { "SOCKINIT",         SockInit,           NULL                 },
      { "SOCKIOCTL",        SockIoctl,          NULL                 },
      { "SOCKLISTEN",       SockListen,         NULL                 },
      { "SOCKPSOCK_ERRNO",  SockPSock_Errno,    NULL                 },
      { "SOCKRECV",         SockRecv,           NULL                 },
      { "SOCKRECVFROM",     SockRecvFrom,       NULL                 },
      { "SOCKSELECT",       SockSelect,         NULL                 },
      { "SOCKSEND",         SockSend,           NULL                 },
      { "SOCKSENDTO",       SockSendTo,         NULL                 },
      { "SOCKSETSOCKOPT",   SockSetSockOpt,     NULL                 },
      { "SOCKSHUTDOWN",     SockShutDown,       NULL                 },
      { "SOCKSOCK_ERRNO",   SockSock_Errno,     NULL                 },
      { "SOCKSOCKET",       SockSocket,         NULL                 },
      { "SOCKSOCLOSE",      SockSoClose,        NULL                 },
      { "SOCKVERSION",      SockVersion,        NULL                 },
      { "RXSOCKVERSION",    RxsockVersion,      NULL                 },
      { "SOCKVARIABLE",     SockVariable,       NULL                 },
      { "SOCKLOADFUNCS",    SockLoadFuncs,      NULL                 },
      { NULL, NULL, NULL }
   } ;
   *FuncBlock = funcarray;
   return (USHORT)0;
}
#endif

