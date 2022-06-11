{**********************************************************************

    Copyright (c) 1994-1996 by International Business Machines Corporation
    Copyright (c) 1997 Antony T Curtis.
    Copyright (c) 2002-2007 by Yuri Prokushev (prokushev@ecomstation.ru)

    System Object Model Run-time library API (SOM.DLL)

 **********************************************************************}

Unit SOM;

{$IFDEF VIRTUALPASCAL}
  {&Cdecl+}
{$ENDIF}

{$IFDEF FPC}
  {$MODE OBJFPC}
  {$WARNING This code is alpha!}
{$ENDIF}

Interface

type
  TString = PChar;
  TLong = Longint;
  PLong = ^Longint;
  TOctet = Byte;
  POctet = ^Byte;
  TBoolean = Boolean;
  PBoolean = ^TBoolean;
  Tva_list = PChar;
  TDouble = Double;

var
  SOM_MajorVersionPtr: ^Longint;       (*  SOM Version Numbers  *)
  SOM_MinorVersionPtr: ^Longint;       (*  SOM Version Numbers  *)

  SOM_MaxThreadsPtr: ^Longint;         (*  SOM Thread Support   *)

type
  TFlags                    = Longint;

type
  TRealSOMObject                = Pointer;
  TRealSOMClass                 = Pointer;
  TSOMMSingleInstanceType       = Pointer;
  TRealSOMClassMgr              = Pointer;
  PRealSOMClass                 = ^TRealSOMClass;
  PRealSOMObject                = ^TRealSOMObject;
  TCORBAObjectType              = TRealSOMObject;    (* in SOM, a CORBA object is a SOM object *)

  TsomToken             = Pointer;           (* Uninterpretted value   *)
  TsomId                = ^PChar;
  PsomId                = ^TsomId;
  PsomToken             = ^TsomToken;         (* Uninterpretted value   *)

  TsomMToken            = TsomToken;
  TsomDToken            = TsomToken;
  PsomMToken            = ^TsomMToken;
  PsomDToken            = ^TsomDToken;

type
  TImplId               = ^PChar;
  TRepositoryId         = PChar;
  TAttributeDef_AttributeMode  = Cardinal;
  TOperationDef_OperationMode  = Longint;
  TParameterDef_ParameterMode  = Cardinal;

  PsomMethod            = Pointer;
  TsomBooleanVector     = ^Byte;
  TsomCtrlInfo          = TsomToken;

  TsomSharedMethodData  = TsomToken;
  PsomSharedMethodData  = ^TsomSharedMethodData;

  PsomClassInfo         = ^TsomClassInfo;
  TsomClassInfo         = TsomToken;


  TIdentifier           = PChar;         (* CORBA 7.5.1, p. 129 *)

  TTypeCode             = pointer;

(* CORBA 5.7, p.89 *)
  Tany                  = record
    _type  : TTypeCode;
    _value : Pointer;
  end;

  TNamedValue           = record
    name       : TIdentifier;
    argument   : Tany;
    len        : Longint;
    arg_modes  : TFlags;
  end;

(* -- Method/Data Tokens -- For locating methods and data members. *)

  TsomRdAppType  = LongInt;       (* method signature code -- see def below *)
  TsomFloatMap   = Array [0..13] of LongInt; (* float map -- see def below *)
  PsomFloatMap   = ^TsomFloatMap;

  TsomMethodInfoStruct  = record
    callType    : TsomRdAppType;
    va_listSize : Longint;
    float_map   : PsomFloatMap;
  end;

  TsomMethodInfo  = TsomMethodInfoStruct;
  PsomMethodInfo  = ^TsomMethodInfo;

  TsomMethodDataStruct   = record
    id         : TsomId;
    ctype      : Longint;             (* 0=static, 1=dynamic 2=nonstatic *)
    descriptor : TsomId;              (* for use with IR interfaces *)
    mToken     : TsomMToken;          (* NULL for dynamic methods *)
    method     : PsomMethod;          (* depends on resolution context *)
    shared     : PsomSharedMethodData;
  end;

  TsomMethodData  = TsomMethodDataStruct;
  PsomMethodData  = ^TsomMethodDataStruct;

  TsomMethodProc  = procedure(somSelf:TRealSOMObject);
  PsomMethodProc  = ^TsomMethodProc;


(*---------------------------------------------------------------------
 * C++-style constructors are called initializers in SOM. Initializers
 * are methods that receive a pointer to a somCtrlStruct as an argument.
 *)

  TsomInitInfo          = record
    cls                  : TRealSOMClass;(* the class whose introduced data is to be initialized *)
    defaultInit          : TsomMethodProc;
    defaultCopyInit      : TsomMethodProc;
    defaultConstCopyInit : TsomMethodProc;
    defaultNCArgCopyInit : TsomMethodProc;
    dataOffset           : Longint;
    legacyInit           : TsomMethodProc;
  end;

  TsomDestructInfo      = record
    cls             : TRealSOMClass;(* the class whose introduced data is to be destroyed *)
    defaultDestruct : TsomMethodProc;
    dataOffset      : Longint;
    legacyUninit    : TsomMethodProc;
  end;

  TsomAssignInfo        = record
    cls                : TRealSOMClass;(* the class whose introduced data is to be assigned *)
    defaultAssign      : TsomMethodProc;
    defaultConstAssign : TsomMethodProc;
    defaultNCArgAssign : TsomMethodProc;
    udaAssign          : TsomMethodProc;
    udaConstAssign     : TsomMethodProc;
    dataOffset         : Longint;
  end;

  T_IDL_SEQUENCE_octet  = record
    _maximum : Cardinal;
    _length  : Cardinal;
    _buffer  : ^Byte;
  end;
  TReferenceData         = T_IDL_SEQUENCE_octet;

(*
 * A special info access structure pointed to by
 * the parentMtab entry of somCClassDataStructure.
 *)
  TsomTD_somRenewNoInitNoZeroThunk      = procedure(var buf); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

  PsomInitInfo          = ^TsomInitInfo;

  TsomInitCtrlStruct    = record
    mask     : TsomBooleanVector;(* an array of booleans to control ancestor calls *)
    info     : PsomInitInfo;  (* an array of structs *)
    infoSize : Longint;         (* increment for info access *)
    ctrlInfo : TsomCtrlInfo;
  end;
  TsomInitCtrl           = TsomInitCtrlStruct;
  Tsom3InitCtrl          = TsomInitCtrlStruct;
  Psom3InitCtrl          = ^Tsom3InitCtrl;

  PsomDestructInfo    = ^TsomDestructInfo;
  TsomDestructCtrlStruct = record
    mask     : TsomBooleanVector;(* an array of booleans to control ancestor calls *)
    info     : PsomDestructInfo;(* an array of structs *)
    infoSize : Longint;         (* increment for info access *)
    ctrlInfo : TsomCtrlInfo;
  end;
  TsomDestructCtrl       = TsomDestructCtrlStruct;
  Tsom3DestructCtrl      = TsomDestructCtrlStruct;
  Psom3DestructCtrl    = ^TsomDestructCtrlStruct;

  TsomAssignInfoPtr     = ^TsomAssignInfo;
  TsomAssignCtrlStruct  = record
    mask     : TsomBooleanVector; (* an array of booleans to control ancestor calls *)
    info     : TsomAssignInfoPtr; (* an array of structs *)
    infoSize : Longint;           (* increment for info access *)
    ctrlInfo : TsomCtrlInfo;
  end;
  TsomAssignCtrl         = TsomAssignCtrlStruct;
  Tsom3AssignCtrl        = TsomAssignCtrlStruct;
  Psom3AssignCtrl        = ^TsomAssignCtrlStruct;

(*----------------------------------------------------------------------
 *  The Class Data Structures -- these are used to implement static
 *  method and data interfaces to SOM objects.
 *)

type
(* -- (Generic) Class data Structure *)
  TsomClassDataStructure =record
    classObject : TRealSOMClass;                 (* changed by shadowing *)
    tokens      : Array[0..0] of TsomToken;      (* method tokens, etc. *)
  end;
  TsomClassDataStructurePtr = ^TsomClassDataStructure;

  TsomInitCtrlPtr        = ^TsomInitCtrl;
  TsomDestructCtrlPtr    = ^TsomDestructCtrl;
  TsomAssignCtrlPtr      = ^TsomAssignCtrl;

