/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005 Rexx Language Association. All rights reserved.         */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/* REXX Macros                                                  oryx.h        */
/*                                                                            */
/* Header file for REXX methods written in C.                                 */
/*                                                                            */
/******************************************************************************/
#ifndef ORYX_H_INCLUDED
#define ORYX_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//#include SYSREXXSAA                    /* include REXX APIs also            */
//#include <rexxsaa.h>

/******************************************************************************/
/* Types (for general use)                                                    */
/******************************************************************************/
typedef char *CSTRING;        /* pointer to zero-terminated string */

typedef void *POINTER;

#ifdef __cplusplus
 class RexxObject;                     /* Object Rexx root object           */
 class RexxInternalObject;             /* Object Rexx root object           */
                                       /* reference to a REXX object        */
 typedef RexxObject *REXXOBJECT;
 #define NULLOBJECT (RexxObject *)NULL
#else
 typedef void *REXXOBJECT;             /* reference to a REXX object        */
 #define NULLOBJECT (REXXOBJECT)0      /* null object                       */
#endif

typedef REXXOBJECT STRING;             /* REXX string object                */

/******************************************************************************/
/* Constant values (for general use)                                          */
/* Also in RexxCore.h */
/******************************************************************************/
#ifndef NO_INT
# define NO_INT                0x80000000
#endif
#ifndef NO_LONG
# define NO_LONG               0x80000000
#endif
#ifndef NO_CSTRING
# define NO_CSTRING            NULL
#endif
#define NULLOBJ               0L

/******************************************************************************/
/* New-style macros and functions                                             */
/******************************************************************************/
#define RexxArray(s)           REXX_ARRAY_NEW(s)
#define RexxArray1(v1)         REXX_ARRAY_NEW1(v1)
#define RexxArray2(v1,v2)      REXX_ARRAY_NEW2(v1,v2)
#define RexxBuffer(l)          REXX_BUFFER_NEW(l)
#define RexxInteger(v)         REXX_INTEGER_NEW(v)
#define RexxPointer(v)         REXX_INTEGER_NEW((LONG)v)
#define RexxReceiver()         REXX_RECEIVER()
#define RexxSend(r,n,aa)       REXX_SEND(r,n,aa)
#define RexxSend0(r,n)         REXX_SEND(r,n,RexxArray(0))
#define RexxSend1(r,n,a1)      REXX_SEND(r,n,RexxArray1(a1))
#define RexxSend2(r,n,a1,a2)   REXX_SEND(r,n,RexxArray2(a1,a2))
#define RexxString(s)          REXX_STRING_NEW(s, strlen(s))
#define RexxStringUpper(s)     REXX_STRING_NEW_UPPER(s)
#define RexxStringD(d)         REXX_STRING_NEWD(&d)
#define RexxStringL(s,l)       REXX_STRING_NEW(s,l)
#define RexxSuper(n,aa)        REXX_SUPER(n,aa)
#define RexxSuper0(n)          REXX_SUPER(n,RexxArray(0))
#define RexxSuper1(n,a1)       REXX_SUPER(n,RexxArray1(a1))
#define RexxSuper2(n,a1,a2)    REXX_SUPER(n,RexxArray2(a1,a2))
#define RexxTable()            REXX_TABLE_NEW)
#define RexxVarSet(n,v)        REXX_SETVAR(n,v)
#define RexxVarValue(n)        REXX_GETVAR(n)
#define RexxNil                REXX_NIL()
#define RexxTrue               REXX_TRUE()
#define RexxFalse              REXX_FALSE()
#define RexxEnvironment        REXX_ENVIRONMENT()
#define RexxLocal              REXX_LOCAL()

#ifndef __cplusplus
#define RexxMethod0(r,n) r  n##_m (void); \
static char n##_t[] = {REXXD_##r,0};    \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m());} return n##_t;} \
r  n##_m (void)

#define RexxMethod1(r,n,t1,p1) r  n##_m (t1 p1); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1))));} return n##_t;} \
r  n##_m (t1 p1)

#define RexxMethod2(r,n,t1,p1,t2,p2) r  n##_m (t1 p1, t2 p2); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2)

#define RexxMethod3(r,n,t1,p1,t2,p2,t3,p3) r  n##_m (t1 p1, t2 p2, t3 p3); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3)

