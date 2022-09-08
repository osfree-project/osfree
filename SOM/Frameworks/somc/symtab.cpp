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

#include <symtab.h>

#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN char * OPTLINK_DECL somtallocBuf(MemBuf *membuf, void *buf, long len)
{
  OPTLINK_3ARGS(membuf, buf, len);

  return somtallocBufSL(membuf, buf, len);
}

SOMEXTERN char * OPTLINK_DECL somtuniqString(MemBuf *membuf, char *s)
{
  OPTLINK_2ARGS(membuf, s);

  return somtuniqStringSL(membuf, s);
}

SOMEXTERN long OPTLINK_DECL somtkeyword(KeytabEntry *keytab, char *kword, long keytabsize)
{
  OPTLINK_3ARGS(keytab, kword, keytabsize);

  return somtkeywordSL(keytab, kword, keytabsize);
}

SOMEXTERN void * OPTLINK_DECL somtaddEntry(Stab *stab, char *name, void *ep)
{
  OPTLINK_3ARGS(stab, name, ep);

  return somtaddEntrySL(stab, name, ep);
}

SOMEXTERN void * OPTLINK_DECL somtgetEntry(Stab *stab, char *name)
{
  OPTLINK_2ARGS(stab, name);

  return somtgetEntrySL(stab, name);
}

SOMEXTERN void * OPTLINK_DECL somtstabFirst(Stab *stab, Sep **sepp)
{
  OPTLINK_2ARGS(stab, sepp);

  return somtstabFirstSL(stab, sepp);
}

SOMEXTERN void * OPTLINK_DECL somtstabFirstName(Stab *stab, char *name, Sep **sepp)
{
  OPTLINK_3ARGS(stab, name, sepp);

  return somtstabFirstNameSL(stab, name, sepp);
}

SOMEXTERN void * OPTLINK_DECL somtstabNext(Stab *stab, Sep **sepp)
{
  OPTLINK_2ARGS(stab, sepp);

  return somtstabNextSL(stab, sepp);
}

SOMEXTERN void * OPTLINK_DECL somtstabNextName(Stab *stab, Sep **sepp)
{
  OPTLINK_2ARGS(stab, sepp);

  return somtstabNextNameSL(stab, sepp);
}

SOMEXTERN void OPTLINK_DECL somtcreateMemBuf(MemBuf **membufp, size_t bufsize, long stabsize)
{
  OPTLINK_3ARGS(membufp, bufsize, stabsize);

  somtcreateMemBufSL(membufp, bufsize, stabsize);
}

SOMEXTERN void OPTLINK_DECL somtcreateStab(Stab *stab, long stabsize, long entrysize)
{
  OPTLINK_3ARGS(stab, stabsize, entrysize);

  somtcreateStabSL(stab, stabsize, entrysize);
}

SOMEXTERN int OPTLINK_DECL somticstrcmp(char *s, char *t)
{
  OPTLINK_2ARGS(s, t);

  return somticstrcmpSL(s, t);
}

SOMEXTERN void * OPTLINK_DECL somtaddEntryBuf(Stab *stab, char *name, void *ep, void *buf, size_t len)
{
  OPTLINK_3ARGS(stab, name, ep);

  return somtaddEntryBufSL(stab, name, ep, buf, len);
}

SOMEXTERN void OPTLINK_DECL somtFreeStab(Stab *stab, BOOL freeEp)
{
  OPTLINK_2ARGS(stab, freeEp);

  somtFreeStabSL(stab, freeEp);
}

#endif

SOMEXTERN char * SOMLINK somtallocBufSL(MemBuf *membuf, void *buf, long len)
{
  return NULL;
}

SOMEXTERN char * SOMLINK somtuniqStringSL(MemBuf *membuf, char *s)
{
  return NULL;
}

SOMEXTERN long SOMLINK somtkeywordSL(KeytabEntry *keytab, char *kword, long keytabsize)
{
  return 0;
}

// Calcualte hash index to include in Stab slot
size_t getHashIndex(Stab *stab, char *name)
{
  size_t index=0;
  if (name)
  {
    while(name[0]!=0)
    {
      index=index+tolower(name[0]);
    }
    if (index) index=(index % (stab->size-1)) + 1;
  }
  return index;
}

Sep * addItem(Stab *stab, Sep *sep, char *name, void *ep, void *epbuf, int entrysize)
{
  sep
}
                             
SOMEXTERN void * SOMLINK somtaddEntrySL(Stab *stab, char *name, void *ep)
{
  Sep * buf=stab->buf+getHashIndex(stab, name)*sizeof(Sep);

  buf=buf->ep;

  if (buf)
  {
    while(buf->next)
    {
      buf=buf->next;
    }
  }

  return addItem(stab, buf, name, ep, NULL, stab->entrysize) ;
}

SOMEXTERN void * SOMLINK somtgetEntrySL(Stab *stab, char *name)
{
  getHashIndex(stab, name);
  return NULL;
}

SOMEXTERN void * SOMLINK somtstabFirstSL(Stab *stab, Sep **sepp)
{
  return NULL;
}

SOMEXTERN void * SOMLINK somtstabFirstNameSL(Stab *stab, char *name, Sep **sepp)
{
  getHashIndex(stab, name);
  return NULL;
}

SOMEXTERN void * SOMLINK somtstabNextSL(Stab *stab, Sep **sepp)
{
  return NULL;
}

SOMEXTERN void * SOMLINK somtstabNextNameSL(Stab *stab, Sep **sepp)
{
  return NULL;
}

void * globalbuf;

SOMEXTERN void SOMLINK somtcreateMemBufSL(MemBuf **membufp, size_t bufsize, long stabsize)
{
  MemBuf * membuf=membufp^;
  if (!membuf)
  {
    membuf=somtsmallocSL(sizeof(MemBuf), 1);
  }
  membuf->size=bufsize;
  membuf->cur=somtsmallocSL(bufsize, 0);
  if (stabsize)
  {
    membuf->stab=somtsmallocSL(sizeof(Stab), 0);
    somtcreateStabSL(membuf->stab, stabsize, sizeof(SEntry));
    membuf->stab->membuf=membuf;
    somtallocBufSL(membuf->base, globalbuf, 1);
  }
}

SOMEXTERN void SOMLINK somtcreateStabSL(Stab *stab, long stabsize, long entrysize)
{
  stab->size=stabsize;
  stab->entrysize=entrysize;
  stab->buf=somtsmallocSL(stabsize*sizeof(Sep), TRUE);
  stab->base=stab->buf;
  stab->limit=stab->buf+stabsize*sizeof(Sep);
  stab->nelms=0;
  stab->membuf=NULL;
}

SOMEXTERN int SOMLINK somticstrcmpSL(char *s, char *t)
{
  return _stricmp(s, t);
}

SOMEXTERN void * SOMLINK somtaddEntryBufSL(Stab *stab, char *name, void *ep, void *buf, size_t len)
{
  getHashIndex(stab, name);
  return NULL;
}

SOMEXTERN void SOMLINK somtFreeStabSL(Stab *stab, BOOL freeEp)
{
}

