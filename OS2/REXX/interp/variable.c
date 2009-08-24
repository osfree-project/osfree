#ifndef lint
static char *RCSid = "$Id: variable.c,v 1.42 2006/09/15 04:06:09 mark Exp $";
#endif

/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992  Anders Christensen <anders@pvv.unit.no>
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
 * Concept: Each REXX procedure (the top - or main - by default) has
 *  an array of hash-pointers. Routines that don't keep local
 *  variables (i.e hasn't seen a PROCEDURE) are using the hashpointers
 *  of the routine above them. The initial size of the array is
 *  vt->initialHashTableLength
 *
 * Each cell in this array is a pointer to a single-linked list of
 *  boxes. In common for all these boxes is that their name returns
 *  the same value when given as parameter to the hashfunc() function.
 *
 * Each of these boxes contains five variables: name, value, index,
 *  realbox and next. 'next' points to next box in the list.
 *
 * 'name' is the name of the variable, and 'value' is the value it
 *  contains. However, if 'realbox' is set, it points to another
 *  box which contains the real value of the variable. This mechanism
 *  gives support for EXPOSE'ing variables in PROCEDUREs.
 *
 * The 'index' is a pointer to another hashtable, and gives support
 *  for compound variables. If a variable is compound, its 'index' is
 *  set to point at the hashtable, each entry in this table do also
 *  point at the start of a single linked list of variable boxes, but
 *  these boxes has the 'after-the-period' part of the compound name
 *  as 'name'. The 'realbox', but not the 'index' may be set in these
 *  boxes.
 *
 * A variable is set when it exists in the datastructures, and the
 *  relevant 'value' pointer is non-NULL. When dropping a variable
 *  that is EXPOSE'ed, the 'value' is set to NULL.
 *
 * The 'test' and the 'test.' variables have two different
 *  variableboxes, and 'index' is only set in the box for 'test.'. A
 *  'realbox' existing for 'test' makes it exposed. A 'realbox'
 *  'test.' make the whole "array" exposed.
 *
 * A 'value' existing for 'test.' denotes the default value.
 *
 * Major performance improvements without much to do which should be
 * implemented (FIXME):
 *
 * 1) Introduce a comparison function which uppercases the second argument
 *    only. The variable box itself always has the correct case. Just the
 *    script's name of the variable may have the "wrong" case.
 *    Check for contents when uppercasing numbers as well.
 */

#include "rexx.h"
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>   /* f*ck sun, they can't write a proper assert!!! */

typedef struct {
   void **Elems;
   unsigned size;
} Pool;

typedef struct { /* var_tsd: static variables of this module (thread-safe) */
#ifdef TRACEMEM
   variableptr    first_invalid;
#endif
#ifdef DEBUG
   int            DoDebug;
   FILE *         DebugFile;
   char           PoolNameBuf[20];
   Pool           NamePool;
   Pool           ValuePool;
   Pool           NumPool;
   Pool           VarPool;
#endif
   int            foundflag;
   variableptr    thespot;
   long           current_valid;
   /* We CAN'T increment current_valid on each new procedure (which results
    * into create_new_varpool) and decrement it in procedure exit (which
    * executes kill_variables). Imagine the following:
    * >call proc1
    * >exit 0
    * >proc1: procedure
    * > locvar = 1
    * > call proc2
    * > call proc1 (not endlessly, but at least one time)
    * > return
    * >proc2: procedure expose locvar
    * > return
    * In the first call to proc2 current_valid will be 3, proc2 returns, proc1
    * will be executed at least once more and current_valid is 3, too. This is
    * OK if and only if the variable accessment in proc1 and proc2 are distinct
    * by a procedure counter (each procedure has its own number). This is NOT
    * realized. Thus, we increment a separate counter (next_current_valid) each
    * time a new procedure is called and assign the value to current_valid.
    * On procedure return we set back current_valid to the current_value of
    * this procedure instance which may be, but don't MUST be, current_level-1.
    * Of course, this is bogus! The next_current_valid counter may wrap around
    * and we run into trouble once more. We can reset next_current_valid to
    * 2 (initial current_valid+1) savely iff current_valid==1. This prevents
    * some problems with multiple calls to Rexx when started but don't help in
    * one execution run. Since Regina is dog slow this will PROBABLY never
    * happen.
    * For a correct way of operation see many compiler building books of
    * languages with call by name.
    * Former releases uses stupid generation counter mechanisms.
    * FGC 27.09.98 (09/27/98)
    */
   long           next_current_valid;
   int            subst;
   unsigned       hashval;
   unsigned       fullhash;
   int            ignore_novalue;
   int            notrace;
   streng *       tmpindex;
   streng *       ovalue;
   streng *       xvalue;
   num_descr *    odescr;
   variableptr    pstem;
   variableptr    ptail;
   variableptr    rstem;
   variableptr    rtail;
   unsigned       stemidx;
   unsigned       tailidx;
   var_hashtable *var_table;
   var_hashtable *pool0;
   treenode       pool0nodes[POOL0_CNT][2];
   int            initialHashTableLength;
} var_tsd_t;

#define SEEK_EXPOSED(ptr) if (ptr)                                          \
                          {                                                 \
                             for ( ; (ptr)->realbox; ptr = (ptr)->realbox ) \
                                ;                                           \
                          }
#define REPLACE_VALUE(val,p) { if ( p->value )                             \
                                  Free_stringTSD( p->value );              \
                               p->value = val;                             \
                               p->guard = 0;                               \
                               p->flag = ( val ) ? VFLAG_STR : VFLAG_NONE; \
                             }

#define REPLACE_NUMBER(val,p) { if ( p->num )                             \
                                {                                         \
                                   FreeTSD( p->num->num );                \
                                   FreeTSD( p->num );                     \
                                }                                         \
                                p->num = val;                             \
                                p->guard = 0;                             \
                                p->flag = ( val ) ? VFLAG_NUM : VFLAG_NONE; \
                              }

#ifdef DEBUG
static void regina_dprintf( const tsd_t *TSD, const char *fmt, ... )
{
   va_list marker;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   if ( !vt->DoDebug )
      return;
   va_start( marker, fmt );
   vfprintf( vt->DebugFile, fmt, marker );
   fflush( vt->DebugFile );
   va_end( marker );
}
#  define DPRINT(x) if ( vt->DoDebug & 1) regina_dprintf x
#  define DPRINT_2(x) if ( vt->DoDebug & 2 ) regina_dprintf x
#  define DPRINT_3(x) if ( vt->DoDebug & 4 ) regina_dprintf x
#  define DSTART DPRINT(( TSD, "%2u %4d ", TSD->thread_id, __LINE__ ))
#  define DSTART_2 DPRINT_2(( TSD, "%2u %4d ", TSD->thread_id, __LINE__ ))
#  define DSTART_3 DPRINT_3(( TSD, "%2u %4d ", TSD->thread_id, __LINE__ ))
#  define DEND DPRINT(( TSD, "\n" ))
#  define DEND_2 DPRINT_2(( TSD, "\n" ))
#  define DEND_3 DPRINT_3(( TSD, "\n" ))

static const volatile char *PoolName( const tsd_t *TSD, Pool *pool,
                                      const void *elem )
{
   unsigned i;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   if ( !vt->DoDebug ) /* PoolName's return value isn't really used if */
      return NULL;     /* debugging is turned off. */
   if ( pool == &vt->NamePool )
      strcpy( vt->PoolNameBuf, "NAME" );
   else if ( pool == &vt->ValuePool )
      strcpy( vt->PoolNameBuf, "VAL" );
   else if ( pool == &vt->NumPool )
      strcpy( vt->PoolNameBuf, "NUM" );
   else if ( pool == &vt->VarPool )
      strcpy( vt->PoolNameBuf, "VAR" );
   else
      return "????";
   for ( i = 0; i < pool->size; i++ )
      if ( pool->Elems[i] == elem )
         break;
   sprintf( vt->PoolNameBuf + strlen( vt->PoolNameBuf ), "%u",
                                                         i + 1 );
   if ( i >= pool->size )
   {
      pool->size++;
      if ( ( pool->Elems = (void **)realloc( pool->Elems,
                                    pool->size * sizeof( void * ) ) ) == NULL )
      {
         exiterror( ERR_STORAGE_EXHAUSTED, 0 ) ;
      }
   }
   pool->Elems[i] = (void *) elem;
   return vt->PoolNameBuf;
}

#define DNAME(TSD,name,n) if ( vt->DoDebug & 1 ) DNAME2( TSD, name, n )
static void DNAME2( const tsd_t *TSD, const char *name, const streng* n )
{
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   if ( !vt->DoDebug )
      return;
   if ( name != NULL )
      regina_dprintf( TSD, "%s=",
                           name );
   if ( n == NULL )
   {
      regina_dprintf( TSD, "NULL" );
      return;
   }
   regina_dprintf( TSD, "\"%*.*s\"%s",
                        Str_len( n ),
                        Str_len( n ),
                        n->value,
                        PoolName( TSD, &vt->NamePool, n ) );
}

#define DVALUE(TSD,name,v) if ( vt->DoDebug & 1 ) DVALUE2( TSD, name, v )
static void DVALUE2( const tsd_t *TSD, const char *name, const streng* v )
{
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   if ( !vt->DoDebug )
      return;
   if ( name != NULL )
      regina_dprintf( TSD, "%s=",
                           name );
   if ( v == NULL )
   {
      regina_dprintf( TSD, "NULL" );
      return;
   }
   regina_dprintf( TSD, "\"%*.*s\"%s",
                        Str_len( v ),
                        Str_len( v ),
                        v->value,
                        PoolName( TSD, &vt->ValuePool, v ) );
}

#define DNUM(TSD,name,n) if ( vt->DoDebug & 1 ) DNUM2( TSD, name, n )
static void DNUM2( const tsd_t *TSD, const char *name, const num_descr* n)
{
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   if ( !vt->DoDebug )
      return;
   if ( name != NULL )
      regina_dprintf( TSD, "%s=",
                           name );
   if ( n == NULL )
   {
      regina_dprintf( TSD, "NULL" );
      return;
   }
   regina_dprintf( TSD, "\"%*.*s\"%s",
                        n->size,
                        n->size,
                        n->num,
                        PoolName( TSD, &vt->NumPool, n ) );
}

static int Dfindlevel(const tsd_t *TSD, cvariableptr v)
{
   proclevel curr;
   int lvl=0;
   unsigned i;

   curr = TSD->currlevel;

   while ( curr )
   {
      if (curr->vars)
      {
         for ( i = 0; i < curr->vars->size; i++ )
         {
            if ( curr->vars->tbl[i] == v )
               goto found;
         }
      }
      curr = curr->prev;
      lvl++;
   }
   return -1;
found:
   while ( curr->prev )
   {
      curr = curr->prev;
      lvl++;
   }
   return lvl;
}

#define DVAR(TSD,name,v) if ( vt->DoDebug & 1 ) DVAR2( TSD, name, v )
static void DVAR2( const tsd_t *TSD, const char *name, cvariableptr v )
{
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   if ( !vt->DoDebug )
      return;
   if ( name != NULL )
      regina_dprintf( TSD, "%s=",
                           name );
   if ( v == NULL )
   {
      regina_dprintf( TSD, "NULL" );
      return;
   }

   regina_dprintf( TSD, "%s,l=%d(",
                        PoolName( TSD, &vt->VarPool, v ),
                        Dfindlevel( TSD, v ) );
   if ( v->valid == 0 )
      regina_dprintf( TSD, "?" );
   else
   {
      DNAME( TSD, NULL, v->name );
      regina_dprintf( TSD, "," );
      DVALUE( TSD, NULL, v->value );
      regina_dprintf( TSD, "=" );
      DNUM( TSD, NULL, v->num );
   }
   regina_dprintf( TSD, ",hwired=%ld,valid=%ld",
                        v->hwired, v->valid );
   if ( v->realbox )
   {
      regina_dprintf( TSD, "->" );
      DVAR( TSD, NULL, v->realbox );
   }
   regina_dprintf( TSD, ")" );
}
#  define DPRINTF(x) DSTART;DPRINT(x);DEND
#  define DPRINTF_2(x) DSTART_2;DPRINT_2(x);DEND_2
#  define DPRINTF_3(x) DSTART_3;DPRINT_3(x);DEND_3
#  define DARG(x,y) x
#else /* !define(DEBUG) */
#  define DPRINT(x)
#  define DPRINT_2(x)
#  define DPRINT_3(x)
#  define DSTART
#  define DSTART_2
#  define DSTART_3
#  define DEND
#  define DEND_2
#  define DEND_3
#  define DNAME(t,n,v)
#  define DVALUE(t,n,v)
#  define DNUM(t,n,v)
#  define DVAR(t,n,v)
#  define DPRINTF(x)
#  define DPRINTF_2(x)
#  define DPRINTF_3(x)
#  define DARG(x,y) y
#endif /* DEBUG */

/*
 * NEW_HASHTABLE_CHECK is invoked after a new entry has been added to the
 * hashtable. Do a check for this special condition and set flag to 1 if
 * a hashtable reorganisation should happen.
 * One (maybe wrong) idea is to increase the table if 100% has been filled.
 * NEVER EVER USE LESS THAN 40% CAPACITY! THIS MAY BE THE LOW WATER MARK!
 */
#define NEW_HASHTABLE_CHECK(tbl,flag) /* */          \
   (flag |= (((tbl)->e * 1 > (tbl)->size * 1) ? 1 : 0)) /* set to 5/4 for 80%*/

/*
 * COLLISION_CHECK is invoked after a collision has been detected.
 * The check may happen rather often. KEEP IT SIMPLE!
 * A hashtable reorganisation should/may happen in this case; this must be
 * indicated by setting flag to 1.
 * One (maybe wrong) idea is to increase the table if the number of collisions
 * is bigger than two time of the number of read/write accesses.
 */
#define COLLISION_CHECK(tbl,flag) /* */                  \
   (flag |= (((tbl)->w + (tbl)->r < (tbl)->c / 2) ? 1 : 0))


/*
 * The SEEK_VAR_... functions walk through the list of variable candidates
 * in the bucket of the found hash with more or less help by other informations
 * to minimize expensive comparisons.
 */
#define SEEK_VAR_CMP(run,var,fullhash,tbl,flag) /* */                         \
   for ( ; (run); (run) = (run)->next )                                       \
   {                                                                          \
      if ( ( (run)->hash == (fullhash) ) &&                                   \
           ( (run)->name->len == (var)->len ) &&                              \
           ( Str_cmp( run->name, var ) == 0 ) )                               \
         break;                                                               \
                                                                              \
      (tbl)->c++;                                                             \
      COLLISION_CHECK(tbl, flag);                                             \
   }

#define SEEK_VAR_CCMP(run,var,fullhash,tbl,flag) /* */                        \
   for ( ; (run); (run) = (run)->next )                                       \
   {                                                                          \
      if ( ( (run)->hash == (fullhash) ) &&                                   \
           ( (run)->name->len == (var)->len ) &&                              \
           ( Str_ccmp( run->name, var ) == 0 ) )                              \
         break;                                                               \
                                                                              \
      (tbl)->c++;                                                             \
      COLLISION_CHECK(tbl, flag);                                             \
   }

#define SEEK_VAR_CNCMP(run,var,fullhash,tbl,l,flag) /* */                     \
   for ( ; (run); (run) = (run)->next )                                       \
   {                                                                          \
      if ( ( (run)->hash == (fullhash) ) &&                                   \
           ( (run)->name->len == (l) ) &&                                     \
           ( Str_cncmp( run->name, var, l ) == 0 ) )                          \
         break;                                                               \
                                                                              \
      (tbl)->c++;                                                             \
      COLLISION_CHECK(tbl, flag);                                             \
   }