#define RexxMethod4(r,n,t1,p1,t2,p2,t3,p3,t4,p4) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4)

#define RexxMethod5(r,n,t1,p1,t2,p2,t3,p3,t4,p4,t5,p5) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,REXXD_##t5,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4)),*((t5 *)*(a+5))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5)

#define RexxMethod6(r,n,t1,p1,t2,p2,t3,p3,t4,p4,t5,p5,t6,p6) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,REXXD_##t5,REXXD_##t6,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4)),*((t5 *)*(a+5)),*((t6 *)*(a+6))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6)

#else

#define RexxMethod0(r,n) r  n##_m (void); \
static char n##_t[] = {REXXD_##r,0};    \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m());} return n##_t;} \
r  n##_m (void)

#define RexxMethod1(r,n,t1,p1) r  n##_m (t1 p1); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1))));} return n##_t;} \
r  n##_m (t1 p1)

#define RexxMethod2(r,n,t1,p1,t2,p2) r  n##_m (t1 p1, t2 p2); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2)

#define RexxMethod3(r,n,t1,p1,t2,p2,t3,p3) r  n##_m (t1 p1, t2 p2, t3 p3); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3)

#define RexxMethod4(r,n,t1,p1,t2,p2,t3,p3,t4,p4) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4)

#define RexxMethod5(r,n,t1,p1,t2,p2,t3,p3,t4,p4,t5,p5) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,REXXD_##t5,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4)),*((t5 *)*(a+5))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5)

#define RexxMethod6(r,n,t1,p1,t2,p2,t3,p3,t4,p4,t5,p5,t6,p6) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,REXXD_##t5,REXXD_##t6,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4)),*((t5 *)*(a+5)),*((t6 *)*(a+6))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6)

#endif


#define _cstring(r)              REXX_STRING(r)
#define _double(r)               REXX_DOUBLE(r)
#define _integer(r)              REXX_INTEGER(r)
#define _unsigned_integer(r)     REXX_UNSIGNED_INTEGER(r)
#define _isdouble(r)             REXX_ISDOUBLE(r)
#define _isdirectory(r)          REXX_ISDIRECTORY(r)
#define _isinteger(r)            REXX_ISINTEGER(r)
#define _ispinteger(r)           REXX_INTEGER(r)
#define _isstring(r)             REXX_ISASTRING(r)
#define _string(r)               REXX_OBJECT_STRING(r)

#define area_get(r,s,b,l)        REXX_BUFFER_GETDATA(r,s,(PCHAR)b,l)
#define area_put(r,s,b,l)        REXX_BUFFER_COPYDATA(r,s,(PCHAR)b,l)

#define array_at(r,i)            REXX_ARRAY_AT(r,i)
#define array_hasindex(r,i)      REXX_ARRAY_HASINDEX(r,i)
#define array_put(r,v,i)         REXX_ARRAY_PUT(r,v,i)
#define array_size(r)            REXX_ARRAY_SIZE(r)

#define buffer_address(r)        REXX_BUFFER_ADDRESS(r)
#define buffer_length(r)         REXX_BUFFER_LENGTH(r)
#define buffer_extend(r, l)      REXX_BUFFER_EXTEND(r, l)
#define buffer_get(r,s,b,l)      REXX_BUFFER_GETDATA(r,s,(PCHAR)b,l)
#define buffer_put(r,s,b,l)      REXX_BUFFER_COPYDATA(r,s,(PCHAR)b,l)

#define integer_value(r)         REXX_INTEGER_VALUE(r)
#define pointer_value(r)         ((PVOID)REXX_INTEGER_VALUE(r))

#define string_get(r,s,b,l)      REXX_STRING_GET(r,s,b,l)
#define string_length(r)         REXX_STRING_LENGTH(r)
#define string_put(r,s,b,l)      REXX_STRING_PUT(r,s,b,l)
#define string_rput(r,s,b,l)     REXX_STRING_RPUT(r,s,b,l)
#define string_set(r,s,c,n)      REXX_STRING_SET(r,s,c,n)
#define string_data(r)           REXX_STRING_DATA(r)

#define table_add(r,v,i)         REXX_TABLE_ADD(r,v,i)
#define table_at(r,i)            REXX_TABLE_GET(r,i)
#define table_remove(r,i)        REXX_TABLE_REMOVE(r,i)

