/**************************************************************************
 *
 *  Copyright 2022, Yuri Prokushev
 *
 *  This file is part of osFree project
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef sm_h
#define sm_h

#include <smcidl.h>

#ifdef __SOMIDL__

/*
 * For now we don't tell the IR about the internal structures of sm.h:
 * these are not required dynamically, and we have all the information we
 * want from the individual classes:
 * To make the IDL compiler happy though we have to give some types for
 * the following:
 */

typedef void *Entry;
typedef void *Mlist;
typedef void *AttList;
typedef void *NumberList;
typedef void *SwitchBody;

#else
//typedef void *Stab;

#define _DBCS         1
#ifndef __SCLITE__
#define SOMDESCRIPTOR 1
#endif

#if defined(__OS2__) || defined(_WDOS) || defined(__WINDOWS__) || defined(_WIN32)
    #define SMTMP   "\\"
    #define SMSLASH   '\\'
#else
    #define SMTMP   "/tmp"
    #define SMSLASH   '/'
#endif /* __OS2__ || _WDOS ||  _WIN32 ||  */

#ifndef SOMEXTERN
    #ifdef __cplusplus
	#define SOMEXTERN extern "C"
    #else
	#define SOMEXTERN extern
    #endif /* __cplusplus */
#endif /* SOMEXTERN */

#ifndef SOM_BOOLEAN
  #define SOM_BOOLEAN
  typedef unsigned char boolean;
#endif /* SOM_BOOLEAN */

#ifndef SOM_TYPECODE
  #define SOM_TYPECODE
  typedef void * TypeCode;
#endif /* SOM_TYPECODE */

/*
 * Types of symbol table entries.
 * Those wtih _BTYPE are base types,
 * Those with _PTYPE are pre-declare types: such as unions, structs, enums.
 * These are typically for forward references.  It is an error to mention
 * a _PTYPE without some form of pointer reference.
 * ISCONTYPE returns TRUE if its a constructed type.  This is used for
 * typedefs as we can give the same name to a typedef of a constructed type.
 */

#define ISMETHODTYPE(t)  (t == SOMTNewMethodE || t == SOMTOverrideMethodE || \
			  t == SOMTOverriddenMethodE)
#define ISPTYPE(t)       (t == SOMTStructPE || t == SOMTEnumPE || \
			  t == SOMTUnionPE)
#define ISCONTYPE(t)     (t == SOMTStructE || t == SOMTUnionE)
#define ISSTYPE(t)       (t == SOMTStructSE || t == SOMTUnionSE)
#define ISBTYPE(t)       (t == SOMTAnyBE || t == SOMTBooleanBE || \
			  t == SOMTCharBE || t == SOMTDoubleBE || \
			  t == SOMTEnumBE || t == SOMTFloatBE || \
			  t == SOMTLongBE || t == SOMTOctetBE || \
			  t == SOMTShortBE || t == SOMTStringBE || \
			  t == SOMTTypeCodeBE || t == SOMTTypedefBE || \
			  t == SOMTUnsignedLongBE || \
			  t == SOMTUnsignedShortBE ||\
			  t == SOMTVoidPtrBE || t == SOMTVoidBE)

#define SMIN            0x1
#define SMOUT           0x2
#define SMINOUT         0x3


/*
 * OIDL Compatibility.
 */
#define OVERRIDE            SOMTOverrideMethodE
#define NEW                 SOMTNewMethodE
#define ALLMETHODS          SMALLMETHODS_TYPE

#define PUBLIC              0x1
#define PRIVATE             0x2
#define PRIV_OR_PUB         (PRIVATE|PUBLIC)


/*
 * These macros and types are to deal with the different extensions that
 * IDL vs OIDL introduce.
 */
_typedef struct {
    char *ext;
    char *ext_tmp;
    char *public_ext;
    char *private_ext;
    char *throw_away_comment;
    char *header_comment;
    char *precompiler;
    char *output_ext;
    char *comment_ext;
    char *filestem;
} _name(SMEXT);

#define SM_IDL_EXT                      "idl"
#define SM_IDL_TMP_EXT                  "id2"
#define SM_IDL_PUBLIC_EXT               "dl"
#define SM_IDL_PRIVATE_EXT              "pdl"
#define SM_IDL_THROWAWAY_COMMENT        "//# "

