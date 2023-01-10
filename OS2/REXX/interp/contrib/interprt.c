/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
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
#include <stdio.h>
#include <string.h>
#include <limits.h>
#ifndef VMS
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif

#define XOR(a,b) (( (a) && (!(b)) )||( (!(a)) && (b) ))

#ifdef WIN32
/* Asynchroneous scheduled in another thread: */
volatile int __regina_Win32CtrlCRaised = 0;
#endif

static const char default_action[SIGNALS] = { 1, 1, 0, 1, 1, 0 } ;
static const char default_ignore[SIGNALS] = { 1, 1, 0, 0, 1, 0 } ;

#define NSTACKELEMS 32 /* nstack elements will be allocated in this size */
typedef struct _nstackbox {
   struct _nstackbox *next;
   struct _nstackbox *prev;
   unsigned           used;
   unsigned           sum; /* To provide safe triggers, never use ptrs */
   nodeptr            elems[NSTACKELEMS];
} nstackbox;

typedef struct _stackelem {
#ifdef OPT_DO
   int                strmath; /* 0 if we can do binary arithmetic for this DO loop */
   /* DO ... FOR x: value of x */
   num_descr *        do_for_val;
   rx_64              do_for_val_num ;
#else
   int                number;
#endif
   int                incrdir ;
   num_descr *        increment ;
   /* DO ... TO x: value of x */
   num_descr *        stopval ;
#ifdef OPT_DO
   rx_64              stopval_num;
#endif
   nodeptr            thisptr ;
   cnodeptr           incr_node;
   struct _stackelem *prev ; /* needed for a look back */
} stackelem;

#define STACKELEMS 64 /* nstack elements will be allocated in this size */
typedef struct _stackbox {
   struct _stackbox *next;
   struct _stackbox *prev;
   unsigned          used;
   unsigned          sum; /* To provide safe triggers, never use ptrs */
   stackelem         elems[STACKELEMS];
} stackbox;

typedef struct { /* itp_tsd: static variables of this module (thread-safe) */
   nstackbox      nbox;
   nstackbox     *nbox_top;
   stackbox       sbox;
   stackbox      *stk_top;
   unsigned long  options;
   int            opts_set;
} itp_tsd_t; /* thread-specific but only needed by this module. see
              * init_spec_vars
              */



static void expose_indir( tsd_t *TSD, const streng *list ) ;

#ifdef TRACEMEM
static void mark_spec_vars(const tsd_t *TSD)
{
   itp_tsd_t *it;

   it = (itp_tsd_t *) TSD->itp_tsd;
}
#endif /* TRACEMEM */


/*
 * The function returns 1 on success, 0 if memory is short.
 */
int init_spec_vars( tsd_t *TSD )
{
   itp_tsd_t *it;

   if (TSD->itp_tsd != NULL)
      return(1);

   if ( ( TSD->itp_tsd = MallocTSD( sizeof(itp_tsd_t) ) ) == NULL )
      return(0);
   it = (itp_tsd_t *)TSD->itp_tsd;
   memset(it,0,sizeof(itp_tsd_t));

   it->nbox_top = &it->nbox;
   it->stk_top = &it->sbox;

#ifdef TRACEMEM
   regmarker( TSD, mark_spec_vars ) ;
#endif
   return(1);
}


void update_envirs( const tsd_t *TSD, proclevel level )
{
   proclevel lptr=NULL ;

   if (!level->environment)
   {
      for (lptr=level->prev; lptr; lptr=lptr->prev)
      {
         if (lptr->environment)
         {
            level->environment = Str_dupTSD(lptr->environment) ;
            break ;
         }
      }
   }
   if (!level->prev_env)
   {
      for (lptr=level->prev; lptr; lptr=lptr->prev)
      {
         if (lptr->prev_env)
         {
            level->prev_env = Str_dupTSD(lptr->prev_env) ;
            break ;
         }
      }
   }

   assert( level->environment ) ;
   assert( level->prev_env ) ;
}

/* nstackpush pushes the arg pnode on the nstack. (copy, then increment)
 */
static void nstackpush(const tsd_t *TSD,nodeptr pnode)
{
   itp_tsd_t *it;
   nstackbox *ns;

   it = (itp_tsd_t *)TSD->itp_tsd;
   ns = it->nbox_top;
   ns->elems[ns->used++] = pnode;
   if (ns->used >= NSTACKELEMS)
   {
      if (ns->next == NULL)
      {
         ns->next = (nstackbox *)MallocTSD(sizeof(nstackbox));
         ns->next->prev = ns;
         ns->next->next = NULL;
         ns->next->used = 0;
         ns->next->sum = ns->sum + NSTACKELEMS;
      }
      assert( ns->next->used == 0 ) ; /* be sure to have an empty block */
      it->nbox_top = ns->next;
   }
}

/* nstackpop pops an element from the nstack. (decrement, then copy)
 * The return is the saved value formerly saved by a call to nstackpush.
 * nstackpush/nstackpop calls may be nested.
 * The return value is NULL in case of an empty stack.
 * We use a delayed cleanup (one free nstackbox is available while popping).
 */
static nodeptr nstackpop(const tsd_t *TSD)
{
   itp_tsd_t *it;
   nstackbox *ns;

   it = (itp_tsd_t *)TSD->itp_tsd;
   ns = it->nbox_top;
   if (ns->used == 0)
   {
      /* For a delayed deletion preserve this box and delete the next one */
      if (ns->next)
      {
         FreeTSD(ns->next);
         ns->next = NULL;
      }
      assert(ns->prev);
      if (!ns->prev)
      {
         return(NULL);
      }
      it->nbox_top = ns = ns->prev;
   }
   ns->used--;
   return(ns->elems[ns->used]);
}

/* nstacktrigger returns a stack descriptor which allow the caller to return
 * to the current state by using nstackcleanup() later.
 */
static unsigned nstacktrigger(const tsd_t *TSD)
{
   itp_tsd_t *it;
   nstackbox *ns;

   it = (itp_tsd_t *)TSD->itp_tsd;
   ns = it->nbox_top;
   return(ns->sum + ns->used);
}

/* nstackcleanup cleans up the stack until either the trigger from a
 * nstacktrigger() is reached or the node encounters, whatever comes first.
 * In the first case the state is as during the call to nstacktrigger().
 * In the second case the matching node is NOT popped. To allow a search for
 * NULL the matching pointer is indexed. Giving NULL means don't use the
 * matching algorithm. Give
 * "cnodeptr m=NULL; nstackcleanup(TSD,?,&m);" to match a NULL pointer.
 * node may be NULL.
 */
static void nstackcleanup(const tsd_t *TSD,
                          unsigned trigger,
                          const nodeptr *match)
{
   itp_tsd_t *it;
   nstackbox *ns;
   cnodeptr m = NULL; /* Keep the compiler happy */

   it = (itp_tsd_t *)TSD->itp_tsd;
   ns = it->nbox_top;
   if (match)
      m = *match;
   while (trigger < ns->sum) /* The complete block may be killed! */
   {
      if (match)
      {
         while (ns->used)
         {
            if (ns->elems[--ns->used] == m)
            {
               ns->used++;
               return;
            }
         }
      }
      else
      {
         ns->used = 0 ;
      }
      /* For a delayed deletion preserve this box and delete the next one */
      if (ns->next)
      {
         FreeTSD(ns->next);
         ns->next = NULL;
      }
      assert(ns->prev);
      if (!ns->prev)
      {
         ns->used = 0;
         return;
      }
      it->nbox_top = ns = ns->prev;
   }

   /* The trigger is within the current box. Do an alignment to force
    * trigger to be used in conjunction with ns->used.
    */
   trigger -= ns->sum;
   if (trigger >= ns->used) /* Be safety. Imagine a wild trigger! */
      return;

   if (!match) /* Things may be simple */
   {
      ns->used = trigger;
      return;
   }

   while (trigger != ns->used)
      if (ns->elems[--ns->used] == m)
         {
            ns->used++;
            return;
         }
   ns->used = trigger;
}

/* stackpush pushes the contents of the arg sbox on the stack. (copy, then
 * increment)
 */
static void stackpush(const tsd_t *TSD,const stackelem *sbox)
{
   itp_tsd_t *it;
   stackbox *sb;

   it = (itp_tsd_t *)TSD->itp_tsd;
   sb = it->stk_top;
   sb->elems[sb->used] = *sbox;
   if (sb->used)
      sb->elems[sb->used].prev = sb->elems + sb->used - 1;
   else if (sb->prev)
      sb->elems[0].prev = sb->prev->elems + STACKELEMS - 1;
   else
      sb->elems[0].prev = NULL;
   sb->used++;

   if (sb->used >= STACKELEMS)
   {
      if (sb->next == NULL)
      {
         sb->next = (stackbox *)MallocTSD(sizeof(stackbox));
         sb->next->prev = sb;
         sb->next->next = NULL;
         sb->next->sum = sb->sum + STACKELEMS; /* const to each block */
      }
      assert( sb->next->used == 0 ) ; /* be sure to have an empty block */
      it->stk_top = sb->next;
   }
}

/* stackpop pops an element from the stack. (decrement, then copy)
 * The return is the saved value formerly saved by a call to stackpush.
 * stackpush/stackpop calls may be nested.
 * The return value is filled with 0 in case of an empty stack.
 * We use a delayed cleanup (one free stackbox is available while popping).
 */
static stackelem stackpop(const tsd_t *TSD)
{
   itp_tsd_t *it;
   stackbox *sb;
   stackelem zero;

   it = (itp_tsd_t *)TSD->itp_tsd;
   sb = it->stk_top;
   if (sb->used == 0)
   {
      /* For a delayed deletion preserve this box and delete the next one */
      if (sb->next)
      {
         FreeTSD(sb->next);
         sb->next = NULL;
      }
      assert(sb->prev);
      if (!sb->prev)
      {
         memset(&zero,0,sizeof(zero));
         return(zero);
      }
      it->stk_top = sb = sb->prev;
   }
   sb->used--;
   return(sb->elems[sb->used]);
}

