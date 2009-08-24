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
/* REXX Kernel                                                  ObjectClass.hpp  */
/*                                                                            */
/* Primitive Object Class Definitions                                         */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
#ifndef Included_RexxObject
#define Included_RexxObject

/* default size of the scope table...these are normally very small */
#define DEFAULT_SCOPE_SIZE  6

#define getAttributeIndex 0            /* location of getAttribute method   */
#define setAttributeIndex 1            /* location of setAttribute method   */

  class RexxObject;
  class RexxCompoundTail;
  class RexxCompoundElement;
  class RexxInternalStack;
  class RexxVirtualBase {              /* create first base level class     */
                                       /* dummy virtual function to force   */
                                       /* the virtual function table to a   */
                                       /* specific location.  Different     */
                                       /* compilers place the virtual       */
                                       /* function table pointer at         */
                                       /* different locations.  This forces */
                                       /* to the front location             */
  protected:
     virtual void      baseVirtual() {;}
  };
                                       /* Base Object REXX class            */
  class RexxInternalObject : public RexxVirtualBase{
    public:

     void * operator new(size_t, RexxClass *);
     void * operator new(size_t, RexxClass *, RexxObject **, size_t);
     inline void   operator delete(void *) { ; }
     inline RexxInternalObject() {;};
                                       /* Following constructor used to     */
                                       /*  reconstruct the Virtual          */
                                       /*  Functiosn table.                 */
                                       /* So it doesn't need to do anything */
     inline RexxInternalObject(RESTORETYPE restoreType) { ; };
     virtual ~RexxInternalObject() {;};

     inline operator RexxObject*() { return (RexxObject *)this; };
                                       /* the following are virtual         */
                                       /* functions required for every      */
                                       /* class                             */
     virtual void         live() {;}
     virtual void         liveGeneral() {;}
     virtual void         flatten(RexxEnvelope *) {;}
     virtual RexxObject  *unflatten(RexxEnvelope *) { return (RexxObject *)this; };
     virtual RexxObject  *makeProxy(RexxEnvelope *);
     virtual RexxObject  *copy();
     virtual RexxObject  *evaluate(RexxActivation *, RexxExpressionStack *) { return OREF_NULL; }
     virtual RexxObject  *getValue(RexxActivation *) { return OREF_NULL; }
     virtual RexxObject  *getValue(RexxVariableDictionary *) { return OREF_NULL; }
     virtual void         uninit() {;}
     virtual ULONG        hash()  { return HASHVALUE(this); }
             ULONG        getHashValue()  { return HASHVALUE(this); }

     virtual BOOL         truthValue(LONG);
     virtual RexxString  *makeString();
     virtual void         copyIntoTail(RexxCompoundTail *buffer);
     virtual RexxString  *primitiveMakeString();
     virtual RexxArray   *makeArray();
     virtual RexxString  *stringValue();
     virtual LONG         longValue(size_t);
     virtual RexxInteger *integerValue(size_t);
     virtual double       doubleValue();
     virtual RexxNumberString *numberString();
     virtual LONG         longValueNoNOSTRING(size_t n) { return this->longValue(n);};
     virtual double       doubleValueNoNOSTRING() { return this->doubleValue();};

     virtual BOOL         isEqual(RexxObject *);

             RexxObject  *hasUninit();
             void         removedUninit();
             void         printObject();


     RexxBehaviour *behaviour;         /* the object's behaviour            */
     HEADINFO header;                  /* memory management header          */
                                       /* Following defined as union to     */
                                       /*  Allow for overloading of         */
                                       /*  hashValue usage/value by         */
                                       /*  other classes.                   */
     union {
       long hashvalue;                 /* Default usage.                    */
       size_t u_size;                  /* used by many variable sized Objs  */
//     long lookaside_size;            /* size of the lookaside table -VDICT */
       BEHAVIOURINFO behaviourInfo;    /* Used by RexxBehaviour.            */
       METHODINFO    methodInfo;       /* Used by RexxMethod                */
       size_t      lineNumber;         /* instruction line number info      */
       RexxMethod *u_method;           /* executed method - Activation      */
       RexxString *library;            /* external Library of Method -NMETH */
       RexxSource *u_source;           /* Source code object - RexxMethod   */
       RexxObject *u_receiver;         /* Receiver object - RexxNativeCode  */
       RexxObject *variableValue;      /* value of variable - OKVAR         */
                                       /* name of function/message          */
       RexxString *u_name;             /* Name for Stem Variable - RexxStem */
       RexxString *variableName;       /* retriever of indirect variable    */
       RexxString *stem;               /* name of stem - OTVSTEM            */
       RexxObject *u_left_term;        /* left term of the operator         */
                                       /* variable retriever                */
       RexxVariableBase *variableObject;
     };
  };

