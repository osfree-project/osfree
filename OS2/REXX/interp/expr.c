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

#include <string.h>
#include <assert.h>

#define TRACEVALUE(a,b) if (TSD->trace_stat=='I') tracevalue(TSD,a,b)

#ifdef TRACEMEM
static void mark_in_expr( const tsd_t *TSD )
{
   if (TSD->rdes.num)
      markmemory( TSD->rdes.num, TRC_STATIC ) ;
   if (TSD->ldes.num)
      markmemory( TSD->ldes.num, TRC_STATIC ) ;
}
#endif

/*
 * COMP_IGNORE returns 1 if c shall be ignored on a non-strict comparison
 * between non-numbers. Previously, this was equal to isspace(c). This was
 * wrong according to ANSI, section 7.4.7. This fixes bug 594674.
 */
#define ANSI_COMP_IGNORE(c) ( (c) == ' ' )
#define REGINA_COMP_IGNORE(c) ( rx_isspace(c) )

#define FREE_TMP_STRING(str) if ( str )               \
                                Free_stringTSD( str )
static char *getoperator( int type )
{
   char *retstr;
   switch ( type )
   {
      case X_PLUSS:
      case X_U_PLUSS: retstr = "+"; break;
      case 0:
      case 255:
      case X_MINUS:
      case X_U_MINUS:
         retstr = "-"; break;
      case X_MULT: retstr = "*"; break;
      case X_DEVIDE: retstr = "/"; break;
      case X_MODULUS: retstr = "//"; break;
      case X_INTDIV: retstr = "%"; break;
      case X_EXP: retstr = "**"; break;
      case X_PLUSASSIGN: retstr = "+="; break;
      case X_MINUSASSIGN: retstr = "-="; break;
      case X_MULTASSIGN: retstr = "*="; break;
      case X_DIVASSIGN: retstr = "*="; break;
      case X_INTDIVASSIGN: retstr = "%="; break;
      case X_MODULUSASSIGN: retstr = "//="; break;
      case X_ANDASSIGN: retstr = "&="; break;
      case X_ORASSIGN: retstr = "|="; break;
      case X_XORASSIGN: retstr = "&&="; break;
      case X_CONCATASSIGN: retstr = "||="; break;
      default: retstr = "unknown"; break;
   }
   return retstr;
}
int init_expr( tsd_t *TSD )
{
#ifdef TRACEMEM
   regmarker( TSD, mark_in_expr ) ;
#endif
   TSD = TSD; /* keep compiler happy */
   return(1);
}


static num_descr *copy_num( const tsd_t *TSD, const num_descr *input )
{
   num_descr *newptr=NULL ;

   newptr = (num_descr *)MallocTSD( sizeof( num_descr )) ;
   newptr->negative = input->negative ;
   newptr->size = input->size ;
   newptr->max = (input->max < 1) ? 1 : input->max ;
   newptr->exp = input->exp ;
   newptr->num = (char *)MallocTSD( newptr->max ) ;
   newptr->used_digits = input->used_digits;
   memcpy( newptr->num, input->num, newptr->size ) ;
   TSD = TSD; /* keep compiler happy */
   return newptr ;
}


static streng *num_to_str( const tsd_t *TSD, num_descr *input )
{
   return str_norm( TSD, input, NULL ) ;
}

static int num_to_bool( const num_descr *input )
{
   char ch=' ' ;

   if (input==NULL)
       exiterror( ERR_UNLOGICAL_VALUE, 0 )  ;

   if (input->size!=1 || input->negative || input->exp!=1)
       exiterror( ERR_UNLOGICAL_VALUE, 0 )  ;

    ch = input->num[0] ;
    if (ch!='0' && ch!='1')
        exiterror( ERR_UNLOGICAL_VALUE, 0 )  ;

    return ch=='1' ;
}

static int str_to_bool( const streng *input )
{
   char ch=' ' ;

   if (input->len!=1)
       exiterror( ERR_UNLOGICAL_VALUE, 0 )  ;

   ch = input->value[0] ;
   if (ch!='0' && ch!='1')
       exiterror( ERR_UNLOGICAL_VALUE, 0 )  ;

   return ch == '1' ;
}

static streng *bool_to_str( const tsd_t *TSD, int input )
{
   return Str_creTSD( input ? "1" : "0" ) ;
}

static num_descr *bool_to_num( const tsd_t *TSD, int input )
{
   num_descr *num=NULL ;

   num = (num_descr *)MallocTSD( sizeof( num_descr )) ;
   num->max = 8 ;
   num->num = (char *)MallocTSD( 8 ) ;
   num->size = 1 ;
   num->negative = 0 ;
   num->exp = 1 ;
   num->num[0] = (char) ((input) ? '1' : '0') ;
   num->used_digits = TSD->currlevel->currnumsize;
   return num ;
}

/*
 * calcul evaluates a numeric expression. thisptr is the current evaluation tree.
 * kill? return value?
 * Note: This is one of the most time-consuming routines. Be careful.
 * The operator arg is the initial arithmetic operator when called from outside. It is
 * overridden by the type of the pointer only for arithmetic operators so the most recent
 * operator is reported when an error occurs
 */
