/*===================================================================
 = conversions.h
 =
 = Contains prototypes of functions to copy/read values and structures to/from
 = the Rexx variable pool.
 =
 = $Log: conversions.h,v $
 = Revision 1.2  2003/01/22 03:33:08  mark
 = Changes for "standard" package structure
 =
 = Revision 1.1.1.1  2002/11/29 23:46:24  mark
 = Importing RxSock 
 =
 = Revision 1.3  1998/10/13 02:13:25  abbott
 = Added prototype for c2r_fd_setarray() needed by SockSelect().
 =
 = Revision 1.2  1998/09/20 23:28:23  abbott
 = Changes to better support WinRexx.  Also added REXXTRANS configuration.
 =
 ===================================================================*/
#ifndef _conversions_h
#define _conversions_h

#include "rxpack.h"

#ifdef HAVE_TYPES_H
#undef _POSIX_SOURCE
#include <types.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_SOCKET_H
# include <socket.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# ifdef HAVE_TIME_H
#  include <time.h>
# endif
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  ifdef HAVE_TIME_H
#   include <time.h>
#  endif
# endif
#endif

#ifdef HAVE_WINSOCK_H
# include <winsock.h>
#else
typedef struct sockaddr_in SOCKADDR_IN;
# define ioctlsocket(x,y,z) ioctl(x,y,z)
# define closesocket(x) close(x)
# define INVALID_SOCKET (-1)
# define LPSOCKADDR struct sockaddr *
#endif

#ifndef INADDR_NONE
# define INADDR_NONE (-1)
#endif

typedef struct
{
   RxPackageGlobalDataDef *RxPackageGlobalData;
} RXSOCKDATA;

/*======================================================
 = r2c_SymbIntValueFunc()
 = Tests to see if the Rexx string rxTstVal is the same
 = as cTstVal.  If it is, sets intVal to cvtVal
 = and returns 1.  Otherwise returns 0 without modifying
 = intVal.
 ======================================================*/
int r2c_SymbIntValueFunc(int *intVal, int cvtVal,
                    PRXSTRING rxTstVal, const char *cTstVal);

/*======================================================
 = r2c_SymbIntValue()
 = Macro to ease use of r2c_SymbIntValueFunc
 ======================================================*/