#define SM_EXT                          somtext.ext
#define SM_EXT_TMP                      somtext.ext_tmp
#define SM_PUBLIC_EXT                   somtext.public_ext
#define SM_PRIVATE_EXT                  somtext.private_ext
#define SM_THROW_AWAY_COMMENT           somtext.throw_away_comment
#define SM_HEADER_COMMENT               somtext.header_comment
#define SM_PRECOMPILER                  somtext.precompiler
#define SM_OUTPUT_EXT                   somtext.output_ext
#define SM_COMMENT_EXT                  somtext.comment_ext
#define SM_FILE_STEM                    somtext.filestem

#define SM_COMMENT_STYLE_START          1
#define END_OF_COMMENT                  "@#$%!^%&*(~)+_) End of SOM Comment"
#ifdef __RESTRICT_MEMORY__
#define MAX_COMMENT_SIZE                24576
#define HASH_SIZE                         109
#else
#define MAX_COMMENT_SIZE                49152
#define HASH_SIZE                         401
#endif
#define LARGEBUF                         4096

/*
 *  Useful Macros.
 */

#define smmalloc(nbytes)    somtsmalloc (nbytes, FALSE)
#define smcalloc(nbytes)    somtsmalloc (nbytes, TRUE)

/*
 * These structures have been written so that they can be
 * included multiple times for the SOM Compiler, for the Emitter Framework,
 * and for IDL.
 */

_typedef enum SOMTTypes {
    SOMTAnyBE,
    SOMTArgumentE,
    SOMTAttE,
    SOMTBadEntryE,
    SOMTBaseE,
    SOMTBooleanBE,
    SOMTCaseEntryE,
    SOMTCaseListE,
    SOMTCaseSTME,
    SOMTCharBE,
    SOMTClassE,
    SOMTConstE,
    SOMTCopyrightE,
    SOMTDataE,
    SOMTDclListE,
    SOMTDefaultE,
    SOMTDoubleBE,
    SOMTEBaseE,
    SOMTEEnumE,
    SOMTEnumBE,
    SOMTEnumE,
    SOMTEnumPE,
    SOMTFloatBE,
    SOMTGroupE,
    SOMTLongBE,
    SOMTMetaE,
    SOMTModuleE,
    SOMTNegativeBE,
    SOMTNewMethodE,
    SOMTOctetBE,
    SOMTOverriddenMethodE,
    SOMTOverrideMethodE,
    SOMTPassthruE,
    SOMTSequenceE,
    SOMTSequenceTDE,
    SOMTShortBE,
    SOMTStringBE,
    SOMTStringE,
    SOMTStructE,
    SOMTStructPE,
    SOMTStructSE,
    SOMTTyDclE,
    SOMTTypeCodeBE,
    SOMTTypedefBE,
    SOMTTypedefE,
    SOMTUnionE,
    SOMTUnionPE,
    SOMTUnionSE,
    SOMTUnsignedLongBE,
    SOMTUnsignedShortBE,
    SOMTVoidBE,
    SOMTVoidPtrBE,
    SOMTEmitterBeginE,
    SOMTEmitterEndE

} _name(SOMTTypes);


/*
 * Map the basic structures, NOT Classes, onto typenames.
 */
#define _Att             Att
#define _AttList         AttList
#define _CaseEntry       CaseEntry
#define _Class           Class
#define _Const           Const
#define _ConstUnion      ConstUnion
#define _Entry           Entry
#define _EntryUnion      EntryUnion
#define _EnumName        EnumName
#define _Enumerator      Enumerator
#define _Meta            Meta
#define _Method_OR_Data  Method_OR_Data
#define _Mlist           Mlist
#define _Module          Module
#define _NameList        NameList
#define _NumberList      NumberList
#define _Parent          Parent
#define _Passthru        Passthru
#define _Sequence        Sequence
#define _Simple          Simple
#define _String          String
#define _Struct          Struct
#define _SwitchBody      SwitchBody
#define _Typedef         Typedef
#define _Union           Union

/*
 * #define all the framework class types:
 */
#define _SOMTAttributeEntryC      _Entry
#define _SOMTBaseClassEntryC      _Entry
#define _SOMTClassEntryC          _Entry
#define _SOMTCommonEntryC         _Entry
#define _SOMTDataEntryC           _Entry
#define _SOMTEmitC                _Entry
#define _SOMTEntryC               _Entry
#define _SOMTMetaClassEntryC      _Entry
#define _SOMTMethodEntryC         _Entry
#define _SOMTParameterEntryC      _Entry
#define _SOMTPassthruEntryC       _Entry
#define _SOMTStructEntryC         _Entry
#define _SOMTUnionEntryC          _Entry
#define _SOMTEnumEntryC           _Entry
#define _SOMTConstEntryC          _Entry
#define _SOMTSequenceEntryC       _Entry
#define _SOMTStringEntryC         _Entry
#define _SOMTEnumNameEntryC       _Entry
#define _SOMTModuleEntryC         _Entry

