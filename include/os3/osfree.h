/****************************** Module Header ******************************\
*                                                                          *
* Module Name: OSFREE.H                                                    *
*                                                                          *
* This is the top level include file that includes all the files necessary *
* for writing an osFree application.                                       *
*                                                                          *
\***************************************************************************/

#ifndef __OSFREE_H__

#define __OSFREE_H__
#define OSFREE_INCLUDED

/* OS/2 definitions */
#ifdef _M_I86
#include "os21x/os2.h"
#else
#include <os2.h>
#endif


/* Missed in OpenWatcom definitions */
#include "mis_basemid.h"
#include "mis_bsedev.h"
#include "mis_bsedos.h"
#include "mis_os2def.h"

#endif /* __OSFREE__ */