#define SEEK_VAR_CNOCMP(run,var,fullhash,tbl,l,off,flag) /* */                \
   for ( ; (run); (run) = (run)->next )                                       \
   {                                                                          \
      if ( ( (run)->hash == (fullhash) ) &&                                   \
           ( (run)->name->len == (l) ) &&                                     \
           ( Str_cnocmp( run->name, var, l, off ) == 0 ) )                    \
         break;                                                               \
                                                                              \
      (tbl)->c++;                                                             \
      COLLISION_CHECK(tbl, flag);                                             \
   }

static const streng *getdirvalue_compound( tsd_t *TSD, var_hashtable *vars,
                                           const streng *name );

/*
 * Allocates and initializes a hashtable for the variables. Can be used
 * both for the main variable hash table, or for an compound variable.
 */
static var_hashtable *make_hash_table( const tsd_t *TSD, int size )
{
   var_hashtable *tab = (var_hashtable *)MallocTSD( sizeof( var_hashtable ) );

   tab->size = size;
   tab->r = 0;
   tab->w = 0;
   tab->c = 0;
   tab->e = 0;
   size = ( size + 1 ) * sizeof( variableptr );
   /* Last element needed to save current_valid */

   tab->tbl = (variableptr *)MallocTSD( size );
   memset( tab->tbl, 0, size );

   return tab;
}

/*
 * known_reserved_variable returns 0 if the arguments don't be a known
 * reserved variable as described in ANSI 6.2.3.1. The value is a
 * POOL0_??? value otherwise.
 *
 * The argument don't has to be uppercased but must not contain whitespaces or
 * garbage.
 */
int known_reserved_variable( const char *name, unsigned length )
{
   char upper[20]; /* good maximum for predefined variable names */

   /*
    * a fast breakout switch
    */
   if ( ( length < 3 /* ".RC" */)
     || ( length > 7 /* ".RESULT" */)
     || ( *name != '.' ) )
      return POOL0_NOT_RESERVED;
   name++;
   length--;

   memcpy( upper, name, length );
   mem_upper( upper, (int) length );

   /*
    * some magic stuff to reduce further errors
    */
#define RET_IF(s)  if ( ( length == sizeof( #s ) - 1 )                   \
                     && ( memcmp( upper, #s, sizeof( #s ) - 1 ) == 0 ) ) \
                      return POOL0_##s
   RET_IF( RC );
   RET_IF( RESULT );
   RET_IF( SIGL );
   RET_IF( RS );
   RET_IF( MN );
   RET_IF( LINE );
#undef REF_IF

   return POOL0_NOT_RESERVED;
}

void detach( const tsd_t *TSD, variableptr ptr )
{
#ifdef DEBUG
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;
#else
   TSD = TSD; /* keep compiler happy */
#endif

   assert( ptr->hwired > 0 );

   ptr->hwired--;
   DSTART;DPRINT((TSD,"detach:            "));DVAR(TSD,NULL,ptr);DEND;
}

#ifdef TRACEMEM
static void mark_ht( var_hashtable *tab )
{
   variableptr vvptr, vptr;
   unsigned i, j;

   if ( !tab )
      return;

   markmemory( (char*)tab->tbl, TRC_HASHTAB );
   for ( i = 0; i < tab->size; i++ )
   {
      for ( vptr = tab->tbl[i]; vptr; vptr = vptr->next )
      {
         markmemory( (char*)vptr, TRC_VARBOX );
         if ( vptr->name )
            markmemory( (char*)vptr->name, TRC_VARNAME );
         if ( vptr->num )
         {
            markmemory( vptr->num, TRC_VARVALUE );
            markmemory( vptr->num->num, TRC_VARVALUE );
         }
         if ( vptr->value )
            markmemory( (char*)vptr->value, TRC_VARVALUE );
         if ( vptr->index )
         {
            markmemory( vptr->index, TRC_VARNAME );
            for ( j = 0; j < vptr->index->size; j++ )
            {
               for ( vvptr = (vptr->index->tbl)[j]; vvptr; vvptr = vvptr->next )
               {
                  markmemory( (char*)vvptr, TRC_VARBOX );
                  if ( vvptr->name )
                     markmemory( (char*)vvptr->name, TRC_VARNAME );
                  if ( vvptr->num )
                  {
                      markmemory( vvptr->num, TRC_VARVALUE );
                      markmemory( vvptr->num->num, TRC_VARVALUE );
                  }
                  if ( vvptr->value )
                     markmemory( (char*)vvptr->value, TRC_VARVALUE );
               }
            }
         }
      }
   }

}

void markvariables( const tsd_t *TSD, cproclevel procptr )
{
   variableptr vptr;
   paramboxptr pptr;
   int i;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   for ( ; procptr; procptr = procptr->next )
   {
      if ( procptr->environment )
         markmemory( procptr->environment, TRC_VARBOX );
      if ( procptr->prev_env )
         markmemory( procptr->prev_env, TRC_VARBOX );
      if ( procptr->sig )
      {
         markmemory( procptr->sig, TRC_VARBOX );
         if ( procptr->sig->info )
            markmemory( procptr->sig->info, TRC_VARBOX );
         if ( procptr->sig->descr )
            markmemory( procptr->sig->descr, TRC_VARBOX );
      }
      if ( procptr->signal_continue )
         markmemory( procptr->signal_continue, TRC_VARBOX );
      if ( procptr->traps )
      {
         markmemory( procptr->traps, TRC_VARBOX );
         for ( i = 0; i < SIGNALS; i++ )
         {
            if ( procptr->traps[i].name )
               markmemory( procptr->traps[i].name, TRC_VARBOX );
         }
      }

      mark_ht( procptr->vars );
      markmemory( (char*)procptr,TRC_PROCBOX );

      markmemory( (char*)procptr->vars, TRC_HASHTAB );
      if ( procptr->args )
      {
         for ( pptr = procptr->args; pptr; pptr = pptr->next )
         {
            markmemory( (char*) pptr, TRC_PROCARG );
            if ( pptr->value )
               markmemory( (char*) pptr->value, TRC_PROCARG );
         }
      }
   }

   mark_ht( vt->pool0 );
   markmemory( (char*)vt->pool0, TRC_HASHTAB );

   for ( vptr = vt->first_invalid; vptr; vptr = vptr->prev )
   {
      markmemory( vptr, TRC_VARBOX );
   }
}
#endif /* TRACEMEM */

static variableptr newbox( const tsd_t *TSD, const streng *name,
                           streng *value, variableptr *oldptr, unsigned hash)
{
   variableptr newptr;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   DSTART;DPRINT((TSD,"newbox:            "));DNAME(TSD,NULL,name);DPRINT((TSD," replaces "));
          DVAR(TSD,NULL,*oldptr);DEND;
   newptr = (variableptr)MallocTSD( sizeof( variable ) );

   newptr->index = NULL;
   newptr->next = *oldptr;
   newptr->prev = NULL;
   newptr->realbox = NULL;
   if ( name )
      newptr->name = Str_dupTSD( name );
   else
      newptr->name = NULL;
   newptr->value = value;
   newptr->guard = 0;
   newptr->num = NULL;
   newptr->flag = value ? VFLAG_STR : VFLAG_NONE;
   newptr->hash = hash;
   newptr->hwired = 0;
   newptr->valid = (long) vt->current_valid;
   newptr->stem = NULL;

   *oldptr = newptr;
   DSTART;DPRINT((TSD,"newbox:            "));DVAR(TSD,"rc",newptr);DEND;
   return newptr;
}

static variableptr make_stem( const tsd_t *TSD, const streng *name,
                              streng *value, variableptr *oldptr, int len,
                              unsigned hash )
{
   variableptr ptr;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   ptr = newbox( TSD, NULL, value, oldptr, hash );
   ptr->index = make_hash_table( TSD, vt->initialHashTableLength );
   DPRINTF((TSD,"make_hash_table:   rc=%p",ptr->index));
   ptr->name = Str_ndupTSD( name, len );
   DSTART;DPRINT((TSD,"makestem:          "));DVAR(TSD,"rc",ptr);DEND;
   return ptr;
}

#define RXDIGIT 0x01
#define RXUPPER 0x02
#define RXLOWER 0x04
#define RXEXTRA 0x08
#define RXDOT   0x10
#define RXWHITE 0x20
#define RXVAR   ( RXUPPER | RXLOWER | RXEXTRA )
#define RXCONST ( RXUPPER | RXLOWER | RXEXTRA | RXDIGIT | RXDOT )
#define RXMANT  ( RXDIGIT | RXDOT )
#define CHAR_TYPE(c) char_types[(unsigned char) ( c )]
#define RXISDIGIT(a) ( CHAR_TYPE( a ) & RXDIGIT )
#define RXISUPPER(a) ( CHAR_TYPE( a ) & RXUPPER )
#define RXISLOWER(a) ( CHAR_TYPE( a ) & RXLOWER )
#define RXISEXTRA(a) ( CHAR_TYPE( a ) & RXEXTRA )
#define RXISDOT(a)   ( CHAR_TYPE( a ) & RXDOT   )
#define RXISWHITE(a) ( CHAR_TYPE( a ) & RXWHITE )
#define RXISVAR(a)   ( CHAR_TYPE( a ) & RXVAR   )
#define RXISCONST(a) ( CHAR_TYPE( a ) & RXCONST )
#define RXISMANT(a)  ( CHAR_TYPE( a ) & RXMANT )

/*
 * 04.10.2003, FGC, FIXME: Is it allowed to move to the new scheme of
 * character manipulation as defined in misc.c? I don't think so, because
 * the lexer isn't/can't be changed to support CHANGING extended character
 * sets. If you have a good idea, please let me know.
 */
static const unsigned char char_types[256] =
{
   0,       0,       0,       0,       0,       0,       0,       0,         /* nul - bel */
   0,       RXWHITE, RXWHITE, 0,       RXWHITE, 0,       0,       RXWHITE,   /* bs  - si  */
   0,       0,       0,       0,       0,       0,       0,       0,         /* dle - etb */
   0,       0,       0,       0,       0,       0,       0,       0,         /* can - us  */
   RXWHITE, RXEXTRA, 0,       RXEXTRA, RXEXTRA, 0,       0,       0,         /* sp  -  '  */
   0,       0,       0,       0,       0,       0,       RXDOT,   0,         /*  (  -  /  */
   RXDIGIT, RXDIGIT, RXDIGIT, RXDIGIT, RXDIGIT, RXDIGIT, RXDIGIT, RXDIGIT,   /*  0  -  7  */
   RXDIGIT, RXDIGIT, 0,       0,       0,       0,       0,       RXEXTRA,   /*  8  -  ?  */
   RXEXTRA, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER,   /*  @  -  G  */
   RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER,   /*  H  -  O  */
   RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER, RXUPPER,   /*  P  -  W  */
   RXUPPER, RXUPPER, RXUPPER, 0,       0,       0,       0,       RXEXTRA,   /*  X  -  _  */
   0,       RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER,   /*  `  -  g  */
   RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER,   /*  h  -  o  */
   RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER, RXLOWER,   /*  p  -  w  */
   RXLOWER, RXLOWER, RXLOWER, 0,       0,       0,       0,       0,         /*  x  - del */
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0,
   0,       0,       0,       0,       0,       0,       0,       0
} ;

/*
 * valid_var_symbol matches a symbol, which can be either a number, a
 * var_symbol, or a const_symbol in terms of ANSI 6.3.2.96.
 * see also: ANSI 6.2.2.29, 6.2.2.31, 6.2.2.35
 * Note that 6.2.2.32 means to parse an exponential character only if it
 * it part of a number. Thus, we have to parse the following:
 * var_symbol=RXVAR RXCONST*
 * const_symbol=RXCONST+
 * number=( '.' RXDIGIT+ | ( RXDIGIT+ ( '.' RXDIGIT* )? ) ) exp?
 * exp=( 'e' | 'E' ) ( '-' | '+' )? RXDIGIT+
 * We split var_symbol into simple, stem, compound and return all
 * SYMBOL_??? values.
 */
int valid_var_symbol( const streng *name )
{
   const unsigned char *cptr,*eptr;
   unsigned char ch;
   int dots,sign;

   if ( name->len == 0 )
      return SYMBOL_BAD;

   cptr = (const unsigned char *) name->value;
   eptr = cptr + name->len;

   ch = *cptr++;
   if ( RXISVAR( ch ) )
   {
      /*
       * valid begin of a variable name. Lets check it. This can't be
       * a number, thus we don't care about sign characters.
       */
      dots = 0;
      do {
         if ( cptr == eptr )
         {
            /*
             * the last character is properly loaded in ch
             */
            if ( RXISDOT( ch ) && ( dots == 1 ) )
               return SYMBOL_STEM;
            else if ( dots )
               return SYMBOL_COMPOUND;
            return SYMBOL_SIMPLE;
         }
         ch = *cptr++;
         if ( RXISDOT( ch ) )
            dots = 1;
         /*
          * characters after the first character are the same as of
          * const_symbol. They don't share the sign behaviour, though.
          */
      } while ( RXISCONST( ch ) );

      return SYMBOL_BAD;
   }

   /*
    * Check for a reserved variable first.
    * In all other cases we may have a number or a const_symbol.
    */
   if ( KNOWN_RESERVED( (char *)cptr - 1, name->len ) )
      return SYMBOL_SIMPLE;

   sign = 0;
   /*
    * Breaking/ending the following loops means to check for a const_symbol
    * with respect to the absense of a sign character.
    */
   for ( ; ; ) {
      /*
       * A number is a const_symbol with the exception of the sign within
       * an exponent. Try parsing a number first and fall back to a const
       * detection on error but keep a seen sign in mind.
       */
      if ( RXISDOT( ch ) )
      {
         /*
          * Check for a plain dot, which isn't a number, and for a following
          * digit.
          */
         if ( cptr == eptr )
            return SYMBOL_CONSTANT;
         ch = *cptr++;
         if ( !RXISDIGIT( ch ) )
            break;

         do {
            if ( cptr == eptr )
               return SYMBOL_NUMBER;

            ch = *cptr++;
            /*
             * expect just RXDIGITs as the mantissa.
             */
         } while ( RXISDIGIT( ch ) );
      }
      else
      {
         if ( !RXISDIGIT( ch ) )
            break;
         /*
          * expect RXDIGITs [ '.' RXDIGITs ] as the mantissa.
          */

         do {
            if ( cptr == eptr )
               return SYMBOL_NUMBER;

            ch = *cptr++;
         } while ( RXISDIGIT( ch ) );
         if ( RXISDOT( ch ) )
         {
            if ( cptr == eptr )
               return SYMBOL_NUMBER;

            ch = *cptr++;
            if ( RXISDIGIT( ch ) )
            {
               do {
                  if ( cptr == eptr )
                     return SYMBOL_NUMBER;

                  ch = *cptr++;
               } while ( RXISDIGIT( ch ) );
            }
         }
      }

      /*
       * We have to parse an exponent. ch has the current character.
       */
      if ( ( ch != 'e' ) && ( ch != 'E' ) )
         break;

      if ( cptr == eptr )
         return SYMBOL_CONSTANT;

      ch = *cptr++;
      if ( ( ch == '+' ) || ( ch == '-' ) )
      {
         sign = 1;
         if ( cptr == eptr )
            return SYMBOL_BAD; /* something like "1.2E+" */
         ch = *cptr++;
      }
      /*
       * parse the exponent value
       */
      if ( !RXISDIGIT( ch ) )
         break;
      do {
         if ( cptr == eptr )
            return SYMBOL_NUMBER;

         ch = *cptr++;
      } while ( RXISDIGIT( ch ) );

      break;
   }

   /*
    * We have to check for a const_symbol. If a sign has occured until now
    * we have a bad symbol.
    * ch is loaded with the current character.
    */
   if ( sign || !RXISCONST( ch ) )
      return SYMBOL_BAD;

   do {
      if ( cptr == eptr )
         return SYMBOL_CONSTANT;

      ch = *cptr++;
   } while ( RXISCONST( ch ) );

   /*
    * garbage in ch
    */
   return SYMBOL_BAD;
}

/*
 * Note: This is one of the most time-consuming routines. Be careful.
 * It is a define to remove one time-consuming function call.
 */
#define hashfunc(vt,name,start,stop,mod) /**/ \
      (vt->hashval = ( (vt->fullhash = hashvalue_var( name, start, stop )) % \
                       (mod) ) )

/*
 * create a standard sized variable pool in a hash table.
 */
var_hashtable *create_new_varpool( const tsd_t *TSD, int size )
{
   var_tsd_t *vt;
   var_hashtable *retval;

   vt = (var_tsd_t *)TSD->var_tsd;
   if (size <= 10)
   {
      size = vt->initialHashTableLength;
   }
   retval = make_hash_table( TSD, size );

   DPRINTF((TSD,"make_hash_table:   rc=%p", retval));
   DPRINTF((TSD,"create_new_varpool:current_valid:new=%d, old=%d",
             vt->next_current_valid,vt->current_valid));
   retval->tbl[size] = (variableptr) vt->current_valid;
   vt->current_valid = vt->next_current_valid++;

   return retval;
}

void set_ignore_novalue( const tsd_t *TSD )
{
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   vt->ignore_novalue = 1;
   DPRINTF((TSD,"set_ignore_novalue"));
}

void clear_ignore_novalue( const tsd_t *TSD )
{
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   vt->ignore_novalue = 0;
   DPRINTF((TSD,"clear_ignore_novalue"));
}

/*
 * variables_per_SAA changes the variable pool's interface so that an access by
 * the SAA API works in the defined way, that is without tracing and without
 * signalling a NOVALUE condition.
 * The returned value must be used to feed restore_variable_state() after the
 * SAA access.
 */
int variables_per_SAA( tsd_t *TSD )
{
   int retval;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   retval = vt->notrace ? 2 : 0;
   retval |= vt->ignore_novalue ? 1 : 0;
   vt->notrace = 1;
   vt->ignore_novalue = 1;
   DPRINTF((TSD,"variables_per_SAA"));
   return retval;
}

/*
 * restore_variable_state restores the state that was before the call to
 * variables_per_SAA(). Look there.
 */
void restore_variable_state( const tsd_t *TSD, int state )
{
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   vt->notrace = ( state >> 1 ) & 1;
   vt->ignore_novalue = state & 1;
   DPRINTF((TSD,"restore_variable_state"));
}

const streng *get_it_anyway( tsd_t *TSD, const streng *str, int pool )
{
   const streng *ptr;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   vt->notrace = 1;
   vt->ignore_novalue = 1;
   ptr = getvalue( TSD, str, pool ); /* changes the pool */
   vt->ignore_novalue = 0;
   vt->notrace = 0;

   if ( !ptr )
      exiterror( ERR_SYMBOL_EXPECTED, 1, tmpstr_of( TSD, str ) );

   DSTART;DPRINT((TSD,"get_it_anyway:     "));DNAME(TSD,"str",str);DVALUE(TSD,", rc",ptr);DEND;
   return ptr;
}

const streng *get_it_anyway_compound( tsd_t *TSD, const streng *str )
/* as get_it_anyway but specific to getdirvalue_compound */
{
   const streng *ptr;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   vt->notrace = 1;
   vt->ignore_novalue = 1;
   ptr = getdirvalue_compound( TSD, TSD->currlevel->vars, str );
   vt->ignore_novalue = 0;
   vt->notrace = 0;

   if ( !ptr )
      exiterror( ERR_SYMBOL_EXPECTED, 1, tmpstr_of( TSD, str ) );

   DSTART;DPRINT((TSD,"get_it_anyway_compound:"));DNAME(TSD,"str",str);DVALUE(TSD,", rc",ptr);DEND;
   return ptr;
}

int var_was_found( const tsd_t *TSD )
{
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   DPRINTF((TSD,"var_was_found:     rc=%d",vt->foundflag));
   return vt->foundflag;
}

const streng *isvariable( tsd_t *TSD, const streng *str )
{
   const streng *ptr;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   vt->ignore_novalue = 1 ;
   ptr = getvalue( TSD, str, -1 ); /* changes the pool */
   vt->ignore_novalue = 0;
   DSTART;DPRINT((TSD,"isvariable:        "));DNAME(TSD,"str",str);
          DVALUE(TSD,", rc",(vt->foundflag)?ptr:NULL);DEND;
   if ( vt->foundflag )
      return ptr;

   return NULL;
}

#ifdef TRACEMEM
static void mark_variables( const tsd_t *TSD )
{
   unsigned i,j;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   markmemory( vt->tmpindex, TRC_STATIC );
   if ( vt->ovalue )
      markmemory( vt->ovalue, TRC_STATIC );
   if ( vt->xvalue )
      markmemory( vt->xvalue, TRC_STATIC );
   if ( vt->odescr )
   {
      markmemory( vt->odescr, TRC_STATIC );
      markmemory( vt->odescr->num, TRC_STATIC );
   }
   for ( i = 0; i < POOL0_CNT; i++ )
   {
      for ( j = 0; j < 2; j++ )
      {
         if ( vt->pool0nodes[i][j].name != NULL )
         {
            markmemory( vt->pool0nodes[i][j].name, TRC_SPCV_NAME );

         }
      }
   }
}
#endif

/* init_vars initializes the module.
 * Currently, we set up the thread specific data and check for environment
 * variables to change debugging behaviour.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_vars( tsd_t *TSD )
{
   var_tsd_t *vt;
   int i, j;

   if ( TSD->var_tsd != NULL )
      return 1;

   if ( ( TSD->var_tsd = MallocTSD( sizeof( var_tsd_t ) ) ) == NULL )
      return 0;
   vt = (var_tsd_t *)TSD->var_tsd;
   memset( vt, 0, sizeof( var_tsd_t ) );
   vt->initialHashTableLength = 71;

#ifdef DEBUG
   {
      char junk[100];
      if ( mygetenv( TSD, "DEBUG_VARIABLE", junk, sizeof( junk ) ) != NULL )
      {
         vt->DoDebug = 1;
         if (rx_isdigit(junk[0]) && ((junk[1] == ',') || (junk[1] == '\0'))) {
            vt->DoDebug = junk[0] - '0';
         }
         vt->DebugFile = stderr;
         if (junk[0] && (junk[1] == ',')) {
            if ((vt->DebugFile = fopen(junk + 2, "ab")) == NULL)
            {
               vt->DebugFile = stderr;
            }
         }
      }
      if ( mygetenv( TSD, "DEBUG_HASHTABLENGTH", junk, sizeof( junk ) ) != NULL )
      {
         int v;
         char c;

         if (sscanf(junk, "%d %c", &v, &c) == 1)
         {
            if ((v >= 4) && (v <= 10000))
            {
               vt->initialHashTableLength = v;
            }
         }
      }
   }
#endif

# ifdef TRACEMEM
   regmarker( TSD, mark_variables );
# endif
   vt->current_valid = 1;
   vt->next_current_valid = 2;
   vt->tmpindex = Str_makeTSD( MAX_INDEX_LENGTH );
   /*
    * pool 0 is not used by the user in most cases. Keep it small.
    */
   vt->pool0 = create_new_varpool( TSD, 17 );

   /*
    * .RC, .RESULT, and .SIGL have dotless counterparts.
    */
   vt->pool0nodes[POOL0_RC][0].name     = Str_creTSD( ".RC" );
   vt->pool0nodes[POOL0_RC][1].name     = Str_creTSD(  "RC" );
   vt->pool0nodes[POOL0_RESULT][0].name = Str_creTSD( ".RESULT" );
   vt->pool0nodes[POOL0_RESULT][1].name = Str_creTSD(  "RESULT" );
   vt->pool0nodes[POOL0_SIGL][0].name   = Str_creTSD( ".SIGL" );
   vt->pool0nodes[POOL0_SIGL][1].name   = Str_creTSD(  "SIGL" );
   vt->pool0nodes[POOL0_RS][0].name     = Str_creTSD( ".RS" );
   vt->pool0nodes[POOL0_MN][0].name     = Str_creTSD( ".MN" );
   vt->pool0nodes[POOL0_LINE][0].name   = Str_creTSD( ".LINE" );
   for ( i = 0; i < POOL0_CNT; i++ )
   {
      for ( j = 0; j < 2; j++ )
      {
         if ( vt->pool0nodes[i][j].name != NULL )
            vt->pool0nodes[i][j].type = X_SIM_SYMBOL;
      }
   }

   DPRINTF((TSD,"init_vars"));
   return(1);
}

/*
 * This routine takes a ptr to a linked list of nodes, each describing
 * one element in a tail of a compound variable. Each of the elements
 * will eventually be cached, since they are retrieved through the
 * shortcut() routine.
 */
static streng *fix_index( tsd_t *TSD, nodeptr thisptr )
{
   char *cptr;
   const streng *value;
   int osetting;
   int freespc;
   streng *large;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   assert( thisptr ) ;
   osetting = vt->ignore_novalue ;
   vt->ignore_novalue = 1 ;

   DPRINTF((TSD,"fix_index, start:  thisptr=%p",thisptr));
   freespc = vt->tmpindex->max ;
   cptr = vt->tmpindex->value ;

   for ( ; ; )
   {
      assert( thisptr->type==X_CTAIL_SYMBOL || thisptr->type==X_VTAIL_SYMBOL) ;
      if ( thisptr->type == X_CTAIL_SYMBOL )
         value = thisptr->name ;
      else
      {
         vt->subst = 1 ;
         value = shortcut( TSD, thisptr ) ;
      }

      freespc -= value->len;
      if ( freespc-- <= 0 )
      {
         large = Str_makeTSD( vt->tmpindex->max * 2 + value->len ) ;
         memcpy( large->value, vt->tmpindex->value, (cptr-vt->tmpindex->value)) ;
         cptr = large->value + (cptr-vt->tmpindex->value) ;
         freespc += (large->max - vt->tmpindex->max) ;
         Free_stringTSD( vt->tmpindex ) ;
         vt->tmpindex = large ;

         assert( freespc >= 0 ) ;
      }

      memcpy( cptr, value->value, value->len ) ;
      cptr += value->len ;
      thisptr = thisptr->p[0] ;
      if ( thisptr )
         *(cptr++) = '.' ;
      else
         break ;
   }
   vt->tmpindex->len = cptr - vt->tmpindex->value ;
   assert( vt->tmpindex->len <= vt->tmpindex->max ) ;
   vt->ignore_novalue = osetting ;
   DSTART;DPRINT((TSD,"fix_index, end:    thisptr=%p, "));DVALUE(TSD,"rc",vt->tmpindex);DEND;
   return vt->tmpindex ;
}

void expand_to_str( const tsd_t *TSD, variableptr ptr )
{
   int flag;
#ifdef DEBUG
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;
#endif

   flag = ptr->flag;

   DSTART;DPRINT((TSD,"expand_to_str:     "));DVAR(TSD,"ptr",ptr);DEND;
   if ( flag & VFLAG_STR )
      return;

   if ( flag & VFLAG_NUM )
   {
      assert( ptr->num );
      ptr->value = str_norm( TSD, ptr->num, ptr->value );
      ptr->flag |= VFLAG_STR;
   }
   DSTART;DPRINT((TSD,"expand_to_str:     "));DVAR(TSD,"ptr",ptr);DEND;
}

static streng *subst_index( const tsd_t *TSD, const streng *name, int start,
                            var_hashtable *vars, int *expand )
{
   int i=0, length=0 ;
   variableptr nptr;
   int stop;
   char *cptr=NULL ;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   assert( start < name->len ) ;

   DPRINTF((TSD,"subst_index:       ?"));
   vt->tmpindex->len = 0;
   vt->subst = 0;

   for ( ; ; )
   {
      nptr = vars->tbl[hashfunc( vt, name, start, &stop, vars->size )];
      vars->r++;

      length = stop - start;
      SEEK_VAR_CNOCMP( nptr, name, vt->fullhash, vars, length, start, *expand );
      SEEK_EXPOSED( nptr );

      if ( nptr )
         expand_to_str( TSD, nptr );

      if ( nptr && nptr->value )
      {
         Str_catTSD( vt->tmpindex, nptr->value );
         vt->subst = 1;
      }
      else
      {
         cptr = vt->tmpindex->value + vt->tmpindex->len;
         for ( i = start; i < stop; i++ )
            *cptr++ = (char) rx_toupper( name->value[i] );
         vt->tmpindex->len = cptr - vt->tmpindex->value;
      }

      if ( stop >= Str_len( name ) )
         break ;

      start = stop + 1;
      vt->tmpindex->value[vt->tmpindex->len++] = '.';
   }

   return vt->tmpindex;
}

/*
 * TRACEMEM_RELINK relinks a variablebox in the queue of vt->first_invalid.
 */
#ifdef TRACEMEM
# define TRACEMEM_RELINK(ptr) {                                               \
    ptr->prev = vt->first_invalid;                                            \
    ptr->next = NULL;                                                         \
    if ( vt->first_invalid )                                                  \
       vt->first_invalid->next = ptr;                                         \
    vt->first_invalid = ptr;                                                  \
 }
