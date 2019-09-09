/*==============================================================================
//
//  Utilities Library
//
// @(#)WUtilities.h	1.4 12:36:02 11/11/96 /users/sccs/src/win/utilities/s.WUtilities.h
//
//  Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//============================================================================*/


extern int * CatchBuffer;

#ifndef __WUTILITIES_H__
#define __WUTILITIES_H__


#include "windows.h"
#include <stdio.h>
#include <string.h>
#include "Willows.h"
#include "WSharedLib.h"
#include "WErr.h"


#ifndef WUTILITIES
#if defined ( __cplusplus )
extern CWSharedLib                      *pWUtilities;
#endif
extern char                             WUDebugString[];
#endif


#if defined ( __cplusplus )
inline DWORD round ( float f ) { return ( f - ( DWORD )f ? ( DWORD )f + 1 : ( DWORD )f ); }
#ifndef WEXCEPTIONS
#define THROW(E)						pWUtilities->THROW(E)
#define CATCH()							pWUtilities->CATCH()
#endif

#else
#define round(f)                        ( f - ( DWORD )f ? ( DWORD )f + 1 : ( DWORD )f )
#endif


#endif /* __WUTILITIES_H__ */

