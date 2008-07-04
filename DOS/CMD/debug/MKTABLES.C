/*
 *	This program reads in the various data files, performs some checks,
 *	and spits out the tables needed for assembly and disassembly.
 *
 *	The input files are:
 *		instr.set	Instruction set
 *		instr.key	Translation from one- or two-character keys to
 *				operand list types
 *		instr.ord	Ordering relations to enforce on the operands
 *
 *	The output tables are merged into the debug source file debug.asm.
 */

#ifndef	DOS
#ifdef	__MSDOS__
#define	DOS	1
#else
#define	DOS	0
#endif
#endif

#include <stdlib.h>
#if	! DOS
#include <unistd.h>
#else
#include <io.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if	DOS
#define	CRLF	"\n"
#define	bzero(a, b)	memset(a, 0, b)
#else
#define	CRLF	"\r\n"
#define	cdecl
#endif

#define	MAX_OL_TYPES	82
#define	MAX_N_ORDS	30
#define	LINELEN		132
#define	MAX_ASM_TAB	2048
#define	MAX_MNRECS	400
#define	MAX_SAVED_MNEMS	10
#define	MAX_SLASH_ENTRIES 20
#define	MAX_HASH_ENTRIES 15
#define	MAX_STAR_ENTRIES 15
#define	MAX_LOCKTAB_ENTRIES 50
#define	MAX_AGROUP_ENTRIES 14
#define	MSHIFT		12	/* number of bits below machine type */

typedef	char	Boolean;
#define	True	1
#define	False	0

#define	NUMBER(x)	(sizeof(x) / sizeof(*x))

char		line[LINELEN];
const char	*filename;
int		lineno;

int	n_keys		= 0;
struct keytab {
	short	key;
	short	value;
	short	width;
};

int	n_ol_types	= 0;
struct keytab	olkeydict[MAX_OL_TYPES];
char		*olnames[MAX_OL_TYPES];
int		oloffset[MAX_OL_TYPES];

int	n_ords		= 0;
struct keytab	*keyord1[MAX_N_ORDS];
struct keytab	*keyord2[MAX_N_ORDS];
Boolean		ordsmall[MAX_OL_TYPES];

/*
 *	Equates for the assembler table.
 *	These should be the same as in debug.asm.
 */

#define	ASM_END		0xff
#define	ASM_DB		0xfe
#define	ASM_DW		0xfd
#define	ASM_DD		0xfc
#define	ASM_ORG		0xfb
#define	ASM_WAIT 	0xfa
#define	ASM_D32		0xf9
#define	ASM_D16		0xf8
#define	ASM_AAX		0xf7
#define	ASM_SEG		0xf6
#define	ASM_LOCKREP	0xf5
#define	ASM_LOCKABLE	0xf4
#define	ASM_MACH6	0xf3
#define	ASM_MACH0	0xed

int	n_asm_tab	= 0;
unsigned char	asmtab[MAX_ASM_TAB];

struct mnrec {
	struct mnrec	*next;
	char		*string;
	short		len;
	short		offset;
	short		asmoffset;
};

int		num_mnrecs;
struct mnrec	mnlist[MAX_MNRECS];
struct mnrec	*mnhead;

int	n_saved_mnems		= 0;
int	saved_mnem[MAX_SAVED_MNEMS];

int	n_slash_entries;
int	slashtab_seq[MAX_SLASH_ENTRIES];
int	slashtab_mn[MAX_SLASH_ENTRIES];

int	n_hash_entries;
int	hashtab_seq[MAX_HASH_ENTRIES];
int	hashtab_mn[MAX_HASH_ENTRIES];

int	n_star_entries;
int	startab_seq[MAX_STAR_ENTRIES];
int	startab_mn[MAX_STAR_ENTRIES];

int	n_locktab;
int	locktab[MAX_LOCKTAB_ENTRIES];

int	n_agroups;
int	agroup_i[MAX_AGROUP_ENTRIES];
int	agroup_inf[MAX_AGROUP_ENTRIES];

volatile void
fail(const char *message, ...)
{
	va_list	args;

	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	putc('\n', stderr);
	exit(1);
}

FILE *
openread(const char *path)
{
	FILE	*f;

	f = fopen(path, "r");
	if (f == NULL) {
	    perror(path);
	    exit(1);
	}
	filename = path;
	lineno = 0;
	return f;
}

volatile void
linenofail(const char *message, ...)
{
	va_list args;

	fprintf(stderr, "Line %d of `%s':  ", lineno, filename);
	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	putc('\n', stderr);
	exit(1);
}

void *
xmalloc(unsigned int len, const char *why)
{
	void	*ptr	= malloc(len);

	if (ptr == NULL) fail("Cannot allocate %u bytes for %s", len, why);
	return ptr;
}