/*
 * These too are possible contenders for class types
 * so lets prime them now.
 */
#define _SOMTAttListC            _struct _AttList
#define _SOMTMlistC              _struct _Mlist
#define _SOMTNumberListC         _struct _NumberList
#define _SOMTNameListC           _struct _NameList
#define _SOMTSwitchBodyC         _struct _SwitchBody
#define _SOMTModuleC             _struct _Module

/* 
 * Forward reference the main Entry/OEntry structure.
 */
_typedef struct  _Entry _name(_Entry);

/*
 *  AttList is used for attributes, and release order.
 */
_typedef struct _AttList {
    char *name;                        /* Name of attribute */
    char *value;                       /* Value of attribute */
    _SOMTAttListC *next;               /* Next attribute */
} _name(_AttList);

typedef struct ReleaseOrder {
    char *name;                        /* Name of attribute */
    char *value;                       /* Value of attribute */
    _SOMTAttListC *next;               /* Next attribute */
    /* Same as AttList above for binary compatibility */
    _AttList *staticlist;
    _AttList *protectedlist;
    _AttList *publiclist;
    _AttList *privatelist;
} ReleaseOrder;

/*
 * Mlist is used as a linked list for Entry pointers.
 */
_typedef struct _Mlist {
    _SOMTEntryC *ep;
    _SOMTMlistC *next;
} _name(_Mlist);

/*
 * NumberList is used for lists of Numbers.
 */
_typedef struct _NumberList {
    unsigned long n;
    _SOMTNumberListC *next;
} _name(_NumberList);


/*
 * NameList is used for lists of Names.
 */
_typedef struct _NameList {
    char *name;
    _SOMTNameListC *next;
} _name(_NameList);

_typedef struct _CaseEntry {
    _NameList *labels;                 /* list of case labels */
    _SOMTEntryC *type;                 /* Type of switch */
    _SOMTEntryC *dcl;                  /* Declarator for this type */
} _name(_CaseEntry);

_typedef struct _SwitchBody {
    _CaseEntry *entry;
    _SOMTSwitchBodyC *next;
} _name(_SwitchBody);

_typedef struct _Class {               /* SOMTClassE */
    char *file;                        /* File name of the class */
    _SOMTMetaClassEntryC *meta;        /* Pointer to meta class statement. OIDL
					* Compatibility */
    _SOMTBaseClassEntryC *parent;      /* Pointer to parent class statement.
					* OIDL Compatibility */
    _Mlist *parents;                   /* Pointer to list of parent class
					* statements. OIDL Compatibility */
    _AttList *release;                 /* Release order associated with class */
    _SOMTPassthruEntryC *passthrus;    /* List of passthrus for this class */
    _SOMTDataEntryC *data;             /* List of instance data declarations */
    TypeCode dataTc;                   /* Instance data struct TypeCode */
    short methodcnt;                   /* Number of NEW methods in the class */
    _SOMTMethodEntryC *methods;        /* Pointer to first method */
    _Mlist *inherited;                 /* Pointer to inherited non/overridden
					* methods */
    _Mlist *pubdefs;                   /* Public IDL type definitions i.e.
					* those types that are defined in the
					* "interface" section */

    _SOMTModuleEntryC *mod;            /* This is set to the containing module,
					* Otherwise NULL. */
    _SOMTClassEntryC *cls;             /* If this Class is a meta class, then
					* this points to the Class of which
					* this is a meta class for, otherwise
					* NULL */
    char isforward;                    /* 1 if forward ref, 0 otherwise */
} _name(_Class);

_typedef struct _Meta {                /* SOMTMetaE */
    char *file;                        /* File name of the meta class */
    _SOMTClassEntryC *metadef;         /* Pointer to the meta class defn */
} _name(_Meta);

_typedef struct _Parent {              /* SOMTBaseE */
    _SOMTClassEntryC *parentdef;       /* Pointer to the parent class defn */
} _name(_Parent);

_typedef struct _Passthru {            /* SOMTPassthruE */
    _SOMTPassthruEntryC *next;         /* Pointer to next passthru */
} _name(_Passthru);

