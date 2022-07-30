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

#ifdef _WIN32
#include <windows.h>
#endif

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

SOMEXTERN void * SOMLINK somtaddEntrySL(Stab *stab, char *name, void *ep)
{
  return NULL;
}

SOMEXTERN void * SOMLINK somtgetEntrySL(Stab *stab, char *name)
{
  return NULL;
}

SOMEXTERN void * SOMLINK somtstabFirstSL(Stab *stab, Sep **sepp)
{
  return NULL;
}

SOMEXTERN void * SOMLINK somtstabFirstNameSL(Stab *stab, char *name, Sep **sepp)
{
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

SOMEXTERN void SOMLINK somtcreateMemBufSL(MemBuf **membufp, size_t bufsize, long stabsize)
{
}

SOMEXTERN void SOMLINK somtcreateStabSL(Stab *stab, long stabsize, long entrysize)
{
}

SOMEXTERN int SOMLINK somticstrcmpSL(char *s, char *t)
{
  return 0;
}

SOMEXTERN void * SOMLINK somtaddEntryBufSL(Stab *stab, char *name, void *ep, void *buf, size_t len)
{
  return NULL;
}

SOMEXTERN void SOMLINK somtFreeStabSL(Stab *stab, BOOL freeEp)
{
}

