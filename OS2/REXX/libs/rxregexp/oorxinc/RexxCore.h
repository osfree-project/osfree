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
/* REXX Kernel                                                  RexxCore.h      */
/*                                                                            */
/* Global Declarations                                                        */
/******************************************************************************/

/******************************************************************************/
/* Globally required include files                                            */
/******************************************************************************/
#ifndef RexxCore_INCLUDED
#define RexxCore_INCLUDED

/* ANSI C definitions */
#include <setjmp.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/* REXX Library definitions */
#include "RexxLibrary.h"

/******************************************************************************/
/* Literal definitions                                                        */
/******************************************************************************/
#include "RexxConstants.hpp"

/******************************************************************************/
/* Kernel Internal Limits                                                     */
/******************************************************************************/

const int MAX_ERROR_NUMBER = 99999;        /* maximum error code number         */
const int MAX_SYMBOL_LENGTH = 250;         /* length of a symbol name           */
const int MAX_STRING_LENGTH = 250;         /* length of a literal string        */
const int MAX_HEX_LENGTH    = 250;         /* packed length of hex literal      */
const int MAX_BIN_LENGTH    = 250;         /* packed length of binary literal   */

/******************************************************************************/
/* Numeric setting constants                                                  */
/******************************************************************************/
                                       /* constants used for setting form   */
const int FORM_SCIENTIFIC    = FALSE;
const int FORM_ENGINEERING   = TRUE;

const size_t DEFAULT_DIGITS  = ((size_t)9); /* default numeric digits setting    */
const size_t DEFAULT_FUZZ    = ((size_t)0); /* default numeric fuzz setting      */
                                       /* default numeric form setting      */
const int DEFAULT_FORM       = FORM_SCIENTIFIC;

/******************************************************************************/
/* Defines for argument error reporting                                       */
/******************************************************************************/

const int ARG_ONE    = 1;
const int ARG_TWO    = 2;
const int ARG_THREE  = 3;
const int ARG_FOUR   = 4;
const int ARG_FIVE   = 5;
const int ARG_SIX    = 6;
const int ARG_SEVEN  = 7;
const int ARG_EIGHT  = 8;
const int ARG_NINE   = 9;
const int ARG_TEN    = 10;

/******************************************************************************/
/* Constants used for trace prefixes                                          */
/******************************************************************************/

enum TracePrefixes {
    TRACE_PREFIX_CLAUSE   ,
    TRACE_PREFIX_ERROR    ,
    TRACE_PREFIX_RESULT   ,
    TRACE_PREFIX_DUMMY    ,
    TRACE_PREFIX_VARIABLE ,
    TRACE_PREFIX_LITERAL  ,
    TRACE_PREFIX_FUNCTION ,
    TRACE_PREFIX_PREFIX   ,
    TRACE_PREFIX_OPERATOR ,
    TRACE_PREFIX_COMPOUND ,
    TRACE_PREFIX_MESSAGE
};

#define MAX_TRACEBACK_LIST 80      /* 40 messages are displayed */
#define MAX_TRACEBACK_INDENT 20    /* 10 messages are indented */

/******************************************************************************/
/* Constants used for setting trace                                           */
/******************************************************************************/

const char TRACE_ALL           = 'A';
const char TRACE_COMMANDS      = 'C';
const char TRACE_LABELS        = 'L';
const char TRACE_NORMAL        = 'N';
const char TRACE_FAILURES      = 'F';
const char TRACE_ERRORS        = 'E';
const char TRACE_RESULTS       = 'R';
const char TRACE_INTERMEDIATES = 'I';
const char TRACE_OFF           = 'O';
const char TRACE_IGNORE        = '0';

/******************************************************************************/
/* Constants used for setting trace interactive debug                         */
/******************************************************************************/
const int DEBUG_IGNORE      =  0x00;
const int DEBUG_ON          =  0x01;
const int DEBUG_OFF         =  0x02;
const int DEBUG_TOGGLE      =  0x04;

/******************************************************************************/
/* Random number generation constants                                         */
/******************************************************************************/

const long RANDOM_FACTOR = 1664525L;   /* random multiplication factor      */
                                       /* randomize a seed number           */
inline long RANDOMIZE(long seed) { return (seed * RANDOM_FACTOR + 1); }

/* Object Reference Assignment */
#ifndef CHECKOREFS
#define OrefSet(o,r,v) (OldSpace(o) ? memoryObject.setOref((RexxObject **)&(r),(RexxObject *)v) : (RexxObject *)(r=v))
#else
#define OrefSet(o,r,v) memoryObject.checkSetOref((RexxObject *)o, (RexxObject **)&(r), (RexxObject *)v, __FILE__, __LINE__)
#endif

/******************************************************************************/
/* Internal Message Send Functions                                            */
/******************************************************************************/
#define send_message(r,m,a) ((r)->sendMessage((RexxString *)(m), (RexxArray *)(a)))
#define send_message0(r,m) ((r)->sendMessage((RexxString *)(m)))
#define send_message1(r,m,a1) ((r)->sendMessage((RexxString *)(m), (RexxObject *)(a1)))
#define send_message2(r,m,a1,a2) ((r)->sendMessage((RexxString *)(m), (RexxObject *)(a1), (RexxObject *)(a2)))
#define send_message3(r,m,a1,a2,a3) ((r)->sendMessage((RexxString *)(m), (RexxObject *)(a1), (RexxObject *)(a2), (RexxObject *)(a3)))
#define send_message4(r,m,a1,a2,a3,a4) ((r)->sendMessage((RexxString *)(m), (RexxObject *)(a1), (RexxObject *)(a2), (RexxObject *)(a3), (RexxObject *)(a4)))
#define send_message5(r,m,a1,a2,a3,a4,a5) ((r)->sendMessage((RexxString *)(m), (RexxObject *)(a1), (RexxObject *)(a2), (RexxObject *)(a3), (RexxObject *)(a4), (RexxObject *)(a5)))

/******************************************************************************/
/* Object creation macros                                                     */
/******************************************************************************/

