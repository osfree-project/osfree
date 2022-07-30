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

#ifndef symtab_h
#define symtab_h

#include <stddef.h>

#include <smcidl.h>

#include <som.h>

#ifdef _WIN32
#include <windows.h>
#endif

//#ifndef bool
//#define bool boolean
//#endif

#define SepNULL     (Sep *) NULL

/*
 * Forward reference the Symbol table:
 */
struct Stab;

/*
 * Default Entry for a symbol table
 */
_typedef struct SEntry {
    char *name;
} _name(SEntry);

/*
 * Keyword structure.
 */
_typedef struct KeyTabEntry {
    char *kword;
    long token;
} _name(KeytabEntry);

/*
 * Symbol table entry linked list
 */
_typedef struct Sep {
    size_t posn;
    SEntry *ep;
    struct Sep *next;
} _name(Sep);

/*
 *  MemBuf is the structure used to allocate new memory in contiguous blocks.
 *  We can use it for unique strings too if the uniq flag is set when calling
 *  somtcreateMemBuf().
 */
_typedef struct MemBuf {
    char *base;
    size_t size;
    size_t nelms;
    char *cur;
    _struct Stab *stab;
} _name(MemBuf);

/*
 * Basic Symbol Table (STab) structure.
 */
_typedef struct Stab {
    size_t size;
    long entrysize;
    size_t nelms;
    Sep *base;
    Sep *limit;
    Sep *buf;
    MemBuf *membuf;
    bool ignorecase;
} _name(Stab);

#ifndef __SOMIDL__

#include <optlink.h>

// Here old Optlink caling convention functions having SOMLINK equals
#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN char * OPTLINK_DECL somtallocBuf(MemBuf *membuf, void *buf, long len);
SOMEXTERN char * OPTLINK_DECL somtuniqString(MemBuf *membuf, char *s);
SOMEXTERN long  OPTLINK_DECL somtkeyword(KeytabEntry *keytab, char *kword, long keytabsize);
SOMEXTERN void * OPTLINK_DECL somtaddEntry(Stab *stab, char *name, void *ep);
SOMEXTERN void * OPTLINK_DECL somtgetEntry(Stab *stab, char *name);
SOMEXTERN void * OPTLINK_DECL somtstabFirst(Stab *stab, Sep **sepp);
SOMEXTERN void * OPTLINK_DECL somtstabFirstName(Stab *stab, char *name, Sep **sepp);
SOMEXTERN void * OPTLINK_DECL somtstabNext(Stab *stab, Sep **sepp);
SOMEXTERN void * OPTLINK_DECL somtstabNextName(Stab *stab, Sep **sepp);
SOMEXTERN void  OPTLINK_DECL somtcreateMemBuf(MemBuf **membufp, size_t bufsize, long stabsize);
SOMEXTERN void  OPTLINK_DECL somtcreateStab(Stab *stab, long stabsize, long entrysize);
SOMEXTERN int  OPTLINK_DECL somticstrcmp(char *s, char *t);
SOMEXTERN void * OPTLINK_DECL somtaddEntryBuf(Stab *stab, char *name, void *ep, void *buf, size_t len);
SOMEXTERN void  OPTLINK_DECL somtFreeStab(Stab *stab, BOOL freeEp);
#endif

SOMEXTERN char * SOMLINK somtallocBufSL(MemBuf *membuf, void *buf, long len);
SOMEXTERN char * SOMLINK somtuniqStringSL(MemBuf *membuf, char *s);
SOMEXTERN long SOMLINK somtkeywordSL(KeytabEntry *keytab, char *kword, long keytabsize);
SOMEXTERN void * SOMLINK somtaddEntrySL(Stab *stab, char *name, void *ep);
SOMEXTERN void * SOMLINK somtgetEntrySL(Stab *stab, char *name);
SOMEXTERN void * SOMLINK somtstabFirstSL(Stab *stab, Sep **sepp);
SOMEXTERN void * SOMLINK somtstabFirstNameSL(Stab *stab, char *name, Sep **sepp);
SOMEXTERN void * SOMLINK somtstabNextSL(Stab *stab, Sep **sepp);
SOMEXTERN void * SOMLINK somtstabNextNameSL(Stab *stab, Sep **sepp);
SOMEXTERN void SOMLINK somtcreateMemBufSL(MemBuf **membufp, size_t bufsize, long stabsize);
SOMEXTERN void SOMLINK somtcreateStabSL(Stab *stab, long stabsize, long entrysize);
SOMEXTERN int SOMLINK somticstrcmpSL(char *s, char *t);
SOMEXTERN void * SOMLINK somtaddEntryBufSL(Stab *stab, char *name, void *ep, void *buf, size_t len);
SOMEXTERN void SOMLINK somtFreeStabSL(Stab *stab, BOOL freeEp);
#endif  /* __SOMIDL__ */

#endif  /* symtab_h */
