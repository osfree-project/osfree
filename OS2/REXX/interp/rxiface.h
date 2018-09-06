/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "wrappers.h"

/* <<<<<<<<<<< Things you might want to configure >>>>>>>>>>>>>> */
/*
 * Although you'd probably better off compiling rxiface.c with these
 * symbols set at the commandline than actually changing this file
 */

/*
 * The cpp symbol RXPROG contains the name of the rexx interpreter to
 * start. If not defined elsewhere (like on the command line), a default
 * value is used. It is invoked using exec*p*, so you don't need to
 * specify a full path unless 'rexx' is located somewhere outside PATH.
 */
#ifndef RXPROG
# define RXPROG "rexx"
#endif

/*
 * When the application starts up the interpreter, a version number is
 * sent from the interpreter to the application, as the first thing.
 * This piece of information serves two purposes: If the interpreter
 * was not properly started, that will be sensed and an apropriate
 * errormessage can be returned to the calling routine in the application.
 * Secondly, if the interpreter and the application have different
 * versions of the interfacing software, that will be detected.
 *
 * MAKE SURE THAT YOU CHANGE THE VERSION NUMBER WHENEVER YOU CHANGE
 * THE DEFINITION OF THE INTERFACE! At worst, pick a number at random,
 * just choose a version number that isn't likely to have been chosen
 * by somebody else.
 *
 * NOTE: I've taken the liberty to 'reserve' 0xdeadbeef as the version
 *       number for the versions I write and distribute. If you change
 *       anything, make sure that you set RXVERSION_MAJ to something
 *       else.  -anders <anders@pvv.unit.no>
 *       The version number for my changes is 0xbadab0de :-)
 *       Mark - <M.Hessling@qut.edu.au>
 */
#ifndef RXVERSION_MAJ
# define RXVERSION_MAJ  0xbadab0de
# define RXVERSION_MIN  0x00002000
#endif

/*
 * These are the commands sent through the communication channel. Their
 * format is fixed.

   RX_RETURN     int=code, int=rval
   RX_SCOMRET    int=code, int=flag, str=retvalue
   RX_SUBCOM

 */
#define RX_RETURN 1
#define RX_SUBCOM 2
#define RX_GETVAR 3
#define RX_SETVAR 4
#define RX_EXECUTE 5
#define RX_VALUE 6
#define RX_ENVIR 7
#define RX_RETVOID 8
#define RX_SYNCREQ 9
#define RX_SYNCACK 10
#define RX_SETEXIT 11
#define RX_CLREXIT 12
#define RX_DO_HOOK 13
#define RX_TYPE_EXTERNAL 14
#define RX_TYPE_INSTORE 15
#define RX_TYPE_MACRO 16
#define RX_TYPE_SOURCE 17
#define RX_INSTORE 18
#define RX_DROP_INSTORE 19
#define RX_GETSVAR 20
#define RX_SETSVAR 21
#define RX_CODE_SOURCE 22
#define RX_CODE_VERSION 23
#define RX_CODE_QUEUE 24
#define RX_CODE_PARAM 25
#define RX_TYPE_COMMAND 26
#define RX_TYPE_FUNCTION 27
#define RX_TYPE_SUBROUTINE 28
#define RX_VERSION 29
#define RX_STRINGS 30
#define RX_NEXTVAR 31
#define RX_ADDFUNC 32
#define RX_DELFUNC 33
#define RX_EXECFUNC 34
#define RX_CODE_PARAMS 35

#define RX_CODE_OK 0
#define RX_CODE_NOVALUE 1
#define RX_CODE_INVNAME 2
#define RX_CODE_EXISTS  3
#define RX_CODE_NOSUCH  4
#define RX_CODE_NOMEM   5

#define RX_HOOK_GO_ON    0
#define RX_HOOK_NOPE     1
#define RX_HOOK_ERROR    2

#define RX_NO_STRING    (-1)

#define RX_EXIT_STDOUT  0
#define RX_EXIT_STDERR  1
#define RX_EXIT_TRCIN   2
#define RX_EXIT_PULL    3
#define RX_EXIT_INIT    4
#define RX_EXIT_TERMIN  5
#define RX_EXIT_SUBCOM  6
#define RX_EXIT_FUNC    7
#define RX_EXIT_GETENV  8
#define RX_EXIT_SETENV  9
#define RX_EXIT_GETCWD 10
#define RX_EXIT_SETCWD 11

#define RX_LASTHOOK    32

#define RXFLAG_OK      0x0000

#define RXFLAG_ERROR   0x0001
#define RXFLAG_FAILURE 0x0002

#define RXFLAG_NOVALUE 0x0001

#define RXFLAG_NOTREG  30

int IfcStartUp( tsd_t *TSD, char *name, int *Major, int *Minor ) ;
int IfcExecScript( tsd_t * volatile TSD, int NameLen, const char *Name,
                   int ArgCount, const int *ParLengths, const char **ParStrings,
                   int CallType, int ExitFlags, int EnvLen, const char *EnvName,
                   int SourceCode, int restricted,
                   const char *SourceString, unsigned long SourceStringLen,
                   const void *TinnedTree, unsigned long TinnedTreeLen,
                   int *RetLen, char **RetString, void **instore_buf,
                   unsigned long *instore_length ) ;
int IfcExecCallBack( tsd_t * volatile TSD, int NameLen, const char *Name,
                     int ArgCount, const int *ParLengths, const char **ParStrings,
                     int *RetLen, char **RetString );

int IfcVarPool( tsd_t *TSD, int Code, int *Lengths, char *Strings[],
                int *allocated );

int IfcSubCmd( tsd_t *TSD, int EnvLen, const char *EnvStr, int CmdLen,
               const char *CmdStr, int *RetLen, char **RetStr ) ;
int IfcDoExit( tsd_t *TSD, int Code,
               int OutputLength1, char *OutputString1,
               int OutputLength2, char *OutputString2,
               int *InputLength, char **InputString ) ;
int IfcExecFunc( tsd_t *TSD, PFN Func, char *Name, int Params,
                 int *Lengths, char **Strings,
                 int queue_name_len, char *queue_name,
                 int *RetLength, char **RetString,
                 int *RC, char called, void *gci_info ) ;
int IfcRegDllFunc( const tsd_t *TSD, const char* rxname, const char* module, const char* objnam ) ;

int IfcHaveFunctionExit(const tsd_t *TSD);

unsigned long IfcFreeMemory( void *);
void *IfcAllocateMemory( unsigned long );

int IfcCreateQueue( tsd_t *TSD, const char *qname, const int qlen, char *data, unsigned long *dupflag, unsigned long buflen );
int IfcDeleteQueue( tsd_t *TSD, const char *qname, const int qlen );
int IfcQueryQueue( tsd_t *TSD, const char *qname, const int qlen, unsigned long *count );
int IfcAddQueue( tsd_t *TSD, const char *qname, const int qlen, const char *data, const int datalen, unsigned long addlifo );
int IfcPullQueue( tsd_t *TSD, const char *qname, const int qlen, char **data, unsigned long *datalen, unsigned long waitforline );
