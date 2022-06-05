/*
 *  Generic Call Interface for Rexx
 *  Copyright © 2003-2004, Florian Groﬂe-Coosmann
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
 *
 * ----------------------------------------------------------------------------
 *
 * This file prepares an input tree for later use.
 */

#include "gci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * While iterating over the tree we need a structure of needed informations
 * we can pass as one single pointer to the tree enumerator.
 */
typedef struct {
   void         *hidden;
   GCI_treeinfo *ti;     /* current tree */
   int           parent; /* current parent, used in the first pass */
   unsigned      runner; /* current memory location, used while computing
                          * positions.
                          */
} WalkerInfo;

/*
 * We don't know where the items will be placed in advance. But we can compute
 * it afterwards. The main information we need is the amount of spare space
 * which depends on the alignment.
 * We currently use a fixed alignment of GCI_ALIGNMENT but it can depend on
 * individual alignment factors.
 */
#define SPARE(pos,align) (((pos)%(align)) ? ((align) - (pos)%(align)) : 0)

/*
 * resizeNodes resizes the w->ti->nodes array to the new_nodemax maximum.
 *
 * Return values:
 * GCI_OK:            Everything's fine.
 * GCI_InternalError: Attempt to shorten the array so far that elements get
 *                    lost.
 * GCI_NoMemory:      Not enough free memory. Note that the old w->ti->nodes
 *                    persists.
 */
static GCI_result resizeNodes( WalkerInfo *w,
                               int new_nodemax )
{
   void *buf;
   GCI_treeinfo *ti = w->ti;

   if ( ti->max == new_nodemax )
      return GCI_OK;
   if ( ti->used > new_nodemax )
      return GCI_InternalError;

   buf = GCI_malloc( w->hidden, sizeof( GCI_nodeinfo ) * new_nodemax );
   if ( buf == NULL )
      return GCI_NoMemory;

   if ( ti->nodes != NULL )
   {
      memcpy( buf, ti->nodes, sizeof( GCI_nodeinfo ) * ti->used );
      GCI_free( w->hidden, ti->nodes );
   }
   ti->max = new_nodemax;
   ti->nodes = (GCI_nodeinfo *) buf;

   return GCI_OK;
}

/*
 * walker is a callback-routine for the tree enumerator GCI_parsetree.
 * Look there for the arguments.
 *
 * Its purpose is the building of a tree structure in ((WalkerInfo *)arg)->ti,
 * which may grow during the iteration.
 * We compute the direct and indirect sizes of each element. At the end of
 * the last iteration step we have the memory count we need to allocate for a
 * call to the function.
 *
 * Return values:
 * GCI_OK:            Everything's fine.
 * GCI_InternalError: Attempt to shorten the array so far that elements get
 *                    lost or illegal values used internally.
 * GCI_NoMemory:      Not enough free memory. Note that the old w->ti->nodes
 *                    persists.
 */