#else
# define TRACEMEM_RELINK(ptr)
#endif

/*
 * REMOVE_ELEMENT deletes the content of a variableptr.
 * The variable itself is NOT freed in most cases. The hollow body of the
 * variable persists. As far as I understood my old stuff, this prevents a
 * new variable lookup.
 */
#define REMOVE_ELEMENT(ptr, hashtbl) {                                        \
    DSTART;DPRINT((TSD,"                   "));DVAR(TSD,"ptr(del)",ptr);DEND; \
    Free_stringTSD( ptr->name );                                              \
    if ( ptr->value )                                                         \
       Free_stringTSD( ptr->value );                                          \
                                                                              \
    if ( ptr->num )                                                           \
    {                                                                         \
      FreeTSD( ptr->num->num );                                               \
      FreeTSD( ptr->num );                                                    \
    }                                                                         \
                                                                              \
    if ( ptr->hwired )                                                        \
    {                                                                         \
       ptr->valid = 0;                                                        \
       TRACEMEM_RELINK( ptr );                                                \
    }                                                                         \
    else                                                                      \
    {                                                                         \
       FreeTSD( ptr );                                                        \
       (hashtbl)->e--;                                                        \
    }                                                                         \
 }

/*
 * remove_foliage removes all elements of a stem unconditionally.
 * Exposed values, better the values and variables the current elements points
 * to, are not affected.
 * The stem's index itself is freed, too.
 */
static void remove_foliage( const tsd_t *TSD, var_hashtable *index )
{
#if defined(DEBUG) || defined(TRACEMEM)
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;
   unsigned start = index->e;
#endif
   unsigned i;
   variableptr ptr, tptr;

   DPRINTF((TSD,"remove_foliage:    ?"));

   assert(index);

   for ( i = 0; i < index->size; i++ )
   {
      if ( ( ptr = index->tbl[i] ) == NULL )
         continue;

      do {
         /*
          * Not needed here but it indicates an error elsewhere:
          */
         /*assert( ptr->stem );*/

         tptr = ptr->next;

         if ( ptr->index )
         {
            /*
             * This indicates a serious problem. Regina doesn't allow branches
             * currently. (branch = stem within a stem)
             */
            assert( ptr->index );
            remove_foliage( TSD, ptr->index );
         }

         REMOVE_ELEMENT( ptr, index );

         ptr = tptr;
      } while ( ptr != NULL );
   }
   FreeTSD( index->tbl );
   DPRINTF((TSD,"                   kill=%p",index));
   DPRINTF_2((TSD,"STATISTICS:        %u(%u) in %u buckets, %u r, %u w, %u colls",
                          index->e, start, index->size, index->r,
                          index->w, index->c));
   FreeTSD( index );
}

