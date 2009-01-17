/* This files solves the problems with instore macros.
 * While parsing a special tree of type tnode is created
 * and fold together with containing strings to one big
 * beast (called a tinned tree, american: canned heat :-) ).
 * Then we have two objects: a special tree and the tinned tree.
 * There is a one to one releationship between them.
 * The tinned tree may be expanded to a tree by ExpandTinnedTree().
 * The tree is tinned by TinTree() which should only be called from
 * the lexical analyser.
 * We are protected against multithreading while normal parsing.
 * Destroying the tree and tinning/expanding is done in
 * multithreaded mode.
 */

#include "rexx.h"
#include "rxiface.h"
#include <assert.h>

/* We begin with some odd tricks. We want a fast but memory
 * saving routine to build the tree. We don't want to copy
 * tree leaves more than one. Thus we pack them into buckets.
 * This allows a very fast kind of searching. Keep in mind
 * that we must create a pointerless list which is included
 * in the tinned tree.
 *
 * This structure is called ttree, have a look at regina_t.h.
 *
 * DEFAULT_TTREE_ELEMS defines the normal number of elements in the ttree.
 * The more elements the better is the performance in searching. The less
 * elements the better is the memory waste overhead. We want some more space
 * than MAX_INTERNAL_SIZE in memory.c. This leads to "normal" memory
 * allocation and can be freed without problems if we parse a nested
 * Rexx program. This is currently 2048. We allocate nearly 8K. This
 * will give a reasonable performance and a slight overhead for
 * external(!) functions, 4K in the middle.
 */
#define DEFAULT_TTREE_ELEMS (8192 / sizeof(treenode))

/* Same as before but for offsrclines: */
#define DEFAULT_OTREE_ELEMS (4096 / sizeof(offsrcline))

static ttree *CurrentT = NULL;
static otree *CurrentO = NULL;
static nodeptr Reused = NULL;
static const char MagicHeader[] = MAGIC;

/* NewProg indicates the start of a new parsing sequence. We don't allow
 * a nesting call. In case of an error is is the job of fetch_... to do the
 * cleanup.
 */
void NewProg(void)
{
   CurrentT = NULL; /* Trivial */
   CurrentO = NULL; /* Trivial */
   Reused = NULL; /* Can't reuse stuff of another parsing process */
}

/* EndProg is called at the end of the parsing process. Start is the
 * starting element of the later interpreter tree.
 * parser_data.root is assigned.
 */
void EndProg(nodeptr Start)
{
   parser_data.root = Start; /* trivial, too */
   CurrentT = NULL;
   CurrentO = NULL;
   Reused = NULL; /* Can't reuse stuff of another parsing process */
}

/* FreshNode returns a new ttree element. Call only within a NewProg/EndProg
 * calling sequence.
 * If you don't want the returned node WHILE PARSING because you want to do
 * some tricks the the node at a later time, you are allowed to call
 * RejectNode(). Rejected nodes are tried to be passed back to the used
 * nodes.
 */
nodeptr FreshNode(void)
{
   nodeptr h;

   if (Reused != NULL) /* This should be put back first */
   {
      h = Reused;
      Reused = Reused->next;
      h->next = NULL; /* Everything except nodeindex is 0 now */
      return(h);
   }

   if (CurrentT && (CurrentT->num < CurrentT->max)) /* bucket not full */
   {
      memset(CurrentT->elems + CurrentT->num, 0, sizeof(treenode));
      CurrentT->elems[CurrentT->num].nodeindex = CurrentT->sum + CurrentT->num;
      return(CurrentT->elems + CurrentT->num++);
   }

   if (CurrentT == NULL) /* First call */
   {
      parser_data.nodes = (ttree *)Malloc_TSD(parser_data.TSD, sizeof(ttree));
      CurrentT = parser_data.nodes;
      CurrentT->sum = 0;
   }
   else /* current bucket is full */
   {
      CurrentT->next = (ttree *)Malloc_TSD(parser_data.TSD, sizeof(ttree));
      CurrentT->next->sum = CurrentT->sum + CurrentT->num;
      CurrentT = CurrentT->next;
   }

   /* always */
   CurrentT->next = NULL;
   CurrentT->max = DEFAULT_TTREE_ELEMS;
   CurrentT->num = 1;
   CurrentT->elems = (treenode *)Malloc_TSD(parser_data.TSD,
                                CurrentT->max * sizeof(treenode));

   memset(CurrentT->elems, 0, sizeof(treenode));
   CurrentT->elems[0].nodeindex = CurrentT->sum;
   return(CurrentT->elems);
}