static GCI_result walker( int depth,
                          int itemnumber,
                          void *arg,
                          const GCI_parseinfo *info )
{
   GCI_result rc;
   WalkerInfo *w = (WalkerInfo *) arg;
   GCI_treeinfo *ti = w->ti;
   GCI_nodeinfo *n, *h;
   int idx, prev, next;

   if ( itemnumber == -1 )
   {
      /*
       * We either have a container or an array.
       * In case of an array: multiply the sizes by the array count
       * In case of a container: Reverse the siblings
       * In both cases we have to adjust other sizes and counts and we have
       * to reinstall the parent's parent.
       */
      assert( w->parent >= 0 );
      if ( w->parent < 0 )
         return GCI_InternalError;

      n = ti->nodes + w->parent;
      w->parent = n->parent;

      if ( info->type == GCI_container )
      {
         /*
          * reverse the siblings. We do
          * n                     n             n--+          n-----+
          * |                     |                |                |
          * v              ->     v          ->    v       ->       v    ->...
          * a->b->c->d->-1    -1<-a|>b->c->d    a<-b|>c->d    a<-b<-c|>d
          *                    p     n          p     n          p     n
          *                    r     e          r     e          r     e
          *                    e     x          e     x          e     x
          *                    v     t          v     t          v     t

          *                n
          *                |
          * ...->          v
          *       a<-b<-c<-d|>-1
          *             p     n
          *             r     e
          *             e     x
          *             v     t
          */
         assert( n->child != -1 );

         prev = -1;
         h = ti->nodes + n->child;
         next = h->sibling;
         h->sibling = prev;

         n->direct_size += h->direct_size;
         n->indirect_size += h->indirect_size;

         while ( next != -1 )
         {
            prev = n->child;
            n->child = next;
            h = ti->nodes + next;
            next = h->sibling;
            h->sibling = prev;

            n->direct_size += h->direct_size;
            n->indirect_size += h->indirect_size;
         }
      }
      else
      {
         assert( info->type == GCI_array );

         h = ti->nodes + n->child;
         n->direct_size = info->size * h->direct_size;
         n->indirect_size = info->size * h->indirect_size;
      }

      if ( info->indirect )
      {
         n->indirect_size += n->direct_size;
         n->direct_size = sizeof( void * );
      }
      return GCI_OK;
   }

   /*
    * Normal call (itemnumber >= 0)
    * Create an entry and set the values for base types. Composite types'
    * sizes must wait until the finalization of the subelements (see above).
    */
   if ( ti->used + 1 >= ti->max )
   {
      if ( ( rc = resizeNodes( w, ti->max + 64 ) ) != GCI_OK )
         return rc;
   }
   idx = ti->used++;
   n = ti->nodes + idx;

   n->type          = *info;
   n->parent        = w->parent;
   n->child         = -1;
   n->sibling       = -1;
   n->direct_size   = 0;
   n->indirect_size = 0;
   n->direct_pos    = 0;
   n->indirect_pos  = 0;
   if ( n->parent == -1 )
   {
      assert( depth == 0 );
   }
   else
   {
      /*
       * We concatenate the sibling in reversed order; it must be rearranged
       * at the end of the current depth.
       */
      n->sibling = ti->nodes[n->parent].child;
      ti->nodes[n->parent].child = idx;
   }

   switch ( info->type )
   {
      case GCI_integer:
      case GCI_unsigned:
      case GCI_float:
      case GCI_char:
         n->direct_size = info->size;
         break;

      case GCI_string:
         n->direct_size = info->size + 1;
         break;

      case GCI_raw:
         n->direct_size = info->size;
         break;

      case GCI_container:
      case GCI_array:
         /*
          * computed later
          */
         w->parent = idx;
         return GCI_OK;

      default:
         return GCI_InternalError;
   }

   if ( info->indirect )
   {
      n->indirect_size = n->direct_size;
      n->direct_size = sizeof( void * );
   }
   return GCI_OK;
}

/*
 * computePosition recursively iterates over the tree w->ti computing the
 * relative positions of each element within an imaginary buffer.
 *
 * The iteration starts at the element start. direct is a flag indicating
 * whether or not the direct size shall be computed. This includes all child
 * sizes up to their end or a branch to an INDIRECT element, in which case
 * the pointer itself is counted and the iteration is stopped.
 * In case of !direct the indirect size of an element is added. This is 1 for
 * an INDIRECT INTEGER8 for example. In case of composite type like array
 * and container we start a new cycle of direct/!direct computation of the
 * sub-elements.
 *
 * This technique shall ensure the "width-first" logic of memory allocation
 * needed for a buffer. Since we double-buffer all values, we have to add
 * every value to an indirect after the direct buffer.
 *
 * Another thing this routine has to do is the computation of alignment.
 * This will increase the size of the used buffer.
 *
 * One(!) example is
 * CONTAINER
 *   INTEGER8
 *   INDIRECT CONTAINER
 *     CHAR
 *     INDIRECT STRING255
 *     INTEGER32
 *   INTEGER16
 * which is equivalent to (assuming a pointer size of 4)
 * CONTAINER
 *   INTEGER8    1 byte, pos 0
 *   INDIRECTION 4 byte, pos 1 ---+
 *   INTEGER16   2 byte, pos 5    |
 *                       pos 7    |
 * +------------------------------+
 * |                         .
 * v                         . ( spare space to the next boundary, )
 * CONTAINER                 . ( e.g. with an alignment of 16      )
 *   CHAR        1 byte, pos 16
 *   INDIRECTION 4 byte, pos 17 --+
 *   INTEGER32   4 byte, pos 21   |
 *                           25   |
 * +------------------------------+
 * |                         . ( spare space to the next boundary, )
 * v                         . ( e.g. with an alignment of 16      )
 * STRING255   256 byte, pos 32
 *
 * whith makes a sum of 288 byte used
 *
 * The used size is added to w->runner.
 */