#define kernel_public(name, object, dir)  ((RexxDirectory *)dir)->setEntry(kernel_name(name), (RexxObject *)object)
#define create_udclass(c)                 The##c##Class = new (sizeof(RexxClass), The##c##ClassBehaviour, The##c##Behaviour) RexxClass
#define create_udsubClass(c,t)            The##c##Class = new (sizeof(t), The##c##ClassBehaviour, The##c##Behaviour) t
#define new_activity(l)                   (TheActivityClass->newActivity(MEDIUM_PRIORITY, l))
#define new_behaviour(t)                  (new (t) RexxBehaviour)
#define new_externalArray(s,c)            (new ((LONG)(s), c) RexxArray)
#define new_array(s)                      (new ((size_t)(s)) RexxArray)
#define new_array1(a1)                    (new ((RexxObject *)a1) RexxArray)
#define new_array2(a1,a2)                 (new ((RexxObject *)a1, (RexxObject *)a2) RexxArray)
#define new_array3(a1,a2,a3)              (new ((RexxObject *)a1, (RexxObject *)a2, (RexxObject *)a3) RexxArray)
#define new_array4(a1,a2,a3,a4)           (new ((RexxObject *)a1, (RexxObject *)a2, (RexxObject *)a3, (RexxObject *)a4) RexxArray)
#define new_buffer(s)                     (new (s) RexxBuffer)
#define new_clause()                      (new RexxClause)
#define new_counter(v)                    (new RexxInteger (v))
#define new_directory()                   (memoryObject.newDirectory())
#define new_envelope()                    (new RexxEnvelope)
#define new_hashtab(s)                    (memoryObject.newHashTable(s))
#define new_hashCollection(s, s2)         (memoryObject.newHashCollection(s, s2))
#define new_list()                        (new RexxList)
#define new_queue()                       (new RexxQueue)
#define new_integer(v)                    (TheIntegerClass->newCache(v))
#define new_message(t,m,a)                (new RexxMessage ((RexxObject *)t, (RexxObject *)m, (RexxArray *)a))
#define new_method(i,e,a,c)               (new RexxMethod (i, e, a, c))
#define new_CPPmethod(p,s,c)              (new RexxMethod (p, s, c))
#define new_nmethod(p,l)                  (TheNativeCodeClass->newClass(p, l))
#define new_numberstring(s,l)             (TheNumberStringClass->classNew(s, l))
#define new_numberstringL(l)              (TheNumberStringClass->newLong(l))
#define new_numberstringUL(l)             (TheNumberStringClass->newULong(l))
#define new_numberstringF(l)              (TheNumberStringClass->newFloat(l))
#define new_numberstringD(l)              (TheNumberStringClass->newDouble(l))
#define new_object(s)                     (memoryObject.newObject((long)(s)))
#define new_arrayofObject(s,c,b)          (memoryObject.newObjects(s, c, b))
#define new_pointer(p)                    (TheIntegerClass->newCache((LONG)p))
#define new_proxy(s)                      (TheStringClass->newProxy(s))
#define new_smartbuffer()                 (new RexxSmartBuffer(1024))
#define new_sizedSmartBuffer(size)        (new RexxSmartBuffer(size))
#define new_stack(s)                      (new(s) RexxStack (s))
#define new_savestack(s,a)                (new(a) RexxSaveStack (s, a))
#define new_internalstack(s)              (memoryObject.newInternalStack(s))
#define new_activationFrameBuffer(s)      (memoryObject.newActivationFrameBuffer(s))
#define new_variableDictionary(s)         (memoryObject.newVariableDictionary(s))
#define new_objectVariableDictionary(s)   (memoryObject.newVariableDictionary(s))
#define new_stemDictionary(s)             (memoryObject.newStemDictionary(s))
#define new_variable(n)                   (memoryObject.newVariable(n))
#define new_compoundElement(s)            (memoryObject.newCompoundElement(s))
#define new_sommethod(s)                  (TheMethodClass->newSOM(s))
#define new_instance()                    (TheObjectClass->newObject())
#define new_string(s,l)                   TheStringClass->newString(s, l)
#define raw_string(l)                     TheStringClass->rawString(l)
#define new_cstring(s)                    (TheStringClass->newCstring(s))
#define new_stringd(pd)                   (TheStringClass->newDouble(pd))
#define new_table()                       (memoryObject.newTable())
#define new_object_table()                (memoryObject.newObjectTable(DEFAULT_HASH_SIZE))
#define new_scope_table()                 (memoryObject.newObjectTable(DEFAULT_SCOPE_SIZE))
#define new_relation()                    (memoryObject.newRelation())
#define new_supplier(c,f)                 (new RexxSupplier (c,f))
#define new_token(c,s,v,l)                (new RexxToken (c, s, v, l))
#define new_arrayOfTokens(n)              (memoryObject.newObjects(sizeof(RexxToken), n, TheTokenBehaviour))

/******************************************************************************/
/* Function prototypes for different message dispatch types                   */
/******************************************************************************/