#define r2c_SymbIntValue(result, rxVal, symb) \
        r2c_SymbIntValueFunc((result), (symb), (rxVal), (#symb))

/*======================================================
 = r2c_SymbLongValue()
 = Tests to see if a Rexx string is the given symbolic value.
 = If it is, sets result to that value and
 = returns 1.  Otherwise returns 0 without nodifying result.
 ======================================================*/
#define r2c_SymbLongValue(result, rxVal, symb) \
        r2c_SymbIntValueFunc((int *)(result), (symb), (rxVal), (#symb))

/*======================================================
 = r2c_SymbUshortValueFunc()
 = Tests to see if the Rexx string rxTstVal is the same
 = as cTstVal.  If it is, sets intVal to cvtVal
 = and returns 1.  Otherwise returns 0 without modifying
 = intVal.
 ======================================================*/
int r2c_SymbUshortValueFunc(u_short *intVal, u_short cvtVal,
                    PRXSTRING rxTstVal, const char *cTstVal);

/*======================================================
 = r2c_SymbUshortValue()
 = Macro to ease use of r2c_SymbIntValueFunc
 ======================================================*/
#define r2c_SymbUshortValue(result, rxVal, symb) \
        r2c_SymbUshortValueFunc((result), (symb), (rxVal), (#symb))


/*======================================================
 = r2c_int()
 = Converts a Rexx variable to an int.  If successful,
 = returns 1.  Otherwise returns 0.
 ======================================================*/
int r2c_int(int *newValue, PRXSTRING rx);

/*======================================================
 = r2c_uint()
 = Converts a Rexx variable to a u_int.  If successful,
 = returns 1.  Otherwise returns 0.
 ======================================================*/
int r2c_uint(u_int *newValue, PRXSTRING rx);

/*======================================================
 = r2c_ulong()
 = Converts a Rexx variable to a u_long.  If successful,
 = returns 1.  Otherwise returns 0.
 ======================================================*/
#define r2c_ulong(newValue, rx) r2c_uint((u_int *)newValue, rx)

/*======================================================
 = r2c_ushort()
 = Converts a Rexx variable to an unsigned short.  If successful,
 = returns 1.  Otherwise returns 0.
 ======================================================*/
int r2c_ushort(u_short *newValue, PRXSTRING rx);

/*======================================================
 = r2c_ushort_htons()
 = Converts a Rexx variable to an unsigned short in network
 = byte order.
 = If successful,
 = returns 1.  Otherwise returns 0.
 ======================================================*/
int r2c_ushort_htons(u_short *newValue, PRXSTRING rx);

/*======================================================
 = r2c_dotAddress()
 = Converts a Rexx variable that is an internet dot address to
 = its C value.  If successful,
 = returns non-zero.  Otherwise returns 0.
 ======================================================*/
int r2c_dotAddress(unsigned long *newValue, PRXSTRING rx);

/*======================================================
 = r2c_sockaddr_in()
 = Converts a Rexx variable to a sockaddr_in.  If successful,
 = returns 1.  Otherwise returns 0.
 ======================================================*/
int r2c_sockaddr_in(SOCKADDR_IN *newValue, PRXSTRING rx);

/*======================================================
 = c2r_sockaddr_in()
 = Converts a sockaddr_in to a Rexx stem variable.
 =  If successful, returns 1.  Otherwise returns 0.
 ======================================================*/
int c2r_sockaddr_in(SOCKADDR_IN const *value, PRXSTRING rx);

/*======================================================
 = c2r_hostent()
 = Converts a hostent to a Rexx stem variable.
 =  If successful, returns 1.  Otherwise returns 0.
 ======================================================*/
int c2r_hostent(struct hostent const *value, PRXSTRING rx);

/*======================================================
 = r2c_fd_setarray()
 = Converts a Rexx stem array of integers to a C fd_set.
 = When called, aryLen should be the size of the array 'ary'.
 = On return, aryLen will be the number of members actually
 = converted.
 = If successful, returns the maximum value of all fds.  
 = Otherwise returns -1.
 ======================================================*/
int r2c_fd_setarray(fd_set *ary, int *fdin, PRXSTRING rx, int *aryLen);

/*======================================================
 = c2r_fd_setarray()
 = Converts a C fd_set to a Rexx stem array of integers.
 = If successful, returns 1.  Otherwise returns 0.
 ======================================================*/
int c2r_fd_setarray(fd_set *ary, int *fdin, PRXSTRING rx, int numin);

/*======================================================
 = setRexxVar()
 = Sets the value of a variable in the Rexx variable pool.
 = If successful, returns 1.  Otherwise returns 0.
 ======================================================*/
int setRexxVar(PRXSTRING rx, char *buf, int len);

/*======================================================
 = r2c_sockopt_option()
 = Converts a Rexx string that contains an option for
 = getsockopt() or setsockopt() to its numeric value.
 ======================================================*/
int r2c_sockopt_option(int *value, PRXSTRING rx);

/*======================================================
 = r2c_recv_flags()
 = Converts a Rexx string that contains flags for
 = recv() or recvfrom() to its numeric value.
 = NOTE: Not MT-Safe since it calls strtok()
 ======================================================*/
int r2c_recv_flags(int *value, PRXSTRING rx);

/*======================================================
 = r2c_2_ints()
 = Converts a Rexx variable that is two integers seperated by a space
 = into two integers.  If successfull
 = returns 1.  Otherwise returns 0.
 ======================================================*/
int r2c_2_ints(int *newValue1, int *newVal2, PRXSTRING rx);

#ifndef MAXSTEMNAMELEN
#  define MAXSTEMNAMELEN 250
#  define MAXVARNAMELEN 250
#endif

#endif