Boolean
getline(FILE *ff)
{
	int	n;

	for (;;) {
	    if (fgets(line, LINELEN, ff) == NULL) return False;
	    ++lineno;
	    if (line[0] == '#') continue;
	    n = strlen(line) - 1;
	    if (n < 0 || line[n] != '\n')
		linenofail("too long.");
	    if (n > 0 && line[n-1] == '\r') --n;
	    if (n == 0) continue;
	    line[n] = '\0';
	    return True;
	}
}

short
getkey(char **pp)
{
	short	key;
	char	*p	= *pp;

	if (*p == ' ' || *p == '\t' || *p == ';' || *p == '\0')
	    linenofail("key expected");
	key = *p++;
	if (*p != ' ' && *p != '\t' && *p != ';' && *p != '\0') {
	    key = (key << 8) | *p++;
	    if (*p != ' ' && *p != '\t' && *p != ';' && *p != '\0')
		linenofail("key too long");
	}
	*pp = p;
	return key;
}

/*
 *	Mark the given key pointer as small, as well as anything smaller than
 *	it (according to instr.ord).
 */

void
marksmall(struct keytab *kp)
{
	int	i;

	ordsmall[kp - olkeydict] = True;
	for (i = 0; i < n_ords; ++i)
	    if (keyord2[i] == kp)
		marksmall(keyord1[i]);
}

/*
 *	Add a byte to the assembler table (asmtab).
 *	The format of this table is described in a long comment in debug.asm,
 *	somewhere within the mini-assembler.
 */

void
add_to_asmtab(unsigned char byte)
{
	if (n_asm_tab >= MAX_ASM_TAB)
	    linenofail("Assembler table overflow.");
	asmtab[n_asm_tab++] = byte;
}


unsigned char
getmachine(char **pp)
{
	char		*p	= *pp;
	unsigned char	value;

	if (*p != ';') return 0;
	++p;
	if (*p < '0' || *p > '6')
	    linenofail("bad machine type");
	value = *p++ - '0';
	add_to_asmtab(ASM_MACH0 + value);
	*pp = p;
	return value;
}

struct keytab *
lookupkey(short key)
{
	struct keytab *kp;

	for (kp = olkeydict; kp < olkeydict + NUMBER(olkeydict); ++kp)
	    if (key == kp->key) return kp;
    linenofail("can't find key");
    return NULL;
}

char *
skipwhite(char *p)
{
	while (*p == ' ' || *p == '\t') ++p;
	return p;
}

/*
 *	Data and setup stuff for the disassembler processing.
 */

/*	Data on coprocessor groups */

unsigned int	fpgrouptab[]	= {0xd9e8, 0xd9f0, 0xd9f8};

#define	NGROUPS		9

#define	GROUP(i)	(256 + 8 * ((i) - 1))
#define	COPR(i)		(256 + 8 * NGROUPS + 16 * (i))
#define	FPGROUP(i)	(256 + 8 * NGROUPS + 16 * 8 + 8 * (i))
#define	SPARSE_BASE	(256 + 8 * NGROUPS + 16 * 8 \
			    + 8 * NUMBER(fpgrouptab))

/* #define OPILLEGAL	0 */
#define	OPTWOBYTE	2
#define	OPGROUP		4
#define	OPCOPR		6
#define	OPFPGROUP	8
#define	OPPREFIX	10
#define	OPSIMPLE	12
#define	OPTYPES		12	/* op types start here (includes simple ops) */

#define	PRESEG		1	/* these should be the same as in debug.asm */
#define	PREREP		2
#define	PREREPZ		4
#define	PRELOCK		8
#define	PRE32D		0x10
#define	PRE32A		0x20

/*
 *	For sparsely filled parts of the opcode map, we have counterparts
 *	to the above, which are compressed in a simple way.
 */

/*	Sparse coprocessor groups */

unsigned int	sp_fpgrouptab[]	= {0xd9d0, 0xd9e0, 0xdae8, 0xdbe0, 
				   0xded8, 0xdfe0};

#define	NSGROUPS	5

#define	SGROUP(i)	(SPARSE_BASE + 256 + 8 * ((i) - 1))
#define	SFPGROUP(i)	(SPARSE_BASE + 256 + 8 * NSGROUPS + 8 * (i))
#define	NOPS		(SPARSE_BASE + 256 + 8 * NSGROUPS \
			+ 8 * NUMBER(sp_fpgrouptab))

int		optype[NOPS];
int		opinfo[NOPS];
unsigned char	opmach[NOPS];

/*
 *	Here are the tables for the main processor groups.
 */