/*
 * assign_foliage manipulates the index of a stem. The index is the list of all
 * variables in the stem.
 * value is the value for the replacement and is read-only. The value may be
 * NULL.
 * Each element in the index is either removed or reassigned depending on the
 * fact whether the element is local or exposed. A value of NULL leads to a
 * complete removal of the element which is equivalent to a "drop".
 * This fixes bug 732146, which removed all elements unconditionally after
 * an assignment.
 * ANSI 7.1.2 and 7.1.4 forces us to assign all exposed elements or to delete
 * all exposed elements. Note that a dropped and exposed variable should be
 * assigned in the formerly used variable pool (aka index), so we use a
 * technique which allows us to lookup such values (we don't delete the realbox
 * chain). On the other hand we really remove local variables to save space.
 */
static void assign_foliage( const tsd_t *TSD, var_hashtable *index,
                            const streng *val )
{
#if defined(DEBUG) || defined(TRACEMEM)
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;
#endif
   unsigned i;
   streng *copy;
   variableptr ptr, tptr, *queue, real;
   streng *nullptr = NULL; /* allow REPLACE_VALUE macro to pass NULL value under C++ */

   DPRINTF((TSD,"assign_foliage:    ?"));

   assert(index);

   for ( i = 0; i < index->size; i++ )
   {
      if ( ( ptr = index->tbl[i] ) == NULL )
         continue;

      queue = &index->tbl[i];

      do {
         /*
          * Not needed here but it indicates an error elsewhere:
          */
         assert( ptr->stem );

         tptr = ptr->next;

         /*
          * We can't handle sub-stems.
          */
         assert( ptr->index == NULL );

         if ( ptr->realbox )
         {
            real = ptr;
            SEEK_EXPOSED(real);
            if ( val )
            {
               copy = Str_dupTSD( val );
               REPLACE_VALUE( copy, real );
               DSTART;DPRINT((TSD,"                   "));DVAR(TSD,"real(now)",real);DEND;
            }
            else if ( real->value )
            {
               REPLACE_VALUE( nullptr, real );
               DSTART;DPRINT((TSD,"                   "));DVAR(TSD,"real(now)",real);DEND;
            }

            *queue = ptr;
            queue = &(ptr->next);
         }
         else
         {
            if ( val == NULL )
            {
               REMOVE_ELEMENT( ptr, index );
            }
            else
            {
               copy = Str_dupTSD( val );
               REPLACE_VALUE( copy, ptr );
               DSTART;DPRINT((TSD,"                   "));DVAR(TSD,"ptr(now)",ptr);DEND;
               *queue = ptr;
               queue = &(ptr->next);
            }
         }

         ptr = tptr;
      } while ( ptr != NULL );

      *queue = NULL;
   }
}
#undef REMOVE_ELEMENT
#undef TRACEMEM_RELINK

/*
 * reorgHashtable reorganises the hash table vars if a potential for better
 * results in the future is seen.
 * returns 0 if the table has its original structure.
 * returns 1 if the table has been reorganised.
 */
static int reorgHashtable( const tsd_t *TSD, var_hashtable *vars )
{
   int f1 = 0, f2 = 0;
   unsigned newSize, i;
   variableptr *newTbl, *dest;
   variableptr thisptr, run;
#ifdef DEBUG
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;
#endif

   NEW_HASHTABLE_CHECK( vars, f1 );
   COLLISION_CHECK( vars, f2 );
   if ( !(f1 | f2 ) )
   {
      /*
       * May happen in case of f2 where a ->r or ->w is set after the
       * collision detection. Ignore it. That is no bug.
       */
      DPRINTF_2((TSD,"reorgHashtable:    %p/%p: BLIND ALERT", vars, vars->tbl));
      return 0;
   }

   /*
    * Try to use a low water mark of 33% capacity. If the collisions are
    * the reason, go below 25% capacity.
    */
   newSize = vars->e * ( ( f2 ) ? 4 : 3 );
   if ( vars->size >= newSize )
   {
      /*
       * We can't help. Check the stupid hashfunc() in this case.
       */
      DPRINTF_2((TSD,"reorgHashtable:    %p/%p: heavy collisions %u",
                     vars, vars->tbl, vars->c));
      vars->r = 0;
      vars->w = 0;
      vars->c = 0;
      return 0;
   }

   newTbl = (variableptr *)MallocTSD( sizeof(variableptr) * newSize );
   memset( newTbl, 0, sizeof(variableptr) * newSize );
   DPRINTF((TSD,"reorgHashtable:    %p -> %p (%u)",
                vars->tbl,newTbl,newSize));
   DPRINTF_2((TSD,"reorgHashtable:    changing size of %p/%p (%u) to %p/%p (%u), c=%u(%d)",
                  vars,vars->tbl,vars->size,vars,newTbl,newSize, vars->c,f2));

   for ( i = 0; i < vars->size; i++ )
   {
      run = vars->tbl[i];

      while ( run )
      {
         thisptr = run;
         run = run->next;

         dest = newTbl + (thisptr->hash % newSize);
         if (*dest)
         {
            (*dest)->prev = thisptr;
         }
         thisptr->next = *dest;
         *dest = thisptr;
         thisptr->prev = NULL;
      }
   }

   FreeTSD( vars->tbl );
   vars->tbl = newTbl;
   vars->size = newSize;
   vars->r = 0;
   vars->w = 0;
   vars->c = 0;
   return 1;
}

static variableptr findsimple( const tsd_t *TSD, var_hashtable *vars,
                               const streng *name, int *expand )
{
   variableptr ptr;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   ptr = vars->tbl[hashfunc( vt, name, 0, NULL, vars->size)];
   SEEK_VAR_CCMP( ptr, name, vt->fullhash, vars, *expand );
   SEEK_EXPOSED( ptr );

   vt->thespot = ptr;
   DSTART;DPRINT((TSD,"findsimple:        "));DNAME(TSD,"name",name);
          DVAR(TSD,", vt->thespot=ptr",ptr);DEND;

   return ptr;
}

static int setvalue_simple( const tsd_t *TSD, var_hashtable *vars,
                            const streng *name, streng *value )
{
   variableptr ptr;
   int rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   vars->w++;
   ptr = findsimple( TSD, vars, name, &rehash );
   if ( ptr )
   {
      vt->foundflag = ptr->flag & VFLAG_BOTH;
      REPLACE_VALUE( value, ptr );
      DSTART;DPRINT((TSD,"setvalue_simple:   "));DVAR(TSD,"replacement",ptr);DEND;
   }
   else
   {
      vt->foundflag = 0;
      vt->thespot = newbox( TSD, name, value, &vars->tbl[vt->hashval],
                            vt->fullhash );
      vars->e++;
      NEW_HASHTABLE_CHECK( vars, rehash );
      DSTART;DPRINT((TSD,"setvalue_simple:   "));DVAR(TSD,"new, vt->thespot",ptr);DEND;
   }
   if ( rehash )
   {
      return reorgHashtable( TSD, vars );
   }
   return 0;
}

static const streng *getvalue_simple( tsd_t *TSD, var_hashtable *vars,
                                      const streng *name )
{
   variableptr ptr;
   const streng *value;
   int rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   vars->r++;
   ptr = findsimple( TSD, vars, name, &rehash );
   if ( rehash )
   {
      reorgHashtable( TSD, vars );
   }

   vt->foundflag =  ptr && ( ptr->flag & VFLAG_BOTH );

   if ( ptr )
      expand_to_str( TSD, ptr );

   if ( vt->foundflag )
      value = ptr->value;
   else
   {
      value = name;
      vt->thespot = NULL;
      if ( !vt->ignore_novalue )
         condition_hook( TSD, SIGNAL_NOVALUE, 0, 0, -1, Str_dupTSD( value ), NULL );
   }

   if ( !vt->notrace )
      tracevalue( TSD, value,(char) ( (ptr) ? 'V' : 'L' ) );

   DSTART;DPRINT((TSD,"getvalue_simple:   "));DNAME(TSD,"name",name);
          DVALUE(TSD," rc",value);DEND;
   return value;
}

static int setvalue_stem( const tsd_t *TSD, var_hashtable *vars,
                          const streng *name, streng *value )
{
   variableptr ptr;
   int rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"setvalue_stem:     ?"));

   vars->w++;
   ptr = findsimple( TSD, vars, name, &rehash );

   if ( ptr )
   {
      vt->foundflag = ( ptr->flag & VFLAG_BOTH );
      REPLACE_VALUE( value, ptr );
      if ( ptr->index )
         assign_foliage( TSD, ptr->index, value );
   }
   else
   {
      vt->foundflag = 0;
      make_stem( TSD, name, value, &vars->tbl[vt->hashval], name->len,
                 vt->fullhash );
      vars->e++;
      NEW_HASHTABLE_CHECK( vars, rehash );
   }
   vt->thespot = NULL;
   if ( rehash )
   {
      return reorgHashtable( TSD, vars );
   }
   return 0;
}

static int setvalue_compound( tsd_t *TSD, var_hashtable *vars,
                              const streng *name, streng *value )
{
   variableptr ptr, nptr, *nnptr, *pptr;
   int stop, rehash = 0, rehashIdx = 0;
   streng *indexstr;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"setvalue_compound: ?"));
   vt->foundflag = 0 ;
   pptr = &vars->tbl[hashfunc( vt, name, 0, &stop, vars->size )];
   stop++;
   ptr = *pptr;
   SEEK_VAR_CNCMP( ptr, name, vt->fullhash, vars, stop, rehash );
   SEEK_EXPOSED( ptr );

   if ( !ptr )
   {
      ptr = make_stem( TSD, name, NULL, pptr, stop, vt->fullhash );
      vars->e++;
      NEW_HASHTABLE_CHECK( vars, rehash );
      vars->w++;
   }
   else
   {
      vars->r++;
   }

   indexstr = subst_index( TSD, name, stop, vars, &rehash );

   if ( rehash )
   {
      rehash = reorgHashtable( TSD, vars );
   }

   if ( vt->subst )   /* trace it */
      tracecompound( TSD, name, stop - 1, indexstr, 'C' );

   vars->w++;
   nnptr = &(ptr->index->tbl[hashfunc( vt, indexstr, 0, NULL, ptr->index->size )]);
   nptr = *nnptr;
   SEEK_VAR_CMP( nptr, indexstr, vt->fullhash, ptr->index, rehashIdx );
   SEEK_EXPOSED( nptr );

   if ( nptr )
   {
      vt->foundflag = nptr && ( nptr->flag & VFLAG_BOTH ) ;
      REPLACE_VALUE( value, nptr );
   }
   else
   {
      newbox( TSD, indexstr, value, nnptr, vt->fullhash );
      ptr->index->e++;
      NEW_HASHTABLE_CHECK( ptr->index, rehashIdx );
      (*nnptr)->stem = ptr;
   }

   vt->thespot = NULL;
   if ( rehashIdx )
   {
      rehashIdx = reorgHashtable( TSD, ptr->index );
   }
   return rehash | rehashIdx;
}

/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */
/****************************************************************************
 *
 *  JH 13/12/1999 (Original code changes on 20/10/1999)
 *
 *  BUG022            To make Direct setting of stems Direct and not Symbolic.
 *   - Adapted from setvalue_compound().
 *   - Started using the global variable, vt->tmpindex, in place of the local,
 *     indexstr.
 *   - manually move the first stem name into vt->tmpindex, do not call
 *     subst_index(), as that not only uppercases the tail, but also
 *     does not uppercase the tail.
 *
 *
 ****************************************************************************/
static int setdirvalue_compound( tsd_t *TSD, var_hashtable *vars,
                                 const streng *name, streng *value )
{
   variableptr ptr, nptr, *nnptr, *pptr;
   int rehash = 0, rehashIdx = 0;
   int stop;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"setdirvalue_compound: ?"));
   vt->foundflag = 0;

   /*
    * Get a good starting point, and find the stem/index separater.
    */
   pptr = &vars->tbl[hashfunc( vt, name, 0, &stop, vars->size )];
   stop++;

   /*
    * Find the stem in the variable pool.
    */
   ptr = *pptr;
   SEEK_VAR_CNCMP( ptr, name, vt->fullhash, vars, stop, rehash );
   SEEK_EXPOSED( ptr );

   /*
    * If the stem does not exist, make one.
    */
   if ( !ptr )
   {
      ptr = make_stem( TSD, name, NULL, pptr, stop, vt->fullhash );
      vars->e++;
      NEW_HASHTABLE_CHECK( vars, rehash );
      vars->w++;
   }
   else
   {
      vars->r++;
   }

   if ( rehash )
   {
      rehash = reorgHashtable( TSD, vars );
   }

   vt->tmpindex->len = 0;
   vt->tmpindex = Str_nocatTSD( vt->tmpindex, name, name->len - stop, stop );

   /*
    * FIXME, FGC: vt->subst from "if" removed, but what shall we do here really?
    */
   if ( !vt->notrace )   /* trace it */
      tracecompound( TSD, name, stop - 1, vt->tmpindex, 'C' );

   nnptr = &((ptr->index->tbl)[hashfunc( vt, vt->tmpindex, 0, NULL,
                                         ptr->index->size)]);
   vars->r++;
   nptr = *nnptr;
   SEEK_VAR_CMP( nptr, vt->tmpindex, vt->fullhash, ptr->index, rehashIdx );
   SEEK_EXPOSED( nptr );

   if ( nptr )
   {
      vt->foundflag = nptr && ( nptr->flag & VFLAG_BOTH );
      REPLACE_VALUE( value, nptr );
   }
   else
   {
      newbox( TSD, vt->tmpindex, value, nnptr, vt->fullhash );
      ptr->index->e++;
      NEW_HASHTABLE_CHECK( ptr->index, rehashIdx );
      (*nnptr)->stem = ptr;
   }

   vt->thespot = NULL;
   if ( rehashIdx )
   {
      rehashIdx = reorgHashtable( TSD, ptr->index );
   }
   return rehash | rehashIdx;
}

static void expose_simple( const tsd_t *TSD, var_hashtable *vars,
                           const streng *name )
{
   unsigned hashv, hashn;
   variableptr ptr;
   int rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   hashv = hashfunc( vt, name, 0, NULL, vt->var_table->size );
   ptr = vt->var_table->tbl[hashv];
   vt->var_table->r++;
   SEEK_VAR_CCMP( ptr, name, vt->fullhash, vt->var_table, rehash );

   if ( ptr )  /* hey, you just exposed that one! */
      return;

   rehash = 0; /* ignore here */
   hashn = vt->fullhash % vars->size;
   ptr = vars->tbl[hashn];
   vars->w++;
   SEEK_VAR_CCMP( ptr, name, vt->fullhash, vars, rehash );
   SEEK_EXPOSED( ptr );

   if ( !ptr )
   {
      newbox( TSD, name, NULL, &vars->tbl[hashn], vt->fullhash );
      vars->e++;
      NEW_HASHTABLE_CHECK( vars, rehash );
      ptr = vars->tbl[hashn];
   }

   if ( rehash )
   {
      reorgHashtable( TSD, vars );
      rehash = 0;
   }

   newbox( TSD, name, NULL, &vt->var_table->tbl[hashv], vt->fullhash );
   vt->var_table->e++;
   NEW_HASHTABLE_CHECK( vt->var_table, rehash );
   vt->var_table->tbl[hashv]->realbox = ptr;
   /*
    * exposing is done after create_new_varpool/assignment of current_valid:
    */
   vt->var_table->tbl[hashv]->realbox->valid = vt->current_valid;
   if ( rehash )
   {
      reorgHashtable( TSD, vt->var_table );
   }

   DSTART;DPRINT((TSD,"expose_simple:     "));DNAME(TSD,"name",name);DEND;
}

