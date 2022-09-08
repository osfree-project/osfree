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

#ifndef smlib_h
#define smlib_h
#include <somc_msg.h>
#include <setjmp.h>
#include <optlink.h>

#define basemalloc  malloc
#define basefree    free
#define baserealloc realloc
#define basecalloc  calloc


#define SOMTOVERRIDE_TYPE(ep) (ep->type == SOMTOverrideMethodE || ep->u.m.omethod)
#define SOMTVALIDENTRY(e,m) ((somtgetatt(e, "class") ? TRUE : FALSE) == m)
#define SMUNKNOWNTYPE(e)    fprintf(stderr, "\"%s\": %d " \
                                      "[%d]. \"%s\"\n", __FILE__, __LINE__,\
                                       e->type, e->name)
#define somtfindBaseType(ep) (somtfindBaseEp(ep)->type)
/*
 * Error handling macros.  We keep a track of our various errors & warnings
 * in a global structure so that emitters can also post errors.
 * They can do this in one of two ways - calling the somtwarn, somterror,
 * somtfatal, somtinternal functions, or using these macros.
 */
#define SMANYERRORS()       (somtg_f.ERROR || somtg_f.FATAL || somtg_f.INTERNAL)
#define SMANYFATAL()        (somtg_f.FATAL || somtg_f.INTERNAL)
#define SMWARNCOUNT()       (somtg_f.WARN)
#define SMERRORCOUNT()      (somtg_f.ERROR + somtg_f.FATAL + somtg_f.INTERNAL)
#define SMPOSTERROR(type)   (somtg_f.type++)
#define SOMTSTAR            (somtaddstar ? "*" : "")
#define SOMTQUAL(cls)       (cls->sname && cls->sname[1])
#define ATTMAP(n)           ((!somtg_f.idl && n && (*n == 'c')) ? somtattMap(n) : n)
#define MSGSET STK
#ifdef __RESTRICT_MEMORY__
#define MAX_MSGSIZE 256
#else
#define MAX_MSGSIZE 500
#endif

/*
 * setjmp/longjmp exit details
 */

typedef struct {
    jmp_buf jmpbuf;
    bool called;
    int  status;
} SOMTExitBuf;

#define SOMTSETEXIT(buf)    if (!buf.called) setjmp(buf.jmpbuf)

#define SOMT_IDL_SEQUENCE_LEN_1  13             /* IDL_SEQUENCE */
#define SOMT_IDL_SEQUENCE_LEN_2  22             /* seq<%10d> */
#define SOMT_IDL_SEQUENCE_LEN (SOMT_IDL_SEQUENCE_LEN_1+SOMT_IDL_SEQUENCE_LEN_2)
#define SOMT_REMOVE_IDL_SEQUENCE(s)   ((*s == '/') ? \
                                         s+SOMT_IDL_SEQUENCE_LEN_2 : s)

/*
 * DBCS enabling macros
 */
#ifdef _DBCS
  #define DBCSBLANK        0x8140
  #define DBCSTOINT(buf)   ((((int)*(buf))<<8)+((int)*((buf)+1)))
  #define DBCSCOPY(buf,c)  if(isDBCS(c))\
    {*(buf)++=(char)((c)>>8);*(buf)++=(char)((c)&0xFF);}\
    else *(buf)++=(char)(c)
  #define DBCSSCAN(buf)    somtdbcsScan((char **)&(buf))
  #define DBCSINCR(buf)    ((buf) += isDBCS(*(buf))? 2: 1)
  #define DBCSCHAR(buf)    (isDBCS(*(buf)) ? DBCSTOINT(buf) : (int)(*(buf)))
  #define DBCS_output      dbcsOutput
  #define dbcsOutput(c,fp) ((isDBCS(c) && (putc((char)(c>>8), fp) == EOF)) \
                                           ? EOF \
                                           : putc((char) (c&0x00FF), fp))
  #define DBCS_strchr      somtdbcsStrchr
  #define DBCS_strrchr     somtdbcsStrrchr
  #define DBCS_strstr      somtdbcsStrstr
  #define DBCS_preincr     somtdbcsPreincr
  #define DBCS_postincr    somtdbcsPostincr
  #define isDBCS           somtisDbcs
  #define DBCS_isspace(c)  ((c >= 0) && (c < 256) && isspace(c))