_typedef struct _Method_OR_Data {      /* SOMTNewMethodE, SOMTOverrideMethodE,
					* SOMTOverriddenMethodE, SOMTDataE,
					* SOMTArgumentE, SOMTTypedefBE.
					* At present we store all of our
					* information for a METHOD declaration
					* and a DATA declartion in the
					* Method_OR_Data structure.  This is
					* convienient since we use the same
					* parsing code to parse a method
					* prototype as we do a data
					* declaration. In addition, with the
					* exception of omethod and oparent all
					* of the other fields are pontentially
					* used by a data declaration. */
    _SOMTEntryC *eptype;               /* Structured form of the char *type
					* entry. IDL Extension. */
    char *ptrs;                        /* Pointers to type. IDL Extension */
    _NumberList *array;                /* List of array dimensions. IDL
					* Extension. */
    char *arrays;                      /* String version of array dimensions,
					* same information in the NumberList
					* array, but retained for OIDL
					* compatibility. */
    char *defn;                        /* Un-parsed defn of method call. OIDL
					* compatibility. */
    char *type;                        /* [Return] type of method. OIDL
					* compatibility. */
    char varargs;                      /* Method has a varargs parameter */
    char isprivate;                    /* Method is 1=PRIVATE, 0=PUBLIC */
    char inoutmode;                    /* In, out, inout. */
    char isselfref;                    /* If =1 then arg points to self */
    char isoneway;                     /* If =1 then oneway method */
    short argcnt;                      /* Number of arguments */
    _SOMTParameterEntryC *args;        /* Structured list of argument
					* declarations */
    _SOMTMethodEntryC *omethod;        /* If method is overridden the original
					* method defn, otherwise NULL. */
    _SOMTClassEntryC *oparent;         /* If method is overridden the original
					* parent defn, otherwise current
					* parent. */
    _SOMTEntryC *group;                /* Pointer to group method appears in.
					* OIDL Compatibility. */
    _SOMTMethodEntryC *next;           /* Next method/data/arg in class */
    char **ctxsa;                      /* context string literal array */
    _Mlist *raiseslist;                /* list of exceptions */
} _name(_Method_OR_Data);

_typedef struct _Struct {              /* SOMTStructE, SOMTStructSE */
    _Mlist *dcllist;                   /* list of declarators */
    _SOMTClassEntryC *cls;             /* Class defined in for scoping */
    char isexcept;                     /* TRUE if exception, otherwise FALSE */
    char ismutref;                     /* TRUE if mutually-referential, 
					* otherwise FALSE  */
} _name(_Struct);

_typedef struct _Typedef {             /* SOMTTypedefE, SOMTTyDclE */
    _SOMTEntryC *type;
    _Mlist *dcllist;                   /* List of declarators */
    _SOMTEntryC *next;                 /* FIX: Migration for old compiler
					* Should really turn all of our list of
					* Entries into Mlists. */
} _name(_Typedef);

_typedef struct _Union {               /* SOMTUnionE, SOMTUnionSE */
    _SOMTEntryC *type;                 /* Switch type */
    _SwitchBody *swbody;               /* Body of switch */
    char ismutref;                     /* TRUE if mutually-referential, 
					* otherwise FALSE */
} _name(_Union);

_typedef struct _Enumerator {          /* SOMTEnumE */
    _Mlist *names;                     /* list of enumerator names */
} _name(_Enumerator);

_typedef union _ConstUnion _switch(SOMTTypes) {
    _case(SOMTShortBE)
    _case(SOMTLongBE)
    _case(SOMTUnsignedShortBE)
    _case(SOMTCharBE)
    _case(SOMTBooleanBE)
    _case(SOMTEnumBE)
    _case(SOMTUnsignedLongBE)
	unsigned long ul;
    _case(SOMTNegativeBE)
	long l;
    _case(SOMTFloatBE)
    _case(SOMTDoubleBE)
	char *d;                       /* Due to errors in AIX compiler we, use
					  strings for floats and doubles! */
    _case(SOMTStringBE)
	char *s;
} _name(_ConstUnion);

_typedef struct _Const {               /* SOMTConstE */
    _SOMTEntryC *type;                 /* Type of constant */
    char *s;                           /* String value of expression */
    SOMTTypes utype;                   /* Union type - if we had one. */
    _ConstUnion u;                     /* Numeric Value of constant */
} _name(_Const);

_typedef struct _Att {                 /* SOMTAttE */
    char isreadonly;                   /* Read-only or not */
    _SOMTEntryC *type;                 /* Simple type. */
    _Mlist *dcllist;                   /* list of declarators */
    _Mlist *get;                       /* list of get methods  */
    _Mlist *set;                       /* list of set methods  */
} _name(_Att);

_typedef struct _Sequence {            /* SOMTSequenceE, SOMTSequenceTDE */
    _SOMTEntryC *type;                 /* Simple type */
    long n;                            /* positive int const */
    _SOMTEntryC *seqty;                /* Pointer to extra Seq. Typedef */
} _name(_Sequence);