num_descr *calcul( tsd_t *TSD, nodeptr thisptr, num_descr **kill, int side, int operator )
{
   num_descr *numthr, *numone, *numtwo ;
   num_descr *ntmp1=NULL, *ntmp2=NULL ;
   num_descr *nptr;
   streng *sptr;
   int strip2 = 0; /* fixes bug 1107763, second part */

   switch ( thisptr->type )
   {
      case 0:
      case 255:
      case X_MINUS:
      case X_MINUSASSIGN:
         operator = thisptr->type;
         numone = calcul( TSD, thisptr->p[0], &ntmp1, SIDE_LEFT, operator ) ;
         numtwo = calcul( TSD, thisptr->p[1], &ntmp2, SIDE_RIGHT, operator ) ;
         if (!ntmp2)
            ntmp2 = numtwo = copy_num( TSD, numtwo ) ;

         numtwo->negative = !numtwo->negative ;
         goto do_an_add ;

      case X_PLUSS:
      case X_PLUSASSIGN:
         operator = thisptr->type;
         numone = calcul( TSD, thisptr->p[0], &ntmp1, SIDE_LEFT, operator ) ;
         numtwo = calcul( TSD, thisptr->p[1], &ntmp2, SIDE_RIGHT, operator ) ;
do_an_add:
         if (ntmp1)
         {
            numthr = numone ;
            ntmp1 = NULL ;
         }
         else if (ntmp2)
         {
            numthr = numtwo ;
            ntmp2 = NULL ;
         }
         else
            numthr = copy_num( TSD, numtwo ) ;

         string_add( TSD, numone, numtwo, numthr, thisptr->p[0], thisptr->p[1] ) ;
         break ;

      case X_MULT:
      case X_MULTASSIGN:
         operator = thisptr->type;
         numone = calcul( TSD, thisptr->p[0], &ntmp1, SIDE_LEFT, operator ) ;
         numtwo = calcul( TSD, thisptr->p[1], &ntmp2, SIDE_RIGHT, operator ) ;
         if (ntmp1)
         {
            numthr = numone ;
            ntmp1 = NULL ;
         }
         else if (ntmp2)
         {
            numthr = numtwo ;
            ntmp2 = NULL ;
         }
         else
            numthr = copy_num( TSD, numtwo ) ;

         string_mul( TSD, numone, numtwo, numthr, thisptr->p[0], thisptr->p[1] );
         break ;

      case X_DEVIDE:
      case X_MODULUS:
      case X_INTDIV:
         operator = thisptr->type;
         numone = calcul( TSD, thisptr->p[0], &ntmp1, SIDE_LEFT, operator ) ;
         numtwo = calcul( TSD, thisptr->p[1], &ntmp2, SIDE_RIGHT, operator ) ;
         if (numtwo->size==1 && numtwo->num[0]=='0')
             exiterror( ERR_ARITH_OVERFLOW, 3 )  ;

         numthr = copy_num( TSD, numtwo ) ;
         string_div( TSD, numone, numtwo, numthr, NULL,
            ((thisptr->type==X_DEVIDE) ? DIVTYPE_NORMAL :
            ((thisptr->type==X_MODULUS) ? DIVTYPE_REMAINDER : DIVTYPE_INTEGER)),
            thisptr->p[0], thisptr->p[1] );
         strip2 = 1;
         break ;
      case X_DIVASSIGN:
      case X_INTDIVASSIGN:
      case X_MODULUSASSIGN:
         operator = thisptr->type;
         numone = calcul( TSD, thisptr->p[0], &ntmp1, SIDE_LEFT, operator ) ;
         numtwo = calcul( TSD, thisptr->p[1], &ntmp2, SIDE_RIGHT, operator ) ;
         if (numtwo->size==1 && numtwo->num[0]=='0')
             exiterror( ERR_ARITH_OVERFLOW, 3 )  ;

         numthr = copy_num( TSD, numtwo ) ;
         string_div( TSD, numone, numtwo, numthr, NULL,
            ((thisptr->type==X_DIVASSIGN) ? DIVTYPE_NORMAL :
            ((thisptr->type==X_MODULUSASSIGN) ? DIVTYPE_REMAINDER : DIVTYPE_INTEGER)),
            thisptr->p[0], thisptr->p[1] );
         strip2 = 1;
         break ;

      case X_EXP:
         operator = thisptr->type;
         numone = calcul( TSD, thisptr->p[0], &ntmp1, SIDE_LEFT, operator ) ;
         numtwo = ntmp2 = calcul( TSD, thisptr->p[1], NULL, SIDE_RIGHT, operator ) ;
         numthr = copy_num( TSD, numone ) ;
         string_pow( TSD, numone, numtwo, numthr, thisptr->p[0], thisptr->p[1] ) ;
         strip2 = 1;
         break ;

      case X_STRING:
      case X_CON_SYMBOL:
         if ( !thisptr->u.number )
            thisptr->u.number = get_a_descr( TSD, NULL, 0, thisptr->name ) ;

         if (TSD->trace_stat=='I')
            tracenumber( TSD, thisptr->u.number, 'L' ) ;

         if (kill)
         {
            *kill = NULL ;
            return thisptr->u.number ;
         }
         else
            return copy_num( TSD, thisptr->u.number ) ;

      case X_SIM_SYMBOL:
      case X_STEM_SYMBOL:
         if (kill)
            *kill = NULL ;

         nptr = shortcutnum( TSD, thisptr ) ;
         if (!nptr)
         {
            /* get value of symbol for error reporting */
            sptr = evaluate( TSD, thisptr, NULL ) ;
            exiterror( ERR_BAD_ARITHMETIC, side, tmpstr_of( TSD, sptr ), getoperator( operator ) );
         }

         if (kill)
            return nptr ;
         else
            return copy_num( TSD, nptr ) ;

      case X_HEAD_SYMBOL:
         if (kill)
            *kill = NULL ;

         nptr = fix_compoundnum( TSD, thisptr, NULL, NULL );
         if (!nptr)
             exiterror( ERR_BAD_ARITHMETIC, 0 )  ;

         if (kill)
            return nptr ;
         else
            return copy_num( TSD, nptr ) ;

      case X_U_PLUSS:
      case X_U_MINUS:
         operator = thisptr->type;
         numthr = calcul( TSD, thisptr->p[0], &ntmp1, SIDE_RIGHT, operator ) ;
         if (!ntmp1)
            numthr = copy_num( TSD, numthr ) ;

         if (thisptr->type==X_U_MINUS)
            numthr->negative = !numthr->negative ;

         if (kill)
            *kill = numthr ;

         if (TSD->trace_stat=='I')
            tracenumber( TSD, numthr, 'P' ) ;

         return numthr ;

      case X_IN_FUNC:
      case X_IS_INTERNAL:
      case X_IS_BUILTIN:
      case X_EX_FUNC:
      case X_IS_EXTERNAL:
      case X_CONCAT:
      case X_SPACE:
      {
         numthr = get_a_descr( TSD, NULL, 0, evaluate( TSD, thisptr, &sptr ) );
         FREE_TMP_STRING( sptr );
         if (kill)
            *kill = numthr ;
         return numthr ;
      }
      case X_LOG_NOT:
      case X_LOG_OR:
      case X_LOG_AND:
      case X_LOG_XOR:
      case X_S_DIFF:
      case X_S_EQUAL:
      case X_EQUAL:
      case X_GT:
      case X_LT:
      case X_GTE:
      case X_LTE:
      case X_DIFF:
      case X_SEQUAL:
      case X_SGT:
      case X_SLT:
      case X_SGTE:
      case X_SLTE:
      case X_SDIFF:
      case X_NEQUAL:
      case X_NGT:
      case X_NLT:
      case X_NGTE:
      case X_NLTE:
      case X_NDIFF:
      case X_S_NGT:
      case X_S_NLT:
      case X_S_GT:
      case X_S_GTE:
      case X_S_LT:
      case X_S_LTE:
      case X_ORASSIGN:
      case X_XORASSIGN:
      case X_ANDASSIGN:
         numthr = bool_to_num( TSD, isboolean( TSD, thisptr, 0, NULL )) ;
         if (kill)
            *kill = numthr ;
         return numthr ;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         return NULL ;
   }

   if (ntmp1)
   {
      FreeTSD( numone->num ) ;
      FreeTSD( numone ) ;
   }
   if (ntmp2)
   {
      FreeTSD( numtwo->num ) ;
      FreeTSD( numtwo ) ;
   }
   if (kill)
      *kill = numthr ;

   if (TSD->trace_stat=='I')
      tracenumber( TSD, numthr, 'O' ) ;

   str_strip( numthr ) ;
   str_round( numthr, TSD->currlevel->currnumsize ) ;
   if ( strip2 )
   {
      /*
       * ANSI 7.4.10, PostOp, add. rounding for / and **
       */
      strip2 = numthr->size;
      while ( ( strip2 > 1 ) &&
              ( numthr->exp < strip2) &&
              ( numthr->num[strip2 - 1] == '0' ) )
         strip2--;
      if ( strip2 != numthr->size )
      {
         numthr->size = strip2;
         if ( strip2 < numthr->used_digits )
            numthr->used_digits = strip2;
      }
   }
   return numthr ;
}