static void expose_stem( const tsd_t *TSD, var_hashtable *vars,
                         const streng *name )
{
   variableptr ptr,tptr;
   unsigned hashv, hashn;
   int junk, rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"expose_stem:       ?"));
   hashv = hashfunc(vt, name, 0, &junk, vt->var_table->size );
   ptr = vt->var_table->tbl[hashv];
   vt->var_table->r++;
   SEEK_VAR_CCMP( ptr, name, vt->fullhash, vt->var_table, rehash );

   if ( ptr && ptr->realbox )
      return; /* once is enough !!! */

   rehash = 0;
   hashn = vt->fullhash % vars->size;
   tptr = vars->tbl[hashn];
   vars->w++;
   SEEK_VAR_CCMP( tptr, name, vt->fullhash, vars, rehash );
   SEEK_EXPOSED( tptr );

   if ( !tptr )
   {
      newbox( TSD, name, NULL, &vars->tbl[hashn], vt->fullhash );
      vars->e++;
      NEW_HASHTABLE_CHECK( vars, rehash );
      tptr = vars->tbl[hashn];
      tptr->index = make_hash_table( TSD, vt->initialHashTableLength );
      DPRINTF((TSD,"make_hash_table:   rc=%p",tptr->index));
   }
   if ( rehash )
   {
      reorgHashtable( TSD, vars );
      rehash = 0;
   }

   if ( ptr )
   {
      /*
       * The stem has been generated by an "expose STEM.x ... " and we now
       * have the variable "STEM.". The "STEM.x" had produced a new copy of
       * "STEM." because of the need to access "STEM.x", but now we have to
       * expose everything of "STEM.". We remove the index with the
       * realbox-chain of ".x" and make this "STEM." point to the exposed one.
       */
      remove_foliage( TSD, ptr->index );
      ptr->index = NULL;
      assert( ( ptr->realbox == NULL ) || ( ptr->realbox == tptr ) );
      ptr->realbox = tptr;
   }
   else
   {
      newbox( TSD, name, NULL, &vt->var_table->tbl[hashv], vt->fullhash );
      vt->var_table->e++;
      NEW_HASHTABLE_CHECK( vt->var_table, rehash );
      vt->var_table->tbl[hashv]->realbox = tptr; /* dont need ->index */
   }
   if ( rehash )
   {
      reorgHashtable( TSD, vt->var_table );
   }
}

static void expose_compound( tsd_t *TSD, var_hashtable *vars,
                             const streng *name )
{
   unsigned hashv, hashn, fh, hashv2, hashn2;
   int length, rehash = 0;

   variableptr ptr, nptr, tptr, tiptr;
   int cptr;
   streng *indexstr;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"expose_compound:   ?"));
   hashv = hashfunc( vt, name, 0, &cptr, vt->var_table->size );
   ptr = vt->var_table->tbl[hashv];
   length = ++cptr;
   vt->var_table->r++;
   fh = vt->fullhash;
   SEEK_VAR_CNCMP( ptr, name, fh, vt->var_table, length, rehash );
   if ( ptr && ptr->realbox )
      return; /* whole array already exposed */

   hashn = fh % vars->size;
   if ( !ptr ) /* array does not exist */
   {
      make_stem( TSD, name, NULL, &vt->var_table->tbl[hashv], length, fh );
      vt->var_table->e++;
      NEW_HASHTABLE_CHECK( vt->var_table, rehash );
      ptr = vt->var_table->tbl[hashv];
   }

   indexstr = subst_index( TSD, name, cptr, vt->var_table, &rehash );

   if ( rehash )
   {
      reorgHashtable( TSD, vt->var_table );
      rehash = 0;
   }

   if ( vt->subst )   /* trace it */
      tracecompound( TSD, name, cptr - 1, indexstr, 'C');

   hashv2 = hashfunc( vt, indexstr, 0, NULL, ptr->index->size );
   nptr = ptr->index->tbl[hashv2];
   ptr->index->r++;
   SEEK_VAR_CMP( nptr, indexstr, vt->fullhash, ptr->index, rehash );

   if ( nptr && nptr->realbox )
      return;
   else
   {
      newbox( TSD, indexstr, NULL, &ptr->index->tbl[hashv2], vt->fullhash );
      ptr->index->e++;
      NEW_HASHTABLE_CHECK( ptr->index, rehash );
      nptr = ptr->index->tbl[hashv2];
      nptr->stem = ptr;
   }
   if ( rehash )
   {
      reorgHashtable( TSD, ptr->index );
      rehash = 0;
   }

   tptr = vars->tbl[hashn];
   vars->w++;
   SEEK_VAR_CNCMP( tptr, name, fh, vars, length, rehash );
   SEEK_EXPOSED( tptr );

   if ( !tptr )
   {
      make_stem( TSD, name, NULL, &vars->tbl[hashn], length, fh );
      vars->e++;
      NEW_HASHTABLE_CHECK( vars, rehash );
      tptr = vars->tbl[hashn];
   }
   if ( rehash )
   {
      reorgHashtable( TSD, vars );
      rehash = 0;
   }

   hashn2 = vt->fullhash % tptr->index->size;
   tiptr = tptr->index->tbl[hashn2];
   tptr->index->w++;
   SEEK_VAR_CMP( tiptr, indexstr, vt->fullhash, tptr->index, rehash );
   SEEK_EXPOSED( tiptr );

   if ( !tiptr )
   {
      /*
       * hopefully no new setting of vt->fullhash has happened during the last
       * lines.
       */
      newbox( TSD, indexstr, NULL, &tptr->index->tbl[hashn2], vt->fullhash );
      tptr->index->e++;
      NEW_HASHTABLE_CHECK( tptr->index, rehash );
      tiptr = tptr->index->tbl[hashn2];
      tiptr->stem = tptr;
   }
   if ( rehash )
   {
      reorgHashtable( TSD, tptr->index );
   }

   nptr->realbox = tiptr;
}

static const streng *getvalue_compound( tsd_t *TSD, var_hashtable *vars,
                                        const streng *name )
{
   int baselength;
   int rehash = 0;
   unsigned hashv;
   variableptr ptr, nptr;
   streng *value;
   streng *indexstr;
   int stop;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"getvalue_compound: ?"));
   hashv = hashfunc( vt, name, 0, &stop, vars->size );
   ptr = vars->tbl[hashv];
   baselength = ++stop;
   vars->r++;
   SEEK_VAR_CNCMP( ptr, name, vt->fullhash, vars, baselength, rehash );
   SEEK_EXPOSED( ptr );

   indexstr = subst_index( TSD, name, stop, vars, &rehash );

   if ( rehash )
   {
      reorgHashtable( TSD, vars );
      rehash = 0;
   }

   if ( vt->subst && !vt->notrace )   /* trace it */
      tracecompound( TSD, name, baselength - 1, indexstr, 'C' );

   if ( ptr )
   {
      hashv = hashfunc( vt, indexstr, 0, NULL, ptr->index->size );
      nptr = ptr->index->tbl[hashv];
      ptr->index->r++;
      SEEK_VAR_CMP( nptr, indexstr, vt->fullhash, ptr->index, rehash );
      SEEK_EXPOSED( nptr );

      if ( !nptr )   /* find default value */
         nptr = ptr;

      vt->foundflag = nptr->flag & VFLAG_BOTH;
      expand_to_str( TSD, nptr );
   }
   else
   {
      vt->foundflag = 0;
      nptr = NULL;
   }
   if ( rehash )
   {
      reorgHashtable( TSD, ptr->index );
   }

   if ( vt->foundflag )
      value = nptr->value;
   else
   {
      if ( !vt->ignore_novalue )
         condition_hook( TSD, SIGNAL_NOVALUE, 0, 0, -1, Str_dupTSD( name ), NULL ) ;

      if ( vt->ovalue )
         Free_stringTSD( vt->ovalue );

      vt->ovalue = value = Str_makeTSD( stop + 1 + Str_len( indexstr ) );
      Str_ncatTSD( value, name, stop );
      Str_catTSD( value, indexstr );
   }

   vt->thespot = NULL;
   return value;
}

/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */
/****************************************************************************
 *
 *  JH 13/12/1999 (Original code changes on 20/10/1999)
 *
 *  BUG022            To make Direct setting of stems Direct and not Symbolic.
 *   - Adapted from getvalue_compound().
 *   - Started using the global variable, vt->tmpindex, in place of the local,
 *     indexstr.
 *   - manually move the first stem name into vt->tmpindex, do not call
 *     subst_index(), as that not only uppercases the tail, but also
 *     does not uppercase the tail.
 *
 *
 ****************************************************************************/
static const streng *getdirvalue_compound( tsd_t *TSD, var_hashtable *vars,
                                           const streng *name )
{
   int baselength, rehash = 0;
   unsigned hashv;
   variableptr ptr, nptr;
   streng *value;
   int stop;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"getdirvalue_compound: ?"));
   /*
    * Get a good starting point, and find the stem/index separater.
    */
   hashv = hashfunc( vt, name, 0, &stop, vars->size );
   ptr = vars->tbl[hashv];
   baselength = ++stop;
   vars->r++;
   /*
    * Find the stem in the variable pool.
    */
   SEEK_VAR_CNCMP( ptr, name, vt->fullhash, vars, baselength, rehash );
   SEEK_EXPOSED( ptr );

   if ( rehash )
   {
      reorgHashtable( TSD, vars );
      rehash = 0;
   }

   vt->tmpindex->len = 0;
   vt->tmpindex = Str_nocatTSD( vt->tmpindex, name, name->len - stop, stop );

   /*
    * FIXME, FGC: vt->subst from "if" removed, but what shall we do here really?
    */
   if ( !vt->notrace )   /* trace it */
      tracecompound( TSD, name, baselength - 1, vt->tmpindex, 'C' );

   if ( ptr )
   {
      hashv = hashfunc( vt, vt->tmpindex, 0, NULL, ptr->index->size );
      nptr = ptr->index->tbl[hashv];
      ptr->index->r++;
      /*
       * Find the index in the variable pool.
       */
      SEEK_VAR_CMP( nptr, vt->tmpindex, vt->fullhash, ptr->index, rehash );
      SEEK_EXPOSED( nptr );

      /*
       * If the stem exists, but the index doesn't, this counts as found.
       */
      if ( !nptr )   /* find default value */
         nptr = ptr;

      vt->foundflag = nptr->flag & VFLAG_BOTH;
      expand_to_str( TSD, nptr );
   }
   else
   {
      vt->foundflag = 0;
      nptr = NULL;
   }
   if ( rehash )
   {
      reorgHashtable( TSD, ptr->index );
   }


   if ( vt->foundflag )
      value = nptr->value;
   else
   {
      if ( !vt->ignore_novalue )
         condition_hook( TSD, SIGNAL_NOVALUE, 0, 0, -1, Str_dupTSD( name ), NULL ) ;

      /*
       * Since this is a direct, we can use the name without change.
       */
      value = (streng *) name;
   }

   vt->thespot = NULL;
   return value;
}

/*
 * getPool returns the variable pool of the specified pool number. The number
 * may be -1 for "autoselect". *isRes is set to 1 if the name is a reserved
 * variable, 0 otherwise.
 */
static var_hashtable *getPool( const tsd_t *TSD, const streng *name, int pool,
                               int *isRes )
{
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;
   sysinfo s;
   proclevel p;

   if ( KNOWN_RESERVED( name->value, Str_len( name ) ) )
   {
      *isRes = 1;
      if ( ( pool == 0 ) || ( pool == -1 ) )
         return vt->pool0;
   }
   else
      *isRes = 0;

   if ( pool == 0 )
      return vt->pool0;

   if ( pool == -1 )
      return TSD->currlevel->vars;

   /*
    * The slow part. We have to find the true pool for the given number.
    * Each system manages a part of the proclevels. We have to walk back
    * through the sysinfo chain until we find the containing system and
    * pick up the desired proclevel afterwards.
    */
   s = TSD->systeminfo;
   while ( s->currlevel0->pool > pool )
   {
      s = s->previous;
      assert( s != NULL );
   }

   if ( s == TSD->systeminfo )
   {
      /*
       * We can use the previous-chain which may be faster.
       */
      if ( pool > ( TSD->currlevel->pool - s->currlevel0->pool ) / 2
                  + s->currlevel0->pool )
      {
         p = TSD->currlevel;
         while ( p->pool != pool )
         {
            p = p->prev;
            assert( p != NULL );
         }
         return p->vars;
      }
   }

   p = s->currlevel0;
   while ( p->pool != pool )
   {
      p = p->next;
      assert( p != NULL );
   }
   return p->vars;
}

/*
 * This is the entry-level routine that will take the parameters,
 * decide what kind of variable it is (simple, stem or compound) and
 * call the appropriate routine to do the dirty work.
 * pool is -1 (select the appropriate one) or a pool number.
 */
void setvalue( tsd_t *TSD, const streng *name, streng *value, int pool )
{
   int i, isRes, len=Str_len( name );
   var_hashtable *vars;

   assert( value->len <= value->max );

   vars = getPool( TSD, name, pool, &isRes );

   if ( isRes )
      setvalue_simple( TSD, vars, name, value );
   else
   {
      for ( i = 0; ( i < len ) && ( name->value[i] != '.' ); i++ )
         ;

      if ( i == len )
         setvalue_simple( TSD, vars, name, value );
      else if ( i + 1 == len )
         setvalue_stem( TSD, vars, name, value );
      else
         setvalue_compound( TSD, vars, name, value );
   }
}

/*
 * This is the entry-level routine used by the Variable Pool Interface
 *  to set stem variables directly.  (no translation on the index name.)
 *  As setvalue() does, it will take the parameters,
 *  decide what kind of variable it is (simple, stem or compound) and
 *  call the appropriate routine to do the dirty work
 */
/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */
/****************************************************************************
 *
 *  JH 13/12/1999 (Original code changes on 20/10/1999)
 *
 *  BUG022            To make Direct setting of stems Direct and not Symbolic.
 *   - Adapted from setvalue().
 *   - changed call from setvalue_compound() to setdirvalue_compound().
 *
 *
 ****************************************************************************/
void setdirvalue( tsd_t *TSD, const streng *name, streng *value )
{
   var_tsd_t *vt;
   int i, len=Str_len( name );
   var_hashtable *vars;

   assert( value->len <= value->max );

   if ( KNOWN_RESERVED( name->value, len ) )
   {
      vt = (var_tsd_t *)TSD->var_tsd;
      setvalue_simple( TSD, vt->pool0, name, value );
   }
   else
   {
      vars = TSD->currlevel->vars;

      for ( i = 0; ( i < len ) && ( name->value[i] != '.' ); i++ )
         ;

      if ( i == len )
         setvalue_simple( TSD, vars, name, value );
      else if ( i + 1 == len )
         setvalue_stem( TSD, vars, name, value );
      else
         setdirvalue_compound( TSD, vars, name, value );
   }
}

void expose_var( tsd_t *TSD, const streng *name )
{
   int i, len;
   var_hashtable *vars;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   if ( !vt->var_table )
   {
      /*
       * First call to expose_var of X_PROC in interpret().
       * Open a new table.
       */
      vt->var_table = create_new_varpool( TSD, 0 );
   }

   if ( !name )
   {
      /*
       * Last call to expose_var of X_PROC in interpret().
       * Use the new table as the current table.
       */
      TSD->currlevel->vars = vt->var_table;
      TSD->currlevel->varflag = 1;
      vt->var_table = NULL;
      return;
   }

   len = Str_len( name );
   if ( KNOWN_RESERVED( name->value, len ) )
      expose_simple( TSD, vt->pool0, name );
   else
   {
      vars = TSD->currlevel->vars;

      for ( i = 0; ( i < len ) && ( name->value[i] != '.' ); i++ )
         ;

      if ( i == len )
         expose_simple( TSD, vars, name );
      else if ( i + 1 == len )
         expose_stem( TSD, vars, name );
      else
         expose_compound( TSD, vars, name );
   }
}

