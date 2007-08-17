#ifndef lint
static char *RCSid = "$Id: extlib.c,v 1.2 2003/12/11 04:43:08 prokushev Exp $";
#endif

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

#include "rexx.h"

struct extlib_funcbox {
   struct extlib_funcbox *next, *prev ;
   streng *name ;
   int type ;
   int hash1 ;
};

#define EXTFUNCS_COUNT (sizeof(((tsd_t *)0)->extfuncs) /   \
                        sizeof(((tsd_t *)0)->extfuncs[0]))

static struct extlib_funcbox *findfunc( const tsd_t *TSD, const streng *name, int *hash,
                                        int *hashbox )
{
   struct extlib_funcbox *fptr=NULL ;
   int lhashbox, lhash ;

   *hash = lhash = hashvalue( name->value, name->len ) ;
   *hashbox = lhashbox = lhash % EXTFUNCS_COUNT ;
   for (fptr=TSD->extfuncs[lhashbox]; fptr; fptr=fptr->prev)
      if (fptr->hash1 == lhash)
         if (!Str_cmp(name, fptr->name))
            return fptr ;

   return NULL ;
}

int delfunc( tsd_t *TSD, const streng *name )
{
   struct extlib_funcbox *old=NULL ;
   int hash, hashbox ;

   old = findfunc( TSD, name, &hash, &hashbox ) ;
   if (!old)
      return 1 ;

   Free_stringTSD( old->name ) ;
   if (old==TSD->extfuncs[hashbox])
      TSD->extfuncs[hashbox] = old->prev ;
   else
      old->next->prev = old->prev ;

   if (old->prev)
      old->prev->next = old->next ;

   FreeTSD( old ) ;
   return 0 ;
}

/* addfunc returns 1 on success, 0 if already defined, -1 if memory is short.
 * The argument name is used for further operation on success only.
 */
int addfunc( tsd_t *TSD, streng *name, int type )
{
   struct extlib_funcbox *new=NULL ;
   int hashbox, hash ;

   if (findfunc( TSD, name, &hash, &hashbox ))
      return 0 ;

   new = MallocTSD( sizeof(struct extlib_funcbox )) ;
   if (!new)
      return -1 ;

   new->name = name ;
   new->type = type ;
   new->next = NULL ;
   new->hash1 = hash ;
   new->prev = TSD->extfuncs[hashbox] ;
   if (TSD->extfuncs[hashbox])
      TSD->extfuncs[hashbox]->next = new ;
   TSD->extfuncs[hashbox] = new ;

   return 1 ;
}

int external_func( const tsd_t *TSD, const streng *name )
{
   struct extlib_funcbox *ptr=NULL ;
   int hash, hashbox ;

   ptr = findfunc( TSD, name, &hash, &hashbox ) ;
   if (ptr)
      return 1 ;
   else
      return 0 ;
}