struct {
	int	seq;	/* sequence number of the group */
	int	info;	/* which group number it is */
}
	grouptab[]	= {
		{0x80, GROUP(1)},	/* Intel group 1 */
		{0x81, GROUP(1)},
		{0x83, GROUP(2)},
		{0xd0, GROUP(3)},	/* Intel group 2 */
		{0xd1, GROUP(3)},
		{0xd2, GROUP(4)},
		{0xd3, GROUP(4)},
		{0xc0, GROUP(5)},	/* Intel group 2a */
		{0xc1, GROUP(5)},
		{0xf6, GROUP(6)},	/* Intel group 3 */
		{0xf7, GROUP(6)},
		{0xff, GROUP(7)},	/* Intel group 5 */
		{SPARSE_BASE + 0x00, GROUP(8)},		/* Intel group 6 */
		{SPARSE_BASE + 0x01, GROUP(9)}};	/* Intel group 7 */

/* #define	NGROUPS	9 (this was done above) */

struct {	/* sparse groups */
	int	seq;	/* sequence number of the group */
	int	info;	/* which group number it is */
}
	sp_grouptab[]	= {
		{0xfe, SGROUP(1)},		/* Intel group 4 */
		{SPARSE_BASE+0xba, SGROUP(2)},	/* Intel group 8 */
		{SPARSE_BASE+0xc7, SGROUP(3)},	/* Intel group 9 */
		{0x8f, SGROUP(4)},		/* Not an Intel group */
		{0xc6, SGROUP(5)},		/* Not an Intel group */
		{0xc7, SGROUP(5)}};

/* #define	NSGROUPS	5 (this was done above) */

/*
 *	Creates an entry in the disassembler lookup table
 */

void
entertable(int i, int type, int info)
{
	if (optype[i] != 0)
	    linenofail("Duplicate information for index %d", i);
	optype[i] = type;
	opinfo[i] = info;
}

/*
 *	Get a hex nybble from the input line or fail.
 */

int
getnybble(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    linenofail("Hex digit expected instead of `%c'", c);
    return -1;
}

/*
 *	Get a hex byte from the input line and update the pointer accordingly.
 */

int
getbyte(char **pp)
{
	char	*p = *pp;
	int	answer;

	answer = getnybble(*p++);
	answer = (answer << 4) | getnybble(*p++);
	*pp = p;
	return answer;
}

/*
 *	Get a `/r' descriptor from the input line and update the pointer
 *	accordingly.
 */

int
getslash(char **pp)
{
	char	*p = *pp;
	int	answer;

	if (*p != '/') linenofail("`/' expected");
	++p;
	if (*p < '0' || *p > '7') linenofail("Octal digit expected");
	answer = *p - '0';
	++p;
	*pp = p;
	return answer;
}

int
entermn(char *str, char *str_end)
{
	char	*p;

	if (num_mnrecs >= MAX_MNRECS)
	    linenofail("Too many mnemonics");

	if (*str == '+') {
	    if (n_saved_mnems >= MAX_SAVED_MNEMS)
		linenofail("Too many mnemonics to save");
	    saved_mnem[n_saved_mnems++] = num_mnrecs;
	    ++str;
	}

	p = xmalloc(str_end - str + 1, "mnemonic name");
	mnlist[num_mnrecs].string = p;
	mnlist[num_mnrecs].len = str_end - str;
	while (str < str_end) *p++ = toupper(*str++);
	*p = 0;
	mnlist[num_mnrecs].asmoffset = n_asm_tab;
	return num_mnrecs++;
}

/*
 *	Merge sort the indicated range of mnemonic records.
 */

struct mnrec *
mn_sort(struct mnrec *start, int len)
{
	struct mnrec	*p1, *p2, *answer;
	struct mnrec	**headpp;
	int		i;

	i = len / 2;
	if (i == 0)
	    return start;

	p1 = mn_sort(start, i);
	p2 = mn_sort(start + i, len - i);
	headpp = &answer;
	for (;;)
	    if (strcmp(p1->string, p2->string) < 0) {
		*headpp = p1;
		headpp = &p1->next;
		p1 = *headpp;
		if (p1 == NULL) {
		    *headpp = p2;
		    break;
		}
	    }
	    else {
		*headpp = p2;
		headpp = &p2->next;
		p2 = *headpp;
		if (p2 == NULL) {
		    *headpp = p1;
		    break;
		}
	    }

	return answer;
}

/*
 *	This reads the main file, "instr.set".
 */