typedef RexxObject *  (far *PMF0)(RexxObject *);
typedef RexxObject *  (far *PMF1)(RexxObject *, RexxObject *);
typedef RexxObject *  (far *PMF2)(RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (far *PMF3)(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (far *PMF4)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (far *PMF5)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (far *PMF6)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (far *PMF7)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);

                                       /* pointer to method function        */
typedef RexxObject *  (far VLAENTRY *PMF) ( RexxObject *,...);

typedef PMF   near *NPPMF;             /* near pointer to above             */

typedef ULONG HEADINFO;                /* Object Header information         */

#define MCPP   0                       /* C++ method start index            */
#define MSSCPP 0                       /* C++ class method start index      */

typedef struct locationinfo {          /* token/clause location information */
  size_t line;                         /* file line location                */
  size_t offset;                       /* token location within the line    */
  size_t endline;                      /* ending line location              */
  size_t endoffset;                    /* ending offset location (+1)       */
} LOCATIONINFO;

typedef LOCATIONINFO *PLOCATIONINFO;   /* pointer to location information   */

typedef struct internalmethodentry {   /* internal method table entry       */
  PCHAR  entryName;                    /* internal entry point name         */
  PFN    entryPoint;                   /* method entry point                */
} internalMethodEntry;

class ACTIVATION_SETTINGS {            /* activation "global" settings      */
    public:
      inline ACTIVATION_SETTINGS()
      {
          digits = DEFAULT_DIGITS;
          fuzz = DEFAULT_FUZZ;
          form = DEFAULT_FORM;
          DBCS_codepage = FALSE;
          codepage = 0;
          DBCS_table = NULL;
      }

      size_t digits;                       /* numeric digits setting            */
      size_t fuzz;                         /* numeric fuzz setting              */
      BOOL form;                           /* numeric form setting              */
      BOOL exmode;                         /* DBCS string processing flag       */
      BOOL DBCS_codepage;                  /* DBCS code page possible           */
      ULONG codepage;                      /* current activity codepage         */
      PUCHAR DBCS_table;                   /* current DBCS vector table         */
};                                     /* global activation settings        */
                                       /* builtin function prototype        */
typedef RexxObject *builtin_func(RexxActivation *, INT, RexxExpressionStack *);
typedef builtin_func *pbuiltin;        /* pointer to a builtin function     */

                                       /*  as "overLoading" of hashValue  */
typedef struct {
  short typeNum;
  short behaviourFlags;
} BEHAVIOURINFO;

typedef struct {
  USHORT methnum;                      /* kernel method number            */
  UCHAR  arguments;
  UCHAR  flags;                        /* flag information                */
} METHODINFO;

typedef struct {
  UCHAR  type;                         /* name of the instruction           */
  UCHAR  flags;                        /* general flag area                 */
  USHORT general;                      /* general reusable short value      */
} INSTRUCTIONINFO;

                                       /* used ofor special constructor   */
typedef enum {RESTOREIMAGE, MOBILEUNFLATTEN, METHODUNFLATTEN} RESTORETYPE;

/******************************************************************************/
/* Change EXTERN definition if not already created by GDATA                   */
/******************************************************************************/

#ifndef INITGLOBALPTR                  // if not the global, this is a NOP.
#define INITGLOBALPTR
#endif
#ifndef EXTERN
#define EXTERN extern                  /* turn into external definition     */
#endif

#ifndef EXTERNMEM
#define EXTERNMEM extern               /* turn into external definition     */
#endif

/******************************************************************************/
/* Primitive Method Type Definition Macros                                    */
/******************************************************************************/
                                       /* following two are used by OKINIT  */
                                       /*  to build the VFT Array.          */
#define CLASS_EXTERNAL(b,c)
#define CLASS_INTERNAL(b,c)

#define koper(name) RexxObject *name(RexxObject *);

                                       /* declare a class creation routine  */
                                       /* for classes with their own        */
                                       /* explicit class objects            */
#define CLASS_CREATE(name, className) The##name##Class = (className *)new (0, The##name##ClassBehaviour, The##name##Behaviour) RexxClass;
                                       /* restore a class from its          */
                                       /* associated primitive behaviour    */
                                       /* (already restored by memory_init) */
#define RESTORE_CLASS(name, location, className) The##name##Class = (className *)pbehav[T_##location].restoreClass();



/******************************************************************************/
/* Global Objects - General                                                   */
/******************************************************************************/
EXTERN RexxActivityClass  * TheActivityClass INITGLOBALPTR;
EXTERN RexxActivity * CurrentActivity INITGLOBALPTR; /* current active activity           */
#ifdef SCRIPTING
EXTERN RexxObject* (__stdcall *NovalueCallback)(void*) INITGLOBALPTR;
#endif

EXTERN RexxClass  * TheArrayClass INITGLOBALPTR;     /* array class                       */
EXTERN RexxClass  * TheClassClass INITGLOBALPTR;     /* class of classes                  */
EXTERN RexxClass  * TheDirectoryClass INITGLOBALPTR; /* directory class                   */
                                       /* envelope class - for mobile       */
                                       /*objects                            */
EXTERN RexxClass  * TheEnvelopeClass INITGLOBALPTR;
EXTERN RexxDirectory * TheEnvironment INITGLOBALPTR; /* environment object                */

EXTERN RexxDirectory * ThePublicRoutines INITGLOBALPTR; /* public_routines directory                */
EXTERN RexxDirectory * TheStaticRequires INITGLOBALPTR; /* static_requires directory                */

EXTERN MemorySegmentPool *GlobalPoolBase INITGLOBALPTR;

EXTERN RexxDirectory * TheEnvironmentBase INITGLOBALPTR; // environment object base ptr
                                       /* function table                    */
EXTERN RexxDirectory * TheFunctionsDirectory INITGLOBALPTR;
                                       /* global string set                 */
EXTERN RexxDirectory * TheGlobalStrings INITGLOBALPTR;
                                       /* integer class                     */
EXTERN RexxIntegerClass  * TheIntegerClass INITGLOBALPTR;
EXTERN RexxDirectory  * TheKernel INITGLOBALPTR;     /* kernel directory                  */
EXTERN RexxListClass  * TheListClass INITGLOBALPTR;  /* list class                        */
EXTERN RexxMemory * TheMemoryObject INITGLOBALPTR;   /* memory object                     */
EXTERNMEM RexxMemory  memoryObject;   /* memory object                     */
EXTERN RexxClass  * TheMessageClass INITGLOBALPTR;   /* message class                     */
                                       /* method class                      */
EXTERN RexxMethodClass  * TheMethodClass INITGLOBALPTR;
                                       /* native method class               */
EXTERN RexxNativeCodeClass  * TheNativeCodeClass INITGLOBALPTR;
EXTERN RexxObject * TheNilObject INITGLOBALPTR;      /* nil object                        */
EXTERN RexxArray  * TheNullArray INITGLOBALPTR;      /* null arg list                     */
                                       /* null pointer object, pointer to   */
                                       /*NULL                               */
EXTERN RexxInteger* TheNullPointer INITGLOBALPTR;
                                       /* NumberString class                */
EXTERN RexxNumberStringClass  * TheNumberStringClass INITGLOBALPTR;
EXTERN RexxClass  * TheObjectClass INITGLOBALPTR;    /* generic object class              */
EXTERN RexxClass  * TheQueueClass INITGLOBALPTR;     /* queue class                       */
                                       /* Generic SOM  method               */
EXTERN RexxSOMCode * TheGenericSomMethod INITGLOBALPTR;
                                       /* Predefined variable retrievers    */
EXTERN RexxDirectory * TheCommonRetrievers INITGLOBALPTR;
EXTERN RexxClass  * TheStemClass INITGLOBALPTR;      /* stem class                        */
                                       /* string class                      */
EXTERN RexxStringClass  * TheStringClass INITGLOBALPTR;
                                       /* mutablebuffer class                */
EXTERN RexxClass  * TheMutableBufferClass INITGLOBALPTR;
                                       /* Save array used for quick lookup  */
                                       /*of objects during restore image    */
EXTERN RexxArray  * TheSaveArray INITGLOBALPTR;
                                       /* saved array of primitive          */
                                       /*behaviours                         */
EXTERN RexxObject * TheSavedBehaviours INITGLOBALPTR;
EXTERN RexxClass  * TheSupplierClass INITGLOBALPTR;  /* supplier class                    */
                                       /* somproxy class                    */
EXTERN RexxSOMProxyClass  * TheSomProxyClass INITGLOBALPTR;
                                       /* M_somproxy class                  */
EXTERN RexxSOMProxyClass  * TheMSomProxyClass INITGLOBALPTR;
EXTERN RexxDirectory * TheSystem INITGLOBALPTR;     /* system directory                  */
EXTERN RexxClass  * TheTableClass INITGLOBALPTR;     /* table class                       */
EXTERN RexxClass  * TheRelationClass INITGLOBALPTR;  /* relation class                    */

EXTERN RexxInteger * TheFalseObject INITGLOBALPTR;   /* false object                      */
EXTERN RexxInteger * TheTrueObject INITGLOBALPTR;    /* true object                       */
EXTERN RexxInteger * IntegerZero INITGLOBALPTR;      /* Static integer 0                  */
EXTERN RexxInteger * IntegerOne INITGLOBALPTR;       /* Static integer 1                  */
EXTERN RexxInteger * IntegerTwo INITGLOBALPTR;       /* Static integer 2                  */
EXTERN RexxInteger * IntegerThree INITGLOBALPTR;     /* Static integer 3                  */
EXTERN RexxInteger * IntegerFour INITGLOBALPTR;      /* Static integer 4                  */
EXTERN RexxInteger * IntegerFive INITGLOBALPTR;      /* Static integer 5                  */
EXTERN RexxInteger * IntegerSix INITGLOBALPTR;       /* Static integer 6                  */
EXTERN RexxInteger * IntegerSeven INITGLOBALPTR;     /* Static integer 7                  */
EXTERN RexxInteger * IntegerEight INITGLOBALPTR;     /* Static integer 8                  */
EXTERN RexxInteger * IntegerNine INITGLOBALPTR;      /* Static integer 9                  */


/******************************************************************************/
/* Primitive Object Types (keep in sync with behaviour_id in RexxBehaviour.c)       */
/******************************************************************************/
                                       /* IMPORTANT NOTE:  The includes in  */
                                       /* PrimitiveClasses.h MUST be included in the */
                                       /* same order as the T_behaviour     */
                                       /* defines created here.  Also,      */
                                       /* the table in behaviour_id in      */
                                       /* RexxBehaviour.c must also have the names*/
                                       /* in exactly the same order         */

                                       /* SECOND IMPORTANT NOTE:  T_        */
                                       /* defines ending in "_class" do not */
                                       /* have their own include files in   */
                                       /* PrimitiveClasses.h, but are rather in the   */
                                       /* corresponding "major" class       */
                                       /* definition file.  For example,    */
                                       /* T_array and T_array_class are both  */
                                       /* covered by including ArrayClass.h     */
#define lowest_T                     0   /* lowest type number */
#define T_object                     0
#define T_object_class               T_object                     + 1
#define T_class                      T_object_class               + 1
#define T_class_class                T_class                      + 1
#define T_array                      T_class_class                + 1
#define T_array_class                T_array                      + 1
#define T_directory                  T_array_class                + 1
#define T_directory_class            T_directory                  + 1
#define T_envelope                   T_directory_class            + 1
#define T_envelope_class             T_envelope                   + 1
#define T_integer                    T_envelope_class             + 1
#define T_integer_class              T_integer                    + 1
#define T_list                       T_integer_class              + 1
#define T_list_class                 T_list                       + 1
#define T_message                    T_list_class                 + 1
#define T_message_class              T_message                    + 1
#define T_method                     T_message_class              + 1
#define T_method_class               T_method                     + 1
#define T_numberstring               T_method_class               + 1
#define T_numberstring_class         T_numberstring               + 1
#define T_queue                      T_numberstring_class         + 1
#define T_queue_class                T_queue                      + 1
#define T_stem                       T_queue_class                + 1
#define T_stem_class                 T_stem                       + 1
#define T_string                     T_stem_class                 + 1
#define T_string_class               T_string                     + 1
#define T_somproxy                   T_string_class               + 1
#define T_somproxy_class             T_somproxy                   + 1
#define T_supplier                   T_somproxy_class             + 1
#define T_supplier_class             T_supplier                   + 1
#define T_table                      T_supplier_class             + 1
#define T_table_class                T_table                      + 1
#define T_relation                   T_table_class                + 1
#define T_relation_class             T_relation                   + 1
#define T_memory                     T_relation_class             + 1
#define T_mutablebuffer              T_memory                     + 1
#define T_mutablebuffer_class        T_mutablebuffer              + 1

                                       /* define to the top point for       */
                                       /* classes of objects that are       */
                                       /* exposed as REXX objects           */

#define highest_exposed_T            T_mutablebuffer_class

#define T_intstack                   highest_exposed_T            + 1
#define T_activation                 T_intstack                   + 1
#define T_activity                   T_activation                 + 1
#define T_activity_class             T_activity                   + 1
#define T_behaviour                  T_activity_class             + 1
#define T_buffer                     T_behaviour                  + 1
#define T_corral                     T_buffer                     + 1
#define T_hashtab                    T_corral                     + 1
#define T_listtable                  T_hashtab                    + 1
#define T_rexxmethod                 T_listtable                  + 1
#define T_nmethod                    T_rexxmethod                 + 1
#define T_nmethod_class              T_nmethod                    + 1
#define T_nativeact                  T_nmethod_class              + 1
#define T_smartbuffer                T_nativeact                  + 1
#define T_sommethod                  T_smartbuffer                + 1
#define T_stack                      T_sommethod                  + 1
#define T_variable                   T_stack                      + 1
#define T_vdict                      T_variable                   + 1
#define T_clause                     T_vdict                      + 1
#define T_source                     T_clause                     + 1
#define T_token                      T_source                     + 1
#define T_parse_instruction          T_token                      + 1
#define T_parse_address              T_parse_instruction          + 1
#define T_parse_assignment           T_parse_address              + 1
#define T_parse_block                T_parse_assignment           + 1
#define T_parse_call                 T_parse_block                + 1
#define T_parse_command              T_parse_call                 + 1
#define T_parse_compound             T_parse_command              + 1
#define T_parse_do                   T_parse_compound             + 1
#define T_parse_dExpressionBaseVariableiable          T_parse_do                   + 1
#define T_parse_drop                 T_parse_dExpressionBaseVariableiable          + 1
#define T_parse_else                 T_parse_drop                 + 1
#define T_parse_end                  T_parse_else                 + 1
#define T_parse_endif                T_parse_end                  + 1
#define T_parse_exit                 T_parse_endif                + 1
#define T_parse_expose               T_parse_exit                 + 1
#define T_parse_forward              T_parse_expose               + 1
#define T_parse_function             T_parse_forward              + 1
#define T_parse_guard                T_parse_function             + 1
#define T_parse_if                   T_parse_guard                + 1
#define T_parse_interpret            T_parse_if                   + 1
#define T_parse_label                T_parse_interpret            + 1
#define T_parse_leave                T_parse_label                + 1
#define T_parse_message              T_parse_leave                + 1
#define T_parse_message_send         T_parse_message              + 1
#define T_parse_nop                  T_parse_message_send         + 1
#define T_parse_numeric              T_parse_nop                  + 1
#define T_parse_operator             T_parse_numeric              + 1
#define T_parse_options              T_parse_operator             + 1
#define T_parse_otherwise            T_parse_options              + 1
#define T_parse_parse                T_parse_otherwise            + 1
#define T_parse_procedure            T_parse_parse                + 1
#define T_parse_queue                T_parse_procedure            + 1
#define T_parse_raise                T_parse_queue                + 1
#define T_parse_reply                T_parse_raise                + 1
#define T_parse_return               T_parse_reply                + 1
#define T_parse_say                  T_parse_return               + 1
#define T_parse_select               T_parse_say                  + 1
#define T_parse_signal               T_parse_select               + 1
#define T_parse_stem                 T_parse_signal               + 1
#define T_parse_then                 T_parse_stem                 + 1
#define T_parse_trace                T_parse_then                 + 1
#define T_parse_trigger              T_parse_trace                + 1
#define T_parse_use                  T_parse_trigger              + 1
#define T_parse_variable             T_parse_use                  + 1
#define T_parse_varref               T_parse_variable             + 1
#define T_compound_element           T_parse_varref               + 1
#define T_activation_frame_buffer    T_compound_element           + 1
#define T_parse_unary_operator       T_activation_frame_buffer    + 1
#define T_parse_binary_operator      T_parse_unary_operator       + 1
#define highest_T                    T_parse_binary_operator

/******************************************************************************/
/* Define location of objects saved in SaveArray during Saveimage processing  */
/*  and used during restart processing.                                       */
/* Currently only used in OKMEMORY.C                                          */
/******************************************************************************/
#define saveArray_ENV                1
#define saveArray_KERNEL             saveArray_ENV               + 1
#define saveArray_NAME_STRINGS       saveArray_KERNEL            + 1
#define saveArray_TRUE               saveArray_NAME_STRINGS      + 1
#define saveArray_FALSE              saveArray_TRUE              + 1
#define saveArray_NIL                saveArray_FALSE             + 1
#define saveArray_GLOBAL_STRINGS     saveArray_NIL               + 1
#define saveArray_CLASS              saveArray_GLOBAL_STRINGS    + 1
#define saveArray_PBEHAV             saveArray_CLASS             + 1
#define saveArray_ACTIVITY           saveArray_PBEHAV            + 1
#define saveArray_NMETHOD            saveArray_ACTIVITY          + 1
#define saveArray_NULLA              saveArray_NMETHOD           + 1
#define saveArray_NULLPOINTER        saveArray_NULLA             + 1
#define saveArray_SYSTEM             saveArray_NULLPOINTER       + 1
#define saveArray_FUNCTIONS          saveArray_SYSTEM            + 1
#define saveArray_GENERIC_SOMMETHOD  saveArray_FUNCTIONS         + 1
#define saveArray_COMMON_RETRIEVERS  saveArray_GENERIC_SOMMETHOD + 1
#define saveArray_M_SOMPROXY         saveArray_COMMON_RETRIEVERS + 1
#define saveArray_STATIC_REQ         saveArray_M_SOMPROXY        + 1
#define saveArray_PUBLIC_RTN         saveArray_STATIC_REQ        + 1
#define saveArray_highest            saveArray_PUBLIC_RTN

/******************************************************************************/
/* Global Objects - Primitive Behaviour                                       */
/******************************************************************************/
#ifndef GDATA
                                       /* table of primitive behaviours     */
EXTERN RexxBehaviour pbehav[highest_T+1];
EXTERN void *VFTArray[highest_T];      /* table of virtual functions        */

#endif

#define TheActivationBehaviour      ((RexxBehaviour *)(&pbehav[T_activation]))
#define TheActivityBehaviour        ((RexxBehaviour *)(&pbehav[T_activity]))
#define TheActivityClassBehaviour   ((RexxBehaviour *)(&pbehav[T_activity_class]))
#define TheArrayBehaviour           ((RexxBehaviour *)(&pbehav[T_array]))
#define TheArrayClassBehaviour      ((RexxBehaviour *)(&pbehav[T_array_class]))
#define TheBehaviourBehaviour       ((RexxBehaviour *)(&pbehav[T_behaviour]))
#define TheBufferBehaviour          ((RexxBehaviour *)(&pbehav[T_buffer]))
#define TheClassBehaviour           ((RexxBehaviour *)(&pbehav[T_class]))
#define TheClassClassBehaviour      ((RexxBehaviour *)(&pbehav[T_class_class]))
#define TheCorralBehaviour          ((RexxBehaviour *)(&pbehav[T_corral]))
#define TheDirectoryBehaviour       ((RexxBehaviour *)(&pbehav[T_directory]))
#define TheDirectoryClassBehaviour  ((RexxBehaviour *)(&pbehav[T_directory_class]))
#define TheEnvelopeBehaviour        ((RexxBehaviour *)(&pbehav[T_envelope]))
#define TheEnvelopeClassBehaviour   ((RexxBehaviour *)(&pbehav[T_envelope_class]))
#define TheHashTableBehaviour       ((RexxBehaviour *)(&pbehav[T_hashtab]))
#define TheIntegerBehaviour         ((RexxBehaviour *)(&pbehav[T_integer]))
#define TheIntegerClassBehaviour    ((RexxBehaviour *)(&pbehav[T_integer_class]))
#define TheListBehaviour            ((RexxBehaviour *)(&pbehav[T_list]))
#define TheListClassBehaviour       ((RexxBehaviour *)(&pbehav[T_list_class]))
#define TheListTableBehaviour       ((RexxBehaviour *)(&pbehav[T_listtable]))
#define TheMemoryBehaviour          ((RexxBehaviour *)(&pbehav[T_memory]))
#define TheMessageBehaviour         ((RexxBehaviour *)(&pbehav[T_message]))
#define TheMessageClassBehaviour    ((RexxBehaviour *)(&pbehav[T_message_class]))
#define TheMethodBehaviour          ((RexxBehaviour *)(&pbehav[T_method]))
#define TheMethodClassBehaviour     ((RexxBehaviour *)(&pbehav[T_method_class]))
#define TheNativeCodeBehaviour      ((RexxBehaviour *)(&pbehav[T_nmethod]))
#define TheNativeCodeClassBehaviour ((RexxBehaviour *)(&pbehav[T_nmethod_class]))
#define TheRexxCodeBehaviour        ((RexxBehaviour *)(&pbehav[T_rexxmethod]))
#define TheNativeActivationBehaviour ((RexxBehaviour *)(&pbehav[T_nativeact]))
#define TheNumberStringBehaviour    ((RexxBehaviour *)(&pbehav[T_numberstring]))
#define TheNumberStringClassBehaviour  ((RexxBehaviour *)(&pbehav[T_numberstring_class]))
#define TheObjectBehaviour          ((RexxBehaviour *)(&pbehav[T_object]))
#define TheObjectClassBehaviour     ((RexxBehaviour *)(&pbehav[T_object_class]))
#define TheQueueBehaviour           ((RexxBehaviour *)(&pbehav[T_queue]))
#define TheQueueClassBehaviour      ((RexxBehaviour *)(&pbehav[T_queue_class]))
#define TheSmartBufferBehaviour     ((RexxBehaviour *)(&pbehav[T_smartbuffer]))
#define TheSomCodeBehaviour         ((RexxBehaviour *)(&pbehav[T_sommethod]))
#define TheStackBehaviour           ((RexxBehaviour *)(&pbehav[T_stack]))
#define TheStemBehaviour            ((RexxBehaviour *)(&pbehav[T_stem]))
#define TheStemClassBehaviour       ((RexxBehaviour *)(&pbehav[T_stem_class]))
#define TheStringBehaviour          ((RexxBehaviour *)(&pbehav[T_string]))
#define TheStringClassBehaviour     ((RexxBehaviour *)(&pbehav[T_string_class]))
#define TheSomProxyBehaviour        ((RexxBehaviour *)(&pbehav[T_somproxy]))
#define TheSomProxyClassBehaviour   ((RexxBehaviour *)(&pbehav[T_somproxy_class]))
#define TheSupplierBehaviour        ((RexxBehaviour *)(&pbehav[T_supplier]))
#define TheSupplierClassBehaviour   ((RexxBehaviour *)(&pbehav[T_supplier_class]))
#define TheTableBehaviour           ((RexxBehaviour *)(&pbehav[T_table]))
#define TheTableClassBehaviour      ((RexxBehaviour *)(&pbehav[T_table_class]))
#define TheRelationBehaviour        ((RexxBehaviour *)(&pbehav[T_relation]))
#define TheRelationClassBehaviour   ((RexxBehaviour *)(&pbehav[T_relation_class]))
#define TheVariableBehaviour        ((RexxBehaviour *)(&pbehav[T_variable]))
#define TheCompoundElementBehaviour ((RexxBehaviour *)(&pbehav[T_compound_element]))
#define TheVariableDictionaryBehaviour ((RexxBehaviour *)(&pbehav[T_vdict]))
#define TheMutableBufferBehaviour   ((RexxBehaviour *)(&pbehav[T_mutablebuffer]))
#define TheMutableBufferClassBehaviour ((RexxBehaviour *)(&pbehav[T_mutablebuffer_class]))

#define TheAddressInstructionBehaviour      ((RexxBehaviour *)(&pbehav[T_parse_address]))
#define TheAssignmentInstructionBehaviour   ((RexxBehaviour *)(&pbehav[T_parse_assignment]))
#define TheDoBlockBehaviour                 ((RexxBehaviour *)(&pbehav[T_parse_block]))
#define TheCallInstructionBehaviour         ((RexxBehaviour *)(&pbehav[T_parse_call]))
#define TheCommandInstructionBehaviour      ((RexxBehaviour *)(&pbehav[T_parse_command]))
#define TheCompoundVariableBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_compound]))
#define TheDoInstructionBehaviour           ((RexxBehaviour *)(&pbehav[T_parse_do]))
#define TheDotVariableBehaviour             ((RexxBehaviour *)(&pbehav[T_parse_dExpressionBaseVariableiable]))
#define TheDropInstructionBehaviour         ((RexxBehaviour *)(&pbehav[T_parse_drop]))
#define TheElseInstructionBehaviour         ((RexxBehaviour *)(&pbehav[T_parse_else]))
#define TheEndInstructionBehaviour          ((RexxBehaviour *)(&pbehav[T_parse_end]))
#define TheEndIfInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_endif]))
#define TheExitInstructionBehaviour         ((RexxBehaviour *)(&pbehav[T_parse_exit]))
#define TheExposeInstructionBehaviour       ((RexxBehaviour *)(&pbehav[T_parse_expose]))
#define TheForwardInstructionBehaviour      ((RexxBehaviour *)(&pbehav[T_parse_forward]))
#define TheFunctionBehaviour                ((RexxBehaviour *)(&pbehav[T_parse_function]))
#define TheGuardInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_guard]))
#define TheIfInstructionBehaviour           ((RexxBehaviour *)(&pbehav[T_parse_if]))
#define TheInstructionBehaviour             ((RexxBehaviour *)(&pbehav[T_parse_instruction]))
#define TheInterpretInstructionBehaviour    ((RexxBehaviour *)(&pbehav[T_parse_interpret]))
#define TheLabelInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_label]))
#define TheLeaveInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_leave]))
#define TheMessageInstructionBehaviour      ((RexxBehaviour *)(&pbehav[T_parse_message]))
#define TheMessageSendBehaviour             ((RexxBehaviour *)(&pbehav[T_parse_message_send]))
#define TheNopInstructionBehaviour          ((RexxBehaviour *)(&pbehav[T_parse_nop]))
#define TheNumericInstructionBehaviour      ((RexxBehaviour *)(&pbehav[T_parse_numeric]))
#define TheOperatorBehaviour                ((RexxBehaviour *)(&pbehav[T_parse_operator]))
#define TheUnaryOperatorBehaviour           ((RexxBehaviour *)(&pbehav[T_parse_unary_operator]))
#define TheBinaryOperatorBehaviour          ((RexxBehaviour *)(&pbehav[T_parse_binary_operator]))
#define TheOptionsInstructionBehaviour      ((RexxBehaviour *)(&pbehav[T_parse_options]))
#define TheOtherWiseInstructionBehaviour    ((RexxBehaviour *)(&pbehav[T_parse_otherwise]))
#define TheParseInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_parse]))
#define TheProcedureInstructionBehaviour    ((RexxBehaviour *)(&pbehav[T_parse_procedure]))
#define TheQueueInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_queue]))
#define TheRaiseInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_raise]))
#define TheReplyInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_reply]))
#define TheReturnInstructionBehaviour       ((RexxBehaviour *)(&pbehav[T_parse_return]))
#define TheSayInstructionBehaviour          ((RexxBehaviour *)(&pbehav[T_parse_say]))
#define TheSelectInstructionBehaviour       ((RexxBehaviour *)(&pbehav[T_parse_select]))
#define TheSignalInstructionBehaviour       ((RexxBehaviour *)(&pbehav[T_parse_signal]))
#define TheStemVariableBehaviour            ((RexxBehaviour *)(&pbehav[T_parse_stem]))
#define TheThenInstructionBehaviour         ((RexxBehaviour *)(&pbehav[T_parse_then]))
#define TheTraceInstructionBehaviour        ((RexxBehaviour *)(&pbehav[T_parse_trace]))
#define TheParseTriggerBehaviour            ((RexxBehaviour *)(&pbehav[T_parse_trigger]))
#define TheUseInstructionBehaviour          ((RexxBehaviour *)(&pbehav[T_parse_use]))
#define TheParseVariableBehaviour           ((RexxBehaviour *)(&pbehav[T_parse_variable]))
#define TheVariableReferenceBehaviour       ((RexxBehaviour *)(&pbehav[T_parse_varref]))
#define TheSourceBehaviour                  ((RexxBehaviour *)(&pbehav[T_source]))
#define TheClauseBehaviour                  ((RexxBehaviour *)(&pbehav[T_clause]))
#define TheTokenBehaviour                   ((RexxBehaviour *)(&pbehav[T_token]))
#define TheInternalStackBehaviour           ((RexxBehaviour *)(&pbehav[T_intstack]))
#define TheActivationFrameBufferBehaviour   ((RexxBehaviour *)(&pbehav[T_activation_frame_buffer]))

/******************************************************************************/
/* Utility Macros                                                             */
/******************************************************************************/

#define RXROUNDUP(n,to)  ((((n)+(to-1))/(to))*to)
#define rounddown(n,to)  (((n)/(to))*to)
#define same_behaviour(oref) ((oref)->behaviour==this->behaviour)

                                       /* current object's behaviour        */
#define THIS_BEHAVIOUR (this->behaviour)
#define PASTE3(a1,a2,a3) a1##a2##a3

#define ObjectType(r) (((RexxObject *)(r))->behaviour)
#define ObjectTypeNumber(r) (ObjectType(r)->typenum())
                                       /* check the object type             */
#define OTYPE(t,r) (ObjectType(r) == The##t##Behaviour)
#define OTYPENUM(t,r) (ObjectTypeNumber(r) == T_##t)
#define IsSameType(me, you) (ObjectType(me) == ObjectType(you))

/* assign a new behaviour */
#define BehaviourSet(o, b)  (o)->behaviour = (RexxBehaviour *)b

                                       /* verify an objects behaviour       */
#define HASBEHAV(b,r) (((r)->behaviour) == b)
                                       /* access an object's hash value     */
#define HASHVALUE(r) ((ULONG)((r)->hashvalue))
                                       /* generate hash value from OREF     */
#define HASHOREF(r) ((long)((ULONG)r>>2))

/******************************************************************************/
/* Utility Functions                                                          */
/******************************************************************************/

void logic_error (char *desc);
                                       /* do a case insensitive compare     */
INT  CaselessCompare(PUCHAR, PUCHAR, LONG);
PCHAR mempbrk(PCHAR, PCHAR, LONG);     /* search for characters             */

                                       /* find an environment symbol        */
#define env_find(s) (TheEnvironment->entry(s))
                                       /* various exception/condition       */
                                       /* reporting routines                */
#define report_novalue(description) report_condition(OREF_NOVALUE, (RexxString *)description)
#define report_nostring(description) report_condition(OREF_NOSTRING, (RexxString *)description)
#define report_condition(condition, description) (CurrentActivity->raiseCondition(condition, OREF_NULL, (RexxString *)description, OREF_NULL, OREF_NULL, OREF_NULL))
void report_halt(RexxString *description);
void report_nomethod(RexxString *message, RexxObject *receiver);
void missing_argument(LONG position);
long message_number(RexxString *);
RexxActivity *activity_find (void);

#define report_exception(error) (CurrentActivity->reportAnException(error))
#define report_exception1(error, a1) (CurrentActivity->reportAnException(error, a1))
#define report_exception2(error, a1, a2) (CurrentActivity->reportAnException(error, a1, a2))
#define report_exception3(error, a1, a2, a3) (CurrentActivity->reportAnException(error, a1, a2, a3))
#define report_exception4(error, a1, a2, a3, a4) (CurrentActivity->reportAnException(error, a1, a2, a3, a4))
#define report_exceptiond(error,d) CurrentActivity->raiseException(error, NULL, OREF_NULL, OREF_NULL, d, OREF_NULL)

                                       /* verify argument presence          */
#define required_arg(arg, position) if (arg == OREF_NULL) missing_argument(ARG_##position)

/******************************************************************************/
/* Thread constants                                                           */
/******************************************************************************/

#define NO_THREAD       -1

/******************************************************************************/
/* Constant GLobal values (for general use)                                   */
/******************************************************************************/

/* MHES
const int  NO_INT  = 0x80000000;
const long NO_LONG = 0x80000000;
const PCHAR NO_CSTRING = NULL;
#define NO_RSTRING       NULL
*/
/*
extern const int  NO_INT;
extern const long NO_LONG;
extern const PCHAR NO_CSTRING;
*/
/* Also in RexxNativeAPI.h */
#ifndef NO_INT
# define NO_INT                0x80000000
#endif
#ifndef NO_LONG
# define NO_LONG               0x80000000
#endif
#ifndef NO_CSTRING
# define NO_CSTRING            NULL
#endif
#define NO_RSTRING       NULL

extern double NO_DOUBLE;

/******************************************************************************/
/* Global Objects - Names                                                     */
/******************************************************************************/
#undef GLOBAL_NAME
#define GLOBAL_NAME(name, value) EXTERN RexxString * OREF_##name INITGLOBALPTR;
#include "GlobalNames.h"

#ifndef GDATA_BUILD_BEHAVIOURS

 #include "ObjectClass.hpp"               /* get real definition of Object     */

/******************************************************************************/
/* Method pointer special types                                               */
/******************************************************************************/

 typedef RexxObject *  (RexxObject::*PCPPM0)();
 typedef RexxObject *  (RexxObject::*PCPPM1)(RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM2)(RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM3)(RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM4)(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM5)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM6)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM7)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPMA1)(RexxArray *);
 typedef RexxObject *  (RexxObject::*PCPPMC1)(RexxObject **, size_t);
                                       /* pointer to method function        */
 typedef RexxObject *  (VLAENTRY RexxObject::*PCPPM) (...);
 #define CPPM(n) ((PCPPM)&n)