(* -- For building lists of method tables *)
  TsomMethodTabPtr       = ^TsomMethodTab;

  TsomMethodTabs         = ^TsomMethodTabList;
  TsomMethodTabList      = record
    mtab: TsomMethodTabPtr;
    next: TsomMethodTabs;
  end;

  TsomParentMtabStruct   = record
    mtab                : TsomMethodTabPtr;       (* this class' mtab -- changed by shadowing *)
    next                : TsomMethodTabs;         (* the parent mtabs -- unchanged by shadowing *)
    classObject         : TRealSOMClass;          (* unchanged by shadowing *)
    somRenewNoInitNoZeroThunk: TsomTD_somRenewNoInitNoZeroThunk; (* changed by shadowing *)
    instanceSize        : Longint;               (* changed by shadowing *)
    initializers        : PsomMethodProc;      (* resolved initializer array in releaseorder *)
    resolvedMTokens     : PsomMethodProc;      (* resolved methods *)
    initCtrl            : TsomInitCtrl;           (* these fields are filled in if somDTSClass&2 is on *)
    destructCtrl        : TsomDestructCtrl;
    assignCtrl          : TsomAssignCtrl;
    embeddedTotalCount  : Longint;
    hierarchyTotalCount : Longint;
    unused              : Longint;
  end;
  TsomParentMtabStructPtr = ^TsomParentMtabStruct;

(*
 * (Generic) Auxiliary Class Data Structure
 *)
  TsomCClassDataStructure = record
    parentMtab          : TsomParentMtabStructPtr;
    instanceDataToken   : TsomDToken;
    wrappers            : Array[0..0] of TsomMethodProc;  (* for valist methods *)
  end;
  TsomCClassDataStructurePtr = ^TsomCClassDataStructure;


(*----------------------------------------------------------------------
 *  The Method Table Structure
 *)

(* -- to specify an embedded object (or array of objects). *)
  TsomEmbeddedObjStructPtr = ^TsomEmbeddedObjStruct;
  TsomEmbeddedObjStruct  = record
    copp  : TRealSOMClass;  (* address of class of object ptr *)
    cnt   : Longint;       (* object count *)
    offset: Longint;       (* Offset to pointer (to embedded objs) *)
  end;

  TsomMethodTabStruct    = record
    classObject         : TRealSOMClass;
    classInfo           : PsomClassInfo;
    className           : PChar;
    instanceSize        : Longint;
    dataAlignment       : Longint;
    mtabSize            : Longint;
    protectedDataOffset : Longint;       (* from class's introduced data *)
    protectedDataToken  : TsomDToken;
    embeddedObjs        : TsomEmbeddedObjStructPtr;
    (* remaining structure is opaque *)
    entries             : Array[0..0] of TsomMethodProc;
  end;
  TsomMethodTab          = TsomMethodTabStruct;

(* -- For building lists of class objects *)
  TsomClasses           = ^TsomClassList;
  TsomClassList         = record
    cls : TRealSOMClass;
    next: TsomClasses;
  end;

(* -- For building lists of objects *)
  TsomObjects            = ^TsomObjectList;
  TsomObjectList         = record
    obj : TRealSOMObject;
    next: TsomObjects;
  end;



(*----------------------------------------------------------------------
 * Method Stubs -- Signature Support
 *
 *
 * This section defines the structures used to pass method signature
 * ingo to the runtime. This supports selection of generic apply stubs
 * and runtime generation of redispatchstubs when these are needed. The
 * information is registered with the runtime when methods are defined.
 *
 * When calling somAddStaticMethod, if the redispatchStub is -1, then a
 * pointer to a struct of type somApRdInfo is passed as the applyStub.
 * Otherwise, the passed redispatchstub and applystub are taken as given.
 * When calling somAddDynamicMethod, an actual apply stub must be passed.
 * Redispatch stubs for dynamic methods are not available, nor is
 * automated support for dynamic method apply stubs. The following
 * atructures only appropriate in relation to static methods.
 *
 * In SOMr2, somAddStaticMethod can be called with an actual redispatchstub
 * and applystub *ONLY* if the method doesn't return a structure. Recall
 * that no SOMr1 methods returned structures, so SOMr1 binaries obey this
 * restriction. The reason for this rule is that SOMr2 *may* use thunks,
 * and thunks need to know if a structure is returned. We therefore assume
 * that if no signature information is provided for a method through the
 * somAddStaticMethod interface, then the method returns a scalar.
 *
 * If a structure is returned, then a -1 *must* be passed to
 * somAddStaticMethod as a redispatchstub. In any case, if a -1 is passed,
 * then this means that the applystub actually points to a structure of type
 * somApRdInfo. This structure is used to hold and access signature
 * information encoded as follows.
 *
 * If the somApRdInfo pointer is NULL, then, if the runtime was built with
 * SOM_METHOD_STUBS defined, a default signature is assumed (no arguments,
 * and no structure returned); otherwise, the stubs are taken as
 * somDefaultMethod (which produces a runtime error when used) if dynamic
 * stubs are not available.
 *
 * If the somApRdInfo pointer is not NULL, then the structure it points to can
 * either include (non-null) redispatch and applystubs (the method is then
 * assumed to return a structure), or null stubs followed by information needed
 * to generate necessary stubs dynamically.
 *)

  TsomApRdInfoStruct     = record
    rdStub  : TsomMethodProc;
    apStub  : TsomMethodProc;
    stubInfo: PsomMethodInfo;
  end;
  TsomApRdInfo           = TsomApRdInfoStruct;


(*
 * Values for somRdAppType are generated by summing one from column A and one
 * from column B of the following constants:
 *)
(* Column A: return type *)
const
  SOMRdRetsimple        = 0; (* Return type is a non-float fullword *)
  SOMRdRetfloat         = 2; (* Return type is (single) float *)
  SOMRdRetdouble        = 4; (* Return type is double *)
  SOMRdRetlongdouble    = 6; (* Return type is long double *)
  SOMRdRetaggregate     = 8; (* Return type is struct or union *)
  SOMRdRetbyte          =10; (* Return type is a byte *)
  SOMRdRethalf          =12; (* Return type is a (2 byte) halfword *)
(* Column B: are there any floating point scalar arguments? *)
  SOMRdNoFloatArgs      = 0;
  SOMRdFloatArgs        = 1;

(* A somFloatMap is only needed on RS/6000 *)
(*
 * This is an array of offsets for up to the first 13 floating point arguments.
 * If there are fewer than 13 floating point arguments, then there will be
 * zero entries following the non-zero entries which represent the float args.
 * A non-zero entry signals either a single- or a double-precision floating point
 * argument. For a double-precision argument, the entry is the stack
 * frame offset. For a single-precision argument the entry is the stack
 * frame offset + 1. For the final floating point argument, add 2 to the
 * code that would otherwise be used.
 *)
  SOMFMSingle           = 1; (* add to indicate single-precision *)
  SOMFMLast             = 2; (* add to indicate last floating point arg *)

const
  SOM_SCILEVEL          = 4;


(* The SCI includes the following information:
 *
 * The address of a class's ClassData structure is passed.
 * This structure should have the external name,
 * <className>ClassData. The classObject field should be NULL
 * (if it is not NULL, then a new class will not be built). somBuildClass will
 * set this field to the address of the new class object when it is built.
 *
 * The address of the class's auxiliary ClassData structure is passed.
 * Thi structure should have the external name,
 * <className>CClassData. The parentMtab field will be set by somBuildClass.
 * This field often allows method calls to a class object to be avoided.
 *
 * The other structures referenced by the static class information (SCI)
 * are used to:
 *)

(*
 * to specify a static method. The methodId used here must be
 * a simple name (i.e., no colons). In all other cases,
 * where a somId is used to identify a registered method,
 * the somId can include explicit scoping. An explicitly-scoped
 * method name is called a method descriptor. For example,
 * the method introduced by TSOMObject as somGetClass has the
 * method descriptor "TSOMObject::somGetClass". When a
 * class is contained in an IDL module, the descriptor syntax
 * <moduleName>::<className>::<methodName> can be used. Method
 * descriptors can be useful when a class supports different methods
 * that have the same name (note: IDL prevents this from occuring
 * statically, but SOM itself has no problems with this).
 *)

type
  TsomStaticMethodStruct = record
    classData        : PsomMToken;
    methodId         : PsomId;        (* this must be a simple name (no colons) *)
    methodDescriptor : PsomId;
    method           : PsomMethod;//somMethodProc;
    redispatchStub   : PsomMethod;//somMethodProc;
    applyStub        : PsomMethod;//somMethodProc;
  end;
  TsomStaticMethod_t     = TsomStaticMethodStruct;
  TsomStaticMethod_p     = ^TsomStaticMethod_t;

(* to specify an overridden method *)
  TsomOverideMethodStruct = record
    methodId : PsomId;        (* this can be a method descriptor *)
    method   : PsomMethod;//somMethodProc;
  end;
  TsomOverrideMethod_t   = TsomOverideMethodStruct;
  TsomOverrideMethod_p   = ^TsomOverrideMethod_t;

(* to inherit a specific parent's method implementation *)
  TsomInheritedMethodStruct = record
    methodId  : PsomId;      (* identify the method *)
    parentNum : Longint;       (* identify the parent *)
    mToken    : PsomMToken;  (* for parentNumresolve *)
  end;
  TsomInheritedMethod_t  = TsomInheritedMethodStruct;
  TsomInheritedMethod_p  = ^TsomInheritedMethod_t;

(* to register a method that has been moved from this *)
(* class <cls> upwards in the class hierachy to class <dest> *)
  TsomMigratedMethodStruct = record
    clsMToken   : PsomMToken;
                                (* points into the <cls> classdata structure *)
                                (* the method token in <dest> will copied here *)
    destMToken  : PsomMToken;
                                (* points into the <dest> classdata structure *)
                                (* the method token here will be copied to <cls> *)
  end;
  TsomMigratedMethod_t   = TsomMigratedMethodStruct;
  TsomMigratedMethod_p   = ^TsomMigratedMethod_t;

(* to specify non-internal data *)
  TsomNonInternalDataStruct = record
    classData           : PsomDToken;
    basisForDataOffset  : PChar;
  end;
  TsomNonInternalData_t  = TsomNonInternalDataStruct;
  TsomNonInternalData_p  = ^TsomNonInternalData_t;

(* to specify a "procedure" or "classdata" *)
  TsomProcMethodsStruct  = record
    classData : PsomMethodProc;
    pEntry    : TsomMethodProc;
  end;
  TsomProcMethods_t      = TsomProcMethodsStruct;
  TsomProcMethods_p      = ^TsomProcMethods_t;

(* to specify a general method "action" using somMethodStruct *)
(*
  the type of action is specified by loading the type field of the
  somMethodStruct. There are three bit fields in the overall type:

  action (in type & 0xFF)
   0: static -- (i.e., virtual) uses somAddStaticMethod
   1: dynamic -- uses somAddDynamicMethod (classData==0)
   2: nonstatic -- (i.e., nonvirtual) uses somAddMethod
   3: udaAssign -- registers a method as the udaAssign (but doesn't add the method)
   4: udaConstAssign -- like 3, this doesn't add the method
   5: somClassResolve Override (using the class pointed to by *classData)
   6: somMToken Override (using the method token pointed to by methodId)
                         (note: classData==0 for this)
   7: classAllocate -- indicates the default heap allocator for this class.
                       If classData == 0, then method is the code address (or NULL)
                       If classData != 0, then *classData is the code address.
                       No other info required (or used)
   8: classDeallocate -- like 7, but indicates the default heap deallocator.
   9: classAllocator -- indicates a non default heap allocator for this class.
                        like 7, but a methodDescriptor can be given.

   === the following is not currently supported ===
   binary data access -- in (type & 0x100), valid for actions 0,1,2,5,6
   0: the method procedure doesn't want binary data access
   1: the method procedure does want binary data access

   aggregate return -- in (type & 0x200), used when binary data access requested
   0: method procedure doesn't return a structure
   1: method procedure does return a structure
*)

  TsomMethodStruct       =record
    mtype            : Longint;
    classData        : PsomMToken;
    methodId         : PsomId;
    methodDescriptor : PsomId;
    method           : TsomMethodProc;
    redispatchStub   : TsomMethodProc;
    applyStub        : TsomMethodProc;
  end;
  TsomMethods_t      = TsomMethodStruct;
  TsomMethods_p      = ^TsomMethods_t;

(* to specify a varargs function *)
  TsomVarargsFuncsStruct = record
    classData : PsomMethodProc;
    vEntry    : TsomMethodProc;
  end;
  TsomVarargsFuncs_t     = TsomVarargsFuncsStruct;
  TsomVarargsFuncs_p     = ^TsomVarargsFuncs_t;

(* to specify dynamically computed information (incl. embbeded objs) *)
  TsomDynamicSCIPtr      = ^TsomDynamicSci;
  TsomDynamicSCI         = record
    version          : Longint;       (* 1 for now *)
    instanceDataSize : Longint;       (* true size (incl. embedded objs) *)
    dataAlignment    : Longint;       (* true alignment *)
    embeddedObjs     : TsomEmbeddedObjStructPtr; (* array end == null copp *)
  end;


(*
   to specify a DTS class, use the somDTSClass entry in the following
   data structure. This entry is a bit vector interpreted as follows:

   (somDTSClass & 0x0001) == the class is a DTS C++ class
   (somDTSClass & 0x0002) == the class wants the initCtrl entries
                             of the somParentMtabStruct filled in.

*)



(*
 *  The Static Class Info Structure passed to somBuildClass
 *)

  TsomStaticClassInfoStruct = record
    layoutVersion      : Longint;  (* this struct defines layout version SOM_SCILEVEL *)
    numStaticMethods   : Longint;   (* count of smt entries *)
    numStaticOverrides : Longint; (* count of omt entries *)
    numNonInternalData : Longint; (* count of nit entries *)
    numProcMethods     : Longint;     (* count of pmt entries *)
    numVarargsFuncs    : Longint;    (* count of vft entries *)
    majorVersion       : Longint;
    minorVersion       : Longint;
    instanceDataSize   : Longint;   (* instance data introduced by this class *)
    maxMethods         : Longint;         (* count numStaticMethods and numMethods *)
    numParents         : Longint;
    classId            : TsomId;
    explicitMetaId     : TsomId;
    implicitParentMeta : Longint;
    parents            : PsomId;
    cds                : TsomClassDataStructurePtr;
    ccds               : TsomCClassDataStructurePtr;
    smt                : TsomStaticMethod_p; (* basic "static" methods for mtab *)
    omt                : TsomOverrideMethod_p; (* overrides for mtab *)
    nitReferenceBase   : PChar;
    nit                : TsomNonInternalData_p; (* datatokens for instance data *)
    pmt                : TsomProcMethods_p; (* Arbitrary ClassData members *)
    vft                : TsomVarargsFuncs_p; (* varargs stubs *)
    cif                : pointer{^somTP_somClassInitFunc}; (* class init function *)
    (* end of layout version 1 *)

    (* begin layout version 2 extensions *)
    dataAlignment      : Longint; (* the desired byte alignment for instance data *)
    (* end of layout version 2 *)

//#define SOMSCIVERSION 1

    (* begin layout version 3 extensions *)
    numDirectInitClasses: Longint;
    directInitClasses   : PsomId;
    numMethods          : Longint; (* general (including nonstatic) methods for mtab *)
    mt                  : TsomMethods_p;
    protectedDataOffset : Longint; (* access = resolve(instanceDataToken) + offset *)
    somSCIVersion       : Longint;  (* used during development. currently = 1 *)
    numInheritedMethods : Longint;
    imt                 : TsomInheritedMethod_p; (* inherited method implementations *)
    numClassDataEntries : Longint; (* should always be filled in *)
    classDataEntryNames : PsomId; (* either NULL or ptr to an array of somIds *)
    numMigratedMethods  : Longint;
    mmt                 : TsomMigratedMethod_p; (* migrated method implementations *)
    numInitializers     : Longint; (* the initializers for this class *)
    initializers        : PsomId;     (* in order of release *)
    somDTSClass         : Longint; (* used to identify a DirectToSOM class *)
    dsci                : TsomDynamicSCIPtr;  (* used to register dynamically computed info *)
    (* end of layout version 3 *)
  end;
  TsomStaticClassInfo    = TsomStaticClassInfoStruct;
  TsomStaticClassInfoPtr = ^TsomStaticClassInfoStruct;


type
(*----------------------------------------------------------------------
 *  Typedefs for pointers to functions
 *)

  TContained_Description         = record
    name  : TIdentifier;
    value : Tany;
  end;

  TInterfaceDef_FullInterfaceDescription = record
    name                : TIdentifier;
    id, defined_in      : TRepositoryId;
    {operation          : T_IDL_SEQUENCE_OperationDef_OperationDescription;
    attributes          : T_IDL_SEQUENCE_AttributeDef_AttributeDescription;}
  end;

  TInterfaceDef_InterfaceDescription = record
    name                : TIdentifier;
    id, defined_in      : TRepositoryId;
  end;

(* CORBA 7.6.1, p.139 plus 5.7, p.89 enum Data Type Mapping *)
type
  TCKind                = Cardinal;
const
  TypeCode_tk_null      = 1;
  TypeCode_tk_void      = 2;
  TypeCode_tk_short     = 3;
  TypeCode_tk_long      = 4;
  TypeCode_tk_ushort    = 5;
  TypeCode_tk_ulong     = 6;
  TypeCode_tk_float     = 7;
  TypeCode_tk_double    = 8;
  TypeCode_tk_boolean   = 9;
  TypeCode_tk_char      = 10;
  TypeCode_tk_octet     = 11;
  TypeCode_tk_any       = 12;
  TypeCode_tk_TypeCode  = 13;
  TypeCode_tk_Principal = 14;
  TypeCode_tk_objref    = 15;
  TypeCode_tk_struct    = 16;
  TypeCode_tk_union     = 17;
  TypeCode_tk_enum      = 18;
  TypeCode_tk_string    = 19;
  TypeCode_tk_sequence  = 20;
  TypeCode_tk_array     = 21;

  TypeCode_tk_pointer   = 101; (* SOM extension *)
  TypeCode_tk_self      = 102; (* SOM extension *)
  TypeCode_tk_foreign   = 103; (* SOM extension *)

(* Short forms of tk_<x> enumerators *)

  tk_null       = TypeCode_tk_null;
  tk_void       = TypeCode_tk_void;
  tk_short      = TypeCode_tk_short;
  tk_long       = TypeCode_tk_long;
  tk_ushort     = TypeCode_tk_ushort;
  tk_ulong      = TypeCode_tk_ulong;
  tk_float      = TypeCode_tk_float;
  tk_double     = TypeCode_tk_double;
  tk_boolean    = TypeCode_tk_boolean;
  tk_char       = TypeCode_tk_char;
  tk_octet      = TypeCode_tk_octet;
  tk_any        = TypeCode_tk_any;
  tk_TypeCode   = TypeCode_tk_TypeCode;
  tk_Principal  = TypeCode_tk_Principal;
  tk_objref     = TypeCode_tk_objref;
  tk_struct     = TypeCode_tk_struct;
  tk_union      = TypeCode_tk_union;
  tk_enum       = TypeCode_tk_enum;
  tk_string     = TypeCode_tk_string;
  tk_sequence   = TypeCode_tk_sequence;
  tk_array      = TypeCode_tk_array;
  tk_pointer    = TypeCode_tk_pointer;
  tk_self       = TypeCode_tk_self;
  tk_foreign    = TypeCode_tk_foreign;

type
  TSOMClass_somOffsets           = record
    cls    : TRealSOMClass;
    offset : Longint;
  end;

  T_IDL_SEQUENCE_SOMClass        = record
    _maximum : Cardinal;
    _length  : Cardinal;
    _buffer  : PRealSOMClass;
  end;
  T_IDL_SEQUENCE_SOMObject       = record
    _maximum : Cardinal;
    _length  : Cardinal;
    _buffer  : PRealSOMObject;
  end;
  TSOMClass_SOMClassSequence     = T_IDL_SEQUENCE_SOMClass;

(*----------------------------------------------------------------------
 *  Windows extra procedures:
 *)

(*
 *  Replaceable character output handler.
 *  Points to the character output routine to be used in development
 *  support.  Initialized to <somOutChar>, but may be reset at anytime.
 *  Should return 0 (false) if an error occurs and 1 (true) otherwise.
 *)
type
  TsomTD_SOMOutCharRoutine = function(ch:Char): Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//var
{ $warning support of external vars required}
//  SOMOutCharRoutine     : TsomTD_SOMOutCharRoutine;//³ 00015 ³ SOMOutCharRoutine

Procedure somSetOutChar(outch:TsomTD_SOMOutCharRoutine); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somMainProgram: TRealSOMClassMgr; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Procedure somEnvironmentEnd; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somAbnormalEnd: Boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(*--------------------------------------------------------*)


(*---------------------------------------------------------------------
 *  Offset-based method resolution.
 *)

Function  somResolve(obj:TRealSOMObject; mdata:TsomMToken):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somParentResolve(parentMtabs:TsomMethodTabs;
                           mToken:TsomMToken):TsomMethodProc; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somParentNumResolve(parentMtabs:TsomMethodTabs;
                              parentNum:Longint;mToken:TsomMToken):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somClassResolve(obj:TRealSOMClass; mdata:TsomMToken):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somAncestorResolve(obj:TRealSOMObject;                 (* the object *)
                             var ccds:TsomCClassDataStructure;   (* id the ancestor *)
                             mToken:TsomMToken):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somResolveByName(obj:TRealSOMObject;
                           methodName:PChar):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(*------------------------------------------------------------------------------
 * Offset-based data resolution
 *)
Function  somDataResolve(obj:TRealSOMObject; dataId:TsomDToken):TsomToken; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somDataResolveChk(obj:TRealSOMObject; dataId:TsomDToken):TsomToken; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(*----------------------------------------------------------------------
 *  Misc. procedures:
 *)

(*
 *  Create and initialize the SOM environment
 *
 *  Can be called repeatedly
 *
 *  Will be called automatically when first object (including a class
 *  object) is created, if it has not already been done.
 *
 *  Returns the SOMClassMgrObject
 *)
Function  somEnvironmentNew:TRealSOMClassMgr; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(*
 * Test whether <obj> is a valid SOM object. This test is based solely on
 * the fact that (on this architecture) the first word of a SOM object is a
 * pointer to its method table. The test is therefore most correctly understood
 * as returning true if and only if <obj> is a pointer to a pointer to a
 * valid SOM method table. If so, then methods can be invoked on <obj>.
 *)
Function  somIsObj(obj:TsomToken):Boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(*
 * Return the class that introduced the method represented by a given method token.
 *)
Function  somGetClassFromMToken(mToken:TsomMToken):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}


(*----------------------------------------------------------------------
 *  String Manager: stem <somsm>
 *)
Function  somCheckID(id:TsomId):TsomId; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* makes sure that the id is registered and in normal form, returns *)
(* the id *)

Function  somRegisterId(id:TsomId):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* Same as somCheckId except returns 1 (true) if this is the first *)
(* time the string associated with this id has been registered, *)
(* returns 0 (false) otherwise *)

Function  somIDFromString(aString:PChar):TsomId; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* caller is responsible for freeing the returned id with SOMFree *)