static void strip_whitespace( tsd_t *TSD, unsigned char **s1,
                              unsigned char **e1, unsigned char **s2,
                              unsigned char **e2 )
{
   if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI )
   ||   get_options_flag( TSD->currlevel, EXT_STRICT_WHITE_SPACE_COMPARISONS ) )
   {
      /*
       * ANSI 7.4.7 behaviour; non-strict comparisons
       * removed ONLY spaces, so single TAB not equal to single SPACE
       */
      /*
       * Strip leading spaces - ignored in comparison
       */
      for( ; ( *s1 < *e1 ) && ANSI_COMP_IGNORE( **s1 ); (*s1)++ )
      {
      }
      for( ; ( *s2 < *e2 ) && ANSI_COMP_IGNORE( **s2 ); (*s2)++ )
      {
      }
      for ( ; ( *s1 < *e1 ) && ( *s2 < *e2 ) && ( **s1 == **s2 ); (*s1)++, (*s2)++ )
      {
      }
      /*
       * Strip trailing spaces - ignored in comparison
       */
      for ( ; ( *e1 > *s1 ) && ANSI_COMP_IGNORE( *( *e1 - 1 ) ); (*e1)-- )
      {
      }
      for ( ; ( *e2 > *s2 ) && ANSI_COMP_IGNORE( *( *e2 - 1 ) ); (*e2)-- )
      {
      }
   }
   else
   {
      /*
       * Original Regina behaviour; non-strict comparisons
       * removed ALL white space, so single TAB equalled single SPACE
       */
      /*
       * Strip leading white space - ignored in comparison
       */
      for( ; ( *s1 < *e1 ) && REGINA_COMP_IGNORE( **s1 ); (*s1)++ )
      {
      }
      for( ; ( *s2 < *e2 ) && REGINA_COMP_IGNORE( **s2 ); (*s2)++ )
      {
      }
      for ( ; ( *s1 < *e1 ) && ( *s2 < *e2 ) && ( **s1 == **s2 ); (*s1)++, (*s2)++ )
      {
      }
      /*
       * Strip trailing white space - ignored in comparison
       */
      for ( ; ( *e1 > *s1 ) && REGINA_COMP_IGNORE( *( *e1 - 1 ) ); (*e1)-- )
      {
      }
      for ( ; ( *e2 > *s2 ) && REGINA_COMP_IGNORE( *( *e2 - 1 ) ); (*e2)-- )
      {
      }
   }
}


