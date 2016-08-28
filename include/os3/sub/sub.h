/*****************************************************************************
    sub.h
    (C) 2004-2010 osFree project

    WARNING! Automaticaly generated file! Don't edit it manually!
*****************************************************************************/

#ifndef __SUB_H__
#define __SUB_H__

#ifdef __cplusplus
   extern "C" {
#endif

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef EXPENTRY
#define EXPENTRY  _System
#endif

#ifndef APIENTRY
#define APIENTRY  _System
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

#define INCL_KBD
#ifdef INCL_KBD
  #include <kbd.h>
#endif

#ifdef INCL_MOU
  #include <mou.h>
#endif

#define INCL_VIO
#ifdef INCL_VIO
  #include <vio.h>
#endif


#ifdef __cplusplus
   }
#endif

#endif /* __SUB_H__ */