/* stacktrigger returns a stack descriptor which allow the caller to return
 * to the current state by using stackcleanup() later. See also stacktop().
 */
static unsigned stacktrigger(const tsd_t *TSD)
{
   itp_tsd_t *it;
   stackbox *sb;

   it = (itp_tsd_t *)TSD->itp_tsd;
   sb = it->stk_top;
   return(sb->sum + sb->used);
}

/* stacktop returns a pointer to the top element of the stack. This may be
 * useful to manipulate stack elements or have a look back. Be careful with
 * the stack and don't use elements which were not pushed by the current
 * incarnation of interpret(). See also stacktrigger().
 * NULL is returned if the stack is empty.
 */
static stackelem * stacktop(const tsd_t *TSD)
{
   itp_tsd_t *it;
   stackbox *sb;

   it = (itp_tsd_t *)TSD->itp_tsd;
   sb = it->stk_top;
   if (sb->used)
      return(sb->elems + sb->used - 1);
   if (sb->prev)
      return(sb->prev->elems + STACKELEMS - 1);
   return(NULL);
}

/* stack_destroyelement kill the increment and stopval values of a stack
 * elements if they exist. The values are reset to zero after the deletion.
 */
static void stack_destroyelement(const tsd_t *TSD,stackelem *se)
{
   if (se->stopval)
   {
      free_a_descr(TSD,se->stopval);
      se->stopval = NULL;
   }
#ifdef OPT_DO
   if (se->do_for_val)
   {
      free_a_descr(TSD,se->do_for_val);
      se->do_for_val = NULL;
   }
#endif
   if (se->increment)
   {
      free_a_descr(TSD,se->increment);
      se->increment = NULL;
   }
}


/* stackcleanup cleans up the stack until the trigger from a
 * stacktrigger() is reached. After the call the stack is in the same state as
 * during the call to stacktrigger.
 * Warning: The elements increment and stopval will be deleted for each
 *          deleted stack elements if they exist. Do a stacktop() and use the
 *          prev value of each entry for the appropriate count do set the
 *          values to NULL if you don't want this.
 */
static void stackcleanup(const tsd_t *TSD,unsigned trigger)
{
   itp_tsd_t *it;
   stackbox *sb;
   unsigned tokill = stacktrigger(TSD);

   if (tokill <= trigger)
      return;

   tokill -= trigger;
   it = (itp_tsd_t *)TSD->itp_tsd;
   sb = it->stk_top;
   while (tokill--)
   {
      if (sb->used == 0)
      {
         /* For a delayed deletion preserve this box and delete the next one */
         if (sb->next)
         {
            FreeTSD(sb->next);
            sb->next = NULL;
         }
         assert(sb->prev);
         if (!sb->prev)
            return;
         it->stk_top = sb = sb->prev;
      }
      sb->used--;
      stack_destroyelement(TSD,sb->elems + sb->used);
   }
}

void SaveInterpreterStatus(const tsd_t *TSD,unsigned *state)
{
   assert(IPRT_BUFSIZE >= 2);
   state[0] = nstacktrigger(TSD);
   state[1] = stacktrigger(TSD);
}

void RestoreInterpreterStatus(const tsd_t *TSD,const unsigned *state)
{
   nstackcleanup(TSD,state[0],NULL);
   stackcleanup(TSD,state[1]);
}

streng *CallInternalFunction( tsd_t *TSD, nodeptr node, nodeptr thisptr,
                              paramboxptr args )
{
   int stackmark;
   streng *result;
   proclevel oldlevel;
   nodeptr savecurrentnode;

   oldlevel = TSD->currlevel;
   TSD->currlevel = newlevel( TSD, TSD->currlevel );
   TSD->currlevel->args = args;
   stackmark = pushcallstack( TSD, thisptr );

   savecurrentnode = TSD->currentnode;
   result = interpret( TSD, node );
   TSD->currentnode = savecurrentnode;

   popcallstack( TSD, stackmark );
   removelevel( TSD, TSD->currlevel );
   TSD->currlevel = oldlevel;
   TSD->currlevel->next = NULL;
   TSD->trace_stat = TSD->currlevel->tracestat;

   return result;
}