#else
  #define DBCSCOPY(buf,c)  (*(buf)++=(char)(c))
  #define DBCSSCAN(buf)    ((int)(*(buf)++))
  #define DBCSINCR(buf)    (++(buf))
  #define DBCSCHAR(buf)    ((int)(*(buf)))
  #define DBCS_output      putc
  #define DBCS_strchr      strchr
  #define DBCS_strrchr     strrchr
  #define DBCS_strstr      strstr
  #define DBCS_preincr     ++
  #define DBCS_postincr(x) ((x)++)
  #define isDBCS(s)        (FALSE)
  #define DBCS_isspace(c)  (isspace(c))
#endif /* _DBCS */

/*
 * ISO LATIN 8859.1 Characater Set
 */
#define SOMTISALNUM(c)        (isalnum(c) || c == '_' || IS_ISO_LATIN_8859_1(c))
#define ISO_LATIN_8859_1_START  192
#define ISO_LATIN_8859_1_FINISH 255
#define IS_ISO_LATIN_8859_1(c)  (c >= ISO_LATIN_8859_1_START && \
                                 c <= ISO_LATIN_8859_1_FINISH && \
                                 c != 215 && c != 247)

/*
 * Enum for parent traversal.
 */
typedef enum _SMTraverse {SMTraverseAll, SMTraverseOnce, SMTraverseMany}
        SMTraverse;

/*
 * Command line flags and error structure.
 */
typedef struct {
    unsigned verbose:1;
    unsigned quiet:1;
    unsigned warn:1;
    unsigned extra:1;
    unsigned comments:1;
    AttList *atts;
    unsigned lex:1;
    unsigned yacc:1;
    unsigned release:1;
    unsigned stab:1;
    unsigned qualify:1;
    unsigned idl:1;
#ifdef __RESTRICT_MEMORY__
#define MAX_NO_EMITTERS 4
#else
#define MAX_NO_EMITTERS 32
#endif
    char *emitters[MAX_NO_EMITTERS];
    char *progname;
    int _ERROR;
    int WARN;
    int FATAL;
    int INTERNAL;
    unsigned append:1;          /* Append to files, like -memitappend */
    unsigned level2:1;          /* level 2 extensions */
    unsigned noint:1;           /* no "int" warning */
    size_t commentsize;         /* size of comment buffer */
    unsigned useshort:1;        /* use short names */
    AttList *pragmas;           /* list of pragmas */
    Mlist *glbemits;            /* list of types to globally emit */
    unsigned fullyqualified:1;  /* TRUE if names have been fullyqualified */
    unsigned tcdone:1;          /* TRUE if typecodes have been created */
    unsigned relchk:1;          /* TRUE if you want release order checking */
    unsigned binary:1;          /* TRUE if somtopenEmitFile should fopen in binary mode */
} Flags;

/*
 * Global variables - that are really macros
 */

#define somtcommentsize      somtg_f.commentsize
#define somtpragmaList       somtg_f.pragmas
#define somtglbemits         somtg_f.glbemits
#define somtfullyqualified   somtg_f.fullyqualified
#define somttcdone           somtg_f.tcdone
#define somtrelchk           somtg_f.relchk

#define ARG_STRING          ((*argc)--, *(*argv)++)
#define MAX_ARG_LIST_LENGTH 2048
#define SOMTSABUF           (4*(FILENAME_MAX+2)+1) /* ::id::id::id::id */
#define SOMT_MSG_SIZE       MAX_ARG_LIST_LENGTH + SOMTSABUF

typedef struct {
    char *name;
    char *desc;
    short len;
} BaseType;

/*
 * Emitter function prototype:
 */
typedef FILE* (*EmitFn)(char*,Entry*,Stab*);

