#ifndef lint
static char *RCSid = "$Id: debug.c,v 1.14 2005/08/16 07:21:48 mark Exp $";
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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "rexx.h"

#ifndef NDEBUG

/*
 * Prints the value of v to fp. ", number ???" may be appended if v is a number.
 * In addition, the helper fields are printed and the line is terminated.
 */
void dumpvarcontent( const tsd_t *TSD, FILE *fp, cvariableptr v, int exposed )
{
   const streng *s;
   const num_descr *n;

   s = v->value;
   if ( s )
   {
      fprintf( fp, "\"%.*s\"", s->len, s->value );
   }
   else
   {
      fprintf( fp, "<none>" );
   }

   n = v->num;
   fprintf( fp, ",\tnumber " );
   if ( n ) /* variable is a number */
   {
      fprintf( fp, "%s0.%.*sE%+d",
               ( n->negative ) ? "-" : "",
               n->size, n->num, n->exp );
   }
   else
   {
      fprintf( fp, "<none>" );
   }

   switch ( v->flag )
   {
      case VFLAG_NONE: fprintf( fp, ",\tflag NONE, " ); break;
      case VFLAG_STR:  fprintf( fp, ",\tflag STR,  " );  break;
      case VFLAG_NUM:  fprintf( fp, ",\tflag NUM,  " );  break;
      case VFLAG_BOTH: fprintf( fp, ",\tflag BOTH, " ); break;
      default:         fprintf( fp, ",\tflag %d, ", v->flag );
   }
   fprintf( fp, "hwired %ld, valid %ld, guard %d%s%s\n",
                v->hwired, v->valid, v->guard,
                ( exposed ) ? ", exposed" : "",
                ( v->flag == VFLAG_NONE ) ? ", dropped" : "" );
}

/*
 * get_realbox returns either p or the realbox associated with p if it exists.
 * This function is NULL-pointer safe.
 * *exposed is set to 0 if p is set in the current frame, it is set to 1 if p
 * is an exposed value from one of the upper frames.
 */
static cvariableptr get_realbox( cvariableptr p, int *exposed )
{
   *exposed = 0;
   if ( p == NULL )
      return p;
   if ( p->realbox == NULL )
      return p;

   *exposed = 1;
   for ( p = p->realbox; p->realbox; p = p->realbox )
      ;
   return p;
}

/*
 * dumpvars dumps the set of valid variables of the current PROCEDURE frame.
 * The destination is stderr of stdout in case of STDOUT_FOR_STDERR.
 */
void dumpvars( const tsd_t *TSD )
{
   cvariableptr ptr,tptr,rb,trb;
   int isstem,isexposed;
   unsigned i,j;
   FILE *fp;
   streng *s;
   cvariableptr *hashptr;

   fp = stderr;
   if ( get_options_flag( TSD->currlevel, EXT_STDOUT_FOR_STDERR ) )
      fp = stdout;

   hashptr = (cvariableptr *) TSD->currlevel->vars->tbl;

   fprintf( fp, "\nDumping variables, 1. no after \">>>\" is the bin number\n" );
   fprintf( fp, "[ %u elements in %u buckets, %u reads, %u writes, %u collisions ]\n",
                TSD->currlevel->vars->e,
                TSD->currlevel->vars->size,
                TSD->currlevel->vars->r,
                TSD->currlevel->vars->w,
                TSD->currlevel->vars->c);
   for ( i = 0; i < TSD->currlevel->vars->size; i++ )
   {
      if ( hashptr[i] == NULL )
         continue;

      /*
       * One bin of same hashvalues may have several vars connected by a
       * simple linked list.
       */
      for ( ptr = hashptr[i]; ptr != NULL; ptr = ptr->next )
      {
         rb = get_realbox( ptr, &isexposed );
         s = rb->name;
         isstem = s->value[s->len - 1] == '.';

         fprintf( fp, "   >>> %3d %s \"%.*s\",\tvalue ",
                      i, ( isstem ) ? "    Stem" : "Variable",
                      s->len, s->value );

         dumpvarcontent( TSD, fp, rb, isexposed );

         if ( !isstem )
            continue;

         fprintf( fp, "   [ %u elements in %u buckets, %u reads, %u writes, %u collisions ]\n",
                      rb->index->e,
                      rb->index->size,
                      rb->index->r,
                      rb->index->w,
                      rb->index->c);
         for ( j = 0; j < rb->index->size; j++ )
         {
            /*
             * The variables of a stem are organized as a normal variable
             * bunch. We have to iterate as for the level's variable set.
             * Keep in mind that a variable "a.b." isn't a stem, we can't
             * iterate once more.
             */
            if ( ( tptr = rb->index->tbl[j] ) != NULL )
            {
               for ( ; tptr; tptr = tptr->next )
               {
                  trb = get_realbox( tptr, &isexposed );
                  s = trb->name;
                  if ( s )
                  {
                     fprintf( fp, "      >>> %3d  Tail \"%.*s\",\tvalue ",
                                  j, s->len, s->value );
                     dumpvarcontent( TSD, fp, trb, isexposed );
                  }
               }
            }
         }
      }
   }

   return;
}