void
read_is(FILE *f1)
{
	int	i;

	entertable(0x0f, OPTWOBYTE, SPARSE_BASE);
	entertable(0x26, OPPREFIX, PRESEG | (0 << 8));	/* seg es */
	entertable(0x2e, OPPREFIX, PRESEG | (1 << 8));	/* seg cs */
	entertable(0x36, OPPREFIX, PRESEG | (2 << 8));	/* seg ss */
	entertable(0x3e, OPPREFIX, PRESEG | (3 << 8));	/* seg ds */
	entertable(0x64, OPPREFIX, PRESEG | (4 << 8));	/* seg fs */
	entertable(0x65, OPPREFIX, PRESEG | (5 << 8));	/* seg gs */
	entertable(0xf2, OPPREFIX, PREREP);		/* other prefixes */
	entertable(0xf3, OPPREFIX, PREREP | PREREPZ);
	entertable(0xf0, OPPREFIX, PRELOCK);
	entertable(0x66, OPPREFIX, PRE32D);
	entertable(0x67, OPPREFIX, PRE32A);
	opmach[0x64] = opmach[0x65] = opmach[0x66] = opmach[0x67] = 3;
	
	for (i = 0; i < NUMBER(grouptab); ++i)
	    entertable(grouptab[i].seq, OPGROUP, grouptab[i].info);
	for (i = 0; i < NUMBER(sp_grouptab); ++i)
	    entertable(sp_grouptab[i].seq, OPGROUP, sp_grouptab[i].info);
	for (i = 0; i < 8; ++i)
	    entertable(0xd8 + i, OPCOPR, COPR(i));
	for (i = 0; i < NUMBER(fpgrouptab); ++i) {
	    unsigned int j = fpgrouptab[i];
	    unsigned int k = (j >> 8) - 0xd8;

	    if (k > 8 || (j & 0xff) < 0xc0)
		fail("Bad value for fpgrouptab[%d]", i);
	    entertable(COPR(k) + 8 + (((j & 0xff) - 0xc0) >> 3),
		OPFPGROUP, FPGROUP(i));
	}
	for (i = 0; i < NUMBER(sp_fpgrouptab); ++i) {
	    unsigned int j = sp_fpgrouptab[i];
	    unsigned int k = (j >> 8) - 0xd8;

	    if (k > 8 || (j & 0xff) < 0xc0)
		fail("Bad value for sp_fpgrouptab[%d]", i);
	    entertable(COPR(k) + 8 + (((j & 0xff) - 0xc0) >> 3),
		OPFPGROUP, SFPGROUP(i));
	}
	while (getline(f1)) {	/* loop over lines in the file */
	    int mnem;
	    int mn_alt;
	    char *p, *p0, *pslash, *phash, *pstar;
	    Boolean asm_only_line;
	    unsigned char atab_addendum;

	    asm_only_line = False;
	    p0 = line;
	    if (line[0] == '_') {
		asm_only_line = True;
		++p0;
	    }
	    atab_addendum = '\0';
	    if (*p0 == '^') {
		static	const	unsigned char	uptab[]	=
					{ASM_AAX, ASM_DB, ASM_DW,
					 ASM_DD, ASM_ORG, ASM_D32};

		++p0;
		atab_addendum = uptab[*p0++ - '0'];
	    }
	    p = strchr(p0, ' ');
        if (p == NULL) p = p0 + strlen(p0);

	    pslash = memchr(p0, '/', p - p0);
	    if (pslash != NULL) {
            mnem = entermn(p0, pslash);
            add_to_asmtab(ASM_D16);
//          ++mnlist[mnem].asmoffset;	/* this one isn't 32 bit */
            ++pslash;
            mn_alt = entermn(pslash, p);
            add_to_asmtab(ASM_D32);
	    } else {
            phash = memchr(p0, '#', p - p0);
            if (phash != NULL) {
                mnem = entermn(p0, phash);
                add_to_asmtab(ASM_D16);
//              ++mnlist[mnem].asmoffset;	/* this one isn't 32 bit */
                ++phash;
                mn_alt = entermn(phash, p);
                add_to_asmtab(ASM_D32);
            } else {
                pstar = memchr(p0, '*', p - p0);
                if (pstar != NULL) {
                    mn_alt = entermn(p0, pstar);	/* note the reversal */
                    ++pstar;
                    add_to_asmtab(ASM_WAIT);
                    mnem = entermn(pstar, p);
                }
                else
                    mnem = entermn(p0, p);
            }
        }

	    if (atab_addendum != '\0') add_to_asmtab(atab_addendum);

	    atab_addendum = ASM_END;
	    bzero(ordsmall, n_keys * sizeof(Boolean));
	    while (*p == ' ') {		/* loop over instruction variants */
		Boolean		lockable;
		Boolean		asm_only;
		Boolean		dis_only;
		unsigned char	machine;
		unsigned long	atab_inf;
		unsigned short	atab_key;
		unsigned char	atab_xtra	= 0;

		while (*p == ' ') ++p;
		asm_only = asm_only_line;
		dis_only = False;
		if (*p == '_') {	/* if assembler only */
		    ++p;
		    asm_only = True;
		}
		else if (*p == 'D') {	/* if disassembler only */
		    ++p;
		    dis_only = True;
		}
		lockable = False;
		if (*p == 'L') {
		    ++p;
		    lockable = True;
		    add_to_asmtab(ASM_LOCKABLE);
		}
		atab_inf = i = getbyte(&p);
		if (i == 0x0f) {
		    i = getbyte(&p);
		    atab_inf = 256 + i;
		    i += SPARSE_BASE;
		}
		if (optype[i] == OPGROUP) {
		    int j = getslash(&p);
		    int k;

		    for (k = 0;; ++k) {
			if (k >= n_agroups) {
			    if (++n_agroups > MAX_AGROUP_ENTRIES)
				linenofail("Too many agroup entries");
			    agroup_i[k] = i;
			    agroup_inf[k] = atab_inf;
			    break;
			}
			if (agroup_i[k] == i)
			    break;
		    }
		    atab_inf = 256 + 256 + 64 + 8 * k + j;
		    i = opinfo[i] + j;
		}
		if (optype[i] == OPCOPR) {
		    if (*p == '/') {
			int	j	= getslash(&p);

			atab_inf = 256 + 256 + j * 8 + (i - 0xd8);
			i = opinfo[i] + j;
		    }
		    else {
			atab_xtra = getbyte(&p);
			if (atab_xtra < 0xc0)
			    linenofail("Bad second escape byte");
			i = opinfo[i] + 8 + ((atab_xtra - 0xc0) >> 3);
			if (optype[i] == OPFPGROUP)
			    i = opinfo[i] + (atab_xtra & 7);
		    }
		}
		switch (*p++) {
		    case '.':
			machine = getmachine(&p);
			if (!asm_only) {
			    entertable(i, OPSIMPLE, mnem);
			    opmach[i] = machine;
			}
			atab_key = 0;
			/* none of these are lockable */
			break;
		    case '*':	/* lock or rep... prefix */
			add_to_asmtab(ASM_LOCKREP);
			add_to_asmtab(atab_inf);	/* special case */
			atab_addendum = '\0';
			break;
		    case '&':	/* segment prefix */
			add_to_asmtab(ASM_SEG);
			add_to_asmtab(atab_inf);	/* special case */
			atab_addendum = '\0';
			break;
		    case ':': {
			    struct keytab *kp = lookupkey(getkey(&p));
			    int width = kp->width;
			    int j;

			    machine = getmachine(&p);
			    if (dis_only)
				atab_addendum = '\0';
			    else {
				if (ordsmall[kp - olkeydict])
				    linenofail("Variants out of order.");
				marksmall(kp);
			    }
			    atab_key = kp->value + 1;
			    if ((i >= 256 && i < SPARSE_BASE)
				    || i >= SPARSE_BASE + 256) {
				if (width > 2)
				    linenofail("width failure");
				width = 1;
			    }
			    if (i & (width - 1))
				linenofail("width alignment failure");
			    if (!asm_only)
				for (j = (i == 0x90); j < width; ++j) {
				    /*    ^^^^^^^^^  kludge for NOP instr. */
				    entertable(i|j, oloffset[kp->value], mnem);
				    opmach[i | j] = machine;
				    if (lockable) {
					if (n_locktab >= MAX_LOCKTAB_ENTRIES)
					    linenofail("Too many lockable "
					      "instructions");
					locktab[n_locktab] = i | j;
					++n_locktab;
				    }
				}
			}
			break;
		    default:
			linenofail("Syntax error.");
		}
		if (atab_addendum != '\0') {
		    atab_inf = atab_inf * (unsigned short) (n_ol_types + 1)
		      + atab_key;
		    add_to_asmtab(atab_inf >> 8);
		    if ((atab_inf >> 8) >= ASM_MACH0)
			fail("Assembler table is too busy");
		    add_to_asmtab(atab_inf);
		    if (atab_xtra != 0)
			add_to_asmtab(atab_xtra);
		}
		if (pslash != NULL) {
                    if (n_slash_entries >= MAX_SLASH_ENTRIES)
                        linenofail("Too many slash entries");
		    slashtab_seq[n_slash_entries] = i;
		    slashtab_mn[n_slash_entries] = mn_alt;
		    ++n_slash_entries;
                } else {
                    if (phash != NULL) {
                        if (n_hash_entries >= MAX_HASH_ENTRIES)
                            linenofail("Too many hash entries");
                        hashtab_seq[n_hash_entries] = i;
                        hashtab_mn[n_hash_entries] = mn_alt;
                        ++n_hash_entries;
                    } else {
                        if (pstar != NULL) {
                            if (n_star_entries >= MAX_STAR_ENTRIES)
                                linenofail("Too many star entries");
                            startab_seq[n_star_entries] = i;
                            startab_mn[n_star_entries] = mn_alt;
                            ++n_star_entries;
                        }
                    }
                }
            }
	    if (*p != '\0')
		linenofail("Syntax error.");
	    if (atab_addendum != '\0')
		add_to_asmtab(atab_addendum);	/* ASM_END, if applicable */
	}
}