static void computePosition( WalkerInfo *w,
                             int start,
                             int direct )
{
   GCI_nodeinfo *n;
   GCI_parseinfo *info;
   GCI_treeinfo *ti = w->ti;
   int child;
   unsigned h, s;

   n = ti->nodes + start;
   info = &n->type;

   if ( direct )
   {
      /*
       * The simpler case.
       */
      n->direct_pos = w->runner;

      if ( info->indirect )
      {
         w->runner += sizeof( void * );
         return;
      }

      switch ( info->type )
      {
         /*
          * In case of container or array we have to add direct sizes of the
          * children, too. They will not be aligned, that's user's stuff.
          */
         case GCI_array:
            h = w->runner;
            computePosition( w, n->child, 1 );
            assert( n->direct_size / info->size == w->runner - h );
            w->runner += ( info->size - 1 ) * ( w->runner - h );
            break;

         case GCI_container:
            h = w->runner;
            for ( child = n->child;
                  child != -1;
                  child = ti->nodes[child].sibling )
               computePosition( w, child, 1 );
            assert( n->direct_size == w->runner - h );
            break;

         default:
            w->runner += n->direct_size;
            break;
      }
      return;
   }

   /*
    * Now handle the indirect case. We either may have a direct or an indirect
    * node.
    */
   if ( !info->indirect )
   {
      /*
       * In case of direct nodes, counted for the indirect case, we can have
       * containers or array only. These type may be direct but may have
       * indirected elements which have to be counted, too.
       * You can think of this case as a forwarder. Nothing really special
       * happens here, despite the fact we have to realign indirect values.
       */
      switch ( info->type )
      {
         case GCI_array:
            s = SPARE( w->runner, GCI_ALIGNMENT );
            w->runner += s;
            h = w->runner;

            computePosition( w, n->child, 0 );

            if ( h == w->runner )
               w->runner -= s;
            else
            {
               s = SPARE( w->runner, GCI_ALIGNMENT );
               n->indirect_size = info->size * ( w->runner - h + s );
               w->runner += ( info->size - 1 ) * ( w->runner - h + s );
               w->runner += s;
            }
            break;

         case GCI_container:
            s = SPARE( w->runner, GCI_ALIGNMENT );
            w->runner += s;
            h = w->runner;

            for ( child = n->child;
                  child != -1;
                  child = ti->nodes[child].sibling )
               computePosition( w, child, 0 );

            if ( h == w->runner )
               w->runner -= s;
            else
               {
                  s = SPARE( w->runner, GCI_ALIGNMENT );
                  n->indirect_size = w->runner - h + s;
                  w->runner += s;
               }
            break;

         default:
            assert( ( info->type != GCI_array ) &&
                    ( info->type != GCI_container ) );
            break;
      }
      return;
   }

   /*
    * The indirect case where the node is also indirect. Simple types are
    * trivial, but the array or container case is not.
    */
   switch ( info->type )
   {
      case GCI_array:
      case GCI_container:
         w->runner += SPARE( w->runner, GCI_ALIGNMENT );
         n->indirect_pos = w->runner;
         break;

      default:
         h = SPARE( w->runner, GCI_ALIGNMENT );
         n->indirect_pos = w->runner + h;
         w->runner += n->indirect_size + h;
         h = SPARE( w->runner, GCI_ALIGNMENT );
         n->indirect_size += h;
         w->runner += h;
         return;
   }

   /*
    * Treating an array as a container, we only have to support the container
    * here for indirect cases. But a container may have both direct and
    * indirect fields. We always have to recompute both types separately to
    * let adjacent fields keep connected (width-first order).
    *
    * We do an alignment after the direct computation. If we have an indirect
    * case we always have a direct case. And if we don't have an indirect case
    * we have to align one level higher, therefore we may do an alignment.
    */
   h = w->runner;
   for ( child = n->child;  child != -1;  child = ti->nodes[child].sibling )
      computePosition( w, child, 1 );
   if ( info->type == GCI_array )
      w->runner += (info->size - 1) * ( w->runner - h );

   s = SPARE( w->runner, GCI_ALIGNMENT );
   w->runner += s;
   n->indirect_size = w->runner - h;

   h = w->runner;
   for ( child = n->child;  child != -1;  child = ti->nodes[child].sibling )
      computePosition( w, child, 0 );
   if ( h != w->runner )
   {
      /*
       * Let the next argument be aligned properly and note it in the indirect
       * size.
       */
      s = SPARE( w->runner, GCI_ALIGNMENT );
      w->runner += s;
      if ( info->type == GCI_array )
      {
         n->indirect_size += info->size * ( w->runner - h );
         w->runner += ( info->size - 1 ) * ( w->runner - h );
      }
      else
      {
         n->indirect_size += w->runner - h;
      }
   }
}

/*
 * computePositions recursively iterates over the tree w->ti computing the
 * relative positions of each element within an imaginary buffer.
 *
 * See computePosition above for a description.
 */