// Not found
//Function  somIdFromStringNoFree(aString:PChar):TsomId; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* call is responsible for *not* freeing the returned id *)

Function  somStringFromId(id:TsomId):PChar; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

Function  somCompareIds(id1,id2:TsomId):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* returns true (1) if the two ids are equal, else false (0) *)

Function  somTotalRegIds:Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* Returns the total number of ids that have been registered so far, *)
(* you can use this to advise the SOM runtime concerning expected *)
(* number of ids in later executions of your program, via a call to *)
(* somSetExpectedIds defined below *)

Procedure somSetExpectedIds(numIds:Longint{ulong}); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* Tells the SOM runtime how many unique ids you expect to use during *)
(* the execution of your program, this can improve space and time *)
(* utilization slightly, this routine must be called before the SOM *)
(* environment is created to have any effect *)

Function  somUniqueKey(id:TsomId):Longint{ulong}; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* Returns the unique key for this id, this key will be the same as the *)
(* key in another id if and only if the other id refers to the same *)
(* name as this one *)

Procedure somBeginPersistentIds; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* Tells the id manager that strings for any new ids that are *)
(* registered will never be freed or otherwise modified. This allows *)
(* the id manager to just use a pointer to the string in the *)
(* unregistered id as the master copy of the ids string. Thus saving *)
(* space *)
(* Under normal use (where ids are static varibles) the string *)
(* associated with an id would only be freed if the code module in *)
(* which it occured was unloaded *)