/*
 *	Print everything onto the file.
 */

struct inforec {	/* strings to put into comment fields */
	int	seqno;
	char	*string;
}
	tblcomments[] = {
		{0, "main opcode part"},
		{GROUP(1), "Intel group 1"},
		{GROUP(3), "Intel group 2"},
		{GROUP(5), "Intel group 2a"},
		{GROUP(6), "Intel group 3"},
		{GROUP(7), "Intel group 5"},
		{GROUP(8), "Intel group 6"},
		{GROUP(9), "Intel group 7"},
		{COPR(0), "Coprocessor d8"},
		{COPR(1), "Coprocessor d9"},
		{COPR(2), "Coprocessor da"},
		{COPR(3), "Coprocessor db"},
		{COPR(4), "Coprocessor dc"},
		{COPR(5), "Coprocessor dd"},
		{COPR(6), "Coprocessor de"},
		{COPR(7), "Coprocessor df"},
		{FPGROUP(0), "Coprocessor groups"},
		{-1, NULL}};

void
put_dw(FILE *f2, const char *label, int *datap, int n)
{
	const char	*initstr;
	int		i;

	fputs(label,f2);
	while (n > 0) {
	    initstr = "\tdw\t";
	    for (i = (n <= 8 ? n : 8); i > 0; --i) {
		fputs(initstr, f2);
		initstr = ",";
		fprintf(f2, "0%xh", *datap++);
	    }
	    fputs(CRLF, f2);
	    n -= 8;
	}
}