void dumptree(const tsd_t *TSD, const treenode *thisNode, int level, int newline)
{
   unsigned i;
   streng *ptr;
   FILE *fp=stderr;

   if ( get_options_flag( TSD->currlevel, EXT_STDOUT_FOR_STDERR ) )
      fp = stdout;

   while ( thisNode ) {
      if ( newline )
         fprintf( fp, "\n%*s", 2 * level, "" );

      fprintf( fp, "%s (type %d)\n",
                   getsym( thisNode->type ), thisNode->type );

      if ( thisNode->name )
      {
         fprintf( fp, "%*sName: [%.*s]\n",
                      2 * level, "",
                      thisNode->name->len, thisNode->name->value );
      }

      if ( ( thisNode->charnr != 0 ) && (thisNode->charnr != -1 ) )
      {
         fprintf( fp, "%*sLineno: %d   Charno: %d",
                      2 * level, "",
                      thisNode->lineno, thisNode->charnr );
         if ( newline )
         {
            ptr = getsourceline( TSD, thisNode->lineno, thisNode->charnr,
                                &TSD->systeminfo->tree );
            fprintf( fp, ", Sourceline: [%.*s]", ptr->len, ptr->value );
         }
         putc( '\n', fp );
      }

      /*
       * See also several places in instore.c where thisNode switch list must be
       * changed. Seek for X_CEXPRLIST.
       */
      switch ( thisNode->type )
      {
         case X_EQUAL:
         case X_DIFF:
         case X_GT:
         case X_GTE:
         case X_LT:
         case X_LTE:
            fprintf( fp, "%*sFlags: lnum %d, rnum %d, lsvar %d, rsvar %d, lcvar %d, rcvar %d\n",
                         2 * level, "",
                         thisNode->u.flags.lnum,
                         thisNode->u.flags.rnum,
                         thisNode->u.flags.lsvar,
                         thisNode->u.flags.rsvar,
                         thisNode->u.flags.lcvar,
                         thisNode->u.flags.rcvar );
            break;

         case X_ADDR_V:
            fprintf( fp, "%*sFlags: %sANSI version\n",
                         2 * level, "",
                         ( thisNode->u.nonansi ) ? "non-" : "" );
            break;

         case X_CEXPRLIST:
            if ( thisNode->u.strng == NULL )
               fprintf( fp, "%*sValue: <null>\n",
                            2 * level, "" );
            else
               fprintf( fp, "%*sValue: [%.*s]\n",
                            2 * level, "",
                            thisNode->u.strng->len, thisNode->u.strng->value );
            break;

         case X_LABEL:
            fprintf( fp, "%*sFlags: %s\n",
                         2 * level, "",
                         ( thisNode->u.trace_only ) ? "trace-only" :
                                                  "is target" );
            break;

         case X_PARSE:
            /*
             * similar to bug 972850, fixed in parallel
             */
            fprintf( fp, "%*sFlags: %s\n",
                         2 * level, "",
                         ( thisNode->u.parseflags == PARSE_UPPER) ? "UPPER" :
                         ( thisNode->u.parseflags == PARSE_LOWER) ? "LOWER" :
                         ( thisNode->u.parseflags == PARSE_CASELESS) ? "CASELESS" :
                         ( thisNode->u.parseflags == (PARSE_CASELESS | PARSE_LOWER)) ? "CASELESS LOWER" :
                         ( thisNode->u.parseflags == (PARSE_CASELESS | PARSE_UPPER)) ? "CASELESS UPPER" :
                                                                 "(normal)" );
            break;

         case X_ADDR_WITH:
            if ( !thisNode->p[0] && !thisNode->p[1] && !thisNode->p[2] )
               fprintf( fp, "%*sFlags: append %d, awt %s, ant %s\n",
                            2 * level, "",
                            thisNode->u.of.append,
                            ( thisNode->u.of.awt == awtUNKNOWN ) ? "unknown" :
                            ( thisNode->u.of.awt == awtSTREAM ) ?  "STREAM" :
                            ( thisNode->u.of.awt == awtSTEM ) ?    "STEM" :
                            ( thisNode->u.of.awt == awtLIFO ) ?    "LIFO" :
                            ( thisNode->u.of.awt == awtFIFO ) ?    "FIFO" :
                                                               "<error>",
                            ( thisNode->u.of.ant == antUNKNOWN )   ? "unknown" :
                            ( thisNode->u.of.ant == antSTRING )    ? "STRING" :
                            ( thisNode->u.of.ant == antSIMSYMBOL ) ? "SYMBOL" :
                                                                 "<error>" );
            break;

         default:
            break;
      }

      for ( i = 0; i < sizeof( thisNode->p ) / sizeof( thisNode->p[0] ); i++ )
         if ( thisNode->p[i] != NULL )
         {
            fprintf( fp, "%*s%d>",
                         2 * level, "",
                         i + 1 );
            dumptree( TSD, thisNode->p[i], level + 1, 0 );
         }

      thisNode = thisNode->next;
      newline = 1;
   }
}