streng *interpret(tsd_t * volatile TSD, treenode * volatile thisptr)
{
   int i ;
   int stackmark ;
   proclevel oldlevel ;
   treenode *entry=NULL ;
   int no_next_interactive=0 ;
   stackelem s;
   volatile unsigned stktrigger ;
   volatile unsigned nstktrigger ;
   nodeptr innerloop=NULL ;
   num_descr *doiterdescr=NULL ;
   volatile nodeptr secure_this ;
   tsd_t * volatile secure_TSD ;
   itp_tsd_t * volatile it;

   it = (itp_tsd_t *)TSD->itp_tsd;

   nstktrigger = nstacktrigger(TSD);
   stktrigger = stacktrigger(TSD);

   secure_TSD = TSD; /* vars used until here */
   secure_this = thisptr;

   if ( TSD->currlevel->signal_continue == NULL )
   {
      TSD->currlevel->signal_continue = (jmp_buf *)MallocTSD( sizeof( jmp_buf ) );

      assert( !TSD->in_protected );
      if ( setjmp( *TSD->currlevel->signal_continue ) )
      {
         /* A signal arrived and a longjmp from anywhere jumps here.
          * We can't believe in anything and have to rebuild it from
          * scratch or volatile pointers. Even an unoptimized compiler
          * may have optimized the access to values of any kind.
          * We have to do the full reinitialization.
          * prevents bugs like 592393
          */
         thisptr = secure_this ;
         TSD = secure_TSD ;
         it = (itp_tsd_t *)TSD->itp_tsd ;

         doiterdescr = NULL ;
         innerloop = NULL ;
         memset(&s,0,sizeof(s));

         nstackcleanup(TSD,nstktrigger,NULL);
         stackcleanup(TSD,stktrigger);
         no_next_interactive = 0 ;

         goto fakerecurse ;
      }
   }
   memset(&s,0,sizeof(s));
   no_next_interactive = 0 ;
   doiterdescr = NULL ;
   innerloop = NULL ;

reinterpret:
#ifdef WIN32
   /*
    * Braindamaged Win32 systems raise ^C in a different thread. We catch the
    * global flag set the thread's own halt-flag.
    */
   if ( __regina_Win32CtrlCRaised )
   {
      TSD->HaltRaised = __regina_Win32CtrlCRaised;
      __regina_Win32CtrlCRaised = 0;
   }
#endif
   if ( TSD->HaltRaised )
      halt_raised( TSD );

   if (thisptr==NULL)
      goto fakereturn ;

   secure_this = thisptr;

   TSD->currentnode = thisptr ;
   if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
   {
      if (thisptr->type != X_DO)  /* let do-stats trace themselves */
         traceline( TSD, thisptr, TSD->trace_stat, 0 );
   }

   if (thisptr->now)
   {
      FreeTSD(thisptr->now);
      thisptr->now = NULL;
   }

   thisptr->o.called = 0;

   switch ( /*(unsigned char)*/ (thisptr->type) )
   {
      case X_PROGRAM:
      case X_STATS:

      case X_WHENS:
      case X_OTHERWISE:
         thisptr = thisptr->p[0] ;
         goto reinterpret ;


      case 0:
      case 255:
      case X_DO:
      {
         streng *tmpstr,*tmpkill=NULL;

         if (innerloop==thisptr)
         {
            assert( thisptr->p[3] ) ;
            if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
            {
               traceline( TSD, thisptr->p[3], TSD->trace_stat, -1 );
               traceline( TSD, thisptr, TSD->trace_stat, -1 );
            }
            goto one ;
         }
         else
         {
            if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
               traceline( TSD, thisptr, TSD->trace_stat, 0 );
         }

         if (!((thisptr->p[0])||(thisptr->p[1])))
         {
            nstackpush(TSD,thisptr->next);
            thisptr = thisptr->p[2] ;
            goto fakerecurse ;
         }

         nstackpush(TSD,thisptr->next); /* for use with leave */

         if (innerloop)
         {
            s.thisptr = innerloop;
            stackpush(TSD,&s);
         }

         s.incr_node = NULL;
#ifdef OPT_DO
         s.increment = s.do_for_val = s.stopval = doiterdescr = NULL ;
         s.do_for_val_num = -1 ;
         s.strmath = 0; /* do numeric calculations by default */
#else
         s.increment = s.stopval = doiterdescr = NULL ;
         s.number = -1;
#endif
         s.incrdir = 1 ;
         tmpstr = NULL ;
         /*
          * If we have a named variable as the iterator, get itl its in tmpstr
          */
         if ((thisptr->p[0])&&(thisptr->p[0]->name))
            tmpstr = evaluate( TSD, thisptr->p[0]->p[0], &tmpkill );
         /*
          * For each of the 4 types of options for the DO command that involve evaluation of a number
          * at the start of the loop, evaluate the starting value of that expression
          */
         for (i=1;i<4;i++)
         {
            if ((thisptr->p[0])&&(thisptr->p[0]->p[i]))
            {
               nodeptr tmpptr ;
               switch( thisptr->p[0]->p[i]->type )
               {
                  case X_DO_TO:
                  {
#ifdef OPT_DO
                     int error ;
                     rx_64 iptr;
                     streng *chptr,*chkill;
                     num_descr *tmpnum;
#endif
                     /* DO ... TO x: s.stopval is the value of x; x evaluates to any decimal number */
                     tmpptr = thisptr->p[0]->p[i]->p[0] ;
#ifdef OPT_DO
                     chptr = evaluate(TSD, tmpptr, &chkill );
                     if ( !myiswnumber( TSD, chptr, &s.stopval, 1 ) )
                        exiterror( ERR_INVALID_INTEGER, (thisptr->p[0]->p[i]->type==X_DO_EXPR) ? 2 : 3, chptr->value );
                     s.stopval_num = streng_to_rx64(TSD, chptr, &error);
                     if ( error )
                        s.strmath = 1; /* have to use string math for this loop */
                     if ( chkill )
                        Free_stringTSD( chkill );
#else
                     s.stopval = calcul( TSD, tmpptr, NULL, SIDE_LEFT, X_DO_TO ) ;
#endif
                     break ;
                  }
                  case X_DO_BY:
                     /* DO ... BY x: s.increment is the value of x; x evaluates to any decimal number, s.incrdir is the "direction", +ve or -ve */
                     s.incr_node = thisptr->p[0]->p[i]->p[0] ;
                     tmpptr = thisptr->p[0]->p[i]->p[0] ;
                     s.increment = calcul( TSD, tmpptr, NULL, SIDE_LEFT, X_DO_BY ) ;
                     s.incrdir = descr_sign( s.increment ) ;
/*
fprintf(stderr,"%s %d: direction: %d increment %s neg %d exp %d\n",__FILE__,__LINE__, s.incrdir,
s.increment->num,
s.increment->negative ,
s.increment->exp
);
*/
                     break ;

                  case X_DO_FOR:
                  case X_DO_EXPR:
                  {
                     /* DO x or DO .. FOR x: s.do_for_val_num is the value of x; x evaluates to any WHOLE number */
#ifdef OPT_DO
                     int error ;
                     rx_64 iptr;
#else
                     int iptr, error ;
#endif
                     streng *chptr,*chkill;
#ifdef OPT_DO
                     num_descr *tmpnum;
#endif

                     tmpptr = thisptr->p[0]->p[i]->p[0] ;
                     chptr = evaluate(TSD, tmpptr, &chkill );
#ifdef OPT_DO
                     if ( !myiswnumber( TSD, chptr, &tmpnum, 1 )
                     ||  tmpnum->negative )
                        exiterror( ERR_INVALID_INTEGER, (thisptr->p[0]->p[i]->type==X_DO_EXPR) ? 2 : 3, chptr->value );

                     s.do_for_val = calcul(TSD,tmpptr,NULL) ;
                     iptr = streng_to_rx64(TSD, chptr, &error);
#else
                     iptr = streng_to_int(TSD, chptr, &error);
#endif
                     if ( error )
#ifdef OPT_DO
                        s.strmath = 1; /* have to use string math for this loop */
                     s.do_for_val_num = iptr ;
#else
                        exiterror( ERR_INVALID_INTEGER, (thisptr->p[0]->p[i]->type==X_DO_EXPR) ? 2 : 3, chptr->value );
                     if ( iptr < 0 )
                        exiterror( ERR_INVALID_RESULT, 0 );
                     s.number = iptr;
#endif
                     if ( chkill )
                        Free_stringTSD( chkill );
                     break ;
                  }
               }
            }
         }
         if ( tmpstr )
         {
            /*
             * Normalise the iterator for the DO loop; must be a number.
             */
            setshortcut( TSD, thisptr->p[0], str_normalize( TSD, tmpstr ) );
            doiterdescr = shortcutnum( TSD, thisptr->p[0] );
            if ( tmpkill )
               Free_stringTSD( tmpkill );
/*
fprintf(stderr,"%s %d: After normalise: iter %s neg %d exp %d\n",__FILE__,__LINE__,
doiterdescr->num,
doiterdescr->negative ,
doiterdescr->exp
);
*/
         }
#ifdef OPT_DO
fprintf(stderr,"%s %d: Using %s arithmetic for DO\n",__FILE__,__LINE__,(s.strmath) ? "string" : "numeric" );
#endif
         if (TSD->systeminfo->interactive)
         {
            if (intertrace(TSD))
            {
               nstackpop(TSD);
               if (s.increment)
               {
                  free_a_descr( TSD, s.increment ) ;
                  s.increment = NULL ;
               }
               if (s.stopval)
               {
                  free_a_descr( TSD, s.stopval ) ;
                  s.stopval = NULL ;
               }
#ifdef OPT_DO
               if (s.do_for_val)
               {
                  free_a_descr( TSD, s.do_for_val ) ;
                  s.do_for_val = NULL ;
               }
#endif
               goto fakerecurse ;
            }
         }
startloop:
         if (thisptr->p[0])
         {
            /*
             * If we have a TO value to terminate the loop, check if the value of the iterator (doiterdescr)
             * ???
             */
            if (s.stopval)
            {
               int tsign ;

               tsign = string_test( TSD, doiterdescr, s.stopval ) ;
/*
fprintf(stderr,"%s %d: tsign %d\n",__FILE__,__LINE__,tsign);
*/
               if (!(tsign ^ s.incrdir))
                  goto endloop ;
/*
fprintf(stderr,"%s %d\n",__FILE__,__LINE__);
*/
            }

            /*
             * If we have a FOR value to terminate the loop, check if it has decremented to less than
             * or equal to zero
             */
#ifdef OPT_DO
            if ((s.do_for_val_num>=0) && (s.do_for_val_num--<=0))
#else
            if ((s.number>=0) && (s.number--<=0))
#endif
               goto endloop ;

         }
         /*
          * If there is WHILE clause, check if the expression is false before execution of the DO block
          */
         if ((thisptr->p[1])&&((thisptr->p[1]->type)==X_WHILE))
         {
            if (!isboolean(TSD,thisptr->p[1]->p[0],3, NULL))
               goto endloop ;
         }
         /*
          * Execute the code in the DO block
          */
         if (thisptr->p[2])
         {
            nstackpush(TSD,thisptr);
            pushcallstack(TSD,NULL) ;

            innerloop = thisptr ;
            thisptr = thisptr->p[2] ;
            goto fakerecurse ;

one:
            popcallstack(TSD,-1) ;
         }
         /*
          * If there is an UNTIL clause, check if the expression is false after execution of the DO block
          */
         if ((thisptr->p[1])&&((thisptr->p[1]->type)==X_UNTIL))
         {
            if (isboolean(TSD,thisptr->p[1]->p[0],4, NULL))
               goto endloop ;
         }

         if ((thisptr->p[0])&&(thisptr->p[0]->name))
         {
            doiterdescr = shortcutnum( TSD, thisptr->p[0] ) ;
            /*
             * Check if we still have a valid number. If not
             * exit with arithmetic error.
             */
            if (!doiterdescr)
               exiterror( ERR_BAD_ARITHMETIC, 0 )  ;

            /*
             * Increment our loop iterator
             */
            if (s.increment)
            {
               string_add( TSD, doiterdescr, s.increment, doiterdescr, thisptr->p[0], s.incr_node ) ;
/*
fprintf(stderr,"%s %d: After add: iter %s neg %d exp %d\n",__FILE__,__LINE__,
doiterdescr->num,
doiterdescr->negative ,
doiterdescr->exp
);
*/
               /* fixes bug 1109729: */
               str_round( doiterdescr, TSD->currlevel->currnumsize ) ;
/*
fprintf(stderr,"%s %d: After round: iter %s neg %d exp %d\n",__FILE__,__LINE__,
doiterdescr->num,
doiterdescr->negative ,
doiterdescr->exp
);
*/
            }
            else
               string_incr( TSD, doiterdescr, thisptr->p[0] ) ;

            if (thisptr->p[0]->u.varbx)
            {
               thisptr->p[0]->u.varbx->num = doiterdescr ;
               thisptr->p[0]->u.varbx->flag = VFLAG_NUM ;
               if ( TSD->trace_stat == 'I' )
                  tracenumber( TSD, doiterdescr, 'V');
            }
            else
               setshortcut( TSD, thisptr->p[0], str_norm( TSD, doiterdescr, NULL )) ;
         }

         if (TSD->nextsig)
            goto fakerecurse ;

         /*
          * Check for ^C before iterating. Fixes bug 882878.
          */
#ifdef WIN32
         if ( __regina_Win32CtrlCRaised )
         {
            TSD->HaltRaised = __regina_Win32CtrlCRaised;
            __regina_Win32CtrlCRaised = 0;
         }
#endif
         if ( TSD->HaltRaised )
            goto fakerecurse ;

         goto startloop ;

endloop: if (s.increment)
         {
            free_a_descr( TSD, s.increment ) ;
            s.increment = NULL ;
         }
         if (s.stopval)
         {
            free_a_descr( TSD, s.stopval ) ;
            s.stopval = NULL ;
         }
#ifdef OPT_DO
         if (s.do_for_val)
         {
            free_a_descr( TSD, s.do_for_val ) ;
            s.do_for_val = NULL ;
         }
#endif
         no_next_interactive = 1 ;
         nstackpop(TSD);

         if (stacktrigger(TSD) > stktrigger)
         {
            s = stackpop(TSD);
            innerloop = s.thisptr;
         }
         else
            innerloop = NULL ;

         break ;
      }
      case X_IF:
      {
         treenode *othis = thisptr, *n;
         int retval = isboolean( TSD, thisptr->p[0], 1, NULL );

         if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
         {
            n = thisptr->p[0]->next;
            while ( n != NULL ) {
               traceline( TSD, n, TSD->trace_stat, 0 );
               n = n->next;
            }
         }

         nstackpush(TSD,thisptr->next);
         thisptr = thisptr->p[retval ? 1 : 2];
         if (TSD->systeminfo->interactive)
         {
            if (intertrace(TSD))
            {
               thisptr = othis ;
            }
         }

         goto fakerecurse ;
      }
      case X_NASSIGN:
      {
         num_descr *ntmp;
         streng *preferred_str;
         int type;

         ntmp = calcul( TSD, thisptr->p[1], NULL, SIDE_LEFT, X_NASSIGN );
         assert( ntmp->size );

         type = thisptr->p[1]->type;
         if ( ( type == X_STRING ) || ( type == X_CON_SYMBOL ) )
            preferred_str = Str_dupTSD( thisptr->p[1]->name );
         else
            preferred_str = NULL;

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, preferred_str );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, preferred_str );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_ASSIGN:
      {
/* This is a CMS-ism; CMS allows the expression in an assignment to
 * be omitted, while TRL does _not_. If a CMS mode is implemented, the
 * code below should be changed to allow p[0] to be null only iff
 * CMS mode is active.
 */
         streng *value ;

         value = thisptr->p[1] ? evaluate(TSD,thisptr->p[1],NULL) : nullstringptr() ;
         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
            fix_compound( TSD, thisptr->p[0], value ) ;
         else
            setshortcut( TSD, thisptr->p[0], value ) ;
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            tracevalue( TSD, value, '=' );
         }
         break ;
      }

      case X_PLUSASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "+=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_PLUSASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_MINUSASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "-=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_MINUSASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_MULTASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "*=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_MULTASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_DIVASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "/=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_DIVASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_INTDIVASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "%=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_INTDIVASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_MODULUSASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "//=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_MODULUSASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_ORASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "|=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_ORASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_XORASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "&&=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_XORASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_ANDASSIGN:
      {
         num_descr *ntmp;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 4, "&=" )  ;

         ntmp = calcul( TSD, thisptr, NULL, SIDE_LEFT, X_ANDASSIGN );
         assert( ntmp->size );

         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         else
         {
            setshortcutnum( TSD, thisptr->p[0], ntmp, NULL );
         }
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            streng *ptr = str_norm( TSD, ntmp, NULL );
            tracevalue( TSD, ptr, '=' );
            FreeTSD( ptr );
         }
         break ;
      }

      case X_CONCATASSIGN:
      {
         streng *value ;

         value = evaluate( TSD, thisptr, NULL ) ;
         if (thisptr->p[0]->type==X_HEAD_SYMBOL)
            fix_compound( TSD, thisptr->p[0], value ) ;
         else
            setshortcut( TSD, thisptr->p[0], value ) ;
         /* trace the result of assignment */
         if ( TSD->trace_stat == 'R' )
         {
            tracevalue( TSD, value, '=' );
         }
         break ;
      }

      case X_IPRET:
      {
         streng *retval, *tptr = evaluate(TSD,thisptr->p[0],NULL) ;
         retval = dointerpret( TSD, tptr ) ;
         if (retval != NULL) /* we interpreted a RETURN WITH a value */
         {
            stackelem *top = stacktop(TSD);
            unsigned i1;
            for (i1 = stacktrigger(TSD);i1 > stktrigger;i1--,top = top->prev)
            {
               if (top->increment == s.increment)
                  s.increment = NULL;
               if (top->stopval == s.stopval)
                  s.stopval = NULL;
#ifdef OPT_DO
               if (top->do_for_val == s.do_for_val)
                  s.do_for_val = NULL;
#endif
            }

            stackcleanup(TSD,stktrigger);
            nstackcleanup(TSD,nstktrigger,NULL);
            return( retval ) ;
         }
         break ;
      }

      case X_NO_OTHERWISE:
         exiterror( ERR_WHEN_EXPECTED, 0 )  ;
         break ;

      case X_SELECT:
         nstackpush(TSD,thisptr->next);
         nstackpush(TSD,thisptr->p[1]);
         thisptr = thisptr->p[0] ;
         goto fakerecurse ;

      case X_WHEN:
      {
         int retval = isboolean( TSD, thisptr->p[0], 2, NULL );
         nodeptr n;
         if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
         {
            n = thisptr->p[0]->next;
            while ( n != NULL ) {
               traceline( TSD, n, TSD->trace_stat, 0 );
               n = n->next;
            }
         }
         if ( retval )
         {
            nstackpop(TSD); /* kill the OTHERWISE on the stack */
            thisptr = thisptr->p[1] ;
            goto fakerecurse ;
         }
         break ;
      }

      case X_SAY:
      {
         int ok=HOOK_GO_ON ;
         streng *stringen,*kill=NULL;

         if (thisptr->p[0])
            stringen = evaluate( TSD, thisptr->p[0], &kill );
         else
            stringen = NULL ;

         if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_STDOUT))
            ok = hookup_output( TSD, HOOK_STDOUT, stringen ) ;

         if (ok==HOOK_GO_ON)
         {
            if (stringen)
            {
#ifdef WIN32
               /*
                * Due to a bug in Windows that gives an error
                * if you try to write too many characters to the
                * console in one attempt, split the output
                * up into chunks.
                * Bug: 1455211
                */
               char *buf = stringen->value;
               long done,chunk;
               long todo = Str_len(stringen);
               do
               {
                  chunk = min( todo, 0x8000);
                  done = fwrite( buf, chunk, 1, stdout ) ;
                  buf += chunk ;
                  todo -= chunk ;
               } while ( todo > 0 ) ;
#else
               fwrite( stringen->value, Str_len(stringen), 1, stdout ) ;
#endif
            }
#if defined(DOS) || defined(OS2) || defined(WIN32)
            /*
             * stdout is open in binary mode, so we need to add the
             * extra CR to the end of the line.
             */
            fputc( REGINA_CR, stdout ) ;
#endif
            fputc( REGINA_EOL, stdout ) ;
            fflush( stdout ) ;
         }

         if (stringen && kill)
            Free_stringTSD( kill );

         break ;
      }

      case X_TRACE:
      {
         streng *tptr ;

         if (!TSD->systeminfo->trace_override)
         {
            if (thisptr->name)
            {
               set_trace( TSD, thisptr->name ) ;
            }
            else if (thisptr->p[0])
            {
               set_trace( TSD, evaluate(TSD,thisptr->p[0], &tptr ) );
               if ( tptr )
                  Free_stringTSD( tptr ) ;
            }
            else
            {
               /* default setting for TRACE with no arguments is "N" */
               streng *tmp = Str_ncre_TSD( TSD, "N", 1 );
               set_trace( TSD, tmp );
               Free_stringTSD( tmp );
            }
         }

         break ;
      }

      case X_EXIT:
      {
         streng *result;

         if ( thisptr->p[0] )
            result = evaluate( TSD, thisptr->p[0], NULL );
         else
            result = NULL;

         TSD->instore_is_errorfree = 1;
         jump_script_exit( TSD, result );

         break;
      }

      case X_COMMAND:
      {
         streng *stmp,*kill;

         update_envirs( TSD, TSD->currlevel ) ;
         if (thisptr->p[0])
         {
            /* bja - added Free_stringTSD() around perform() */
            stmp = evaluate( TSD, thisptr->p[0], &kill );
            Free_stringTSD(perform(TSD, stmp, TSD->currlevel->environment, thisptr, NULL)) ;
            if ( kill )
               Free_stringTSD( kill );
            break ;
         }
      }

      case X_ADDR_N:   /* ADDRESS environment [expr] */
      {
         streng *envir,*tmp,*kill;
         update_envirs( TSD, TSD->currlevel ) ;
         envir = thisptr->name ;
         if (thisptr->p[0])
         {
            /*
             * This path is executed when the command is:
             *  ADDRESS env [command] WITH [expr]
             * ie. executing a command
             */
            /* bja - added Free_stringTSD() around perform() */
            /* the IO-redirection is temporarily in this case. */
            tmp = evaluate( TSD, thisptr->p[0], &kill );
            Free_stringTSD(perform(TSD, tmp, envir, thisptr, thisptr->p[1]));
            if ( kill )
               Free_stringTSD( kill ) ;
         }
         else
         {
            /*
             * This path is executed when the command is:
             *  ADDRESS env WITH [expr]
             * ie. setting the default address, but not executing anything
             */
            set_envir( TSD, envir, thisptr->p[1] ) ;
            Free_stringTSD( TSD->currlevel->prev_env ) ;
            TSD->currlevel->prev_env = TSD->currlevel->environment ;
            TSD->currlevel->environment = Str_dupTSD(envir) ;
         }
         break ;
      }


      case X_ADDR_V:   /* ADDRESS [VALUE] expr */
      {
         streng *cptr ;

         if ( thisptr->u.nonansi &&
              get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 2, "ADDRESS \"(\"...\")\"") ;

         update_envirs( TSD, TSD->currlevel ) ;
         cptr = evaluate(TSD,thisptr->p[0],NULL) ;
         set_envir( TSD, cptr, thisptr->p[1] ) ;
         Free_stringTSD( TSD->currlevel->prev_env ) ;
         TSD->currlevel->prev_env = TSD->currlevel->environment ;
         TSD->currlevel->environment = cptr ;
         break ;
      }


      case X_ADDR_S:   /* ADDRESS */
      {
         streng *tptr ;

         update_envirs( TSD, TSD->currlevel ) ;
         tptr = TSD->currlevel->environment ;
         TSD->currlevel->environment = TSD->currlevel->prev_env ;
         TSD->currlevel->prev_env = tptr ;
         break ;
      }


      case X_DROP:
      {
         nodeptr nptr ;
         for (nptr=thisptr->p[0]; nptr; nptr=nptr->p[0] )
         {
            if (nptr->name)
            {
               if (nptr->type == X_SIM_SYMBOL)
               {
                  drop_var( TSD, nptr->name ) ;
               }
               else
               {
                  if (nptr->type == X_IND_SYMBOL)
                  {
                     int begin,end;
                     streng *name;
                     const streng *value = shortcut(TSD,nptr) ;

                     /* Chop space separated words and drop them one by one */
                     for (end = 0;;)
                     {
                        begin = end; /* end of last word processed + 1 */
                        while ((begin < Str_len(value)) &&
                               rx_isspace(value->value[begin]))
                           begin++;
                        if (begin == Str_len(value))
                           break;
                        end = begin + 1; /* find next separator */
                        while ((end < Str_len(value)) &&
                               !rx_isspace(value->value[end]))
                           end++;
                        /* end now on space after word or past end of string */
                        name = Str_makeTSD(end - begin);
                        name->len = end - begin;
                        memcpy(name->value, value->value + begin, Str_len(name));
                        Str_upper(name);
                        drop_var( TSD, name ) ;
                        Free_stringTSD( name ) ;
                     }
                  }
               }
            }
         }
         break ;
      }

      case X_UPPER_VAR:
      {
         /*
          * If we are running in STRICT_ANSI mode, disallow this
          * keyword.
          * Copy the code above for DROP.
          * Need to cause error if stem variable is specified
          * Need to handle NOVALUE
          */
         nodeptr nptr ;

         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 2, "UPPER" )  ;
         for (nptr=thisptr->p[0]; nptr; nptr=nptr->p[0] )
         {
            if (nptr->name)
            {
               if (nptr->type == X_SIM_SYMBOL)
               {
                  upper_var( TSD, nptr->name ) ;
               }
               else
               {
                  if (nptr->type == X_IND_SYMBOL)
                  {
                     int begin,end;
                     streng *name;
                     const streng *value = shortcut(TSD,nptr) ;

                     /* Chop space separated words and drop them one by one */
                     for (end = 0;;)
                     {
                        begin = end; /* end of last word processed + 1 */
                        while ((begin < Str_len(value)) &&
                               rx_isspace(value->value[begin]))
                           begin++;
                        if (begin == Str_len(value))
                           break;
                        end = begin + 1; /* find next separator */
                        while ((end < Str_len(value)) &&
                               !rx_isspace(value->value[end]))
                           end++;
                        /* end now on space after word or past end of string */
                        name = Str_makeTSD(end - begin);
                        name->len = end - begin;
                        memcpy(name->value, value->value + begin, Str_len(name));
                        Str_upper(name);
                        upper_var( TSD, name ) ;
                        Free_stringTSD( name ) ;
                     }
                  }
               }
            }
         }
         break ;
      }

      case X_SIG_SET:
      case X_CALL_SET:
      {
         int type ;
         trap *traps = gettraps( TSD, TSD->currlevel ) ;

         /* which kind of condition is this? */
         type = identify_trap( thisptr->p[1]->type ) ;

         /* We always set this */
         traps[type].invoked = (thisptr->type == X_SIG_SET) ;
         traps[type].delayed = 0 ;
         traps[type].on_off = (thisptr->p[0]->type == X_ON ) ;

         /* set the name of the variable to work on */
         FREE_IF_DEFINED( TSD, traps[type].name ) ;
         if (thisptr->name)
            traps[type].name = Str_dupTSD( thisptr->name ) ;
         else if (thisptr->p[0]->type == X_ON)
            traps[type].name = Str_creTSD( signalnames[type] ) ;

         break ;
      }

      case X_SIG_VAL:
      case X_SIG_LAB:
      {
         streng *cptr, *kill=NULL;
         volatile char *tmp_str;
         stackelem *top;
         unsigned i1;

         cptr = (thisptr->name) ? thisptr->name : evaluate( TSD, thisptr->p[0], &kill );
         nstackcleanup( TSD, nstktrigger, NULL );
         top = stacktop( TSD );
         for ( i1 = stacktrigger( TSD ); i1 > stktrigger; i1--, top = top->prev )
         {
            if ( top->increment == s.increment )
               s.increment = NULL;
            if ( top->stopval == s.stopval )
               s.stopval = NULL;
#ifdef OPT_DO
            if ( top->do_for_val == s.do_for_val )
               s.do_for_val = NULL;
#endif
         }

         stackcleanup( TSD, stktrigger );
         /*
          * Fixes bug 764458
          */
         innerloop = NULL;

         entry = getlabel( TSD, cptr );
         /*
          * We have to make a temporary copy of the label we are signalling
          * in case it doesn't exist because the "kill" processing will destroy
          * the value.
          * Also set_reserved_value() also destroys the value.
          */
         tmp_str = tmpstr_of( TSD, cptr );

         set_reserved_value( TSD, POOL0_SIGL, NULL, thisptr->lineno, VFLAG_NUM );

         if ( kill )
            Free_stringTSD( kill );

         if ( entry == NULL )
            exiterror( ERR_UNEXISTENT_LABEL, 1, tmp_str );
         if ( entry->u.trace_only )
            exiterror( ERR_UNEXISTENT_LABEL, 2, tmpstr_of( TSD, entry->name) );
         thisptr = entry->next;
         goto fakerecurse;
      }
      case X_PROC:
      {
         treenode *ptr;

         if (TSD->currlevel->varflag)
             exiterror( ERR_UNEXPECTED_PROC, 1 )  ;

         for (ptr=thisptr->p[0];(ptr);ptr=ptr->p[0])
         {
            if (ptr->name)
            {
               expose_var(TSD,ptr->name) ;
               if (ptr->type==X_IND_SYMBOL)
                  expose_indir( TSD, getvalue( TSD, ptr->name, -1 ) );
               else
                  assert( ptr->type==X_SIM_SYMBOL) ;
            }
            else
               exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         }
         expose_var(TSD,NULL) ;
         break ;
      }
      case X_CALL:
      {
         nodeptr n;

         /*
          * Find an internal label matching the label on the CALL
          * statement, and determine if its in internal of external
          * subroutine call.
          */
         n = getlabel( TSD, thisptr->name );
         if ( n )
         {
            if ( n->u.trace_only )
               exiterror( ERR_UNEXISTENT_LABEL, 3, tmpstr_of( TSD, n->name ) );
            thisptr->type = X_IS_INTERNAL;
         }
         else
            thisptr->type = X_IS_BUILTIN;
         thisptr->u.node = n;
         thisptr->o.called = 1;
      }
      /* THIS IS MEANT TO FALL THROUGH! */
      case X_IS_INTERNAL:
      {
         paramboxptr args;
         streng *result;

         if ( thisptr->u.node )
         {
            no_next_interactive = 1;
            args = initplist( TSD, thisptr );
            set_reserved_value( TSD, POOL0_SIGL, NULL, thisptr->lineno, VFLAG_NUM );

            result = CallInternalFunction( TSD, thisptr->u.node, thisptr, args );

            TSD->systeminfo->interactive = TSD->currlevel->traceint;

            set_reserved_value( TSD, POOL0_RESULT, result, 0, ( result ) ? VFLAG_STR : VFLAG_NONE );
            break;
         }
      }
      /* THIS IS MEANT TO FALL THROUGH! */
      case X_EX_FUNC:
      case X_IS_BUILTIN:
      {
         streng *result ;

         /*
          * Call a builtin procedure...
          */
         if ((result = buildtinfunc( TSD, thisptr )) == NOFUNC)
         {
            /*
             * ... and it wasn't a builtin procedure so assume its an external function
             */
            thisptr->type = X_IS_EXTERNAL ;
         }
         else
         {
            /*
             * ... and if it was a builtin procedure set RESULT and break
             */
            set_reserved_value( TSD, POOL0_RESULT, result, 0,
                                ( result ) ? VFLAG_STR : VFLAG_NONE );

            break ;
         }
      }
      /* THIS IS MEANT TO FALL THROUGH! */
      case X_IS_EXTERNAL:
      {
         streng *ptr, *command;
         paramboxptr args, targs;
         int len,err;

         if ( TSD->restricted )
            exiterror( ERR_RESTRICTED, 5 );

         update_envirs( TSD, TSD->currlevel );

         args = targs = initplist( TSD, thisptr );
         stackmark = pushcallstack( TSD, TSD->currentnode );
         ptr = execute_external( TSD,
                                 thisptr->name,
                                 args,
                                 TSD->systeminfo->environment,
                                 &err,
                                        /* Fixes bug 604219                  */
                                 TSD->systeminfo->hooks,
                                 INVO_SUBROUTINE );
         popcallstack( TSD, stackmark );

         if ( ptr == thisptr->name )
         {
            /*
             * FIXME,MH: no idea what this does
             *      FGC: agreed, added an assert. Remove this block in
             *           complete in 2005 if nothing happens.
             */
            assert( ptr );
            ptr = NULL;
         }

         if ( err == -ERR_PROG_UNREADABLE )
         {
            /*
             * "thisptr->name" wasn't an external Rexx program, so
             * see if it is an OS command
             * Only do this if the OPTIONS EXT_COMMANDS_AS_FUNCS is
             * set and STRICT_ANSI is NOT set.
             */
            if ( get_options_flag( TSD->currlevel, EXT_EXT_COMMANDS_AS_FUNCS )
            &&  !get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            {
               len = Str_len( thisptr->name );
               for( targs = args; targs; targs = targs->next )
               {
                  if ( targs->value )
                     len += 1 + Str_len( targs->value );
               }
               command = Str_makeTSD( len );
               command = Str_catTSD( command, thisptr->name );
               for( targs = args; targs; targs = targs->next )
               {
                  if ( targs->value )
                  {
                     command = Str_catstrTSD( command, " " );
                     command = Str_catTSD( command, targs->value );
                  }
               }
               ptr = run_popen( TSD, command, TSD->currlevel->environment );
               if ( ptr != NULL )
                  err = 0;
               Free_stringTSD( command );
            }
         }

         deallocplink( TSD, args );

         if ( ptr && ( TSD->trace_stat == 'I' ) )
            tracevalue( TSD, ptr, 'F' );

         if ( ptr )
            set_reserved_value( TSD, POOL0_RESULT, ptr, 0, VFLAG_STR );
         else
            set_reserved_value( TSD, POOL0_RESULT, NULL, 0, VFLAG_NONE );

         if ( err == -ERR_PROG_UNREADABLE )
         {
            exiterror( ERR_ROUTINE_NOT_FOUND, 1, tmpstr_of( TSD, thisptr->name ) );
         }
         else if ( err )
         {
            post_process_system_call( TSD, thisptr->name, -err, NULL, thisptr );
         }

         break;
      }

      case X_PARSE:
      {
         /*
          * We always have to produce a duplicate of the content we have to
          * parse. We can't use variable locking and we can't assume that
          * the content doesn't contain variable names used in the template.
          * This fixes bug 688503.
          */

         if ( thisptr->u.parseflags & ( PARSE_LOWER | PARSE_CASELESS ) )
         {
            if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
               exiterror( ERR_NON_ANSI_FEATURE, 2,
                   ( thisptr->u.parseflags & PARSE_LOWER ) ? "PARSE LOWER" :
                                                          "PARSE CASELESS" )  ;
         }
         if ( thisptr->p[0]->type == X_PARSE_ARG )
         {
            parseargtree( TSD, TSD->currlevel->args, thisptr->p[1],
                          thisptr->u.parseflags );
         }
         else
         {
            streng *source = NULL;
            nodeptr templ;

            switch ( thisptr->p[0]->type )
            {
               case X_PARSE_VAR:
                  /* must duplicate, parsing may have side effects */
                  /* else, we must have locking of variables */
                  source = Str_dupTSD( shortcut( TSD, thisptr->p[0] ) );
                  break ;

               case X_PARSE_VAL:
               {
                  /*
                   * Must duplicate, parsing may have side effects, we must
                   * have locking of variables otherwise.
                   * fixes bug ?
                   */
                  /*
                   * Empty value allowed.
                   * Fixes bug 952229
                   */
                  if ( thisptr->p[0]->p[0] )
                     source = evaluate( TSD, thisptr->p[0]->p[0], NULL );
                  else
                     source = nullstringptr();
                  break ;
               }

               case X_PARSE_PULL:
                  source = popline( TSD, NULL, NULL, 0 );
                  break ;

               case X_PARSE_VER:
                  source = Str_creTSD( PARSE_VERSION_STRING );
                  break ;

               case X_PARSE_EXT:
                  source = readkbdline( TSD );
                  break ;

               case X_PARSE_SRC:
               {
                  const char *stype ;
                  streng *inpfile;

                  stype = system_type();
                  inpfile = TSD->systeminfo->input_file;
                  source = Str_makeTSD( strlen( stype ) + 4 +
                           strlen( invo_strings[TSD->systeminfo->invoked] ) +
                           Str_len( inpfile ) );
                  source->len = 0;

                  Str_catstrTSD( source, stype );
                  Str_catstrTSD( source, " " );
                  Str_catstrTSD( source, invo_strings[TSD->systeminfo->invoked] );
                  Str_catstrTSD( source, " " );
                  Str_catTSD( source, inpfile );
                  break;
               }
            }

            if ( thisptr->u.parseflags & PARSE_UPPER )
            {
               Str_upper( source );
            }
            if ( thisptr->u.parseflags & PARSE_LOWER )
            {
               Str_lower( source );
            }

            doparse( TSD, source, thisptr->p[1],
               thisptr->u.parseflags & PARSE_CASELESS );

            for ( templ = thisptr->p[1]->next; templ != NULL; templ = templ->next )
            {
               /*
                * This fixes bug 755801.
                * Actually, this will happen rarely, but we have to assign the
                * empty string to all template members of all comma-separated
                * lists of templates except of the first one.
                * We use the slow and long-term reliable code of doparse().
                */
               Str_len( source ) = 0;
               doparse( TSD, source, templ, 0 );
            }


            Free_stringTSD( source );
         }
         break;
      }

      case X_PULL:
      {
         streng *stmp ;

         doparse(TSD, stmp=Str_upper(popline( TSD, NULL, NULL, 0 )), thisptr->p[0], 0 ) ;
         Free_stringTSD( stmp ) ;
         break ;
      }

      case X_PUSH:
         stack_lifo( TSD, (thisptr->p[0]) ? evaluate(TSD,thisptr->p[0],NULL) : nullstringptr(), NULL ) ;
         break ;

      case X_QUEUE:
         stack_fifo( TSD, (thisptr->p[0]) ? evaluate(TSD,thisptr->p[0],NULL) : nullstringptr(), NULL ) ;
         break ;

      case X_OPTIONS: /* fixes 1116894 */
         do_options(TSD, TSD->currlevel, (thisptr->p[0]) ? evaluate(TSD,thisptr->p[0],NULL) : nullstringptr(), 0) ;
         break ;

      case X_RETURN:
      {
         stackelem *top;
         unsigned i1;
         streng *retval;

         /* buggy, need to deallocate procbox and vars ... */
         if (thisptr->p[0])
            retval = evaluate(TSD,thisptr->p[0],NULL) ;
         else
            retval = NULL ;

         top = stacktop(TSD);
         for (i1 = stacktrigger(TSD);i1 > stktrigger;i1--,top = top->prev)
         {
            if (top->increment == s.increment)
               s.increment = NULL;
            if (top->stopval == s.stopval)
               s.stopval = NULL;
#ifdef OPT_DO
            if (top->do_for_val == s.do_for_val)
               s.do_for_val = NULL;
#endif
         }

         stackcleanup(TSD,stktrigger);
         nstackcleanup(TSD,nstktrigger,NULL);
         return( retval ) ;
         break ;
      }
      case X_LEAVE:
      case X_ITERATE:
      {
         int Stacked;
         stackelem *top;
         treenode *iptr;

         Stacked = stacktrigger(TSD) - stktrigger;

         if ( innerloop )
         { /* push the current count to let it been found below if "LEAVE name". */
            s.thisptr = innerloop;
            stackpush( TSD, &s );
            Stacked++;
         }

         top = stacktop(TSD);
         for ( ;; ) /* while iteration counter name not found */
         {
            if ( Stacked <= 0 )
            {
               if ( innerloop )
                  exiterror( ERR_INVALID_LEAVE, (thisptr->type==X_LEAVE)?3:4, tmpstr_of( TSD, thisptr->name ) );
               else
                  exiterror( ERR_INVALID_LEAVE, (thisptr->type==X_LEAVE)?1:2 );
            }

            iptr = top->thisptr;

            if ( thisptr->name == NULL )
            {
               /*
                * LEAVE/ITERATE without any argument. Automatically pop one
                * stack element later.
                */
               break;
            }

            /*
             * Backtrace all pending loops and compare the iterator name if
             * one exists. We have to keep care for unnamed loops like
             * "do 5 ; say fred ; end".
             */
            if ( ( iptr->p[0] != NULL ) &&
                 ( iptr->p[0]->name != NULL ) &&
                 ( Str_cmp( thisptr->name, iptr->p[0]->name ) == 0 ) )
            {
               /*
                * Iterator name equals our argument. Automatically pop one
                * stack element later.
                */
               break;
            }

            /*
             * Unnamed loop or a loop with a nonmatching name, cleanup!
             * fixes bug 672864
             */
            popcallstack( TSD, -1 );
            if ( top->stopval == s.stopval )
               s.stopval = NULL;
#ifdef OPT_DO
            if ( top->do_for_val == s.do_for_val )
               s.do_for_val = NULL;
#endif
            if ( top->increment == s.increment )
               s.increment = NULL;
            stack_destroyelement( TSD, top );

            Stacked--;
            top = top->prev;
         }

         nstackcleanup(TSD,nstktrigger,&iptr);

         if (Stacked<=0)
            exiterror( ERR_INVALID_LEAVE, 0 );
         if (thisptr->type==X_LEAVE)
         {
            popcallstack(TSD,-1) ;
            if (top->stopval == s.stopval )
               s.stopval = NULL ;
#ifdef OPT_DO
            if (top->do_for_val == s.do_for_val )
               s.do_for_val = NULL ;
#endif
            if ( top->increment == s.increment )
               s.increment = NULL ;
            stack_destroyelement(TSD,top);
            Stacked--;
            top = top->prev;

            nstackpop(TSD);
         }
         if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
            traceline( TSD, iptr, TSD->trace_stat, 0 );
         stackcleanup(TSD,stktrigger + Stacked);

         if (TSD->systeminfo->interactive)
         {
            if (intertrace(TSD))
               goto fakerecurse ;
         }

         thisptr = nstackpop(TSD);

         if (Stacked)
         {
            s = stackpop(TSD);
            innerloop = s.thisptr;
         }
         else
            innerloop = NULL;
         goto fakereturn ;
         break ;
      }
      case X_NUM_D:
      {
         int tmp, error ;
         streng *cptr,*kill;
         volatile char *err ;

         cptr = evaluate( TSD, thisptr->p[0], &kill );
         tmp = streng_to_int( TSD, cptr, &error );
         if ( error || tmp <= 0 )
         {
             err = tmpstr_of( TSD, cptr );
             if ( kill )
               Free_stringTSD( kill );
             exiterror( ERR_INVALID_INTEGER, 5, err ) ;
         }
         if ( kill )
            Free_stringTSD( kill );
         if (TSD->currlevel->numfuzz >= tmp)
             exiterror( ERR_INVALID_RESULT, 1, tmp, TSD->currlevel->numfuzz );
         if (tmp > INT_MAX)
             exiterror( ERR_INVALID_RESULT, 2, tmp, INT_MAX )  ;
         TSD->currlevel->currnumsize = tmp ;
         break ;
      }

      case X_NUM_DDEF:
         if (TSD->currlevel->numfuzz >= DEFAULT_NUMERIC_SIZE)
             exiterror( ERR_INVALID_RESULT, 1, DEFAULT_NUMERIC_SIZE, TSD->currlevel->numfuzz )  ;
         TSD->currlevel->currnumsize = DEFAULT_NUMERIC_SIZE ;
         break ;

      case X_NUM_FDEF:
         if (TSD->currlevel->currnumsize <= DEFAULT_NUMERIC_FUZZ)
             exiterror( ERR_INVALID_RESULT, 1, TSD->currlevel->currnumsize, DEFAULT_NUMERIC_FUZZ )  ;
         TSD->currlevel->numfuzz = DEFAULT_NUMERIC_FUZZ ;
         break ;

      case X_NUM_FRMDEF:
         TSD->currlevel->numfuzz = DEFAULT_NUMFORM ;
         break ;

      case X_NUM_FUZZ:
      {
         int tmp, error ;
         streng *cptr,*kill;
         volatile char *err ;

         cptr = evaluate( TSD, thisptr->p[0], &kill );
         tmp = streng_to_int( TSD, cptr, &error );
         if ( error || tmp < 0 )
         {
             err = tmpstr_of( TSD, cptr );
             if ( kill )
               Free_stringTSD( kill );
             exiterror( ERR_INVALID_INTEGER, 6, err )  ;
         }
         if ( kill )
            Free_stringTSD( kill );
         if (TSD->currlevel->currnumsize <= tmp)
             exiterror( ERR_INVALID_RESULT, 1, TSD->currlevel->currnumsize, tmp )  ;
         TSD->currlevel->numfuzz = tmp ;
         break ;
      }

      case X_NUM_F:
      {
         if (thisptr->p[0]->type == X_NUM_SCI)
            TSD->currlevel->numform = NUM_FORM_SCI ;
         else if (thisptr->p[0]->type == X_NUM_ENG)
            TSD->currlevel->numform = NUM_FORM_ENG ;
         else
            assert( 0 ) ;
         break ;
      }

      case X_NUM_V:
      {
         streng *tmpstr,*kill;
         int len;
         char *s1;

         tmpstr = evaluate( TSD, thisptr->p[0], &kill );
         len = tmpstr->len;
         s1 = tmpstr->value;

         if ( ( len == 10 ) && ( mem_cmpic( s1, "SCIENTIFIC", 10 ) == 0 ) )
            TSD->currlevel->numform = NUM_FORM_SCI ;
         else if ( ( len == 11 ) && ( mem_cmpic( s1, "ENGINEERING", 11 ) == 0 ) )
            TSD->currlevel->numform = NUM_FORM_ENG ;
         else if ( ( len == 1 ) && ( rx_toupper( *s1 ) == 'S' ) )
            TSD->currlevel->numform = NUM_FORM_SCI ;
         else if ( ( len == 1 ) && ( rx_toupper( *s1 ) == 'E' ) )
            TSD->currlevel->numform = NUM_FORM_ENG ;
         else
            exiterror( ERR_INVALID_RESULT, 0 )  ;
         if ( kill )
            Free_stringTSD( kill );
         break ;
      }

      case X_LABEL:
      case X_NULL:
         break ;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         break ;
   }

   if ((TSD->systeminfo->interactive)&&(!no_next_interactive))
   {
      if (intertrace(TSD))
         goto fakerecurse ;
   }

   no_next_interactive = 0 ;

   if (thisptr)
      thisptr = thisptr->next ;