char * copystry(char * pSrc)
{
    static char cHlp[20];
    char * pDst = cHlp;
    *pDst++ = '\"';
    for (;;pDst++, pSrc++) {
        if (*(pSrc+1) == 0) {
            *pDst++ = '\"';
            *pDst++ = ',';
            *pDst++ = ' ';
            *pDst++ = '\"';
            *pDst++ = *pSrc;
            strcpy(pDst, "\"+80h");
            break;
        }
        *pDst = *pSrc;
    }
    return cHlp;
}

void
dumptables(FILE *f2)
{
	int		offset;
	struct mnrec	*mnp;
	int		colsleft;
	char		*auxstr;
	struct inforec	*tblptr;
	int		i;
	int		j;
    int		k;

	/*
	 * Dump out asmtab.
	 */

	auxstr = CRLF ";\tMain data table for the assembler." CRLF CRLF
        "asmtab\tdb ";
    offset = 0;
	for (i = 0; i < n_asm_tab; ++i) {
	    if (asmtab[i] == 0xFF) {
            fprintf(f2, "%s-1\t; ofs %04x" CRLF, auxstr, offset);
            auxstr = "\tdb ";
            offset = i+1;
        } else {
            fprintf(f2, "%s%03xh", auxstr, asmtab[i]);
            auxstr = ", ";
	    }
	}

	/*
	 * Dump out agroup_inf.
	 */

	auxstr = CRLF CRLF ";\tData on groups (for the assembler)." CRLF CRLF
	  "agroups\tdw\t";
	for (i = 0; i < n_agroups; ++i) {
	    fprintf(f2, "%s%d", auxstr, agroup_inf[i]);
	    auxstr = ",";
	}

	/*
	 * Sort the mnemonics alphabetically, compute their offsets,
	 * and print out the table.
	 */

	if (num_mnrecs == 0)
	    fail("No assembler mnemonics!");
	mnhead = mn_sort(mnlist, num_mnrecs);
	offset = 0;
	fputs(CRLF CRLF ";\tThis is the list of assembler mnemonics." CRLF
        CRLF "mnlist:\n", f2);
	for (mnp = mnhead; mnp != NULL; mnp = mnp->next) {
	    mnp->offset = offset + 2;
	    offset += mnp->len + 2;
	    fprintf(f2, "\tmne %04xh, %s\n", mnp->asmoffset, copystry(mnp->string));
	}
	fputs(CRLF "end_mnlist:" CRLF, f2);
	if (offset >= (1 << MSHIFT)) {
	    fprintf(stderr, "%d bytes of mnemonics.  That's too many.\n",
		offset);
	    exit(1);
	}

	/*
	 * Print out mnemonics we want to refer to.
	 */

	fputs(CRLF ";\tThese are equates to refer to the above mnemonics."
		CRLF CRLF, f2);
	for (i = 0; i < n_saved_mnems; ++i) {
        mnp = mnlist + saved_mnem[i];
	    fprintf(f2, "MNEM_%s\tEQU\t%d" CRLF, mnp->string, mnp->offset);
	}

	/*
	 * Print out ASMMOD and the opindex array.
	 */

	fprintf(f2, CRLF ";\tNumber of entries in the 'opindex' array." CRLF
	  CRLF "ASMMOD\tEQU\t%d" CRLF, n_ol_types + 1);

	auxstr = CRLF ";\tThis is an array of byte offsets for the oplists array "
	  "(below)." CRLF
	  ";\tIt is used by the assembler to save space." CRLF CRLF
	  "opindex\tdb\t0,";
	colsleft = 16;
	for (i = 0; i < n_ol_types; ++i) {
	    fprintf(f2, "%s%d", auxstr, oloffset[i] - OPTYPES);
	    auxstr = ",";
	    if (--colsleft <= 0) {
		auxstr = CRLF "\tdb\t";
		colsleft = 16;
	    }
	}

	/*
	 * Print out oplists[]
	 */

	fputs(CRLF CRLF ";\tThese are the lists of operands "
	    "for the various instruction types." CRLF CRLF
	    "oplists\tdb\t0\t;simple instruction" CRLF, f2);
	for (i = 0; i < n_ol_types; ++i)
	    fprintf(f2, "\tdb\t%s, 0" CRLF, olnames[i]);
	fprintf(f2, CRLF "OPTYPES_BASE\tEQU\t%d" CRLF, OPTYPES);
	fprintf(f2, CRLF "OPLIST_Z\tEQU\t%d" CRLF,
	    oloffset[lookupkey('z')->value]);
	fprintf(f2, "OPLIST_ES\tEQU\t%d" CRLF,
	    oloffset[lookupkey(('E' << 8) | 'S')->value]);

	/*
	 * Print out optype[]
	 */

	auxstr = CRLF ";\tHere is the compressed table of the opcode types."
	    CRLF CRLF "optypes\tdb\t";
	tblptr = tblcomments;

	for (i = 0; i < SPARSE_BASE; i += 8) {
	    for (j = 0; j < 8; ++j) {
		fputs(auxstr, f2);
		fprintf(f2, "%3d", optype[i + j]);
		auxstr = ",";
	    }
	    fprintf(f2, "\t; %02x - %02x", i, i + 7);
        if (i == tblptr->seqno) {
            fprintf(f2, " (%s)", (tblptr++)->string);
        }
	    auxstr = CRLF "\tdb\t";
	}
	auxstr = CRLF ";\tThe rest of these are squeezed." CRLF "\tdb\t 0,";
	colsleft = 7;
	for (i = SPARSE_BASE; i < NOPS; ++i)
	    if ((j = optype[i]) != 0) {
		if (--colsleft < 0) {
		    colsleft = 7;
		    auxstr = CRLF "\tdb\t";
		}
		fputs(auxstr, f2);
		fprintf(f2, "%3d", j);
		auxstr = ",";
	    }
	fputs(CRLF, f2);

	/*
	 * Print out opinfo[]
	 */

	fputs(CRLF ";\tAnd here is the compressed table of additional "
	    "information." CRLF CRLF "opinfo", f2);

	for (i = 0; i < SPARSE_BASE; i += 8) {
	    auxstr = "\tdw\t";
	    for (j = 0; j < 8; ++j) {
		fputs(auxstr, f2);
		k = opinfo[i + j];
		if (optype[i + j] >= OPTYPES)
		    k = mnlist[k].offset;
		fprintf(f2, "0%04xh", k | (opmach[i + j] << MSHIFT));
		auxstr = ",";
	    }
	    fprintf(f2, "\t; %02x" CRLF, i);
	}
	auxstr = ";\tThe rest of these are squeezed." CRLF "\tdw\t     0,";
	colsleft = 7;
	for (i = SPARSE_BASE; i < NOPS; ++i)
	    if ((j = optype[i]) != 0) {
		if (--colsleft < 0) {
		    colsleft = 7;
		    auxstr = CRLF "\tdw\t";
		}
		fputs(auxstr, f2);
		k = opinfo[i];
		if (j >= OPTYPES) k = mnlist[k].offset;
		fprintf(f2, "0%04xh", k | (opmach[i] << MSHIFT));
		auxstr = ",";
	    }
	fputs(CRLF, f2);

	/*
	 * Print out sqztab
	 */

	fputs(CRLF ";\tThis table converts unsqueezed numbers to squeezed."
	    CRLF CRLF "sqztab", f2);

	k = 0;
	for (i = SPARSE_BASE; i < NOPS; i += 8) {
	    auxstr = "\tdb\t";
	    for (j = 0; j < 8; ++j) {
		fprintf(f2, "%s%3d", auxstr, optype[i + j] == 0 ? 0 : ++k);
		auxstr = ",";
	    }
	    fputs(CRLF, f2);
	}

	/*
	 * Print out the cleanup tables.
	 */

	fputs(CRLF ";\tThis is the table of mnemonics that change in the "
		"presence of a WAIT" CRLF ";\tinstruction." CRLF CRLF, f2);
	put_dw(f2, "wtab1", startab_seq, n_star_entries);
	for (i = 0; i < n_star_entries; ++i)
	    startab_mn[i] = mnlist[startab_mn[i]].offset;
	put_dw(f2, "wtab2", startab_mn, n_star_entries);
	fprintf(f2, "N_WTAB\tequ\t%d" CRLF, n_star_entries);

	fputs(CRLF ";\tThis is the table for operands which have a different "
		"mnemonic for" CRLF ";\ttheir 32 bit versions (66h prefix)." CRLF CRLF, f2);
	put_dw(f2, "ltab1", slashtab_seq, n_slash_entries);
	for (i = 0; i < n_slash_entries; ++i)
	    slashtab_mn[i] = mnlist[slashtab_mn[i]].offset;
	put_dw(f2, "ltab2", slashtab_mn, n_slash_entries);
	fprintf(f2, "N_LTAB\tequ\t%d" CRLF, n_slash_entries);

	fputs(CRLF ";\tThis is the table for operands which have a different "
		"mnemonic for" CRLF ";\ttheir 32 bit versions (67h prefix)." CRLF CRLF, f2);
	put_dw(f2, "ltab1X", hashtab_seq, n_hash_entries);
	for (i = 0; i < n_hash_entries; ++i)
	    hashtab_mn[i] = mnlist[hashtab_mn[i]].offset;
	put_dw(f2, "ltab2X", hashtab_mn, n_hash_entries);
	fprintf(f2, "N_LTABX\tequ\t%d" CRLF, n_hash_entries);

	fputs(CRLF ";\tThis is the table of lockable instructions" CRLF CRLF,
	    f2);
	put_dw(f2, "locktab", locktab, n_locktab);
	fprintf(f2, "N_LOCK\tequ\t%d" CRLF, n_locktab);

	/*
	 * Print out miscellaneous equates.
	 */

	fprintf(f2, CRLF ";\tEquates used in the assembly-language code." CRLF
	    CRLF "SPARSE_BASE\tequ\t%d" CRLF
	    CRLF "SFPGROUP3\tequ\t%d" CRLF CRLF, SPARSE_BASE, SFPGROUP(3));
}