#define send_condition(c,d,a)    REXX_CONDITION(c,d,a)
#define send_exception(m)        REXX_EXCEPT(m,NULLOBJECT)
#define send_exception1(m,a1)    REXX_EXCEPT(m,a1)
#define RexxRaiseCondition(c, d, a, r) REXX_RAISE(c, d, a, r)


/******************************************************************************/
/* Old-style macros and internal definitions follow..... don't use!!          */
/******************************************************************************/

/******************************************************************************/
/* Types (used in macro expansions and function prototypes)                   */
/******************************************************************************/
#define ONULL   REXXOBJECT
#define OSELF   REXXOBJECT
#define CSELF   PVOID
#define BUFFER  PVOID
#define SOMSELF REXXOBJECT
#define ARGLIST REXXOBJECT
#define SCOPE   REXXOBJECT
#define MSGNAME REXXOBJECT

/******************************************************************************/
/* Primitive Message Names (for general use except where indicated)           */
/******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*
REXXOBJECT REXXENTRY REXX_ARRAY_AT(REXXOBJECT, size_t);
size_t     REXXENTRY REXX_ARRAY_HASINDEX(REXXOBJECT, size_t);
void       REXXENTRY REXX_ARRAY_PUT(REXXOBJECT, REXXOBJECT, size_t);
size_t     REXXENTRY REXX_ARRAY_SIZE(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_ARRAY_NEW(size_t);
REXXOBJECT REXXENTRY REXX_ARRAY_NEW1(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_ARRAY_NEW2(REXXOBJECT, REXXOBJECT);

PCHAR      REXXENTRY REXX_BUFFER_ADDRESS(REXXOBJECT);
size_t     REXXENTRY REXX_BUFFER_LENGTH(REXXOBJECT);
long       REXXENTRY REXX_BUFFER_GETDATA(REXXOBJECT, long, PVOID, ULONG);
long       REXXENTRY REXX_BUFFER_COPYDATA(REXXOBJECT, long, PVOID, ULONG);
REXXOBJECT REXXENTRY REXX_BUFFER_NEW(size_t);
REXXOBJECT REXXENTRY REXX_BUFFER_EXTEND(REXXOBJECT, size_t);

long       REXXENTRY REXX_INTEGER_VALUE(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_INTEGER_NEW(long);

double     REXXENTRY REXX_DOUBLE(REXXOBJECT);
BOOL       REXXENTRY REXX_ISDOUBLE(REXXOBJECT);
BOOL       REXXENTRY REXX_ISASTRING(REXXOBJECT);
BOOL       REXXENTRY REXX_ISDIRECTORY(REXXOBJECT);
void       REXXENTRY REXX_EXCEPT(int, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_CONDITION(REXXOBJECT, REXXOBJECT, REXXOBJECT);
void       REXXENTRY REXX_RAISE(PCHAR, REXXOBJECT, REXXOBJECT, REXXOBJECT);
long       REXXENTRY REXX_INTEGER(REXXOBJECT);
ULONG      REXXENTRY REXX_UNSIGNED_INTEGER(REXXOBJECT);
BOOL       REXXENTRY REXX_ISINTEGER(REXXOBJECT);
PCHAR      REXXENTRY REXX_STRING(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_MSGNAME(void);
REXXOBJECT REXXENTRY REXX_RECEIVER(void);
BOOL       REXXENTRY REXX_PTYPE(REXXOBJECT, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_SEND(REXXOBJECT, PCHAR, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_DISPATCH(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_SUPER(PCHAR, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_SETVAR(PCHAR, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_SETVAR2(PCHAR, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_SETFUNC(PCHAR, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_GETFUNCTIONNAMES(char ***, int*);
REXXOBJECT REXXENTRY REXX_GETVAR(PCHAR);
ULONG      REXXENTRY REXX_VARIABLEPOOL(PVOID);
ULONG      REXXENTRY REXX_VARIABLEPOOL2(PVOID);      // for everyone
ULONG      REXXENTRY REXX_STEMSORT(PCHAR, INT, INT, size_t, size_t, size_t, size_t);
void       REXXENTRY REXX_ENABLE_VARIABLEPOOL(void);
void       REXXENTRY REXX_DISABLE_VARIABLEPOOL(void);
void       REXXENTRY REXX_PUSH_ENVIRONMENT(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_POP_ENVIRONMENT(void);
REXXOBJECT REXXENTRY REXX_NIL(void);
REXXOBJECT REXXENTRY REXX_TRUE(void);
REXXOBJECT REXXENTRY REXX_FALSE(void);
REXXOBJECT REXXENTRY REXX_ENVIRONMENT(void);
REXXOBJECT REXXENTRY REXX_LOCAL(void);
void       REXXENTRY REXX_GUARD_ON(void);
void       REXXENTRY REXX_GUARD_OFF(void);


REXXOBJECT REXXENTRY REXX_OBJECT_METHOD(REXXOBJECT, REXXOBJECT);

REXXOBJECT REXXENTRY REXX_OBJECT_NEW(REXXOBJECT);

size_t     REXXENTRY REXX_STRING_GET(REXXOBJECT, size_t, PCHAR, size_t);
void       REXXENTRY REXX_STRING_PUT(REXXOBJECT, size_t, PCHAR, size_t);
void       REXXENTRY REXX_STRING_SET(REXXOBJECT, size_t, CHAR, size_t);
size_t     REXXENTRY REXX_STRING_LENGTH(REXXOBJECT);
PCHAR      REXXENTRY REXX_STRING_DATA(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_STRING_NEW(PCHAR, size_t);
REXXOBJECT REXXENTRY REXX_STRING_NEW_UPPER(PCHAR);
REXXOBJECT REXXENTRY REXX_STRING_NEWD(double *);

REXXOBJECT REXXENTRY REXX_TABLE_ADD(REXXOBJECT, REXXOBJECT, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_TABLE_GET(REXXOBJECT, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_TABLE_REMOVE(REXXOBJECT, REXXOBJECT);

BOOL       REXXENTRY RexxQuery (void);
int        REXXENTRY RexxTerminate (void);
BOOL       REXXENTRY RexxInitialize (void);
LONG       VLAREXXENTRY RexxSendMessage (REXXOBJECT receiver, PCHAR msgname, REXXOBJECT start_class, PCHAR interfacedefn, PVOID result, ...);
REXXOBJECT REXXENTRY RexxDispatch (REXXOBJECT argList);
LONG       VLAREXXENTRY RexxCallProgram (PCHAR filename, PCHAR interfacedefn, PVOID result, ...);
LONG       VLAREXXENTRY RexxCallString  (PCHAR filename, PCHAR interfacedefn, PVOID result, ...);

ULONG      REXXENTRY REXX_EXECUTIONINFO(PULONG, PSZ, BOOL);

*/