const streng *getvalue( tsd_t *TSD, const streng *name, int pool )
{
   int i, isRes, len=Str_len( name );
   var_hashtable *vars;

   vars = getPool( TSD, name, pool, &isRes );

   if ( isRes )
      return getvalue_simple( TSD, vars, name );

   for ( i = 0; ( i < len ) && ( name->value[i] != '.' ); i++ )
      ;

   /*
    * getvalue_stem is equivalent to getvalue_simple
    */
   if ( i >= len - 1 )
      return getvalue_simple( TSD, vars, name );

   return getvalue_compound( TSD, vars, name );
}

/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */
/****************************************************************************
 *
 *  JH 13/12/1999 (Original code changes on 20/10/1999)
 *
 *  BUG022            To make Direct setting of stems Direct and not Symbolic.
 *   - Adapted from getvalue().
 *   - changed call from getvalue_compound() to getdirvalue_compound().
 *
 *
 ****************************************************************************/
const streng *getdirvalue( tsd_t *TSD, const streng *name )
{
   var_tsd_t *vt;
   int i, len=Str_len( name );
   var_hashtable *vars;

   if ( ( i = KNOWN_RESERVED( name->value, len ) ) != 0 )
   {
      vt = (var_tsd_t *)TSD->var_tsd;
      return getvalue_simple( TSD, vt->pool0, name );
   }

   for ( i = 0; ( i < len ) && ( name->value[i] != '.' ); i++ )
      ;

   vars = TSD->currlevel->vars;

   if ( i >= len - 1 )
      return getvalue_simple( TSD, vars, name );

   return getdirvalue_compound( TSD, vars, name );
}

static void drop_var_simple( const tsd_t *TSD, var_hashtable *vars,
                             const streng *name )
{
   variableptr ptr;
   int rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   vars->w++;
   ptr = findsimple( TSD, vars, name, &rehash );
   DSTART;DPRINT((TSD,"drop_var_simple:   "));DNAME(TSD,"name",name);DVAR(TSD,", var",ptr);
          DEND;

   vt->foundflag = 0;
   if ( ptr )
   {
      vt->foundflag = ptr->flag & VFLAG_BOTH;
      ptr->flag = VFLAG_NONE;
      if ( ptr->value )
      {
         Free_stringTSD( ptr->value );
         ptr->value = NULL;
      }
      if ( ptr->num )
      {
         FreeTSD( ptr->num->num );
         FreeTSD( ptr->num );
         ptr->num = NULL;
      }
   }
   if ( rehash )
   {
      reorgHashtable( TSD, vars );
   }
}

static void drop_var_stem( const tsd_t *TSD, var_hashtable *vars,
                           const streng *name )
{
   variableptr ptr;
   int rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"drop_var_stem:     ?"));
   vars->w++;
   ptr = findsimple( TSD, vars, name, &rehash );

   vt->foundflag = 0;
   if ( ptr )
   {
      vt->foundflag = ptr->flag & VFLAG_BOTH;
      ptr->flag = VFLAG_NONE;
      if ( ptr->value )
      {
         Free_stringTSD( ptr->value );
         ptr->value = NULL;
      }
      if ( ptr->num )
      {
         FreeTSD( ptr->num->num );
         FreeTSD( ptr->num );
         ptr->num = NULL;
      }

      assert( ptr->index );
      if ( ptr->index )
         assign_foliage( TSD, ptr->index, NULL );
   }
   if ( rehash )
   {
      reorgHashtable( TSD, vars );
   }
}

static void drop_var_compound( tsd_t *TSD, var_hashtable *vars,
                               const streng *name )
{
   int baselength;
   unsigned hashv;
   variableptr ptr, nptr=NULL;
   streng *indexstr;
   int stop, rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"drop_var_compound: ?"));
   hashv = hashfunc( vt, name, 0, &stop, vars->size );
   ptr = vars->tbl[hashv];
   baselength = ++stop;
   vars->r++;

   SEEK_VAR_CNCMP( ptr, name, vt->fullhash, vars, baselength, rehash );
   SEEK_EXPOSED( ptr );

   indexstr = subst_index( TSD, name, stop, vars, &rehash );

   if ( rehash )
   {
      reorgHashtable( TSD, vars );
      rehash = 0;
   }

   if ( vt->subst && !vt->notrace )   /* trace it */
      tracecompound( TSD, name, baselength - 1, indexstr, 'C' );

   if ( ptr )
   {
      hashv = hashfunc( vt, indexstr, 0, NULL, ptr->index->size );
      nptr = ptr->index->tbl[hashv];
      ptr->index->w++;
      SEEK_VAR_CMP( nptr, indexstr, vt->fullhash, ptr->index, rehash );
      SEEK_EXPOSED( nptr );

      if ( rehash )
      {
         reorgHashtable( TSD, ptr->index );
      }
   }

   vt->foundflag = nptr && ( nptr->flag & VFLAG_BOTH );

   if ( nptr )
   {
      nptr->flag = VFLAG_NONE;
      if ( nptr->value )
      {
         FreeTSD( nptr->value );
         nptr->value = NULL;
      }
      if ( nptr->num )
      {
         FreeTSD( nptr->num->num );
         FreeTSD( nptr->num );
         nptr->num = NULL;
      }
   }
   else
   {
      /*
       * We are playing with the NULL-ptr ... take care !
       */
      if ( ptr )
         setvalue_compound( TSD, vars, name, NULL );
   }
}

/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */
/****************************************************************************
 *
 *  JH 13/12/1999 (Original code changes on 20/10/1999)
 *
 *  BUG022            To make Direct setting of stems Direct and not Symbolic.
 *   - Adapted from drop_var_compound().
 *   - Started using the global variable, vt->tmpindex, in place of the local,
 *     indexstr.
 *   - manually move the first stem name into vt->tmpindex, do not call
 *     subst_index(), as that not only uppercases the tail, but also
 *     does not uppercase the tail.
 *   - changed call from setvalue_compound() to setdirvalue_compound().
 *
 *
 ****************************************************************************/
static void drop_dirvar_compound( tsd_t *TSD, var_hashtable *vars,
                                  const streng *name )
{
   int baselength;
   unsigned hashv;
   variableptr ptr, nptr=NULL;
   int stop, rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"drop_dirvar_compound: ?"));
   hashv = hashfunc( vt, name, 0, &stop, vars->size );
   ptr = vars->tbl[hashv];
   baselength = ++stop;
   vars->r++;

   SEEK_VAR_CNCMP( ptr, name, vt->fullhash, vars, baselength, rehash );
   SEEK_EXPOSED( ptr );

   vt->tmpindex->len = 0;
   vt->tmpindex = Str_nocatTSD( vt->tmpindex, name, name->len - stop, stop );

   if ( rehash )
   {
      reorgHashtable( TSD, vars );
      rehash = 0;
   }

   /*
    * FIXME, FGC: vt->subst isn't set anywhere.
    */
   if ( vt->subst && !vt->notrace )   /* trace it */
      tracecompound( TSD, name, baselength - 1, vt->tmpindex, 'C' );

   if ( ptr )
   {
      hashv = hashfunc( vt, vt->tmpindex, 0, NULL, ptr->index->size );
      nptr = ptr->index->tbl[hashv];
      ptr->index->w++;
      SEEK_VAR_CMP( nptr, vt->tmpindex, vt->fullhash, ptr->index, rehash );
      SEEK_EXPOSED( nptr );

      if ( rehash )
      {
         reorgHashtable( TSD, ptr->index );
      }
   }

   vt->foundflag = nptr && ( nptr->flag & VFLAG_BOTH );

   if ( nptr )
   {
      nptr->flag = VFLAG_NONE;
      if ( nptr->value )
      {
         FreeTSD( nptr->value );
         nptr->value = NULL;
      }
      if ( nptr->num )
      {
         FreeTSD( nptr->num->num );
         FreeTSD( nptr->num );
         nptr->num = NULL;
      }
   }
   else
   {
      /*
       * We are playing with the NULL-ptr ... take care !
       */
      if ( ptr )
         setdirvalue_compound( TSD, vars, name, NULL );
   }
}

void drop_var( tsd_t *TSD, const streng *name )
{
   var_tsd_t *vt;
   int i, len=Str_len( name );
   var_hashtable *vars;

   if ( ( i = KNOWN_RESERVED( name->value, len ) ) != 0 )
   {
      vt = (var_tsd_t *)TSD->var_tsd;
      drop_var_simple( TSD, vt->pool0, name );
   }
   else
   {
      for ( i = 0; ( i < len ) && ( name->value[i] != '.' ); i++ )
         ;

      vars = TSD->currlevel->vars;

      if ( i == len )
         drop_var_simple( TSD, vars, name );
      else if ( i + 1 == len )
         drop_var_stem( TSD, vars, name );
      else
         drop_var_compound( TSD, vars, name );
   }
}

/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */
/****************************************************************************
 *
 *  JH 13/12/1999 (Original code changes on 20/10/1999)
 *
 *  BUG022            To make Direct setting of stems Direct and not Symbolic.
 *   - Adapted from drop_var().  Changed call drop_var_compound() to
 *     drop_dirvar_compound().  *** May need to do the same for drop_var_stem(). ****
 *
 ****************************************************************************/
void drop_dirvar( tsd_t *TSD, const streng *name )
{
   var_tsd_t *vt;
   int i, len=Str_len( name );
   var_hashtable *vars;

   if ( ( i = KNOWN_RESERVED( name->value, len ) ) != 0 )
   {
      vt = (var_tsd_t *)TSD->var_tsd;
      drop_var_simple( TSD, vt->pool0, name );
   }
   else
   {
      for ( i = 0; ( i < len ) && ( name->value[i] != '.' ); i++ )
         ;

      vars = TSD->currlevel->vars;

      if ( i == len )
         drop_var_simple( TSD, vars, name );
      else if ( i + 1 == len )
         drop_var_stem( TSD, vars, name );
      else
         drop_dirvar_compound( TSD, vars, name );
   }
}

static void upper_var_simple( tsd_t *TSD, var_hashtable *vars,
                              const streng *name )
{
   variableptr ptr;
   streng *value=NULL;
   int rehash = 0;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   vars->w++;
   ptr = findsimple( TSD, vars, name, &rehash );
   if ( rehash )
   {
      reorgHashtable( TSD, vars );
   }

   vt->foundflag = ptr && (ptr->flag & VFLAG_BOTH );

   if ( ptr )
   {
      expand_to_str( TSD, ptr );
   }

   if ( vt->foundflag )
   {
      value = ptr->value;
      Str_upper( value );
      if ( !vt->notrace )
         tracevalue( TSD, value, 'V' );
   }
   else
   {
      vt->thespot = NULL;
      if ( !vt->notrace )
         tracevalue( TSD, name, 'L' );
      if ( !vt->ignore_novalue )
         condition_hook( TSD, SIGNAL_NOVALUE, 0, 0, -1, Str_dupTSD( name ), NULL ) ;
   }

   DSTART;DPRINT((TSD,"upper_var_simple:   "));DNAME(TSD,"name",name);
          DVALUE(TSD," rc",value);DEND;
}

static void upper_var_compound( tsd_t *TSD, var_hashtable *vars,
                                const streng *name )
{
   int baselength, rehash = 0;
   unsigned hashv;
   variableptr ptr, nptr;
   streng *indexstr;
   streng *value;
   int stop;
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"upper_var_compound: ?"));

   hashv = hashfunc( vt, name, 0, &stop, vars->size );
   ptr = vars->tbl[hashv];
   baselength = ++stop;
   vars->r++;
   SEEK_VAR_CNCMP( ptr, name, vt->fullhash, vars, baselength, rehash );
   SEEK_EXPOSED( ptr);

   indexstr = subst_index( TSD, name, stop, vars, &rehash );

   if ( rehash )
   {
      reorgHashtable( TSD, vars );
      rehash = 0;
   }

   if ( vt->subst && !vt->notrace )   /* trace it */
      tracecompound( TSD, name, baselength - 1, indexstr, 'C' );

   if ( ptr )
   {
      hashv = hashfunc( vt, indexstr, 0, NULL, ptr->index->size );
      nptr = ptr->index->tbl[hashv];
      ptr->index->w++;
      SEEK_VAR_CMP( nptr, indexstr, vt->fullhash, ptr->index, rehash );
      SEEK_EXPOSED( nptr );

      if ( rehash )
      {
         reorgHashtable( TSD, ptr->index );
      }
   }
   else
      nptr = NULL;

   vt->foundflag = nptr && ( nptr->flag & VFLAG_BOTH );
   if ( nptr )
   {
      expand_to_str( TSD, nptr );
   }
   if ( vt->foundflag )
   {
      value = nptr->value;
      Str_upper( value );
      if ( !vt->notrace )
         tracevalue( TSD, value, 'V' );
   }
   else
   {
      if ( !vt->notrace )
         tracevalue( TSD, name, 'L' );
      if ( !vt->ignore_novalue )
         condition_hook( TSD, SIGNAL_NOVALUE, 0, 0, -1, Str_dupTSD( name ), NULL ) ;
   }

   vt->thespot = NULL;
}

void upper_var( tsd_t *TSD, const streng *name )
{
   var_tsd_t *vt;
   int i, len=Str_len( name );
   var_hashtable *vars;

   if ( ( i = KNOWN_RESERVED( name->value, len ) ) != 0 )
   {
      vt = (var_tsd_t *)TSD->var_tsd;
      upper_var_simple( TSD, vt->pool0, name );
   }
   else
   {
      for ( i = 0; ( i < len ) && ( name->value[i] != '.' ); i++ )
         ;

      vars = TSD->currlevel->vars;

      if ( i == len )
         upper_var_simple( TSD, vars, name );
      else if ( i + 1 == len )
         exiterror( ERR_INVALID_STEM, 0 );
      else
      {
         set_ignore_novalue( TSD );
         upper_var_compound( TSD, vars, name );
         clear_ignore_novalue( TSD );
      }
   }
}

void kill_variables( const tsd_t *TSD, var_hashtable *array )
{
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   DPRINTF((TSD,"kill_variables:    current_valid:old=%ld, new=%ld",
            vt->current_valid,(long) array->tbl[array->size]));

   vt->current_valid = (long) array->tbl[array->size];

   remove_foliage( TSD, array );

   if ( vt->current_valid == 1 )
      vt->next_current_valid = 2;
   assert( vt->current_valid );
}

/*
 * This is the shortcut method for retrieving the value of a variable.
 * It requires you to have a nodeptr, which may contain a shortcut
 * pointer into the variable pool. Unless, such a shortcut pointer is
 * established, if possible.
 * Note: This is one of the most time-consuming routines. Be careful.
 */