#ifdef LINUX

 #include "TableClass.hpp"
 #include "StackClass.hpp"
 #include "RexxMemory.hpp"               /* memory next, to get OrefSet       */
 #include "RexxBehaviour.hpp"                /* now behaviours and                */
 #include "ClassClass.hpp"                /* classes, which everything needs   */
 #include "RexxEnvelope.hpp"                /* envelope is needed for flattens   */
 #include "RexxActivity.hpp"               /* activity is needed for errors     */
 #include "NumberStringClass.hpp"               /* added to make 'number_digits()'   */
                                       /* in 'ArrayClass.c' visible            */
 #define PCPPSOM    PCPPM
 #define PCPPINT    PCPPM
 #define PCPPSTR    PCPPM
 #define PCPPNUMSTR PCPPM
 #define PCPPMUTB   PCPPM

 #define CPPMIO(n) CPPM(n)
 #define CPPMC(n)  CPPM(n)
 #define CPPMLC(n) CPPM(n)
 #define CPPMA(n)  CPPM(n)
 #define CPPMC1(n)  CPPM(n)
 #define CPPMD(n)  CPPM(n)
 #define CPPMHC(n)  CPPM(n)
 #define CPPME(n)  CPPM(n)
 #define CPPMI(n)  CPPM(n)
 #define CPPML(n)  CPPM(n)
 #define CPPMSG(n)  CPPM(n)
 #define CPPMTD(n)  CPPM(n)
 #define CPPMTDC(n)  CPPM(n)
 #define CPPMNM(n)  CPPM(n)
 #define CPPMQ(n)  CPPM(n)
 #define CPPMSTEM(n)  CPPM(n)
 #define CPPMSTR(n)  CPPM(n)
 #define CPPMSTRCL(n)  CPPM(n)
 #define CPPMSOM(n)  CPPM(n)
 #define CPPMSOMCL(n)  CPPM(n)
 #define CPPMSUP(n)  CPPM(n)
 #define CPPMSUPCL(n)  CPPM(n)
 #define CPPMTBL(n)  CPPM(n)
 #define CPPMREL(n)  CPPM(n)
 #define CPPMMEM(n)  CPPM(n)
 #define CPPMSOMS(n)  CPPM(n)
 #define CPPMLOC(n)  CPPM(n)
 #define CPPMSND(n)  CPPM(n)
 #define CPPMSRV(n)  CPPM(n)
 #define CPPMSOMDS(n)  CPPM(n)
 #define CPPMSOMDO(n)  CPPM(n)
 #define CPPMMUTB(n)  CPPM(n)
 #define CPPMMUTBCL(n) CPPM(n)

