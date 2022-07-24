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

#include <emitlib.h>

#include <direct.h> // _getcwd

//extern "C" __declspec(dllexport) int somtopenEmitFileCalled;

int somtopenEmitFileCalled=0;

//extern "C" __declspec(dllexport) int somtremoveExt(char *file, char *filename, char *fileext)
//{
//}

extern "C" __declspec(dllexport) FILE * SOMLINK somtopenEmitFileSL (char *file, char *ext)
{
  somtopenEmitFileCalled=1;
//  somtremoveExt(file, filename, fileext);
//  somtaddExt();
  // Здесь необходимо предварительно добавить или заменить расширение на ext
  return fopen(file, "w");
};

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN SOMDLLIMPORT FILE * OPTLINK_DECL somtopenEmitFile (char *file, char *ext)
{
  OPTLINK_2ARGS(file,ext);
  return somtopenEmitFileSL (file, ext);
};
#endif

SOMEXTERN SOMDLLEXPORT int SOMLINK somtfcloseSL (FILE *fp)
{
  return fclose(fp);
}

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN int OPTLINK_DECL somtfclose (FILE *fp)
{
  OPTLINK_1ARG(fp);
  return somtfcloseSL (fp);
}
#endif

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN void OPTLINK_DECL somtmsg(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  OPTLINK_3ARGS(file, lineno, fmt);

  printf("(%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}
#endif


SOMEXTERN __declspec(dllexport) void SOMLINK somtmsgSL(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  printf("(%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN void OPTLINK_DECL somtwarn(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  OPTLINK_3ARGS(file, lineno, fmt);

  printf("warning: (%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}
#endif

SOMEXTERN __declspec(dllexport) void SOMLINK somtwarnSL(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  printf("warning: (%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}

//SOMEXTERN SOMDLLIMPORT void SOMLINK somtresetEmitSignalsSL();
//SOMEXTERN SOMDLLIMPORT void SOMLINK somtunsetEmitSignalsSL(void);
//SOMEXTERN SOMDLLIMPORT void somtresetEmitSignals();
//SOMEXTERN SOMDLLIMPORT void somtunsetEmitSignals(void);

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN void OPTLINK_DECL somterror(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  OPTLINK_3ARGS(file, lineno, fmt);

  printf("error: (%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}
#endif

SOMEXTERN __declspec(dllexport) void SOMLINK somterrorSL(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  printf("error: (%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN void OPTLINK_DECL somtfatal(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  OPTLINK_3ARGS(file, lineno, fmt);

  printf("fatal error: (%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}
#endif

SOMEXTERN __declspec(dllexport) void SOMLINK somtfatalSL(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  printf("Fatal error: (%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN void OPTLINK_DECL somtinternal(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  OPTLINK_3ARGS(file, lineno, fmt);

  printf("internal error: (%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}
#endif

SOMEXTERN __declspec(dllexport) void SOMLINK somtinternalSL(char *file, long lineno, char *fmt, ...)
{
  va_list args;

  printf("internal error: (%s: %d) ", file, lineno);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end( args );
  printf("\n");
}

SOMEXTERN BOOL OPTLINK_DECL somtfexists(char *file)
{
  OPTLINK_1ARG(file);

  return somtfexistsSL(file);
}

SOMEXTERN __declspec(dllexport) BOOL SOMLINK somtfexistsSL(char *file)
{
  return (_access(file, 4)==0);  // Try RO access
}

SOMEXTERN __declspec(dllexport) char * OPTLINK_DECL somtsearchFile(char *file, char *fullpath, char *env)
{
  return somtsearchFileSL(file, fullpath, env);
}

SOMEXTERN __declspec(dllexport) char * SOMLINK somtsearchFileSL(char *file, char *fullpath, char *env)
{
  char *path;
#ifdef _WIN32
  char magic=';';
#else
  char magic=':';
#endif

  if (!file) return NULL;
  if (!file[0]) return NULL;

  if (somtfexistsSL(file))
  {
    return strcpy(fullpath, file);
  }
  else
  {
    if (file[0]=='/') return NULL;
#ifdef _WIN32
    if (file[0]=='\\') return NULL;
    if (file[1]==':') return NULL;
#endif

    path=getenv(env);

#ifndef _WIN32
    if (!path) path="/bin:/usr/bin:";
#endif

    if (path)
    {
      while (*path)
      {
        char buf[1024];
        char *q;

        q=buf;

        while ((*path)&&(*path !=magic))
        {
          *q=*path;
          path++;
          q++;
        }
        *q=0;
        if (*path) path++;

        if (!buf[0])
        {
          _getcwd(buf,sizeof(buf));
        }

        if (buf[0])
        {
          if (!strcmp(buf,"."))
          {
            _getcwd(buf,sizeof(buf));
          }
#ifdef _WIN32
          strncat(buf,"\\",sizeof(buf)-1);
#else
          strncat(buf,"/",sizeof(buf)-1);
#endif
          strncat(buf,file,sizeof(buf)-1);

          if (somtfexistsSL(buf))
          {
            return strcpy(fullpath,buf);
          }
        }
      }
    }
    return NULL;
  }
}

/*
     1        00050973        TCCalloc
     2        0005097C        TCFree
     3        00050983        TCMalloc
     4        0005098C        TCRealloc
     5        00050996        _DLL_InitTerm
     6        000509A4        get_double_alignment
     7        000509B9        somVaBuf_add
     8        000509C6        somVaBuf_create
     9        000509D6        somVaBuf_destroy
    10        000509E7        somVaBuf_get_valist
    11        000509FB        somtAncestorClass
    12        00050A0D        somtCPP
    13        00050A15        somtDllStem
    14        00050A21        somtGetReintroducedMethods
    15        00050A3C        somtInitialiseCreatetc
    16        00050A53        somtInitialiseEmitlib
    17        00050A69        somtInitialiseSmmeta
    18        00050A7E        somtInitialiseSmsmall
    19        00050A94        somtInitialiseSmtypes
    20        00050AAA        somtInitialiseSomc
    21        00050ABD        somtParentNumFromName
    22        00050AD3        somtaddEntry
    23        00050AE0        somtaddEntryBuf
    24        00050AF0        somtaddExt
    25        00050AFB        somtaddGAtt
    26        00050B07        somtaddHeader
    27        00050B15        somtaddstar
    28        00050B21        somtallocBuf
    29        00050B2E        somtallocDataList
    30        00050B40        somtallocMethodList
    31        00050B54        somtallocMlist
    32        00050B63        somtargFlag
    33        00050B6F        somtarrayToPtr
    34        00050B7E        somtattMap
    35        00050B89        somtattNormalise
    36        00050B9A        somtattalloc
    37        00050BA7        somtattjoin
    38        00050BB3        somtbasename
    39        00050BC0        somtcalcFileName
    40        00050BD1        somtcalloc
    41        00050BDC        somtclassIdlCall
    42        00050BED        somtclassList
    43        00050BFB        somtclassName
    44        00050C09        somtclassPrefix
    45        00050C19        somtcleanFiles
    46        00050C28        somtcleanFilesFatal
    47        00050C3C        somtcleanup_f
    48        00050C4A        somtclsfilename
    49        00050C5A        somtclsname
    50        00050C66        somtcreateMemBuf
    51        00050C77        somtcreateStab
    52        00050C86        somtcreateTypeCodes
    53        00050C9A        somtctos
    54        00050CA3        somtdbcsLastChar
    55        00050CB4        somtdbcsPostincr
    56        00050CC5        somtdbcsPreincr
    57        00050CD5        somtdbcsScan
    58        00050CE2        somtdbcsStrchr
    59        00050CF1        somtdbcsStrrchr
    60        00050D01        somtdbcsStrstr
    61        00050D10        somtdbcslowercase
    62        00050D22        somtdbcsuppercase
    63        00050D34        somtdeclareIdlVarargs
    64        00050D4A        somtdiskFull
    65        00050D57        somtdupMlist
    66        00050D64        somtdymain
    67        00050D6F        somtemitClassDataTokens
    68        00050D87        somtemitModule
    69        00050D96        somtemitModuleTypes
    70        00050DAA        somtemitPassthru
    71        00050DBB        somtemitPredefinedTcConstants
    72        00050DD9        somtemitPredefinedTypeCode
    73        00050DF4        somtemitReturns
    74        00050E04        somtemitTcConstant
    75        00050E17        somtemitTypes
    76        00050E25        somtepname
    77        00050E30        somteptotype
+    78        00050E3D        somterror
+    79        00050E47        somterrorSL
    80        00050E53        somtexit
    81        00050E5C        somtext
+    82        00050E64        somtfatal
+    83        00050E6E        somtfatalSL
+    84        00050E7A        somtfclose
+    85        00050E85        somtfcloseSL
+    86        00050E92        somtfexists
    87        00050E9E        somtfilePath
    88        00050EAB        somtfileStem
    89        00050EB8        somtfindBaseEp
    90        00050EC7        somtfindBaseEpNonPtr
    91        00050EDC        somtfindMethodName
    92        00050EEF        somtfree
    93        00050EF8        somtfreeDataList
    94        00050F09        somtfreeMethodList
    95        00050F1C        somtfreeMlist
    96        00050F2A        somtfreeStab
    97        00050F37        somtfreeWorld
    98        00050F45        somtfullComment
    99        00050F55        somtfullPrototype
   100        00050F67        somtfullTypedef
   101        00050F77        somtg_buf
   102        00050F81        somtg_f
   103        00050F89        somtg_membuf
   104        00050F96        somtg_sccsid
   105        00050FA3        somtg_somlink
   106        00050FB1        somtgenAttStubs
   107        00050FC1        somtgenSeqName
   108        00050FD0        somtgetAbistyle
   109        00050FE0        somtgetAtt
   110        00050FEB        somtgetDesc
   111        00050FF7        somtgetEntry
   112        00051004        somtgetNonRepeatedParent
   113        0005101D        somtgetOidlAtt
   114        0005102C        somtgetType
   115        00051038        somtgetVersion
   116        00051047        somtgetap
   117        00051051        somtgetatt
   118        0005105C        somtgetdatt
   119        00051068        somtgetgatt
   120        00051074        somtglbdefs
   121        00051080        somthandleDiskFull
   122        00051093        somticstrcmp
   123        000510A0        somtimplicit
   124        000510AD        somtimplicitArgs
   125        000510BE        somtimplicitMeta
   126        000510CF        somtincludeOnce
   127        000510DF        somtinitMalloc
   128        000510EE        somtinitialiseMeta
+   129        00051101        somtinternal
+   130        0005110E        somtinternalSL
   131        0005111D        somtinternal_f
   132        0005112C        somtipcbuf
   133        00051137        somtisDbcs
   134        00051142        somtisInheritedModifierSet
   135        0005115D        somtisMutRef
   136        0005116A        somtisSpecialMethod
   137        0005117E        somtisparent
   138        0005118B        somtisvoid
   139        00051196        somtkeyword
   140        000511A2        somtlistAttribute
   141        000511B4        somtload
   142        000511BD        somtlowercase
   143        000511CB        somtmajorVersion
   144        000511DC        somtmalloc
   145        000511E7        somtmclose
   146        000511F2        somtmeta
   147        000511FB        somtmget
   148        00051204        somtminorVersion
   149        00051215        somtmlistend
   150        00051222        somtmodule
   151        0005122D        somtmopen
   152        00051237        somtmp
   153        0005123E        somtmprintf
   154        0005124A        somtmrierror
   155        00051257        somtmrifatal
   156        00051264        somtmriinternal
   157        00051274        somtmrimsg
   158        0005127F        somtmriwarn
+   159        0005128B        somtmsg
+   160        00051293        somtmsgSL
   161        0005129D        somtnewMethodsCount
   162        000512B1        somtnextword
   163        000512BE        somtnotc
   164        000512C7        somtnthArg
   165        000512D2        somtobseleteHeaderFile
   166        000512E9        somtoidlComment
   167        000512F9        somtokfopen
   168        00051305        somtokremove
+   169        00051312        somtopenEmitFile
   170        00051323        somtopenEmitFileCalled
+   171        0005133A        somtopenEmitFileSL
   172        0005134D        somtoutPath
   173        00051359        somtpclsfilename
   174        0005136A        somtpclsname
   175        00051377        somtprefixedPrototype
   176        0005138D        somtprivateMethodsCount
   177        000513A5        somtprocessTraps
   178        000513B6        somtqualifyNames
   179        000513C7        somtreadDescFile
   180        000513D8        somtrealExit
   181        000513E5        somtrealloc
   182        000513F1        somtremoveExt
   183        000513FF        somtreplaceDataName
   184        00051413        somtresetEmitSignals
   185        00051428        somtresetEmitSignalsSL
   186        0005143F        somtreturnsPtr
   187        0005144E        somtreturnsStruct
   188        00051460        somtrmSelf
   189        0005146B        somtrmstar
   190        00051476        somtsatos
   191        00051480        somtsaved
   192        0005148A        somtscbuf
   193        00051494        somtscmsg
   194        0005149E        somtsearchFile
   195        000514AD        somtsetDefaultDesc
   196        000514C0        somtsetEmitSignals
   197        000514D3        somtsetInternalMessages
   198        000514EB        somtsetTypeDefn
   199        000514FB        somtsetext
   200        00051506        somtshortArgList
   201        00051517        somtshortDefine
   202        00051527        somtshowVersion
   203        00051537        somtsimpleName
   204        00051546        somtsizeofEntry
   205        00051556        somtskipws
   206        00051561        somtsmalloc
   207        0005156D        somtsmfree
   208        00051578        somtsomcls
   209        00051583        somtstab
   210        0005158C        somtstabFirst
   211        0005159A        somtstabFirstName
   212        000515AC        somtstabNext
   213        000515B9        somtstabNextName
   214        000515CA        somtstrictidl
   215        000515D8        somtstringFmt
   216        000515E6        somttcAlignment
   217        000515F6        somttcKind
   218        00051601        somttcPrint
   219        0005160D        somttcSeqFromListString
   220        00051625        somttcSize
   221        00051630        somttraverseParents
   222        00051644        somttype
   223        0005164D        somtuninitialiseMeta
   224        00051662        somtuniqFmt
   225        0005166E        somtuniqString
   226        0005167D        somtunload
   227        00051688        somtunsetEmitSignals
   228        0005169D        somtunsetEmitSignalsSL
   229        000516B4        somtuppercase
+   230        000516C2        somtwarn
+   231        000516CB        somtwarnSL
   232        000516D6        somtwidenType
   233        000516E4        somtwriteaccess
   234        000516F4        somvalistGetTarget
   235        00051707        somvalistSetTarget
   236        0005171A        tciPrintf
   237        00051724        tciSetException
*/