fakereturn:
   if (!thisptr)
   {
      if (nstacktrigger(TSD) <= nstktrigger)
      {
         stackcleanup(TSD,stktrigger);
         return NULL ;
      }
      else
         thisptr = nstackpop(TSD);
   }

fakerecurse:

   /* check if there is any traps to process */
   while (TSD->nextsig)
   {
      trap *traps = gettraps( TSD, TSD->currlevel ) ;

      i = TSD->nextsig->type ;

      if (i == SIGNAL_NOTREADY)
         fixup_file( TSD, TSD->nextsig->descr ) ;

      /* if this condition is in delayed mode, ignore it for now */
      if (traps[i].delayed)
         goto aftersignals ;

      /* if this condition is no begin trapped, use default action */
      if (traps[i].on_off == 0)
      {
         if (traps[i].def_act)
            goto aftersignals ;   /* default==1 ==> ignore it */
         else
            exiterror( TSD->nextsig->rc, 0 ) ;
      }
      if (traps[i].invoked)  /* invoke as SIGNAL */
      {
         /* simulate a SIGNAL, first empty the stack */
/* Sorry, not safe to operate on these at this point, we just have to
   accept that some memory is lost ... "can't make omelette without..." */
/*       if (stkidx)
 *          for (stkidx--;stkidx;stkidx--)
 *          {
 *             FREE_IF_DEFINED(TSD,stack[stkidx].increment) ;
 *             FREE_IF_DEFINED(TSD,stack[stkidx].stopval) ;
 *          }
 */  /* hey, this should really be ok, .... must be a BUG */
         stackcleanup(TSD,stktrigger); /* think it, too. stackcleanup
                                        * (re-)introduced Feb. 2000 */

         /* turn off the condition */
         traps[i].on_off = 0 ;
         traps[i].delayed = 0 ;
/*       traps[i].trapped = 0 ; */

         /* set the current condition information */
         if (TSD->currlevel->sig)
         {
            FREE_IF_DEFINED( TSD, TSD->currlevel->sig->info ) ;
            FREE_IF_DEFINED( TSD, TSD->currlevel->sig->descr ) ;
            FreeTSD( TSD->currlevel->sig ) ;
         }
         TSD->currlevel->sig = TSD->nextsig ;
         TSD->nextsig = NULL ;

         /* simulate the SIGNAL statement */
         entry = getlabel( TSD, traps[i].name ) ;
         set_reserved_value( TSD, POOL0_SIGL, NULL,
                             TSD->currlevel->sig->lineno, VFLAG_NUM );
         if (TSD->currlevel->sig->type == SIGNAL_SYNTAX )
            set_reserved_value( TSD, POOL0_RC, NULL, TSD->currlevel->sig->rc,
                                VFLAG_NUM );

         if ( entry == NULL )
            exiterror( ERR_UNEXISTENT_LABEL, 1, tmpstr_of( TSD, traps[i].name ) );
         if ( entry->u.trace_only )
            exiterror( ERR_UNEXISTENT_LABEL, 2, tmpstr_of( TSD, entry->name ) );
         thisptr = entry;
         nstackcleanup( TSD, nstktrigger, NULL );
         goto reinterpret;
      }
      else /*if ((i<SIGNALS))*/ /* invoke as CALL */
      {
         nodeptr savecurrentnode; /* pgb */
         streng *h;

         if ( ( entry = getlabel( TSD, traps[i].name ) ) == NULL )
            exiterror( ERR_UNEXISTENT_LABEL, 1, tmpstr_of( TSD, traps[i].name ) );
         if ( entry->u.trace_only )
            exiterror( ERR_UNEXISTENT_LABEL, 3, tmpstr_of( TSD, entry->name ) );

         traps[i].delayed = 1;

         set_reserved_value( TSD, POOL0_SIGL, NULL, TSD->nextsig->lineno,
                             VFLAG_NUM );
         oldlevel = TSD->currlevel;
         TSD->currlevel = newlevel( TSD, TSD->currlevel );
         TSD->currlevel->sig = TSD->nextsig;
         TSD->nextsig = NULL;

         stackmark = pushcallstack( TSD, thisptr );
         if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
            traceline( TSD, entry, TSD->trace_stat, 0 );

         savecurrentnode = TSD->currentnode; /* pgb */
         h = interpret( TSD, entry->next );
         if ( h != NULL )
            Free_stringTSD( h );
         TSD->currentnode = savecurrentnode; /* pgb */

         traps[i].delayed = 0;
         popcallstack( TSD, stackmark );
         removelevel( TSD, TSD->currlevel );
         TSD->currlevel = oldlevel;
         TSD->currlevel->next = NULL;
         TSD->trace_stat = TSD->currlevel->tracestat;
         TSD->systeminfo->interactive = TSD->currlevel->traceint; /* MDW 30012002 */
      }
   }