#else

 typedef RexxObject *  (VLAENTRY RexxInternalObject::*PCPPMINTOBJ) (...);
 #define CPPMIO(n) ((PCPPMINTOBJ)&n)

 #include "TableClass.hpp"
 #include "StackClass.hpp"
 #include "RexxMemory.hpp"               /* memory next, to get OrefSet       */
 #include "RexxBehaviour.hpp"                /* now behaviours and                */
 #include "ClassClass.hpp"                /* classes, which everything needs   */
 #include "RexxEnvelope.hpp"                /* envelope is needed for flattens   */
 #include "RexxActivity.hpp"               /* activity is needed for errors     */

 // added these for VC++
 typedef RexxObject *  (VLAENTRY RexxClass::*PCPPCLASS) (...);
 #define CPPMC(n) (PCPPM) ((PCPPCLASS)&n)
 #include "ArrayClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxArray::*PCPPARRAY) (...);
 #define CPPMA(n) (PCPPM) ((PCPPARRAY)&n)
 typedef RexxObject *  (VLAENTRY RexxArray::*PCPPCOUNT) (...);
 #define CPPMC1(n) (PCPPM) ((PCPPCOUNT)&n)
 #include "DirectoryClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxDirectory::*PCPPDIR) (...);
 #define CPPMD(n) (PCPPM) ((PCPPDIR)&n)
 #include "RexxCollection.hpp"
 typedef RexxObject *  (VLAENTRY RexxHashTableCollection::*PCPPHASHCOL) (...);
 #define CPPMHC(n) (PCPPM) ((PCPPHASHCOL)&n)
 #include "RexxEnvelope.hpp"                /* envelope is needed for flattens   */
 typedef RexxObject *  (VLAENTRY RexxEnvelope::*PCPPENV) (...);
 #define CPPME(n) (PCPPM) ((PCPPENV)&n)
 #include "IntegerClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxInteger::*PCPPINT) (...);
 #define CPPMI(n) (PCPPM) ((PCPPINT)&n)
 #include "ListClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxList::*PCPPLIST) (...);
 #define CPPML(n) (PCPPM) ((PCPPLIST)&n)
 typedef RexxObject *  (VLAENTRY RexxListClass::*PCPPLISTCLASS) (...);
 #define CPPMLC(n) (PCPPM) ((PCPPLISTCLASS)&n)
 #include "MessageClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxMessage::*PCPPMSG) (...);
 #define CPPMSG(n) (PCPPM) ((PCPPMSG)&n)
 #include "MethodClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxMethod::*PCPPMETHOD) (...);
 #define CPPMTD(n) (PCPPM) ((PCPPMETHOD)&n)
 typedef RexxObject *  (VLAENTRY RexxMethodClass::*PCPPMETHODCLASS) (...);
 #define CPPMTDC(n) (PCPPM) ((PCPPMETHODCLASS)&n)
 #include "NumberStringClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxNumberString::*PCPPNUMSTR) (...);
 #define CPPMNM(n) (PCPPM) ((PCPPNUMSTR)&n)
 #include "QueueClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxQueue::*PCPPQ) (...);
 #define CPPMQ(n) (PCPPM) ((PCPPQ)&n)
 #include "StemClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxStem::*PCPPSTEM) (...);
 #define CPPMSTEM(n) (PCPPM) ((PCPPSTEM)&n)
 #include "StringClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxString::*PCPPSTR) (...);
 #define CPPMSTR(n) (PCPPM) ((PCPPSTR)&n)
 typedef RexxObject *  (VLAENTRY RexxStringClass::*PCPPSTRCLASS) (...);
 #define CPPMSTRCL(n) (PCPPM) ((PCPPSTRCLASS)&n)
 #include "RexxSOMProxy.hpp"
 typedef RexxObject *  (VLAENTRY RexxSOMProxy::*PCPPSOM) (...);
 #define CPPMSOM(n) (PCPPM) ((PCPPSOM)&n)
 typedef RexxObject *  (VLAENTRY RexxSOMProxyClass::*PCPPSOMCLASS) (...);
 #define CPPMSOMCL(n) (PCPPM) ((PCPPSOMCLASS)&n)
 #include "SupplierClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxSupplier::*PCPPSUP) (...);
 #define CPPMSUP(n) (PCPPM) ((PCPPSUP)&n)
 typedef RexxObject *  (VLAENTRY RexxSupplierClass::*PCPPSUPCLASS) (...);
 #define CPPMSUPCL(n) (PCPPM) ((PCPPSUPCLASS)&n)
 #include "TableClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxTable::*PCPPTBL) (...);
 #define CPPMTBL(n) (PCPPM) ((PCPPTBL)&n)
 #include "RelationClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxRelation::*PCPPREL) (...);
 #define CPPMREL(n) (PCPPM) ((PCPPREL)&n)
 #include "RexxMemory.hpp"
 typedef RexxObject *  (VLAENTRY RexxMemory::*PCPPMEM) (...);
 #define CPPMMEM(n) (PCPPM) ((PCPPMEM)&n)
 #include "RexxMisc.hpp"
 typedef RexxObject *  (VLAENTRY RexxSOMServer::*PCPPSOMSRV) (...);
 #define CPPMSOMS(n) (PCPPM) ((PCPPSOMSRV)&n)
 typedef RexxObject *  (VLAENTRY RexxLocal::*PCPPLOCAL) (...);
 #define CPPMLOC(n) (PCPPM) ((PCPPLOCAL)&n)
 typedef RexxObject *  (VLAENTRY RexxSender::*PCPPSEND) (...);
 #define CPPMSND(n) (PCPPM) ((PCPPSEND)&n)
 typedef RexxObject *  (VLAENTRY RexxServer::*PCPPSRV) (...);
 #define CPPMSRV(n) (PCPPM) ((PCPPSRV)&n)
 typedef RexxObject *  (VLAENTRY RexxSOMDServer::*PCPPSOMD) (...);
 #define CPPMSOMDS(n) (PCPPM) ((PCPPSOMD)&n)
 typedef RexxObject *  (VLAENTRY RexxSOMDObjectMgr::*PCPPSOMO) (...);
 #define CPPMSOMDO(n) (PCPPM) ((PCPPSOMO)&n)
 #include "MutableBufferClass.hpp"
 typedef RexxObject *  (VLAENTRY RexxMutableBuffer::*PCPPMUTB) (...);
 #define CPPMMUTB(n) (PCPPM) ((PCPPMUTB)&n)
 typedef RexxObject *  (VLAENTRY RexxMutableBufferClass::*PCPPMUTBCL) (...);
 #define CPPMMUTBCL(n) (PCPPM) ((PCPPMUTBCL)&n)