const streng *shortcut( tsd_t *TSD, nodeptr thisptr )
{
   const streng *result;
   char ch;
   variableptr vptr;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   DSTART;DPRINT((TSD,"shortcut:          "));DNAME(TSD,"thisptr->name",thisptr->name);DEND;
   if ( ( vptr = thisptr->u.varbx ) != NULL )
   {
      if ( vptr->valid == vt->current_valid )
      {
         DSTART;DPRINT((TSD,"shortcut:          "));DVAR(TSD,"valid vptr",vptr);
                DPRINT((TSD," on start"));DEND;
         ch = 'V';
         SEEK_EXPOSED( vptr );
         if ( vptr->flag & VFLAG_STR )
            result = vptr->value;
         else if ( vptr->flag & VFLAG_NUM )
         {
            expand_to_str( TSD, vptr );
            result = vptr->value;
         }
         else
         {
            ch = 'L';
            result = vptr->name;
            if ( !vt->ignore_novalue )
               condition_hook( TSD, SIGNAL_NOVALUE, 0, 0, -1, Str_dupTSD( result ), NULL );
         }
         DSTART;DPRINT((TSD,"shortcut:          "));DVAR(TSD,"valid vptr",vptr);
                DPRINT((TSD," on end"));DEND;

         if ( TSD->trace_stat == 'I' )
            tracevalue( TSD, result, ch );

         assert( !result || result->len <= result->max );
         DSTART;DPRINT((TSD,"shortcut:          "));DVALUE(TSD,"rc",result);DEND;
         return result;
      }
      else
      {
         DSTART;DPRINT((TSD,"shortcut:          "));DVAR(TSD,"INVALID vptr",vptr);
                DPRINT((TSD," on start"));DEND;
         if ( --vptr->hwired == 0 )
            if ( !vptr->valid )
            {
#ifdef TRACEMEM
               if ( vptr->prev )
                  vptr->prev->next = vptr->next;
               if ( vptr->next )
                  vptr->next->prev = vptr->prev;
               else
                  vt->first_invalid = vptr->prev;
#endif
               /*
                * See comment on REMOVE_ELEMENT why we just remove the body
                */
               FreeTSD( vptr );
            }
         thisptr->u.varbx = NULL;
      }
   }

   result = getvalue( TSD, thisptr->name, -1 ); /* changes the pool */
   if ( vt->thespot )
   {
      vt->thespot->hwired++;
      thisptr->u.varbx = vt->thespot;
   }

   DSTART;DPRINT((TSD,"shortcut:          "));DVAR(TSD,"new vt->thespot",vt->thespot);DEND;
   DSTART;DPRINT((TSD,"shortcut:          "));DVALUE(TSD,"rc",result);DEND;
   assert( !result || result->len <= result->max );
   return result;
}

num_descr *shortcutnum( tsd_t *TSD, nodeptr thisptr )
{
   variableptr vptr;
   num_descr *result;
   const streng *resstr;
   char ch;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   DSTART;DPRINT((TSD,"shortcutnum:       "));DNAME(TSD,"thisptr->name",thisptr->name);DEND;
   if ( ( vptr = thisptr->u.varbx ) != NULL )
   {
      if ( vptr->valid == vt->current_valid )
      {
         DSTART;DPRINT((TSD,"shortcutnum:       "));DVAR(TSD,"valid vptr",vptr);
                DPRINT((TSD," on start"));DEND;
         SEEK_EXPOSED( vptr );
         ch = 'V';
         if ( vptr->flag & VFLAG_NUM )
         {
            result = vptr->num;
            if ( TSD->trace_stat == 'I' )
               tracenumber( TSD, result, 'V' );
         }
         else if ( vptr->flag & VFLAG_STR )
         {
            if ( vptr->num )
            {
               FreeTSD( vptr->num->num );
               FreeTSD( vptr->num );
            }
            if ( TSD->trace_stat == 'I' )
               tracevalue( TSD, vptr->value, 'V' );
            vptr->num = is_a_descr( TSD, vptr->value );
            if ( vptr->num )
               vptr->flag |= VFLAG_NUM;
            result = vptr->num;
         }
         else
         {
            result = NULL;
            if ( TSD->trace_stat == 'I' )
               tracevalue( TSD, thisptr->name, 'L' );
            if ( !vt->ignore_novalue )
               condition_hook( TSD, SIGNAL_NOVALUE, 0, 0, -1, Str_dupTSD( thisptr->name ), NULL );
         }
         DSTART;DPRINT((TSD,"shortcutnum:       "));DVAR(TSD,"valid vptr",vptr);
                DPRINT((TSD," on end"));DEND;
         DSTART;DPRINT((TSD,"shortcutnum:       "));DNUM(TSD,"rc",result);DEND;
         return result;
      }
      else
      {
         DSTART;DPRINT((TSD,"shortcutnum:       "));DVAR(TSD,"INVALID vptr",vptr);
                DPRINT((TSD," on start"));DEND;
         if ( --vptr->hwired == 0 )
            if ( !vptr->valid )
            {
#ifdef TRACEMEM
               if ( vptr->prev )
                  vptr->prev->next = vptr->next;
               if ( vptr->next )
                  vptr->next->prev = vptr->prev;
               else
                  vt->first_invalid = vptr->prev;
#endif
               /*
                * See comment on REMOVE_ELEMENT why we just remove the body
                */
               FreeTSD( thisptr->u.varbx );
            }
         thisptr->u.varbx = NULL;
      }
   }

   resstr = getvalue( TSD, thisptr->name, -1 ); /* changes the pool */
   if ( vt->thespot )
   {
      vt->thespot->hwired++;
      thisptr->u.varbx = vt->thespot;
      if ( vt->thespot->num )
      {
         if ( vt->thespot->flag & VFLAG_NUM )
            return vt->thespot->num;
         FreeTSD( vt->thespot->num->num );
         FreeTSD( vt->thespot->num );

      }
      vt->thespot->num = is_a_descr( TSD, resstr );
      if ( vt->thespot->num )
         vt->thespot->flag |= VFLAG_NUM;
   }
   else
   {
      if ( vt->odescr )
      {
         FreeTSD( vt->odescr->num );
         FreeTSD( vt->odescr );
      }
      vt->odescr = is_a_descr( TSD, resstr );
      DSTART;DPRINT((TSD,"shortcutnum:       "));DVALUE(TSD,"NO!!! vt->thespot, resstr",resstr);DEND;
      DSTART;DPRINT((TSD,"shortcutnum:       "));DNUM(TSD,"rc",vt->odescr);DEND;
      return vt->odescr;
   }
   DSTART;DPRINT((TSD,"shortcutnum:       "));DVAR(TSD,"new vt->thespot",vt->thespot);DEND;
   DSTART;DPRINT((TSD,"shortcutnum:       "));DNUM(TSD,"rc",vt->thespot->num);DEND;

   return vt->thespot->num;
}

/*
 * Note: This is one of the most time-consuming routines. Be careful.
 */
void setshortcut( tsd_t *TSD, nodeptr thisptr, streng *value )
{
   variableptr vptr;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   assert( !value || value->len <= value->max );
   DSTART;DPRINT((TSD,"setshortcut:       "));DNAME(TSD,"thisptr->name",thisptr->name);
          DVALUE(TSD,", value",value);DEND;

   if ( ( vptr = thisptr->u.varbx ) != NULL )
   {
      if ( vptr->valid == vt->current_valid )
      {
         DSTART;DPRINT((TSD,"setshortcut:       "));DVAR(TSD,"valid vptr",vptr);
                DPRINT((TSD," on start"));DEND;
         SEEK_EXPOSED( vptr );
         if ( vptr->value )
            Free_stringTSD( vptr->value );
         if ( vptr->num )
         {
            FreeTSD( vptr->num->num );
            FreeTSD( vptr->num );
            vptr->num = 0;
         }
         vptr->flag = value ? VFLAG_STR : VFLAG_NONE;
         vptr->value = value;
         DSTART;DPRINT((TSD,"setshortcut:       "));DVAR(TSD,"valid vptr",vptr);
                DPRINT((TSD," on end"));DEND;
         return;
      }
      else
      {
         DSTART;DPRINT((TSD,"setshortcut:       "));DVAR(TSD,"INVALID vptr",vptr);
                DPRINT((TSD," on start"));DEND;
         if ( --vptr->hwired == 0 )
            if ( !vptr->valid )
            {
#ifdef TRACEMEM
               if ( vptr->prev )
                  vptr->prev->next = vptr->next;
               if ( vptr->next )
                  vptr->next->prev = vptr->prev;
               else
                  vt->first_invalid = vptr->prev;
#endif
               /*
                * See comment on REMOVE_ELEMENT why we just remove the body
                */
               FreeTSD( thisptr->u.varbx );
            }
         thisptr->u.varbx = NULL;
      }
   }

   setvalue( TSD, thisptr->name, value, -1 );
   if ( vt->thespot )
   {
      vt->thespot->hwired++;
      thisptr->u.varbx = vt->thespot;
   }
   DSTART;DPRINT((TSD,"setshortcut:       "));DVAR(TSD,"vt->thespot",vt->thespot);
          DPRINT((TSD," on end"));DEND;
}

/*
 * sets the varbox in this to the given value, creating it if it doesn't
 * exists. this->name is set to the string-representation of value; if
 * string_val is non-NULL, string_val is used instead. string_val must not
 * be used later.
 */
void setshortcutnum( tsd_t *TSD, nodeptr thisptr, num_descr *value,
                     streng *string_val )
{
   variableptr vptr;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   assert( value->size );

   DSTART;DPRINT((TSD,"setshortcutnum:    "));DNAME(TSD,"thisptr->name",thisptr->name);
          DNUM(TSD,", value",value);DEND;
   if ( ( vptr = thisptr->u.varbx ) != NULL )
   {
      if ( vptr->valid == vt->current_valid )
      {
         DSTART;DPRINT((TSD,"setshortcutnum:    "));DVAR(TSD,"valid vptr",vptr);
                DPRINT((TSD," on start"));DEND;
         SEEK_EXPOSED( vptr );
         if ( vptr->num )
         {
            FreeTSD( vptr->num->num );
            FreeTSD( vptr->num );
         }
         if ( vptr->value )
         {
            Free_stringTSD( vptr->value );
            vptr->value = NULL;
         }
         vptr->flag = value ? VFLAG_NUM : VFLAG_NONE;
         vptr->num = value;
         DSTART;DPRINT((TSD,"setshortcutnum:    "));DVAR(TSD,"valid vptr",vptr);
                DPRINT((TSD," on end"));DEND;
         return;
      }
      else
      {
         DSTART;DPRINT((TSD,"setshortcutnum:    "));DVAR(TSD,"INVALID vptr",vptr);
                DPRINT((TSD," on start"));DEND;
         if ( --vptr->hwired == 0 )
            if ( !vptr->valid )
            {
#ifdef TRACEMEM
               if ( vptr->prev )
                  vptr->prev->next = vptr->next;
               if ( vptr->next )
                  vptr->next->prev = vptr->prev;
               else
                  vt->first_invalid = vptr->prev;
#endif
               /*
                * See comment on REMOVE_ELEMENT why we just remove the body
                */
               FreeTSD( thisptr->u.varbx );
             }
         thisptr->u.varbx = NULL;
      }
   }

   if ( string_val == NULL )
      string_val = str_norm( TSD, value, NULL );
   setvalue( TSD, thisptr->name, string_val, -1 );
   if ( vt->thespot )
   {
      vt->thespot->hwired++;
      if ( value )
      {
         if ( vt->thespot->num )
         {
            FreeTSD( vt->thespot->num->num );
            FreeTSD( vt->thespot->num );
         }
         vt->thespot->num = value;
         vt->thespot->flag |= VFLAG_NUM;
      }
      thisptr->u.varbx = vt->thespot;
   }
   else
   {
      FreeTSD( value->num );
      FreeTSD( value );
   }
   DSTART;DPRINT((TSD,"setshortcutnum:    "));DVAR(TSD,"vt->thespot",vt->thespot);DEND;
   DSTART;DPRINT((TSD,"setshortcutnum:    "));DVAR(TSD,"thisptr->u.varbx",thisptr->u.varbx);
          DPRINT((TSD," on end"));DEND;
}

streng *fix_compound( tsd_t *TSD, nodeptr thisptr, streng *newstr )
{
   variableptr iptr,ptr;
   streng *value=NULL;
   streng *indeks;
   unsigned hhash, thash, hfh = 0;
   int rehash = 0, rehashIdx = 0;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   DPRINTF((TSD,"fix_compound:      ?"));
   value = NULL;
   hhash = (unsigned) -1;   /* Intentionally erroneous */

   assert( !newstr || newstr->len <= newstr->max );

   iptr = thisptr->u.varbx;
   if ( iptr )
   {
      if ( iptr->valid != vt->current_valid )
      {
         if ( ( --iptr->hwired == 0 ) && !iptr->valid )
         {
#ifdef TRACEMEM
            if ( iptr->prev )
               iptr->prev->next = iptr->next;
            if ( thisptr->u.varbx->next )
               iptr->next->prev = iptr->prev;
            else
               vt->first_invalid = iptr->prev;
#endif
            /*
             * See comment on REMOVE_ELEMENT why we just remove the body
             */
            FreeTSD( iptr );
         }
         iptr = thisptr->u.varbx = NULL;
      }
   }

   if ( !iptr )
   {
      hhash = hashfunc( vt, thisptr->name, 0, NULL, TSD->currlevel->vars->size );
      iptr = TSD->currlevel->vars->tbl[hhash];
      hfh = vt->fullhash;
      TSD->currlevel->vars->r++;
      /*
       * The stem's name is uppercased both in the parsing tree as in our
       * variable pool --> no need to Str_ccmp the elements.
       */
      SEEK_VAR_CMP( iptr, thisptr->name, hfh, TSD->currlevel->vars, rehash );
      SEEK_EXPOSED( iptr );

      if ( iptr )
      {
         thisptr->u.varbx = iptr;
         iptr->hwired++;
      }
      else if ( newstr && thisptr->p[0] )
      {
         if ( setvalue_simple( TSD, TSD->currlevel->vars, thisptr->name, NULL ) )
         {
            rehash = 0;
            hhash = hfh % TSD->currlevel->vars->size;
         }
         iptr = vt->thespot;
         iptr->index = make_hash_table( TSD, vt->initialHashTableLength );
         DPRINTF((TSD,"make_hash_table:   rc=%p",iptr->index));
      }

   }

   assert( thisptr->p[0] );
   indeks = fix_index( TSD, thisptr->p[0] );

   if ( vt->subst )
      tracecompound( TSD, thisptr->name, thisptr->name->len - 1, indeks, 'C' );

   if ( iptr )
   {
      thash = hashfunc( vt, indeks, 0, NULL, iptr->index->size );
      ptr = iptr->index->tbl[thash];
      iptr->index->w++;
      SEEK_VAR_CMP( ptr, indeks, vt->fullhash, iptr->index, rehashIdx );
      SEEK_EXPOSED( ptr );

      if ( newstr )
      {
         vt->foundflag = ptr != NULL;
         if ( vt->foundflag )
         {
            REPLACE_VALUE( newstr, ptr );
         }
         else
         {
            newbox( TSD, indeks, newstr, &iptr->index->tbl[thash], vt->fullhash );
            iptr->index->e++;
            NEW_HASHTABLE_CHECK( iptr->index, rehashIdx );
            iptr->index->tbl[thash]->stem = iptr;
         }
      }
      else
      {
         vt->foundflag = ptr && ( ptr->flag & VFLAG_BOTH );
         if ( ptr )
         {
            if ( ptr->flag & VFLAG_STR )
               value = ptr->value;
            else if ( ptr->flag & VFLAG_NUM )
            {
               expand_to_str( TSD, ptr );
               value = ptr->value;
            }
            else
               goto the_default;
         }
         else if ( iptr->flag & VFLAG_STR )
            value = iptr->value;
         else if ( iptr->flag & VFLAG_NUM )
         {
            expand_to_str( TSD, iptr );
            value = iptr->value;
         }
         else
            goto the_default ;

         tracevalue( TSD, value, 'V' );
      }
   }
   else
   {
      if ( newstr )
      {
         iptr = newbox( TSD, thisptr->name, NULL,
                        &TSD->currlevel->vars->tbl[hhash], hfh );
         TSD->currlevel->vars->e++;
         NEW_HASHTABLE_CHECK( TSD->currlevel->vars, rehash );
         iptr->index = make_hash_table( TSD, vt->initialHashTableLength );
         DPRINTF((TSD,"make_hash_table:   rc=%p",iptr->index));
         thash = hashfunc( vt, indeks, 0, NULL, iptr->index->size );
         iptr->index->w++;
         newbox( TSD, indeks, newstr, &iptr->index->tbl[thash], vt->fullhash );
         iptr->index->e++;
         NEW_HASHTABLE_CHECK( iptr->index, rehashIdx );
         iptr->index->tbl[thash]->stem = iptr;
      }
      else
      {
the_default:
         if ( vt->xvalue )
            Free_stringTSD( vt->xvalue );
         vt->xvalue = Str_makeTSD( thisptr->name->len + indeks->len );
         vt->xvalue = Str_catTSD( vt->xvalue, thisptr->name );
         vt->xvalue = Str_catTSD( vt->xvalue, indeks );
         tracevalue( TSD, vt->xvalue, 'L' );
         if ( !vt->ignore_novalue )
            condition_hook( TSD, SIGNAL_NOVALUE, 0, 0, -1, Str_dupTSD( vt->xvalue ), NULL ) ;
         value = vt->xvalue;
      }
   }

   if ( rehashIdx )
   {
      reorgHashtable( TSD, iptr->index );
   }
   if ( rehash )
   {
      reorgHashtable( TSD, TSD->currlevel->vars );
   }

   assert( !value || value->len <= value->max );
   return value;
}

