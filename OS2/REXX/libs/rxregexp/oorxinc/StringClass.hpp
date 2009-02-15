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
/* REXX Kernel                                                  StringClass.hpp  */
/*                                                                            */
/* Primitive String Class Definition                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxString
#define Included_RexxString

//#include "NumberStringClass.hpp"
//#include "IntegerClass.hpp"
                                       /* return values from the is_symbol  */
                                       /* validation method                 */
#define  STRING_BAD_VARIABLE   0
#define  STRING_STEM           1
#define  STRING_COMPOUND_NAME  2
#define  STRING_LITERAL        3
#define  STRING_LITERAL_DOT    4
#define  STRING_NUMERIC        5
#define  STRING_NAME           6

#define  STRING_HASLOWER       0x01    /* string does contain lowercase     */
#define  STRING_NOLOWER        0x02    /* string does not contain lowercase */
#define  STRING_NONNUMERIC     0x04    /* string is non-numeric             */
#define  STRING_NODBCS         0x08    /* string has no DBCS characters     */

#define  NoDBCS(r)                     (((RexxString *)r)->Attributes&STRING_NODBCS)

#define  INITIAL_NAME_SIZE     10      /* first name table allocation       */
#define  EXTENDED_NAME_SIZE    10      /* amount to extend table by         */