#endif

#endif

/******************************************************************************/
/* Method arguments special codes                                             */
/******************************************************************************/

const int A_COUNT   = 127;            /* pass arguments as pointer/count pair */

/******************************************************************************/
/* Return codes                                                               */
/******************************************************************************/

const int RC_OK         = 0;
const int RC_LOGIC_ERROR  = 2;

RexxString *last_msgname (void);       /* last message issued               */
RexxMethod *last_method  (void);       /* last method invoked               */

                                       /* data converstion and validation   */
                                       /* routines                          */
void process_new_args(RexxObject **, size_t, RexxObject ***, size_t *, size_t, RexxObject **, RexxObject **);

const int POSITIVE    = 1;             /* integer must be positive          */
const int NONNEGATIVE = 2;             /* integer must be non-negative      */
const int WHOLE       = 3;             /* integer must be whole             */

/* The next macro is specifically for REQUESTing a STRING, since there are    */
/* four primitive classes that are equivalents for strings.  It will trap on  */
/* OREF_NULL. */
#ifndef GDATA
inline RexxString *REQUEST_STRING(RexxObject *object)
{
  return (OTYPE(String, object) ? (RexxString *)object : (object)->requestString());
}
#endif

/* The next routine is specifically for REQUESTing a STRING needed as a method*/
/* argument.  This raises an error if the object cannot be converted to a     */
/* string value.                                                              */
inline RexxString * REQUIRED_STRING(RexxObject *object, LONG position)
{
  if (object == OREF_NULL)             /* missing argument?                 */
    missing_argument(position);        /* raise an error                    */
                                       /* force to a string value           */
  return object->requiredString(position);
}