/* RejectNode gives the argument back to the pool of unused treenode entries
 * which are managed and passed back by FreshNode().
 * You should use the function ONLY IF YOU ARE WITHING THE PARSING PROCESS!
 * It is not guaranteed that the memory of the entry is freed. It can only
 * be reused.
 * Note that the content of the entry is NOT FREED in any kind.
 */
void RejectNode(nodeptr NoLongerUsed)
{
   unsigned long idx;

   assert(CurrentT != NULL);
   /* CurrentT == NULL can't happen, since CurrentT is only set within the
    * parsing process while at least one treenode has been returned.
    */
   if (CurrentT == NULL) /* In case of no assertion we return simply */
      return;

   /* Save exactly the nodeindex and destroy everything else */
   idx = NoLongerUsed->nodeindex;
   memset(NoLongerUsed, 0, sizeof(treenode)); /* Clean it up */
   NoLongerUsed->nodeindex = idx;

   NoLongerUsed->next = Reused;
   Reused = NoLongerUsed;
}

/* FreshLine returns a new otree element. Call only within a NewProg/EndProg
 * calling sequence.
 */
offsrcline *FreshLine(void)
{
   if (CurrentO && (CurrentO->num < CurrentO->max)) /* bucket not full */
   {
      memset(CurrentO->elems + CurrentO->num, 0, sizeof(offsrcline));
      return(CurrentO->elems + CurrentO->num++);
   }

   if (CurrentO == NULL) /* First call */
   {
      parser_data.srclines = (otree *)Malloc_TSD(parser_data.TSD, sizeof(otree));
      CurrentO = parser_data.srclines;
      CurrentO->sum = 0;
   }
   else /* current bucket is full */
   {
      CurrentO->next = (otree *)Malloc_TSD(parser_data.TSD, sizeof(otree));
      CurrentO->next->sum = CurrentO->sum + CurrentO->num;
      CurrentO = CurrentO->next;
   }

   /* always */
   CurrentO->next = NULL;
   CurrentO->max = DEFAULT_OTREE_ELEMS;
   CurrentO->num = 1;
   CurrentO->elems = (offsrcline *)Malloc_TSD(parser_data.TSD,
                                CurrentO->max * sizeof(offsrcline));

   memset(CurrentO->elems, 0, sizeof(offsrcline));
   return(CurrentO->elems);
}

/*****************************************************************************
 *****************************************************************************
 * start of the multithreaded part *******************************************
 *****************************************************************************
 *****************************************************************************/


/* DestroyNode kills all allocated elements within a nodeptr
 * without freeing the node itself.
 */
static void DestroyNode(const tsd_t *TSD, nodeptr p)
{
   int type ;

   if (p->name)
      Free_stringTSD( p->name ) ;

   if (p->now)
      FreeTSD( p->now ) ;

   type = p->type ;
   if (type == X_CON_SYMBOL || type == X_STRING)
   {
      if (p->u.number)
      {
         FreeTSD( p->u.number->num ) ;
         FreeTSD( p->u.number ) ;
      }
   }
   if (type==X_SIM_SYMBOL || type==X_STEM_SYMBOL || type==X_HEAD_SYMBOL ||
       type==X_CTAIL_SYMBOL || type==X_VTAIL_SYMBOL )
   {
      if (p->u.varbx)
      {
         detach( TSD, p->u.varbx ) ;
      }
   }

   if (type == X_CEXPRLIST)
   {
      if (p->u.strng)
         Free_stringTSD( p->u.strng ) ;
   }
}

/* DestroyInternalParsingTree frees all allocated memory used by a parsing
 * tree. The structure itself is not freed.
 */
