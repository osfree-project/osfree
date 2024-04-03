/*!
   $Id: all_shared.h,v 1.2 2004/08/21 14:49:21 prokushev Exp $

   @file all_shared.h
   @brief utility functions header, shared along all os source

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
   @author JMA <jma@jma.se>
*/

#ifndef _ALL_SHARED_H_
#define _ALL_SHARED_H_


#undef __cdecl  /* 06/07/2002: fix for Toolkit libc headers */

#define  INCL_DOSERRORS
#include <os2.h>

#ifndef __386__ 
#define APIRET USHORT

#define MUST_HAVE_READONLY  ((FILE_READONLY  << 8) | FILE_READONLY)
#define MUST_HAVE_HIDDEN    ((FILE_HIDDEN    << 8) | FILE_HIDDEN)
#define MUST_HAVE_SYSTEM    ((FILE_SYSTEM    << 8) | FILE_SYSTEM)
#define MUST_HAVE_DIRECTORY ((FILE_DIRECTORY << 8) | FILE_DIRECTORY)
#define MUST_HAVE_ARCHIVED  ((FILE_ARCHIVED  << 8) | FILE_ARCHIVED)

#define FIL_STANDARD           1
#define FIL_QUERYFULLNAME      5

#define DSPI_WRTTHRU 0x10

#define FSAIL_QUERYNAME 1

#define MSG_BAD_PARM2                   1002
#define MSG_BAD_SYNTAX                  1003

#endif

#include "all_dlist.h"
#include "all_messageids.h"     // message id's
#include "all_pathmanipulation.h"
#include "all_performrecursiveaction.h"
#include "all_querycurrentdisk.h"
#include "all_queryfsname.h"
#include "all_messages.h"

#endif /* _ALL_SHARED_H_ */