/*
 * queries or sets the varbox in this depending on the presence of new. If
 * new is set, varbox in this is set to the given value, creating it if it
 * doesn't exists. this->name is set in this case to the string-representation
 * of value; if string_val is non-NULL, string_val is used instead.
 * string_val must not be used later.
 * The current or new value of the variable is returned in all cases, it may
 * be NULL.
 */
num_descr *fix_compoundnum( tsd_t *TSD, nodeptr thisptr, num_descr *newdescr,
                            streng *string_val )
{
   variableptr iptr, ptr;
   num_descr *value;
   streng *indeks;
   int rehash = 0, rehashIdx = 0;
   unsigned hhash, hfh = 0, thash;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;
   DPRINTF((TSD,"fix_compoundnum:   ?"));
   value = NULL;
   thash = hhash = 0x12345678;   /* Intentionally errorneous */

   iptr = thisptr->u.varbx;
   if ( iptr )
   {
      if ( iptr->valid != vt->current_valid )
      {
         if ( ( --iptr->hwired == 0 ) && !iptr->valid )
         {
#ifdef TRACEMEM
            if ( iptr->prev )
               iptr->prev->next = iptr->next;
            if ( thisptr->u.varbx->next )
               iptr->next->prev = iptr->prev;
            else
               vt->first_invalid = iptr->prev;
#endif
            /*
             * See comment on REMOVE_ELEMENT why we just remove the body
             */
            FreeTSD( iptr );
         }
         iptr = thisptr->u.varbx = NULL;
      }
   }

   if ( !iptr )
   {
      hhash = hashfunc( vt, thisptr->name, 0, NULL, TSD->currlevel->vars->size );
      iptr = TSD->currlevel->vars->tbl[hhash];
      hfh = vt->fullhash;
      TSD->currlevel->vars->r++;
      /*
       * The stem's name is uppercased both in the parsing tree as in our
       * variable pool --> no need to Str_ccmp the elements.
       */
      SEEK_VAR_CMP( iptr, thisptr->name, hfh, TSD->currlevel->vars, rehash );
      SEEK_EXPOSED( iptr );

      if ( iptr )
      {
         thisptr->u.varbx = iptr;
         iptr->hwired++;
      }
      else if ( newdescr && thisptr->p[0] )
      {
         if ( setvalue_simple( TSD, TSD->currlevel->vars, thisptr->name, NULL ) )
         {
            rehash = 0;
            hhash = hfh % TSD->currlevel->vars->size;
         }
         iptr = vt->thespot;
         iptr->index = make_hash_table( TSD, vt->initialHashTableLength );
         DPRINTF((TSD,"make_hash_table:   rc=%p",iptr->index));
      }
   }

   assert( thisptr->p[0] );
   indeks = fix_index( TSD, thisptr->p[0] );

   if ( vt->subst )
      tracecompound( TSD, thisptr->name, thisptr->name->len - 1, indeks, 'C' );

   if ( iptr )
   {
      thash = hashfunc( vt, indeks, 0, NULL, iptr->index->size );
      ptr = iptr->index->tbl[thash];
      iptr->index->w++;
      SEEK_VAR_CMP( ptr, indeks, vt->fullhash, iptr->index, rehashIdx );
      SEEK_EXPOSED( ptr );

      if ( newdescr )
      {
         vt->foundflag = ptr != NULL;
         if ( vt->foundflag )
         {
            REPLACE_NUMBER( newdescr, ptr );
         }
         else
         {
            newbox( TSD, indeks, NULL, &iptr->index->tbl[thash], vt->fullhash );
            iptr->index->e++;
            NEW_HASHTABLE_CHECK( iptr->index, rehashIdx );
            ptr = iptr->index->tbl[thash];
            ptr->stem = iptr;
            ptr->num = newdescr;
            ptr->flag = VFLAG_NUM;
         }
         if ( ptr->value != NULL )
            Free_stringTSD( ptr->value );

         if ( string_val != NULL )
            ptr->flag |= VFLAG_STR;
         else
            ptr->flag &= ~VFLAG_STR;
         ptr->value = string_val;
      }
      else
      {
         vt->foundflag = ptr && ( ptr->flag & VFLAG_BOTH );
         if ( ptr )
         {
            if ( ptr->flag & VFLAG_NUM )
            {
               value = ptr->num;
               tracenumber( TSD, value, 'V' );
            }
            else if ( ptr->flag & VFLAG_STR )
            {
               if ( ptr->num )
               {
                  FreeTSD( ptr->num->num );
                  FreeTSD( ptr->num );
               }
               ptr->num = is_a_descr( TSD, ptr->value );
               if ( ( value = ptr->num ) != NULL )
               {
                  tracevalue( TSD, ptr->value, 'V' );
                  ptr->flag |= VFLAG_NUM;
               }
            }
            else
               goto the_default;
         }
         else if ( iptr->flag & VFLAG_NUM )
         {
            value = iptr->num;
            tracenumber( TSD, value, 'V' );
         }
         else if ( iptr->flag & VFLAG_STR )
         {
            if ( iptr->num )
            {
               FreeTSD( iptr->num->num );
               FreeTSD( iptr->num );
            }
            iptr->num = is_a_descr( TSD, iptr->value );
            if ( ( value = iptr->num ) != NULL )
            {
               iptr->flag |= VFLAG_NUM;
               tracevalue( TSD, iptr->value, 'V' );
            }
         }
         else
            goto the_default;
      }

   }
   else
   {
      if ( newdescr )
      {
         /*
          * Happens on:
          * a) first access
          * b) left part of X_NASSIGN
          * c) thisptr->p[0] is NULL
          * according to this function, interprt.c and yaccsrc.y.
          *
          * We have to be a X_HEAD_SYMBOL. X_HEAD_SYMBOL are created by
          * create_head in yaccsrc.y only if thisptr->p[0] != NULL.
          *
          * --> This code will never be executed!
          * Reenable this only if you check the presence of stringval and
          * know what you do.
          * Comment last visited on 12.12.2003, this makes sense.
          * If no response by users are made, the else-part of "#if 1" should
          * be removed at the end of 2010.
          */
#if 1
         fprintf( stderr, "Regina internal error detected in %s, line %u.\n"
                          "Please, send an email to M.Hessling@qut.edu.au.\n",
                          __FILE__, __LINE__ );
      }
#else
         iptr = newbox( TSD, thisptr->name, NULL, &TSD->currlevel->vars[hhash],
                        hfh );
         TSD->currlevel->vars->e++;
         NEW_HASHTABLE_CHECK( TSD->currlevel->vars, rehash );
         iptr->index = make_hash_table( TSD, vt->initialHashTableLength );
         DPRINTF((TSD,"make_hash_table:   rc=%p",iptr->index));
         thash = hashfunc( vt, indeks, 0, NULL );
         iptr->index->r++;
         newbox( TSD, indeks, NULL, &iptr->index->tbl[thash], vt->fullhash );
         iptr->index->e++;
         NEW_HASHTABLE_CHECK( iptr->index, rehashIdx );
         ptr = iptr->index[thash];
         ptr->stem = iptr;
         ptr->num = newdescr;
         ptr->flag = VFLAG_NUM;
         if ( string_val != NULL )
            ptr->flag |= VFLAG_STR;
         ptr->value = string_val;
      }
      else
#endif
      {
the_default:
         tracecompound( TSD, thisptr->name, thisptr->name->len - 1, indeks, 'L' );
         return NULL;
      }
   }
   if ( rehashIdx )
   {
      reorgHashtable( TSD, iptr->index );
   }
   if ( rehash )
   {
      reorgHashtable( TSD, TSD->currlevel->vars );
   }

   return value;
}

/*
 * get_realbox returns either p or the realbox associated with p if it exists.
 * This function is NULL-pointer safe.
 */
static variableptr get_realbox( variableptr p )
{
   if ( p == NULL )
      return p;
   if ( p->realbox == NULL )
      return p;
   for ( p = p->realbox; p->realbox; p = p->realbox )
      ;
   return p;
}

/*
 * get_next_variable either initializes the iteration system (reset != 0) or
 * it returns the next variable of the variable's array. We have to do some
 * extra housekeeping to become reentrant in each call.
 *
 * NULL is returned in initializing or after all variables are processed.
 *
 * This function returns the variables of the current PROCEDURE frame only
 * and has a similar function dumpvars in debug.c.
 */
variableptr get_next_variable( tsd_t *TSD, int reset )
{
   variableptr retval;
   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"get_next_variable: ?"));
   if ( reset )
   {
      /*
       * Initializes the 4 loop variables and returns NULL.
       * rstem and rtail are the corresponding "realbox" shadows of pstem
       * and ptail. This fixes bug 681991.
       */
      vt->pstem = vt->ptail = NULL;
      vt->stemidx = vt->tailidx = 0;
      vt->rstem = vt->rtail = NULL;
      return NULL;
   }

   /*
    * We have to do some tricks to change the four nested loops (see dumpvars)
    * into a reentrant loop system. The loop itself is identical to that one
    * of dumpvars.
    */
   for ( ; ; )
   {
      /*
       * While the masterindex stemidx isn't out of range.
       */
      if ( vt->rstem )
      {
         /*
          * Something's left in the masterindex's iterator pstem.
          */
         if ( vt->rstem->index )
         {
            /*
             * This works as the outer loop pair, but for stem variables.
             */
            for ( ; ; )
            {
               if ( vt->rtail )
               {
                  retval = vt->rtail;
                  vt->ptail = vt->ptail->next;
                  vt->rtail = get_realbox( vt->ptail );
                  return retval;
               }

               if ( vt->tailidx >= vt->rstem->index->size )
                  break;

               vt->ptail = vt->rstem->index->tbl[vt->tailidx++];
               vt->rtail = get_realbox( vt->ptail );
            }
         }

         vt->ptail = vt->rtail = NULL;
         vt->tailidx = 0;

         retval = vt->rstem;
         vt->pstem = vt->pstem->next;
         vt->rstem = get_realbox( vt->pstem );
         return retval;
      }

      if ( vt->stemidx >= TSD->currlevel->vars->size )
         break;

      vt->pstem = TSD->currlevel->vars->tbl[vt->stemidx++];
      vt->rstem = get_realbox( vt->pstem );
      vt->ptail = NULL;
      vt->tailidx = 0;
   }

   return NULL;
}

#if 0
/* this was an attempt to mimic the behaviour of Object Rexx stem
 * assignment, but proved non-ANSI complient, so was dropped. I leave
 * it here so I can remember how to work with the variable pool ;-)
 */
void copy_stem( tsd_t *TSD, nodeptr dststem, cnodeptr srcstem )
{
Changed too much, never try to use it anyway! FGC
   /*
    * Drop dststem
    * set default value of dststem to default value of srcstem
    * for each valid stem of srcstem, set dststem value to src value
    */
   variableptr ptr;

   var_tsd_t *vt;

   vt = (var_tsd_t *)TSD->var_tsd;

   DPRINTF((TSD,"copy_stem:         ?"));
   drop_var( dststem->name );
   ptr = findsimple( TSD, srcstem->name );
   if ( ptr )
   {
      if ( ptr->value )
      {
         /*
          * The srcstem has a default value, so set the dststem's
          * default value to this...
          */
         setvalue_stem( TSD, dststem->name, ptr->value );
      }
      else
      {
         /*
          * The srcstem does not have a default value, so set the dststem's
          * default value to the name of the srcstem...
          */
         setvalue_stem( TSD, dststem->name, ptr->name );
      }
#if 0
      /*
       * THE following code copies all explicitly set variables in the srcstem
       * to the equivalent dststem compound values, but this is NOT the way
       * that the ANSI standard states the behaviour should be :-(
       *
       * Find each variable for srcstem, and set dststem equivalents...
       */
      if ( ptr->index )
      {
         for ( j = 0; j < HASHTABLENGTH; j++ )
         {
            if ( ( tptr = (ptr->index)[j] ) != NULL )
            {
               for ( ; tptr; tptr = tptr->next )
               {
                  if ( tptr->name )
                  {
                     newname = Str_makeTSD( Str_len( dststem->name ) + 1 + Str_len( tptr->name ) );
                     Str_ncpyTSD( newname, dststem->name, Str_len( dststem->name) );
                     Str_catTSD( newname, tptr->name );
                     if ( tptr->value )
                     {
                        setvalue_compound( TSD, newname, tptr->value );
                     }
                     else
                     {
                     /*
                      * If the srcstem compund variable was dropped,
                      * then to make the destination compund variable
                      * also "appear" to be dropped, call the following
                      * line, otherwise leave it excluded.
                      */
#if 0
                        setvalue_compound( TSD, newname, NULL );
#endif
                     }
                     Free_stringTSD( newname );
                  }
               }
            }
         }
      }
#endif
   }
   else
   {
      /*
       * The source stem doesn't exist, so set the default value of
       * dststem to the name of the srcstem.
       */
      setvalue_stem( TSD, dststem->name, srcstem->name );
   }
}
#endif


/*
 * set_reserved_value sets reserved values (those with a leading dot) and
 * their normal counterparts.
 * poolid is one of the POOL0_??? ids and one of var_str or val_int is
 * honoured depending on vflag, which may be either VFLAG_NONE, VFLAG_STR,
 * VFLAG_NUM.
 * We do our best to maximize the throughput here, but keep in mind that
 * a plain setvalue invoked by the external variable interface may interfere
 * with our values. So we can't hurry ahead the dot-variables any more.
 */
void set_reserved_value( tsd_t *TSD, int poolid, streng *val_str, int val_int,
                         int vflag )
{
   var_tsd_t *vt = (var_tsd_t *)TSD->var_tsd;
   int cv;

   assert( ( poolid > 0 ) && ( poolid < POOL0_CNT ) );

   if ( vflag == VFLAG_NONE )
   {
      drop_var_simple( TSD, vt->pool0, vt->pool0nodes[poolid][0].name );
      drop_var_simple( TSD, TSD->currlevel->vars,
                       vt->pool0nodes[poolid][1].name );
      return;
   }

   if ( vflag == VFLAG_NUM )
      val_str = int_to_streng( TSD, val_int );

   /*
    * We can prevent a variable-box-switch by setting the current_valid flag
    * to a const value. Because the value always uses pool0 we can safely set
    * the flag. Don't do it for non-dot variables.
    */
   cv = vt->current_valid;
   vt->current_valid = 1;
   setshortcut( TSD, &vt->pool0nodes[poolid][0], val_str );
   vt->current_valid = cv;

   if ( vt->pool0nodes[poolid][1].name )
   {
      if ( val_str )
         val_str = Str_dupTSD( val_str );
      setshortcut( TSD, &vt->pool0nodes[poolid][1], val_str );
   }
}