void DestroyInternalParsingTree(const tsd_t *TSD, internal_parser_type *ipt)
{
   ttree *tr, *th;
   otree *otr, *oh;
   lineboxptr lr, lh;
   labelboxptr ar, ah;
   unsigned long i;

   if (!ipt)
      return;

   /* Cleanup all the nodes */
   if (ipt->nodes != NULL)
   {
      tr = ipt->nodes;

      while (tr)
      {
         for (i = 0; i < tr->num; i++)
            DestroyNode(TSD, tr->elems + i);
         th = tr->next;
         FreeTSD(tr->elems);
         FreeTSD(tr);
         tr = th;
      }

      ipt->nodes = NULL;
   }
   ipt->root = NULL; /* not really needed */

   /* Cleanup all the lineboxes */
   if (ipt->first_source_line != NULL)
   {
      lr = ipt->first_source_line;

      while (lr)
      {
         lh = lr->next;
         Free_stringTSD(lr->line);
         FreeTSD(lr);
         lr = lh;
      }

      ipt->first_source_line = ipt->last_source_line = NULL;
   }

   /* Cleanup all the labelboxes */
   if (ipt->first_label != NULL)
   {
      ar = ipt->first_label;

      while (ar)
      {
         ah = ar->next;
         FreeTSD(ar);
         ar = ah;
      }

      ipt->first_label = ipt->last_label = NULL;
   }

   if (ipt->sort_labels != NULL)
   {
      FreeTSD(ipt->sort_labels);

      ipt->sort_labels = NULL;
   }

   /* Cleanup the incore sourceline informations */
   /* Cleanup all the nodes */
   if (ipt->srclines != NULL)
   {
      otr = ipt->srclines;

      while (otr)
      {
         oh = otr->next;
         FreeTSD(otr->elems);
         FreeTSD(otr);
         otr = oh;
      }

      ipt->srclines = NULL;
   }
   if (ipt->kill)
      Free_stringTSD(ipt->kill);
   ipt->kill = NULL;
}

/* ExpandTinnedTree expands the external tree from a former parsing operation
 * to a fully usable tree. All allocations and relacations are done to fake
 * a normal parsing operation.
 * The external tree won't be used any longer after this operation but the
 * external tree must have been checked before this operation.
 * The freshly allocated tree is returned.
 */
