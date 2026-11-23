#ifndef DOSQSS_H
#define DOSQSS_H

#ifdef __cplusplus
extern "C" {
#endif
APIRET APIENTRY DosQuerySysState(ULONG func,ULONG arg1,ULONG arg2,
			ULONG _res_,PVOID buf,ULONG bufsz);
#ifdef __cplusplus
}
#endif

typedef struct {
	ULONG	threadcnt;
	ULONG	proccnt;
	ULONG	modulecnt;
} QGLOBAL, *PQGLOBAL;

typedef struct {
	ULONG	rectype;	/* 256 for thread */
	USHORT	threadid;
	USHORT	slotid;
	ULONG	sleepid;
	ULONG	priority;
	ULONG	systime;
	ULONG	usertime;
	UCHAR	state;
	UCHAR	_reserved1_;	/* padding to ULONG */
	USHORT	_reserved2_;	/* padding to ULONG */
} QTHREAD, *PQTHREAD;

typedef struct {
	USHORT	sfn;
	USHORT	refcnt;
	ULONG	flags;
	ULONG	accmode;
	ULONG	filesize;
	USHORT  volhnd;
	USHORT	attrib;
	USHORT	_reserved_;
} QFDS, *PQFDS;

typedef struct qfile {
	ULONG		rectype;	/* 8 for file */
	struct qfile	*next;
	ULONG		opencnt;
	PQFDS		filedata;
	char		name[1];
} QFILE, *PQFILE;

typedef struct {
	ULONG	rectype;	/* 1 for process */
	PQTHREAD threads;
	USHORT	pid;
	USHORT	ppid;
	ULONG	type;
	ULONG	state;
	ULONG	sessid;
	USHORT	hndmod;
	USHORT	threadcnt;
	ULONG	privsem32cnt;
	ULONG	_reserved2_;
	USHORT	sem16cnt;
	USHORT	dllcnt;
	USHORT	shrmemcnt;
	USHORT	fdscnt;
	PUSHORT	sem16s;
	PUSHORT	dlls;
	PUSHORT	shrmems;
	PUSHORT	fds;
} QPROCESS, *PQPROCESS;

typedef struct sema {
	struct sema *next;
	USHORT	refcnt;
	UCHAR	sysflags;
	UCHAR	sysproccnt;
	ULONG	_reserved1_;
	USHORT	index;
	CHAR	name[1];
} QSEMA, *PQSEMA;

typedef struct {
	ULONG	rectype;	/**/
	ULONG	_reserved1_;
	USHORT	_reserved2_;
	USHORT	syssemidx;
	ULONG	index;
	QSEMA	sema;
} QSEMSTRUC, *PQSEMSTRUC;

typedef struct {
	USHORT	pid;
	USHORT	opencnt;
} QSEMOWNER32, *PQSEMOWNER32;

typedef struct {
	PQSEMOWNER32	own;
	PCHAR		name;
	PVOID		semrecs; /* array of associated sema's */
	USHORT		flags;
	USHORT		semreccnt;
	USHORT		waitcnt;
	USHORT		_reserved_;	/* padding to ULONG */
} QSEMSMUX32, *PQSEMSMUX32;

typedef struct {
	PQSEMOWNER32	own;
	PCHAR		name;
	PQSEMSMUX32	mux;
	USHORT		flags;
	USHORT		postcnt;
} QSEMEV32, *PQSEMEV32;

typedef struct {
	PQSEMOWNER32	own;
	PCHAR		name;
	PQSEMSMUX32	mux;
	USHORT		flags;
	USHORT		refcnt;
	USHORT		thrdnum;
	USHORT		_reserved_;	/* padding to ULONG */
} QSEMMUX32, *PQSEMMUX32;

typedef struct semstr32 {
	struct semstr32 *next;
	QSEMEV32 evsem;
	QSEMMUX32  muxsem;
	QSEMSMUX32 smuxsem;
} QSEM32STRUC, *PQSEM32STRUC;

typedef struct shrmem {
	struct shrmem *next;
	USHORT	hndshr;
	USHORT	selshr;
	USHORT	refcnt;
	CHAR	name[1];
} QSHRMEM, *PQSHRMEM;

typedef struct module {
	struct module *next;
	USHORT	hndmod;
	USHORT	type;
	ULONG	refcnt;
	ULONG	segcnt;
	PVOID	_reserved_;
	PCHAR	name;
	USHORT	modref[1];
} QMODULE, *PQMODULE;

typedef struct {
	PQGLOBAL	gbldata;
	PQPROCESS	procdata;
	PQSEMSTRUC	semadata;
	PQSEM32STRUC	sem32data;	/* not always present */
	PQSHRMEM	shrmemdata;
	PQMODULE	moddata;
	PVOID		_reserved2_;
	PQFILE		filedata;	/* only present in FP19 or later or W4 */
} QTOPLEVEL, *PQTOPLEVEL;

#endif