static void computePositions( WalkerInfo *w,
                              int start )
{
   unsigned s, hrun;

   s = SPARE( w->runner, GCI_ALIGNMENT );
   w->runner += s;
   hrun = w->runner;
   computePosition( w, start, 1 );
   if ( hrun == w->runner )
      w->runner -= s;

   s = SPARE( w->runner, GCI_ALIGNMENT );
   w->runner += s;
   hrun = w->runner;
   computePosition( w, start, 0 );
   if ( hrun == w->runner )
      w->runner -= s;
   else
   {
      w->ti->nodes[start].indirect_size = w->runner - hrun;
      /*
       * Let the next argument be aligned properly and note it in the indirect
       * size.
       */
      s = SPARE( w->runner, GCI_ALIGNMENT );
      w->runner += s;
      w->ti->nodes[start].indirect_size += s;
   }
}

/*
 * dumpNode dumps a node and all its childs and siblings to stdout.
 * start is the starting node withing the tree, indent is the current indention
 * level.
 */
static void dumpNode( const GCI_treeinfo *ti,
                      int start,
                      int indent )
{
   char buf[80], *p;
   GCI_nodeinfo *n;
   GCI_parseinfo *info;

   do {
      n = ti->nodes + start;
      info = &n->type;
      strcpy( buf, (info->indirect) ? "INDIRECT " : "" );
      p = buf + strlen(buf);
      switch ( info->type )
      {
         case GCI_integer:   sprintf(p, "INTEGER%u", info->size*8 );  break;
         case GCI_unsigned:  sprintf(p, "UNSIGNED%u", info->size*8 ); break;
         case GCI_float:     sprintf(p, "FLOAT%u", info->size*8 );    break;
         case GCI_char:      sprintf(p, "CHAR%u", info->size*8 );     break;
         case GCI_string:    sprintf(p, "STRING%u", info->size );     break;
         case GCI_raw:       sprintf(p, "RAW%u", info->size );        break;
         case GCI_container: sprintf(p, "CONTAINER" );                break;
         case GCI_array:     sprintf(p, "ARRAY[%u]", info->size );    break;
         default:            sprintf(p, "???[%u]", info->size );      break;
      }
      printf( "%3d %3d %3d %3d, %5u %5u %5u %5u ",
              start, n->parent, n->child, n->sibling,
              n->direct_size, n->indirect_size,
              n->direct_pos, n->indirect_pos );
      printf( "%*s%s%s\n",
              indent * 3, "",
              buf,
              ( n->type.generated ) ? " (generated)" : "" );
      start = n->sibling;

      if ( n->child != -1 )
         dumpNode( ti, n->child, indent + 1 );
   } while ( start != -1 );
}

#ifdef DEBUG
/*
 * dump dumps every argument and the return value including every child to
 * stdout if the environment variable "_GCI_DUMP" is set.
 */
static void dump( const GCI_treeinfo *ti )
{
   int i;

   if ( getenv("_GCI_DUMP") == NULL )
      return;
   printf( "TreeInfo:\n"
           "%d (%d) nodes used\n"
           "%u bytes used for the buffer\n",
           ti->used, ti->max, ti->size );
   printf( "idx par chl sib,   dsi   isi   dpo   ipo\n");

   for ( i = 0; i < (int) elements( ti->args ); i++ )
   {
      if ( ti->args[i] == -1 )
         break;
      printf( "\n"
              "Argument %d starts on index %d:\n",
              i + 1, ti->args[i] );
      dumpNode( ti, ti->args[i], 0 );
   }

   if ( ti->retval != -1 )
   {
      printf( "\n"
              "Return value starts on index %d:\n",
              ti->retval );
      dumpNode( ti, ti->retval, 0 );
   }
}
#else
# define dump(x)
#endif

/*****************************************************************************
 *****************************************************************************
 ** GLOBAL FUNCTIONS *********************************************************
 *****************************************************************************
 *****************************************************************************/

/*
 * GCI_parsenodes parses a type definition stem (that thing you provide in a
 * call to RxFuncDefine) and assigns the values to a structure.
 *
 * The stem's name will come from base, which must be readable in a
 * non-symbolic manner (uppercased, etc). The string must have some spare
 * space for the additional fields of the names all over the structure. I
 * think 200 byte will be sufficient.
 * The structure *ti is filled, ti->nodes shall be NULL initially and may or
 * may not contain valid entries on return even in case of an error.
 * ti->callinfo is used to determine if more restriction have to be applied to
 * the arguments or to the return value.
 * argc contains the number of arguments this function shall parse.
 * return_valid shall be set to 1 if the "stem.RETURN.TYPE" is not the
 * empty string. In this case we parse the corresponding tree, too.
 *
 * prefixChar is the prefix that must be used in front of stem names.
 *
 * The string base may contain the error location within the stem in case of
 * an error.
 *
 * Return values:
 * GCI_OK:              Everything is fine.
 *
 *                      In case of an error cb->buffer will contain the
 *                      variable's name where the problem raises first.
 *
 * GCI_MissingName:     A variable's name isn't set. This is the equivalence
 *                      for GCI_MissingValue in the type parsing step. The
 *                      system may or may not raise a NOVALUE condition instead
 *                      depending on the implementation.
 * GCI_BufferTooSmall:  The variable's name buffer cb->buffer can't hold the
 *                      complete variable's name or the type string exceeds
 *                      256 byte.
 * GCI_IllegalName:     The variables name in cb->buffer is illegal in terms of
 *                      Rexx. In general, the basename of GCI_paretree is
 *                      wrong.
 * GCI_RexxError:       An unexpected error is returned by the interpreter
 *                      while trying to access Rexx variables.
 * GCI_UnsupportedType: Wrong type of input, e.g. FLOAT31 or the empty string
 *                      in a type description string. Another reason is an
 *                      internal error since the default sizes for "unsigned"
 *                      and "integer" are not supported.
 * GCI_WrongInput:      Strange characters occur in the input string as the
 *                      bit size of the type.
 * GCI_NumberRange:     Number to small or big to fit into the desired type
 *                      with the desired destbyte-size. This applies to the
 *                      element count of an "ARRAY" or "CONTAINER" type size
 *                      or the bit size of the plain type.
 * GCI_NoBaseType:      The type won't fit the requirements for basic types.
 * GCI_InternalError:   Attempt to shorten the array so far that elements get
 *                      lost or illegal values used internally.
 */
GCI_result GCI_parsenodes( void *hidden,
                           GCI_str *base,
                           GCI_treeinfo *ti,
                           unsigned argc,
                           unsigned return_valid,
                           const char *prefixChar )
{
   GCI_result rc = GCI_OK;
   unsigned i;
   int origlen = GCI_strlen( base );
   char iter[4];
   WalkerInfo w;

   w.hidden = hidden;
   w.ti = ti;
   w.runner = 0;

   for ( i = 0; i < argc; i++ )
   {
      GCI_strsetlen( base, origlen );
      sprintf( iter, ".%u", i+1 );
      GCI_strcats( base, iter );
      w.parent = -1;
      ti->args[i] = ti->used;
      if ( ( rc = GCI_parsetree( hidden,
                                 base,
                                 walker,
                                 &w,
                                 prefixChar ) ) != GCI_OK )
         break;
      if ( ti->callinfo.with_parameters &&
           ( ti->nodes[ti->args[i]].child != -1 ) )
      {
         rc = GCI_NoBaseType;
         break;
      }
      ti->size += ti->nodes[ti->args[i]].direct_size;
      ti->size += ti->nodes[ti->args[i]].indirect_size;
      computePositions( &w, ti->args[i] );
      assert( w.runner >= ti->size );
      ti->size = w.runner;
   }

   if ( ( rc == GCI_OK ) && return_valid )
   {
      GCI_strsetlen( base, origlen );
      GCI_strcats( base, "." );
      GCI_strcats( base, prefixChar );
      GCI_strcats( base, "RETURN" );
      w.parent = -1;
      ti->retval = ti->used;
      rc = GCI_parsetree( hidden, base, walker, &w, prefixChar );
      if ( rc == GCI_OK )
      {
         if ( ti->callinfo.as_function &&
              ( ti->nodes[ti->retval].child != -1 ) )
            return GCI_NoBaseType;
         GCI_strsetlen( base, origlen );

         /*
          * It doesn't make any sense to add the return value's sizes, but it
          * makes sense to buffer at least the first direct return value for
          * later use.
          */
         if ( w.runner != 0 )
         {
            ti->size += SPARE( ti->size, GCI_ALIGNMENT );
            ti->size += ti->nodes[ti->retval].direct_size;
         }
         computePositions( &w, ti->retval );
      }
   }

   /*
    * Don't let malloc throw an error if no parameters are addressed.
    */
   ti->size = ( w.runner ) ? w.runner : 1;

   if ( rc == GCI_OK )
   {
      /*
       * Minimize the used memory.
       */
      resizeNodes( &w, ti->used );
      dump( ti );
   }

   return rc;
}