internal_parser_type ExpandTinnedTree(const tsd_t *TSD,
                                      const external_parser_type *ept,
                                      unsigned long size,
                                      const char *incore_source,
                                      unsigned long incore_source_length)
{
   internal_parser_type ipt;
   unsigned long i,j;
   const extstring *es;
   const offsrcline *lastsrcline;
   nodeptr thisptr;

   memset(&ipt, 0, sizeof(ipt));

   /* We build the sourcelines first *****************************************/
   if (incore_source_length == 0)
      incore_source = NULL;
   if (ept->NumberOfSourceLines == 0)
      incore_source = NULL;
   if (incore_source) /* Its worth to check exactly */
   {
      lastsrcline = (const offsrcline *) ((char *) ept + ept->source);
      lastsrcline += ept->NumberOfSourceLines - 1;
      j = lastsrcline->length + lastsrcline->offset;
      /* j shall be very close to the end of the source string. It may
       * follow a linefeed (or carriage return/linefeed) and probably a
       * ^Z for CP/M descendents which includes Microsoft products. It's
       * fais to assume the following check:
       */
      if ((j > incore_source_length) ||
          (j + 3 < incore_source_length))
      incore_source = NULL;
   }
   if (incore_source) /* We are sure enough to use the source string */
   {
      ipt.incore_source = incore_source;
      ipt.srclines = (otree *)MallocTSD(sizeof(otree));
      ipt.srclines->sum = 0;
      ipt.srclines->next = NULL;
      ipt.srclines->max = ept->NumberOfSourceLines;
      ipt.srclines->num = ipt.srclines->max;
      ipt.srclines->elems = (offsrcline *)MallocTSD(ipt.srclines->num * sizeof(offsrcline));
      memcpy(ipt.srclines->elems,
             (char *) ept + ept->source,
             ipt.srclines->num * sizeof(offsrcline));
   }
   /**************************************************************************/

   ipt.tline = -1; /* unused */
   ipt.tstart = -1; /* unused */
   ipt.result = 0; /* unused */
   ipt.first_label = ipt.last_label = NULL; /* initialize it for newlabel() */
   ipt.numlabels = 0; /* initialize it for newlabel() */
   ipt.sort_labels = NULL; /* initialize it for newlabel() */

   ipt.nodes = (ttree *)MallocTSD(sizeof(ttree));
   ipt.nodes->sum = 0;
   ipt.nodes->next = NULL;
   ipt.nodes->max = ept->NumberOfTreeElements;
   ipt.nodes->num = ipt.nodes->max;
   ipt.nodes->elems = (treenode *)MallocTSD(ipt.nodes->num * sizeof(treenode));

   memcpy(ipt.nodes->elems,
          (char *) ept + ept->tree,
          ipt.nodes->num * sizeof(treenode));
   ipt.root = ipt.nodes->elems + ept->TreeStart;

   /* Everything is ready for a relocation step. Don't forget to *************
    * create the labelboxes as necessary.
    */
   for (i = 0;i < ept->NumberOfTreeElements;i++)
   {
      thisptr = ipt.nodes->elems + i;
      if (thisptr->name)
      {
         es = (extstring *) ((char *) ept + (unsigned long) thisptr->name);
         thisptr->name = Str_makeTSD(es->length);
         thisptr->name->len = es->length;
         memcpy(thisptr->name->value,
                es + 1 /* position of string content */,
                es->length);
      }

      /*
       * Do things the parsing step would have do. Simple values in thisptr->u
       * are copied already.
       */

      /*
       * See also several places in this file and in debug.c where this
       * switch list must be changed. Seek for X_CEXPRLIST.
       */
      switch ( thisptr->type )
      {
         case X_CEXPRLIST:
            if ( thisptr->u.strng )
            {
               es = (extstring *) ((char *) ept + (unsigned long) thisptr->u.strng);
               thisptr->u.strng = Str_makeTSD( es->length );
               thisptr->u.strng->len = es->length;
               memcpy( thisptr->u.strng->value,
                       es + 1 /* position of string content */,
                       es->length);
            }
            break;

         case X_LABEL:
            newlabel(TSD, &ipt, thisptr);
            break;

         default:
            break;
      }

      if (thisptr->next == (nodeptr) (unsigned long) -1)
         thisptr->next = NULL;
      else
         thisptr->next = ipt.nodes->elems + (unsigned long) thisptr->next;
      for (j = 0;j < sizeof(thisptr->p) / sizeof(thisptr->p[0]);j++)
      {
         if (thisptr->p[j] == (nodeptr) (unsigned long) -1)
            thisptr->p[j] = NULL;
         else
            thisptr->p[j] = ipt.nodes->elems + (unsigned long) thisptr->p[j];
      }
   }
   size = size; /* keep compiler happy */

   return(ipt);
}

/* We must take care of the alignment of structure. We may get a SIGBUS in
 * the following if we don't do it. We assume that an alignment for an
 * unsigned long is sufficient for all types including structures. We also
 * assume a power of two for an unsigned's size.
 */
#define USIZ sizeof(unsigned long)
#define USIZ_1 (USIZ-1)
/* Wastes one byte in average but is much faster */
#define StringSize(s) (((sizeof(extstring)+s->len)|USIZ_1)+1)