_typedef struct _String {              /* SOMTStringE */
    long n;                            /* length of string */
} _name(_String);

_typedef struct _EnumName {            /* SOMTEnumBE */
    _SOMTEnumEntryC *ep;               /* pointer to ENUM Entry */
    unsigned long value;               /* Value of Enumeration */
} _name(_EnumName);

_typedef struct _Module {              /* SOMTModuleE */
    _Mlist *defs;                      /* list of module definitions */
    _SOMTModuleEntryC *mod;            /* This is set to the containing module,
					* Otherwise NULL. */
    char *file;                        /* File in which Module appears */
} _name(_Module);

/*
 * The union of all of these different types
 */
_typedef union _EntryUnion _switch(SOMTTypes)
{
    _case(SOMTClassE)
	_Class c;
    _case(SOMTMetaE)
	_Meta mt;
    _case(SOMTBaseE)
	_Parent p;
    _case(SOMTPassthruE)
	_Passthru pt;
    _case(SOMTNewMethodE)
    _case(SOMTOverrideMethodE)
    _case(SOMTOverriddenMethodE)
    _case(SOMTDataE)
    _case(SOMTArgumentE)
    _case(SOMTTypedefBE)
    _case(SOMTVoidPtrBE)
	_Method_OR_Data m;
    _case(SOMTStructE)
	_Struct struc;
    _case(SOMTTyDclE)
    _case(SOMTTypedefE)
	_Typedef ty;
    _case(SOMTUnionE)
    _case(SOMTUnionSE)
	_Union un;
    _case(SOMTEnumE)
	_Enumerator enumerator;
    _case(SOMTConstE)
	_Const con;
    _case(SOMTAttE)
	_Att att;
    _case(SOMTSequenceE)
    _case(SOMTSequenceTDE)
	_Sequence seq;
    _case(SOMTStringE)
	_String str;
    _case(SOMTEnumBE)
	_EnumName enumN;
    _case(SOMTModuleE)
	_Module mod;
} _name(_EntryUnion);

/*
 *  Entry is the main data structure for storing classes, methods, passthrus,
 *  groups, and data.
 */

struct _Entry {
    char *name;                        /* Name of entry */
    SOMTTypes type;                    /* SOMTClassE, SMMETHOD_TYPE, etc */
    char **sname;                      /* Scoped name */
    char *comment;                     /* Comment associated with entry */
    _AttList *atts;                    /* List of attributes */
    TypeCode tc;                       /* TypeCode if appropriate, or NULL */
    long lineno;                       /* Line number in source file */
#ifdef SOMTEntryC
    SOMTEntryC *objref;                /* Points to the object containing this
					* entry */
#else
    _SOMTEntryC *objref;               /* Points to the object containing this
					* entry */
#endif
    char isref;                        /* TRUE if this is a reference to the
					* real type, rather than a declaration
					* of it. */
    _EntryUnion u;
#ifdef __PRIVATE__
    RHBelement *data;
    char *filestem;
#endif    
};

/*
 * Attempt to reset the world by #undef as much as possible.
 */
#undef _Att
#undef _AttList
#undef _CaseEntry
#undef _Class
#undef _Const
#undef _ConstUnion
#undef _Entry
#undef _EntryUnion
#undef _EnumName
#undef _Enumerator
#undef _Meta
#undef _Method_OR_Data
#undef _Mlist
#undef _Module
#undef _NameList
#undef _NumberList
#undef _Parent
#undef _Passthru
#undef _SOMTAttListC
#undef _SOMTAttributeEntryC
#undef _SOMTBaseClassEntryC
#undef _SOMTClassEntryC
#undef _SOMTCommonEntryC
#undef _SOMTConstEntryC
#undef _SOMTDataEntryC
#undef _SOMTEmitC
#undef _SOMTEntryC
#undef _SOMTEnumEntryC
#undef _SOMTEnumNameEntryC
#undef _SOMTMetaClassEntryC
#undef _SOMTMethodEntryC
#undef _SOMTMlistC
#undef _SOMTModuleC
#undef _SOMTModuleEntryC
#undef _SOMTNameListC
#undef _SOMTNumberListC
#undef _SOMTParameterEntryC
#undef _SOMTPassthruEntryC
#undef _SOMTSequenceEntryC
#undef _SOMTStringEntryC
#undef _SOMTStructEntryC
#undef _SOMTSwitchBodyC
#undef _SOMTUnionEntryC
#undef _Sequence
#undef _Simple
#undef _String
#undef _Struct
#undef _SwitchBody
#undef _Typedef
#undef _Union

#endif /* __SOMIDL__ */
#endif /* sm_h */