Procedure somEndPersistentIds; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
(* Tells the id manager that strings for any new ids that are *)
(* registered may be freed or otherwise modified.  Therefore the id *)
(* manager must copy the strings inorder to remember the name of an *)
(* id. *)

(*----------------------------------------------------------------------
 *  Class Manager: SOMClassMgrType, stem <somcm>
 *)

(* Global class manager object *)

var
  SOMClassMgrObjectPtr: ^TRealSOMClassMgr;

(* The somRegisterClassLibrary function is provided for use
 * in SOM class libraries on platforms that have loader-invoked
 * entry points associated with shared libraries (DLLs).
 *
 * This function registers a SOM Class Library with the SOM Kernel.
 * The library is identified by its file name and a pointer
 * to its initialization routine.  Since this call may occur
 * prior to the invocation of somEnvironmentNew, its actions
 * are deferred until the SOM environment has been initialized.
 * At that time, the SOMClassMgrObject is informed of all
 * pending library initializations via the _somRegisterClassLibrary
 * method.  The actual invocation of the library's initialization
 * routine will occur during the execution of the SOM_MainProgram
 * macro (for statically linked libraries), or during the _somFindClass
 * method (for libraries that are dynamically loaded).
 *)
Procedure somRegisterClassLibrary(libraryName:PChar;
                                  libraryInitRun:TsomMethodProc); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(*----------------------------------------------------------------------
 * -- somApply --
 *
 * This routine replaces direct use of applyStubs in SOMr1. The reason
 * for the replacement is that the SOMr1 style of applyStub is not
 * generally available in SOMr2, which uses a fixed set of applyStubs,
 * according to method information in the somMethodData. In particular,
 * neither the redispatch stub nor the apply stub found in the method
 * data structure are necessarily useful as such. The method somGetRdStub
 * is the way to get a redispatch stub, and the above function is the
 * way to call an apply stub. If an appropriate apply stub for the
 * method indicated by md is available, then this is invoked and TRUE is
 * returned; otherwise FALSE is returned.
 *
 * The va_list passed to somApply *must* include the target object,
 * somSelf, as its first entry, and any single precision floating point
 * arguments being passed to the the method procedure must be
 * represented on the va_list using double precision values. retVal cannot
 * be NULL.
 *)