static unsigned long ComputeExternalSize(const internal_parser_type *ipt,
                                         unsigned long *SourceLines,
                                         unsigned long *Nodes)
{
   otree *otp;
   ttree *ttp;
   nodeptr np;
   unsigned long size = sizeof(external_parser_type);
   unsigned long i, elems, bufchars;

   /* sourceline table */
   elems = 0;
   if ((otp = ipt->srclines) == NULL)
   {
      if (ipt->last_source_line)
      {
         elems = ipt->last_source_line->lineno;
      }
   }
   else
   {
      while (otp->next)
         otp = otp->next;
      elems = otp->sum + otp->num;
   }
   *SourceLines = elems;
   size += elems * sizeof(offsrcline); /* the table */

   /* nodetable */
   elems = bufchars = 0;
   ttp = ipt->nodes;
   while (ttp)
   {
      for (i = 0;i < ttp->num;i++)
      {
         elems++;
         np = ttp->elems + i;
         if (np->name)
            bufchars += StringSize(np->name);

         /*
          * Add all sizes of strings that have been generated at the parsing
          * step.
          */

         /*
          * See also several places in this file and in debug.c where this
          * switch list must be changed. Seek for X_CEXPRLIST.
          */
         switch ( np->type )
         {
            case X_CEXPRLIST:
               if ( np->u.strng )
                  bufchars += StringSize( np->u.strng );
            break;

            default:
               break;
         }
      }
      ttp = ttp->next;
   }
   *Nodes = elems;
   size += elems * sizeof(treenode);
   size += bufchars;

   size += sizeof(((external_parser_type *)0)->Magic);
   return(size);
}

/* FillStrings copies all offsrclines from the otree to base+start
 * consecutively.
 * The index just beyond the last copied byte is returned.
 */
static unsigned long FillStrings(char *base, unsigned long start,
                                 const otree *otp)
{
   if (otp != NULL)
   {
      while (otp != NULL)
      {
         memcpy(base + start, otp->elems, otp->num * sizeof(offsrcline));
         start += otp->num * sizeof(offsrcline);
         otp = otp->next;
      }
   }
   return(start);
}

/* FillTree copies all treenodes of the ttree to base+buf in a relocatable
 * manner. Look at ExpandTinnedTree() or regina_t.h for a description.
 * Each treenode is copied to the table and the containing strings are copied
 * as extstrings to base+start which is incremented.
 * The table must be large enough.
 * The index just beyond the last copied character is returned.
 */
static unsigned long FillTree(treenode *table, char *base, unsigned long start,
                              const ttree *ttp)
{
   cnodeptr np;
   unsigned long i,j;
   extstring *e;

   while (ttp)
   {
      for (i = 0;i < ttp->num;i++)
      {
         np = (cnodeptr) (ttp->elems + i);
         *table = *np; /* Full copy includes unnecessary stuff but is fast */

         if (np->name)
         {
            table->name = (streng *) start;
            e = (extstring *) (base + start);
            e->length = np->name->len;
            memcpy(e + 1 /* just beyond the head */, np->name->value, e->length);
            start += StringSize(np->name);
         }

         /*
          * Remove all "flags" from the target and copy only approved values
          * the parser computes already.
          */
         memset( &table->u, 0, sizeof( table->u ) );

         /*
          * See also several places in this file and in debug.c where this
          * switch list must be changed. Seek for X_CEXPRLIST.
          */
         switch ( np->type )
         {
            case X_EQUAL:
            case X_DIFF:
            case X_GT:
            case X_GTE:
            case X_LT:
            case X_LTE:
               table->u.flags = np->u.flags;
               break;

            case X_PARSE:
               /*
                * fixes 972850
                */
               table->u.parseflags = np->u.parseflags;
               break;

            case X_ADDR_V:
               table->u.nonansi = np->u.nonansi;
               break;

            case X_CEXPRLIST:
               if ( np->u.strng )
               {
                  table->u.strng = (streng *) start;
                  e = (extstring *) (base + start);
                  e->length = np->u.strng->len;
                  memcpy(e + 1, np->u.strng->value, e->length);
                  start += StringSize(np->u.strng);
               }
            break;

            case X_LABEL:
               table->u.trace_only = np->u.trace_only;
               break;

            case X_ADDR_WITH:
               if ( !np->p[0] && !np->p[1] && !np->p[2] )
                  table->u.of = np->u.of;
               break;

            default:
               break;
         }

         if (table->next == NULL)
            table->next = (nodeptr) (unsigned long) -1;
         else
            table->next = (nodeptr) np->next->nodeindex;
         for (j = 0;j < sizeof(np->p) / sizeof(np->p[0]);j++)
         {
            if (table->p[j] == NULL)
               table->p[j] = (nodeptr) (unsigned long) -1;
            else
               table->p[j] = (nodeptr) np->p[j]->nodeindex;
         }
         table++;
      }
      ttp = ttp->next;
   }

   return(start);
}

