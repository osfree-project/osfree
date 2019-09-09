/*==============================================================================
//
//  Willows
//
//  @(#)Willows.h	1.1 17:30:51 7/11/96 /users/sccs/src/win/utilities/s.Willows.h
//
//  Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//============================================================================*/



#ifndef __WILLOWS_H__
#define __WILLOWS_H__



#if defined ( _WINDOWS )
#define EXPORT                          __declspec( dllexport )
#else
#define EXPORT
#define OutputDebugString               printf
#endif



#endif /* __WILLOWS_H__ */