Function  somApply(var somSelf:TRealSOMObject;
                   var retVal:TsomToken;
                   mdPtr:PsomMethodData;
                   var ap):Boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(*---------------------------------------------------------------------
 * -- somBuildClass --
 *
 * This procedure automates construction of a new class object. A variety of
 * special structures are used to allow language bindings to statically define
 * the information necessary to specify a class. Pointers to these static
 * structures are accumulated into an overall "static class information"
 * structure or SCI, passed to somBuildClass. The SCI has evolved over time.
 * The current version is defined here.
 *)


Function  somBuildClass(inherit_vars:Longint;
                        var sci:TsomStaticClassInfo;
                        majorVersion,minorVersion:Longint):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

  (*
  The arguments to somBuildClass are as follows:

   inherit_vars: a bit mask used to control inheritance of implementation
   Implementation is inherited from parent i iff the bit 1<<i is on, or i>=32.

   sci: the somStaticClassInfo defined above.

   majorVersion, minorVersion: the version of the class implementation.

   *)


(*---------------------------------------------------------------------
 *  Used by old single-inheritance emitters to make class creation
 *  an atomic operation. Kept for backwards compatability.
 *)
type
  TsomTD_classInitRoutine = Procedure(var a,b: TRealSOMClass); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

Procedure somConstructClass(classInitRoutine:TsomTD_ClassInitRoutine;
                            parentClass,metaClass:TRealSOMClass;
                            var cds :TsomClassDataStructure); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}


(*
 * Uses <SOMOutCharRoutine> to output its arguments under control of the ANSI C
 * style format.  Returns the number of characters output.
 *)
Function  somPrintf(fnt:PChar;buf:pointer):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

// vprint form of somPrintf
Function  somVPrintf(fnt:PChar;var ap):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

// Outputs (via somPrintf) blanks to prefix a line at the indicated level
Procedure somPrefixLevel(level:Longint); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

// Combines somPrefixLevel and somPrintf
Procedure somLPrintf(level:Longint;fmt:PChar;var buf); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//Function SOMObjectNewClass(majorVersion,minorVersion:Longint):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
//Function SOMClassNewClass(majorVersion,minorVersion:Longint):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//Function SOMClassMgrNewClass(majorVersion,minorVersion:Longint):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(*----------------------------------------------------------------------
 * Pointers to routines used to do dynamic code loading and deleting
 *)
type
  TsomTD_SOMLoadModule          = Function({IN}Module:PChar      (* className *);
                                          {IN}FileName:PChar    (* fileName *);
                                          {IN}FuncName:PChar    (* functionName *);
                                          {IN}MajorVer:Longint  (* majorVersion *);
                                          {IN}MinorVer:Longint  (* minorVersion *);
                                          {OUT}var ref:TsomToken(* modHandle *)):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  TsomTD_SOMDeleteModule        = Function({IN} ref:TsomToken    (* modHandle *)):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  TsomTD_SOMClassInitFuncName   = Function:PChar; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//var
{ $warning support of external vars required}
//  SOMLoadModule         :TsomTD_SOMLoadModule;//³ 00011 ³ SOMLoadModule
{ $warning support of external vars required}
//  SOMDeleteModule       :TsomTD_SOMDeleteModule;//³ 00008 ³ SOMDeleteModule
{ $warning support of external vars required}
//  SOMClassInitFuncName  :TsomTD_SOMClassInitFuncName; //³ 00004 ³ SOMClassInitFuncName

(*----------------------------------------------------------------------
 *  Replaceable SOM Memory Management Interface
 *
 *  External procedure variables SOMCalloc, SOMFree, SOMMalloc, SOMRealloc
 *  have the same interface as their standard C-library analogs.
 *)

type
  TsomTD_SOMMalloc               = Function({IN} size_t:Longint   (* nbytes *)):TsomToken; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  TsomTD_SOMCalloc               = Function({IN} size_c:Longint   (* element_count *);
                                          {IN} size_e:Longint   (* element_size *)):TsomToken; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  TsomTD_SOMRealloc              = Function({IN} ref:TsomToken     (* memory *);
                                          {IN} size:Longint     (* nbytes *)):TsomToken; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  TsomTD_SOMFree                 = Procedure({IN} ref:TsomToken    (* memory *)); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//var
{ $warning support of external vars required}
//  SOMCalloc             :TsomTD_SOMCalloc; // ³ 00001 ³ SOMCalloc
{ $warning support of external vars required}
//  SOMFree               :TsomTD_SOMFree; //³ 00010 ³ SOMFree
{ $warning support of external vars required}
//  SOMMalloc             :TsomTD_SOMMalloc;//³ 00012 ³ SOMMalloc
{ $warning support of external vars required}
//  SOMRealloc            :TsomTD_SOMRealloc;//³ 00016 ³ SOMRealloc

(*----------------------------------------------------------------------
 *  Replaceable SOM Error handler
 *)

type
  TsomTD_SOMError               = Procedure({IN} code:Longint    (* code *);
                                           {IN} fn:PChar        (* fileName *);
                                           {IN} ln:Longint      (* linenum *)); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//var
{ $warning support of external vars required}
//  SOMError              :TsomTD_SOMError;//³ 00009 ³ SOMError

(*----------------------------------------------------------------------
 *  Replaceable SOM Semaphore Operations
 *
 *  These operations are used by the SOM Kernel to make thread-safe
 *  state changes to internal resources.
 *)

type
  TsomTD_SOMCreateMutexSem       =Function({OUT}var sem:TsomToken ):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  TsomTD_SOMRequestMutexSem      =Function({IN}sem:TsomToken ):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  TsomTD_SOMReleaseMutexSem      =Function({IN}sem:TsomToken ):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  TsomTD_SOMDestroyMutexSem      =Function({IN}sem:TsomToken ):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//var
{ $warning support of external vars required}
//  SOMCreateMutexSem     :TsomTD_SOMCreateMutexSem;//³ 00090 ³ SOMCreateMutexSem
{ $warning support of external vars required}
//  SOMRequestMutexSem    :TsomTD_SOMRequestMutexSem;//³ 00091 ³ SOMRequestMutexSem
{ $warning support of external vars required}
//  SOMReleaseMutexSem    :TsomTD_SOMReleaseMutexSem;//³ 00092 ³ SOMReleaseMutexSem
{ $warning support of external vars required}
//  SOMDestroyMutexSem    :TsomTD_SOMDestroyMutexSem;//³ 00093 ³ SOMDestroyMutexSem