/* The next macro is specifically for REQUESTing an ARRAY, since there are    */
/* six primitive classes that can produce array equivalents.  It will trap on */
/* OREF_NULL. */
inline RexxArray * REQUEST_ARRAY(RexxObject *obj) { return ((obj)->requestArray()); }

/* The next macro is specifically for REQUESTing an INTEGER,                  */
inline RexxInteger * REQUEST_INTEGER(RexxObject *obj) { return ((obj)->requestInteger(DEFAULT_DIGITS));}

/* The next macro is specifically for REQUESTing a LONG value                 */
inline long REQUEST_LONG(RexxObject *obj, int precision) { return ((obj)->requestLong(precision)); }

/* The next macro is specifically for REQUESTing an LONG value                */
inline long REQUIRED_LONG(RexxObject *obj, int precision, int position) { return ((obj)->requiredLong(position, precision)); }

/******************************************************************************/
/* Function:  Test for primitive method status of an object                   */
/******************************************************************************/
inline BOOL isPrimitive(RexxObject *object) { return object->behaviour->isPrimitiveBehaviour(); }

/******************************************************************************/
/* Floating-point conversions                                                 */
/******************************************************************************/

void db2st (double source, char *target);
int  st2db (char *source, int length, double *target);
void ln2db (long source, double *target);
BOOL double2Float(double value, float *newValue);