#endif /* !NDEBUG */


#ifdef TRACEMEM
void marksource( clineboxptr ptr )
{
   for (;ptr;ptr=ptr->next) {
      markmemory( ptr->line,TRC_SOURCEL ) ;
      markmemory( (char *)ptr, TRC_SOURCE ) ; }
}
#endif


static const char *sourceline( int line, const internal_parser_type *ipt, unsigned *size)
{
   clineboxptr first;
   const otree *otp;

   if (ipt->first_source_line == NULL)
   { /* must be incore_source but that value may be NULL because of a failed
      * instore[0] of RexxStart!
      */
      otp = ipt->srclines; /* NULL if incore_source==NULL */
      while (otp && (otp->num < (unsigned long) line)) {
         line -= otp->num;
         otp = otp->next;
      }
      if (otp == NULL)
      {
         *size = 0 ;
         return NULL ;
      }
      line--;
      *size = otp->elems[line].length ;
      return ipt->incore_source + otp->elems[line].offset ;
   }
   first = ipt->first_source_line;
   for (;first;)
   {
      if (first->lineno==line)
      {
         *size = first->line->len ;
         return first->line->value ;
      }
      else
         first = (first->lineno<line) ? first->next : first->prev ;
   }

   *size = 0 ;
   return NULL ;
}



streng *getsourceline( const tsd_t *TSD, int line, int charnr, const internal_parser_type *ipt )
{
   int dquote=0, squote=0 ;
   unsigned len ;
   streng *string ;
   const char *ptr, *chptr, *chend, *tmptr ;
   char *outptr ;
   char *STR_VAL_LIMIT ;

   assert( charnr>=0 ) ;
   if (!charnr)
     charnr++ ;

   ptr = sourceline(line,ipt,&len) ;
/*   assert( ptr ) ; */
   if (!ptr || (charnr >= (int) len))
      return nullstringptr() ;

   chptr = ptr + --charnr ;
   chend = ptr + len ;
   for (; (chptr < chend) && rx_isspace(*chptr); chptr++) ;
   string = Str_makeTSD(BUFFERSIZE+1) ;
   outptr = string->value ;
   STR_VAL_LIMIT = BUFFERSIZE + outptr ;

   for (;;)
   {
restart:
      if (chptr>=chend || outptr >= STR_VAL_LIMIT)
         break ;

      if (!squote && *chptr=='\"')
         dquote = !dquote ;

      else if (!dquote && *chptr=='\'')
         squote = !squote ;

      else if (!(dquote || squote))
      {
         switch (*chptr)
         {
            case ',':
               for(tmptr=chptr+1; tmptr<chend && rx_isspace(*tmptr); tmptr++ ) ;
               assert( tmptr<=chend ) ;
               if (tmptr==chend)
               {
                  *(outptr++) = ' ' ;
                  chptr = sourceline(++line,ipt,&len) ;
                  chend = chptr + len ;
                  for(; chptr<chend && rx_isspace(*chptr); chptr++) ;
                  goto restart;
               }
               break ;

            case ':':
               *(outptr++) = *chptr ;

            case ';':
               goto endloop ;

         }
      }

      *(outptr++) = *(chptr++) ;
   }

endloop:
   assert( outptr - string->value <= BUFFERSIZE ) ;
   *outptr = '\0'; /* needs to be 0-terminated */
   string->len = outptr - string->value ;
   return string ;
}