int cdecl
main() {
	FILE	*f1;
	FILE	*f2;
	int	offset;

	/*
	 * Read in the key dictionary.
	 */

	f1 = openread("instr.key");
	offset = OPTYPES + 1;
	while (getline(f1)) {
	    char *p = line;
	    int i;

	    if (n_keys >= MAX_OL_TYPES)
		fail("Too many keys.");
	    olkeydict[n_keys].key = getkey(&p);
	    p = skipwhite(p);
	    for (i = 0;; ++i) {
		if (i >= n_ol_types) {
		    char *q = xmalloc(strlen(p) + 1, "operand type name");

		    strcpy(q, p);
		    if (n_ol_types >= MAX_OL_TYPES)
			fail("Too many operand list types.");
		    olnames[n_ol_types] = q;
		    oloffset[n_ol_types] = offset;
		    for (;;) {
			++offset;
			q = strchr(q, ',');
			if (q == NULL) break;
			++q;
		    }
		    ++offset;
		    ++n_ol_types;
		}
		if (strcmp(p, olnames[i]) == 0)
		    break;
	    }
	    olkeydict[n_keys].value = i;
	    olkeydict[n_keys].width = 1;
	    if (strstr(p, "OP_ALL") != NULL)
		olkeydict[n_keys].width = 2;
	    else if (strstr(p, "OP_R_ADD") != NULL)
		olkeydict[n_keys].width = 8;
	    ++n_keys;
	}
	fclose(f1);
	if (offset >= 256) {
	    fprintf(stderr, "%d bytes of operand lists.  That's too many.\n",
		offset);
	    exit(1);
	}

	/*
	 * Read in the ordering relations.
	 */

	f1 = openread("instr.ord");
	while (getline(f1)) {
	    char *p = line;

	    if (n_ords >= MAX_N_ORDS)
		fail ("Too many ordering restrictions.");
	    keyord1[n_ords] = lookupkey(getkey(&p));
	    p = skipwhite(p);
	    keyord2[n_ords] = lookupkey(getkey(&p));
	    if (*p != '\0')
		fail("Syntax error in ordering file.");
	    ++n_ords;
	}
	fclose(f1);

	/*
	 * Do the main processing.
	 */

	f1 = openread("instr.set");
	read_is(f1);
	fclose(f1);

	/*
	 * Write the file.
	 */

	f2 = fopen("debugtbl.tmp", "w");
	if (f2 == NULL) {
	    perror("debugtbl.tmp");
	    exit(1);
	}

	dumptables(f2);

    fclose(f2);

	/*
	 * Move the file to its original position.
	 */

    unlink("debugtbl.old");

	if (rename("debugtbl.inc", "debugtbl.old") == -1) {
	    perror("rename debugtbl.inc -> debugtbl.old");
//	    return 1;
	}
	if (rename("debugtbl.tmp", "debugtbl.inc") == -1) {
	    perror("rename debugtbl.tmp -> debugtbl.inc");
	    return 1;
	}

	puts("Done.");

	return 0;
}