class RexxObject : public RexxInternalObject {
  public:
     void * operator new(size_t, RexxClass *);
     void * operator new(size_t, RexxClass *, RexxObject **, size_t);
     void * operator new(size_t size, void *objectPtr) { return objectPtr; };
     void   operator delete(void *);
                                       // Followin are used to create new objects.
                                       // Assumed that the message is sent to a class Object
                                       // These may move to RexxClass in the future......
     RexxObject *newRexx(RexxObject **arguments, size_t argCount);
     RexxObject *newObject() {return new ((RexxClass *)this) RexxObject; };

     operator RexxInternalObject*() { return (RexxInternalObject *)this; };
     inline RexxObject(){;};
                                       /* Following constructor used to   */
                                       /*  reconstruct the Virtual        */
                                       /*  Functiosn table.               */
                                       /* So it doesn't need to do anythin*/
     inline RexxObject(RESTORETYPE restoreType) { ; };
     virtual ~RexxObject(){;};

     virtual RexxMethod  *methodObject(RexxString *);
     virtual RexxObject  *defMethod(RexxString *, RexxMethod *, RexxString *a = OREF_NULL);
     virtual RexxString  *defaultName();
       // Following Virtual functions for SOMProxy polymoprphism
     virtual RexxObject  *unknown(RexxString *msg, RexxArray *args){return OREF_NULL;};
     virtual RexxObject  *initProxy(RexxInteger*somObj) {return OREF_NULL;};
     virtual RexxObject  *freeSOMObj() {return OREF_NULL;};
     virtual RexxObject  *server() {return TheNilObject;};
     virtual RexxObject  *SOMObj() {return TheNilObject;};
     virtual RexxInteger *hasMethod(RexxString *msg);
             BOOL         hasUninitMethod();
     virtual void        *realSOMObject() {return OREF_NULL;};

     RexxObject *init();
     void        uninit();
     void live();
     void liveGeneral();
     void flatten(RexxEnvelope *);
     RexxObject  *copy();
// see ObjectClass.c for defect description
#if defined(REXX_DEBUG)
//     ULONG        hash();
     ULONG        hash() { fprintf(stderr,"*** RexxObject::hash called ***\n"); }
#endif
     BOOL         truthValue(LONG);
     long         longValue(size_t);
     long         longValueNoNOSTRING(size_t);
     RexxNumberString *numberString();
     double       doubleValue();
     double       doubleValueNoNOSTRING();
     RexxInteger *integerValue(size_t);
     RexxString  *makeString();
     RexxString  *primitiveMakeString();
     void         copyIntoTail(RexxCompoundTail *buffer);
     RexxArray   *makeArray();
     RexxString  *stringValue();
     RexxString  *requestString();
     RexxString  *requestStringNoNOSTRING();
     RexxInteger *requestInteger(size_t);
     LONG         requestLong(size_t);
     RexxArray   *requestArray();
     RexxString  *requiredString(LONG);
     RexxInteger *requiredInteger(LONG, size_t);
     LONG         requiredLong(LONG, size_t precision = DEFAULT_DIGITS);
     LONG         requiredPositive(LONG, size_t precision = DEFAULT_DIGITS);
     LONG         requiredNonNegative(LONG, size_t precision=DEFAULT_DIGITS);

     BOOL         isEqual(RexxObject *);
     RexxString  *objectName();
     RexxObject  *objectNameEquals(RexxObject *);
     RexxClass   *classObject();
     RexxObject  *setMethod(RexxString *, RexxMethod *, RexxString *a = OREF_NULL);
     RexxObject  *unsetMethod(RexxString *);
     RexxObject  *requestRexx(RexxString *);
     RexxMessage *start(RexxObject **, size_t);
     RexxEnvelope*startAt(RexxObject **, size_t);
     RexxString  *oref();
     RexxObject  *pmdict();
     RexxObject  *shriekRun(RexxMethod *, RexxString *, RexxString *, RexxObject **, size_t);
     RexxObject  *run(RexxObject **, size_t);