/*
 * evaluate evaluates an expression. The nodeptr "thisptr" must point to an
 * expression part. The return value is the value of the expression.
 * For a proper cleanup the caller probably has to delete the returned
 * value. For this purpose, the caller may set "kill" to non-NULL.
 * *kill is set to NULL, if the returned value is a const value and must
 * not be freed. *kill is set to a temporary value which has to be deleted
 * after the use of the returned value.
 * The caller may omit kill, this forces evaluate to create a freshly allocated
 * return value.
 */
streng *evaluate( tsd_t *TSD, nodeptr thisptr, streng **kill )
{
#define RETURN_NEW(val) if ( kill )     \
                           *kill = val; \
                        return val;
   streng *strone,*strtwo,*strthr;
   streng *stmp1,*stmp2;
   const streng *cstmp;
   num_descr *ntmp;

   if ( kill )
      *kill = NULL;
   switch ( thisptr->type )
   {
      case 0:
      case 255:
      case X_PLUSS:
      case X_MINUS:
      case X_MULT:
      case X_DEVIDE:
      case X_MODULUS:
      case X_INTDIV:
      case X_EXP:
      case X_U_MINUS:
      case X_U_PLUSS:
         ntmp = NULL;
         stmp1 = num_to_str( TSD, calcul( TSD, thisptr, &ntmp, SIDE_LEFT, thisptr->type ) );
         if ( ntmp )
         {
            FreeTSD( ntmp->num );
            FreeTSD( ntmp );
         }
         RETURN_NEW( stmp1 );

      case X_NULL:
         return NULL ;

      case X_STRING:
      case X_CON_SYMBOL:
         cstmp = thisptr->name;
         if ( TSD->trace_stat == 'I' )
            tracevalue( TSD, cstmp, 'L' );
         if ( kill )
            return (streng *) cstmp; /* and *kill is set to NULL above */
         stmp1 = Str_dupTSD( cstmp );
         RETURN_NEW( stmp1 );

      case X_HEAD_SYMBOL:
         /* always duplicate, since stmp1 might point to tmp area */
         stmp1 = Str_dupTSD( fix_compound( TSD, thisptr, NULL ) );
         RETURN_NEW( stmp1 );

      case X_STEM_SYMBOL:
      case X_SIM_SYMBOL:
         cstmp = shortcut(TSD,thisptr) ;
         if ( kill )
            return (streng *) cstmp; /* and *kill is set to NULL above */
         stmp1 = Str_dupTSD( cstmp );
         RETURN_NEW( stmp1 );

      case X_IN_FUNC:
      {
         nodeptr entry;

         if ( ( entry = getlabel( TSD, thisptr->name ) ) != NULL )
         {
            if ( entry->u.trace_only )
               exiterror( ERR_UNEXISTENT_LABEL, 3, tmpstr_of( TSD, thisptr->name ) );
            thisptr->type = X_IS_INTERNAL;
            thisptr->u.node = entry;
         }
         else
            thisptr->u.node = NULL;
      }

      case X_IS_INTERNAL:
      {
         nodeptr entry ;
         paramboxptr args ;
         streng *ptr ;

         /*
          * Check if the internal function name ends with a '.'.
          * This is an error in the ANSI standard, but it is possible
          * that existing code allows this, so only generate an error
          * if STRICT_ANSI OPTION is set.
          */
         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
         {
            if ( thisptr->name->value[(thisptr->name->len)-1] == '.' )
               exiterror( ERR_UNQUOTED_FUNC_STOP, 1, tmpstr_of( TSD, thisptr->name ) )  ;
         }
         if ( ( entry = thisptr->u.node ) != NULL )
         {
            /*
             * We are in an internal routine...
             */
            traceline( TSD, entry, TSD->trace_stat, 0 );
            set_reserved_value( TSD, POOL0_SIGL, NULL, TSD->currentnode->lineno, VFLAG_NUM );
            args = initplist( TSD, thisptr );

            ptr = CallInternalFunction( TSD, entry->next, TSD->currentnode,
                                        args );

            if (ptr==NULL) /* fixes bug 592393 */
               exiterror( ERR_NO_DATA_RETURNED, 1, tmpstr_of( TSD, thisptr->name ) );

            if (TSD->trace_stat=='I')
               tracevalue( TSD, ptr, 'F' );

            RETURN_NEW( ptr );
         }
      }
      /* THIS IS MEANT TO FALL THROUGH! */
      case X_IS_BUILTIN:
      case X_EX_FUNC:
      {
         streng *ptr ;

         if ((ptr=buildtinfunc( TSD, thisptr )) != NOFUNC)
         {
            if (thisptr->type != X_IS_BUILTIN)
               thisptr->type = X_IS_BUILTIN ;

            if (!ptr)
                exiterror( ERR_NO_DATA_RETURNED, 1, tmpstr_of( TSD, thisptr->name ) )  ;

            if (TSD->trace_stat=='I')
               tracevalue( TSD, ptr, 'F' ) ;

            RETURN_NEW( ptr );
         }
         else
            thisptr->type = X_IS_EXTERNAL ;
      }
      /* THIS IS MEANT TO FALL THROUGH! */
      case X_IS_EXTERNAL:
      {
         streng *ptr, *command;
         int stackmark,len,err;
         paramboxptr args, targs;

         if ( TSD->restricted )
            exiterror( ERR_RESTRICTED, 5 );

         update_envirs( TSD, TSD->currlevel );

         args = initplist( TSD, thisptr );
         stackmark = pushcallstack( TSD, TSD->currentnode );
         ptr = execute_external( TSD, thisptr->name,
                                 args,
                                 TSD->systeminfo->environment,
                                 &err,
                                 TSD->systeminfo->hooks,
                                 INVO_FUNCTION );
         popcallstack( TSD, stackmark );

         if ( err == -ERR_PROG_UNREADABLE )
         {
            /*
             * "thisptr->name" wasn't a Rexx program, so
             * see if it is an OS command.
             * Only do thisptr if the OPTIONS EXT_COMMANDS_AS_FUNCS is
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

         if ( err == -ERR_PROG_UNREADABLE )
         {
            exiterror( ERR_ROUTINE_NOT_FOUND, 1, tmpstr_of( TSD, thisptr->name ) );
         }
         else if ( err )
         {
            post_process_system_call( TSD, thisptr->name, -err, NULL, thisptr );
         }

         if ( !ptr )
            exiterror( ERR_NO_DATA_RETURNED, 1, tmpstr_of( TSD, thisptr->name ) );

         RETURN_NEW( ptr );
      }

      case X_CONCAT:
      case X_CONCATASSIGN:
      case X_SPACE:
      {
         char *cptr ;

         strone = evaluate( TSD, thisptr->p[0], &stmp1 ) ;
         if ( thisptr->type == X_CONCATASSIGN )
         {
            strtwo = (thisptr->p[1]) ? evaluate( TSD, thisptr->p[1], &stmp2 ) : nullstringptr() ;
            stmp2 = NULL;
         }
         else
            strtwo = evaluate( TSD, thisptr->p[1], &stmp2 ) ;
         strthr = Str_makeTSD(Str_len(strone)+Str_len(strtwo)+1) ;
         cptr = strthr->value ;
         memcpy( cptr, strone->value, strone->len ) ;
         cptr += strone->len ;
         if (thisptr->type==X_SPACE)
            *(cptr++) = ' ' ;

         memcpy( cptr, strtwo->value, strtwo->len ) ;
         strthr->len = (cptr-strthr->value) + strtwo->len ;

         if ( stmp1) FREE_TMP_STRING( stmp1 );
         if ( stmp2) FREE_TMP_STRING( stmp2 );

         if (TSD->trace_stat=='I')
            tracevalue( TSD, strthr, 'O' ) ;


         RETURN_NEW( strthr );
      }


      case X_LOG_NOT:
      case X_LOG_OR:
      case X_LOG_AND:
      case X_LOG_XOR:
      case X_S_DIFF:
      case X_S_EQUAL:
      case X_EQUAL:
      case X_GT:
      case X_LT:
      case X_GTE:
      case X_LTE:
      case X_DIFF:
      case X_SEQUAL:
      case X_SGT:
      case X_SLT:
      case X_SGTE:
      case X_SLTE:
      case X_SDIFF:
      case X_NEQUAL:
      case X_NGT:
      case X_NLT:
      case X_NGTE:
      case X_NLTE:
      case X_NDIFF:
      case X_S_NGT:
      case X_S_NLT:
      case X_S_GT:
      case X_S_GTE:
      case X_S_LT:
      case X_S_LTE:
         stmp1 = bool_to_str( TSD, isboolean( TSD, thisptr, 0, NULL )) ;
         RETURN_NEW( stmp1 );

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         return NULL ;
   }
#undef RETURN_NEW
}


/*
 * isboolean evaluates a boolean expression and returns 0 for false, another
 * value for true. "thisptr" is the current evaluation tree.
 * Note: This is one of the most time-consuming routines. Be careful.
 */
int isboolean( tsd_t *TSD, nodeptr thisptr, int suberror, const char *op )
{
   streng *strone,*strtwo;
   streng *stmp1,*stmp2;
   int tmp,sint;
   num_descr *ntmp;

   switch ( thisptr->type )
   {
      case 0:
      case 255:
      case X_PLUSS:
      case X_MINUS:
      case X_MULT:
      case X_DEVIDE:
      case X_MODULUS:
      case X_INTDIV:
      case X_EXP:
      case X_U_MINUS:
      case X_U_PLUSS:
         ntmp = NULL;
         tmp = num_to_bool( calcul( TSD, thisptr, &ntmp, SIDE_LEFT, thisptr->type )) ;
         if (ntmp)
         {
            FreeTSD( ntmp->num ) ;
            FreeTSD( ntmp ) ;
         }
         return tmp ;

      case X_STRING:
      case X_CON_SYMBOL:
         if ( !thisptr->u.number )
            thisptr->u.number = get_a_descr( TSD, NULL, 0, thisptr->name ) ;
         if ( Str_len( thisptr->name ) != 1 )
         {
            /* fixes bug 1111931, "01" is not a logical value in ANSI */
            if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            {
               if ( op )
               {
                  exiterror( ERR_UNLOGICAL_VALUE, suberror, op, tmpstr_of( TSD, thisptr->name ) );
               }
               else
               {
                  exiterror( ERR_UNLOGICAL_VALUE, suberror, tmpstr_of( TSD, thisptr->name ) );
               }
            }
         }
         return num_to_bool( thisptr->u.number ) ;

      case X_SIM_SYMBOL:
      case X_STEM_SYMBOL:
         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
         {
            /* fixes bug 1111931 */
            stmp1 = (streng *) shortcut( TSD, thisptr );
            tmp = Str_val( stmp1 )[0] - '0';
            if ( ( Str_len( stmp1 ) != 1 ) || ( ( tmp != 0 ) && ( tmp != 1 ) ) )
            {
               if ( op )
               {
                  exiterror( ERR_UNLOGICAL_VALUE, suberror, op, tmpstr_of( TSD, stmp1 ) );
               }
               else
               {
                  exiterror( ERR_UNLOGICAL_VALUE, suberror, tmpstr_of( TSD, stmp1 ) );
               }
            }
            return tmp;
         }
         return num_to_bool( shortcutnum( TSD, thisptr )) ;

      case X_HEAD_SYMBOL:
         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
         {
            /* fixes bug 1111931 */
            volatile char *s;

            stmp1 = (streng *) fix_compound( TSD, thisptr, NULL );
            tmp = Str_val( stmp1 )[0] - '0';
            if ( ( Str_len( stmp1 ) != 1 ) || ( ( tmp != 0 ) && ( tmp != 1 ) ) )
            {
               s = tmpstr_of( TSD, stmp1 );
               Free_stringTSD( stmp1 );
               if ( op )
               {
                  exiterror( ERR_UNLOGICAL_VALUE, suberror, op, s );
               }
               else
               {
                  exiterror( ERR_UNLOGICAL_VALUE, suberror, s );
               }
            }
            Free_stringTSD( stmp1 );
            return tmp;
         }
         return num_to_bool( fix_compoundnum( TSD, thisptr, NULL, NULL ) );

      case X_IN_FUNC:
      case X_IS_INTERNAL:
      case X_IS_BUILTIN:
      case X_EX_FUNC:
      case X_IS_EXTERNAL:
      case X_CONCAT:
      case X_SPACE:
         tmp = str_to_bool( evaluate( TSD, thisptr, &stmp1 ) );
         FREE_TMP_STRING( stmp1 );
         return tmp;

      case X_LOG_NOT:
         sint = !isboolean( TSD, thisptr->p[0], 6, "\\" ) ;
         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'U' ) ;
         return sint ;

      case X_LOG_OR:
      case X_ORASSIGN:
         sint = ( isboolean(TSD, thisptr->p[0], 5, "|") | isboolean( TSD, thisptr->p[1], 6, "|" )) ;
         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'U' ) ;
         return sint ;

      case X_LOG_AND:
      case X_ANDASSIGN:
         sint = ( isboolean(TSD, thisptr->p[0], 5, "&" ) & isboolean( TSD, thisptr->p[1], 6, "&" )) ;
         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'U' ) ;
         return sint ;

      case X_LOG_XOR:
      case X_XORASSIGN:
         /* Well, sort of ... */
         sint = ( isboolean( TSD, thisptr->p[0], 5, "&&" ) ^ isboolean( TSD, thisptr->p[1], 6, "&&" )) ;
         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'U' ) ;
         return sint ;

      case X_EQUAL:
      case X_DIFF:
      case X_GT:
      case X_GTE:
      case X_LT:
      case X_LTE:
      {
         int type ;
         compflags flags ;
         num_descr *rnum, *lnum ;
         streng *lval, *rval ;

         flags = thisptr->u.flags ;
         rnum = lnum = 0 ;
         rval = lval = NULL ;
         stmp1 = stmp2 = NULL ;

         if (flags.lnum)
         {
            if ( !thisptr->p[0]->u.number )
               thisptr->p[0]->u.number = get_a_descr( TSD, NULL, 0, thisptr->p[0]->name );

            lnum = thisptr->p[0]->u.number ;
            if (TSD->trace_stat=='I')
               tracenumber( TSD, lnum, 'L' ) ;
         }
         else if (flags.lsvar)
            lnum = shortcutnum( TSD, thisptr->p[0] ) ;
         else if (flags.lcvar)
            lnum = fix_compoundnum( TSD, thisptr->p[0], NULL, NULL );

         if (!lnum)
            lval = evaluate( TSD, thisptr->p[0], &stmp1 ) ;

         if (flags.rnum)
         {
            if ( !thisptr->p[1]->u.number )
               thisptr->p[1]->u.number = get_a_descr( TSD, NULL, 0, thisptr->p[1]->name );

            rnum = thisptr->p[1]->u.number ;
            if (TSD->trace_stat=='I')
               tracenumber( TSD, rnum, 'L' ) ;
         }
         else if (flags.rsvar)
            rnum = shortcutnum( TSD, thisptr->p[1] ) ;
         else if (flags.rcvar)
            rnum = fix_compoundnum( TSD, thisptr->p[1], NULL, NULL );

         if (!rnum)
            rval = evaluate( TSD, thisptr->p[1], &stmp2 ) ;

         if (!lnum && !getdescr( TSD, lval, &TSD->ldes ))
            lnum = &TSD->ldes ;

         if (!rnum && !getdescr( TSD, rval, &TSD->rdes ))
            rnum = &TSD->rdes ;

         if (rnum && lnum)
            tmp = string_test( TSD, lnum, rnum ) ;
         else
         {
            unsigned char *s1,*s2,*e1,*e2;

            if ( !lval )
            {
               assert( !stmp1 );
               stmp1 = lval = str_norm( TSD, lnum, NULL );
            }

            if ( !rval )
            {
               assert( !stmp2 );
               stmp2 = rval = str_norm( TSD, rnum, NULL );
            }

            s1 = (unsigned char *) lval->value;
            s2 = (unsigned char *) rval->value;
            e1 = (unsigned char *) s1 + lval->len;
            e2 = (unsigned char *) s2 + rval->len;

            strip_whitespace( TSD, &s1, &e1, &s2, &e2 );

            if ( s1 == e1 && s2 == e2 )
               tmp = 0;
            else if ( s1 < e1 && s2 < e2 )
               tmp = ( *s1 < *s2 ) ? -1 : 1;
            else
               tmp = ( s1 < e1 ) ? 1 : -1;
         }

         FREE_TMP_STRING( stmp1 );
         FREE_TMP_STRING( stmp2 );

         type = thisptr->type ;
         if (tmp==0)
            sint = (type==X_GTE || type==X_LTE || type==X_EQUAL) ;
         else if (tmp>0)
            sint = (type==X_GT || type==X_GTE || type==X_DIFF) ;
         else
            sint = (type==X_LT || type==X_LTE || type==X_DIFF) ;

         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'O' ) ;

         return sint ;
      }

      case X_SGT:
      case X_SLT:
      case X_SLTE:
      case X_SGTE:
      case X_SEQUAL:
      case X_SDIFF:
      {  /* string comparison */
         unsigned char *s1, *s2, *e1, *e2 ;
         int type ;

         type = thisptr->type ;
         strone = evaluate( TSD, thisptr->p[0], &stmp1 ) ;
         strtwo = evaluate( TSD, thisptr->p[1], &stmp2 ) ;

         s1 = (unsigned char *) strone->value ;
         s2 = (unsigned char *) strtwo->value ;
         e1 = (unsigned char *) s1 + strone->len ;
         e2 = (unsigned char *) s2 + strtwo->len ;

         strip_whitespace( TSD, &s1, &e1, &s2, &e2 );

         if (s1==e1 && s2==e2)
            tmp = 0 ;
         else if (s1<e1 && s2<e2)
            tmp = (*s1<*s2) ? -1 : 1 ;
         else
            tmp = (s1<e1) ? 1 : -1 ;

         FREE_TMP_STRING( stmp1 );
         FREE_TMP_STRING( stmp2 );

         if (tmp==0)
            sint = (type==X_SGTE || type==X_SLTE || type==X_SEQUAL) ;
         else if (tmp>0)
            sint = (type==X_SGT || type==X_SGTE || type==X_SDIFF) ;
         else
            sint = (type==X_SLT || type==X_SLTE || type==X_SDIFF) ;

         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'O' ) ;

         return sint ;
      }

      case X_NGT:
      case X_NLT:
      case X_NLTE:
      case X_NGTE:
      case X_NEQUAL:
      case X_NDIFF:
      {
         /* numeric NOT comparison */
         int type ;
         num_descr *ntmp1, *ntmp2 ;
         num_descr *numone, *numtwo ;

         type = thisptr->type ;

         ntmp1 = ntmp2 = NULL;
         numone = calcul( TSD, thisptr->p[0], &ntmp1, SIDE_LEFT, thisptr->type ) ;
         numtwo = calcul( TSD, thisptr->p[1], &ntmp2, SIDE_RIGHT, thisptr->type ) ;
         tmp = string_test( TSD, numone, numtwo ) ;

         if (ntmp1)
         {
            FreeTSD( ntmp1->num ) ;
            FreeTSD( ntmp1 ) ;
         }
         if (ntmp2)
         {
            FreeTSD( ntmp2->num ) ;
            FreeTSD( ntmp2 ) ;
         }

         if (tmp==0)
            sint = (type==X_NGTE || type==X_NLTE || type==X_NEQUAL) ;
         else if (tmp>0)
            sint = (type==X_NGT || type==X_NGTE || type==X_NDIFF) ;
         else
            sint = (type==X_NLT || type==X_NLTE || type==X_NDIFF) ;

         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'O' ) ;

         return sint ;
      }

      case X_S_DIFF:
         strone = evaluate( TSD, thisptr->p[0], &stmp1 ) ;
         strtwo = evaluate( TSD, thisptr->p[1], &stmp2 ) ;
         tmp = Str_cmp(strone,strtwo)!=0 ;

         FREE_TMP_STRING( stmp1 );
         FREE_TMP_STRING( stmp2 );

         if (TSD->trace_stat=='I')
            tracebool( TSD, tmp, 'O' ) ;

         return tmp ;


      case X_S_EQUAL:
         strone = evaluate( TSD, thisptr->p[0], &stmp1 ) ;
         strtwo = evaluate( TSD, thisptr->p[1], &stmp2 ) ;
         tmp = Str_cmp(strone,strtwo)==0 ;

         FREE_TMP_STRING( stmp1 );
         FREE_TMP_STRING( stmp2 );

         if (TSD->trace_stat=='I')
            tracebool( TSD, tmp, 'O' ) ;

         return tmp ;

      case X_S_NGT:
      case X_S_NLT:
      {  /* strict string NOT comparison */
         unsigned char *s1, *s2, *e1, *e2 ;
         int type ;

         type = thisptr->type ;
         strone = evaluate( TSD, thisptr->p[0], &stmp1 ) ;
         strtwo = evaluate( TSD, thisptr->p[1], &stmp2 ) ;

         s1 = (unsigned char *) strone->value ;
         s2 = (unsigned char *) strtwo->value ;
         e1 = (unsigned char *) s1 + strone->len ;
         e2 = (unsigned char *) s2 + strtwo->len ;
         /*
          * same compare as non-strict except that leading and trailing spaces
          * are retained for comparison.
          */
         for (;(s1<e1)&&(s2<e2)&&(*s1==*s2);s1++,s2++) ;
         if (s1==e1 && s2==e2)
            tmp = 0 ;
         else if (s1<e1 && s2<e2)
            tmp = (*s1<*s2) ? 1 : -1 ;
         else
            tmp = (s1<e1) ? -1 : 1 ;

         FREE_TMP_STRING( stmp1 );
         FREE_TMP_STRING( stmp2 );

         if (tmp==0)
            sint = 1;
         else if (tmp>0)
            sint = (type==X_S_NGT) ;
         else
            sint = (type==X_S_NLT) ;

         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'O' ) ;

         return sint ;
      }

      case X_S_GT:
      case X_S_GTE:
      case X_S_LT:
      case X_S_LTE:
      {  /* strict string comparison */
         unsigned char *s1, *s2, *e1, *e2 ;
         int type ;

         type = thisptr->type ;
         strone = evaluate( TSD, thisptr->p[0], &stmp1 ) ;
         strtwo = evaluate( TSD, thisptr->p[1], &stmp2 ) ;

         s1 = (unsigned char *) strone->value ;
         s2 = (unsigned char *) strtwo->value ;
         e1 = (unsigned char *) s1 + strone->len ;
         e2 = (unsigned char *) s2 + strtwo->len ;
         /*
          * same compare as non-strict except that leading and trailing spaces
          * are retained for comparison.
          */
         for (;(s1<e1)&&(s2<e2)&&(*s1==*s2);s1++,s2++) ;
         if (s1==e1 && s2==e2)
            tmp = 0 ;
         else if (s1<e1 && s2<e2)
            tmp = (*s1<*s2) ? -1 : 1 ;
         else
            tmp = (s1<e1) ? 1 : -1 ;

         FREE_TMP_STRING( stmp1 );
         FREE_TMP_STRING( stmp2 );

         if (tmp==0)
            sint = (type==X_S_GTE || type==X_S_LTE) ;
         else if (tmp>0)
            sint = (type==X_S_GT || type==X_S_GTE ) ;
         else
            sint = (type==X_S_LT || type==X_S_LTE ) ;

         if (TSD->trace_stat=='I')
            tracebool( TSD, sint, 'O' ) ;

         return sint ;
      }


      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         return 0 ;
   }
}