(*----------------------------------------------------------------------
 *  Replaceable SOM Thread Identifier Operation
 *
 *  This operation is used by the SOM Kernel to index data unique to the
 *  currently executing thread.  It must return a small integer that
 *  uniquely represents the current thread within the current process.
 *)

type
  TsomTD_SOMGetThreadId          =Function:Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//var
{ $warning support of external vars required}
//  SOMGetThreadId        :TsomTD_SOMGetThreadId;//³ 00094 ³ SOMGetThreadId


(*----------------------------------------------------------------------
 * Externals used in the implementation of SOM, but not part of the
 * SOM API.
 *)

Function  somTestCls(obj:TRealSOMObject; classObj:TRealSOMClass;
                     fileName:PChar; lineNumber:Longint):TRealSOMObject; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Procedure somTest(condition,severity:Longint;fileName:PChar;
                  lineNum:Longint;msg:PChar); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Procedure somAssert(condition,ecode:Longint;
                    fileName:PChar;lineNum:Longint;msg:PChar); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

type
  Texception_type        = (NO_EXCEPTION, USER_EXCEPTION, SYSTEM_EXCEPTION);
  Tcompletion_status     = (YES, NO, MAYBE);
  TStExcep               = record
    minot               : Cardinal;
    completed           : Tcompletion_status;
  end;

  TEnvironment           = ^TEnvironmentType;
  TEnvironmentType       = record
    _major              : Texception_type;
    exception           : record
      _exception_name   : PChar;
      _params           : Pointer;
    end;
    _somdAnchor         : pointer;
  end;
  PEnvironment           = ^TEnvironmentType;

Function  somExceptionId(ev:TEnvironment):PChar; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somExceptionValue(ev:TEnvironment):Pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Procedure somExceptionFree(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Procedure somSetException(ev:TEnvironment;major:Texception_type;exception_name:PChar;params:pointer); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  somGetGlobalEnvironment:TEnvironment; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(* Exception function names per CORBA 5.19, p.99 *)
Function  exception_id(ev:TEnvironment):PChar; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function  exception_value(ev:TEnvironment):Pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Procedure exception_free(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}


(*  Convenience macros for manipulating environment structures
 *
 *  SOM_CreateLocalEnvironment returns a pointer to an Environment.
 *  The other 3 macros all expect a single argument that is also
 *  a pointer to an Environment.  Use the create/destroy forms for
 *  a dynamic local environment and the init/uninit forms for a stack-based
 *  local environment.
 *
 *  For example
 *
 *      Environment *ev;
 *      ev = SOM_CreateLocalEnvironment ();
 *      ... Use ev in methods
 *      SOM_DestroyLocalEnvironment (ev);
 *
 *  or
 *
 *      Environment ev;
 *      SOM_InitEnvironment (&ev);
 *      ... Use &ev in methods
 *      SOM_UninitEnvironment (&ev);
 *)
//Function SOM_CreateLocalEnvironment:TEnvironment; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//Procedure SOM_DestroyLocalEnvironment(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//Procedure SOM_InitEnvironment(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

//Procedure SOM_UninitEnvironment(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

(*----------------------------------------------------------------------
 * Macros are used in the C implementation of SOM... However, Pascal
 * doesn't have macro capability... (from SOMCDEV.H)
 *)

{ Change SOM_Resolve(o,ocn,mn) to...
  somTD_ocn_mn(somResolve(SOM_TestCls(o, ocnClassData.classObject), ocnClassData.mn)))

  Change SOM_ResolveNoCheck(o,ocn,mn) to...
  somTD_ocn_mn(somResolve(o,ocnClassData,mn))

  Change SOM_ParentNumResolveCC(pcn,pcp,ocn,mn) to...
  somTD_pcn_mn(somParentNumResolve(ocn_CClassData.parentMtab,pcp,pcnClassData.mn))

  Change SOM_ParentNumResolve(pcn,pcp,mtabs,mn) to...
  somTD_pcn_mn(somParentNumResolve(mtabs,pcp,pcnClassData.mn))

  Change SOM_ClassResolve(cn,class,mn) to...
  somTD_cn_mn(somClassResolve(class,cnClassData.mn))

  Change SOM_ResolveD(o,tdc,cdc,mn) to...
  somTD_tdc_mn(somResolve(SOM_TestCls(o,cdcClassData.classObject), cdcClassData.mn)))

  Change SOM_ParentResolveE(pcn,mtbls,mn) to...
  somTD_pcn_mn(somParentResolve(mtbls,pcnClassData.mn))

  Change SOM_DataResolve(obj,dataId) to...
  somDataResolve(obj, dataId)

  Change SOM_ClassLibrary(name) to...
  somRegisterClassLibrary(name,somMethodProc(SOMInitModule))
}

type
  TSOMClassCClassDataStructure   = record
    parentMtab                  : TsomMethodTabs;
    instanceDataToken           : TsomDToken;
  end;

//var
{ $warning support of external vars required}
//  SOMClassCClassData            : TSOMClassCClassDataStructure;//³ 00002 ³ SOMClassCClassData

type
  TSOMClassClassDataStructure    = record
    classObject                 : TRealSOMClass;
    somNew                      : TsomMToken;
    somRenew                    : TsomMToken;
    somInitClass                : TsomMToken;
    somClassReady               : TsomMToken;
    somGetName                  : TsomMToken;
    somGetParent                : TsomMToken;
    somDescendedFrom            : TsomMToken;
    somCheckVersion             : TsomMToken;
    somFindMethod               : TsomMToken;
    somFindMethodOk             : TsomMToken;
    somSupportsMethod           : TsomMToken;
    somGetNumMethods            : TsomMToken;
    somGetInstanceSize          : TsomMToken;
    somGetInstanceOffset        : TsomMToken;
    somGetInstancePartSize      : TsomMToken;
    somGetMethodIndex           : TsomMToken;
    somGetNumStaticMethods      : TsomMToken;
    somGetPClsMtab              : TsomMToken;
    somGetClassMtab             : TsomMToken;
    somAddStaticMethod          : TsomMToken;
    somOverrideSMethod          : TsomMToken;
    somAddDynamicMethod         : TsomMToken;
    somcPrivate0                : TsomMToken;
    somGetApplyStub             : TsomMToken;
    somFindSMethod              : TsomMToken;
    somFindSMethodOk            : TsomMToken;
    somGetMethodDescriptor      : TsomMToken;
    somGetNthMethodInfo         : TsomMToken;
    somSetClassData             : TsomMToken;
    somGetClassData             : TsomMToken;
    somNewNoInit                : TsomMToken;
    somRenewNoInit              : TsomMToken;
    somGetInstanceToken         : TsomMToken;
    somGetMemberToken           : TsomMToken;
    somSetMethodDescriptor      : TsomMToken;
    somGetMethodData            : TsomMToken;
    somOverrideMtab             : TsomMToken;
    somGetMethodToken           : TsomMToken;
    somGetParents               : TsomMToken;
    somGetPClsMtabs             : TsomMToken;
    somInitMIClass              : TsomMToken;
    somGetVersionNumbers        : TsomMToken;
    somLookupMethod             : TsomMToken;
    _get_somInstanceDataOffsets : TsomMToken;
    somRenewNoZero              : TsomMToken;
    somRenewNoInitNoZero        : TsomMToken;
    somAllocate                 : TsomMToken;
    somDeallocate               : TsomMToken;
    somGetRdStub                : TsomMToken;
    somGetNthMethodData         : TsomMToken;
    somcPrivate1                : TsomMToken;
    somcPrivate2                : TsomMToken;
    _get_somDirectInitClasses   : TsomMToken;
    _set_somDirectInitClasses   : TsomMToken;
    somGetInstanceInitMask      : TsomMToken;
    somGetInstanceDestructionMask: TsomMToken;
    somcPrivate3                : TsomMToken;
    somcPrivate4                : TsomMToken;
    somcPrivate5                : TsomMToken;
    somcPrivate6                : TsomMToken;
    somcPrivate7                : TsomMToken;
    somDefinedMethod            : TsomMToken;
    somcPrivate8                : TsomMToken;
    somcPrivate9                : TsomMToken;
    somcPrivate10               : TsomMToken;
    somcPrivate11               : TsomMToken;
    somcPrivate12               : TsomMToken;
    somcPrivate13               : TsomMToken;
    somcPrivate14               : TsomMToken;
    somcPrivate15               : TsomMToken;
    _get_somDataAlignment       : TsomMToken;
    somGetInstanceAssignmentMask: TsomMToken;
    somcPrivate16               : TsomMToken;
    somcPrivate17               : TsomMToken;
    _get_somClassAllocate       : TsomMToken;
    _get_somClassDeallocate     : TsomMToken;
  end;