// Here old Optlink caling convention functions having SOMLINK equals
#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN SOMDLLIMPORT int OPTLINK_DECL somttraverseParents(FILE *fp, Entry * cls, Entry *arg, int (*fn)(FILE*,Entry*,Entry*), SMTraverse flg);
SOMEXTERN SOMDLLIMPORT AttList * OPTLINK_DECL somtattalloc(MemBuf *membuf, char *name, char *value);
SOMEXTERN SOMDLLIMPORT AttList * OPTLINK_DECL somtgetap(AttList *ap, char *att);
SOMEXTERN SOMDLLIMPORT EmitFn  OPTLINK_DECL somtload(char *fileName, char *functionName, void **modHandle);
SOMEXTERN SOMDLLIMPORT Entry * OPTLINK_DECL somtfindBaseEp(Entry *ep);
SOMEXTERN SOMDLLIMPORT Entry * OPTLINK_DECL somtgetType(char *name, SOMTTypes type);
SOMEXTERN SOMDLLIMPORT FILE * OPTLINK_DECL somtokfopen(char *path, char *mode);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokfprintf(FILE *, const char *, ...);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokfputs(const char *, FILE *);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokfputc(const char, FILE *);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtokfgets(char *, int, FILE *);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtokrewind(FILE *);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokrename(const char*, const char *);
SOMEXTERN SOMDLLIMPORT FILE * OPTLINK_DECL somtopenEmitFile(char *file, char *ext);
SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtisDbcs(int c);
SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtremoveExt(char *name, char *ext, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtaddExt(char *name, char *ext, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtarrayToPtr(Entry *ep, char *stars, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtattNormalise(char *name, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtbasename(char *path);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtctos(Const *con, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsPostincr(char **p);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsPreincr(char **p);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsStrchr(char *s, int c);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsStrrchr(char *s, int c);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsStrstr(char *s1, char *s2);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somteptotype(Entry *ep, char *ptrs, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetDesc(Stab *stab, Entry *cls, Entry *method, char *desc, BOOL addQuotes, BOOL use, BOOL versflg);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetVersion(char *sccsid, char *version);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgetgatt(char *s);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtnextword(const char *s, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtnormaliseDesc(char *desc, char *normal);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtsatos(char **sa, char *sep, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtsearchFile(char *file, char *path, char *envvar);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtskipws(const char *s);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtstringFmt(char *fmt, ...);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somttype(SOMTTypes type);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtuniqFmt(MemBuf *membuf, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtargFlag(int *argc, char ***argv);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtattjoin(register AttList *ap1, AttList *ap2);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtdbcsLastChar(char *buf);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtdbcsScan(char **buf);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtdiskFull(FILE *fp);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtfclose(FILE *fp);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtfexists(char *file);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtisparent(Entry *cls, Entry *parent);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtmget(int setnum, int msgnum, char *msgbuf);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtmopen(char *filename);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtmprintf(int setnum, int msgnum, ...);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtokremove(char *file);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtunload(void *modHandle);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtwriteaccess(char *file);

// Allocate memory size of nbytes and filled with zeroes if clear flag is set
SOMEXTERN SOMDLLIMPORT void * OPTLINK_DECL somtsmalloc(size_t nbytes, BYTE clear);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtaddGAtt(MemBuf **membuf, AttList **ap, char *buf);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtcalcFileName(char *def, char *over, char *ext);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtcleanFilesFatal(int status);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtemitTypes(FILE *fp, Mlist *mp, Stab *stab);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somterror(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtfatal(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtinternal(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmclose(void);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmsg(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtreadDescFile(Stab *stab, char *file);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetDefaultDesc(Stab *stab);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetEmitSignals(void(*cleanup) (int), void (*internal) (int));
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetTypeDefn(Entry *type, Entry *ep, char *ptrs, Entry *ret, BOOL array);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetext(int csc);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtshowVersion(char *s, char *progname, char *sccsid);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsmfree(void *first, ...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtunsetEmitSignals(void);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtwarn(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtuppercase(char *s, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtlowercase(char *s, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcsuppercase(char *s, char *buf);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtdbcslowercase(char *s, char *buf);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtresetEmitSignals(void);
SOMEXTERN SOMDLLIMPORT size_t  OPTLINK_DECL somtsizeofEntry(SOMTTypes type);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtepname(Entry *ep, char *buf, BOOL suppressImpctxCheck);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtgenSeqName(long n, Entry *base, char *buf, BOOL fullname);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmrifatal(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmriinternal(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmrierror(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmrimsg(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtmriwarn(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtsetInternalMessages(char *too_long, char *cant_continue, char *segv, char *bus);
SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtisvoid(Entry *type, char *defn);
SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtreturnsStruct(Entry *ep);
SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtreturnsPtr(Entry *ep);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtsimpleName(Entry *ep);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtqualifyNames(Stab * stab, BOOL fully);
SOMEXTERN SOMDLLIMPORT Entry * OPTLINK_DECL somtfindBaseEpNonPtr(Entry *ep);
SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtprocessTraps(void);
SOMEXTERN SOMDLLIMPORT Mlist * OPTLINK_DECL somtallocMlist(Entry * ep);
SOMEXTERN SOMDLLIMPORT Mlist * OPTLINK_DECL somtmlistend(Mlist * mp, char *name);
SOMEXTERN SOMDLLIMPORT BOOL  OPTLINK_DECL somtisMutRef(Entry *ep, Mlist *seen, BOOL isself, long level);
SOMEXTERN SOMDLLIMPORT Mlist *  OPTLINK_DECL somtfreeMlist(Mlist *mp);
SOMEXTERN SOMDLLIMPORT Mlist *  OPTLINK_DECL somtdupMlist(Mlist *mp, Entry *ep);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtfreeWorld();
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtinitMalloc(BOOL dynamic);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseEmitlib(void);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseSmmeta(void);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseCreatetc(void);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseSmtypes(void);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseSomc(void);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtInitialiseSmsmall(void);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtattMap(char *name);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtexit(SOMTExitBuf *ebuf, int status);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtdymain(char *file, Entry *cls, EmitFn emitfn, char *emitter, int first, char *version, Stab *stab);
SOMEXTERN SOMDLLIMPORT void  OPTLINK_DECL somtaddHeader(char *file, FILE *fp, char *ext);
#endif

SOMEXTERN SOMDLLIMPORT int SOMLINK somttraverseParentsSL(FILE *fp, Entry * cls, Entry *arg, int (*fn)(FILE*,Entry*,Entry*), SMTraverse flg);
SOMEXTERN SOMDLLIMPORT AttList * SOMLINK somtattallocSL(MemBuf *membuf, char *name, char *value);
SOMEXTERN SOMDLLIMPORT AttList * SOMLINK somtgetapSL(AttList *ap, char *att);
SOMEXTERN SOMDLLIMPORT EmitFn  SOMLINK somtloadSL(char *fileName, char *functionName, void **modHandle);
SOMEXTERN SOMDLLIMPORT Entry * SOMLINK somtfindBaseEpSL(Entry *ep);
SOMEXTERN SOMDLLIMPORT Entry * SOMLINK somtgetTypeSL(char *name, SOMTTypes type);
SOMEXTERN SOMDLLIMPORT FILE * SOMLINK somtokfopenSL(char *path, char *mode);
SOMEXTERN SOMDLLIMPORT int  SOMLINK somtokfprintfSL(FILE *, const char *, ...);
SOMEXTERN SOMDLLIMPORT int  SOMLINK somtokfputsSL(const char *, FILE *);
SOMEXTERN SOMDLLIMPORT int  SOMLINK somtokfputcSL(const char, FILE *);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtokfgetsSL(char *, int, FILE *);
SOMEXTERN SOMDLLIMPORT void  SOMLINK somtokrewindSL(FILE *);
SOMEXTERN SOMDLLIMPORT int  SOMLINK somtokrenameSL(const char*, const char *);
SOMEXTERN SOMDLLIMPORT FILE * SOMLINK somtopenEmitFileSL(char *file, char *ext);
SOMEXTERN SOMDLLIMPORT BOOL  SOMLINK somtisDbcsSL(int c);
SOMEXTERN SOMDLLIMPORT BOOL  SOMLINK somtremoveExtSL(char *name, char *ext, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtaddExtSL(char *name, char *ext, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtarrayToPtrSL(Entry *ep, char *stars, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtattNormaliseSL(char *name, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtbasenameSL(char *path);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtctosSL(Const *con, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsPostincrSL(char **p);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsPreincrSL(char **p);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsStrchrSL(char *s, int c);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsStrrchrSL(char *s, int c);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsStrstrSL(char *s1, char *s2);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somteptotypeSL(Entry *ep, char *ptrs, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetDescSL(Stab *stab, Entry *cls, Entry *method, char *desc, BOOL addQuotes, BOOL use, BOOL versflg);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetVersionSL(char *sccsid, char *version);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgetgattSL(char *s);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtnextwordSL(const char *s, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtnormaliseDescSL(char *desc, char *normal);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtsatosSL(char **sa, char *sep, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtsearchFileSL(char *file, char *path, char *envvar);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtskipwsSL(const char *s);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtstringFmtSL(char *fmt, ...);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somttypeSL(SOMTTypes type);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtuniqFmtSL(MemBuf *membuf, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtargFlagSL(int *argc, char ***argv);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtattjoinSL(register AttList *ap1, AttList *ap2);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtdbcsLastCharSL(char *buf);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtdbcsScanSL(char **buf);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtdiskFullSL(FILE *fp);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtfcloseSL(FILE *fp);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtfexistsSL(char *file);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtisparentSL(Entry *cls, Entry *parent);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtmgetSL(int setnum, int msgnum, char *msgbuf);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtmopenSL(char *filename);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtmprintfSL(int setnum, int msgnum, ...);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtokremoveSL(char *file);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtunloadSL(void *modHandle);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtwriteaccessSL(char *file);

// Allocate memory size of nbytes and filled with zeroes if clear flag is set
SOMEXTERN SOMDLLIMPORT void * SOMLINK somtsmallocSL(size_t nbytes, BOOL clear);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtaddGAttSL(MemBuf **membuf, AttList **ap, char *buf);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtcalcFileNameSL(char *def, char *over, char *ext);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtcleanFilesFatalSL(int status);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtemitTypesSL(FILE *fp, Mlist *mp, Stab *stab);
SOMEXTERN SOMDLLIMPORT void SOMLINK somterrorSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtfatalSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtinternalSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtmcloseSL(void);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtmsgSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtreadDescFileSL(Stab *stab, char *file);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetDefaultDescSL(Stab *stab);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetEmitSignalsSL(void(*cleanup) (int), void (*internal) (int));
SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetTypeDefnSL(Entry *type, Entry *ep, char *ptrs, Entry *ret, BOOL array);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetextSL(int csc);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtshowVersionSL(char *s, char *progname, char *sccsid);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtsmfreeSL(void *first, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtunsetEmitSignalsSL(void);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtwarnSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtuppercaseSL(char *s, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtlowercaseSL(char *s, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcsuppercaseSL(char *s, char *buf);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtdbcslowercaseSL(char *s, char *buf);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtresetEmitSignalsSL(void);
SOMEXTERN SOMDLLIMPORT size_t SOMLINK somtsizeofEntrySL(SOMTTypes type);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtepnameSL(Entry *ep, char *buf, BOOL suppressImpctxCheck);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtgenSeqNameSL(long n, Entry *base, char *buf, BOOL fullname);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtmrifatalSL(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtmriinternalSL(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtmrierrorSL(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtmrimsgSL(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtmriwarnSL(char *file, long lineno, int msgnum,...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtsetInternalMessagesSL(char *too_long, char *cant_continue, char *segv, char *bus);
SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtisvoidSL(Entry *type, char *defn);
SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtreturnsStructSL(Entry *ep);
SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtreturnsPtrSL(Entry *ep);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtsimpleNameSL(Entry *ep);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtqualifyNamesSL(Stab * stab, BOOL fully);
SOMEXTERN SOMDLLIMPORT Entry * SOMLINK somtfindBaseEpNonPtrSL(Entry *ep);
SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtprocessTrapsSL(void);
SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtallocMlistSL(Entry * ep);
SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtmlistendSL(Mlist * mp, char *name);
SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtisMutRefSL(Entry *ep, Mlist *seen, BOOL isself, long level);
SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtfreeMlistSL(Mlist *mp);
SOMEXTERN SOMDLLIMPORT Mlist * SOMLINK somtdupMlistSL(Mlist *mp, Entry *ep);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtfreeWorldSL();
SOMEXTERN SOMDLLIMPORT void SOMLINK somtinitMallocSL(BOOL dynamic);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseEmitlibSL(void);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseSmmetaSL(void);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseCreatetcSL(void);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseSmtypesSL(void);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseSomcSL(void);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtInitialiseSmsmallSL(void);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtattMapSL(char *name);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtexitSL(SOMTExitBuf *ebuf, int status);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtdymainSL(char *file, Entry *cls, EmitFn emitfn, char *emitter, int first, char *version, Stab *stab);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtaddHeaderSL(char *file, FILE *fp, char *ext);

/*
 * Global variables
 */

SOMEXTERN global SOMDLLIMPORT Flags somtg_f;        /* Command line flags structure */

SOMEXTERN global SOMDLLIMPORT MemBuf *somtg_membuf; /* Memory buffer for string allocation */

SOMEXTERN global SOMDLLIMPORT Mlist *somtglbdefs;   /* Global defs */

SOMEXTERN global SOMDLLIMPORT SMEXT somtext;        /* Different idl/oidl extensions */

SOMEXTERN global SOMDLLIMPORT Stab *somtstab;       /* Pointer to symbol table */

SOMEXTERN global SOMDLLIMPORT BOOL somtaddstar;     /* Cached copy of global WIN32_DLLIMPORT modifier */

SOMEXTERN global SOMDLLIMPORT BOOL somtrmstar;      /* Cached copy of global WIN32_DLLIMPORT modifier */

SOMEXTERN global SOMDLLIMPORT char *somtg_buf;      /* Comment/pass thru buffer */

SOMEXTERN global SOMDLLIMPORT char *somtg_sccsid;   /* version information string */

SOMEXTERN global SOMDLLIMPORT char somtfilePath[];  /* input file full path */

SOMEXTERN global SOMDLLIMPORT char somtfileStem[];  /* infput file stem */

SOMEXTERN global SOMDLLIMPORT char somtoutPath[];   /* output file */

SOMEXTERN global SOMDLLIMPORT char *(*somtgetAtt) (AttList *, char *, char **);

SOMEXTERN global SOMDLLIMPORT Entry *somtmodule;    /* Pinter to last module encountered */

SOMEXTERN global SOMDLLIMPORT Mlist *somtclassList; /* List of classes that sc emits */

SOMEXTERN global SOMDLLIMPORT long somtsaved;       /* Bytes saved due to various
                                        * optimisations steps */

SOMEXTERN global SOMDLLIMPORT void *(*somtmalloc)(size_t nbytes)=malloc; 
SOMEXTERN global SOMDLLIMPORT void (*somtfree)(void *bp);        
SOMEXTERN global SOMDLLIMPORT void *(*somtcalloc)(size_t nmemb, size_t size);
SOMEXTERN global SOMDLLIMPORT void *(*somtrealloc)(void *bp, size_t nbytes); 

SOMEXTERN global SOMDLLIMPORT void (*somtcleanup_f) (int); /* cleanup signal function to call */
SOMEXTERN global SOMDLLIMPORT void (*somtinternal_f) (int);/* internal signal function to call */
SOMEXTERN global SOMDLLIMPORT int somtopenEmitFileCalled; 

SOMEXTERN global SOMDLLIMPORT SOMTExitBuf somtipcbuf;  /* global WIN32_DLLIMPORT buffer for the setjmp/longjmp */

SOMEXTERN global SOMDLLIMPORT SOMTExitBuf somtscbuf;   /* global WIN32_DLLIMPORT buffer for the setjmp/longjmp */

SOMEXTERN global SOMDLLIMPORT void *somtemitReturns; /* Emitter returns */

SOMEXTERN global SOMDLLIMPORT void (*somtrealExit)(int status); /* real exit to call */


#endif