/* TinTree "tins" a tree into an external structure. The complete structure
 * is allocated by one call to IfcAllocateMemory. The returned value shall
 * be used as an instore macro for RexxStart.
 * *length is set to the allocated size of the memory block on return.
 * ExpandedTinnedTree can expand the returned value and IsValidTin checks it.
 */
external_parser_type *TinTree(const tsd_t *TSD,
                              const internal_parser_type *ipt,
                              unsigned long *length)
{
   external_parser_type *retval;
   unsigned long srclines, nodecount, len;

   *length = ComputeExternalSize(ipt, &srclines, &nodecount);

   retval = (external_parser_type *)IfcAllocateMemory(*length);
   if (retval == NULL)
      return(NULL);
   memset(retval, 0, sizeof(external_parser_type));

   /* Build the envelope */
   len = sizeof(MagicHeader); /* includes a terminating 0 */
   if (len > sizeof(retval->Magic))
      len = sizeof(retval->Magic);
   memcpy(retval->Magic, MagicHeader, len);
   len = sizeof(PARSE_VERSION_STRING);
   if (len > sizeof(retval->ReginaVersion))
      len = sizeof(retval->ReginaVersion);
   memcpy(retval->ReginaVersion, PARSE_VERSION_STRING, len);

   retval->arch_detector.s.one = 1;
   retval->arch_detector.s.two = 2;
   retval->arch_detector.s.ptr3 = (void *)3;
   retval->arch_detector.s.ptr4 = (void *)4;
   retval->OverallSize = (unsigned long) *length;
   retval->NumberOfSourceLines = srclines;
   retval->version = INSTORE_VERSION;
   retval->NumberOfTreeElements = nodecount;

   retval->source = sizeof(external_parser_type);
   len = FillStrings((char *) retval,
                     sizeof(external_parser_type),
                     ipt->srclines);

   retval->tree = len;
   retval->TreeStart = ipt->root->nodeindex;
   len = FillTree((treenode *) ((char *) retval + len),
                  (char *) retval,
                  len + nodecount*sizeof(treenode),
                  ipt->nodes);

   memcpy((char *) retval + len, retval->Magic, sizeof(retval->Magic));

   assert((unsigned long) len + sizeof(retval->Magic) == *length);

   /* DEBUGGING: return NULL if you don't want tinned trees */
   TSD = TSD; /* keep compiler happy */
   return(retval);
}

/* IsValidTin returns 1 if the structure ept if of length eptlen and seems
 * to contain a valid parsing tree. 0 is returned if this is not the case.
 */
int IsValidTin(const external_parser_type *ept, unsigned long eptlen)
{
   char Magic[sizeof(((external_parser_type *)0)->Magic)];
   unsigned long len;

   /* Some paranoia tests first: */
   if ((ept == NULL) || (eptlen < sizeof(external_parser_type)))
      return(0);

   /* Be sure to fill Magic as described */
   memset(Magic, 0, sizeof(Magic));
   len = sizeof(MagicHeader); /* includes a terminating 0 */
   if (len > sizeof(ept->Magic))
      len = sizeof(ept->Magic);
   memcpy(Magic, MagicHeader, len);

   if (memcmp(Magic, ept->Magic, sizeof(Magic)) != 0)
      return(0);

   if ((ept->arch_detector.s.one != 1) ||
       (ept->arch_detector.s.two != 2) ||
       (ept->arch_detector.s.ptr3 != (void *)3) ||
       (ept->arch_detector.s.ptr4 != (void *)4))
      return(0);

   if (ept->OverallSize != eptlen)
      return(0);

   if (ept->version != INSTORE_VERSION)
      return(0);

   if (memcmp(Magic,
              (char *) ept + eptlen - sizeof(Magic),
              sizeof(Magic)) != 0)
      return(0);

   return(1);
}
