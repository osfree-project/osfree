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

#ifndef emitlib_h
#define emitlib_h
#ifdef _WIN32
#include <windows.h>
#endif
#include <io.h>	
#include <stdio.h>
#include <stdlib.h>

#include <smstd.h>
#include <sm.h>
#include <symtab.h>
#include <smlib.h>

#ifndef SOMObject
  #define SOMObject void *
  #include <somcorba.h>
  #undef SOMObject
#else
  #include <somcorba.h>
#endif /* !SOMObject */

#ifdef __cplusplus
 extern "C" {
#endif

#include <optlink.h>

#ifndef _IDL_SEQUENCE_string_defined
#define _IDL_SEQUENCE_string_defined
SOM_SEQUENCE_TYPEDEF (string);
#endif /* _IDL_SEQUENCE_string_defined */

#define MAX_COMMENT_ARG_LIST_LENGTH (MAX_ARG_LIST_LENGTH * 2)
#define MAX_DESCRIPTOR_LENGTH       512
#define PASSTHRU_STRIP              0x2
#define SMMETA_PREFIX               "M_"

#if defined(_OS2) || defined(_WDOS)
    #define SMDELIMITER   ';'
#else
    #define SMDELIMITER   ':'
#endif /* _OS2 || _WDOS */

/*
 * Macro version of single line functions
 */
#define somtattExists(ep,s)          (somtgetatt(ep,s) ? TRUE : FALSE)
#define somtlookupComment(n)         (n ? n : "")

/* typedef needed by somtGetReintroducedMethods: */
typedef SOM_SEQUENCE(Entry *) _IDL_SEQUENCE_EntryPtr;

/* a False value is an error */
/* pick values so one can test for abistyle_2 or abistyle_2_3 by bitwise and.
   e.g. (cur_abi & abistyle_2) == True indicates cur_abi is _2 or _2_3 */
enum SOMTABIStyle { SOMTABIStyle_unknown = 0,
                    SOMTABIStyle_2 = 0x01,   /* 001 */
                    SOMTABIStyle_3 = 0x02,   /* 010 */
                    SOMTABIStyle_2_3 = 0x03, /* 011 */
                    SOMTABIStyle_4 = 0x04,   /* 100 */
                    SOMTABIStyle_3_4 = 0x06, /* 110 */
                    SOMTABIStyle_2_3_4 = 0x07/* 111 */
            };

/*
 * External function prototypes
 */
// Here old Optlink caling convention functions having SOMLINK equals
#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN SOMDLLIMPORT Entry * OPTLINK_DECL somtnthArg(Entry * method, int n);
SOMEXTERN SOMDLLIMPORT FILE * OPTLINK_DECL somtemitModule(char *file, Entry *cls, char *ext);
SOMEXTERN SOMDLLIMPORT Mlist * OPTLINK_DECL somtallocDataList(Entry *cls);
SOMEXTERN SOMDLLIMPORT Mlist * OPTLINK_DECL somtallocMethodList(Entry *cls, bool all);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtclsfilename(Entry * cls);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtclsname(Entry * cls);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtfindMethodName(const char *bp, char *name);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtfullPrototype(char *buf, Entry * method, char *sep, int varargs);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtfullTypedef(char *buf, Entry * cls, Entry * method);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetNonRepeatedParent(Entry *cls, int i);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetatt(Entry * ep, char *s);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetdatt(Entry * ep, char *s);
SOMEXTERN SOMDLLIMPORT enum SOMTABIStyle  OPTLINK_DECL somtgetAbistyle( Entry * ep );
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtimplicit(Entry *ep, bool shortform, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtimplicitArgs(Entry *ep);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtincludeOnce(Entry *cls, char *ext, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtpclsfilename(Entry *parent);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtpclsname(Entry *parent);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtprefixedPrototype(char *buf, Entry * method, char *sep, int varargs, char *prefix);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtreplaceDataName(char *buf, Entry * data, char *replace);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtrmSelf(char *str);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtshortArgList(char *buf, Entry * method, char *sep, bool varargs, bool addself);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtimplicitMeta(Entry *cls);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtlistAttribute(FILE * fp, int n, AttList * ap, char *s, bool value, bool breakLine, bool firstComma);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtnewMethodsCount(Entry * cls, int meta, bool procflg);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtprivateMethodsCount(Entry * cls, int meta);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtaddHeader(char *file, FILE *fp, char *ext);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtcleanFiles(int status);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtdeclareIdlVarargs(FILE *fp, Entry *ep);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtdymain(char *file, Entry *cls, EmitFn emitfn, char *emitter, int first, char *version, Stab *stab);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtemitModuleTypes(FILE *fp, Entry *ep, Stab *stab);
SOMEXTERN SOMDLLIMPORT long  OPTLINK_DECL somtemitPassthru(FILE * fp, Entry * cls, char *name, int mode, char *att);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtfreeDataList(Mlist *mlist);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtfreeMethodList(Mlist *mlist);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtfullComment(FILE * fp, char *fmt,...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somthandleDiskFull(FILE *fp);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtinitialiseMeta(Entry * cls, Stab * stab, bool meta, int imp);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtoidlComment(FILE * fp, int min, int max, char style, char *comment);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtscmsg(Entry *cls, Entry *ep, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtshortDefine(FILE *fp, Entry *ep, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtuninitialiseMeta(Entry * cls);
SOMEXTERN SOMDLLIMPORT FILE * OPTLINK_DECL somtobseleteHeaderFile(char *file, Entry *cls, char *ext, char *newext);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtwidenType(Entry *ep, char *args, char *type);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtgenAttStubs(FILE *fp, Entry *cls, char *prefix, char *classprefix);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtstrictidl(FILE *fp);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtcreateTypeCodes (Stab *stab);
SOMEXTERN SOMDLLIMPORT TypeCode * OPTLINK_DECL somtemitTcConstant (TypeCode t, FILE *f, char *name, TypeCode *alreadyDone);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtemitPredefinedTcConstants (FILE *f);
SOMEXTERN SOMDLLIMPORT Entry * OPTLINK_DECL somtAncestorClass(Entry *cls, char *name);
SOMEXTERN SOMDLLIMPORT short  OPTLINK_DECL somttcAlignment (TypeCode t, Environment *ev);
SOMEXTERN SOMDLLIMPORT long  OPTLINK_DECL somttcSize (TypeCode t, Environment *ev);
SOMEXTERN SOMDLLIMPORT TCKind  OPTLINK_DECL somttcKind (TypeCode t, Environment *ev);
SOMEXTERN SOMDLLIMPORT sequence(string)  OPTLINK_DECL somttcSeqFromListString (string s);
SOMEXTERN SOMDLLIMPORT _IDL_SEQUENCE_EntryPtr  OPTLINK_DECL somtGetReintroducedMethods(Entry *cls);
#endif

SOMEXTERN SOMDLLIMPORT Entry * SOMLINK somtnthArgSL(Entry * method, int n);
SOMEXTERN SOMDLLIMPORT FILE * SOMLINK somtemitModuleSL(char *file, Entry *cls, char *ext);
SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtallocDataListSL(Entry *cls);
SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtallocMethodListSL(Entry *cls, bool all);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtclsfilenameSL(Entry * cls);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtclsnameSL(Entry * cls);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtfindMethodNameSL(const char *bp, char *name);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtfullPrototypeSL(char *buf, Entry * method, char *sep, int varargs);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtfullTypedefSL(char *buf, Entry * cls, Entry * method);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetNonRepeatedParentSL(Entry *cls, int i);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetattSL(Entry * ep, char *s);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetdattSL(Entry * ep, char *s);
SOMEXTERN SOMDLLIMPORT enum SOMTABIStyle  SOMLINK somtgetAbistyleSL( Entry * ep );
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtimplicitSL(Entry *ep, bool shortform, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtimplicitArgsSL(Entry *ep);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtincludeOnceSL(Entry *cls, char *ext, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtpclsfilenameSL(Entry *parent);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtpclsnameSL(Entry *parent);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtprefixedPrototypeSL(char *buf, Entry * method, char *sep, int varargs, char *prefix);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtreplaceDataNameSL(char *buf, Entry * data, char *replace);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtrmSelfSL(char *str);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtshortArgListSL(char *buf, Entry * method, char *sep, bool varargs, bool addself);
SOMEXTERN SOMDLLIMPORT int  SOMLINK somtimplicitMetaSL(Entry *cls);
SOMEXTERN SOMDLLIMPORT int  SOMLINK somtlistAttributeSL(FILE * fp, int n, AttList * ap, char *s, bool value, bool breakLine, bool firstComma);
SOMEXTERN SOMDLLIMPORT int  SOMLINK somtnewMethodsCountSL(Entry * cls, int meta, bool procflg);
SOMEXTERN SOMDLLIMPORT int  SOMLINK somtprivateMethodsCountSL(Entry * cls, int meta);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtaddHeaderSL(char *file, FILE *fp, char *ext);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtcleanFilesSL(int status);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtdeclareIdlVarargsSL(FILE *fp, Entry *ep);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtdymainSL(char *file, Entry *cls, EmitFn emitfn, char *emitter, int first, char *version, Stab *stab);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtemitModuleTypesSL(FILE *fp, Entry *ep, Stab *stab);
SOMEXTERN SOMDLLIMPORT long  SOMLINK somtemitPassthruSL(FILE * fp, Entry * cls, char *name, int mode, char *att);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtfreeDataListSL(Mlist *mlist);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtfreeMethodListSL(Mlist *mlist);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtfullCommentSL(FILE * fp, char *fmt,...);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somthandleDiskFullSL(FILE *fp);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtinitialiseMetaSL(Entry * cls, Stab * stab, bool meta, int imp);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtoidlCommentSL(FILE * fp, int min, int max, char style, char *comment);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtscmsgSL(Entry *cls, Entry *ep, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtshortDefineSL(FILE *fp, Entry *ep, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtuninitialiseMetaSL(Entry * cls);
SOMEXTERN SOMDLLIMPORT FILE * SOMLINK somtobseleteHeaderFileSL(char *file, Entry *cls, char *ext, char *newext);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtwidenTypeSL(Entry *ep, char *args, char *type);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtgenAttStubsSL(FILE *fp, Entry *cls, char *prefix, char *classprefix);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtstrictidlSL(FILE *fp);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtcreateTypeCodesSL(Stab *stab);
SOMEXTERN SOMDLLIMPORT TypeCode * SOMLINK somtemitTcConstantSL(TypeCode t, FILE *f, char *name, TypeCode *alreadyDone);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtemitPredefinedTcConstantsSL(FILE *f);
SOMEXTERN SOMDLLIMPORT Entry * SOMLINK somtAncestorClassSL(Entry *cls, char *name);
SOMEXTERN SOMDLLIMPORT short  SOMLINK somttcAlignmentSL(TypeCode t, Environment *ev);
SOMEXTERN SOMDLLIMPORT long  SOMLINK somttcSizeSL(TypeCode t, Environment *ev);
SOMEXTERN SOMDLLIMPORT TCKind  SOMLINK somttcKindSL(TypeCode t, Environment *ev);
SOMEXTERN SOMDLLIMPORT sequence(string)  SOMLINK somttcSeqFromListStringSL(string s);
SOMEXTERN SOMDLLIMPORT _IDL_SEQUENCE_EntryPtr  SOMLINK somtGetReintroducedMethodsSL(Entry *cls);

/*
 * Global variables
 */
SOMEXTERN global SOMDLLIMPORT int somtCPP;          /* C++ bindings */

SOMEXTERN global SOMDLLIMPORT bool somtclassIdlCall;/* IDL Calling convention */

SOMEXTERN global SOMDLLIMPORT char *somtclassName;  /* Current Class Name */

SOMEXTERN global SOMDLLIMPORT char *somtclassPrefix;/* Current Class Prefix */

SOMEXTERN global SOMDLLIMPORT char *somtg_somlink;  /* Set to "SOMLINK" when required - csc,
                                        * psc emitters turn this off. */

SOMEXTERN global SOMDLLIMPORT char *somtmajorVersion; /* Major version number for class */

SOMEXTERN global SOMDLLIMPORT char *somtminorVersion; /* Minor version number for class */

SOMEXTERN global SOMDLLIMPORT int somtmeta;         /* TRUE when in implicit meta mode */

SOMEXTERN global SOMDLLIMPORT char *somtmp;         /* Set to SMMETA_PREFIX when somtmeta is
                                        * TRUE */

SOMEXTERN global SOMDLLIMPORT Entry *somtsomcls;    /* Pointer to SOMClass Entry in Symbol
                                        * Table */



#ifdef __cplusplus
 }
#endif

#endif
