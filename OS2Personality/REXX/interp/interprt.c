#ifndef lint
static char *RCSid = "$Id: interprt.c,v 1.2 2003/12/11 04:43:10 prokushev Exp $";
#endif

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
#ifndef VMS
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif

#define XOR(a,b) (( (a) && (!(b)) )||( (!(a)) && (b) ))

#define TRACELINE(a) if (TSD->trace_stat!='O' && TSD->trace_stat!='N' && TSD->trace_stat!='F') traceline(TSD,a,(char)TSD->trace_stat,0)
#define TRACEVALUE(a,b) if (TSD->trace_stat=='I') tracevalue(TSD, a,b)
#define TRACENUMBER(a,b) if (TSD->trace_stat=='I') tracenumber(TSD, a,b)

#ifdef WIN32
/* Asynchroneous scheduled in another thread: */
volatile int __regina_Win32CtrlCRaised = 0;
#endif
staticstreng(    RC_name,      "RC" );
staticstreng( dotRC_name,     ".RC" );
staticstreng(    RESULT_name,  "RESULT" );
staticstreng( dotRESULT_name, ".RESULT" );
staticstreng(    SIGL_name,    "SIGL" );
staticstreng( dotSIGL_name,   ".SIGL" );
conststreng(  dotMN_name,     ".MN" );
conststreng(  dotRS_name,     ".RS" );

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
   int                number ;
   int                incrdir ;
   num_descr *        increment ;
   num_descr *        stopval ;
   nodeptr            this ;
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
   nodeptr    nvar_rc;
   nodeptr    nvar_sigl;
   nodeptr    nvar_result;
   nstackbox  nbox;
   nstackbox *nbox_top;
   stackbox   sbox;
   stackbox  *stk_top;
} itp_tsd_t; /* thread-specific but only needed by this module. see
              * init_spec_vars
              */



static void expose_indir( tsd_t *TSD, const streng *list ) ;

#ifdef TRACEMEM
static void mark_spec_vars(const tsd_t *TSD)
{
   itp_tsd_t *it;

   it = TSD->itp_tsd;
   markmemory( it->nvar_rc, TRC_SPCV_BOX ) ;
   markmemory( it->nvar_rc->name, TRC_SPCV_NAME ) ;

   markmemory( it->nvar_sigl, TRC_SPCV_BOX ) ;
   markmemory( it->nvar_sigl->name, TRC_SPCV_NAME ) ;
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

   if ((it = TSD->itp_tsd = MallocTSD(sizeof(itp_tsd_t))) == NULL)
      return(0);
   memset(it,0,sizeof(itp_tsd_t));

   it->nbox_top = &it->nbox;
   it->stk_top = &it->sbox;

   it->nvar_sigl = MallocTSD(sizeof(*it->nvar_sigl)) ;
   it->nvar_sigl->u.varbx = NULL ;
   it->nvar_sigl->name = Str_dupTSD( SIGL_name ) ;
   it->nvar_sigl->type = X_SIM_SYMBOL ;

   it->nvar_rc = MallocTSD(sizeof(*it->nvar_rc)) ;
   it->nvar_rc->u.varbx = NULL ;
   it->nvar_rc->name = Str_dupTSD( RC_name ) ;
   it->nvar_rc->type = X_SIM_SYMBOL ;

#ifdef TRACEMEM
   regmarker( TSD, mark_spec_vars ) ;
#endif
   return(1);
}


void update_envirs( const tsd_t *TSD, proclevel level )
{
   proclevel lptr=NULL ;

   if (!level->environment)
      for (lptr=level->prev; lptr; lptr=lptr->prev)
         if (lptr->environment)
         {
            level->environment = Str_dupTSD(lptr->environment) ;
            break ;
         }

   if (!level->prev_env)
      for (lptr=level->prev; lptr; lptr=lptr->prev)
         if (lptr->prev_env)
         {
            level->prev_env = Str_dupTSD(lptr->prev_env) ;
            break ;
         }

   assert( level->environment ) ;
   assert( level->prev_env ) ;
}

/*
 * set_sigl sets a shortcut to nvar_sigl (the variable called "SIGL"). This
 * is the setting to the numeric value argument.
 * FIXME, FGC: This routines gives me a bad feeling until we know more
 *             about setshortcut. Does setshortcut always use the top
 *             level procedure layer or the "pool 0"? Strange things may
 *             occur. Next, set_sigl sets the variable at least once before
 *             any exceptions occurs. Very bad for SIGL processing routines
 *             in this case.
 */
void set_sigl( tsd_t *TSD, int line )
{
   itp_tsd_t *it;

   it = TSD->itp_tsd;
   setshortcut( TSD, it->nvar_sigl, int_to_streng( TSD, line )) ;
   setvalue( TSD, dotSIGL_name, int_to_streng( TSD, line ) );
}

/*
 * set_rc sets a shortcut to nvar_rc (the variable called "RC").
 * FIXME, FGC: This routines gives me a bad feeling until we know more
 *             about setshortcut. Does setshortcut always use the top
 *             level procedure layer or the "pool 0"? Strange things may
 *             occur.
 *             The argument will be consumed.
 */
void set_rc( tsd_t *TSD, streng *value )
{
   itp_tsd_t *it;

   it = TSD->itp_tsd;
   setshortcut( TSD, it->nvar_rc, Str_dupTSD( value ) );
   setvalue( TSD, dotRC_name, value );
}