//var
{ $warning support of external vars required}
//  SOMClassClassData             : TSOMClassClassDataStructure;//³ 00003 ³ SOMClassClassData
{ $warning support of external vars required}
//  SOMClassMgrCClassData         : TsomCClassDataStructure;//³ 00005 ³ SOMClassMgrCClassData

type
  TSOMClassMgrClassDataStructure = record
    classObject                 : TRealSOMClass;
    somFindClsInFile            : TsomMToken;
    somFindClass                : TsomMToken;
    somClassFromId              : TsomMToken;
    somRegisterClass            : TsomMToken;
    somUnregisterClass          : TsomMToken;
    somLocateClassFile          : TsomMToken;
    somLoadClassFile            : TsomMToken;
    somUnloadClassFile          : TsomMToken;
    somGetInitFunction          : TsomMToken;
    somMergeInto                : TsomMToken;
    somGetRelatedClasses        : TsomMToken;
    somSubstituteClass          : TsomMToken;
    _get_somInterfaceRepository : TsomMToken;
    _set_somInterfaceRepository : TsomMToken;
    _get_somRegisteredClasses   : TsomMToken;
    somBeginPersistentClasses   : TsomMToken;
    somEndPersistentClasses     : TsomMToken;
    somcmPrivate1               : TsomMToken;
    somcmPrivate2               : TsomMToken;
    somRegisterClassLibrary     : TsomMToken;
    somJoinAffinityGroup        : TsomMToken;
  end;

var
{ $warning support of external vars required}
  SOMClassMgrClassData          : ^TSOMClassMgrClassDataStructure;//³ 00006 ³ SOMClassMgrClassData

type
  TSOMObjectCClassDataStructure  = record
    parentMtab                  : TsomMethodTabs;
    instanceDataToken           : TsomDToken;
  end;

//var
{ $warning support of external vars required}
//  SOMObjectCClassData           : TSOMObjectCClassDataStructure;//³ 00013 ³ SOMObjectCClassData

{type
  TSOMObjectClassDataStructure   = record
    classObject                 : TRealSOMClass;
    somInit                     : TsomMToken;
    somUninit                   : TsomMToken;
    somFree                     : TsomMToken;
    somDefaultVCopyInit         : TsomMToken;
    somGetClassName             : TsomMToken;
    somGetClass                 : TsomMToken;
    somIsA                      : TsomMToken;
    somRespondsTo               : TsomMToken;
    somIsInstanceOf             : TsomMToken;
    somGetSize                  : TsomMToken;
    somDumpSelf                 : TsomMToken;
    somDumpSelfInt              : TsomMToken;
    somPrintSelf                : TsomMToken;
    somDefaultConstVCopyInit    : TsomMToken;
    somDispatchV                : TsomMToken;
    somDispatchL                : TsomMToken;
    somDispatchA                : TsomMToken;
    somDispatchD                : TsomMToken;
    somDispatch                 : TsomMToken;
    somClassDispatch            : TsomMToken;
    somCastObj                  : TsomMToken;
    somResetObj                 : TsomMToken;
    somDefaultInit              : TsomMToken;
    somDestruct                 : TsomMToken;
    somPrivate1                 : TsomMToken;
    somPrivate2                 : TsomMToken;
    somDefaultCopyInit          : TsomMToken;
    somDefaultConstCopyInit     : TsomMToken;
    somDefaultAssign            : TsomMToken;
    somDefaultConstAssign       : TsomMToken;
    somDefaultVAssign           : TsomMToken;
    somDefaultConstVAssign      : TsomMToken;
  end;
}

//var
{ $warning support of external vars required}
//  SOMObjectClassData            : TSOMObjectClassDataStructure;//³ 00014 ³ SOMObjectClassData

(* Another not ported vars *)
// Control the printing of method and procedure entry messages,
// 0-none, 1-user, 2-core&user */
//  SOM_TraceLevel: Longint; //³ 00018 ³ SOM_TraceLevel

// Control the printing of warning messages, 0-none, 1-all
//  SOM_WarnLevel: Longint; //³ 00019 ³ SOM_WarnLevel

// Control the printing of successful assertions, 0-none, 1-user,
// 2-core&user
//  SOM_AssertLevel: Longint; //³ 00017 ³ SOM_AssertLevel