     RexxObject  *messageSend(RexxString *, LONG, RexxObject **);
     RexxObject  *messageSend(RexxString *, LONG, RexxObject **, RexxObject *);
     RexxMethod  *checkPrivate(RexxMethod *);
     RexxObject  *processUnknown(RexxString *, LONG, RexxObject **);
     RexxObject  *processProtectedMethod(RexxString *, LONG, RexxObject **);
     RexxObject  *sendMessage(RexxString *, RexxArray *);
     inline RexxObject  *sendMessage(RexxString *message) { return this->messageSend(message, 0, OREF_NULL); };
     inline RexxObject  *sendMessage(RexxString *message, RexxObject **args, size_t argCount) { return this->messageSend(message, argCount, args); };
     inline RexxObject  *sendMessage(RexxString *message, RexxObject *argument1)
         { return this->messageSend(message, 1, &argument1); }
     RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *);
     RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *);
     RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
     RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);

                                       // Following are internal OREXX methods
     RexxObject  *defMethods(RexxDirectory *);
     void         setObjectVariable(RexxString *, RexxObject *, RexxObject *);
     RexxObject  *getObjectVariable(RexxString *, RexxObject *);
     RexxObject  *setAttribute(RexxObject *);
     RexxObject  *getAttribute();
     void         addObjectVariables(RexxVariableDictionary *);
     void         copyObjectVariables(RexxObject *newObject);
     RexxObject  *superScope(RexxObject *);
     RexxMethod  *superMethod(RexxString *, RexxObject *);
     RexxObject  *mdict();
     RexxObject  *setMdict(RexxObject *);
     inline RexxBehaviour *behaviourObject() { return this->behaviour; }

     short        ptype();
     char        *idString();
     RexxString  *id();
     RexxMethod  *methodLookup(RexxString *name );
     RexxVariableDictionary *getObjectVariables(RexxObject *);
     RexxObject  *equal(RexxObject *);
     RexxInteger *notEqual(RexxObject *other);
     RexxObject  *strictEqual(RexxObject *);
     RexxInteger *strictNotEqual(RexxObject *other);

     RexxString  *stringRexx();
     RexxObject  *makeStringRexx();
     RexxObject  *makeArrayRexx();
     RexxString  *defaultNameRexx();
     RexxObject  *copyRexx();
     RexxObject  *unknownRexx(RexxString *, RexxArray *);
     RexxObject  *hasMethodRexx(RexxString *);
     RexxObject  *initProxyRexx(RexxInteger *);
     RexxObject  *freeSOMObjRexx();
     RexxObject  *SOMObjRexx();
     RexxObject  *serverRexx();
     BOOL         callSecurityManager(RexxString *, RexxDirectory *);

 // Define operator methods here.

   koper  (operator_plus)
   koper  (operator_minus)
   koper  (operator_multiply)
   koper  (operator_divide)
   koper  (operator_integerDivide)
   koper  (operator_remainder)
   koper  (operator_power)
   koper  (operator_abuttal)
   koper  (operator_concat)
   koper  (operator_concatBlank)
   koper  (operator_equal)
   koper  (operator_notEqual)
   koper  (operator_isGreaterThan)
   koper  (operator_isBackslashGreaterThan)
   koper  (operator_isLessThan)
   koper  (operator_isBackslashLessThan)
   koper  (operator_isGreaterOrEqual)
   koper  (operator_isLessOrEqual)
   koper  (operator_strictEqual)
   koper  (operator_strictNotEqual)
   koper  (operator_strictGreaterThan)
   koper  (operator_strictBackslashGreaterThan)
   koper  (operator_strictLessThan)
   koper  (operator_strictBackslashLessThan)
   koper  (operator_strictGreaterOrEqual)
   koper  (operator_strictLessOrEqual)
   koper  (operator_lessThanGreaterThan)
   koper  (operator_greaterThanLessThan)
   koper  (operator_and)
   koper  (operator_or)
   koper  (operator_xor)
   koper  (operator_not)

   RexxVariableDictionary *objectVariables;   /* set of object variables           */
};

class RexxNilObject : public RexxObject {
  public:
     RexxNilObject();
     inline RexxNilObject(RESTORETYPE restoreType) { ; };

     RexxString * nilString();

};

class RexxActivationBase : public RexxInternalObject{
  public:
     inline RexxActivationBase() {;};
     inline RexxActivationBase(RESTORETYPE restoreType) { ; };
     virtual RexxObject  *dispatch() {return NULL;};
     virtual RexxObject  *getReceiver() {return NULL;};
     virtual void traceBack(RexxList *) {;};
     virtual long digits() {return 0;};
     virtual long fuzz() {return 0;};
     virtual BOOL form() {return FALSE;};
     virtual void setDigits(LONG) {;};
     virtual void setFuzz(LONG) {;};
     virtual void setForm(BOOL) {;}
     virtual BOOL trap(RexxString *, RexxDirectory *) {return FALSE;};
     virtual void setObjNotify(RexxMessage *) {;};
     virtual void termination(){;};
     virtual BOOL hasSecurityManager() { return FALSE; }
     virtual BOOL isForwarded() { return FALSE; }
};
#endif