/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1993-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * $Id: staticld.c,v 1.2 2003/12/11 04:43:22 prokushev Exp $
 */

#include "rexx.h"

#if defined( DYNAMIC_STATIC )
typedef void *(*RPFN)( char *);

#ifdef HAVE_REXXUTIL_PACKAGE
extern void *getRexxUtilFunctionAddress( char *name );
#endif

#ifdef HAVE_TEST_PACKAGE
extern void *getTest1FunctionAddress( char *name );
extern void *getTest2FunctionAddress( char *name );
#endif

#ifdef HAVE_REXXTK_PACKAGE
extern void *getRexxTkFunctionAddress( char *name );
#endif

#ifdef HAVE_REXXCURSES_PACKAGE
extern void *getRexxCursesFunctionAddress( char *name );
#endif

#ifdef HAVE_REXXGD_PACKAGE
extern void *getRexxGdFunctionAddress( char *name );
#endif

#ifdef HAVE_REXXISAM_PACKAGE
extern void *getRexxISAMFunctionAddress( char *name );
#endif

#ifdef HAVE_REXXCURL_PACKAGE
extern void *getRexxCURLFunctionAddress( char *name );
#endif

#ifdef HAVE_REXXSQL_PACKAGE
extern void *getRexxSQLFunctionAddress( char *name );
#endif

#ifdef HAVE_REXXDW_PACKAGE
extern void *getRexxDWFunctionAddress( char *name );
#endif

#ifndef max
# define max(a,b)        (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
# define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif

static struct
{
    char *name;
    void *(*funcptr)(char *name);
} RexxPackages[] =
{
#ifdef HAVE_TEST_PACKAGE
   { "test1", getTest1FunctionAddress} ,
   { "test2", getTest2FunctionAddress} ,
#endif
#ifdef HAVE_REXXUTIL_PACKAGE
   { "rexxutil", getRexxUtilFunctionAddress} ,
#endif
#ifdef HAVE_REXXTK_PACKAGE
   { "rexxtk", getRexxTkFunctionAddress },
#endif
#ifdef HAVE_REXXCURSES_PACKAGE
   { "rxcurses", getRexxCursesFunctionAddress },
#endif
#ifdef HAVE_REXXGD_PACKAGE
   { "rexxgd", getRexxGdFunctionAddress },
#endif
#ifdef HAVE_REXXISAM_PACKAGE
   { "rexxisam", getRexxISAMFunctionAddress },
#endif
#ifdef HAVE_REXXCURL_PACKAGE
   { "rexxcurl", getRexxCURLFunctionAddress },
#endif
#ifdef HAVE_REXXSQL_PACKAGE
   { "rexxsql", getRexxSQLFunctionAddress },
#endif
#ifdef HAVE_RXSOCK_PACKAGE
   { "rxsock", getRxSockFunctionAddress },
#endif
#ifdef HAVE_REXXDW_PACKAGE
   { "rexxdw", getRexxDWFunctionAddress },
#endif
   { "", NULL },
};
static int my_stricmp( char *str1,char *str2 )
{
   int len1,len2,len,rc;

   len1 = strlen( str1 );
   len2 = strlen( str2 );
   len = min( len1, len2 );

   rc = mem_cmpic( str1, str2 );
   if ( rc != 0 )
      return rc;

   if ( len1 > len2 )
      return(1);
   if ( len1 < len2 )
      return(-1);
   return(0);
}

void *static_dlopen( char *name )
{
   int i, num_packages ;
   num_packages = sizeof( RexxPackages ) / sizeof( *RexxPackages );
   for ( i = 0; i < num_packages; i++)
   {
      if ( my_stricmp( RexxPackages[i].name, name ) == 0 )
         return RexxPackages[i].funcptr;
   }
   return NULL;
}

int static_dlsym( void *addr, char *name, void **faddr )
{
   RPFN rpaddr = (RPFN)addr;
   if ( rpaddr == NULL )
   {
      return 30;
   }
   *faddr = (*rpaddr)( name );
   if (*faddr == NULL )
      return 1;
   else
      return 0;
}
#endif
