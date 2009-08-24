; $Id: basemaca.inc,v 1.1 2000/04/21 10:57:53 ktk Exp $ 

;	SCCSID = @(#)1.2  7/30/98 14:09:01 src/jfs/ifs/basemaca.inc, sysjfs, w45.fs32, 990417.1
;***	BASEMACA.INC
;
; IBM CONFIDENTIAL
;
; OCO Source Materials
;
; (c) Copyright IBM Corp. 1988, 1996
;
; The source code for this program is not published or otherwise divested of
; its tradesecrets, irrespective of what has been deposited with the U.S.
; Copyright Office.
;
;
;	Macros required for other include files
;	Copyright (c) 1988 Microsoft Corporation
;
;	DESCRIPTION
;
;	This file provides the following macros (enabled with INCL_DEF):
;
;	  DefEntry	prototype-name
;	  DefType	type-name,{type,structure}[,def-value[,def-rept]]
;	  DefStruc	[type-name[,{type,structure}]]
;	  EndStruc	type-name[,{type,structure}]
;	  DefSeg	seg,class,type,align,combine,use,link-class
;	  DefCode	[{EXPORT,IMPORT,PRIVATE,LOCAL}[,class[,{C,PASCAL}]]]
;	  EndCode
;	  DefData	[{EXPORT,IMPORT,PRIVATE,LOCAL}[,class[,{C,PASCAL}]]]
;	  EndData
;	  DefCon	constant-name,value
;	  DefFn 	function-name
;
;	Always enabled:
;
;	  B1EMac	[rep] {ins ...,movs ...,stos ...}
;	  Break 	subtitle
;	  Procedure	name[,distance[,scope[,treg[,abase[,cseg[,blank]]]]]]
;	  EndProc	name[,NOCHECK]
;	  ProcError	message
;	  GenPublic	name[,local]
;	  GenHybrid	name[,[local] [,usecall]]
;	  GenFar32	name[,[local] [,usecall]]
;	  Entry 	name[,distance[,scope]]
;	  ArgVar	name,{type,length}[,lowname[,highname[,nowarn]]]
;	  LocalVar	name,{type,length}[,lowname[,highname[,pad]]]
;	  EnterProc	[varlist]
;	  LeaveProc
;	  ExitProc
;	  FallInto	procedure-name
;	  FallFrom	procedure-name
;	  SaveReg	[reglist]
;	  RestoreReg	[reglist]
;	  CPUMode	{cpu,RESET}
;	  CallFn	name[,arglist]
;	  PCall 	name[,arglist]
;	  CCall 	name[,arglist][,modifiers]
;	  BugBug	date,initials,note
;	  movzxESP						;whs
;	  SSToDS_PS	dest-reg[,source]			;whs
;	  DSToSS_PS	dest-reg[,source]			;whs
;	  SSToDS	dest-reg[,source]
;	  DSToSS	dest-reg[,source]
;	  PopAll
;	  PushAll
;
;	  IODELAY
;	  RETC		[n]
;	  RETP		[n]
;	  RETD		[n]
;	  RETND 	[n]
;	  RETFD 	[n]
;	  RETW		[n]
;	  RETNW 	[n]
;	  RETFW 	[n]
;	  PUSHD 	value
;	  POPD		value
;	  LOADSL	reg,sel
;	  LOADAR	reg,sel
;	  VERIFYREAD	sel,scratch-reg
;	  VERIFYWRITE	sel,scratch-reg
;
;	  FLATOffset			;text-macros
;
;	MAGIC DEFINES
;
;	  NOALIGN     Disables automatic data alignment
;	  NOBUGBUG    Disables BUGBUG messages
;	  ERRBUGBUG   Forces compilation error if any BUGBUGs exist
;
;	MODIFICATION HISTORY
;	10/03/88    JTP     Created.
;	11/29/88    JTP     Added new macros (DefCode, DefData, etc)
;	12/15/88    JTP     Added auto-alignment-and-warning for data
;			    according to segment type (can be disabled if
;			    NOALIGN defined)
;	07/17/90    JTP     Added andb, orb, testb, btrx and bsrx macros
;

NULL	equ	0
FALSE	equ	0
TRUE	equ	-1

?C	equ 1		; constants for ?model
?PASCAL equ 2

?PUBLIC equ 1		; constants for ?declare
?EXTRN	equ 2

?CS_16bit equ 0 	; constants for ?cstype
?CS_32bit equ 1
?cstype  = ?CS_16bit
?wdsz	 = 2
?cpumode = 286

?nsegs	 = 0		; counter for segment nesting
?nstrucs = 0		; counter for structure nesting
?nfields = 0		; tracks field # within a structure

?segname equ <> 	; initialize variables for DefCode/DefData....
?curseg  equ <>
?model	 = NULL
?declare = NULL

?case	 = TRUE 	; assume case-sensitivity
ifdef ?CASE
  ?case  = FALSE	; apparently, a bad assumption...
endif

;*** Text-macros

FLAToffset equ <offset FLAT:>

ifdef INCL_DEF

;*** DefEntry - Declare a new function prototype-type
;
;   ENTRY
;	dtyp	    prototype-type name
;

DefEntry macro dtyp
      dtyp macro pname,args
	     ifndef pname
	       ?&&pname = 0
	       irp x,<args>
		 ifdef INCL_TYPES
		   ifidn <x>,<...>
		     ?&&pname = -1
		     exitm
		   elseifidni <x>,<void>
		     ?&&pname = 0
		     exitm
		   elseifndef x
		    if1
		     %out Unknown type (&x) in function:  &pname
		    endif
		   endif
		 endif
		 ?&&pname = ?&&pname + 1
	       endm
	     endif
	   endm
	 endm

;*** DefType - Define a type
;
;   Used to define new type-macros.
;
;   ENTRY
;	type	the type (a type-macro name)
;	dtyp	the type declarator (eg, db, dw, structure name, etc)
;	defval	a default initial value; 0 if unspecified
;	defrep	a default repeat count; 1 if unspecified (ie, # elements)
;
;   EXAMPLES
;	DefType HVDM,dd
;
;	  becomes:
;
;	HVDM macro var,val,rep
;	     ?DefVar <dd>,var,<val>,<>,<rep>,<>
;	     endm
;
;	DefType NAME,db,byte,' ',8
;
;	  becomes
;
;	NAME macro var,val,rep
;	     ?DefVar <db>,var,<val>,<' '>,<rep>,<8>
;	     endm
;

DefType macro type,dtyp,defval,defrep
	  ifdef ?&dtyp
	    @&type catstr @&dtyp
	    ?&type = ?&dtyp
  type	    macro   var,val,rep
	     % ?DefVar	<@&dtyp>,var,<val>,<defval>,<rep>,<defrep>
	    endm
	  else
	    @&type equ <dtyp>
	    ifidni <dtyp>,<db>
	      ?&type = 1
	    elseifidni <dtyp>,<dw>
	      ?&type = 2
	    elseifidni <dtyp>,<dd>
	      ?&type = 4
	    elseifidni <dtyp>,<df>
	      ?&type = 6
	    elseifidni <dtyp>,<dq>
	      ?&type = 8
	    elseifidni <dtyp>,<dt>
	      ?&type = 10
	    elseifnb <dtyp>
	      ?&type = size dtyp
	    endif
  type	    macro   var,val,rep
	      ?DefVar  <dtyp>,var,<val>,<defval>,<rep>,<defrep>
	    endm
	  endif
	endm

;*** DefStruc - Define a structure-type
;
;   Used to define new structure-type-macros.
;
;   ENTRY
;	type	the type (a type-macro name;  optional)
;	dtyp	the type declarator (eg, crf_s, ptda_s, etc;  optional)
;
;   EXAMPLES
;	DefStruc
;	  ULONG crf_EDI
;	  ULONG crf_ESI
;	EndStruc CRF,crf_s
;
;	  becomes:
;
;	crf_s struc
;	 crf_EDI dd 0
;	 crf_ESI dd 0
;	crf_s ends
;	CRF  macro var,val,rep
;	     ?DefVar <crf_s>,var,<val>,<>,<rep>,<>
;	     endm
;

DefStruc macro type,dtyp
	   if ?nstrucs
	     if1
	       %out Nested structure definitions not supported
	       .err
	     endif
	   else
	     ?nstrucs = ?nstrucs + 1
	     ?nfields = 0
	     ?OPENSTRUC macro
	     dtyp struc
	     endm
	     ?CLOSESTRUC macro
	     dtyp ends
	     endm
	     ?TYPE macro mtyp
	       DefType mtyp,dtyp
	     endm
	   endif
	 endm

;*** EndStruc - End a structure-type definition (see DefStruc)
;
;   ENTRY
;	type	the type (a type-macro name)
;	dtyp	the type declarator (eg, crf_s, ptda_s, etc;
;		optional if specified in the DefStruc directive)
;

EndStruc macro	type,dtyp
	   ifnb <dtyp>
	     ?OPENSTRUC macro
	     dtyp struc
	     endm
	     ?CLOSESTRUC macro
	     dtyp ends
	     endm
	   endif
	   ?OPENSTRUC
	   if ?nfields GE 1
	   ?F1
	   endif
	   if ?nfields GE 2
	   ?F2
	   endif
	   if ?nfields GE 3
	   ?F3
	   endif
	   if ?nfields GE 4
	   ?F4
	   endif
	   if ?nfields GE 5
	   ?F5
	   endif
	   if ?nfields GE 6
	   ?F6
	   endif
	   if ?nfields GE 7
	   ?F7
	   endif
	   if ?nfields GE 8
	   ?F8
	   endif
	   if ?nfields GE 9
	   ?F9
	   endif
	   if ?nfields GE 10
	   ?F10
	   endif
	   if ?nfields GE 11
	   ?F11
	   endif
	   if ?nfields GE 12
	   ?F12
	   endif
	   if ?nfields GE 13
	   ?F13
	   endif
	   if ?nfields GE 14
	   ?F14
	   endif
	   if ?nfields GE 15
	   ?F15
	   endif
	   if ?nfields GE 16
	   ?F16
	   endif
	   if ?nfields GE 17
	   ?F17
	   endif
	   if ?nfields GE 18
	   ?F18
	   endif
	   if ?nfields GE 19
	   ?F19
	   endif
	   if ?nfields GE 20
	   ?F20
	   endif
	   if ?nfields GE 21
	   ?F21
	   endif
	   if ?nfields GE 22
	   ?F22
	   endif
	   if ?nfields GE 23
	   ?F23
	   endif
	   if ?nfields GE 24
	   ?F24
	   endif
	   if ?nfields GE 25
	   ?F25
	   endif
	   if ?nfields GE 26
	   ?F26
	   endif
	   if ?nfields GE 27
	   ?F27
	   endif
	   if ?nfields GE 28
	   ?F28
	   endif
	   if ?nfields GE 29
	   ?F29
	   endif
	   if ?nfields GE 30
	   ?F30
	   endif
	   if ?nfields GE 31
	   ?F31
	   endif
	   if ?nfields GE 32
	   ?F32
	   endif
	   if ?nfields GE 33
	   ?F33
	   endif
	   if ?nfields GE 34
	   ?F34
	   endif
	   if ?nfields GE 35
	   ?F35
	   endif
	   if ?nfields GE 36
	   ?F36
	   endif
	   if ?nfields GE 37
	   ?F37
	   endif
	   if ?nfields GE 38
	   ?F38
	   endif
	   if ?nfields GE 39
	   ?F39
	   endif
	   if ?nfields GE 40
	   ?F40
	   endif
	   if ?nfields GE 41
	   ?F41
	   endif
	   if ?nfields GE 42
	   ?F42
	   endif
	   if ?nfields GE 43
	   ?F43
	   endif
	   if ?nfields GE 44
	   ?F44
	   endif
	   if ?nfields GE 45
	   ?F45
	   endif
	   if ?nfields GE 46
	   ?F46
	   endif
	   if ?nfields GE 47
	   ?F47
	   endif
	   if ?nfields GE 48
	   ?F48
	   endif
	   if ?nfields GE 49
	   ?F49
	   endif
	   if ?nfields GE 50
	   ?F50
	   endif
	   if ?nfields GE 51
	   ?F51
	   endif
	   if ?nfields GE 52
	   ?F52
	   endif
	   if ?nfields GE 53
	   ?F53
	   endif
	   if ?nfields GE 54
	   ?F54
	   endif
	   if ?nfields GE 55
	   ?F55
	   endif
	   if ?nfields GE 56
	   ?F56
	   endif
	   if ?nfields GE 57
	   ?F57
	   endif
	   if ?nfields GE 58
	   ?F58
	   endif
	   if ?nfields GE 59
	   ?F59
	   endif
	   if ?nfields GE 60
	   ?F60
	   endif
	   if ?nfields GE 61
	   ?F61
	   endif
	   if ?nfields GE 62
	   ?F62
	   endif
	   if ?nfields GE 63
	   ?F63
	   endif
	   if ?nfields GE 64
	   ?F64
	   endif
	   if ?nfields GE 65
	   ?F65
	   endif
	   if ?nfields GE 66
	   ?F66
	   endif
	   if ?nfields GE 67
	   ?F67
	   endif
	   if ?nfields GE 68
	   ?F68
	   endif
	   if ?nfields GE 69
	   ?F69
	   endif
	   if ?nfields GE 70
	   ?F70
	   endif
	   if ?nfields GE 71
	   ?F71
	   endif
	   if ?nfields GE 72
	   ?F72
	   endif
	   if ?nfields GE 73
	   ?F73
	   endif
	   if ?nfields GE 74
	   ?F74
	   endif
	   if ?nfields GE 75
	   ?F75
	   endif
	   if ?nfields GE 76
	   ?F76
	   endif
	   if ?nfields GE 77
	   ?F77
	   endif
	   if ?nfields GE 78
	   ?F78
	   endif
	   if ?nfields GE 79
	   ?F79
	   endif
	   if ?nfields GE 80
	   ?F80
	   endif
	   if ?nfields GE 81
	   ?F81
	   endif
	   if ?nfields GE 82
	   ?F82
	   endif
	   if ?nfields GE 83
	   ?F83
	   endif
	   if ?nfields GE 84
	   ?F84
	   endif
	   if ?nfields GE 85
	   ?F85
	   endif
	   if ?nfields GE 86
	   ?F86
	   endif
	   if ?nfields GE 87
	   ?F87
	   endif
	   if ?nfields GE 88
	   ?F88
	   endif
	   if ?nfields GE 89
	   ?F89
	   endif
	   if ?nfields GE 90
	   ?F90
	   endif
	   if ?nfields GE 91
	   ?F91
	   endif
	   if ?nfields GE 92
	   ?F92
	   endif
	   if ?nfields GE 93
	   ?F93
	   endif
	   if ?nfields GE 94
	   ?F94
	   endif
	   if ?nfields GE 95
	   ?F95
	   endif
	   if ?nfields GE 96
	   ?F96
	   endif
	   if ?nfields GE 97
	   ?F97
	   endif
	   if ?nfields GE 98
	   ?F98
	   endif
	   if ?nfields GE 99
	   ?F99
	   endif
	   if ?nfields GE 100
	   ?F100
	   endif
	   if ?nfields GE 101
	   ?F101
	   endif
	   if ?nfields GE 102
	   ?F102
	   endif
	   if ?nfields GE 103
	   ?F103
	   endif
	   if ?nfields GE 104
	   ?F104
	   endif
	   if ?nfields GE 105
	   ?F105
	   endif
	   if ?nfields GE 106
	   ?F106
	   endif
	   if ?nfields GE 107
	   ?F107
	   endif
	   if ?nfields GE 108
	   ?F108
	   endif
	   if ?nfields GE 109
	   ?F109
	   endif
	   if ?nfields GE 110
	   ?F110
	   endif
	   if ?nfields GE 111
	   ?F111
	   endif
	   if ?nfields GE 112
	   ?F112
	   endif
	   if ?nfields GE 113
	   ?F113
	   endif
	   if ?nfields GE 114
	   ?F114
	   endif
	   if ?nfields GE 115
	   ?F115
	   endif
	   if ?nfields GE 116
	   ?F116
	   endif
	   if ?nfields GE 117
	   ?F117
	   endif
	   if ?nfields GE 118
	   ?F118
	   endif
	   if ?nfields GE 119
	   ?F119
	   endif
	   if ?nfields GE 120
	   ?F120
	   endif
	   if1
	     if ?nfields GT 120
	       %out More than 120 fields in structure-type:  &type
	       .err
	     endif
	   endif
	   ?CLOSESTRUC
	   ifb <dtyp>
	     ?TYPE type
	   else
	     DefType type,dtyp
	   endif
	   ?nstrucs = ?nstrucs - 1
	   rept ?nfields
	     ?Purge <?F>,%?nfields
	     ifdef DEBUG
	       if DEBUG eq 1
		 ?InvPrg <?D>,%?nfields
	       endif
	     endif
	     ?nfields = ?nfields - 1
	   endm
	 endm

;*** DefSeg - Define a segment
;
;   ENTRY
;
;   EFFECTS
;

DefSeg macro sname,class,stype,align,combine,use,lclass
	 ifdif <stype>,<CODE>
	   ifdif <stype>,<DATA>
	     if1
	       %out Unknown type (stype) for segment:  sname
	     endif
	   endif
	 endif
	 @&class&_&stype equ <sname>   ;; save away the real segment name
	 ?&class&_&stype equ <use>     ;; save away use (ie, USE16, etc)
	 sname SEGMENT align combine use lclass
	 sname ENDS
       endm

;*** DefCode - Begin code definitions
;
;   ENTRY
;	scope	PRIVATE (default), EXPORT or IMPORT
;
;	class	GLOBAL, INIT, etc.  (default is none)
;
;	model	C or PASCAL or none;  if C, function names are prepended
;		with an underscore;  if PASCAL, names are upper-cased
;
;	type	USE16 or USE32 or none
;
;   EFFECTS
;	Sets ?nsegs, ?segname, ?cstype, ?model and ?declare
;

DefCode macro scope,class,model,type
	  ?PushSeg %?nsegs
	  ifdef @&class&_CODE
	    ?segname catstr @&class&_CODE
	    ?SetType %?&class&_CODE
	  elseifnb <class>
	    ?segname equ <class>
	    ?SetType type
	  endif
	  ?OpenSeg scope,class,model,type
	 endm

;*** EndCode - End code definitions
;
;   ENTRY
;	Same as above (although everything is optional)
;

EndCode macro scope,class,model,type
	  ?CloseSeg scope,class,model,type,DefCode
	  ?PopSeg %?nsegs
	 endm

;*** DefData - Begin data definitions
;
;   ENTRY
;	scope	PRIVATE (default), EXPORT or IMPORT
;
;	class	GLOBAL, INIT, INSTANCE, etc.  (default is none)
;
;	model	C or PASCAL or none;  if C, function names are prepended
;		with an underscore;  if PASCAL, names are upper-cased
;   EFFECTS
;	Sets ?nsegs, ?segname, ?cstype, ?model and ?declare
;

DefData macro scope,class,model,type
	  ?PushSeg %?nsegs
	  ifdef @&class&_DATA
	    ?segname catstr @&class&_DATA
	    ?SetType %?&class&_DATA
	  elseifnb <class>
	    ?segname equ <class>
	    ?SetType type
	  endif
	  ?OpenSeg scope,class,model,type
	 endm

;*** EndData - End data definitions
;
;   ENTRY
;	Same as above (although everything is optional)
;

EndData macro scope,class,model,type
	  ?CloseSeg scope,class,model,type,DefData
	  ?PopSeg %?nsegs
	 endm

;*** DefCon - Define a constant
;
;   ENTRY
;	name	name of constant
;	val	value of constant
;

DefCon macro name,val
	 ?DefVar <equ>,name,val
       endm

;*** DefFn - Define function name(s)
;
;   ENTRY
;	name(s)     name(s) of function
;

DefFn macro n1,n2,n3,n4,n5,n6,n7,n8
	irp n,<n1,n2,n3,n4,n5,n6,n7,n8>
	  ifnb <n>
	    if ?model EQ ?C
	      n equ <_&&n>
	    elseif ?model EQ ?PASCAL
	      if ?case
		?ToUpper n
		% ifdif <n>,<?upper>
		  n catstr ?upper
		endif
	      endif
	    endif
	    if ?declare EQ ?EXTRN
	      % ?DefExtrn n,near
	    elseif ?declare EQ ?PUBLIC
	      % ?DefPublic n
	    endif
	  endif
	endm
      endm

;*** ?DefVar - Define a variable, with defaults
;
;   Used by DefType to create new type-macros, which simply
;   use this as the worker macro.
;
;   See ?DefMem for details.  All this macro does is substitute
;   the value of defaults (defval and defrep) for the other inputs.

?DefVar macro	dtyp,var,val,defval,rep,defrep
	  ?t equ <var>
	  if ?nstrucs
	    ?nfields = ?nfields + 1
	  else
	    ifnb <var>
	      if ?model EQ ?C
		ifndef var
		  var equ <_&var>
		  ?t catstr var
		endif
	      elseif ?model EQ ?PASCAL
		ifndef var
		  if ?case
		    ?ToUpper var
		    % ifdif <var>,<?upper>
		      var catstr ?upper
		    endif
		    ?t catstr ?upper
		  endif
		endif
	      endif
	    endif
	  endif
	  ifb <val>
	    ifb <rep>
	      % ?DefMem <dtyp>,?t,<defval>,<defrep>,%?nfields
	    else
	      % ?DefMem <dtyp>,?t,<defval>,<rep>,%?nfields
	    endif
	  else
	    ifb <rep>
	      % ?DefMem <dtyp>,?t,<val>,<defrep>,%?nfields
	    else
	      % ?DefMem <dtyp>,?t,<val>,<rep>,%?nfields
	    endif
	  endif
	endm

DEFdb	equ	<byte>		;text-macros used by ?DefMem macro
DEFdw	equ	<word>
DEFdd	equ	<dword>
DEFdf	equ	<fword>
DEFdq	equ	<qword>
DEFdt	equ	<tbyte>
DEFequ	equ	<abs>
DEFDB	equ	<byte>
DEFDW	equ	<word>
DEFDD	equ	<dword>
DEFDF	equ	<fword>
DEFDQ	equ	<qword>
DEFDT	equ	<tbyte>
DEFEQU	equ	<abs>

;*** ?DefMem - Define actual storage
;
;   Used by ?DefVar to actually define a variable.
;
;   ENTRY
;	dtyp	the type declarator (db, structure name, etc)
;	var	variable name
;	val	an initial value; 0 if unspecified
;	rep	a repeat count; 1 if unspecified (ie, # elements)
;	fn	field number, if within a structure definition

?DefMem macro dtyp,var,val,rep,fn
	  ?fStruc = 1
	  irp x,<db,dw,dd,df,dq,dt,equ>
	    ifidni <dtyp>,<x>
	      ?fStruc = 0
	    endif
	  endm
	  if ?declare EQ ?EXTRN
	    if ?nstrucs EQ 0
	      ifnb <var>
		if ?fStruc
		  % ?DefExtrn var,byte
		else
		  % ?DefExtrn var,%DEF&dtyp
		endif
	      endif
	    endif
	  endif
	  ?fExtrn = 0
	  ifnb <var>
	    ?fExtrn = (.type var) and 80h
	  endif
	  if ?nstrucs
	    ?DefField fn
	  endif
	  if ?fExtrn EQ 0
	    if ?fStruc
	      if ?nstrucs
		ifb <rep>
		  ?DefField fn,var,<db size dtyp dup(?)>
		else
		  ?DefField fn,var,<db size dtyp * rep dup(?)>
		endif
		?fExtrn = 80h
	      endif
	    endif
	  endif
	  if ?fExtrn EQ 0
	    ifb <val>
	      ifb <rep>
		ifb <dtyp>
		  ifnb <var>
		    if ?fStruc
		      % ?DefLabel var,byte
		    else
		      % ?DefLabel var,DEF&dtyp
		    endif
		  endif
		else
		  if ?fStruc
		    ?Alloc var,dtyp,<<>>,1
		  else
		    if ?nstrucs EQ 0
		      ?Alloc var,dtyp,0,0
		    else
		      ?DefField fn,var,<dtyp 0>
		    endif
		  endif
		endif
	      else
		if ?fStruc
		  ?Alloc var,dtyp,<rep dup(<>)>,1
		else
		  if ?nstrucs EQ 0
		    ?Alloc var,dtyp,<rep dup(0)>,0
		  else
		    ?DefField fn,var,<dtyp rep dup(0)>
		  endif
		endif
	      endif
	    else
	      ifb <rep>
		if ?fStruc
		  ?Alloc var,dtyp,<<val>>,1
		else
		  if ?nstrucs EQ 0
		    ?Alloc var,dtyp,<val>,0
		  else
		    ?DefField fn,var,<dtyp val>
		  endif
		endif
	      else
		if ?fStruc
		  ?Alloc var,dtyp,<rep dup(<val>)>,1
		else
		  if ?nstrucs EQ 0
		    ?Alloc var,dtyp,<rep dup(val)>,0
		  else
		    ?DefField fn,var,<dtyp rep dup(val)>
		  endif
		endif
	      endif
	    endif
	  endif
	  if ?declare EQ ?PUBLIC
	    if ?nstrucs EQ 0
	      ifnb <var>
		% ?DefPublic var
	      endif
	    endif
	  endif
	endm

endif ; INCL_DEF

;*** Break - break a listing and give new subtitle
;
;   ENTRY   subtitle = new subtitle to use
;
;   EXIT    listing starts on new page with new subtitle

Break	macro	subtitle
	subttl	subtitle
	ifdef NOPAGE
	    page 255
	else
	    page
	endif
endm

F16PRE_ equ	<f_>		; far16 label prefix
F32PRE_ equ	<g_>		; far32 label prefix
HYBPRE_ equ	<h_>		; faronly label prefix

;*** Procedure - declares a procedure
;
;   This produces the appropriate "proc" and "public" pseudo ops
;   and initializes a number of global symbols used by ArgVar,
;   LocalVar, EnterProc, SaveReg, RestoreReg, LeaveProc, and
;   EndProc.  Procedures may not nest; nesting will cause assembler
;   errors.  If declared hybrid or faronly, a far 16 bit procedure,
;   HYBPRE_&name, is generated.  If declared far16, a far 32 bit procedure,
;   F16PRE_&name, is generated.
;
;   ENTRY   name     = name of procedure
;	    distance = "near" - a near procedure can be called
;		       with the "call" instruction.
;		     = "far" - a far procedure can be called with
;		       the "call" instruction.  Can't be combined.
;		     = "hybrid" - a hybrid procedure can be
;		       called with the "call" instruction (for near
;		       call) or the CALLFAR macro (for far call).
;		       Same as <near,faronly>.	Can only be used
;		       in 16 bit code.
;		     = "faronly" - a faronly procedure can
;		       only be called with the CALLFAR macro.
;		       Can only be used in 16 bit code.
;		     = blank - defaults to near
;		     = "far16" - 16 bit procedure that can be called
;		       far by 32-bit code (via the CALL16 macro).
;		       Can only be used in 16 bit code.
;		     If in a 16 bit code segment,
;		     <near,far16,faronly,hybrid> may be combined.
;	    scope    = "local" - don't make "name" public
;		     = "farlocal" - hybrid procedure will have far
;		       local and near public attribute.
;		     = "nearlocal" - hybrid procedure will have near
;		       local and far public attributes.
;		     = blank - defaults to public for both near and
;		       far attributes.
;     treg  scratch register - OBSOLETE!!
;	    If non-blank, this register is used to optimizize far calls
;	    to hybrid procedures, at the expense of near calls.
;	    By default, near calls are the more efficient of the two.
;     abase  the number of bytes to skip for calculating the ArgVar
;	     offsets. The default (if this is blank) depends on the
;	     "distance" variable.  16 bit "NEAR" is 4 and "FAR" is 6.
;	     If set to the string "ESP", then ArgVar uses ESP instead
;	     of [E]BP to refer to parameters on the stack, and assumes
;	     that [E]BP is not saved on the stack.  LocalVar, EnterProc,
;	     and LeaveProc are then illegal to use within the procedure.
;
;	    ?argfar = 0 - ok to use ArgVar in this procedure
;		    = 4 - need special care using ArgVar.  Also used
;			  by user as a suffix to access arguments
;			  pushed by far callers.  See example at top.
;			  near+faronly (or hybrid)
;		    = 8 - need special care using ArgVar.  near+far16
;		    = -1 - Don't use ArgVar.  near+far16+faronly

?frame	     =	0	; initial
?aframe      =	0	; initial
?abase	     =	0	; initial
?stackdepth  =	0	; initial stack size
?initstack   =	0	; initial stack size
?local	     =	0	; local proc flag
?distance    =	0	; near/far, 16/32 bit, faronly, bpframe distance flags
?olddistance =	0	; outer scope distance flags
?depth	     =	0	; procedure nesting level
?argfar      =	0	; ok to use ArgVar in procedure

; ?distance variable bit fields:
?PD_NEAR	equ	0001h	; callable near (16 or 32 bit)	 "name"
?PD_FAR 	equ	0002h	; 16-bit far routine		 "name"
?PD_FARONLY	equ	0004h	; callable through CALLFAR macro "HYBPRE_&name"
?PD_FAR16	equ	0008h	; 16-bit callable from 32-bit	 "F16PRE_&name"
?PD_FAR32	equ	0010h	; 32-bit callable from 16-bit	 "F32PRE_&name"
?PD_ESPFRAME	equ	0020h	; uses ESP ArgVar frame
?PD_ENTERED	equ	0040h	; EnterProc seen
?PD_DIST	equ	0080h	; distance arg seen
?PD_CDECL	equ	0100h	; cdecl seen
?PD_PASCAL	equ	0200h	; pascal seen

?PD_DISTMASK	equ	(?PD_NEAR+?PD_FAR+?PD_FARONLY+?PD_FAR16+?PD_FAR32)

; ?local variable bit fields:
?LC_NEARLOCAL	equ	0001h	; near label is local
?LC_FARLOCAL	equ	0002h	; faronly label is local
?LC_FAR16LOCAL	equ	0004h	; far16 label is local
?LC_FAR32LOCAL	equ	0008h	; far32 label is local
?LC_LOCAL	equ	0010h	; all labels are local

ProcError macro msg
%   %out @FileName.asm: Error in unknown Procedure: msg
endm

CatPrefix macro before, prefix, name, after
    .lall
    before prefix&name after
    .xall
endm

RETC macro n
    ?bm1 = ?aframe		;; assume new C calling convention (or pascal)
    if (?distance and ?PD_CDECL)
	?bm1 = 0		;; oops, cdecl - don't clean off args
    endif
    ifnb <n>
	?bm1 = n
    endif
    ?Eval RETP,%?bm1
endm

Procedure macro   name,distance,scope,treg,abase,blank
ifdef ALIGNCODE
    align 4
endif
    ProcError macro msg
%	%out @FileName.asm: Error in Procedure name: msg
    endm
    if2
	ifnb <treg>
	    ProcError <treg_arg must be _blank: treg>
	    .err
	endif
	ifnb <blank>
	    ProcError <Too many parameters>
	    .err
	endif
	if ?depth gt 1
	    ProcError <Nesting too deep>
	    .err
	endif
    endif
    ?depth	 = ?depth + 1
    ?olddistance = ?distance	;; save previous value
    ?frame	 = 0
    ?aframe	 = 0
    ?initstack	 = ?stackdepth	;; beginning of procedure
    ?local	 = 0		;; default to public
    ?distance	 = 0		;; default is set elsewhere
    ?argfar	 = 0		;; okay to use ArgVar
    ?nfields	 = 0		;; used here to count ArgVars
    ?name	 equ <name>	;; save current procedure name
    ?wdsz	 = 2		;; default word size
    if (?cstype eq ?CS_32bit)
	?wdsz = 4
    endif
    ifnb <scope>
	irp x,<scope>
	    ifidni <x>,<nearlocal>		;; near is local
		?local = ?local or ?LC_NEARLOCAL
	    elseifidni <x>,<far16local> 	;; far16 is local
		?local = ?local or ?LC_FAR16LOCAL
	    elseifidni <x>,<far32local> 	;; far16 is local
		?local = ?local or ?LC_FAR32LOCAL
	    elseifidni <x>,<farlocal>		;; faronly is local
		?local = ?local or ?LC_FARLOCAL
	    elseifidni <x>,<local>		;; all are local
		?local = ?local or ?LC_NEARLOCAL or ?LC_FARLOCAL \
				or ?LC_FAR16LOCAL or ?LC_FAR32LOCAL \
				or ?LC_LOCAL
	    elseif2
		ProcError <Bad scope_arg: x>
		.err
	    endif
	endm
    endif
    .lall
    ?bm1 = ?local				;; only for listing files
    .xall

    ifnb <distance>
	irp x,<distance>
	    ifidni <x>,<near>		;; is near
		?distance = ?distance or ?PD_NEAR or ?PD_DIST
	    elseifidni <x>,<far16>	;; is 16 bit callable from 32
		?distance = ?distance or ?PD_FAR16 or ?PD_DIST
	    elseifidn <x>,<far32>	;; is 32 bit callable from 16
		?distance = ?distance or ?PD_FAR32 or ?PD_DIST
	    elseifidni <x>,<far>	;; is 16 bit far
		?distance = ?distance or ?PD_FAR or ?PD_DIST
	    elseifidni <x>,<faronly>	;; is 16 bit faronly
		?distance = ?distance or ?PD_FARONLY or ?PD_DIST
	    elseifidni <x>,<hybrid>	;; is 16-bit near/faronly hybrid
		?distance = ?distance or ?PD_NEAR or ?PD_FARONLY or ?PD_DIST
	    elseifidn <x>,<cdecl>	;; is cdecl (caller cleanup)
		?distance = ?distance or ?PD_CDECL
	    elseifidn <x>,<pascal>	;; is pascal (callee cleanup)
		?distance = ?distance or ?PD_PASCAL
	    elseif2
		ProcError <Bad distance_arg: x>
		.err
	    endif
	endm

	;; If both types of 16 bit thunks are required, add a near
	;; procedure body, and have both thunks call it near:

	if ((?distance and ?PD_DISTMASK) eq (?PD_FAR16 or ?PD_FARONLY))
	    ?distance = ?distance or ?PD_NEAR
	endif

	if2
	    ;; Disallow 16 bit thunks in 32 bit code.

	    if (?cstype eq ?CS_32bit)
		if (?distance and (?PD_FAR16 or ?PD_FARONLY))
		    ProcError <16-bit Procedure in 32-bit code>
		    .err
		endif
	    else
		if (?distance and ?PD_FAR32)
		    ProcError <32-bit Procedure in 16-bit code>
		    .err
		endif
	    endif
	    if (?distance and ?PD_FAR)
		if ((?distance and ?PD_DISTMASK) ne ?PD_FAR)
		    ProcError <distance_arg cannot use FAR combinations>
		    .err
		endif
	    endif
	    if ((?distance and (?PD_CDECL or ?PD_PASCAL)) eq \
		(?PD_CDECL or ?PD_PASCAL))

		ProcError <pascal and cdecl conflict>
		.err
	    endif
	endif
    endif
    if ((?distance and ?PD_DIST) eq 0)
	?distance = ?distance or ?PD_NEAR	;; default is near
    endif
    ifndef STDCALL
	if ((?distance and ?PD_PASCAL) eq 0)
	    ?distance = ?distance or ?PD_CDECL	;; force cdecl override
	endif
    endif
    .lall
    ?bm1 = ?distance				;; only for listing files
    .xall

    RETP macro n				;; use masm's default RET type
	ret n
    endm

    ;; if near, generate any needed thunks first

    if (?distance and ?PD_NEAR)
	if (?distance and ?PD_FAR16)		;; if far16 thunk needed
	    if (?distance and ?PD_FARONLY)	;; and if faronly thunk needed,
		if (?local and ?LC_FARLOCAL)	;; use call for faronly thunk
		    GenHybrid name,<local>,<usecall>
		else
		    GenHybrid name,,<usecall>
		endif
	    endif
	    if (?local and ?LC_FAR16LOCAL)	;; far16 thunk falls through
		?Gen16 name,local
	    else
		?Gen16 name
	    endif
						;; far16 thunk not needed
	elseif (?distance and ?PD_FARONLY)	;; if faronly thunk needed,
	    if (?local and ?LC_FARLOCAL)	;; faronly thunk falls through
		GenHybrid name,local
	    else
		GenHybrid name
	    endif
	elseif (?distance and ?PD_FAR32)	;; if far32 thunk needed
	    if (?local and ?LC_FAR32LOCAL)	;; use call for far32 thunk
		GenFar32 name,local
	    else
		GenFar32 name
	    endif
	endif
	if (?local and ?LC_NEARLOCAL)		;; generate main proc label
	    GenPublic name,<local>,code
	else
	    GenPublic name,,code
	endif
	name proc near				;; main proc is near
    else

    ;; main proc isn't near; may need to prepend HYBPRE_, F16PRE_ or F32PRE_

	if (?distance and ?PD_FARONLY)
	    if (?local and ?LC_FARLOCAL)	;; generate proc HYBPRE_label
		?PrePublic %HYBPRE_, name,<local>,code
	    else
		?PrePublic %HYBPRE_, name,,code
	    endif
	    CatPrefix , %HYBPRE_, <name>, <proc far> ;; main proc is far
	endif
	if (?distance and ?PD_FAR16)
	    if (?local and ?LC_FAR16LOCAL)	;; generate proc F16PRE_label
		?PrePublic %F16PRE_, name,<local>,code
	    else
		?PrePublic %F16PRE_, name,,code
	    endif
	    CatPrefix , %F16PRE_, <name>, <proc far> ;; main proc is far
	    RETP macro n			;; use RETFD
		RETFD n
	    endm
	endif
	if (?distance and ?PD_FAR32)
	    if (?local and ?LC_FAR32LOCAL)	;; generate proc F32PRE_label
		?PrePublic %F32PRE_, name,<local>,code
	    else
		?PrePublic %F32PRE_, name,,code
	    endif
	    CatPrefix , %F32PRE_, <name>, <proc far> ;; main proc is far
	endif
	if (?distance and ?PD_FAR)
	    if (?local and ?LC_LOCAL)		;; generate main proc label
		GenPublic name,<local>,code
	    else
		GenPublic name,,code
	    endif
	    name proc far			;; main proc is far
	endif
    endif

    if (?distance and ?PD_NEAR)
	?abase = 4 + 4			;; ret address, EBP are 4 bytes each?
	if (?cstype ne ?CS_32bit)
	    ?abase = 2 + 2		;; ret address, BP are 2 bytes each?
	endif
	if (?distance and (?PD_FAR32 or ?PD_FAR16))
	    ?argfar = 8 		;; extra 32 bit far call on stack
	endif
	if (?distance and ?PD_FARONLY)
	    ife ?argfar
		?argfar = 4		;; extra 16 bit far call on stack
	    else
		?argfar = -1		;; doesn't work when thunks combined
	    endif
	endif
    else
	if (?distance and ?PD_FAR32)
	    ?abase = 8 + 4		;; ret addr is 8 bytes, EBP is 4 bytes!
	endif
	if (?distance and ?PD_FAR16)
	    ?abase = 8 + 2		;; ret addr is 8 bytes, BP is 2 bytes!
	endif
	if (?distance and ?PD_FARONLY)
	    ?abase = 4 + 2		;; ret addr is 4 bytes, BP is 2 bytes!
	endif
	if (?distance and ?PD_FAR)
	    ?abase = 8 + 4		;; ret addr is 8 bytes, EBP is 4 bytes?
	    if (?cstype ne ?CS_32bit)
		?abase = 4 + 2		;; ret addr is 4 bytes, BP is 2 bytes!
	    endif
	endif
    endif
    ifnb <abase>			;; if abase is not blank, then use it
	ifidni <abase>,<esp>		;; if abase is ESP, then no [E]BP frame
	    ?abase = ?abase - 4 	;; uncompensate for EBP
	    if (?cstype ne ?CS_32bit)
		?abase = ?abase + 2	;; oops.  uncompensate only for BP
	    endif
	    ?distance = ?distance or ?PD_ESPFRAME
	else
	    ?abase = abase
	endif
    endif
    .lall
    ?bm1 = ?abase				;; only for listing files
    ?bm1 = ?argfar				;; only for listing files
    .xall
endm

;*** EndProc - End a procedure
;
;   This ends a procedure declaration by generating appropriate
;   endp pseudo ops. It also checks to make sure the number of
;   registers saved with SaveReg are the same as the number of
;   registers restored with RestoreReg within the procedure body.
;   This check is not fool-proof because it assumes all the
;   SaveReg and RestoreReg macros will be executed exactly once
;   during run time, which may not be a valid assumption.
;
;   ENTRY   name = name of procedure to end
;	    chk  = blank - verify #registers saved by SaveReg
;		   and restored by RestoreReg are the same.
;		 = "NoCheck" - don't check
;   (global var)
;	    ?depth	 = level of Procedure nested.  Max is 1.
;	    ?distance	 = set by Procedure macro
;	    ?olddistance = value of ?distance when Procedure macro
;			   was called.
;
;   EXIT    "endp" with appropriate label generated.
;   (global variables)
;	    ?depth   = decremented
;	    ?distance = restored to previous value when
;			Procedure macro was called.
;
;   SEE ALSO: Procedure, ArgVar, LocalVar, EnterProc, SaveReg,
;	      RestoreReg, LeaveProc, EndProc.

EndProc macro name, chk
ifdef ALIGNCODE
    align 4
endif
    if2
	if (?distance and ?PD_ENTERED)
	    ifdif <chk>,<NoCheck>
		ProcError <EndProc invoked without LeaveProc>
		.err
	    endif
	endif
	ife ?depth
	    ProcError <EndProc without matching Procedure>
	    .err
	endif
	?bm1 = 0				;; assume no NoCheck
	ifnb <chk>
	    ifdif <chk>,<NoCheck>
		ProcError <EndProc: bad NoCheck arg: chk>
		.err
	    else
		?bm1 = 1			;; saw NoCheck
	    endif
	endif
	if (?bm1 eq 0)
	    if (?initstack ne ?stackdepth)	;; is it different?
		ProcError <SaveReg/RestoreReg mismatch>
	    endif
	endif
    endif
    if (?distance and ?PD_NEAR)
	name endp			;; procedure is near
    elseif (?distance and ?PD_FAR16)
	CatPrefix , %F16PRE_, <name>, <endp> ;; procedure is is far16
    elseif (?distance and ?PD_FAR32)
	CatPrefix , %F32PRE_, <name>, <endp> ;; procedure is is far32
    elseif (?distance and ?PD_FARONLY)
	CatPrefix , %HYBPRE_, <name>, <endp> ;; procedure is is faronly
    else
	name endp			;; procedure is far
    endif
    ?depth = ?depth - 1
    ?distance = ?olddistance		;; restore previous value
    ifdef KILLASSUMES
	assume ds:nothing, es:nothing, ss:nothing
	if (?cstype eq ?CS_32bit)
	    assume fs:nothing, gs:nothing
	endif
    endif
ifdef ALIGNCODE
    align 4
endif
endm

;*** GenPublic - generate a (unique) public label
;
;   Given a name, this macro tags a prefix of the form lxxx_
;   (where xxx is a unique number) to it to form a unique public
;   label labeling the current location. This is used for symbolic
;   debugger support.
;
;   ENTRY   name  = name to be made public
;	    local = blank - simply make "name" into a public label
;		      without tagging a prefix
;		    anything else - tag a prefix unless NOLOCAL
;		      is defined.
;   (global variables)
;	    NOLOCAL = if defined, make "name" into a public label
;		      without tagging a prefix regardless of what
;		      "local" is.
;
;   EXIT    "name" is declared public (with prefix attached if
;	    "local" is not blank or NOLOCAL is defined) and labels
;	    the current location.

GenPublic macro name,local,code        ;; generate a public or local symbol
    ifb <local> 			;; if local symbol not requested
	% ?DefPublic name
    else
	ifdef NOLOCAL			;; if all symbols should be public
	    % ?DefPublic name
	else
	    % ?DefPublic @FileName&&$&name
	    ifb <code>			;; allow masm PUBDEF DS association
		% ?Eval @FileName&&$&name <label byte>
	    else			;; avoid masm PUBDEF DS association
		% ?Eval @FileName&&$&name::
	    endif
	endif
    endif
endm

?PrePublic macro prefix,name,local,code
    GenPublic prefix&name,<local>,<code>
endm

;*** Entry - generate a near/hybrid/faronly entry into a procedure
;
;   This is used for creating additional entry points into a
;   procedure.
;
;   ENTRY   name     = name of entry point
;	    distance = blank - create near entry point
;		     = "near" - create near entry point
;		     = "far" - create far entry point
;		     = "faronly" - create faronly entry point
;		     = "far16" - create far16 entry point
;		     = "hybrid" - create hybrid entry point
;	    scope    = blank - make entry point public
;		     = non-blank - if NOLOCAL is defined, the entry
;				   point is still made public. Else
;				   it is made local (fake local,
;				   see the macro GENHYBRID).
;	    nocheck  = non-blank - defeat context checking
;   (global var)
;	    NOLOCAL  = undefined - "scope" controls the scope of the
;				   entry point
;		     = defined - make the entry point public
;				 regardless of "scope."
;
;   EXIT    new entry point and "public" pseudo op generated.
;
;   SEE ALSO: Procedure

?entrydistance = 0
Entry macro name,distance,scope,nocheck
local a
    ifnb <scope>
	if2
	    ifdifi <local>,<scope>
		%out Bad scope_arg scope in Entry name
		.err
	    endif
	endif
    endif
    ?entrydistance = ?PD_NEAR
    ifnb <distance>
	ifidni <distance>,<near>	;; was it near?
	    GenPublic name,<scope>	;; generate a public or local symbol
	    name::
	elseifidni <distance>,<far>	;; was it far?
	    ?entrydistance = ?PD_FAR
	    GenPublic name,<scope>	;; generate a public or local symbol
	    name::
	elseifidni <distance>,<far16>	;; was it far16?
	    ?entrydistance = ?PD_FAR16
	    ?PrePublic %F16PRE_, name,<scope> ;; generate a public/local symbol
	    CatPrefix , %F16PRE_, <name>, <::>
	elseifidn <distance>,<far32>	;; was it far32?
	    ?entrydistance = ?PD_FAR32
	    ?PrePublic %F32PRE_, name,<scope> ;; generate a public/local symbol
	    CatPrefix , %F32PRE_, <name>, <::>
	elseifidni <distance>,<faronly> ;; was it faronly?
	    ?entrydistance = ?PD_FARONLY
	    if ((?distance and (?PD_FAR16 or ?PD_NEAR)) eq ?PD_FAR16)
		?entrydistance = ?PD_FAR16
		jmp short a		;; other callers skip faronly linkage
	    endif
	    ?PrePublic %HYBPRE_, name,<scope> ;; generate a public/local symbol
	    CatPrefix , %HYBPRE_, <name>, <::>
	    if ((?distance and (?PD_FAR16 or ?PD_NEAR)) eq ?PD_FAR16)

		; If this is a FAR16 entry in a faronly procedure, munge
		; the 16:16 return address so we can do a common RETFD.

		.386p
		push	dword ptr ss:[esp]	; duplicate cs:ip on stack
		mov	word ptr ss:[esp+2],0	; zero high word of new eip
		shr	dword ptr ss:[esp+4],16 ; move cs to low word
		CPUMode reset
		a::
	    endif
	elseifidni <distance>,<hybrid>	;; was it hybrid?
	    jmp short name		;; previous callers skip hybrid linkage
	    GenHybrid name,<scope>
	    GenPublic name,<scope>	;; generate a public or local symbol
	    name::
	else
	    if2
		%out Bad distance_arg distance in Entry name
		.err
	    endif
	endif
    else
	GenPublic name,<scope>		;; generate a public or local symbol
	name::
    endif
    if2
	ifb <nocheck>
	    if ((?distance and (?PD_NEAR or ?entrydistance)) ne ?entrydistance)
		%out Entry name not in distance context
		.err
	    endif
	    if (?entrydistance and (?PD_FARONLY or ?PD_FAR16))
		if (?cstype eq ?CS_32bit)	;; invoked in 32 bit segment?
		    ProcError <Entry name with distance context in 32 bit code>
		    .err
		endif
	    endif
	endif
    endif
endm

;*** ArgVar - declare an argument (procedure parameter)
;
;   Assign "name" to an argument (pushed by caller) in the stack frame.
;
;   ENTRY   name     = name of argument
;	    length   = "BYTE"
;		     = "DBYTE" - double byte. Two new names
;			 "name&l" and "name&h" are created to refer
;			 to the low and high bytes.
;		     = "WORD"
;		     = "DWORD" - double word. Two new names
;			 "name&l" and "name&h" are created to refer
;			 to the low and high words.
;		     = a number - declare an argument of this
;			 length
;	    lowname  = if non-blank this is used instead of
;		       "name&l" when length is DBYTE.
;	    highname = if non-blank this is used instead of
;		       "name&h" when length is DBYTE.
;	    nowarn   = "hybrid" to stop ArgVar from generating a warning
;		       message about using ArgVar in a hybrid procedure.
;		       See example at top.
;
;   (global variables)
;	    ?aframe  = number of bytes declared as arguments so
;		       far. Set to 0 by Procedure.
;	    ?abase   = distance between BP and last byte of
;		       argument pushed. Usually 4 for near
;		       procedure and 6 for far.
;	    ?argfar  = 0 if ArgVars are at fixed offset from BP.
;		       This is the case for near, far, faronly and far16.
;		     = 4 if ArgVars are at different offset from BP
;		       depending on whether the procedure is called near
;		       or called far.  This is the case for near
;		       + faronly procedures (hybrid).
;		     = 8 if ArgVars are at different offset from BP
;		       depending on whether the procedure is called near
;		       or called 32 bit far.  This is the case for near
;		       + far16 procedures.
;		     = -1 if multiple thunks exist; ArgVars are broken.
;
;   EXIT    name EQU to the appropriate location in the stack
;	    frame (uses BP).
;
;	    (global variables)
;		 ?aframe = incremented by number of bytes specified
;		      in "length".
;
;   NOTES   In 16 bit code segments argvars are word aligned and in
;	    32 bit code segments argvars are dword aligned.
;
;   SEE ALSO: Procedure, EnterProc, LeaveProc, EndProc.

ArgVar	macro	name,length,lowname,highname,nowarn,bpsize
    if2
	if ?argfar eq -1
	    ProcError <ArgVar cannot be used with multiple thunks>
	    .err
	endif
	ifidni <nowarn>,<hybrid>
	    ife ?argfar
		ProcError <ArgVar <nowarn> parameter invalid>
		.err
	    endif
	else
	    if ?argfar
		ProcError <references to ArgVar name may fail when called far>
		.err
	    endif
	endif
	if (?distance and ?PD_ENTERED)
	    ProcError <ArgVar name invoked inside EnterProc>
	    .err
	endif
    endif
    if ?model EQ ?PASCAL
	?nfields = ?nfields + 1
	?MArg ArgVar,<name,length,lowname,highname,nowarn>,%?nfields
    else
	if (?distance and ?PD_ESPFRAME)
	    ?bp equ <ESP>
	else
	    ifnb <bpsize>
	       ifidni <bpsize>,<32>
		   ?bp equ <EBP>
	       else
		   ?bp equ <BP>
	       endif
	    else   	
	       if (?cstype eq ?CS_32bit)
		   ?bp equ <EBP>
	       else
		   ?bp equ <BP>
	       endif
	    endif
	endif
	?bm1 = ?aframe + ?abase
	ifidni <length>,<byte>
	    ?aframe = ?aframe + 1
	    ?BPEqu name,byte,%?bp,+,%?bm1
	elseifidni <length>,<dbyte>
	    ?aframe = ?aframe + 2
	    ?BPEqu name,word,%?bp,+,%?bm1
	    ifb <lowname>
		?BPEqu name&l,byte,%?bp,+,%?bm1
	    else
		?BPEqu lowname,byte,%?bp,+,%?bm1
	    endif
	    ifb <highname>
		?BPEqu name&h,byte,%?bp,+,%(?bm1+1)
	    else
		?BPEqu highname,byte,%?bp,+,%(?bm1+1)
	    endif
	elseifidni <length>,<word>
	    ?aframe = ?aframe + 2
	    ?BPEqu name,word,%?bp,+,%?bm1
	elseifidni <length>,<dword>
	    ?aframe = ?aframe + 4
	    ?BPEqu  name,dword,%?bp,+,%?bm1
	    ?BPEqu  name&l,word,%?bp,+,%?bm1
	    ?BPEqu  name&h,word,%?bp,+,%(?bm1+2)
	elseifidni <length>,<fword>
	    ?aframe = ?aframe + 8
	    ?BPEqu  name,fword,%?bp,+,%?bm1
	    ?BPEqu  name&l,dword,%?bp,+,%?bm1
	    ?BPEqu  name&h,dword,%?bp,+,%(?bm1+4)
	elseifdef ?&length
	    ?aframe = ?aframe + ?&length
	    if ?&length EQ 1
	      ?BPEqu  name,byte,%?bp,+,%?bm1
	    elseif ?&length EQ 2
	      ?BPEqu  name,word,%?bp,+,%?bm1
	    elseif ?&length EQ 4
	      ?BPEqu  name,dword,%?bp,+,%?bm1
	    else
	      ?BPEqu  name,,%?bp,+,%?bm1
	    endif
	else
	    ?aframe = ?aframe + length
	    ?BPEqu  name,byte,%?bp,+,%?bm1
	endif
	if (?cstype eq ?CS_32bit)		;; align arguments
	    if ?aframe and 03h
		?aframe = (?aframe and (not 03h)) + 04h
	    endif
	else
	    if ?aframe and 01h
		?aframe = ?aframe + 1
	    endif
	endif
    endif
endm

;*** LocalVar - declare a local variable
;
;   Reserves space on the stack frame for a local variable.
;
;   ENTRY   name     = name of local variable
;	    length   = "BP" - special case, no space is allocated,
;			 "name" labels the location where the old
;			 BP is stored.
;		     = "BYTE" - reserves a byte
;		     = "DBYTE" - reserves two bytes. Two additional
;			 names "name&l" and "name&h" are created to
;			 refer to the low and high byte.
;		     = "WORD" - reserves a word
;		     = "DWORD" - reserves a double word (4 bytes).
;			 Two additional names "name&l" and "name&h"
;			 are created to refer to the low and high
;			 word. This is always padded and not
;			 affected by "pad".
;		     = a number - reserves this many bytes.
;	    lowname  = if non-blank and "length" is "DBYTE", then
;		       this is used instead of "name&l".
;	    highname = if non-blank and "length" is "DBYTE", then
;		       this is used instead of "name&h".
;	    pad      = "PAD" - make WORD, DWORD, and numeric length
;			 variables start on even address.
;		     = "NOPAD" - don't pad WORD variables, but pad
;			 DWORD variables.
;		     = anything else - pad WORD and DWORD
;			 variables. Don't pad numeric length
;			 variables.
;   (global variables)
;	    ?frame = number of bytes reserved as local var
;		     (including padding) so far; set to 0 by
;		     Procedure.
;
;   EXIT    "name" EQU to a reserved space in the stack frame (uses
;	    BP register). No executable code is generated.
;   (global variables)
;	    ?pad   = don't care; should really be a local symbol
;	    ?nopad = don't care; should really be a local symbol
;	    ?frame = incremented by the number of bytes reserved
;		     as local var.
;
;   SEE ALSO: Procedure, EnterProc, LeaveProc, EndProc.

LocalVar macro	name,length,lowname,highname,pad,bpsize
ifdef ALIGNCODE
align 4
endif
    if2
	if (?distance and ?PD_ESPFRAME)
	    ProcError <LocalVar name invoked with ESP frame>
	    .err
	endif
	if (?distance and ?PD_ENTERED)
	    ProcError <LocalVar name invoked inside EnterProc>
	    .err
	endif
    endif
    ?pad = 0
    ?nopad = 0
    ifidni <pad>,<PAD>
	?pad = 1
    endif
    ifidni <pad>,<NOPAD>
	?nopad = 1
    endif
    ifnb <bpsize>
	ifidni <bpsize>,<32>
	    ?bp equ <EBP>
	else
	    ?bp equ <BP>
	endif
    else
	if (?cstype eq ?CS_32bit)
	    ?bp equ <EBP>
	else
	    ?bp equ <BP>
	endif
    endif
    ifidni <length>,<BP>	;; makes a variable point to the old BP
	name EQU  (WORD PTR [BP])
    elseifidni <length>,<EBP>	;; makes a variable point to the old EBP
	name EQU  (DWORD PTR [EBP])
    elseifidni <length>,<BYTE>
	?frame =  ?frame + 1
	?bm1 = ?frame
	?BPEqu	name,byte,%?bp,-,%?bm1
    elseifidni <length>,<DBYTE>
	if ?frame and 1
	    ?frame = ?frame + 1
	endif
	?frame =  ?frame + 2
	?bm1 = ?frame
	?BPEqu	name,word,%?bp,-,%?bm1
	ifb <lowname>
	    ?BPEqu  name&l,byte,%?bp,-,%?bm1
	else
	    ?BPEqu  lowname,byte,%?bp,-,%?bm1
	endif
	ifb <highname>
	    ?BPEqu  name&h,byte,%?bp,-,%(?bm1-1)
	else
	   ?BPEqu  highname,byte,%?bp,-,%(?bm1-1)
	endif
    elseifidni <length>,<WORD>
	ife ?nopad
	    if ?frame and 1
		?frame = ?frame + 1
	    endif
	endif
	?frame =  ?frame + 2
	?bm1 = ?frame
	?BPEqu	name,word,%?bp,-,%?bm1
    elseifidni <length>,<DWORD>
	if ?frame and 1
	    ?frame = ?frame + 1
	endif
	?frame =  ?frame + 4
	?bm1 = ?frame
	?BPEqu	name,dword,%?bp,-,%?bm1
	?BPEqu	name&l,word,%?bp,-,%?bm1
	?BPEqu	name&h,word,%?bp,-,%(?bm1-2)
    elseifidni <length>,<FWORD>
	if ?frame and 1
	    ?frame = ?frame + 1
	endif
	?frame =  ?frame + 8
	?bm1 = ?frame
	?BPEqu	name,fword,%?bp,-,%?bm1
	?BPEqu	name&l,dword,%?bp,-,%?bm1
	?BPEqu	name&h,dword,%?bp,-,%(?bm1-4)
    elseifdef ?&length
	if ?frame and 1
	    ?frame = ?frame + 1
	endif
	?frame =  ?frame + ?&length
	?bm1 = ?frame
	if ?&length EQ 1
	    ?BPEqu  name,byte,%?bp,-,%?bm1
	elseif ?&length EQ 2
	    ?BPEqu  name,word,%?bp,-,%?bm1
	elseif ?&length EQ 4
	    ?BPEqu  name,dword,%?bp,-,%?bm1
	else
	    ?BPEqu  name,,%?bp,-,%?bm1
	endif
    else
	?frame =  ?frame + length
	if ?pad
	    if ?frame and 1
		?frame = ?frame + 1
	    endif
	endif
	?bm1 = ?frame
	?BPEqu	name,byte,%?bp,-,%?bm1
    endif
endm

;*** EnterProc - generate code to set up stack frame
;
;   This should followed all the LocalVar's and before any other
;   executable code in the procedure.
;
;   ENTRY   varlist = list of word or dword values for initializing
;		      localvar's declared with LocalVar's.
;	      each initializer must be in the following form:
;		[<{word,dword},]{constant,register,address}[>]
;	      example:
;		EnterProc <0,<dword, -1>,ax,eax,<word,ds>,<word,es:[di].field>>
;	    chk     = "NoCheck" - defeat context checking
;	    falignesp = "alignesp" - dword align ESP
;
;   (global variables)
;	    ?frame = number of bytes reserved as localvar's by LocalVar.
;
;   EXIT    code generated to set up the stack frame
;   (global variables)
;	    ?frame = may be incremented by 1, 2 or 3 to start ESP on a
;		     word or dword boundary relative to the stack frame.
;
;   NOTE: OPTIMIZATION for 486/Pentium
;	  Although one might think that a hardware/microcode implementation
;	  would be faster than a software implementation,  The ENTER/LEAVE
;	  instructions on the 486 are slower than the equivalent sequence
;	  of simpler instructions.
;
;      Timings:
;     ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;     ³ Instruction    ³size³ 386DX clocks ³  486 clocks   ³ Pentium clocks³
;     ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
;     ³ push ebp       ³ 1  ³	   2	   ³   1	   ³	1	   ³
;     ³ mov  ebp,esp   ³ 2  ³	   2	   ³   1 +1 penalty³	1	   ³
;     ³ add  esp,IMMED ³ 3  ³	   7	   ³   3 +1 penalty³	3	   ³
;     ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
;     ³ enter	       ³ 4  ³	  10	   ³  14	   ³   11	   ³
;     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
;
;
;   SEE ALSO: Procedure, LeaveProc, EndProc.

EnterProc   macro varlist, chk, falignesp, nularg
    if2
	ifnb <chk>
	    ifdif <chk>,<NoCheck>
		ProcError <EnterProc: bad NoCheck arg: chk>
		.err
	    endif
	elseif (?distance and ?PD_ENTERED)
	    ProcError <EnterProc invoked twice>
	    .err
	endif
	ifnb <falignesp>
	    ifdif <falignesp>,<alignesp>
		ProcError <EnterProc: bad alignesp arg: falignesp>
		.err
	    endif
	endif
	ifnb <nularg>
	    ProcError <EnterProc: too many args: nularg>
	    .err
	endif
    endif
    if ?model EQ ?PASCAL
      if ?nfields
	?model = NULL
	rept ?nfields
	  ?InvPrg <?AM>,%?nfields
	  ?nfields = ?nfields - 1
	endm
	?model = ?PASCAL
      endif
    endif
    ?distance = ?distance or ?PD_ENTERED
    if (?cstype eq ?CS_32bit)
	?ax equ <EAX>
	?bp equ <EBP>
	?sp equ <ESP>
    else
	?ax equ <AX>
	?bp equ <BP>
	?sp equ <SP>
    endif
    ?frame = (?frame + 1) and not 1	;; make final frame even
    if (?cstype eq ?CS_32bit)
	?frame = (?frame + 3) and not 3 ;; make final frame dword multiple
    endif
    ifdef ALIGNFRAME
    ;;	Force all LocalVar frames to dword multiple:
	?frame = (?frame + 3) and not 3 ;; make final frame dword multiple
    endif
    if ?frame
	if2
	    if (?distance and ?PD_ESPFRAME)
		ProcError <LocalVars used with ESP frame>
		.err
	    endif
	endif
	?Eval push %?bp 	    ;; else set up frame
	?Eval mov  %?bp,%?sp
	ifb <varlist>			;; if no LocalVars initialized
	    ?Eval lea  %?sp,%?bp,-,%?frame
	else
	    ?bm1 = ?frame
	    irp var,<varlist>		;; and push each value
		?bm3 = 1
		irp token,<var> 		;; examine each token
		    if (?bm3 eq 1)		;; if first token is word,dword
			?bm3 = 0
			ifidni <token>,<word>
			    ?bm2 = 2		;; two byte initializer
			    ?bm3 = 2
			elseifidni <token>,<dword>
			    ?bm2 = 4		;; four byte initializer
			    ?bm3 = 2
			endif
		    elseif (?bm3 eq 2)		;; if second token is seg reg
			irp x,<cs,ds,es,fs,gs,ss>
			    ifidni <x>,<token>
				if (((?bm2 eq 2) and (?cstype eq ?CS_32bit)) or \
				    ((?bm2 eq 4) and (?cstype ne ?CS_32bit)))
					db MI_OPERANDSIZE
				endif
				push  x
				?bm3 = 3
			    endif
			endm
			if (?bm3 eq 2)		;; else is memory reference
			    if (?bm2 eq 2)
				push  word ptr token
			    else
				push  dword ptr token
			    endif
			    ?bm3 = 3
			endif
		    elseif (?bm3 eq 3)		;; if fourth token exists
			if2
			    ProcError <EnterProc: var: extra characters: token>
			    .err
			endif
		    else			;; else invalid token: ignore
			?bm3 = 0
		    endif
		endm
		ife ?bm3
		    ?RegSize var		;; ?bm2 == size
		    push  var
		endif
		if2
		    if ?bm1 lt ?bm2
			ProcError <EnterProc: too many args>
			.err
		    endif
		    if (?cstype eq ?CS_32bit)
			irp x,<cs,ds,es,fs,gs,ss>
			    ifidni <x>,<var>
				ProcError <EnterProc: var is ambiguous: use word,dword override>
			    endif
			endm
		    endif
		endif
		?bm1 = ?bm1 - ?bm2
	    endm
	    if ?bm1			;; if any left over, pad the stack
		if ((?bm1 eq ?wdsz) or (?bm1 eq 2 * ?wdsz))
		    ?Eval push %?ax
		    if ?bm1 eq (2 * ?wdsz)
			?Eval push %?ax ;; if ?bm1 == (2 * ?wdsz), takes 2 bytes
		    endif
		elseif (?bm1 eq 2)	;; push ax takes 2 bytes (w/override)
		    push  ax
		else			;; lea instruction takes 3 bytes
		    ?Eval lea  %?sp,%?bp,-,%?frame
		endif
	    endif
	endif
	ifnb <falignesp>
	    ?Eval and  %?sp,<not 3>
	endif
    else
	if2
	    ifnb <varlist>		;; better be blank
		ProcError <EnterProc: no LocalVars to initialize>
		.err
	    endif
	endif
	if (?distance and ?PD_ESPFRAME) eq 0	;; if not an ESP frame
	    ?bm1 = 1				;; generate a frame
	    ifdef NONULLFRAMES			;; if null frames suppressed
		if (?aframe eq 0)		;; and no ArgVars given
		    ifb <falignesp>		;; and no alignesp
			?bm1 = 0		;; suppress the frame
		    endif
		endif
	    endif
	    if (?bm1)				;; if should generate a frame
		?Eval push %?bp
		?Eval mov  %?bp,%?sp
		ifnb <falignesp>
		    ?Eval and  %?sp,<not 3>
		endif
	    endif
	endif
    endif
endm

?RegSize macro var
    ?bm2 = ?wdsz		;; default initializer size
    irp x,<ax,bx,cx,dx,si,di,bp,sp>
	ifidni <x>,<var>
	    ?bm2 = 2		;; two byte initializer
	endif
    endm
    irp x,<eax,ebx,ecx,edx,esi,edi,ebp,esp>
	ifidni <x>,<var>
	    ?bm2 = 4		;; four byte initializer
	endif
    endm
endm

;*** LeaveProc - generate code to remove stack frame
;
;   This does the opposite of EnterProc. Use this before returning
;   from the procedure.
;
;   ENTRY   chk  = "NoCheck" - defeat context checking
;
;   EXIT    code generated to remove stack frame
;
;   SEE ALSO: Procedure, EnterProc, EndProc.

LeaveProc   macro chk
    if2
	if (?distance and ?PD_ENTERED) eq 0
	   ifdif <chk>,<NoCheck>
		ProcError <LeaveProc invoked without EnterProc>
		.err1
	    endif
	endif
    endif
    ?distance = ?distance and not ?PD_ENTERED
    if (?distance and ?PD_ESPFRAME) eq 0	;; if not an ESP frame
	ifdef NONULLFRAMES			;; if null frames suppressed
	    if (?frame or ?aframe)		;; if ArgVars or LocalVars used
		leave
	    endif
	else					;; else generate null frames
	  leave
	endif
    endif
endm

;*** ExitProc - generate code to remove stack frame and return
;
;   This does the opposite of EnterProc, just like LeaveProc.  Use
;   this instead of LeaveProc to ALSO generate an appropriate RET instruction.
;
;   ENTRY   "premature" to document a premature exit (optional)
;
;   EXIT    code generated to remove stack frame and return
;
;   SEE ALSO: Procedure, EnterProc, LeaveProc, EndProc.

ExitProc   macro  arg
    if (?distance and ?PD_ENTERED) ne 0     ;; if an EnterProc is active
      LeaveProc 			    ;; do the LeaveProc automatically
      ifidni <arg>,<premature>
       ?distance = ?distance or ?PD_ENTERED ;; premature exits should leave
      endif				    ;; the EnterProc state "in force"
    endif
    % ifdef _&&?name			    ;; try to determine if this is
      ?Eval RETP			    ;; a C function -JTP
    else
      ?Eval RETP,%?aframe		    ;; otherwise, do a PASCAL-style RET
    endif
endm

;*** FallInto - Specify procedure to fall into
;
;   This makes documentation of "fall-through" code cleaner.  Use this
;   macro to state where you expect to fall, and use FallFrom to state where
;   you expected to fall from (OPTIONAL).
;
;   ENTRY   name = name of procedure to fall into
;
;   EXIT    NONE

FallInto    macro   name
    ifnb <name>
      if1
	% ?name&&_end = $
      endif
      if2
	ifndef name
	  %out FallInto:  name unknown
	else
	  % if ?name&&_end NE ?segname:-1
	    if (name-$ NE 0 and name-$ NE 1 and name-$ NE 2 and name-$ NE 3)
	      % %out FallInto:	name does not follow ?name
	      .err
	    endif
	  endif
	endif
      endif
    endif
ifdef ALIGNCODE
    align 4
endif
endm

;*** FallFrom - Verify that we fell from the specified procedure
;
;   This makes documentation of "fall-through" code cleaner.  Use FallInto
;   to state where you expect to fall (MANDATORY), and use this macro to
;   state where you expected to fall from.
;
;   ENTRY   name = name of procedure fell from
;
;   EXIT    code generated to remove stack frame and return

FallFrom    macro   name
    ifnb <name>
      ifdef name&_end
	if (name&_end NE $ and name&_end NE $-1 and  name&_end NE $-2 and name&_end NE $-3)
	  if1
	    % %out FallFrom:  name does not precede ?name
	  endif
	  % name&_end = ?segname:-1
	  .err
	endif
      else
	if1
	  %out FallFrom:  name unknown
	endif
      endif
    endif
ifdef ALIGNCODE
    align 4
endif
endm

;*** SaveReg - generate code to save registers on stack
;
;   Use with macros Procedure, ArgVar, LocalVar, EnterProc,
;   RestoreRegs, LeaveProc, and EndProc.
;
;   ENTRY   reglist = list of registers to save
;   (global variables)
;	    ?stackdepth = #items pushed on the stack in the
;			  current procedure. Used for error
;			  checking in the EndProc macro.
;
;   EXIT    code generated to push registers on the stack.
;   (global variables)
;	    ?stackdepth = incremented once for each register saved.
;
;   SEE ALSO: Procedure, RestoreReg, EndProc.

SaveReg macro	reglist 	;; push those registers
    irp reg,<reglist>
	?RegSize reg
	?stackdepth = ?stackdepth + ?bm2
	  push reg
    endm
endm

;*** RestoreReg - generate code to restore registers from the stack
;
;   Use with macros Procedure, ArgVar, LocalVar, EnterProc,
;   SaveReg, LeaveProc, and EndProc.
;
;   ENTRY   reglist = list of registers to restore. The order of
;		      registers specfied must be the reverse of
;		      that specified in SaveReg.
;   (global variables)
;	    ?stackdepth = #items pushed on the stack in the
;			  current procedure. Used for error
;			  checking in the EndProc macro.
;
;   EXIT    code generated to pop registers.
;   (global variables)
;	    ?stackdepth = decremented once for each register
;			  restored.
;
;   SEE ALSO: Procedure, SaveReg, EndProc.

RestoreReg  macro   reglist	;; pop those registers
    irp reg,<reglist>
	?RegSize reg
	?stackdepth = ?stackdepth - ?bm2
	  pop reg
    endm
endm

;*** CPUMode - put a wrapper around .processor directives
;

CPUMode macro cpu
    ifidn <cpu>,<8086>
	?cpumode = 8086
	.8086
    elseifidn <cpu>,<8088>
	?cpumode = 8086
	.8086
    elseifidn <cpu>,<286>
	?cpumode = 286
	.286p
    elseifidn <cpu>,<386>
	?cpumode = 386
	.386p
    elseifidni <cpu>,<reset>
	if ?cpumode eq 386
	    .386p
	elseif ?cpumode eq 286
	    .286p
	elseif ?cpumode eq 8086
	    .8086
	endif
    else
	%out Unknown processor type cpu
	.err
    endif
endm

;*** CallFn - Determine function type and call it
;
;   Invoke CCall if "_name" is defined;  otherwise, use PCall.
;

CallFn	macro	name,arglst
	  ifdef ?&name
	    ifdef _&name
	      CCall name,<arglst>
	    else
	      PCall name,<arglst>
	    endif
	  else
	    if1
	      %out CallFn name:  No prototype
	    endif
	    .err
	  endif
	endm


;***	The following four macros implement a call level stack
;	for use by the CCall, PCall, and PushP macros.	They should
;	not be invoked externally.

?callsp = 0				;; Call stack pointer is zero

?stcallset macro	num,value
	?stcall&num = value
	.xcref	?stcall&num
	endm

?stcallget macro	num,sym
	sym	= ?stcall&num
	endm

?callpush macro value
	.lall
	?callsp = ?callsp + 1
	.xall
	?stcallset %?callsp,value
	endm

?callpop macro	sym
	.errnz	(?callsp eq 0)
	?stcallget %?callsp,sym
	.lall
	?callsp = ?callsp - 1
	.xall
	endm


;*** PCall - Call PASCAL "C" function
;
;	Call a near pascal procedure that may be external.  Uses pascal
;	argument order (args are pushed left to right).  The callee must
;	remove the arguments (with RET n).

PCall	macro	name, arglst, testarg
    if2
	ifnb <testarg>
	    ProcError <Too many args in PCall name>
	    .err
	endif
	if ?bmcpushp
	    ProcError <PushP used before PCall name>
	    .err
	endif
    endif
    ?argc = 0
    irp x,<arglst>
	push x
	?argc = ?argc + 1
    endm
    ifdef ?&name
	if ?&name NE -1
	    if ?argc NE ?&name
		if1
		    ProcError <Wrong number of arguments in PCall name>
		endif
	       .err
	    endif
	endif
    endif
    call  name
endm


;*** CCall - Call normal "C" function
;
;	Call a near "C" procedure that may be external.  Uses "C" argument
;	order (args are pushed right to left).	The arguments are removed
;	after the call if cdecl was indicated or -DSTDCALL not used.
;	"name" is empty when the caller just wants to clear PushP nesting
;
;   USES
;	Flags (only when arguments are removed from the stack by this macro).

?CC_CDECL	equ	01h
?CC_PASCAL	equ	02h
?CC_PUSHP	equ	04h
?CC_FAR32	equ	08h

CCall	macro	name, arglst, modifiers, testarg
    ?ccflags = 0		;; assume new C calling convention, no PushP
    irp x,<modifiers>				;; examine modifiers
	ifnb <x>
	    ifidn <x>,<cdecl>
		?ccflags = ?ccflags or ?CC_CDECL	;; cdecl override
	    elseifidn <x>,<pascal>
		?ccflags = ?ccflags or ?CC_PASCAL	;; pascal override
	    elseifidn <x>,<PushP>
		?ccflags = ?ccflags or ?CC_PUSHP	;; PushP used
	    elseifidn <x>,<far32>
		?ccflags = ?ccflags or ?CC_FAR32	;; far32 call
	    elseif2
		ProcError <Bad PushP/cdecl/pascal arg in CCall name: x>
		.err
	    endif
	endif
    endm
    ifndef STDCALL		;; if using old C compiler calling convention
	if ((?ccflags and ?CC_PASCAL) eq 0)
	    ?ccflags = ?ccflags or ?CC_CDECL	;; force cdecl override
	endif
    endif
    if2
	ifnb <testarg>
	    ProcError <Too many args in CCall name>
	    .err
	endif
	if ((?ccflags and ?CC_PUSHP) eq 0) and (?bmcpushp ne 0)
	    ProcError <PushP expected in CCall name> ;; if no PushP, but needed
	    .err
	endif
	if ((?ccflags and (?CC_CDECL or ?CC_PASCAL)) eq \
			  (?CC_CDECL or ?CC_PASCAL))
	    ProcError <pascal and cdecl conflict in CCall name>
	    .err
	endif
    endif
    .lall
    ?bm1 = ?ccflags				;; only for listing files
    .xall

    ?argc = 0
    if (?ccflags and ?CC_PASCAL)		;; if pascal used
	irp x,<arglst>				;; push args left to right
	    push x
	    ?argc = ?argc + 1
	endm
    else					;; else C order
	?Arg <arglst>				;; push args right to left
    endif
    if (?ccflags and ?CC_PUSHP) 		;; if PushP used
	if2
	    ife ?bmcpushp			;; if PushP not needed
		ProcError <PushP unexpected in CCall name>
		.err
	    endif
	endif
	?argc = ?argc + ?bmcpushp		;; remove the PushP parms, too
    endif
    if (?ccflags and ?CC_FAR32)
	CatPrefix <call far ptr FLAT:>, %F32PRE_, <name>
    else
	call  name				;; call the target
    endif
						;; if cdecl && arg count != 0
    if ((?ccflags and ?CC_CDECL) ne 0) and (?argc ne 0)
	?Eval add  esp,%(?argc*4)		;; remove the parms
    endif
    ife ?callsp
	?bmcpushp = 0
    else
	?callpop ?bmcpushp
    endif
endm


;*** PushP - push a dword parameter in advance of a CCall invocation
;
;	Push the parameter and increment the parameter count.

?bmcpushp = 0

PushP	macro	value, testarg
    if2
	ifnb <testarg>
	    ProcError <Too many args in PushP value>
	    .err
	endif
    endif
    ?bmcpushp = ?bmcpushp + 1
    PUSHD  <value>
endm


;***	CCallNest - begin nested CCall level
;
;	Push the current parameter count and reinitialize the
;	counter to zero.

CCallNest macro
	ife ?bmcpushp or ?callsp
	    ProcError <No parameters pushed before nesting>
	    .err
	    exitm
	endif
	?callpush ?bmcpushp
	?bmcpushp = 0
	endm


;*** BugBug - Document incomplete code, temporary assumptions, etc
;
;   ENTRY
;	d    - in the format DD-MMM-YY (eg, 06-Jan-88)
;	id   - your email ID (eg, JeffPar)
;	note - a few words about the problem, assumption, etc, in brackets (<>)
;

BugBug	macro	d,id,note
	?bm1 sizestr <d>
	if ?bm1 NE 9
	  %out Bad date in BUGBUG
	  .err
	endif
	if1
	  ifndef NOBUGBUG
	    %out BUGBUG d id:  note
	  endif
	endif
	ifdef ERRBUGBUG
	  .err
	endif
endm

;*** movzxESP - movzx esp,sp with private stack check			   ;whs
;									   ;whs
;   ENTRY								   ;whs
;	none								   ;whs
;									   ;whs
;   USES								   ;whs
;	reg, Flags							   ;whs
;									   ;whs
movzxESP   macro
local	l1
ifdef SMP
	ifndef _PSDStackSel
	   _DATA segment
	      extrn _PSDStackSel:dword
	   _DATA ends
	endif
	push	ax
	push	ds
	mov	ax,seg FLAT:DGROUP
	mov	ds,ax
     ASSUME DS:FLAT
	mov	ax,ss
	cmp	ax,seg FLAT:DGROUP
	je	short l1
	cmp	ax,word ptr [_PSDStackSel]
	je	short l1
	movzx	esp,sp
 l1:
	pop	ds
     ASSUME DS:NOTHING
	pop	ax
else
	push	ax							   ;whs
	mov	ax,ss							   ;whs
	cmp	ax,seg FLAT:DGROUP					   ;whs
	pop	ax							   ;whs
	je	short l1						   ;whs
	movzx	esp,sp							   ;whs
 l1:									   ;whs
endif										;whs
	endm								   ;whs
									   ;whs
;*** SSToDS_PS - Convert SS-rel offset to DS-relative offset (private St ac;whs
;									   ;whs
;   ENTRY								   ;whs
;	reg - register containing offset to be converted		   ;whs
;	val - optional memory location to take offset of (if not in reg  al;whs
;									   ;whs
;   USES								   ;whs
;	reg, Flags							   ;whs
;									   ;whs

ifndef SMP
SSToDS_PS  macro   reg,val						   ;whs
local	l1								   ;whs
	ifnb	<val>							   ;whs
	  lea	  reg,val						   ;whs
	endif								   ;whs
	  push	  ax							   ;whs
	  mov	  ax,ss 						   ;whs
	  cmp	  ax,seg FLAT:DGROUP					   ;whs
	  pop	  ax							   ;whs
	  je	  l1							   ;whs
	  add	  reg,[_TKSSBase]					   ;whs
     l1:								   ;whs
	endm								   ;whs
else
SSToDS_PS  macro   reg,val						   ;whs
local	l0,l1
	ifnb	<val>							   ;whs
	  lea	  reg,val						   ;whs
	endif								   ;whs
	  push	  ax							   ;whs
	  mov	  ax,ss 						   ;whs
	  cmp	  ax,seg FLAT:DGROUP					   ;whs
	  pop	  ax							   ;whs
	  je	  l1							   ;whs
ifdef SMPKERNEL
	  cmp	  _PGEnabled,1
	  jz	  short l0
	  add	  reg,[_TKSSBase1]					   ;whs
	  jmp	  short l1
       assume ds:FLAT
     l0:
endif
	  add	  reg,[_TKSSBase]					   ;whs
     l1:								   ;whs
	endm								   ;whs
endif
									   ;whs
;*** DSToSS_PS - Convert DS-relative offset to SS-relative offset (priva te;whs
;									   ;whs
;   ENTRY								   ;whs
;	reg - register containing offset to be converted		   ;whs
;	val - optional memory location to take offset of (if not in reg  al;whs
;									   ;whs
;   USES								   ;whs
;	reg, Flags							   ;whs

ifndef SMP
DSToSS_PS  macro   reg,val						   ;whs
local	l1								   ;whs
	ifnb	<val>							   ;whs
	  lea	  reg,val						   ;whs
	endif								   ;whs
	  push	  ax							   ;whs
	  mov	  ax,ss 						   ;whs
	  cmp	  ax,seg FLAT:DGROUP					   ;whs
	  pop	  ax							   ;whs
	  je	  l1							   ;whs
	  sub	  reg,[_TKSSBase]					   ;whs
     l1:								   ;whs
	endm								   ;whs
else
DSToSS_PS  macro   reg,val						   ;whs
local	l0,l1
	ifnb	<val>							   ;whs
	  lea	  reg,val						   ;whs
	endif								   ;whs
	  push	  ax							   ;whs
	  mov	  ax,ss 						   ;whs
	  cmp	  ax,seg FLAT:DGROUP					   ;whs
	  pop	  ax							   ;whs
	  je	  l1							   ;whs
ifdef SMPKERNEL
	  cmp	  _PGEnabled,1
	  jz	  short l0
	  sub	  reg,[_TKSSBase1]					   ;whs
	  jmp	  short l1
       assume ds:FLAT
     l0:
endif
	  sub	  reg,[_TKSSBase]					   ;whs
     l1:								   ;whs
	endm								   ;whs
endif
									   ;whs

;*** SSToDS - Convert SS-relative offset to DS-relative offset
;
;   ENTRY
;	reg - register containing offset to be converted
;	val - optional memory location to take offset of (if not in reg already)
;
;   USES
;	reg, Flags
;

ifndef SMP
SSToDS	macro	reg,val
	ifnb	<val>
	  lea	  reg,val
	endif
	  add	  reg,[_TKSSBase]
	endm
else
SSToDS	macro	reg,val
local l0,l1
	ifnb	<val>
	  lea	  reg,val
	endif
ifdef SMPKERNEL
	  cmp	  _PGEnabled,1
	  jz	  short l0
	  add	  reg,[_TKSSBase1]					   ;whs
	  jmp	  short l1
       assume ds:FLAT
     l0:
endif
	  add	  reg,[_TKSSBase]
     l1:
	endm
endif

;*** DSToSS - Convert DS-relative offset to SS-relative offset
;
;   ENTRY
;	reg - register containing offset to be converted
;	val - optional memory location to take offset of (if not in reg already)
;
;   USES
;	reg, Flags

ifndef SMP
DSToSS	macro	reg,val
	ifnb	<val>
	  lea	  reg,val
	endif
	  sub	  reg,[_TKSSBase]
	endm
else
DSToSS	macro	reg,val
local l0,l1
	ifnb	<val>
	  lea	  reg,val
	endif
ifdef SMPKERNEL
	  cmp	  _PGEnabled,1
	  jz	  short l0
	  sub	  reg,[_TKSSBase1]					   ;whs
	  jmp	  short l1
       assume ds:FLAT
     l0:
endif
	  sub	  reg,[_TKSSBase]
     l1:
	endm
endif

;*** PopAll - pop user's registers
;

PopAll	macro
    .386p
	popad
	pop	gs
	pop	fs
    CPUMode reset
	pop	es
	pop	ds
endm

;*** PushAll - push user's registers
;

PushAll macro
	push	ds
	push	es
    .386p
	push	fs
	push	gs
	pushad
    CPUMode reset
endm

;*** IODELAY - generate delay between I/O instructions
;
;   Use this between two adjacent I/O instructions.
;
;   EXIT    Code generated to cause a slight delay.

IODELAY macro
	jmp	short $+2
endm

;*** RETD - Default 32-bit return
;

RETD	macro	nparms
    ?RET <ne>, <ret>, <nparms>
endm

;*** RETND - Near 32-bit return
;

RETND	macro	nparms
    ?RET <ne>, <retn>, <nparms>
endm

;*** RETFD - Far 32-bit return
;

RETFD	macro	nparms
    ?RET <ne>, <retf>, <nparms>
endm

;*** RETW - Default 16-bit return
;

RETW	macro	nparms
    ?RET <eq>, <ret>, <nparms>
endm

;*** RETNW - Near 16-bit return
;

RETNW	macro	nparms
    ?RET <eq>, <retn>, <nparms>
endm

;*** RETFW - Far 16-bit return
;

RETFW	macro	nparms
    ?RET <eq>, <retf>, <nparms>
endm

;*** ?RET - RET* worker macro
;

?RET	macro	rel, retinst, nparms
    if (?cstype rel ?CS_32bit)
	db	MI_OPERANDSIZE
    endif
    retinst nparms
endm

;*** PUSHD - push dword value no matter the code segment type
;
;   Normally immediate and segment registers push a word or
;   dword on the stack depending on the code segment type.  This
;   macro always pushes the "value" as a dword (unless "value" is
;   a 16 bit register).
;
;   ENTRY   value
;
;   EXIT    NONE
;
;   USES    NONE

PUSHD	macro	value, testarg
    if2
	.errnb <testarg>			;; too many args
	irp x,<ax,bx,cx,dx,si,di,bp,sp>
	    ifidni <x>,<value>
		ProcError <PUSHD: 16 bit register: x>
		.err
	    endif
	endm
    endif
    ?bm1 = .type value				;; helps debug the macro
    ?bm1 = 2					;; use simple push
    if (?cstype ne ?CS_32bit)
	?bm1 = 0				;; use dword ptr push
	irp x,<cs,ds,es,fs,gs,ss>
	    ifidni <x>,<value>
		?bm1 = 1			;; use explicit override
	    endif
	endm
	if (?bm1 eq 0)
	    if (((.type (value)) and 07h) eq 04h) ;; if simple constant
		if ((value) lt 128) and ((value) gt -129)
		    ?bm1 = 1			;; use explicit override
		endif
	    elseif ((.type (value)) and 10h)	;; if register
		?bm1 = 2			;; use simple push
	    endif
	endif
    endif
    if (?bm1 eq 0)
	push	dword ptr (value)
    else
	if (?bm1 eq 1)				;; explicit override
	    db	MI_OPERANDSIZE
	endif
	push	value
    endif
endm

;*** POPD - pop dword value no matter the code segment type
;
;   Normally segment registers pop a word or dword on the stack
;   depending on the code segment type.  This macro always popes
;   the "value" as a dword (unless "value" is a 16 bit register).
;
;   ENTRY   value
;
;   EXIT    NONE
;
;   USES    NONE

POPD	macro	value, testarg
    if2
	.errnb <testarg>			;; too many args
	irp x,<ax,bx,cx,dx,si,di,bp,sp>
	    ifidni <x>,<value>
		ProcError <POPD: 16 bit register: x>
		.err
	    endif
	endm
    endif
    ?bm1 = .type value				;; helps debug the macro
    ?bm1 = FALSE
    if (?cstype eq ?CS_32bit)
	pop	value
    else
	irp x,<cs,ds,es,fs,gs,ss>
	    ifidni <x>,<value>
		db	MI_OPERANDSIZE
		pop	value
		?bm1 = TRUE
	    endif
	endm
	if ?bm1 eq FALSE
	    if ((.type value) and 10h)	;; if register
		pop	value
	    else
		pop	dword ptr value
	    endif
	endif
    endif
endm

;   This section contains macros to get around the 386 chip bug
;   regarding use of LSL, LAR, VERR, and VERW instructions.
;
;   If a bad selector is passed to any instruction above, it may
;   cause the 386 processor to hang up after executing the
;   instruction.  A workaround for this is to follow the instructions
;   with a jmp, and have the last byte of instructions aligned in the
;   same dword as all of jmp instruction; so that both instructions will
;   be prefetched together.
;
;   BUGBUG:  The code segments used must be DWORD aligned in order for
;   these macros to work properly.  There is an ALIGN 4 directive in
;   the ?Bug386 macro that should help insure this (however, the assembler
;   only seems to complain if the align-type is BYTE;  WORD works, but
;   musn't be allowed).
;

;*** LOADSL - Macro to replace 'lsl reg,sel'
;

LOADSL	macro	reg,sel
    .errb <reg>
    .errb <sel>
    ?Bug386 lsl,<reg>,sel
endm

;*** LOADAR - Macro to replace 'lar reg,sel'
;

LOADAR	macro	reg,sel
    .errb <reg>
    .errb <sel>
    ?Bug386 lar,<reg>,sel
endm

;*** VERIFYREAD - Macro to replace 'verr sel'
;
;   The second variable is a scratch register which improves
;   performance if provided and sel is not a register variable.
;

VERIFYREAD macro    sel,screg
    .errb <sel>
    ?Bug386 verr,,<sel>,screg
endm

;*** VERIFYWRITE - Macro to replace 'verw sel'
;
;   The second variable is a scratch register which improves
;   performance if provided and sel is not a register variable.
;

VERIFYWRITE macro   sel,screg
    .errb <sel>
    ?Bug386 verw,,<sel>,screg
endm

;*** ?Bug386
;
;   This macro provides a workaround the 386 chip bug
;   that causes the processor to hang if a bad selector
;   is passsed to any of the instructions lsl, lar, verr,
;   and verw. It puts the last byte of the given instruction
;   in the same dword with the following jnz instruction
;

?Bug386 macro	inst,reg,sel,screg
    local zero
    ?bm1 = TRUE
    irp x,<ax,bx,cx,dx,si,di>
	ifidni <sel>,<x>		;; Is sel a register?
	    ?bm1 = FALSE
	    exitm
	endif
    endm
    ifnb <reg>				;; if it is lsl or lar
	irp x,<eax,ebx,ecx,edx,esi,edi>
	    ifidni <sel>,<x>		;; 32 bit registers are also OK
		?bm1 = FALSE
		exitm
	    endif
	endm
    endif
    if ?bm1				;; Set up sel in a register
	ifb <reg>			;; if it is verr or verw
	    ifb <screg> 		;; if no scratch register
		push  ax		;; save ax
		mov   ax,sel		;; (ax) = sel
	    else			;; else use scratch register
		mov   screg,sel
	    endif
	else				;; if it is lsl or lar
	    mov  reg,sel
	endif
    endif

    ?i = $
    org 0
    zero label near
    align 4				;; the "align" is superfluous at
					;; offset 0;  idea is to try to insure
					;; proper segment align-type (DWORD)
    org ?i
    ?i = ($ - offset cs:zero) mod 4	;; calculate how many
    ?i = (6 - ?i) mod 4 		;; NOPS needed for padding
    rept ?i
	nop
    endm
    ?i = $				;; this label must be odd-word aligned
    if ?bm1
	ifb <reg>			;; If it is verr or verw
	    ifb <screg> 		;; No scratch register
		inst  ax
	    else			;; use scratch register
		inst  screg
	    endif
	else
	    inst  reg,reg		;; lsl or lar
	endif
    else
	ifb <reg>
	    inst  sel
	else
	    inst  reg,sel
	endif
    endif
    jmp short $+2			;; Must be in the same dword with the
					;; last byte of previous instruction
    if ($ - ?i - 5)
	if ($ - ?i - 6)
	    .err			;; Len must be 5 bytes (6 if 32bit reg)
	endif
    endif
    if ?bm1				  ;; Did we push ax?
	ifb <reg>
	    ifb <screg>
		pop  ax
	    endif
	endif
    endif
endm

;***	B1EMac - 386 B1 Errata Macro
;
;	This macro generates workarounds for the erratum 7 on the 386 B1
;	errata sheet dated September 1 1987:
;
;	<Begin Quote:>
;	Wrong Register Size for String Instructions in Mixed 16/32-bit
;	Addressing Systems.
;
;	Problem:  If certain string and loop instructions are followed by
;	instructions that either:
;
;	1)  use a different address size (that is, if either the string
;	    instruction or the following instruction uses an address size
;	    prefix), or
;
;	2)  reference the stack (e.g. PUSH/POP/CALL/RET) and the "B" bit
;	    in the SS descriptor is different from the address size used
;	    by the string instructions,
;
;	then one or more of (E)CX, (E)SI, or (E)DI is not updated properly.
;	The size of the register (16 vs. 32) is taken from the following
;	instruction rather than the from the string or loop instruction.
;	This could result in updating only the lower 16 bits of a 32 bit
;	register, or in updating all 32 bits of a register being used as
;	16 bits.  The instructions and registers affected by this are listed
;	below:
;
;		Instruction	Register(s)
;		    MOVS	  (E)DI
;		    REP MOVS	  (E)SI
;		    STOS	  (E)DI
;		    INS 	  (E)DI
;		    REP INS	  (E)CX
;
;	Workaround: No workaround is necessary if all code is 16-bit or if
;	all code is 32-bit.  The problem only occurs if instructions with
;	different address sizes are mixed together, or if a code segment of
;	one size used with a stack segment of the other size.
;
;	In a system which mixes address sizes, add a NOP after each of the
;	above instrcutions and ensure that the NOP has the same address
;	size as the string/loop (i.e., if the string/loop instruction
;	includes as address prefix, place the same address prefix before
;	the NOP; conversely, if the string/loop instruction does not have
;	an address prefix, do not place a prefix before the NOP).
;	<End Quote>
;
;	Invocation examples:
;	    B1EMac   insb
;	    B1EMac   repne ins dword ptr es:[di],dx
;	    B1EMac   rep movsw
;	    B1EMac   movs byte ptr es:[edi],byte ptr gs:[esi]
;	    B1EMac   stos dword ptr es:[di]
;	No white space is allowed on either size of the colons or on either
;	side of the registers within square brackets ("es:[edi]" must be one
;	token).
;
;	ENTRY	reparg	= <blank>,rep,repne,etc.	(optional)
;		instr	= ins*,movs*,stos*
;		dstsize = <blank>,byte,word,dword
;		dstptr	= <blank>,ptr
;		dstreg	= <blank>,es:[di],es:[edi]
;		srcsize = <blank>,byte,word,dword
;		srcptr	= <blank>,ptr
;		srcreg	= <blank>,?s:[si],?s:[esi]
;		testarg = <blank>
;	(global variables)
;		?cstype = ?CS_32bit or 0 to indicate default address size
;
;	EXIT	code generated to invoke the instruction with workaround

?BE_REP   equ 0001h	; valid rep prefix
?BE_LONG  equ 0002h	; long form of instruction
?BE_SHORT  equ 0004h	; short form of instruction

B1EMac	macro  reparg,instr,dstsize,dstptr,dstreg,srcsize,srcptr,srcreg,testarg
    if2
	.errnb <testarg>			;; too many args
    endif
    ?bm1 = 0
    ?bm2 = ?cstype				;; assume default address size
    ifnb <reparg>
	irp reptmp,<rep,repe,repz,repne,repnz>	;; valid rep prefix?
	    ifidni <reparg>,<reptmp>
		?bm1 = ?bm1 or ?BE_REP		;; Yes.
	    endif
	endm
	if ((?bm1 and ?BE_REP) eq 0)		;; else reinvoke with blank rep
	    B1EMac <> reparg instr dstsize dstptr dstreg, srcsize srcptr srcreg
	    exitm				;; and terminate the macro
	endif
    endif
    irp insttmp,<movs,ins,stos> 		;; long form of instruction?
	ifidni <instr>,<insttmp>
	    ?bm1 = ?bm1 or ?BE_LONG		;; Yes.
	endif
    endm
    if (?bm1 and ?BE_LONG)			;; long form?
	ifidni <dstreg>,<es:[edi]>
	    ?bm2 = ?CS_32bit			;; 32-bit address size
	elseifidni <dstreg>,<es:[di]>
	    ?bm2 = 0				;; 16-bit address size
	elseif2
	    ifb <dstreg>
		%out B1EMac: destination register required
	    else
		%out B1EMac: bad destination register: dstreg
	    endif
	    .err
	endif
    else					;; else valid short form?
	irp insttmp,<movsb,movsw,movsd,insb,insw,insd,stosb,stosw,stosd>
	    ifidni <instr>,<insttmp>
		?bm1 = ?bm1 or ?BE_SHORT	;; Yes.
	    endif
	endm
	if ((?bm1 and ?BE_SHORT) eq 0)
	    if2
		%out B1EMac: bad instruction: instr
		.err
	    endif
	endif
    endif
    ifnb <srcsize>				;; comma required?
	reparg instr dstsize dstptr dstreg, srcsize srcptr srcreg
    else
	reparg instr dstsize dstptr dstreg
    endif
    if (?bm2 ne (?cstype and ?CS_32bit))	;; add override if needed
	db	MI_ADDRESSSIZE
    endif
    nop
endm

;***	btrx, btsx, orb, andb, testb, etc.
;
;	These macros allow you to generate more space-efficient 386 code,
;	by oring/anding/testing only individual bytes of dwords, provided the
;	mask (source operand) contains bits in exactly one byte; eg,
;
;	    test    [foo],00000000000000001111111100000000b
;
;	can be written as
;
;	    testb   [foo],00000000000000001111111100000000b
;
;	which will automatically generate the following:
;
;	    test    byte ptr [foo+1],11111111b
;
;	The btrx and and btsx macros simply let you feed a normal bitmask
;	into a btr or bts instruction, instead of a bit offset (which we
;	generally don't have equates for).
;

orb	macro	dst,src
	?ins	or,dst,src,0
	endm

andb	macro	dst,src
	?ins	and,dst,src,0ffh
	endm

clrb	macro	dst,src
	?ins	and,dst,%(not src),0ffh
	endm

xorb	macro	dst,src
	?ins	xor,dst,src,0
	endm

testb	macro	dst,src
	?ins	test,dst,src,0
	endm

?ins	macro	ins,dst,src,b
	?bm1 = 0
	?bm2 = 0
	?bm3 = 0ffh
	?bm4 = b
	rept 4
	  if (src and ?bm3) ne ?bm4
	    ?bm5 = ?bm1
	    ?bm2 = ?bm2 + 1
	  endif
	  ?bm1 = ?bm1 + 1
	  ?bm3 = ?bm3 shl 8
	  ?bm4 = ?bm4 shl 8
	endm
	if (?bm2 ne 1)
	ins	dst,src
	else
	?insx	ins,dst,src,%?bm5
	endif
	endm

?insx	macro	ins,dst,src,i
	ins	byte ptr dst+i,src shr (i*8)
	endm

btx	macro	dst,src
	?bitno	src
	bt	dst,?bm1
	endm

btcx	macro	dst,src
	?bitno	src
	btc	dst,?bm1
	endm

btrx	macro	dst,src
	?bitno	src
	btr	dst,?bm1
	endm

btsx	macro	dst,src
	?bitno	src
	bts	dst,?bm1
	endm

?bitno	macro	m
	?bm1 = 0
	?bm2 = m
	rept 32
	  ?bm2 = ?bm2 shr 1
	  if ?bm2 ne 0
	    ?bm1 = ?bm1 + 1
	  endif
	endm
	endm


;***	GenHybrid - generate a far entry header for a faronly procedure
;***	?Gen16 - generate a far entry header for a far16 procedure
;***	GenFar32 - generate a far entry header for a far32 procedure
;
;	A faronly/far16/far32 procedure is constructed by creating a
;	faronly/far16/far32 entry header followed by a near procedure.
;
;   ENTRY   name = name of procedure
;	    lcl  = blank - make the far entry point public
;		 = non-blank - if symbol NOLOCAL is defined, then
;			       the entry is made public anyway.
;			       Else it is made 'local' by renaming
;			       it into lxx_&name (xx is a unique
;			       number) and declared public. This is
;			       useful for symbolic debugging.
;	    usecall = blank - assume that this thunk immediately
;			      precedes the real function, and that
;			      control can fall through
;		    = non-blank - assume that another thunk will be
;				  between this one and the real function,
;				  so we must use a CALL to invoke it.  Note
;				  that this precludes the use of ArgVar.
;	    (global vars)
;		NOLOCAL = undefined - let lcl controls whether the
;				      entry point should be (fake)
;				      local or public.
;			= defined - make entry point public no matter
;				    what "lcl" is.
;
;   EXIT    A far entry header (code and pseudo ops) generated.
;	    A "public" pseudo op is generated to declare the
;	    entry point public or local (fake local).

GenHybrid macro name, lcl, usecall, half
    ?GenThunk <RETFOFFSET>, <RETFOFFSET>, <retf>, %HYBPRE_, <name>, <lcl>, <usecall>, <half>
endm

?Gen16 macro name, lcl, usecall
    ?GenThunk <RETFDOFFSET>, <RETFDOFFSET>, <RETFD>, %F16PRE_, <name>, <lcl>, <usecall>
endm

GenFar32 macro name, lcl, usecall, half
    if2
	ifndef RETFDLabel
	    extrn RETFDLabel:near
	endif
    endif
    ?GenThunk <RETFDOFFSET>, <offset FLAT:RETFDLabel>, <retf>, %F32PRE_, <name>, <lcl>, <usecall>, <half>
endm

?GenThunk macro fretdef, retoffset, retinst, prefix, name, lcl, usecall, half
    local a
    ?bm1 = 3
    ifnb <half>
	?bm1 = 0
	irp x,<half>
	    ifidn <x>,<tolabel>
		?bm1 = ?bm1 or 1
	    elseifidn <x>,<afterlabel>
		?bm1 = ?bm1 or 2
	    elseif2
		%out bad half_arg to ?GenThunk: half
		.err
	    endif
	endm
    endif
    if (?bm1 and 1)
	ifndef fretdef
	    a:	retinst
	endif
	GenPublic prefix&name,<lcl>,<code> ;; generate a public or local symbol
	prefix&name label far
    endif
    if (?bm1 and 2)
	ifb <usecall>
	    ifdef fretdef
		push  retoffset 	;; near-code must immediately follow
	    else
		push  offset cs:a	;; near-code must immediately follow
	    endif
	else
	    call	name
	    retinst
	endif
    endif
endm

ifdef INCL_DEF

;*** ?Alloc - Allocate well-aligned data
;

?Alloc	macro var,dtyp,val,f
	  ifndef NOALIGN
	    if ?cstype EQ ?CS_16bit
	      ifidni <dtyp>,<dw>
		?Align var,2
	      elseif f
		?Align var,2		;; align structures as well
	      endif
	    elseif ?cstype EQ ?CS_32bit
	      ifidni <dtyp>,<dd>
		?Align var,4
	      elseif f
		?Align var,4		;; align structures as well
	      endif
	    endif
	  endif
	  var dtyp val
endm

;*** ?Align - Insure alignment is as specified
;

?Align	macro var,b
	  ?bm1 = offset $
	  ALIGN b
	  if ?bm1 NE offset $
	    ifnb <var>
	      if1
		%out b-byte alignment adjustment:  var
	      endif
	    endif
	  endif
endm

;*** REALEVEN - This macro replaces EVEN when we have offsets
;		above 32k in a segment. It is used in pdata.asm
;		and is required because the current version of
;		masm (5.10A.06) has a bug which makes it put
;		3 bytes of padding rather than 1.
;		If we change to a version of the assembler
;		without this bug, we can remove this macro.

REALEVEN macro

	?intgr = ($- TASKAREA:BASEPTDA) mod 2

	REPT	?intgr
		db	0
	ENDM

endm

;*** ?SetType - Set ?cstype to value specified
;

?SetType macro val
	   ifdif <val>,<USE32>
	     ?cstype = ?CS_16bit
	   else
	     ?cstype = ?CS_32bit
	   endif
endm

;*** ?PushSeg - Save segment state
;

?PushSeg macro n
	ifdif <n>,<0>
	  ?segname&n catstr ?segname
	  ?curseg&n catstr ?curseg
	  ?cstype&n = ?cstype
	  ?model&n = ?model
	  ?declare&n = ?declare
	endif
endm

;*** ?PopSeg - Restore segment state
;

?PopSeg macro n
	ifdif <n>,<0>
	  ?segname catstr ?segname&n
	  ?curseg catstr ?curseg&n
	  ?cstype = ?cstype&n
	  ?model = ?model&n
	  ?declare = ?declare&n
	else
	  ?segname equ <>
	  ?curseg equ <>
	  ?cstype = NULL
	  ?model = NULL
	  ?declare = NULL
	endif
endm

;*** ?OpenSeg - Parse parameters to DefCode/DefData
;

?OpenSeg macro scope,class,model,type
	  ?declare = NULL
	  ifidni <scope>,<EXPORT>
	    ?declare = ?PUBLIC
	  elseifidni <scope>,<IMPORT>
	    ?declare = ?EXTRN
	  elseifdifi <scope>,<LOCAL>
	   ifdef DEBUG
	    if DEBUG eq 1
	      ?declare = ?PUBLIC
	    endif
	   endif
	  endif
	  ifidni <model>,<C>
	    ?model = ?C
	  elseifidni <model>,<PASCAL>
	    ?model = ?PASCAL
	  else
	    ?model = NULL
	  endif
	  ?nsegs = ?nsegs + 1
	  % ifidni <?curseg>,<?segname>
	    ?segname equ <>
	  else
	    ?Eval %?segname,segment
	    ?curseg catstr ?segname
	  endif
endm

;*** ?CloseSeg - Parse parameters to EndCode/EndData
;

?CloseSeg macro scope,class,model,type,m
	  % ifnb <?segname>
	    ?Eval %?segname,ends
	  endif
	  if ?nsegs EQ 0
	    if1
	      %out Missing m directive
	    endif
	  else
	    ?nsegs = ?nsegs - 1
	  endif
endm

;*** ?ToUpper - Converts string to upper-case, returned in ?upper
;

?ToUpper macro s
	  ?upper equ <>
	  irpc x,<s>
	    if '&x' GE 'a'
	      if '&x' LE 'z'
		?t1 substr <ABCDEFGHIJKLMNOPQRSTUVWXYZ>,'&x'-'a'+1,1
		?upper catstr ?upper,?t1
	      else
		?upper catstr ?upper,<&x>
	      endif
	    else
	      ?upper catstr ?upper,<&x>
	    endif
	  endm
	endm

endif ; INCL_DEF

;*** ?DefLabel - define a label using text-macro arguments
;

?DefLabel macro var,type
	  .lall
	  var label type
	  .xall
	endm

;*** ?DefExtrn - define an external using text-macro arguments
;

?DefExtrn macro var,type
	  .lall
	  extrn var:type
	  .xall
	endm

;*** ?DefPublic - define a public using text-macro arguments
;

?DefPublic macro var
	  .lall
	  public var
	  .xall
	endm

;*** ?DefField - define a field, and make it public if DEBUG defined true
;

?DefField macro fn,var,stmt
	    ?F&fn macro
	    var stmt
	    endm
	    ifdef DEBUG
	      if DEBUG eq 1
		ifnb <var>
		  ?D&fn macro
		  public _&var
		  _&var = var
		  endm
		endif
	      endif
	    endif
	  endm

;*** ?Eval - Evaluates text-macros for readability in listings
;

?Eval	macro ins,a1,a2,a3,a4
      ifb <a3>
	ifb <a2>
	  ins	a1
	else
	  ins	a1,a2
	endif
      else
	  ins	a1,[a2&a3&a4]
      endif
endm

;*** ?BPEqu - Creates an equate for referencing vars via BP/EBP/ESP
;

?BPEqu	macro sym,typ,bpr,s,a
	ifb <typ>
	  ifdifi <bpr>,<esp>
	    .lall
	    sym equ <[bpr&s&a]>
	    .xall
	  else
	    .lall
	    sym equ <[bpr&s&a+?stackdepth]>
	    .xall
	  endif
	else
	  ifdifi <bpr>,<esp>
	    .lall
	    sym equ <(typ ptr [bpr&s&a])>
	    .xall
	  else
	    .lall
	    sym equ <(typ ptr [bpr&s&a+?stackdepth])>
	    .xall
	  endif
	endif
	endm

;*** ?Arg - Processes argument list - used by CCall only
;

?Arg	macro	arglst
	  irp x,<arglst>
	    ?argc = ?argc + 1
	    ?MArg push,<x>,%?argc
	  endm
	  ?bm1 = ?argc
	  rept ?bm1
	    ?InvPrg <?AM>,%?bm1
	    ?bm1 = ?bm1 - 1
	  endm
	endm

;*** ?MArg - Makes a macro to do the specified operation
;

?MArg	macro	op,arg,num
	  ?AM&num &macro
	  op  arg
	  &endm
	endm

;*** ?InvPrg - Concatenates, invokes and purges a macro
;

?InvPrg macro	name1,name2
	  name1&name2
	  purge name1&name2
	endm

;*** ?Purge - Concatenates and purges a macro
;

?Purge	macro	name1,name2
	  purge name1&name2
	endm