// ToDo: Move this to corresponding place
Procedure somCheckArgs(argc: longint; argv: array of pchar); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Procedure somUnregisterClassLibrary (libraryName: PChar); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function somResolveTerminal(x : PRealSOMClass; mdata: TsomMToken): PsomMethodProc; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function somPCallResolve(obj: PRealSOMObject; callingCls: PRealSOMClass; method: TsomMToken): PsomMethodProc; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function va_SOMObject_somDispatchA(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): Pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function somva_SOMObject_somDispatchA(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): Pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function va_SOMObject_somDispatchL(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function somva_SOMObject_somDispatchL(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

Function va_SOMObject_somDispatch(somSelf: PRealSOMObject;
                retValue: PsomToken;
                methodId: TsomId;
                args: array of const): Boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

Procedure va_SOMObject_somDispatchV(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

Procedure somva_SOMObject_somDispatchV(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

Function va_SOMObject_somDispatchD(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): double; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

Function somva_SOMObject_somDispatchD(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): double; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function somva_SOMObject_somDispatch(somSelf: PRealSOMObject;
                retValue: PsomToken;
                methodId: TsomId;
                args: array of const): boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
Function somva_SOMObject_somClassDispatch(somSelf: PRealSOMObject;
                clsObj: PRealSOMClass;
                retValue: PsomToken;
                methodId: TsomId;
                args: array of const): boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}

Implementation

uses Os2Def, Os2Base, VPUtils;

Procedure somSetOutChar(outch:TsomTD_SOMOutCharRoutine); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somSetOutChar'; {index 85}
Function  somMainProgram: TRealSOMClassMgr; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somMainProgram'; {index 88}
Procedure somEnvironmentEnd; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somEnvironmentEnd'; {index 83}
Function  somAbnormalEnd: Boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somAbnormalEnd'; {index 84}
Function  somResolve(obj:TRealSOMObject; mdata:TsomMToken):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somResolve'; {index 37}
Function  somParentResolve(parentMtabs:TsomMethodTabs;
                           mToken:TsomMToken):TsomMethodProc; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somParentResolve'; {index 33}
Function  somParentNumResolve(parentMtabs:TsomMethodTabs;
                              parentNum:Longint;mToken:TsomMToken):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somParentNumResolve'; {index 50}
Function  somClassResolve(obj:TRealSOMClass; mdata:TsomMToken):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somClassResolve'; {index 48}
Function  somAncestorResolve(obj:TRealSOMObject;                 (* the object *)
                             var ccds:TsomCClassDataStructure;   (* id the ancestor *)
                             mToken:TsomMToken):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somAncestorResolve'; {index 74}
Function  somResolveByName(obj:TRealSOMObject;
                           methodName:PChar):{somMethodProc}pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somResolveByName'; {index 61}
Function  somDataResolve(obj:TRealSOMObject; dataId:TsomDToken):TsomToken; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somDataResolve'; {index 47}
Function  somDataResolveChk(obj:TRealSOMObject; dataId:TsomDToken):TsomToken; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somDataResolveChk'; {index 72}
Function  somEnvironmentNew:TRealSOMClassMgr; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somEnvironmentNew'; {index 30}
Function  somIsObj(obj:TsomToken):Boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somIsObj'; {index 60}
Function  somGetClassFromMToken(mToken:TsomMToken):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somGetClassFromMToken'; {index 82}
Function  somCheckID(id:TsomId):TsomId; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somCheckId'; {index 26}
Function  somRegisterId(id:TsomId):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somRegisterId'; {index 36}
Function  somIDFromString(aString:PChar):TsomId; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somIdFromString'; {index 31}
Function  somStringFromId(id:TsomId):PChar; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somStringFromId'; {index 40}
Function  somCompareIds(id1,id2:TsomId):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somCompareIds'; {index 27}
Function  somTotalRegIds:Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somTotalRegIds'; {index 43}
Procedure somSetExpectedIds(numIds:Longint{ulong}); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somSetExpectedIds'; {index 39}
Function  somUniqueKey(id:TsomId):Longint{ulong}; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somUniqueKey'; {index 44}
Procedure somBeginPersistentIds; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somBeginPersistentIds'; {index 24}
Procedure somEndPersistentIds; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somEndPersistentIds'; {index 29}
Procedure somConstructClass(classInitRoutine:TsomTD_ClassInitRoutine;
                            parentClass,metaClass:TRealSOMClass;
                            var cds :TsomClassDataStructure); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somConstructClass'; {index 28}
Function  somPrintf(fnt:PChar;buf:pointer):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somPrintf'; {index 35}
Function  somVPrintf(fnt:PChar;var ap):Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somVprintf'; {index 45}
Procedure somPrefixLevel(level:Longint); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somPrefixLevel'; {index 34}
Procedure somLPrintf(level:Longint;fmt:PChar;var buf); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somLPrintf'; {index 32}
//Function SOMObjectNewClass(majorVersion,minorVersion:Longint):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
//  external 'som' name 'SOMObjectNewClass'; {index 22}
//Function SOMClassNewClass(majorVersion,minorVersion:Longint):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
//  external 'som' name 'SOMClassNewClass'; {index 21}
//Function SOMClassMgrNewClass(majorVersion,minorVersion:Longint):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
//  external 'som' name 'SOMClassMgrNewClass'; {index 20}
Function  somTestCls(obj:TRealSOMObject; classObj:TRealSOMClass;
                     fileName:PChar; lineNumber:Longint):TRealSOMObject; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somTestCls'; {index 42}
Procedure somTest(condition,severity:Longint;fileName:PChar;
                  lineNum:Longint;msg:PChar); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somTest'; {index 41}
Procedure somAssert(condition,ecode:Longint;
                    fileName:PChar;lineNum:Longint;msg:PChar); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somAssert'; {index 23}
Function  somExceptionId(ev:TEnvironment):PChar; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somExceptionId'; {index 52}
Function  somExceptionValue(ev:TEnvironment):Pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somExceptionValue'; {index 53}
Procedure somExceptionFree(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somExceptionFree'; {index 54}
Procedure somSetException(ev:TEnvironment;major:Texception_type;exception_name:PChar;params:pointer); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somSetException'; {index 55}
Function  somGetGlobalEnvironment:TEnvironment; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somGetGlobalEnvironment'; {index 58}
Procedure somCheckArgs(argc: longint; argv: array of pchar); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somCheckArgs'; {index 25}
Procedure somUnregisterClassLibrary (libraryName: PChar); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somUnregisterClassLibrary'; {index 89}
Function somResolveTerminal(x : PRealSOMClass; mdata: TsomMToken): PsomMethodProc; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somResolveTerminal'; {index 133}
Function somPCallResolve(obj: PRealSOMObject; callingCls: PRealSOMClass; method: TsomMToken): PsomMethodProc; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somPCallResolve'; {index 362}
Function va_SOMObject_somDispatchA(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): Pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'va_SOMObject_somDispatchA'; {index 64}
Function somva_SOMObject_somDispatchA(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): Pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somva_SOMObject_somDispatchA'; {index 96}
Function va_SOMObject_somDispatchL(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'va_SOMObject_somDispatchL'; {index 66}
Function somva_SOMObject_somDispatchL(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): Longint; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somva_SOMObject_somDispatchL'; {index 98}
Function va_SOMObject_somDispatch(somSelf: PRealSOMObject;
                retValue: PsomToken;
                methodId: TsomId;
                args: array of const): Boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'va_SOMObject_somDispatch'; {index 68}
Procedure va_SOMObject_somDispatchV(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'va_SOMObject_somDispatchV'; {index 67}
Procedure somva_SOMObject_somDispatchV(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somva_SOMObject_somDispatchV'; {index 99}
Function va_SOMObject_somDispatchD(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): double; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'va_SOMObject_somDispatchD'; {index 65}
Function somva_SOMObject_somDispatchD(somSelf: PRealSOMObject;
                methodId: TsomId;
                descriptor: TsomId;
                args: array of const): double; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somva_SOMObject_somDispatchD'; {index 97}
Function somva_SOMObject_somDispatch(somSelf: PRealSOMObject;
                retValue: PsomToken;
                methodId: TsomId;
                args: array of const): boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somva_SOMObject_somDispatch'; {index 100}
Function somva_SOMObject_somClassDispatch(somSelf: PRealSOMObject;
                clsObj: PRealSOMClass;
                retValue: PsomToken;
                methodId: TsomId;
                args: array of const): boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somva_SOMObject_somClassDispatch'; {index 101}
Procedure somRegisterClassLibrary(libraryName:PChar;
                                  libraryInitRun:TsomMethodProc); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somRegisterClassLibrary'; {index 86}
Function  somApply(var somSelf:TRealSOMObject;
                   var retVal:TsomToken;
                   mdPtr:PsomMethodData;
                   var ap):Boolean; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somApply'; {index 69}
Function  somBuildClass(inherit_vars:Longint;
                        var sci:TsomStaticClassInfo;
                        majorVersion,minorVersion:Longint):TRealSOMClass; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
  external 'som' name 'somBuildClass'; {index 49}

Function exception_id(ev:TEnvironment):PChar; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
begin
  Result := somExceptionId(ev)
end;

Function  exception_value(ev:TEnvironment):Pointer; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
begin
  Result := somExceptionValue(ev)
end;

Procedure exception_free(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
begin
  somExceptionFree(ev)
end;

//Function SOM_CreateLocalEnvironment:TEnvironment; {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
//begin
//  Result:=SOMCalloc(1, sizeof(TEnvironmentType))
//end;

//Procedure SOM_DestroyLocalEnvironment(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
//begin
//  somExceptionFree(ev);
//  if somGetGlobalEnvironment<>ev then SOMFree(ev);
//end;

Procedure SOM_InitEnvironment(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
begin
  if somGetGlobalEnvironment<>ev then FillChar(ev^,sizeof(TEnvironmentType),0);
end;

Procedure SOM_UninitEnvironment(ev:TEnvironment); {$IFNDEF VirtualPascal}cdecl;{$ENDIF}
begin
  somExceptionFree(ev);
end;

// Import of pointers to external variables

const
  ModuleName   : PChar = 'SOM.DLL';             // Name of module
var
  LoadError    : Array[0..255] of Char;         // Area for Load failure information
  ModuleHandle : hModule;                       // Module handle
  ModuleType   : ULong;                         // Module type
  rc           : ApiRet;                        // Return code
Begin
  // Query address of ClassData variable
  rc:=DosLoadModule(LoadError,                  // Failure information buffer
      sizeof(LoadError),                        // Size of buffer
      ModuleName,                               // Module to load
      ModuleHandle);                            // Module handle returned

  rc:=DosQueryProcAddr(
      ModuleHandle,                             // Handle to module
      70,                                       // No ProcName specified
      nil,                                      // ProcName (not specified)
      Pointer(SOM_MajorVersionPtr));            // Address returned

  rc:=DosQueryProcAddr(
      ModuleHandle,                             // Handle to module
      71,                                       // No ProcName specified
      nil,                                      // ProcName (not specified)
      Pointer(SOM_MinorVersionPtr));            // Address returned

  rc:=DosQueryProcAddr(
      ModuleHandle,                             // Handle to module
      95,                                       // No ProcName specified
      nil,                                      // ProcName (not specified)
      Pointer(SOM_MaxThreadsPtr));              // Address returned

  rc:=DosQueryProcAddr(
      ModuleHandle,                             // Handle to module
      7,                                        // No ProcName specified
      nil,                                      // ProcName (not specified)
      Pointer(SOMClassMgrObjectPtr));           // Address returned

  rc:=DosFreeModule(ModuleHandle);
End.

(*
³ 00038 ³ somSaveMetrics // not found
³ 00046 ³ somWriteMetrics // not found
³ 00051 ³ somCreateDynamicClass // not found
³ 00056 ³ SOM_IdTable // not found
³ 00057 ³ SOM_IdTableSize // not found
³ 00062 ³ somStartCriticalSection // not found
³ 00063 ³ somEndCriticalSection // not found
³ 00080 ³ somfixMsgTemplate // not found
³ 00087 ³ SOMParentDerivedMetaclassClassData // not found
³ 00132 ³ somFreeThreadData // not found
³ 00135 ³ somIdMarshal  // not found
³ 00361 ³ somMakeUserRdStub // not found
*)