#ifdef __cplusplus
}
#endif

/******************************************************************************/
/* Interface Datatypes (used in macro expansions)                             */
/******************************************************************************/
#define REXXD_void         1
#define REXXD_OBJECT       2
#define REXXD_REXXOBJECT   REXXD_OBJECT
#define REXXD_int          3
#define REXXD_long         4
#define REXXD_double       5
#define REXXD_CSTRING      6
#define REXXD_OSELF        7
#define REXXD_SOMSELF      8
#define REXXD_somRef       9
#define REXXD_somTok      10
#define REXXD_ARGLIST     11
#define REXXD_MSGNAME     12
#define REXXD_SCOPE       13
#define REXXD_POINTER     14
#define REXXD_CSELF       15
#define REXXD_STRING      16
#define REXXD_BUFFER      17

/******************************************************************************/
/* Internal Macros (used in macro expansions)                                 */
/******************************************************************************/
#define REXX_ret_void(v)           v
#define REXX_ret_REXXOBJECT(v)    *((REXXOBJECT *)*a) = v
#define REXX_ret_int(v)           *((int *)*a) = v
#define REXX_ret_long(v)          *((long *)*a) = v
#define REXX_ret_double(v)        *((double *)*a) = v
#define REXX_ret_CSTRING(v)       *((CSTRING *)*a) = v
#define REXX_ret_somRef(v)        *((SOMAny **)*a) = v
#define REXX_ret_somTok(v)        *((somTok *)*a) = v
#define REXX_ret_POINTER(v)       *((void  **)*a) = v
#ifdef __cplusplus
}
#endif

#endif