/******************************************************************************/
/* Version number (okver.c)                                                   */
/******************************************************************************/

RexxString *version_number (void);

/******************************************************************************/
/* Memory management macros                                                   */
/******************************************************************************/


#define save(oref)    memoryObject.saveObject((RexxObject*)(oref))
#define discard(oref) memoryObject.discardObject((RexxObject *)(oref))
#define hold(oref)    memoryObject.holdObject((RexxObject *)(oref))
#define discard_hold(oref) memoryObject.discardHoldObject((RexxObject *)(oref))

#define setUpMemoryMark                \
 {                                     \
   long headerMarkedValue = memoryObject.markWord | OldSpaceBit;

#define cleanUpMemoryMark               \
 }

#define setUpMemoryMarkGeneral       {
#define cleanUpMemoryMarkGeneral     }

#define setUpFlatten(type)        \
  {                               \
  long  newSelf = envelope->currentOffset; \
  type *newThis = (type *)this;

#define cleanUpFlatten                    \
 }

#define ObjectNeedsMarking(oref) ((oref) != OREF_NULL && !ObjectIsMarked(oref))
#define memory_mark(oref)  if (ObjectNeedsMarking(oref)) memoryObject.mark((RexxObject *)(oref))
#define memory_mark_general(oref) (memoryObject.markGeneral((RexxObject **)&(oref)))

/* Following macros are for Flattening and unflattening of objects  */
#define flatten_reference(oref,envel)  if (oref) envel->flattenReference((RexxObject **)&newThis, newSelf, (RexxObject **)&(oref))

/******************************************************************************/
/* Typed method invocation macros                                             */
/******************************************************************************/

inline RexxObject * callOperatorMethod(RexxObject *object, LONG methodOffset, RexxObject *argument) {
  PCPPM cppEntry;                      /* kernel method entry point         */

                                       /* get the entry point               */
  cppEntry = object->behaviour->operatorMethods[methodOffset];
                                       /* go issue the method               */
  return (object->*((PCPPM1)cppEntry))(argument);
}

/******************************************************************************/
/* Kernel synchronization and control macros                                  */
/******************************************************************************/

                                       /* macro for entering kernel         */
#define RequestKernelAccess(acti) ((RexxActivity *)acti)->requestKernel()

                                       /* macro for releasing kernel sem    */
#define ReleaseKernelAccess(acti) ((RexxActivity *)acti)->releaseKernel()

/******************************************************************************/
/* Native method and external interface macros                                */
/******************************************************************************/
                                       /* macros for creating methods that  */
                                       /* are part of the native code       */
                                       /* interface.  These are used to     */
                                       /* create directly callable methods  */
                                       /* that can be called from native    */
                                       /* methods (via the RexxNativeAPI.h macros)   */
#define native0(result, name) result REXXENTRY REXX_##name(REXXOBJECT self)
#define native1(result, name, t1, a1) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1)
#define native2(result, name, t1, a1, t2, a2) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2)
#define native3(result, name, t1, a1, t2, a2, t3, a3) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3)
#define native4(result, name, t1, a1, t2, a2, t3, a3, t4, a4) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3, t4 a4)
#define native5(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)
#define native6(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)
#define native7(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7)

#define nativei0(result, name) result REXXENTRY REXX_##name(void)
#define nativei1(result, name, t1, a1) result REXXENTRY REXX_##name(t1 a1)
#define nativei2(result, name, t1, a1, t2, a2) result REXXENTRY REXX_##name(t1 a1, t2 a2)
#define nativei3(result, name, t1, a1, t2, a2, t3, a3) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3)
#define nativei4(result, name, t1, a1, t2, a2, t3, a3, t4, a4) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3, t4 a4)
#define nativei5(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)
#define nativei6(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)
#define nativei7(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7)

                                       /* native method cleanup             */
RexxObject *  native_release(RexxObject *);
                                       /* macro for common native entry     */
#define native_entry  RequestKernelAccess(activity_find())
                                       /* value termination routine         */
#define return_oref(value)  return (REXXOBJECT)native_release(value);
                                       /* return for no value returns       */
#define return_void native_release(OREF_NULL); return;
                                       /* return for non-oref values        */
#define return_value(value) native_release(OREF_NULL); return value;
#endif