aftersignals:

   goto reinterpret;

}

/* getlabel searches for a label (procedure) in the current rexx program.
 * The label is case-insensitively searched. Its name must be name. The first
 * name found matches. The returned value is either NULL or the node of the
 * name.
 * This function uses a lazy evaluation mechanism and creates from the linked
 * list an array. The hash value of each entry is generated during the copy.
 * This function may become faster or much faster if the array is sorted
 * by the hashvalue which allows a bsearch() call. But, and this is the
 * problem, it is useful only if labels are search often and the number of
 * labels are more than a few. I think, external functions which are registered
 * in huge amounts are better candidates for this.
 */
nodeptr getlabel( const tsd_t *TSD, const streng *name )
{
   labelboxptr lptr, h;
   internal_parser_type *ipt = &TSD->systeminfo->tree;
   unsigned i, hash;

   if (ipt->sort_labels == NULL)
   {
      if (ipt->first_label == NULL)
         return(NULL);

      ipt->sort_labels = (labelboxptr)MallocTSD(ipt->numlabels * sizeof(ipt->sort_labels[0]));
      for (i = 0, lptr = ipt->first_label;i < ipt->numlabels;i++)
         {
            lptr->hash = hashvalue_ic(lptr->entry->name->value, lptr->entry->name->len);
            ipt->sort_labels[i] = *lptr;
            h = lptr->next;
            FreeTSD(lptr);
            lptr = h;
         }
      ipt->first_label = ipt->last_label = NULL;
   }

   hash = hashvalue_ic(name->value, name->len);
   for (i = 0;i < ipt->numlabels;i++)
   {
      if (hash != ipt->sort_labels[i].hash)
         continue;
      if (Str_ccmp(ipt->sort_labels[i].entry->name, name) == 0)
         return(ipt->sort_labels[i].entry);
   }
   return(NULL);
}


void removelevel( tsd_t *TSD, proclevel level )
{
   int i=0 ;

   if ( level->next )
   {
      removelevel( TSD, level->next ) ;
/*      level->next = NULL; */
   }

   if (level->varflag==1) /* does not belong *here* !!! */
      kill_variables( TSD, level->vars ) ;

   if (level->args)
      deallocplink( TSD, level->args ) ;

   if (level->environment)
      Free_stringTSD( level->environment ) ;

   if (level->prev_env)
      Free_stringTSD( level->prev_env ) ;

   if (level->prev)
      level->prev->next = NULL ;

   FREE_IF_DEFINED( TSD, level->signal_continue );

   if (level->sig)
   {
      FREE_IF_DEFINED( TSD, level->sig->info ) ;
      FREE_IF_DEFINED( TSD, level->sig->descr ) ;
      FreeTSD( level->sig ) ;
   }

   if (level->traps)
   {
      for (i=0; i<SIGNALS; i++)
         FREE_IF_DEFINED( TSD, level->traps[i].name ) ;

      FreeTSD( level->traps ) ;
   }

   FreeTSD(level) ;
}


/*
 * NOTE: The ->buf variable is not set here, It must be set. When
 *    an old level is duplicated, the old ->buf is also duplicated,
 *    but DO_NO_USE_IT, since it will point to the reentring point
 *    of the mother-routine
 */
proclevel newlevel( tsd_t *TSD, proclevel oldlevel )
{
   itp_tsd_t *it = (itp_tsd_t *)TSD->itp_tsd;
   proclevel level;
   int i;
   char *str;
   streng *opts;

   level = (proclevel) MallocTSD( sizeof( proclevbox ) );

   if ( oldlevel == NULL )
   {
      /*
       * This is executed only once on startup of the interpreter.
       */
#ifdef __CHECKER__
      /* There is a memcpy below which Checker don't like. The reason
       * may be the aligned "char"s which will use one machine word
       * but are initialized simply by an assignment of one byte.
       * Checker sees 3 byte of uninitialized data --> error.
       * (Of course, this isn't an error.)
       * Always double-check the initializations below in case of
       * any changes.
       * FGC
       */
      memset( level, 0, sizeof( proclevbox ) );
#endif
      level->numfuzz = DEFAULT_NUMERIC_FUZZ;
      level->currnumsize = DEFAULT_NUMERIC_SIZE;
      level->numform = DEFAULT_NUMFORM;
      level->rx_time.sec = 0;
      level->rx_time.usec = 0;
      level->mathtype = DEFAULT_MATH_TYPE;
      level->prev = NULL;
      level->next = NULL;
      level->args = NULL;
      level->options = 0;

      if ( it->opts_set )
         level->options = it->options;
      else
      {
         /* never call set_options_flag() on MATAOP options */
         set_options_flag( level, EXT_LINEOUTTRUNC, DEFAULT_LINEOUTTRUNC );
         set_options_flag( level, EXT_FLUSHSTACK, DEFAULT_FLUSHSTACK );
         set_options_flag( level, EXT_MAKEBUF_BIF, DEFAULT_MAKEBUF_BIF );
         set_options_flag( level, EXT_DROPBUF_BIF, DEFAULT_DROPBUF_BIF );
         set_options_flag( level, EXT_DESBUF_BIF, DEFAULT_DESBUF_BIF );
         set_options_flag( level, EXT_BUFTYPE_BIF, DEFAULT_BUFTYPE_BIF );
         set_options_flag( level, EXT_CACHEEXT, DEFAULT_CACHEEXT );
         set_options_flag( level, EXT_PRUNE_TRACE, DEFAULT_PRUNE_TRACE );
         set_options_flag( level, EXT_EXT_COMMANDS_AS_FUNCS, DEFAULT_EXT_COMMANDS_AS_FUNCS );
         set_options_flag( level, EXT_STDOUT_FOR_STDERR, DEFAULT_STDOUT_FOR_STDERR );
         set_options_flag( level, EXT_TRACE_HTML, DEFAULT_TRACE_HTML );
         set_options_flag( level, EXT_FAST_LINES_BIF_DEFAULT, DEFAULT_FAST_LINES_BIF_DEFAULT );
         set_options_flag( level, EXT_STRICT_ANSI, DEFAULT_STRICT_ANSI );
         set_options_flag( level, EXT_INTERNAL_QUEUES, DEFAULT_INTERNAL_QUEUES );
         set_options_flag( level, EXT_REGINA_BIFS, DEFAULT_REGINA_BIFS );
         set_options_flag( level, EXT_STRICT_WHITE_SPACE_COMPARISONS, DEFAULT_STRICT_WHITE_SPACE_COMPARISONS );
         set_options_flag( level, EXT_AREXX_SEMANTICS, DEFAULT_AREXX_SEMANTICS );
         set_options_flag( level, EXT_AREXX_BIFS, DEFAULT_AREXX_BIFS );
         set_options_flag( level, EXT_BROKEN_ADDRESS_COMMAND, DEFAULT_BROKEN_ADDRESS_COMMAND );
         set_options_flag( level, EXT_CALLS_AS_FUNCS, DEFAULT_CALLS_AS_FUNCS );
         set_options_flag( level, EXT_QUEUES_301, DEFAULT_QUEUES_301 );
         set_options_flag( level, EXT_HALT_ON_EXT_CALL_FAIL, DEFAULT_HALT_ON_EXT_CALL_FAIL );
         set_options_flag( level, EXT_SINGLE_INTERPRETER, DEFAULT_SINGLE_INTERPRETER );
         set_options_flag( level, EXT_SINGLE_LINE_COMMENTS, DEFAULT_SINGLE_LINE_COMMENTS );

         if ( ( str = mygetenv( TSD, "REGINA_OPTIONS", NULL, 0 ) ) != NULL )
         {
            opts = Str_creTSD( str );
            FreeTSD( str );
            do_options( TSD, level, opts, 0 );
         }
         it->opts_set = 1;
         it->options = level->options;
      }

      level->varflag = 1;
      level->tracestat = (char) TSD->systeminfo->tracing;
      level->traceint = (char) TSD->systeminfo->interactive;
      level->environment = Str_dupTSD( TSD->systeminfo->environment );
      level->prev_env = Str_dupTSD( TSD->systeminfo->environment );
      level->vars = create_new_varpool( TSD, 0 );
      level->signal_continue = NULL;
      level->sig = NULL;
      level->traps = (trap *)MallocTSD( sizeof(trap) * SIGNALS );
#ifdef __CHECKER__
      /* See above */
      memset( level->traps, 0, sizeof(trap) * SIGNALS );
#endif
      for (i=0; i<SIGNALS; i++)
      {
         level->traps[i].name = NULL;
         level->traps[i].on_off = 0;
         level->traps[i].delayed = 0;
         level->traps[i].def_act = default_action[i];
         level->traps[i].ignored = default_ignore[i];
         level->traps[i].invoked = 0;
      }
      level->pool = 1;
   }
   else
   {
      /* Stupid SunOS acc gives incorrect warning for the next line */
      memcpy( level, oldlevel, sizeof( proclevbox ) );
#ifdef DONT_DO_THIS
      level->prev_env = NULL;
      level->environment = NULL;
#else
      level->prev_env = Str_dupTSD( oldlevel->prev_env );
      level->environment = Str_dupTSD( oldlevel->environment );
#endif
      level->prev = oldlevel;
      level->varflag = 0;
      oldlevel->next = level;
      level->signal_continue = NULL;
      level->args = NULL;
/*    level->next = NULL;*/
      level->sig = NULL;
      level->traps = NULL;
      level->pool++;
   }

   TSD->trace_stat = level->tracestat;
   return level;
}


static void expose_indir( tsd_t *TSD, const streng *list )
{
   const char *cptr=NULL, *eptr=NULL, *sptr=NULL ;
   streng *tmp=NULL ;

   cptr = list->value ;
   eptr = cptr + list->len ;
   tmp = Str_makeTSD( 64 ) ;
   for (;cptr<eptr;)
   {
      for (; cptr<eptr && rx_isspace(*cptr); cptr++ ) ;
      for (sptr=cptr; cptr<eptr && !rx_isspace(*cptr); cptr++ ) ;
      if (cptr-sptr >= 64)
          exiterror( ERR_TOO_LONG_STRING, 0 )  ;
      if (cptr==sptr)
         continue;

      memcpy( tmp->value, sptr, cptr-sptr ) ;
      tmp->len = cptr-sptr ;
/* need to uppercase each variable in the list!! */
      Str_upper( tmp );
      expose_var( TSD, tmp ) ;
   }
   Free_stringTSD( tmp ) ;
}


/*
 * jump_rexx_signal should be used when a "SIGNAL ON" condition happens.
 * This function jumps to the previously assigned handler. This function
 * ensures a proper cleanup if the global lock flag "in_protected" is set.
 */
void jump_rexx_signal( tsd_t *TSD )
{
   if ( TSD->in_protected )
   {
      /*
       * The lexer is running. We have to terminate him and let him do his
       * cleanup. After it, we'll be called again but without "in_protected".
       */
      TSD->delayed_error_type = PROTECTED_DelayedRexxSignal;
      longjmp( TSD->protect_return, 1 );
   }
   longjmp( *TSD->currlevel->signal_continue, 1 );
}


/*
 * jump_interpreter_exit should be used when the whole interpreter should
 * terminate. This usually happens in case of a hard error or when the main
 * script ends.
 *
 * processExitCode tells the interpreter what return code shall be used on the
 * last exit.
 *
 * DON'T GET CONFUSED WITH jump_script_exit!
 *
 * This function jumps to the previously assigned handler. This function
 * ensures a proper cleanup if the global lock flag "in_protected" is set.
 */
void jump_interpreter_exit( tsd_t *TSD, int processExitCode )
{
   if ( TSD->in_protected )
   {
      /*
       * The lexer is running. We have to terminate him and let him do his
       * cleanup. After it, we'll be called again but without "in_protected".
       */
      TSD->expected_exit_error = processExitCode;
      TSD->delayed_error_type = PROTECTED_DelayedInterpreterExit;
      longjmp( TSD->protect_return, 1 );
   }
   TSD->MTExit( processExitCode );
}


/*
 * jump_script_exit should be used when a script ends or enters an EXIT
 * instraction.
 *
 * result tells the interpreter what return string shall be returned to the
 * caller.
 *
 * DON'T GET CONFUSED WITH jump_interpreter_exit!
 *
 * This function jumps to the previously assigned handler. This function
 * ensures a proper cleanup if the global lock flag "in_protected" is set.
 */
void jump_script_exit( tsd_t *TSD, streng *result )
{
   TSD->systeminfo->result = result;

   if ( TSD->in_protected && TSD->systeminfo->script_exit )
   {
      /*
       * The lexer is running. We have to terminate him and let him do his
       * cleanup. After it, we'll be called again but without "in_protected".
       */
      TSD->delayed_error_type = PROTECTED_DelayedScriptExit;
      longjmp( TSD->protect_return, 1 );
   }

   if ( !TSD->systeminfo->script_exit )
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__,
                 "script EXIT not registered" );

   longjmp( *TSD->systeminfo->script_exit, 1 );
}