/*
 * set_result sets or drops the value of the variables called "RESULT" and
 * ".RESULT". The value is NULL for a drop operation or non-NULL for a
 * set operation and will be consumed.
 */
static void set_result( tsd_t *TSD, streng *value )
{
   if ( value )
   {
      setvalue( TSD, RESULT_name, Str_dupTSD( value ) );
      setvalue( TSD, dotRESULT_name, value );
   }
   else
   {
      drop_var( TSD, RESULT_name );
      drop_var( TSD, dotRESULT_name );
   }
}

/* nstackpush pushes the arg pnode on the nstack. (copy, then increment)
 */
static void nstackpush(const tsd_t *TSD,nodeptr pnode)
{
   itp_tsd_t *it;
   nstackbox *ns;

   it = TSD->itp_tsd;
   ns = it->nbox_top;
   ns->elems[ns->used++] = pnode;
   if (ns->used >= NSTACKELEMS)
   {
      if (ns->next == NULL)
      {
         ns->next = MallocTSD(sizeof(nstackbox));
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

   it = TSD->itp_tsd;
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

   it = TSD->itp_tsd;
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

   it = TSD->itp_tsd;
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

   it = TSD->itp_tsd;
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
         sb->next = MallocTSD(sizeof(stackbox));
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

   it = TSD->itp_tsd;
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

   it = TSD->itp_tsd;
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

   it = TSD->itp_tsd;
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
   it = TSD->itp_tsd;
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

streng *CallInternalFunction( tsd_t *TSD, nodeptr node, nodeptr this,
                              paramboxptr args )
{
   int stackmark;
   streng *result;
   proclevel oldlevel;
   nodeptr savecurrentnode;

   oldlevel = TSD->currlevel;
   TSD->currlevel = newlevel( TSD, TSD->currlevel );
   TSD->currlevel->args = args;
   stackmark = pushcallstack( TSD, this );

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

streng *interpret(tsd_t * volatile TSD, treenode * volatile this)
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
   num_descr *tdescr=NULL ;
   volatile nodeptr secure_this ;
   tsd_t * volatile secure_TSD ;
   itp_tsd_t * volatile it;

   it = TSD->itp_tsd;

   nstktrigger = nstacktrigger(TSD);
   stktrigger = stacktrigger(TSD);

   secure_TSD = TSD; /* vars used until here */
   secure_this = this;

   if ( TSD->currlevel->buf == NULL )
   {
      TSD->currlevel->buf = MallocTSD( sizeof(jmp_buf) ) ;

      assert(!TSD->in_protected);
      if (setjmp( *(TSD->currlevel->buf) ))
      {
         /* A signal arrived and a longjmp from anywhere jumps here.
          * We can't believe in anything and have to rebuild it from
          * scratch or volatile pointers. Even an unoptimized compiler
          * may have optimized the access to values of any kind.
          * We have to do the full reinitialization.
          * prevents bugs like 592393
          */
         this = secure_this ;
         TSD = secure_TSD ;
         it = TSD->itp_tsd ;

         tdescr = NULL ;
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
   tdescr = NULL ;
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

   if (this==NULL)
      goto fakereturn ;

   secure_this = this;

   TSD->currentnode = this ;
   if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
   {
      if (this->type != X_DO)  /* let do-stats trace themselves */
         traceline(TSD, this,(char) TSD->trace_stat,0) ;
   }

   if (this->now)
   {
      FreeTSD(this->now);
      this->now = NULL;
   }

   this->o.called = 0;

   switch ( /*(unsigned char)*/ (this->type) )
   {
      case X_PROGRAM:
      case X_STATS:

      case X_WHENS:
      case X_OTHERWISE:
         this = this->p[0] ;
         goto reinterpret ;


      case 0:
      case 255:
      case X_DO:
      {
         streng *tmpstr,*tmpkill=NULL;

         if (innerloop==this)
         {
            assert( this->p[3] ) ;
            if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
            {
               traceline(TSD, this->p[3],(char) TSD->trace_stat,-1) ;
               traceline(TSD, this,(char) TSD->trace_stat,-1) ;
            }
            goto one ;
         }
         else
            TRACELINE(this) ;

         if (!((this->p[0])||(this->p[1])))
         {
            nstackpush(TSD,this->next);
            this = this->p[2] ;
            goto fakerecurse ;
         }

         nstackpush(TSD,this->next); /* for use with leave */

         if (innerloop)
         {
            s.this = innerloop;
            stackpush(TSD,&s);
         }

         s.incr_node = NULL;
         s.increment = s.stopval = tdescr = NULL ;
         s.incrdir = 1 ;
         s.number = -1 ;
         tmpstr = NULL ;
         tdescr = NULL ;
         if ((this->p[0])&&(this->p[0]->name))
            tmpstr = evaluate( TSD, this->p[0]->p[0], &tmpkill );

         for (i=1;i<4;i++)
         {
            if ((this->p[0])&&(this->p[0]->p[i]))
            {
               nodeptr tmpptr ;
               switch( this->p[0]->p[i]->type )
               {
                  case X_DO_TO:
                     tmpptr = this->p[0]->p[i]->p[0] ;
                     s.stopval = calcul(TSD,tmpptr,NULL) ;
                     break ;

                  case X_DO_BY:
                     s.incr_node = this->p[0]->p[i]->p[0] ;
                     tmpptr = this->p[0]->p[i]->p[0] ;
                     s.increment = calcul(TSD,tmpptr,NULL) ;
                     s.incrdir = descr_sign( s.increment ) ;
                     break ;

                  case X_DO_FOR:
                  case X_DO_EXPR:
                  {
                     int iptr, error ;
                     streng *chptr,*chkill;

                     tmpptr = this->p[0]->p[i]->p[0] ;
                     chptr = evaluate(TSD, tmpptr, &chkill );
                     iptr = streng_to_int(TSD, chptr, &error);
                     if ( error )
                        exiterror( ERR_INVALID_INTEGER, (this->p[0]->p[i]->type==X_DO_EXPR) ? 2 : 3, chptr->value );
                     if ( iptr < 0 )
                        exiterror( ERR_INVALID_RESULT, 0 );
                     s.number = iptr ;
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
             * what says ANSI? must "???" in "do i = ???" be normalized?
             * FIXME, Florian, 26.12.2002
             */
            setshortcut( TSD, this->p[0], str_normalize( TSD, tmpstr ) );
            tdescr = shortcutnum( TSD, this->p[0] );
            if ( tmpkill )
               Free_stringTSD( tmpkill );
         }

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
               goto fakerecurse ;
            }
         }
startloop:
         if (this->p[0])
         {
            if (s.stopval)
            {
               int tsign ;

               tsign = string_test( TSD, tdescr, s.stopval ) ;
               if (!(tsign ^ s.incrdir))
                  goto endloop ;
            }

            if ((s.number>=0) && (s.number--<=0))
               goto endloop ;
         }

         if ((this->p[1])&&((this->p[1]->type)==X_WHILE))
            if (!isboolean(TSD,this->p[1]->p[0]))
               goto endloop ;

         if (this->p[2])
         {
            nstackpush(TSD,this);
            pushcallstack(TSD,NULL) ;

            innerloop = this ;
            this = this->p[2] ;
            goto fakerecurse ;

one:
            popcallstack(TSD,-1) ;
         }
         if ((this->p[1])&&((this->p[1]->type)==X_UNTIL))
         {
            if (isboolean(TSD,this->p[1]->p[0]))
               goto endloop ;
         }

         if ((this->p[0])&&(this->p[0]->name))
         {
            tdescr = shortcutnum( TSD, this->p[0] ) ;
            /*
             * Check if we still have a valid number. If not
             * exit with arithmetic error.
             */
            if (!tdescr)
               exiterror( ERR_BAD_ARITHMETIC, 0 )  ;
/*
            if ((this->p[0]->u.varbx) && ( this->p[0]->u.varbx->valid ))
            {
               tdescr = this->p[0]->u.varbx->num ;
               this->p[0]->u.varbx->num = NULL ;
            }
            else
            {
               this->p[0]->u.varbx = NULL ;
               if (!(tdescr=get_a_descr(TSD,shortcut(TSD,this->p[0]))))
                   exiterror( ERR_BAD_ARITHMETIC, 0 )  ;
               try = NULL ;
            }
 */

            if (s.increment)
               string_add( TSD, tdescr, s.increment, tdescr, this->p[0],
                           s.incr_node ) ;
            else
               string_incr( TSD, tdescr, this->p[0] ) ;

            if (this->p[0]->u.varbx)
            {
/*
               if (this->p[0]->u.varbx->num)
               {
                  FreeTSD( this->p[0]->u.varbx->num->num ) ;
                  FreeTSD( this->p[0]->u.varbx->num ) ;
               }
 */
               this->p[0]->u.varbx->num = tdescr ;
               this->p[0]->u.varbx->flag = VFLAG_NUM ;
               TRACENUMBER( tdescr, 'V' ) ;
            }
            else
               setshortcut( TSD, this->p[0], str_norm( TSD, tdescr, NULL )) ;
         }

         if (TSD->nextsig)
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
         no_next_interactive = 1 ;
         nstackpop(TSD);

         if (stacktrigger(TSD) > stktrigger)
         {
            s = stackpop(TSD);
            innerloop = s.this;
         }
         else
            innerloop = NULL ;

         break ;
      }
      case X_IF:
      {
         treenode *othis = this, *n;
         int retval = isboolean( TSD, this->p[0] );

         if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
         {
            n = this->p[0]->next;
            while ( n != NULL ) {
               traceline( TSD, n, (char) TSD->trace_stat, 0 );
               n = n->next;
            }
         }

         nstackpush(TSD,this->next);
         this = this->p[retval ? 1 : 2];
         if (TSD->systeminfo->interactive)
         {
            if (intertrace(TSD))
            {
               this = othis ;
            }
         }

         goto fakerecurse ;
      }
      case X_NASSIGN:
      {
         num_descr *ntmp;
         streng *preferred_str;
         int type;

         ntmp = calcul(TSD,this->p[1],NULL);
         assert( ntmp->size );

         type = this->p[1]->type;
         if ( ( type == X_STRING ) || ( type == X_CON_SYMBOL ) )
            preferred_str = Str_dupTSD( this->p[1]->name );
         else
            preferred_str = NULL;

         if (this->p[0]->type==X_HEAD_SYMBOL)
         {
            fix_compoundnum( TSD, this->p[0], ntmp, preferred_str );
         }
         else
         {
            setshortcutnum( TSD, this->p[0], ntmp, preferred_str );
         }
      }
      break ;

      case X_ASSIGN:
         {
/* This is a CMS-ism; CMS allows the expression in an assignment to
 * be omitted, while TRL does _not_. If a CMS mode is implemented, the
 * code below should be changed to allow p[0] to be null only iff
 * CMS mode is active.
 */
            streng *value ;

            value = this->p[1] ? evaluate(TSD,this->p[1],NULL) : nullstringptr() ;
            if (this->p[0]->type==X_HEAD_SYMBOL)
               fix_compound( TSD, this->p[0], value ) ;
            else
               setshortcut( TSD, this->p[0], value ) ;
         }
         break ;

      case X_IPRET:
      {
         streng *retval, *tptr = evaluate(TSD,this->p[0],NULL) ;
         retval = dointerpret( TSD, tptr ) ;
         if (retval != NULL) /* we interpreted a RETURN WITH a value */
         {
            stackelem *top = stacktop(TSD);
            unsigned i;
            for (i = stacktrigger(TSD);i > stktrigger;i--,top = top->prev)
            {
               if (top->increment == s.increment)
                  s.increment = NULL;
               if (top->stopval == s.stopval)
                  s.stopval = NULL;
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
         nstackpush(TSD,this->next);
         nstackpush(TSD,this->p[1]);
         this = this->p[0] ;
         goto fakerecurse ;

      case X_WHEN:
      {
         int retval = isboolean( TSD, this->p[0] );
         nodeptr n;
         if ( TSD->trace_stat != 'O' && TSD->trace_stat != 'N' && TSD->trace_stat != 'F' )
         {
            n = this->p[0]->next;
            while ( n != NULL ) {
               traceline( TSD, n, (char) TSD->trace_stat, 0 );
               n = n->next;
            }
         }
         if ( retval )
         {
            nstackpop(TSD); /* kill the OTHERWISE on the stack */
            this = this->p[1] ;
            goto fakerecurse ;
         }
         break ;
      }

      case X_SAY:
      {
         int ok=HOOK_GO_ON ;
         streng *stringen,*kill=NULL;

         if (this->p[0])
            stringen = evaluate( TSD, this->p[0], &kill );
         else
            stringen = NULL ;

         if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_STDOUT))
            ok = hookup_output( TSD, HOOK_STDOUT, stringen ) ;

         if (ok==HOOK_GO_ON)
         {
            if (stringen)
               fwrite( stringen->value, Str_len(stringen), 1, stdout ) ;
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
            if (this->name)
               set_trace( TSD, this->name ) ;
            else if (this->p[0])
            {
               set_trace( TSD, evaluate(TSD,this->p[0], &tptr ) );
               if ( tptr )
                  Free_stringTSD( tptr ) ;
            }
            else
               exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         }

         break ;
      }

      case X_EXIT:
      {
         int rc ;

         if (TSD->systeminfo->panic)
         {
            if (this->p[0])
               TSD->systeminfo->result = evaluate(TSD,this->p[0],NULL) ;
            else
               TSD->systeminfo->result = NULL ;

            TSD->instore_is_errorfree = 1 ;
            if (TSD->in_protected)
            {
               TSD->delayed_error_type = PROTECTED_DelayedSetjmpPanic;
               longjmp( TSD->protect_return, 1 ) ;
            }
            longjmp( *(TSD->systeminfo->panic), 1 ) ;
         }

         if ( this->p[0] == NULL )
            rc = EXIT_SUCCESS ;
         else
         {
            streng *retval,*kill; /* fixes bug 481482 */
            int error;

            retval = evaluate( TSD, this->p[0], &kill );

            /* fixes bug 657345 */
            rc = streng_to_int( TSD, retval, &error );
            if ( error )
               rc = EXIT_SUCCESS;
            if ( kill )
               Free_stringTSD( kill );
         }

         if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_TERMIN))
            hookup( TSD, HOOK_TERMIN ) ;

#if defined(FLISTS) && defined(NEW_FLISTS)
         free_flists();
#endif

#ifdef TRACEMEM
         if (TSD->listleakedmemory)
            listleaked( TSD, MEMTRC_LEAKED ) ;
#endif
         CloseOpenFiles( TSD ) ;
         if (TSD->in_protected)
         {
            TSD->delayed_error_type = PROTECTED_DelayedExit;
            TSD->expected_exit_error = rc;
            longjmp( TSD->protect_return, 1 ) ;
         }
         TSD->MTExit( rc ) ;
         break ;
      }

      case X_COMMAND:
      {
         streng *stmp,*kill;

         update_envirs( TSD, TSD->currlevel ) ;
         if (this->p[0]) {
            /* bja - added Free_stringTSD() around perform() */
            stmp = evaluate( TSD, this->p[0], &kill );
            Free_stringTSD(perform(TSD, stmp, TSD->currlevel->environment, this, NULL)) ;
            if ( kill )
               Free_stringTSD( kill );
            break ; }
      }

      case X_ADDR_N:   /* ADDRESS environment [expr] */
      {
         streng *envir,*tmp,*kill;
         update_envirs( TSD, TSD->currlevel ) ;
         envir = this->name ;
         if (this->p[0])
         {
            /* bja - added Free_stringTSD() around perform() */
            /* the IO-redirection is temporarily in this case. */
            tmp = evaluate( TSD, this->p[0], &kill );
            Free_stringTSD(perform(TSD, tmp, envir, this, this->p[1]));
            if ( kill )
               Free_stringTSD( kill ) ;
         }
         else
         {
            set_envir( TSD, envir, this->p[1] ) ;
            Free_stringTSD( TSD->currlevel->prev_env ) ;
            TSD->currlevel->prev_env = TSD->currlevel->environment ;
            TSD->currlevel->environment = Str_dupTSD(envir) ;
         }
         break ;
      }


      case X_ADDR_V:   /* ADDRESS [VALUE] expr */
      {
         streng *cptr ;

         if ( this->u.nonansi &&
              get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 2, "ADDRESS \"(\"...\")\"") ;

         update_envirs( TSD, TSD->currlevel ) ;
         cptr = evaluate(TSD,this->p[0],NULL) ;
         set_envir( TSD, cptr, this->p[1] ) ;
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
         for (nptr=this->p[0]; nptr; nptr=nptr->p[0] )
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
                               isspace(value->value[begin]))
                           begin++;
                        if (begin == Str_len(value))
                           break;
                        end = begin + 1; /* find next separator */
                        while ((end < Str_len(value)) &&
                               !isspace(value->value[end]))
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
         for (nptr=this->p[0]; nptr; nptr=nptr->p[0] )
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
                               isspace(value->value[begin]))
                           begin++;
                        if (begin == Str_len(value))
                           break;
                        end = begin + 1; /* find next separator */
                        while ((end < Str_len(value)) &&
                               !isspace(value->value[end]))
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
         type = identify_trap( this->p[1]->type ) ;

         /* We always set this */
         traps[type].invoked = (this->type == X_SIG_SET) ;
         traps[type].delayed = 0 ;
         traps[type].on_off = (this->p[0]->type == X_ON ) ;

         /* set the name of the variable to work on */
         FREE_IF_DEFINED( traps[type].name ) ;
         if (this->name)
            traps[type].name = Str_dupTSD( this->name ) ;
         else if (this->p[0]->type == X_ON)
            traps[type].name = Str_creTSD( signalnames[type] ) ;

         break ;
      }

      case X_SIG_VAL:
      case X_SIG_LAB:
      {
         streng *cptr, *kill=NULL;
         volatile char *tmp_str;
         stackelem *top;
         unsigned i;

         cptr = (this->name) ? this->name : evaluate( TSD, this->p[0], &kill );
         nstackcleanup( TSD, nstktrigger, NULL );
         top = stacktop( TSD );
         for ( i = stacktrigger( TSD ); i > stktrigger; i--, top = top->prev )
         {
            if ( top->increment == s.increment )
               s.increment = NULL;
            if ( top->stopval == s.stopval )
               s.stopval = NULL;
         }

         stackcleanup( TSD, stktrigger );
         /*
          * Fixes bug 764458
          */
         innerloop = NULL;

         set_sigl( TSD, this->lineno );
         entry = getlabel( TSD, cptr );
         /*
          * We have to make a temporary copy of the label we are signalling
          * in case it doesn't exist because the "kill" processing will destroy
          * the value.
          */
         tmp_str = tmpstr_of( TSD, cptr );

         if ( kill )
            Free_stringTSD( kill );

         if ( entry == NULL )
            exiterror( ERR_UNEXISTENT_LABEL, 1, tmp_str );
         if ( entry->u.trace_only )
            exiterror( ERR_UNEXISTENT_LABEL, 2, tmpstr_of( TSD, entry->name) );
         this = entry->next;
         goto fakerecurse;
      }
      case X_PROC:
      {
         treenode *ptr;

         if (TSD->currlevel->varflag)
             exiterror( ERR_UNEXPECTED_PROC, 0 )  ;

         for (ptr=this->p[0];(ptr);ptr=ptr->p[0])
         {
            if (ptr->name)
            {
               expose_var(TSD,ptr->name) ;
               if (ptr->type==X_IND_SYMBOL)
                  expose_indir( TSD, getvalue( TSD, ptr->name, 0 ) );
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
         n = getlabel( TSD, this->name );
         if ( n )
         {
            if ( n->u.trace_only )
               exiterror( ERR_UNEXISTENT_LABEL, 3, tmpstr_of( TSD, n->name ) );
            this->type = X_IS_INTERNAL;
         }
         else
            this->type = X_IS_BUILTIN;
         this->u.node = n;
         this->o.called = 1;
      }
      /* THIS IS MEANT TO FALL THROUGH! */
      case X_IS_INTERNAL:
      {
         paramboxptr args;
         streng *result;

         if ( this->u.node )
         {
            no_next_interactive = 1;
            args = initplist( TSD, this );
            set_sigl( TSD, this->lineno );

            result = CallInternalFunction( TSD, this->u.node, this, args );

            TSD->systeminfo->interactive = TSD->currlevel->traceint;

            set_result( TSD, result );
            break;
         }
      }
      /* THIS IS MEANT TO FALL THROUGH! */
      case X_EX_FUNC:
      case X_IS_BUILTIN:
      {
         streng *result ;

         if ((result = buildtinfunc( TSD, this )) == NOFUNC)
         {
            this->type = X_IS_EXTERNAL ;
         }
         else
         {
            set_result( TSD, result );

            break ;
         }
      }
      /* THIS IS MEANT TO FALL THROUGH! */
      case X_IS_EXTERNAL:
      {
         streng *ptr, *command ;
         paramboxptr args, targs ;

         if ( TSD->restricted )
            exiterror( ERR_RESTRICTED, 5 )  ;

         update_envirs( TSD, TSD->currlevel ) ;

         args = targs = initplist( TSD, this ) ;
         stackmark = pushcallstack( TSD, TSD->currentnode ) ;
         ptr = execute_external(TSD,
                                this->name,
                                args,
                                TSD->systeminfo->environment,
                                NULL,
                                        /* Fixes bug 604219                  */
                                TSD->systeminfo->hooks,
                                INVO_SUBROUTINE);
         popcallstack( TSD, stackmark ) ;

         if (ptr==this->name) /* FIXME,MH: no idea what this does */
         {
            set_result( TSD, NULL );
            ptr = NULL ;
         }

         if (!ptr)
         {
            /*
             * "this->name" wasn't an external Rexx program, so
             * see if it is an OS command
             * Only do this if the OPTIONS EXT_COMMANDS_AS_FUNCS is
             * set and STRICT_ANSI is NOT set.
             */
            if ( get_options_flag( TSD->currlevel, EXT_EXT_COMMANDS_AS_FUNCS )
            &&  !get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            {
               command = Str_makeTSD( 1000 ) ;
               command = Str_catTSD(command,this->name ) ;
               for (;targs;targs=targs->next)
               {
                  if (targs->value)
                  {
                     command = Str_catstrTSD(command," ") ;
                     command = Str_catTSD(command,targs->value) ;
                  }
               }
               ptr = run_popen( TSD, command, TSD->currlevel->environment ) ;
               Free_stringTSD( command ) ;
            }
            if (!ptr)
            {
               exiterror( ERR_ROUTINE_NOT_FOUND, 1, tmpstr_of( TSD, this->name ) ) ;
               ptr = nullstringptr() ;
            }
         }
         if (ptr)
            set_result( TSD, ptr );
         deallocplink( TSD, args ) ;

         break ;
      }

      case X_PARSE:
      {
         /*
          * We always have to produce a duplicate of the content we have to
          * parse. We can't use variable locking and we can't assume that
          * the content doesn't contain variable names used in the template.
          * This fixes bug 688503.
          */

         if ( this->u.parseflags & ( PARSE_LOWER | PARSE_CASELESS ) )
         {
            if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
               exiterror( ERR_NON_ANSI_FEATURE, 2,
                   ( this->u.parseflags & PARSE_LOWER ) ? "PARSE LOWER" :
                                                          "PARSE CASELESS" )  ;
         }
         if ( this->p[0]->type == X_PARSE_ARG )
         {
            parseargtree( TSD, TSD->currlevel->args, this->p[1],
                          this->u.parseflags );
         }
         else
         {
            streng *source = NULL;
            nodeptr templ;

            switch ( this->p[0]->type )
            {
               case X_PARSE_VAR:
                  /* must duplicate, parsing may have side effects */
                  /* else, we must have locking of variables */
                  source = Str_dupTSD( shortcut( TSD, this->p[0] ) );
                  break ;

               case X_PARSE_VAL:
               {
                  /*
                   * Must duplicate, parsing may have side effects, we must
                   * have locking of variables otherwise.
                   * fixes bug
                   */
                  source = evaluate( TSD, this->p[0]->p[0], NULL );
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
                  streng *origfile, *inpfile;

                  stype = system_type();
                  origfile = TSD->systeminfo->called_as;
                  inpfile = TSD->systeminfo->input_file;
                  source = Str_makeTSD( strlen( stype ) + 4 +
                           strlen( invo_strings[TSD->systeminfo->invoked] ) +
                           Str_len( origfile ) + Str_len( inpfile ) );
                  source->len = 0;

                  Str_catstrTSD( source, stype );
                  Str_catstrTSD( source, " " );
                  Str_catstrTSD( source, invo_strings[TSD->systeminfo->invoked] );
                  Str_catstrTSD( source, " " );
                  Str_catTSD(source,inpfile );
                  break;
               }
            }

            if ( this->u.parseflags & PARSE_UPPER )
            {
               Str_upper( source );
            }
            if ( this->u.parseflags & PARSE_LOWER )
            {
               Str_lower( source );
            }

            doparse( TSD, source, this->p[1],
               this->u.parseflags & PARSE_CASELESS );

            for ( templ = this->p[1]->next; templ != NULL; templ = templ->next )
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

         doparse(TSD, stmp=Str_upper(popline( TSD, NULL, NULL, 0 )), this->p[0], 0 ) ;
         Free_stringTSD( stmp ) ;
         break ;
      }

      case X_PUSH:
         stack_lifo( TSD, (this->p[0]) ? evaluate(TSD,this->p[0],NULL) : nullstringptr(), NULL ) ;
         break ;

      case X_QUEUE:
         stack_fifo( TSD, (this->p[0]) ? evaluate(TSD,this->p[0],NULL) : nullstringptr(), NULL ) ;
         break ;

      case X_OPTIONS:
         do_options(TSD, evaluate(TSD,this->p[0],NULL),0) ;
         break ;

      case X_RETURN:
      {
         stackelem *top;
         unsigned i;
         streng *retval;

         /* buggy, need to deallocate procbox and vars ... */
         if (this->p[0])
            retval = evaluate(TSD,this->p[0],NULL) ;
         else
            retval = NULL ;

         top = stacktop(TSD);
         for (i = stacktrigger(TSD);i > stktrigger;i--,top = top->prev)
         {
            if (top->increment == s.increment)
               s.increment = NULL;
            if (top->stopval == s.stopval)
               s.stopval = NULL;
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
            s.this = innerloop;
            stackpush( TSD, &s );
            Stacked++;
         }

         top = stacktop(TSD);
         for ( ;; ) /* while iteration counter name not found */
         {
            if ( Stacked <= 0 )
            {
               if ( innerloop )
                  exiterror( ERR_INVALID_LEAVE, (this->type==X_LEAVE)?3:4, tmpstr_of( TSD, this->name ) );
               else
                  exiterror( ERR_INVALID_LEAVE, (this->type==X_LEAVE)?1:2 );
            }

            iptr = top->this;

            if ( this->name == NULL )
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
                 ( Str_cmp( this->name, iptr->p[0]->name ) == 0 ) )
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
            if ( top->increment == s.increment )
               s.increment = NULL;
            stack_destroyelement( TSD, top );

            Stacked--;
            top = top->prev;
         }

         nstackcleanup(TSD,nstktrigger,&iptr);

         if (Stacked<=0)
            exiterror( ERR_INVALID_LEAVE, 0 );
         if (this->type==X_LEAVE)
         {
            popcallstack(TSD,-1) ;
            if (top->stopval == s.stopval )
               s.stopval = NULL ;
            if ( top->increment == s.increment )
               s.increment = NULL ;
            stack_destroyelement(TSD,top);
            Stacked--;
            top = top->prev;

            nstackpop(TSD);
         }
         TRACELINE(iptr) ;
         stackcleanup(TSD,stktrigger + Stacked);

         if (TSD->systeminfo->interactive)
         {
            if (intertrace(TSD))
               goto fakerecurse ;
         }

         this = nstackpop(TSD);

         if (Stacked)
         {
            s = stackpop(TSD);
            innerloop = s.this;
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

         cptr = evaluate( TSD, this->p[0], &kill );
         tmp = streng_to_int( TSD, cptr, &error );
         if ( error || tmp < 0 )
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
#if 0
/*
 * Remove unneccessary limitaion on numeric digits as suggested by
 * Patrick McPhee
 */
         if (tmp > MAXNUMERIC)
             exiterror( ERR_INVALID_RESULT, 2, tmp, MAXNUMERIC )  ;
#endif
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

         cptr = evaluate( TSD, this->p[0], &kill );
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
         if (this->p[0]->type == X_NUM_SCI)
            TSD->currlevel->numform = NUM_FORM_SCI ;
         else if (this->p[0]->type == X_NUM_ENG)
            TSD->currlevel->numform = NUM_FORM_ENG ;
         else
            assert( 0 ) ;
         break ;
      }

      case X_NUM_V:
      {
         streng *tmpstr,*kill;
         int len;
         char *s;

         tmpstr = evaluate( TSD, this->p[0], &kill );
         len = tmpstr->len;
         s = tmpstr->value;

         if ( ( len == 10 ) && ( mem_cmpic( s, "SCIENTIFIC", 10 ) == 0 ) )
            TSD->currlevel->numform = NUM_FORM_SCI ;
         else if ( ( len == 11 ) && ( mem_cmpic( s, "ENGINEERING", 11 ) == 0 ) )
            TSD->currlevel->numform = NUM_FORM_ENG ;
         else if ( ( len == 1 ) && ( toupper( *s ) == 'S' ) )
            TSD->currlevel->numform = NUM_FORM_SCI ;
         else if ( ( len == 1 ) && ( toupper( *s ) == 'E' ) )
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

   if (this)
      this = this->next ;

fakereturn:
   if (!this)
   {
      if (nstacktrigger(TSD) <= nstktrigger)
      {
         stackcleanup(TSD,stktrigger);
         return NULL ;
      }
      else
         this = nstackpop(TSD);
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
 *             FREE_IF_DEFINED(stack[stkidx].increment) ;
 *             FREE_IF_DEFINED(stack[stkidx].stopval) ;
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
            FREE_IF_DEFINED( TSD->currlevel->sig->info ) ;
            FREE_IF_DEFINED( TSD->currlevel->sig->descr ) ;
            FreeTSD( TSD->currlevel->sig ) ;
         }
         TSD->currlevel->sig = TSD->nextsig ;
         TSD->nextsig = NULL ;

         /* simulate the SIGNAL statement */
         entry = getlabel( TSD, traps[i].name ) ;
         set_sigl( TSD, TSD->currlevel->sig->lineno ) ;
         if (TSD->currlevel->sig->type == SIGNAL_SYNTAX )
            set_rc( TSD, int_to_streng( TSD, TSD->currlevel->sig->rc ) );

         if ( entry == NULL )
            exiterror( ERR_UNEXISTENT_LABEL, 1, tmpstr_of( TSD, traps[i].name ) );
         if ( entry->u.trace_only )
            exiterror( ERR_UNEXISTENT_LABEL, 2, tmpstr_of( TSD, entry->name ) );
         this = entry;
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

         set_sigl( TSD, TSD->nextsig->lineno );
         oldlevel = TSD->currlevel;
         TSD->currlevel = newlevel( TSD, TSD->currlevel );
         TSD->currlevel->sig = TSD->nextsig;
         TSD->nextsig = NULL;

         stackmark = pushcallstack( TSD, this );
         TRACELINE( entry );

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

      ipt->sort_labels = MallocTSD(ipt->numlabels * sizeof(ipt->sort_labels[0]));
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

   FREE_IF_DEFINED(level->buf) ;

   if (level->sig)
   {
      FREE_IF_DEFINED( level->sig->info ) ;
      FREE_IF_DEFINED( level->sig->descr ) ;
      FreeTSD( level->sig ) ;
   }

   if (level->traps)
   {
      for (i=0; i<SIGNALS; i++)
         FREE_IF_DEFINED( level->traps[i].name ) ;

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
   proclevel level=NULL ;
   int i=0 ;

   level = (proclevel)MallocTSD(sizeof(proclevbox)) ;

   if (!oldlevel)
   {
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
      memset(level,0,sizeof(proclevbox));
#endif
      level->numfuzz = DEFAULT_NUMERIC_FUZZ ;
      level->currnumsize = DEFAULT_NUMERIC_SIZE ;
      level->numform = DEFAULT_NUMFORM ;
      level->time.sec = 0 ;
      level->time.usec = 0 ;
      level->mathtype = DEFAULT_MATH_TYPE ;
      level->prev = NULL ;
      level->next = NULL ;
      level->args = NULL ;
      memset(level->u.flags,0,sizeof(level->u.flags));

      set_options_flag( level, EXT_LINEOUTTRUNC, DEFAULT_LINEOUTTRUNC ) ;
      set_options_flag( level, EXT_FLUSHSTACK, DEFAULT_FLUSHSTACK ) ;
      set_options_flag( level, EXT_MAKEBUF_BIF, DEFAULT_MAKEBUF_BIF ) ;
      set_options_flag( level, EXT_DROPBUF_BIF, DEFAULT_DROPBUF_BIF ) ;
      set_options_flag( level, EXT_DESBUF_BIF, DEFAULT_DESBUF_BIF ) ;
      set_options_flag( level, EXT_BUFTYPE_BIF, DEFAULT_BUFTYPE_BIF ) ;
      set_options_flag( level, EXT_CACHEEXT, DEFAULT_CACHEEXT ) ;
      set_options_flag( level, EXT_PRUNE_TRACE, DEFAULT_PRUNE_TRACE ) ;
      set_options_flag( level, EXT_EXT_COMMANDS_AS_FUNCS, DEFAULT_EXT_COMMANDS_AS_FUNCS ) ;
      set_options_flag( level, EXT_STDOUT_FOR_STDERR, DEFAULT_STDOUT_FOR_STDERR ) ;
      set_options_flag( level, EXT_TRACE_HTML, DEFAULT_TRACE_HTML ) ;
      set_options_flag( level, EXT_FAST_LINES_BIF_DEFAULT, DEFAULT_FAST_LINES_BIF_DEFAULT ) ;
      set_options_flag( level, EXT_STRICT_ANSI, DEFAULT_STRICT_ANSI ) ;
      set_options_flag( level, EXT_INTERNAL_QUEUES, DEFAULT_INTERNAL_QUEUES ) ;
      set_options_flag( level, EXT_REGINA_BIFS, DEFAULT_REGINA_BIFS ) ;
      set_options_flag( level, EXT_STRICT_WHITE_SPACE_COMPARISONS, DEFAULT_STRICT_WHITE_SPACE_COMPARISONS ) ;
      set_options_flag( level, EXT_AREXX_SEMANTICS, DEFAULT_AREXX_SEMANTICS ) ;
      set_options_flag( level, EXT_AREXX_BIFS, DEFAULT_AREXX_BIFS ) ;
      set_options_flag( level, EXT_BROKEN_ADDRESS_COMMAND, DEFAULT_BROKEN_ADDRESS_COMMAND ) ;

      level->varflag = 1 ;
      level->tracestat = (char) TSD->systeminfo->tracing ;
      level->traceint = (char) TSD->systeminfo->interactive ; /* MDW 30012002 */
      level->environment = Str_dupTSD( TSD->systeminfo->environment ) ;
      level->prev_env = Str_dupTSD( TSD->systeminfo->environment ) ;
      level->vars = create_new_varpool( TSD ) ;
      level->buf = NULL ;
      level->sig = NULL ;
      level->traps = MallocTSD( sizeof(trap) * SIGNALS ) ;
#ifdef __CHECKER__
      /* See above */
      memset( level->traps, 0, sizeof(trap) * SIGNALS ) ;
#endif
      for (i=0; i<SIGNALS; i++)
      {
         level->traps[i].name = NULL ;
         level->traps[i].on_off = 0 ;
         level->traps[i].delayed = 0 ;
         level->traps[i].def_act = default_action[i] ;
         level->traps[i].ignored = default_ignore[i] ;
         level->traps[i].invoked = 0 ;
      }
   }
   else
   {
      /* Stupid SunOS acc gives incorrect warning for the next line */
      memcpy(level,oldlevel,sizeof(proclevbox)) ;
#ifdef DONT_DO_THIS
      level->prev_env = NULL ;
      level->environment = NULL ;
#else
      level->prev_env = Str_dupTSD( oldlevel->prev_env ) ;
      level->environment = Str_dupTSD( oldlevel->environment ) ;
#endif
      level->prev = oldlevel ;
      level->varflag = 0 ;
      oldlevel->next = level ;
      level->buf = NULL ;
      level->args = NULL ;
/*    level->next = NULL ;*/
      level->sig = NULL ;
      level->traps = NULL ;
   }

   TSD->trace_stat = level->tracestat ;
   return( level ) ;
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
      for (; cptr<eptr && isspace(*cptr); cptr++ ) ;
      for (sptr=cptr; cptr<eptr && !isspace(*cptr); cptr++ ) ;
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
