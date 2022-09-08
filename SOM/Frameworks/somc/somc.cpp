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
  OPTLINK_3ARGS(file, fullpath, env);

  return somtsearchFileSL(file, fullpath, env);
}

SOMEXTERN __declspec(dllexport) char * SOMLINK somtsearchFileSL(char *file, char *fullpath, char *env)
{
  char *path;
#if defined(_WIN32) || defined(__OS2__) || defined(__DOS__)
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
#if defined(_WIN32) || defined(__OS2__) || defined(__DOS__)
    if (file[0]=='\\') return NULL;
    if (file[1]==':') return NULL;
#endif

    path=getenv(env);

#if !defined(_WIN32) || !defined(__OS2__) || !defined(__DOS__)
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
#if defined(_WIN32) || defined(__OS2__) || defined(__DOS__)
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

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN SOMDLLIMPORT int OPTLINK_DECL somttraverseParents(FILE *fp, Entry * cls, Entry *arg, int (*fn)(FILE*,Entry*,Entry*), SMTraverse flg)
{
  OPTLINK_3ARGS(fp, cls, arg);

  return somttraverseParentsSL(fp, cls, arg, fn, flg);
}

SOMEXTERN SOMDLLIMPORT AttList * OPTLINK_DECL somtattalloc(MemBuf *membuf, char *name, char *value)
{
  OPTLINK_3ARGS(membuf, name, value);

  return somtattallocSL(membuf, name, value);
}

SOMEXTERN SOMDLLIMPORT AttList * OPTLINK_DECL somtgetap(AttList *ap, char *att)
{
  OPTLINK_2ARGS(ap, att);

  return somtgetapSL(ap, att);
}

SOMEXTERN SOMDLLIMPORT EmitFn  OPTLINK_DECL somtload(char *fileName, char *functionName, void **modHandle)
{
  OPTLINK_3ARGS(fileName, functionName, modHandle);

  return somtloadSL(fileName, functionName, modHandle);
}

SOMEXTERN SOMDLLIMPORT Entry * OPTLINK_DECL somtfindBaseEp(Entry *ep)
{
  OPTLINK_1ARG(ep);

  return somtfindBaseEpSL(ep);
}

SOMEXTERN SOMDLLIMPORT Entry * OPTLINK_DECL somtgetType(char *name, SOMTTypes type)
{
  OPTLINK_2ARGS(name, type);

  return somtgetTypeSL(name, type);
}

SOMEXTERN SOMDLLIMPORT FILE * OPTLINK_DECL somtokfopen(char *path, char *mode)
{
  OPTLINK_2ARGS(path, mode);

  return somtokfopenSL(path, mode);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokfprintf(FILE *, const char *, ...)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokfputs(const char * c, FILE * f)
{
  OPTLINK_2ARGS(c, f);

  return somtokfputsSL(c, f);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokfputc(const char c, FILE * f)
{
  OPTLINK_2ARGS(c, f);

  return somtokfputcSL(c, f);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtokfgets(char * c, int n, FILE * f)
{
  OPTLINK_3ARGS(c, n, f);

  return somtokfgetsSL(c, n, f);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtokrewind(FILE * f)
{
  OPTLINK_1ARG(f);

  somtokrewindSL(f);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokrename(const char * a, const char * b)
{
  OPTLINK_2ARGS(a, b);

  return somtokrenameSL(a, b);
}

SOMEXTERN SOMDLLIMPORT BOOL OPTLINK_DECL somtisDbcs(int c)
{
  OPTLINK_1ARG(c);

  return somtisDbcsSL(c);
}

SOMEXTERN SOMDLLIMPORT BOOL OPTLINK_DECL somtremoveExt(char *name, char *ext, char *buf)
{
  OPTLINK_3ARGS(name, ext, buf);

  return somtremoveExtSL(name, ext, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtaddExt(char *name, char *ext, char *buf)
{
  OPTLINK_3ARGS(name, ext, buf);

  return somtaddExtSL(name, ext, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtarrayToPtr(Entry *ep, char *stars, char *buf)
{
  OPTLINK_3ARGS(ep, stars, buf);

  return somtarrayToPtrSL(ep, stars, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtattNormalise(char *name, char *buf)
{
  OPTLINK_2ARGS(name, buf);

  return somtattNormaliseSL(name, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtbasename(char *path)
{
  OPTLINK_1ARG(path);

  return somtbasenameSL(path);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtctos(Const *con, char *buf)
{
  OPTLINK_2ARGS(con, buf);

  return somtctosSL(con, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsPostincr(char **p)
{
  OPTLINK_1ARG(p);

  return somtdbcsPostincrSL(p);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsPreincr(char **p)
{
  OPTLINK_1ARG(p);

  return somtdbcsPreincrSL(p);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsStrchr(char *s, int c)
{
  OPTLINK_2ARGS(s, c);

  return somtdbcsStrchrSL(s, c);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsStrrchr(char *s, int c)
{
  OPTLINK_2ARGS(s, c);

  return somtdbcsStrrchrSL(s, c);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsStrstr(char *s1, char *s2)
{
  OPTLINK_2ARGS(s1, s2);

  return somtdbcsStrstrSL(s1, s2);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somteptotype(Entry *ep, char *ptrs, char *buf)
{
  OPTLINK_3ARGS(ep, ptrs, buf);

  return somteptotypeSL(ep, ptrs, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetDesc(Stab *stab, Entry *cls, Entry *method, char *desc, BOOL addQuotes, BOOL use, BOOL versflg)
{
  OPTLINK_3ARGS(stab, cls, method);

  return somtgetDescSL(stab, cls, method, desc, addQuotes, use, versflg);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetVersion(char *sccsid, char *version)
{
  OPTLINK_2ARGS(sccsid, version);

  return somtgetVersion(sccsid, version);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetgatt(char *s)
{
  OPTLINK_1ARG(s);

  return somtgetgattSL(s);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtnextword(const char *s, char *buf)
{
  OPTLINK_2ARGS(s, buf);

  return somtnextwordSL(s, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtnormaliseDesc(char *desc, char *normal)
{
  OPTLINK_2ARGS(desc, normal);

  return somtnormaliseDescSL(desc, normal);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtsatos(char **sa, char *sep, char *buf)
{
  OPTLINK_3ARGS(sa, sep, buf);

  return somtsatosSL(sa, sep, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtskipws(const char *s)
{
  OPTLINK_1ARG(s);

  return somtskipwsSL(s);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtstringFmt(char *fmt, ...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somttype(SOMTTypes type)
{
  OPTLINK_1ARG(type);

  return somttypeSL(type);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtuniqFmt(MemBuf *membuf, char *fmt, ...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtargFlag(int *argc, char ***argv)
{
  OPTLINK_2ARGS(argc, argv);

  return somtargFlagSL(argc, argv);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtattjoin(register AttList *ap1, AttList *ap2)
{
  OPTLINK_2ARGS(ap1, ap2);

  return somtattjoinSL(ap1, ap2);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtdbcsLastChar(char *buf)
{
  OPTLINK_1ARG(buf);

  return somtdbcsLastCharSL(buf);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtdbcsScan(char **buf)
{
  OPTLINK_1ARG(buf);

  return somtdbcsScanSL(buf);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtdiskFull(FILE *fp)
{
  OPTLINK_1ARG(fp);

  return somtdiskFullSL(fp);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtisparent(Entry *cls, Entry *parent)
{
  OPTLINK_2ARGS(cls, parent);

  return somtisparentSL(cls, parent);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtmget(int setnum, int msgnum, char *msgbuf)
{
  OPTLINK_3ARGS(setnum, msgnum, msgbuf);

  return somtmgetSL(setnum, msgnum, msgbuf);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtmopen(char *filename)
{
  OPTLINK_1ARG(filename);

  return somtmopenSL(filename);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtmprintf(int setnum, int msgnum, ...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokremove(char *file)
{
  OPTLINK_1ARG(file);

  return somtokremoveSL(file);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtunload(void *modHandle)
{
  OPTLINK_1ARG(modHandle);

  return somtunloadSL(modHandle);
}

SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtwriteaccess(char *file)
{
  OPTLINK_1ARG(file);

  return somtwriteaccessSL(file);
}

SOMEXTERN SOMDLLIMPORT void * OPTLINK_DECL somtsmalloc(size_t nbytes, BYTE clear)
{
  OPTLINK_2ARGS_DWORD_BYTE(nbytes, clear);

  return somtsmallocSL(nbytes, clear);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtaddGAtt(MemBuf **membuf, AttList **ap, char *buf)
{
  OPTLINK_3ARGS(membuf, ap, buf);

  somtaddGAttSL(membuf, ap, buf);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtcalcFileName(char *def, char *over, char *ext)
{
  OPTLINK_3ARGS(def, over, ext);

  somtcalcFileNameSL(def, over, ext);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtcleanFilesFatal(int status)
{
  OPTLINK_1ARG(status);

  somtcleanFilesFatalSL(status);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtemitTypes(FILE *fp, Mlist *mp, Stab *stab)
{
  OPTLINK_3ARGS(fp, mp, stab);

  somtemitTypesSL(fp, mp, stab);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmclose(void)
{
  somtmcloseSL();
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtreadDescFile(Stab *stab, char *file)
{
  OPTLINK_2ARGS(stab, file);

  somtreadDescFileSL(stab, file);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetDefaultDesc(Stab *stab)
{
  OPTLINK_1ARG(stab);

  return somtsetDefaultDescSL(stab);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetEmitSignals(void(*cleanup) (int), void (*internal) (int))
{
  OPTLINK_2ARGS(cleanup, internal);

  somtsetEmitSignalsSL(cleanup, internal);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetTypeDefn(Entry *type, Entry *ep, char *ptrs, Entry *ret, BOOL array)
{
  OPTLINK_3ARGS(type, ep, ptrs);

  somtsetTypeDefnSL(type, ep, ptrs, ret, array);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetext(int csc)
{
  OPTLINK_1ARG(csc);

  somtsetextSL(csc);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtshowVersion(char *s, char *progname, char *sccsid)
{
  OPTLINK_3ARGS(s, progname, sccsid);

  return somtshowVersionSL(s, progname, sccsid);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsmfree(void *first, ...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtunsetEmitSignals(void)
{
  somtunsetEmitSignalsSL();
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtuppercase(char *s, char *buf)
{
  OPTLINK_2ARGS(s, buf);

  return somtuppercaseSL(s, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtlowercase(char *s, char *buf)
{
  OPTLINK_2ARGS(s, buf);

  return somtlowercaseSL(s, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsuppercase(char *s, char *buf)
{
  OPTLINK_2ARGS(s, buf);

  return somtdbcsuppercaseSL(s, buf);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcslowercase(char *s, char *buf)
{
  OPTLINK_2ARGS(s, buf);

  return somtdbcslowercaseSL(s, buf);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtresetEmitSignals(void)
{
  somtresetEmitSignalsSL();
}

SOMEXTERN SOMDLLIMPORT size_t  OPTLINK_DECL somtsizeofEntry(SOMTTypes type)
{
  OPTLINK_1ARG(type);

  return somtsizeofEntrySL(type);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtepname(Entry *ep, char *buf, BOOL suppressImpctxCheck)
{
  OPTLINK_3ARGS(ep, buf, suppressImpctxCheck);

  return somtepnameSL(ep, buf, suppressImpctxCheck);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgenSeqName(long n, Entry *base, char *buf, BOOL fullname)
{
  OPTLINK_3ARGS(n, base, buf);

  return somtgenSeqNameSL(n, base, buf, fullname);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmrifatal(char *file, long lineno, int msgnum,...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmriinternal(char *file, long lineno, int msgnum,...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmrierror(char *file, long lineno, int msgnum,...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmrimsg(char *file, long lineno, int msgnum,...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmriwarn(char *file, long lineno, int msgnum,...)
{
  return ;
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetInternalMessages(char *too_long, char *cant_continue, char *segv, char *bus)
{
  OPTLINK_3ARGS(too_long, cant_continue, segv);

  somtsetInternalMessagesSL(too_long, cant_continue, segv, bus);
}

SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtisvoid(Entry *type, char *defn)
{
  OPTLINK_2ARGS(type, defn);

  return somtisvoidSL(type, defn);
}

SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtreturnsStruct(Entry *ep)
{
  OPTLINK_1ARG(ep);

  return somtreturnsStructSL(ep);
}

SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtreturnsPtr(Entry *ep)
{
  OPTLINK_1ARG(ep);

  return somtreturnsPtrSL(ep);
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtsimpleName(Entry *ep)
{
  OPTLINK_1ARG(ep);

  return somtsimpleNameSL(ep);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtqualifyNames(Stab * stab, BOOL fully)
{
  OPTLINK_2ARGS(stab, fully);

  return somtqualifyNamesSL(stab, fully);
}

SOMEXTERN SOMDLLIMPORT Entry * OPTLINK_DECL somtfindBaseEpNonPtr(Entry *ep)
{
  OPTLINK_1ARG(ep);

  return somtfindBaseEpNonPtrSL(ep);
}

SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtprocessTraps(void)
{
  return somtprocessTrapsSL();
}

SOMEXTERN SOMDLLIMPORT Mlist * OPTLINK_DECL somtallocMlist(Entry * ep)
{
  OPTLINK_1ARG(ep);

  return somtallocMlistSL(ep);
}

SOMEXTERN SOMDLLIMPORT Mlist * OPTLINK_DECL somtmlistend(Mlist * mp, char *name)
{
  OPTLINK_2ARGS(mp, name);

  return somtmlistendSL(mp, name);
}

SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtisMutRef(Entry *ep, Mlist *seen, BOOL isself, long level)
{
  OPTLINK_3ARGS(ep, seen, isself);

  return somtisMutRefSL(ep, seen, isself, level);
}

SOMEXTERN SOMDLLIMPORT Mlist *  OPTLINK_DECL somtfreeMlist(Mlist *mp)
{
  OPTLINK_1ARG(mp);

  return somtfreeMlistSL(mp);
}

SOMEXTERN SOMDLLIMPORT Mlist *  OPTLINK_DECL somtdupMlist(Mlist *mp, Entry *ep)
{
  OPTLINK_2ARGS(mp, ep);

  return somtdupMlistSL(mp, ep);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtfreeWorld()
{
  somtfreeWorldSL();
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtinitMalloc(BOOL dynamic)
{
  OPTLINK_1ARG(dynamic);

  somtinitMallocSL(dynamic);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseEmitlib(void)
{
  somtInitialiseEmitlibSL();
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseSmmeta(void)
{
  somtInitialiseSmmetaSL();
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseCreatetc(void)
{
  somtInitialiseCreatetcSL();
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseSmtypes(void)
{
  somtInitialiseSmtypesSL();
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseSomc(void)
{
  somtInitialiseSomcSL();
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseSmsmall(void)
{
  somtInitialiseSmsmallSL();
}

SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtattMap(char *name)
{
  OPTLINK_1ARG(name);

  return somtattMapSL(name);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtexit(SOMTExitBuf *ebuf, int status)
{
  OPTLINK_2ARGS(ebuf, status);

  somtexitSL(ebuf, status);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtdymain(char *file, Entry *cls, EmitFn emitfn, char *emitter, int first, char *version, Stab *stab)
{
  OPTLINK_3ARGS(file, cls, emitfn);

  return somtdymain(file, cls, emitfn, emitter, first, version, stab);
}

SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtaddHeader(char *file, FILE *fp, char *ext)
{
  OPTLINK_3ARGS(file, fp, ext);

  return somtaddHeaderSL(file, fp, ext);
}

#endif

SOMEXTERN SOMDLLIMPORT int SOMLINK somttraverseParentsSL(FILE *fp, Entry * cls, Entry *arg, int (*fn)(FILE*,Entry*,Entry*), SMTraverse flg)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT AttList * SOMLINK somtattallocSL(MemBuf *membuf, char *name, char *value)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT AttList * SOMLINK somtgetapSL(AttList *ap, char *att)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT EmitFn  SOMLINK somtloadSL(char *fileName, char *functionName, void **modHandle)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT Entry * SOMLINK somtfindBaseEpSL(Entry *ep)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT Entry * SOMLINK somtgetTypeSL(char *name, SOMTTypes type)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT FILE * SOMLINK somtokfopenSL(char *path, char *mode)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT int  SOMLINK somtokfprintfSL(FILE *, const char *, ...)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int  SOMLINK somtokfputsSL(const char *, FILE *)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int  SOMLINK somtokfputcSL(const char, FILE *)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtokfgetsSL(char *, int, FILE *)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT void  SOMLINK somtokrewindSL(FILE *)
{
}

SOMEXTERN SOMDLLIMPORT int  SOMLINK somtokrenameSL(const char*, const char *)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT BOOL  SOMLINK somtisDbcsSL(int c)
{
  return FALSE;
}

SOMEXTERN SOMDLLIMPORT BOOL  SOMLINK somtremoveExtSL(char *name, char *ext, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtaddExtSL(char *name, char *ext, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtarrayToPtrSL(Entry *ep, char *stars, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtattNormaliseSL(char *name, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtbasenameSL(char *path)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtctosSL(Const *con, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsPostincrSL(char **p)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsPreincrSL(char **p)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsStrchrSL(char *s, int c)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsStrrchrSL(char *s, int c)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsStrstrSL(char *s1, char *s2)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somteptotypeSL(Entry *ep, char *ptrs, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetDescSL(Stab *stab, Entry *cls, Entry *method, char *desc, BOOL addQuotes, BOOL use, BOOL versflg)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetVersionSL(char *sccsid, char *version)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetgattSL(char *s)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtnextwordSL(const char *s, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtnormaliseDescSL(char *desc, char *normal)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtsatosSL(char **sa, char *sep, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtskipwsSL(const char *s)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtstringFmtSL(char *fmt, ...)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somttypeSL(SOMTTypes type)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtuniqFmtSL(MemBuf *membuf, char *fmt, ...)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtargFlagSL(int *argc, char ***argv)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtattjoinSL(register AttList *ap1, AttList *ap2)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtdbcsLastCharSL(char *buf)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtdbcsScanSL(char **buf)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtdiskFullSL(FILE *fp)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtisparentSL(Entry *cls, Entry *parent)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtmgetSL(int setnum, int msgnum, char *msgbuf)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtmopenSL(char *filename)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtmprintfSL(int setnum, int msgnum, ...)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtokremoveSL(char *file)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtunloadSL(void *modHandle)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT int SOMLINK somtwriteaccessSL(char *file)
{
  return 0;
}

// @fix params checks
// @fix somtmalloc checks
SOMEXTERN SOMDLLIMPORT void * SOMLINK somtsmallocSL(size_t nbytes, BOOL clear)
{
  void * buf;
  buf=somtmalloc(nbytes);
  if (clear) memset(buf, 0, nbytes);
  return buf;
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtaddGAttSL(MemBuf **membuf, AttList **ap, char *buf)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtcalcFileNameSL(char *def, char *over, char *ext)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtcleanFilesFatalSL(int status)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtemitTypesSL(FILE *fp, Mlist *mp, Stab *stab)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtmcloseSL(void)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtreadDescFileSL(Stab *stab, char *file)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetDefaultDescSL(Stab *stab)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetTypeDefnSL(Entry *type, Entry *ep, char *ptrs, Entry *ret, BOOL array)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetextSL(int csc)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtshowVersionSL(char *s, char *progname, char *sccsid)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtsmfreeSL(void *first, ...)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtunsetEmitSignalsSL(void)
{
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtuppercaseSL(char *s, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtlowercaseSL(char *s, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsuppercaseSL(char *s, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcslowercaseSL(char *s, char *buf)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtresetEmitSignalsSL(void)
{
}

SOMEXTERN SOMDLLIMPORT size_t SOMLINK somtsizeofEntrySL(SOMTTypes type)
{
  return 0;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtepnameSL(Entry *ep, char *buf, BOOL suppressImpctxCheck)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgenSeqNameSL(long n, Entry *base, char *buf, BOOL fullname)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtmrifatalSL(char *file, long lineno, int msgnum,...)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtmriinternalSL(char *file, long lineno, int msgnum,...)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtmrierrorSL(char *file, long lineno, int msgnum,...)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtmrimsgSL(char *file, long lineno, int msgnum,...)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtmriwarnSL(char *file, long lineno, int msgnum,...)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetInternalMessagesSL(char *too_long, char *cant_continue, char *segv, char *bus)
{
}

SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtisvoidSL(Entry *type, char *defn)
{
  return FALSE;
}

SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtreturnsStructSL(Entry *ep)
{
  return FALSE;
}

SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtreturnsPtrSL(Entry *ep)
{
  return FALSE;
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtsimpleNameSL(Entry *ep)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtqualifyNamesSL(Stab * stab, BOOL fully)
{
}

SOMEXTERN SOMDLLIMPORT Entry * SOMLINK somtfindBaseEpNonPtrSL(Entry *ep)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtprocessTrapsSL(void)
{
  return FALSE;
}

SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtallocMlistSL(Entry * ep)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtmlistendSL(Mlist * mp, char *name)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtisMutRefSL(Entry *ep, Mlist *seen, BOOL isself, long level)
{
  return FALSE;
}

SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtfreeMlistSL(Mlist *mp)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtdupMlistSL(Mlist *mp, Entry *ep)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtfreeWorldSL()
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtinitMallocSL(BOOL dynamic)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseEmitlibSL(void)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseSmmetaSL(void)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseCreatetcSL(void)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseSmtypesSL(void)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseSomcSL(void)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseSmsmallSL(void)
{
}

SOMEXTERN SOMDLLIMPORT char * SOMLINK somtattMapSL(char *name)
{
  return NULL;
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtexitSL(SOMTExitBuf *ebuf, int status)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtdymainSL(char *file, Entry *cls, EmitFn emitfn, char *emitter, int first, char *version, Stab *stab)
{
}

SOMEXTERN SOMDLLIMPORT void SOMLINK somtaddHeaderSL(char *file, FILE *fp, char *ext)
{
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
+   194        0005149E        somtsearchFile
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

/*

somtf_f is a global flags structure

somtg_f + 8
somipc option		flag
	s		+8
	r		+4
	y		+2
	l		+1

somtg_f
	x		+8
	w		-4
	v		+1
	q		+2
	c		-16

somtg_f + A0H  - misc modifiers?

*/
