/*
 *  stdarg.h    Variable argument macros
 *              definitions for use with variable argument lists
 *
 *                          Open Watcom Project
 *
 *    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
 *
 *  ========================================================================
 *
 *    This file contains Original Code and/or Modifications of Original
 *    Code as defined in and that are subject to the Sybase Open Watcom
 *    Public License version 1.0 (the 'License'). You may not use this file
 *    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
 *    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
 *    provided with the Original Code and Modifications, and is also
 *    available at www.sybase.com/developer/opensource.
 *
 *    The Original Code and all software distributed under the License are
 *    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 *    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
 *    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
 *    NON-INFRINGEMENT. Please see the License for the specific language
 *    governing rights and limitations under the License.
 *
 *  ========================================================================
 */
#ifndef _STDARG_H_INCLUDED
#define _STDARG_H_INCLUDED
#if !defined(_ENABLE_AUTODEPEND)
  #pragma read_only_file;
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _COMDEF_H_INCLUDED
 #include "_comdef.h"
#endif

#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
#if defined(__PPC__)
 #if defined(__NT__)
    typedef char * __va_list;
 #else
    typedef struct {
      char  __gpr;
      char  __fpr;
      char  __reserved[2];
      char *__input_arg_area;
      char *__reg_save_area;
    } __va_list;
  #endif
#elif defined(__AXP__)
  typedef struct {
    char *__base;
    int   __offset;
  } __va_list;
#elif defined(__HUGE__) || defined(__SW_ZU)
  typedef char _WCFAR *__va_list[1];
#else
  typedef char *__va_list[1];
#endif
#endif
typedef __va_list va_list;

#if defined(__AXP__)
 #define _AXP_WORD_SIZE     8
 #define _AXP_FLOAT_SIZE    8
 #define _AXP_FLOAT_REGS    6
 extern void *__builtin_va_start(va_list*, ...);
 #pragma intrinsic(__builtin_va_start);
 #define va_start(ap,pn) (__builtin_va_start(&ap,pn,1))
 #define va_arg(ap,type) ((ap).__offset+=\
     ((sizeof(type)+_AXP_WORD_SIZE-1)&~(_AXP_WORD_SIZE-1)),\
     (*(type *)((ap).__base+(ap).__offset-\
       ((__builtin_isfloat(type)&&(ap).__offset <=\
         (_AXP_FLOAT_REGS*_AXP_FLOAT_SIZE)) ?\
           (_AXP_FLOAT_REGS*_AXP_FLOAT_SIZE)+_AXP_FLOAT_SIZE :\
             ((sizeof(type)+_AXP_WORD_SIZE-1)&~(_AXP_WORD_SIZE-1))))))
 #define va_end(ap)      ((void)0)
#elif defined(__PPC__)
 #if defined(__NT__)
  #define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
  #define       _ALIGNIT(ap,t) \
        ((((int)(ap))+(sizeof(t)<8?3:7)) & (sizeof(t)<8?~3:~7))
  #define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
  #define va_arg(ap,t)    ( *(t *)((ap = (char *) (_ALIGNIT(ap, t) + _INTSIZEOF(t))) - _INTSIZEOF(t)) )
  #define va_end(ap)      ( ap = (va_list)0 )
 #else
  extern void *__builtin_va_start( va_list * );
  #pragma intrinsic(__builtin_va_start);
  #define va_start(ap,pn) (__builtin_va_start(&ap,pn))
  #define va_arg(ap,type) (__builtin_va_arg(&ap,type))
 #endif
#else
 #if defined(__HUGE__) || defined(__SW_ZU)
  #define va_start(ap,pn) ((ap)[0]=(char _WCFAR*)&pn+\
      ((sizeof(pn)+sizeof(int)-1)&~(sizeof(int)-1)),(void)0)
  #define va_arg(ap,type)     ((ap)[0]+=\
      ((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)),\
      (*(type _WCFAR*)((ap)[0]-((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)))))
  #define va_end(ap)        ((ap)[0]=0,(void)0)
 #else
  #define va_start(ap,pn) ((ap)[0]=(char *)&pn+\
      ((sizeof(pn)+sizeof(int)-1)&~(sizeof(int)-1)),(void)0)
  #define va_arg(ap,type)     ((ap)[0]+=\
      ((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)),\
      (*(type *)((ap)[0]-((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)))))
  #define va_end(ap)        ((ap)[0]=0,(void)0)
 #endif
#endif

#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif
