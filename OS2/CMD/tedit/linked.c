/***********************************************************************/
/* LINKED.C - Linked list routines.                                    */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

static char RCSid[] = "$Id: linked.c,v 1.5 2002/03/26 10:04:01 mark Exp $";

#include <the.h>
#include <proto.h>

#if 0 /* TBD */
struct llstruct
{
   struct llstruct *prev;
   struct llstruct *next;
   void *buf;
};
typedef struct llstruct LL;

typedef void (*LLDEL)( LL*);

LL *ll_add( LL *first, LL *curr, unsigned long size )
{
   LL *next=NULL;

   TRACE_FUNCTION( "linked.c:    ll_add" );

   if ( (next=(LL *)(*the_malloc)(size)) != (LL *)NULL )
   {
      memset( next, 0, size );
      if ( curr == NULL )
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if ( curr->next != NULL )
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return next;
}

LL *ll_del( LL **first, LL **last, LL *curr, short direction, LLDEL delfunc )
{
   LL *new_curr=NULL;

   TRACE_FUNCTION( "linked.c:    ll_del" );
   if ( delfunc )
      (*delfunc)(curr);
   /*
    * Delete the only record
    */
   if ( curr->prev == NULL && curr->next == NULL )
   {
      (*the_free)(curr);
      *first = NULL;
      if (last != NULL)
         *last = NULL;
      TRACE_RETURN();
      return NULL;
   }
   /*
    * Delete the first record
    */
   if (curr->prev == NULL)
   {
      curr->next->prev = NULL;
      *first = new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return curr;
   }
   /*
    * Delete the last  record
    */
   if ( curr->next == NULL )
   {
      curr->prev->next = NULL;
      new_curr = curr->prev;
      if (last != NULL)
         *last = curr->prev;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return curr;
   }
   /*
    * All others
    */
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   if ( direction == DIRECTION_FORWARD )
      new_curr = curr->next;
   else
      new_curr = curr->prev;

   (*the_free)(curr);
   curr = new_curr;
   TRACE_RETURN();
   return curr;
}

LL *ll_free( LL *first, LLDEL delfunc )
{
   LL *curr=NULL;
   LL *new_curr=NULL;

   TRACE_FUNCTION( "linked.c:    ll_free" );
   curr = first;
   while ( curr != NULL )
   {
      if ( delfunc )
         (*delfunc)(curr);
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return (LL *)NULL;
}
#endif


/***********************************************************************/
#ifdef HAVE_PROTO
_LINE *lll_add(_LINE *first,_LINE *curr,unsigned short size)
#else
_LINE *lll_add(first,curr,size)
_LINE *first;
_LINE *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a LINE to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first LINE in linked list                   */
/* curr       - pointer to current LINE in linked list                 */
/* size       - size of a LINE item                                    */
/* RETURN:    - pointer to next LINE item                              */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 _LINE *next=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    lll_add");

 if ((next=(_LINE *)(*the_malloc)(size)) != (_LINE *)NULL)
   {
    if (curr == NULL)
      {
       if (first == NULL)
         {
          /*
           * First entry in LL
           */
/*          first = next; */
          next->next = NULL;
          next->prev = NULL;
         }
       else
         {
          /*
           * Insert this entry before first. Calling routine
           * must reset first to returned pointer.
           */
          next->next = first;
          next->prev = NULL;
          first->prev = next;
         }
      }
    else
      {
       if (curr->next != NULL)
          curr->next->prev = next;
       next->next = curr->next;
       curr->next = next;
       next->prev = curr;
      }
   }
/*---------------------------------------------------------------------*/
/* Ensure all pointers in the structure are set to NULL                */
/*---------------------------------------------------------------------*/
 next->line = NULL;
 next->name = NULL;
 next->pre = NULL;
 TRACE_RETURN();
 return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
_LINE *lll_del(_LINE **first,_LINE **last,_LINE *curr,short direction)
#else
_LINE *lll_del(first,last,curr,direction)
_LINE **first;
_LINE **last;
_LINE *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 _LINE *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    lll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
 if (curr->prev == NULL && curr->next == NULL)
   {
    (*the_free)(curr);
    *first = NULL;
    if (last != NULL)
       *last = NULL;
    TRACE_RETURN();
    return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
 if (curr->prev == NULL)
   {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    (*the_free)(curr);
    curr = new_curr;
    TRACE_RETURN();
    return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
 if (curr->next == NULL)
   {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL)
       *last = curr->prev;
    (*the_free)(curr);
    curr = new_curr;
    TRACE_RETURN();
    return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
 curr->prev->next = curr->next;
 curr->next->prev = curr->prev;
 if (direction == DIRECTION_FORWARD)
    new_curr = curr->next;
 else
    new_curr = curr->prev;

 (*the_free)(curr);
 curr = new_curr;
 TRACE_RETURN();
 return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
_LINE *lll_free(_LINE *first)
#else
_LINE *lll_free(first)
_LINE *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 _LINE *curr=NULL;
 _LINE *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    lll_free");
 curr = first;
 while (curr != NULL)
   {
    if (curr->line) (*the_free)(curr->line);
    if (curr->name) (*the_free)(curr->name);
    new_curr = curr->next;
    (*the_free)(curr);
    curr = new_curr;
   }
 TRACE_RETURN();
 return((_LINE *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
_LINE *lll_find(_LINE *first,_LINE *last,LINETYPE line_number,LINETYPE max_lines)
#else
_LINE *lll_find(first,last,line_number,max_lines)
_LINE *first,*last;
LINETYPE line_number,max_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 _LINE *curr=NULL;
 LINETYPE i=0L;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    lll_find");
 if (line_number < (max_lines/2))
 {
    curr = first;
    if (curr != NULL)
    {
/*     for(i=0L;i<line_number && curr->next != NULL; i++, curr=curr->next); */
       for(i=0L;i<line_number; i++, curr=curr->next); /* FGC - removed check for NULL */
    }
 }
 else
 {
    curr = last;
    if (curr != NULL)
    {
/*       for(i=max_lines+1L;i>line_number && curr->prev != NULL; i--, curr=curr->prev); */
       for(i=max_lines+1L;i>line_number; i--, curr=curr->prev); /* FGC - removed check for NULL */
    }
 }
 TRACE_RETURN();
 return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
_LINE *lll_locate(_LINE *first,CHARTYPE *value)
#else
_LINE *lll_locate(first,value)
_LINE *first;
CHARTYPE *value;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 _LINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    lll_locate");
 curr = first;
 while (curr)
   {
    if (curr->name
    &&  strcmp((DEFCHAR*)curr->name,(DEFCHAR*)value) == 0)
       break;
    curr = curr->next;
   }
 TRACE_RETURN();
 return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
VIEW_DETAILS *vll_add(VIEW_DETAILS *first,VIEW_DETAILS *curr,unsigned short size)
#else
VIEW_DETAILS *vll_add(first,curr,size)
VIEW_DETAILS *first;
VIEW_DETAILS *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a VIEW_DETAILS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first VIEW_DETAILS in linked list                   */
/* curr       - pointer to current VIEW_DETAILS in linked list                 */
/* size       - size of a VIEW_DETAILS item                                    */
/* RETURN:    - pointer to next VIEW_DETAILS item                              */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 VIEW_DETAILS *next=(VIEW_DETAILS *)NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    vll_add");

 if ((next=(VIEW_DETAILS *)(*the_malloc)(size)) != (VIEW_DETAILS *)NULL)
   {
    if (curr == (VIEW_DETAILS *)NULL)
      {
       first = next;
       next->next = (VIEW_DETAILS *)NULL;
      }
    else
      {
       if (curr->next != (VIEW_DETAILS *)NULL)
          curr->next->prev = next;
       next->next = curr->next;
       curr->next = next;
      }
    next->prev = curr;
   }
/*---------------------------------------------------------------------*/
/* Ensure all pointers in the structure are set to NULL                */
/*---------------------------------------------------------------------*/
 next->file_for_view = (FILE_DETAILS *)NULL;
 TRACE_RETURN();
 return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
VIEW_DETAILS *vll_del(VIEW_DETAILS **first,VIEW_DETAILS **last,VIEW_DETAILS *curr,short direction)
#else
VIEW_DETAILS *vll_del(first,last,curr,direction)
VIEW_DETAILS **first;
VIEW_DETAILS **last;
VIEW_DETAILS *curr;
short direction;
#endif
/***********************************************************************/
/* This ll_del() function is different to others!!!!!!!!               */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 VIEW_DETAILS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    vll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
 if (curr->prev == NULL && curr->next == NULL)
   {
    (*the_free)(curr);
    *first = NULL;
    if (last != NULL)
       *last = NULL;
    TRACE_RETURN();
    return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
 if (curr->prev == NULL)
   {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    (*the_free)(curr);
    curr = new_curr;
    TRACE_RETURN();
    return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/* If DIRECTION_FORWARD, curr becomes first, otherwise curr becomes prev*/
/*---------------------------------------------------------------------*/
 if (curr->next == NULL)
   {
    curr->prev->next = NULL;
    if (direction == DIRECTION_FORWARD)
       new_curr = *first;
    else
       new_curr = curr->prev;
    if (last != NULL)
       *last = curr->prev;
    (*the_free)(curr);
    curr = new_curr;
    TRACE_RETURN();
    return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
 curr->prev->next = curr->next;
 curr->next->prev = curr->prev;
 if (direction == DIRECTION_FORWARD)
   new_curr = curr->next;
 else
   new_curr = curr->prev;

 (*the_free)(curr);
 curr = new_curr;
 TRACE_RETURN();
 return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
DEFINE *dll_add(DEFINE *first,DEFINE *curr,unsigned short size)
#else
DEFINE *dll_add(first,curr,size)
DEFINE *first;
DEFINE *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a DEFINE to the current linked list after the current member.  */
/* PARAMETERS:                                                         */
/* first      - pointer to first DEFINE in linked list                 */
/* curr       - pointer to current DEFINE in linked list               */
/* size       - size of a DEFINE item                                  */
/* RETURN:    - pointer to next DEFINE item                            */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 DEFINE *next=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    dll_add");
 if ((next=(DEFINE *)(*the_malloc)(size)) != (DEFINE *)NULL)
   {
    if (curr == NULL)
      {
       first = next;
       next->next = NULL;
      }
    else
      {
       if (curr->next != NULL)
          curr->next->prev = next;
       next->next = curr->next;
       curr->next = next;
      }
    next->prev = curr;
   }
/*---------------------------------------------------------------------*/
/* Ensure all pointers in the structure are set to NULL                */
/*---------------------------------------------------------------------*/
 next->def_params = NULL;
 next->pcode = NULL;
 next->pcode_len = 0;
 TRACE_RETURN();
 return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
DEFINE *dll_del(DEFINE **first,DEFINE **last,DEFINE *curr,short direction)
#else
DEFINE *dll_del(first,last,curr,direction)
DEFINE **first;
DEFINE **last;
DEFINE *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 DEFINE *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    dll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
 if (curr->prev == NULL && curr->next == NULL)
   {
    (*the_free)(curr);
    *first = NULL;
    if (last != NULL)
       *last = NULL;
    TRACE_RETURN();
    return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
 if (curr->prev == NULL)
   {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    (*the_free)(curr);
    curr = new_curr;
    TRACE_RETURN();
    return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
 if (curr->next == NULL)
   {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL)
       *last = curr->prev;
    (*the_free)(curr);
    curr = new_curr;
    TRACE_RETURN();
    return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
 curr->prev->next = curr->next;
 curr->next->prev = curr->prev;
 if (direction == DIRECTION_FORWARD)
   new_curr = curr->next;
 else
   new_curr = curr->prev;

 (*the_free)(curr);
 curr = new_curr;
 TRACE_RETURN();
 return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
DEFINE *dll_free(DEFINE *first)
#else
DEFINE *dll_free(first)
DEFINE *first;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 DEFINE *curr=NULL;
 DEFINE *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    dll_free");
 curr = first;
 while (curr != (DEFINE *)NULL)
   {
    if (curr->def_params != NULL)
       (*the_free)(curr->def_params);
    if (curr->pcode != NULL)
       (*the_free)(curr->pcode);
    new_curr = curr->next;
    (*the_free)(curr);
    curr = new_curr;
   }
 TRACE_RETURN();
 return((DEFINE *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
THE_PPC *pll_add(THE_PPC *first,THE_PPC *curr,unsigned short size)
#else
THE_PPC *pll_add(first,curr,size)
THE_PPC *first;
THE_PPC *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a THE_PPC to the current linked list after the current member.     */
/* PARAMETERS:                                                         */
/* first      - pointer to first THE_PPC in linked list                    */
/* curr       - pointer to current THE_PPC in linked list                  */
/* size       - size of a THE_PPC item                                     */
/* RETURN:    - pointer to next THE_PPC item                               */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 THE_PPC *next=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    pll_add");

 if ((next=(THE_PPC *)(*the_malloc)(size)) != (THE_PPC *)NULL)
   {
    if (curr == NULL)
      {
       first = next;
       next->next = NULL;
      }
    else
      {
       if (curr->next != NULL)
          curr->next->prev = next;
       next->next = curr->next;
       curr->next = next;
      }
    next->prev = curr;
   }
/*---------------------------------------------------------------------*/
/* Ensure all pointers in the structure are set to NULL                */
/*---------------------------------------------------------------------*/
 TRACE_RETURN();
 return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
THE_PPC *pll_del(THE_PPC **first,THE_PPC **last,THE_PPC *curr,short direction)
#else
THE_PPC *pll_del(first,last,curr,direction)
THE_PPC **first;
THE_PPC **last;
THE_PPC *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 THE_PPC *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    pll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
 if (curr->prev == NULL && curr->next == NULL)
   {
    (*the_free)(curr);
    *first = NULL;
    if (last != NULL)
       *last = NULL;
    TRACE_RETURN();
    return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
 if (curr->prev == NULL)
   {
    curr->next->prev = NULL;
    *first = new_curr = curr->next;
    (*the_free)(curr);
    curr = new_curr;
    TRACE_RETURN();
    return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
 if (curr->next == NULL)
   {
    curr->prev->next = NULL;
    new_curr = curr->prev;
    if (last != NULL)
       *last = curr->prev;
    (*the_free)(curr);
    curr = new_curr;
    TRACE_RETURN();
    return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
 curr->prev->next = curr->next;
 curr->next->prev = curr->prev;
 if (direction == DIRECTION_FORWARD)
   new_curr = curr->next;
 else
   new_curr = curr->prev;

 (*the_free)(curr);
 curr = new_curr;
 TRACE_RETURN();
 return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
THE_PPC *pll_free(THE_PPC *first)
#else
THE_PPC *pll_free(first)
THE_PPC *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 THE_PPC *curr=NULL;
 THE_PPC *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    pll_free");
 curr = first;
 while (curr != NULL)
   {
    new_curr = curr->next;
    (*the_free)(curr);
    curr = new_curr;
   }
 TRACE_RETURN();
 return((THE_PPC *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
THE_PPC *pll_find(THE_PPC *first,LINETYPE line_number)
#else
THE_PPC *pll_find(first,line_number)
THE_PPC *first;
LINETYPE line_number;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 THE_PPC *curr_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("linked.c:    pll_find");
 curr_ppc = first;
 while (curr_ppc != NULL)
   {
    if (curr_ppc->ppc_line_number == line_number)
      {
       TRACE_RETURN();
       return(curr_ppc);
      }
    curr_ppc = curr_ppc->next;
   }
 TRACE_RETURN();
 return(NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
RESERVED *rll_add(RESERVED *first,RESERVED *curr,unsigned short size)
#else
RESERVED *rll_add(first,curr,size)
RESERVED *first;
RESERVED *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a RESERVED to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first RESERVED in linked list                   */
/* curr       - pointer to current RESERVED in linked list                 */
/* size       - size of a RESERVED item                                    */
/* RETURN:    - pointer to next RESERVED item                              */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   RESERVED *next=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    rll_add");

   if ((next=(RESERVED *)(*the_malloc)(size)) != (RESERVED *)NULL)
   {
      memset(next,0,sizeof(RESERVED));
      if (curr == NULL)
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if (curr->next != NULL)
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
RESERVED *rll_del(RESERVED **first,RESERVED **last,RESERVED *curr,short direction)
#else
RESERVED *rll_del(first,last,curr,direction)
RESERVED **first;
RESERVED **last;
RESERVED *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   RESERVED *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    rll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL && curr->next == NULL)
   {
      (*the_free)(curr);
      *first = NULL;
      if (last != NULL)
         *last = NULL;
      TRACE_RETURN();
      return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL)
   {
      curr->next->prev = NULL;
      *first = new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
   if (curr->next == NULL)
   {
      curr->prev->next = NULL;
      new_curr = curr->prev;
      if (last != NULL)
         *last = curr->prev;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   if (direction == DIRECTION_FORWARD)
      new_curr = curr->next;
   else
      new_curr = curr->prev;

   (*the_free)(curr);
   curr = new_curr;
   TRACE_RETURN();
   return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
RESERVED *rll_free(RESERVED *first)
#else
RESERVED *rll_free(first)
RESERVED *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   RESERVED *curr=NULL;
   RESERVED *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    rll_free");
   curr = first;
   while (curr != NULL)
   {
      if (curr->line != (CHARTYPE *)NULL)
         (*the_free)(curr->line);
      if (curr->spec != (CHARTYPE *)NULL)
         (*the_free)(curr->spec);
      if (curr->attr != (COLOUR_ATTR *)NULL)
         (*the_free)(curr->attr);
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return((RESERVED *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
RESERVED *rll_find(RESERVED *first,short row)
#else
RESERVED *rll_find(first,row)
RESERVED *first;
short row;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   RESERVED *curr=NULL;
   short i=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    rll_find");
   curr = first;
   if (curr != NULL)
   {
      for(i=0;i<row && curr->next != NULL; i++, curr=curr->next);
   }
   TRACE_RETURN();
   return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_DETAILS *parserll_add(PARSER_DETAILS *first,PARSER_DETAILS *curr,unsigned short size)
#else
PARSER_DETAILS *parserll_add(first,curr,size)
PARSER_DETAILS *first;
PARSER_DETAILS *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a PARSER_DETAILS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSER_DETAILS in linked list         */
/* curr       - pointer to current PARSER_DETAILS in linked list       */
/* size       - size of a PARSER_DETAILS item                          */
/* RETURN:    - pointer to next PARSER_DETAILS item                    */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSER_DETAILS *next=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parserll_add");

   if ((next=(PARSER_DETAILS *)(*the_malloc)(size)) != (PARSER_DETAILS *)NULL)
   {
      memset(next,0,sizeof(PARSER_DETAILS));
      if (curr == NULL)
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if (curr->next != NULL)
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_DETAILS *parserll_del(PARSER_DETAILS **first,PARSER_DETAILS **last,PARSER_DETAILS *curr,short direction)
#else
PARSER_DETAILS *parserll_del(first,last,curr,direction)
PARSER_DETAILS **first;
PARSER_DETAILS **last;
PARSER_DETAILS *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSER_DETAILS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parserll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL && curr->next == NULL)
   {
      (*the_free)(curr);
      *first = NULL;
      if (last != NULL)
         *last = NULL;
      TRACE_RETURN();
      return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL)
   {
      curr->next->prev = NULL;
      *first = new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
   if (curr->next == NULL)
   {
      curr->prev->next = NULL;
      new_curr = curr->prev;
      if (last != NULL)
         *last = curr->prev;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   if (direction == DIRECTION_FORWARD)
      new_curr = curr->next;
   else
      new_curr = curr->prev;

   (*the_free)(curr);
   curr = new_curr;
   TRACE_RETURN();
   return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_DETAILS *parserll_free(PARSER_DETAILS *first)
#else
PARSER_DETAILS *parserll_free(first)
PARSER_DETAILS *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSER_DETAILS *curr=NULL;
   PARSER_DETAILS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parserll_free");
   curr = first;
   while (curr != NULL)
   {
      if (curr->first_comments != NULL)
         (*the_free)(curr->first_comments);
      if (curr->first_keyword != NULL)
         (*the_free)(curr->first_keyword);
      if (curr->first_function != NULL)
         (*the_free)(curr->first_function);
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return((PARSER_DETAILS *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_DETAILS *parserll_find(PARSER_DETAILS *first,CHARTYPE *name)
#else
PARSER_DETAILS *parserll_find(first,name)
PARSER_DETAILS *first;
CHARTYPE *name;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 PARSER_DETAILS *curr=NULL;
 short i=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parserll_find");
   curr = first;
   if (curr != NULL)
   {
      for(i=0; curr != NULL; i++, curr=curr->next)
      {
         if (my_stricmp((DEFCHAR *)name,(DEFCHAR *)curr->parser_name) == 0)
         {
            TRACE_RETURN();
            return curr;
         }
      }
   }
   TRACE_RETURN();
   return(NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_KEYWORDS *parse_keywordll_add(PARSE_KEYWORDS *first,PARSE_KEYWORDS *curr,unsigned short size)
#else
PARSE_KEYWORDS *parse_keywordll_add(first,curr,size)
PARSE_KEYWORDS *first;
PARSE_KEYWORDS *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a PARSE_KEYWORDS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_KEYWORDS in linked list         */
/* curr       - pointer to current PARSE_KEYWORDS in linked list       */
/* size       - size of a PARSE_KEYWORDS item                          */
/* RETURN:    - pointer to next PARSE_KEYWORDS item                    */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_KEYWORDS *next=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_keywordll_add");

   if ((next=(PARSE_KEYWORDS *)(*the_malloc)(size)) != (PARSE_KEYWORDS *)NULL)
   {
      memset(next,0,sizeof(PARSE_KEYWORDS));
      if (curr == NULL)
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if (curr->next != NULL)
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_KEYWORDS *parse_keywordll_del(PARSE_KEYWORDS **first,PARSE_KEYWORDS **last,PARSE_KEYWORDS *curr,short direction)
#else
PARSE_KEYWORDS *parse_keywordll_del(first,last,curr,direction)
PARSE_KEYWORDS **first;
PARSE_KEYWORDS **last;
PARSE_KEYWORDS *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_KEYWORDS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_keywordll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL && curr->next == NULL)
   {
      (*the_free)(curr);
      *first = NULL;
      if (last != NULL)
         *last = NULL;
      TRACE_RETURN();
      return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL)
   {
      curr->next->prev = NULL;
      *first = new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
   if (curr->next == NULL)
   {
      curr->prev->next = NULL;
      new_curr = curr->prev;
      if (last != NULL)
         *last = curr->prev;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   if (direction == DIRECTION_FORWARD)
      new_curr = curr->next;
   else
      new_curr = curr->prev;

   (*the_free)(curr);
   curr = new_curr;
   TRACE_RETURN();
   return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_KEYWORDS *parse_keywordll_free(PARSE_KEYWORDS *first)
#else
PARSE_KEYWORDS *parse_keywordll_free(first)
PARSE_KEYWORDS *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_KEYWORDS *curr=NULL;
   PARSE_KEYWORDS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_keywordll_free");
   curr = first;
   while (curr != NULL)
   {
      if (curr->keyword != NULL)
         (*the_free)(curr->keyword);
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return((PARSE_KEYWORDS *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_FUNCTIONS *parse_functionll_add(PARSE_FUNCTIONS *first,PARSE_FUNCTIONS *curr,unsigned short size)
#else
PARSE_FUNCTIONS *parse_functionll_add(first,curr,size)
PARSE_FUNCTIONS *first;
PARSE_FUNCTIONS *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a PARSE_FUNCTIONS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_FUNCTIONS in linked list         */
/* curr       - pointer to current PARSE_FUNCTIONS in linked list       */
/* size       - size of a PARSE_FUNCTIONS item                          */
/* RETURN:    - pointer to next PARSE_FUNCTIONS item                    */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_FUNCTIONS *next=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_functionll_add");

   if ((next=(PARSE_FUNCTIONS *)(*the_malloc)(size)) != (PARSE_FUNCTIONS *)NULL)
   {
      memset(next,0,sizeof(PARSE_FUNCTIONS));
      if (curr == NULL)
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if (curr->next != NULL)
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_FUNCTIONS *parse_functionll_del(PARSE_FUNCTIONS **first,PARSE_FUNCTIONS **last,PARSE_FUNCTIONS *curr,short direction)
#else
PARSE_FUNCTIONS *parse_functionll_del(first,last,curr,direction)
PARSE_FUNCTIONS **first;
PARSE_FUNCTIONS **last;
PARSE_FUNCTIONS *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_FUNCTIONS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_functionll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL && curr->next == NULL)
   {
      (*the_free)(curr);
      *first = NULL;
      if (last != NULL)
         *last = NULL;
      TRACE_RETURN();
      return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL)
   {
      curr->next->prev = NULL;
      *first = new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
   if (curr->next == NULL)
   {
      curr->prev->next = NULL;
      new_curr = curr->prev;
      if (last != NULL)
         *last = curr->prev;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   if (direction == DIRECTION_FORWARD)
      new_curr = curr->next;
   else
      new_curr = curr->prev;

   (*the_free)(curr);
   curr = new_curr;
   TRACE_RETURN();
   return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_FUNCTIONS *parse_functionll_free(PARSE_FUNCTIONS *first)
#else
PARSE_FUNCTIONS *parse_functionll_free(first)
PARSE_FUNCTIONS *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_FUNCTIONS *curr=NULL;
   PARSE_FUNCTIONS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_functionll_free");
   curr = first;
   while (curr != NULL)
   {
      if (curr->function != NULL)
         (*the_free)(curr->function);
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return((PARSE_FUNCTIONS *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_HEADERS *parse_headerll_add(PARSE_HEADERS *first,PARSE_HEADERS *curr,unsigned short size)
#else
PARSE_HEADERS *parse_headerll_add(first,curr,size)
PARSE_HEADERS *first;
PARSE_HEADERS *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a PARSE_HEADERS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_HEADERS in linked list         */
/* curr       - pointer to current PARSE_HEADERS in linked list       */
/* size       - size of a PARSE_HEADERS item                          */
/* RETURN:    - pointer to next PARSE_HEADERS item                    */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_HEADERS *next=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_headerll_add");

   if ((next=(PARSE_HEADERS *)(*the_malloc)(size)) != (PARSE_HEADERS *)NULL)
   {
      memset(next,0,sizeof(PARSE_HEADERS));
      if (curr == NULL)
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if (curr->next != NULL)
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_HEADERS *parse_headerll_free(PARSE_HEADERS *first)
#else
PARSE_HEADERS *parse_headerll_free(first)
PARSE_HEADERS *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_HEADERS *curr=NULL;
   PARSE_HEADERS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_headerll_free");
   curr = first;
   while (curr != NULL)
   {
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return((PARSE_HEADERS *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_MAPPING *mappingll_add(PARSER_MAPPING *first,PARSER_MAPPING *curr,unsigned short size)
#else
PARSER_MAPPING *mappingll_add(first,curr,size)
PARSER_MAPPING *first;
PARSER_MAPPING *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a PARSER_MAPPING to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSER_MAPPING in linked list         */
/* curr       - pointer to current PARSER_MAPPING in linked list       */
/* size       - size of a PARSER_MAPPING item                          */
/* RETURN:    - pointer to next PARSER_MAPPING item                    */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSER_MAPPING *next=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    mappingll_add");

   if ((next=(PARSER_MAPPING *)(*the_malloc)(size)) != (PARSER_MAPPING *)NULL)
   {
      memset(next,0,sizeof(PARSER_MAPPING));
      if (curr == NULL)
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if (curr->next != NULL)
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_MAPPING *mappingll_del(PARSER_MAPPING **first,PARSER_MAPPING **last,PARSER_MAPPING *curr,short direction)
#else
PARSER_MAPPING *mappingll_del(first,last,curr,direction)
PARSER_MAPPING **first;
PARSER_MAPPING **last;
PARSER_MAPPING *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSER_MAPPING *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    mappingll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL && curr->next == NULL)
   {
      (*the_free)(curr);
      *first = NULL;
      if (last != NULL)
         *last = NULL;
      TRACE_RETURN();
      return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL)
   {
      curr->next->prev = NULL;
      *first = new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
   if (curr->next == NULL)
   {
      curr->prev->next = NULL;
      new_curr = curr->prev;
      if (last != NULL)
         *last = curr->prev;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   if (direction == DIRECTION_FORWARD)
      new_curr = curr->next;
   else
      new_curr = curr->prev;

   (*the_free)(curr);
   curr = new_curr;
   TRACE_RETURN();
   return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_MAPPING *mappingll_free(PARSER_MAPPING *first)
#else
PARSER_MAPPING *mappingll_free(first)
PARSER_MAPPING *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSER_MAPPING *curr=NULL;
   PARSER_MAPPING *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    mappingll_free");
   curr = first;
   while (curr != NULL)
   {
      if (curr->parser_name != NULL)
         (*the_free)(curr->parser_name);
      if (curr->filemask != NULL)
         (*the_free)(curr->filemask);
      if (curr->magic_number != NULL)
         (*the_free)(curr->magic_number);
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return((PARSER_MAPPING *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_MAPPING *mappingll_find(PARSER_MAPPING *first,CHARTYPE *filemask, CHARTYPE *magic_number)
#else
PARSER_MAPPING *mappingll_find(first,filemask,magic_number)
PARSER_MAPPING *first;
CHARTYPE *filemask;
CHARTYPE *magic_number;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 PARSER_MAPPING *curr=NULL;
 short i=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    mappingll_find");
   curr = first;
   if (curr != NULL)
   {
      for(i=0; curr != NULL; i++, curr=curr->next)
      {
         if (filemask != NULL)
         {
#ifdef UNIX
            if (curr->filemask
            &&  strcmp((DEFCHAR *)filemask,(DEFCHAR *)curr->filemask) == 0)
            {
               TRACE_RETURN();
               return curr;
            }
#else
            if (curr->filemask
            &&  my_stricmp(filemask,curr->filemask) == 0)
            {
               TRACE_RETURN();
               return curr;
            }
#endif
         }
         else
         {
#ifdef UNIX
            if (curr->magic_number
            &&  strcmp((DEFCHAR *)magic_number,(DEFCHAR *)curr->magic_number) == 0)
            {
               TRACE_RETURN();
               return curr;
            }
#else
            if (curr->magic_number
            &&  my_stricmp(magic_number,curr->magic_number) == 0)
            {
               TRACE_RETURN();
               return curr;
            }
#endif
         }
      }
   }
   TRACE_RETURN();
   return(NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_COMMENTS *parse_commentsll_add(PARSE_COMMENTS *first,PARSE_COMMENTS *curr,unsigned short size)
#else
PARSE_COMMENTS *parse_commentsll_add(first,curr,size)
PARSE_COMMENTS *first;
PARSE_COMMENTS *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a PARSE_COMMENTS to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_COMMENTS in linked list         */
/* curr       - pointer to current PARSE_COMMENTS in linked list       */
/* size       - size of a PARSE_COMMENTS item                          */
/* RETURN:    - pointer to next PARSE_COMMENTS item                    */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_COMMENTS *next=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_commentsll_add");

   if ((next=(PARSE_COMMENTS *)(*the_malloc)(size)) != (PARSE_COMMENTS *)NULL)
   {
      memset(next,0,sizeof(PARSE_COMMENTS));
      if (curr == NULL)
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if (curr->next != NULL)
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_COMMENTS *parse_commentsll_del(PARSE_COMMENTS **first,PARSE_COMMENTS **last,PARSE_COMMENTS *curr,short direction)
#else
PARSE_COMMENTS *parse_commentsll_del(first,last,curr,direction)
PARSE_COMMENTS **first;
PARSE_COMMENTS **last;
PARSE_COMMENTS *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_COMMENTS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_commentsll_del");
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL && curr->next == NULL)
   {
      (*the_free)(curr);
      *first = NULL;
      if (last != NULL)
         *last = NULL;
      TRACE_RETURN();
      return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL)
   {
      curr->next->prev = NULL;
      *first = new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
   if (curr->next == NULL)
   {
      curr->prev->next = NULL;
      new_curr = curr->prev;
      if (last != NULL)
         *last = curr->prev;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   if (direction == DIRECTION_FORWARD)
      new_curr = curr->next;
   else
      new_curr = curr->prev;

   (*the_free)(curr);
   curr = new_curr;
   TRACE_RETURN();
   return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_COMMENTS *parse_commentsll_free(PARSE_COMMENTS *first)
#else
PARSE_COMMENTS *parse_commentsll_free(first)
PARSE_COMMENTS *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first line for the file                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_COMMENTS *curr=NULL;
   PARSE_COMMENTS *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_commentsll_free");
   curr = first;
   while (curr != NULL)
   {
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return((PARSE_COMMENTS *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_POSTCOMPARE *parse_postcomparell_add(PARSE_POSTCOMPARE *first,PARSE_POSTCOMPARE *curr,unsigned short size)
#else
PARSE_POSTCOMPARE *parse_postcomparell_add(first,curr,size)
PARSE_POSTCOMPARE *first;
PARSE_POSTCOMPARE *curr;
unsigned short size;
#endif
/***********************************************************************/
/* Adds a PARSE_POSTCOMPARE to the current linked list after the current member.    */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_POSTCOMPARE in linked list      */
/* curr       - pointer to current PARSE_POSTCOMPARE in linked list    */
/* size       - size of a PARSE_POSTCOMPARE item                       */
/* RETURN:    - pointer to next PARSE_POSTCOMPARE item                 */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_POSTCOMPARE *next=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_postcomparell_add");

   if ((next=(PARSE_POSTCOMPARE *)(*the_malloc)(size)) != (PARSE_POSTCOMPARE *)NULL)
   {
      memset(next,0,sizeof(PARSE_POSTCOMPARE));
      if (curr == NULL)
      {
         first = next;
         next->next = NULL;
      }
      else
      {
         if (curr->next != NULL)
            curr->next->prev = next;
         next->next = curr->next;
         curr->next = next;
      }
      next->prev = curr;
   }
   TRACE_RETURN();
   return(next);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_POSTCOMPARE *parse_postcomparell_del(PARSE_POSTCOMPARE **first,PARSE_POSTCOMPARE **last,PARSE_POSTCOMPARE *curr,short direction)
#else
PARSE_POSTCOMPARE *parse_postcomparell_del(first,last,curr,direction)
PARSE_POSTCOMPARE **first;
PARSE_POSTCOMPARE **last;
PARSE_POSTCOMPARE *curr;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_POSTCOMPARE *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_postcomparell_del");
   if ( curr->string )
      (*the_free)(curr->string);
   if ( curr->is_class_type )
      the_regfree(&curr->pattern_buffer);
/*---------------------------------------------------------------------*/
/* Delete the only record                                              */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL && curr->next == NULL)
   {
      (*the_free)(curr);
      *first = NULL;
      if (last != NULL)
         *last = NULL;
      TRACE_RETURN();
      return(NULL);
   }
/*---------------------------------------------------------------------*/
/* Delete the first record                                             */
/*---------------------------------------------------------------------*/
   if (curr->prev == NULL)
   {
      curr->next->prev = NULL;
      *first = new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* Delete the last  record                                             */
/*---------------------------------------------------------------------*/
   if (curr->next == NULL)
   {
      curr->prev->next = NULL;
      new_curr = curr->prev;
      if (last != NULL)
         *last = curr->prev;
      (*the_free)(curr);
      curr = new_curr;
      TRACE_RETURN();
      return(curr);
   }
/*---------------------------------------------------------------------*/
/* All others                                                          */
/*---------------------------------------------------------------------*/
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   if (direction == DIRECTION_FORWARD)
      new_curr = curr->next;
   else
      new_curr = curr->prev;

   (*the_free)(curr);
   curr = new_curr;
   TRACE_RETURN();
   return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
PARSE_POSTCOMPARE *parse_postcomparell_free(PARSE_POSTCOMPARE *first)
#else
PARSE_POSTCOMPARE *parse_postcomparell_free(first)
PARSE_POSTCOMPARE *first;
#endif
/***********************************************************************/
/* Free up all allocated memory until the last item in the linked-list */
/* PARAMETERS:                                                         */
/* first      - pointer to first PARSE_POSTCOMPARE                     */
/* RETURN:    - NULL                                                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_POSTCOMPARE *curr=NULL;
   PARSE_POSTCOMPARE *new_curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("linked.c:    parse_postcomparell_free");
   curr = first;
   while (curr != NULL)
   {
      if ( curr->string )
         (*the_free)(curr->string);
      if ( curr->is_class_type )
         the_regfree(&curr->pattern_buffer);
      new_curr = curr->next;
      (*the_free)(curr);
      curr = new_curr;
   }
   TRACE_RETURN();
   return((PARSE_POSTCOMPARE *)NULL);
}