class RexxStringClass : public RexxClass {
 public:
   RexxStringClass(RESTORETYPE restoreType) { ; };
   void *operator new(size_t size, void *ptr) {return ptr;};
   RexxString *newString(const PCHAR, size_t);
   RexxString *rawString(size_t);
   RexxString *newCstring(const PCHAR);
   RexxString *newDouble(PDBL);
   RexxString *newProxy(const PCHAR);
   RexxString *newRexx(RexxObject **, size_t);
};

 class RexxString : public RexxObject {
  public:
   inline void       *operator new(size_t size, void *ptr){return ptr;};
   inline RexxString() {;} ;
   inline RexxString(RESTORETYPE restoreType) { ; };

   void        live();
   void        liveGeneral();
   void        flatten(RexxEnvelope *envelope);
   RexxObject *unflatten(RexxEnvelope *);

   ULONG       hash();
   long        longValue(size_t);
   RexxNumberString *numberString();
   double      doubleValue();
   RexxInteger *integerValue(size_t);
   RexxString  *makeString();
   RexxString  *primitiveMakeString();
   void         copyIntoTail(RexxCompoundTail *buffer);
   RexxString  *stringValue();
   BOOL         truthValue(LONG);

   BOOL        isEqual(RexxObject *);
   BOOL        primitiveIsEqual(RexxObject *);
   long        strictComp(RexxObject *);
   long        comp(RexxObject *);
   RexxInteger *equal(RexxObject *);
   RexxInteger *strictEqual(RexxObject *);
   RexxInteger *notEqual(RexxObject *);
   RexxInteger *strictNotEqual(RexxObject *);
   RexxInteger *isGreaterThan(RexxObject *);
   RexxInteger *isLessThan(RexxObject *);
   RexxInteger *isGreaterOrEqual(RexxObject *);
   RexxInteger *isLessOrEqual(RexxObject *);
   RexxInteger *strictGreaterThan(RexxObject *);
   RexxInteger *strictLessThan(RexxObject *);
   RexxInteger *strictGreaterOrEqual(RexxObject *);
   RexxInteger *strictLessOrEqual(RexxObject *);

   size_t      get(size_t, PCHAR, size_t);
   RexxObject *lengthRexx();
   RexxString *concatRexx(RexxObject *);
   RexxString *concat(RexxString *);
   RexxString *concatToCstring(PCHAR);
   RexxString *concatWithCstring(PCHAR);
   RexxString *concatBlank(RexxObject *);
   BOOL        checkLower();
   RexxString *upper();
   RexxString *stringTrace();
   RexxString *lower();
   void        setNumberString(RexxObject *);
   RexxString *concatWith(RexxString *, char);

   RexxObject *plus(RexxObject *right);
   RexxObject *minus(RexxObject *right);
   RexxObject *multiply(RexxObject *right);
   RexxObject *divide(RexxObject *right);
   RexxObject *integerDivide(RexxObject *right);
   RexxObject *remainder(RexxObject *right);
   RexxObject *power(RexxObject *right);
   RexxObject *abs();
   RexxObject *sign();
   RexxObject *notOp();
   RexxObject *operatorNot(RexxObject *);
   RexxObject *andOp(RexxObject *);
   RexxObject *orOp(RexxObject *);
   RexxObject *xorOp(RexxObject *);
   RexxObject *Max(RexxObject **args, size_t argCount);
   RexxObject *Min(RexxObject **args, size_t argCount);
   RexxObject *trunc(RexxInteger *decimals);
   RexxObject *format(RexxObject *Integers, RexxObject *Decimals, RexxObject *MathExp, RexxObject *ExpTrigger);
   RexxObject *isInteger();
   RexxObject *logicalOperation(RexxObject *, RexxObject *, UINT);
   RexxString *extract(size_t offset, size_t sublength) { return new_string(this->stringData + offset, sublength); }
   RexxObject *evaluate(RexxActivation *, RexxExpressionStack *);
   RexxObject *getValue(RexxActivation *context) {return this;}
   RexxObject *getValue(RexxVariableDictionary *context) {return this;}
                                       /* the following methods are in    */
                                       /* OKBSUBS                         */
   RexxString  *center(RexxInteger *, RexxString *);
   RexxString  *delstr(RexxInteger *, RexxInteger *);
   RexxString  *insert(RexxString *, RexxInteger *, RexxInteger *, RexxString *);
   RexxString  *left(RexxInteger *, RexxString *);
   RexxString  *overlay(RexxString *, RexxInteger *, RexxInteger *, RexxString *);
   RexxString  *reverse();
   RexxString  *right(RexxInteger *, RexxString *);
   RexxString  *strip(RexxString *, RexxString *);
   RexxString  *substr(RexxInteger *, RexxInteger *, RexxString *);
   RexxString  *delWord(RexxInteger *, RexxInteger *);
   RexxString  *space(RexxInteger *, RexxString *);
   RexxString  *subWord(RexxInteger *, RexxInteger *);
   RexxString  *word(RexxInteger *);
   RexxInteger *wordIndex(RexxInteger *);
   RexxInteger *wordLength(RexxInteger *);
   RexxInteger *wordPos(RexxString *, RexxInteger *);
   RexxInteger *words();
                                       /* the following methods are in    */
                                       /* OKBMISC                         */
   RexxString  *changeStr(RexxString *, RexxString *);
   RexxInteger *abbrev(RexxString *, RexxInteger *);
   RexxInteger *compare(RexxString *, RexxString *);
   RexxString  *copies(RexxInteger *);
   RexxObject  *dataType(RexxString *);
   RexxInteger *lastPos(RexxString *, RexxInteger *);
   RexxInteger *posRexx(RexxString *, RexxInteger *);
   RexxString  *translate(RexxString *, RexxString *, RexxString *);
   RexxInteger *verify(RexxString *, RexxString *, RexxInteger *);
   RexxInteger *countStrRexx(RexxString *);
   size_t       countStr(RexxString *);
                                       /* the following methods are in    */
                                       /* OKBBITS                         */
   RexxString  *bitAnd(RexxString *, RexxString *);
   RexxString  *bitOr(RexxString *, RexxString *);
   RexxString  *bitXor(RexxString *, RexxString *);
                                       /* the following methods are in    */
                                       /* OKBCONV                         */
   RexxString  *b2x();
   RexxString  *c2d(RexxInteger *);
   RexxString  *c2x();
   RexxString  *d2c(RexxInteger *);
   RexxString  *d2x(RexxInteger *);
   RexxString  *x2b();
   RexxString  *x2c();
   RexxString  *x2d(RexxInteger *);
   RexxString  *x2dC2d(RexxInteger *, BOOL);

   RexxArray   *makeArray(RexxString *);

/****************************************************************************/
/*                                                                          */
/*      RexxString Methods in OKBMISC.C                                     */
/*                                                                          */
/****************************************************************************/
   int         isSymbol();
   size_t      pos(RexxString *, size_t);
   size_t      caselessPos(RexxString *, size_t);

   size_t      validDBCS();
   RexxString  *DBCSreverse();
   RexxString  *DBCSsubstr(RexxInteger *, RexxInteger *, RexxString *);
   RexxString  *DBCSdelstr(RexxInteger *, RexxInteger *);
   RexxString  *DBCSsubWord(RexxInteger *, RexxInteger *);
   RexxString  *DBCSdelWord(RexxInteger *, RexxInteger *);
   RexxString  *DBCSstrip(RexxString *, RexxString *);
   RexxInteger *DBCSlength();
   RexxInteger *DBCSwordPos(RexxString *, RexxInteger *);
   RexxInteger *DBCSdatatype(INT);
   RexxInteger *DBCScompare(RexxString *, RexxString *);
   RexxString  *DBCScopies(RexxInteger *);
   RexxInteger *DBCSabbrev(RexxString *, RexxInteger *);
   RexxString  *DBCSspace(RexxInteger *, RexxString *);
   RexxString  *DBCSleft(RexxInteger *, RexxString *);
   RexxString  *DBCSright(RexxInteger *, RexxString *);
   RexxString  *DBCScenter(RexxInteger *, RexxString *);
   RexxString  *DBCSinsert(RexxString *, RexxInteger *, RexxInteger *, RexxString *);
   RexxString  *DBCSoverlay(RexxString *, RexxInteger *, RexxInteger *, RexxString *);
   RexxInteger *DBCSverify(RexxString *, RexxString *, RexxInteger *);
   RexxInteger *DBCSwords();
   size_t       DBCSpos(RexxString *, size_t);
   size_t       DBCScaselessPos(RexxString *, size_t);
   RexxInteger *DBCSlastPos(RexxString *, RexxInteger *);
   RexxInteger *DBCSwordIndex(RexxInteger *);
   RexxInteger *DBCSwordLength(RexxInteger *);
   RexxString  *DBCSword(RexxInteger *);
   RexxString  *DBCStranslate(RexxString *, RexxString *, RexxString *);
   RexxString  *dbLeft(RexxInteger *, RexxString *, RexxString *);
   RexxString  *dbRight(RexxInteger *, RexxString *, RexxString *);
   RexxString  *dbCenter(RexxInteger *, RexxString *, RexxString *);
   RexxString  *dbRleft(RexxInteger *, RexxString *);
   RexxString  *dbRright(RexxInteger *, RexxString *);
   RexxString  *dbToDbcs();
   RexxString  *dbToSbcs();
   RexxInteger *dbValidate(RexxString *);
   RexxInteger *dbWidth(RexxString *);
   RexxString  *dbAdjust(RexxString *);
   RexxString  *dbBracket();
   RexxString  *dbUnBracket();
   int          DBCSstringCompare(RexxString *);
   size_t       DBCSmovePointer(size_t, INT, size_t);



/* Inline_functions */

   inline size_t  getLength() { return this->length; };
   inline PCHAR getStringData() { return this->stringData; };
   inline void put(size_t s, const void *b, int l) { memcpy((this->stringData+s),b,(size_t)l); };
   inline void set(size_t s,int c,int l) { memset((this->stringData+s),c,(size_t)l); };
   inline char getChar(size_t p) { return *(this->stringData+p); };
   inline char putChar(size_t p,char c) { return *(this->stringData+p) = c; };
   inline UCHAR upperOnly() {return (UCHAR)this->Attributes&STRING_NOLOWER;};
   inline UCHAR hasLower() {return (UCHAR)this->Attributes&STRING_HASLOWER;};
   inline UCHAR setUpperOnly() {return (UCHAR)(this->Attributes |= STRING_NOLOWER);};
   inline UCHAR setHasLower() {return (UCHAR)(this->Attributes |= STRING_HASLOWER);};
   inline UCHAR nonNumeric() {return (UCHAR)(this->Attributes&STRING_NONNUMERIC);};
   inline UCHAR setNonNumeric() {return (UCHAR)(this->Attributes |= STRING_NONNUMERIC);};
   inline BOOL  strCompare(PCHAR s) {return this->memCompare((s), strlen(s));};
   inline BOOL  strICompare(PCHAR s) { return (size_t)this->length == strlen(s) && stricmp(s, this->stringData) == 0;}
   inline BOOL  memCompare(PCHAR s, size_t l) { return l == this->length && !memcmp(s, this->stringData, l); }
   inline BOOL  memCompare(RexxString *other) { return other->length == this->length && !memcmp(other->stringData, this->stringData, length); }
   inline void  memCopy(PCHAR s) { memcpy(s, stringData, length); }
   inline void  generateHash() {
                                       /* the following logic is duplicated */
                                       /* in the operator new function.  Any*/
                                       /* changes to the hashing must be    */
                                       /* reflected in both locations       */
     if (this->length == 0)            /* nullstring?                       */
       this->hashvalue = 1;            /* use 1 for the hash value          */
                                       /* got a long string?                */
     else if (this->length >= sizeof(this->hashvalue))
                                       /* just pick up the first 4 bytes and*/
                                       /* add in the length                 */
       this->hashvalue = *((PLONG)this->stringData) + this->length + this->stringData[this->length-1];
     else
                                       /* just pick up the first 2 bytes and*/
                                       /* add in the length (this may pick  */
                                       /* up the trailing NULL if only one  */
                                       /* character long                    */
       this->hashvalue = *((PSHORT)this->stringData) + this->length + this->stringData[this->length-1];
   }

   RexxNumberString *createNumberString();

   inline RexxNumberString *fastNumberString() {
       if (this->nonNumeric())              /* Did we already try and convert to */
                                            /* to a numberstring and fail?       */
        return OREF_NULL;                   /* Yes, no need to try agian.        */

       if (this->NumberString != OREF_NULL) /* see if we have already converted  */
         return this->NumberString;         /* return the numberString Object.   */
       return createNumberString();         /* go build the number string version */
   }

   inline INT sortCompare(RexxString *other) {
       size_t compareLength = length;
       if (compareLength > other->length) {
           compareLength = other->length;
       }
       INT result = memcmp(stringData, other->stringData, compareLength);
       if (result == 0) {
           if (length > other->length) {
               result = 1;
           }
           else if (length < other->length) {
               result = -1;
           }
       }
       return result;
   }

   inline INT sortCaselessCompare(RexxString *other) {
       size_t compareLength = length;
       if (compareLength > other->length) {
           compareLength = other->length;
       }
       INT result = memicmp(stringData, other->stringData, compareLength);
       if (result == 0) {
           if (length > other->length) {
               result = 1;
           }
           else if (length < other->length) {
               result = -1;
           }
       }
       return result;
   }

   inline INT sortCompare(RexxString *other, size_t startCol, size_t colLength) {
       int result = 0;
       if ((startCol < length ) && (startCol < other->length)) {
           size_t stringLength = length;
           if (stringLength > other->length) {
               stringLength = other->length;
           }
           stringLength = stringLength - startCol + 1;
           size_t compareLength = colLength;
           if (compareLength > stringLength) {
               compareLength = stringLength;
           }

           result = memcmp(stringData + startCol, other->stringData + startCol, compareLength);
           if (result == 0 && stringLength < colLength) {
               if (length > other->length) {
                   result = 1;
               }
               else if (length < other->length) {
                   result = -1;
               }
           }
       }
       else {
           if (length == other->length) {
               result = 0;
           }
           else {
               result = length < other->length ? -1 : 1;
           }
       }
       return result;
   }

   inline INT sortCaselessCompare(RexxString *other, size_t startCol, size_t colLength) {
       INT result = 0;
       if ((startCol < length ) && (startCol < other->length)) {
           size_t stringLength = length;
           if (stringLength > other->length) {
               stringLength = other->length;
           }
           stringLength = stringLength - startCol + 1;
           size_t compareLength = colLength;
           if (compareLength > stringLength) {
               compareLength = stringLength;
           }

           result = memicmp(stringData + startCol, other->stringData + startCol, compareLength);
           if (result == 0 && stringLength < colLength) {
               if (length > other->length) {
                   result = 1;
               }
               else if (length < other->length) {
                   result = -1;
               }
           }
       }
       else {
           if (length == other->length) {
               result = 0;
           }
           else {
               result = length < other->length ? -1 : 1;
           }
       }
       return result;
   }

   size_t length;                      /* string length                   */
   RexxNumberString *NumberString;     /* lookaside information           */
   ULONG Attributes;                   /* string attributes               */
   char stringData[4];                 /* Start of the string data part   */
 };
#endif