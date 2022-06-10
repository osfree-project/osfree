
/*
 *@@sourcefile bs_list.h:
 *      replacement for the STL list class template.
 *      All STL code has been removed from WarpIN
 *      with 0.9.14.
 *
 *      This list class only supplies a subset of the
 *      STL list functionality. The advantages of using
 *      this new implementation though are the following:
 *
 *      1)  This list template produces FAR less code
 *          bloat. Most code here is inlined and only
 *          consists of a couple of lines for every
 *          method, which mostly calls into the
 *          linklist.c code from the XWPhelpers. In
 *          other words, the list class template is
 *          only a wrapper around linklist.c for
 *          type-safety.
 *
 *      2)  WarpIN compiles almost twice as fast now
 *          because the incredibly complex STL headers
 *          with all those nested templates are gone.
 *
 *      3)  This implementation does not call copy
 *          constructors all the time. The STL is not
 *          good at managing pointers; instead, its
 *          containers assume to contain the real
 *          data at all times.
 *
 *          By contrast, the list code here _only_
 *          operates on pointers. With list<P>, it
 *          is assumed that P is a type that was
 *          created with new and that delete can
 *          (and will) be invoked on it.
 *          You cannot instantiate the list class for
 *          a non-pointer type.
 *
 *      The XWPHelpers linklist.c code has been used
 *      in XWorkplace forever and can be considered
 *      fairly bug-free.
 *
 *      This header supplies two class templates:
 *
 *      1)  list_iterator<P> is a type-safe wrapper
 *          around the LISTNODE. list_iterator++
 *          will give you the next node, for example,
 *          and *list_iterator gives you the actual
 *          data. This is very similar to the STL.
 *
 *      2)  list<P> is a type-safe wrapper around
 *          LINKLIST. It typedefs an iterator
 *          internally so you can use list<P>::iterator
 *          as with the STL. Again, this will only
 *          handle _pointer_ types.
 *
 *      Terminology:
 *
 *      --  list<P>: the list itself. A template wrapper
 *          around a LINKLIST.
 *
 *      --  list node: a node inside a list<P>, uses
 *          a LISTNODE inside.
 *
 *      --  list<P>::iterator: a template wrapper around
 *          a LISTNODE. *(list<P>::iterator) points
 *          to the data.
 *
 *      --  data: a pointer of type P that is stored with
 *          the list node.
 *
 *      One more difference to the STL is that the
 *      list operates in two modes. When constructing
 *      a list<P>, the constructor expects the
 *      mode to be specified, which must be one
 *      of the following:
 *
 *      --  STORE: whenever a node is deleted,
 *          the list should invoke delete on
 *          the data (P pointer) also.
 *
 *      --  SHADOW: delete must not be invoked.
 *
 *      Presently, the list constructor enforces the
 *      mode to be specified to make sure that the
 *      implementor makes a decision about which list
 *      mode should be used. This might change in
 *      the future.
 *
 *      Example usage:
 *
 +          struct MYSTRUCT
 +          {
 +              int dummy;
 +
 +              MYSTRUCT(i) {dummy = i;}
 +          }
 +
 +      // create a list of MYSTRUCT pointers
 +      list<MYSTRUCT*> MyList(STORE);
 +      // store new MYSTRUCT's in the list
 +      MyList.push_back(new MYSTRUCT(1));
 +      MyList.push_back(new MYSTRUCT(2));
 +
 +      // now run through the list
 +      list<MYSTRUCT*>::iterator start, end;
 +      start = MyList.begin();             // returns first list node
 +      end = MyList.end();                 // returns past last list node
 +      while (start != end)                // for all items
 +      {
 +          MYSTRUCT *pData = *start;       // dereference the data
 +          if (pData->dummy == 2)
 +              break;
 +
 +          // next list node
 +          start++
 +      }
 *
 *@@added V0.9.14 (2001-07-12) [umoeller]
 *@@include #include "helpers\linklist.h"
 */

#ifndef BSLIST_INCLUDED
    #define BSLIST_INCLUDED

    /* ******************************************************************
     *
     *  Replacement list class template
     *
     ********************************************************************/

    #include "helpers\linklist.h"

    template <class P>
    class list;

    /*
     *@@ list_iterator:
     *      a list_iterator<P> is a LISTNODE
     *      whose pNode->pItemData == P.
     *
     *      list<P> typedefs list<P>::iterator
     *      to be a list_iterator<P>.
     *
     *      P must be deletable (i.e. created
     *      on the heap with new()).
     */

    template <class P>
    class list_iterator
    {
        /*
        public:
            void* operator new(size_t n) { return malloc(n);};
            void operator delete(void* p) { free(p);};
        */

        public:
            PLISTNODE pNode;

            /*
             *@@ list_iterator:
             *      constructor.
             */

            inline list_iterator()
            {
                pNode = 0;
            }

            /*
             *@@ list_iterator:
             *      copy constructor to convert from a
             *      PLISTNODE.
             */

            inline list_iterator(const PLISTNODE p)
            {
                pNode = p;
            }

            /*
             *@@ ~list_iterator:
             *      destructor. Does nothing.
             */

            inline ~list_iterator()
            {

            }

            /*
             *@@ operator++:
             *      overloaded prefix ++ operator so we
             *      can easily run thru the list nodes.
             */

            void inline operator++()
            {
                if (pNode)
                    pNode = pNode->pNext;
            }

            /*
             *@@ operator++:
             *      overloaded postfix ++ operator so
             *      we can easily run thru the list nodes.
             *
             *      The int i is just stupid C++ syntax, the
             *      int is always zero, it's just for
             *      distinguishing between prefix and postfix.
             */

            void inline operator++(int i)
            {
                if (pNode)
                    pNode = pNode->pNext;
            }
    };

    template <class P>
    bool inline operator==(list_iterator<P> &p1, list_iterator<P> &p2)
    {
        return (p1.pNode == p2.pNode);
    }

    template <class P>
    bool inline operator==(const list_iterator<P> &p1, const list_iterator<P> &p2)
    {
        return (p1.pNode == p2.pNode);
    }

    template <class P>
    bool inline operator!=(list_iterator<P> &p1, list_iterator<P> &p2)
    {
        return (p1.pNode != p2.pNode);
    }

    template <class P>
    P inline operator*(list_iterator<P> &p)
    {
        return (P)p.pNode->pItemData;
    }

    /*
     *@@ list_reverse_iterator:
     *
     */

    template <class T>
    class list_reverse_iterator : public list_iterator<T>
    {
        public:
            inline list_reverse_iterator()
                : list_iterator<T>()
            {
            }

            inline list_reverse_iterator(const PLISTNODE p)
                : list_iterator<T>(p)
            {
            }

            void inline operator++()
            {
                if (pNode)
                    pNode = pNode->pPrevious;
            }

            // overload postfix ++ operator: first arg is class, second is int
            // (stupid compiler syntax, the int is always zero, it's just
            // for distinguishing between prefix and postfix)
            void inline operator++(int i)
            {
                if (pNode)
                    pNode = pNode->pPrevious;
            }
    };

    enum LISTMODE {SHADOW, STORE};

    /*
     *@@ list:
     *      a list<P> is a list whose LISTNODES
     *      point to instances of P.
     *
     *      As opposed to the STL list, this does
     *      not store copies of the data. Instead,
     *      it is assumed that P is deletable. It
     *      must be a pointer and have been created
     *      with new().
     *
     *      A list can operate in two modes. In
     *      "shadow" mode, the list will not invoke
     *      delete on the data pointers when the
     *      nodes are removed. In "store" mode,
     *      it will. It is not a good idea to have
     *      to "store" lists point to the same
     *      data members; in general, if you have
     *      several lists pointing to the same data,
     *      you should have one master list in
     *      "store" mode only.
     */

    template <class P>
    class list
    {
        /*
        public:
            void* operator new( size_t n) { return malloc(n);};
            void operator delete( void* p) { free(p);};
        */

        public:
            const LISTMODE    _mode;

        protected:
            LINKLIST    ll;

            void KillNode(PLISTNODE p)
            {
                P p2 = (P)p->pItemData;

                #ifdef __DEBUG__
                    BSRoot::WriteToDebugLog("list::KillNode for %s at 0x%lX\n",
                                            p2->QueryClassName(),
                                            p2);
                #endif

                if (_mode == STORE)
                    delete p2;
                lstRemoveNode(&ll, p);
            }

            /*
             *@@ CopyFrom:
             *      private helper for copy constructors and such.
             */

            void CopyFrom(const list<P> &llSource)
            {
                PLISTNODE p = lstQueryFirstNode((PLINKLIST)&llSource.ll);
                while (p)
                {
                    lstAppendItem(&ll, p->pItemData);

                    p = p->pNext;
                }
            }

        public:
            typedef list_iterator<P>            iterator;
            typedef list_reverse_iterator<P>    reverse_iterator;

            /*
             *@@ list:
             *      default constructor.
             */

            list(LISTMODE mode) //  = SHADOW)
                : _mode(mode)
            {
                lstInit(&ll, FALSE);
            }

            // copy constructor
            list(const list<P> &llSource)
                : _mode(SHADOW)
            {
                CopyFrom(llSource);
            }

            /*
             *@@ ~list:
             *      destructor.
             *
             *      If the list is in STORE mode,
             *      delete will be invoked on each
             *      list data item as well.
             */

            ~list()
            {
                clear();
            }

            /*
             *@@ operator=:
             *      copies the list nodes, but does NOT
             *      copy the data nodes.
             */

            const list<P> operator=(const list<P> &llSource)
            {
                CopyFrom(llSource);
                return *this;
            }

            /*
             *@@ push_front:
             *
             */

            void inline push_front(const P p)
            {
                lstInsertItemBefore(&ll,
                                    p,
                                    0);     // index to insert before
            }

            /*
             *@@ push_back:
             *
             */

            void inline push_back(const P p)
            {
                lstAppendItem(&ll, p);
            }

            /*
             *@@ remove:
             *      removes the list node which
             *      points to the specified data
             *      item.
             *
             *      If the list is in STORE mode,
             *      delete will be invoked on p's
             *      data item as well.
             */

            void inline remove(P p)
            {
                PLISTNODE pNode = lstNodeFromItem(&ll, p);
                if (pNode)
                    KillNode(pNode);
            }

            /*
             *@@ pop_back:
             *      removes the last element.
             *
             *      If the list is in STORE mode,
             *      delete will be invoked on the
             *      data item as well.
             */

            void inline pop_back()
            {
                PLISTNODE pNode = lstQueryLastNode(&ll);
                if (pNode)
                    KillNode(pNode);
            }

            /*
             *@@ erase:
             *      erases the specified list node.
             *      Returns the node after that node
             *      or end().
             *
             *      If the list is in STORE mode,
             *      delete will be invoked on the
             *      data item as well.
             */

            void inline erase(iterator pos)
            {
                KillNode(pos.pNode);
            }

            /*
             *@@ erase:
             *
             *      If the list is in STORE mode,
             *      delete will be invoked on each
             *      list data item as well.
             */

            void inline erase(iterator first,      // first to erase
                              iterator last)       // last to erase (not included)
            {
                PLISTNODE p = first.pNode;
                while (p)
                {
                    PLISTNODE pNext = p->pNext;

                    KillNode(p);

                    p = pNext;
                    if (p == last.pNode)
                        break;
                }
            }

            /*
             *@@ clear:
             *      empties the entire list.
             *
             *      If the list is in STORE mode,
             *      delete will be invoked on each
             *      list data item as well.
             */

            void clear()
            {
                PLISTNODE p = lstQueryFirstNode(&ll);
                while (p)
                {
                    PLISTNODE pNext = p->pNext;

                    KillNode(p);

                    p = pNext;
                }
            }

            /*
             *@@ empty:
             *      returns TRUE if the list is empty.
             */

            bool inline empty() const
            {
                return (lstCountItems(&ll) == 0);
            }

            /*
             *@@ count:
             *      returns the no.of items on the list.
             *
             *@@added V0.9.18 (2002-03-08) [umoeller]
             */

            unsigned long inline count() const
            {
                return (lstCountItems(&ll));
            }

            /*
             *@@ begin:
             *      returns a list iterator to the first
             *      list item.
             */

            list<P>::iterator inline begin() const
            {
                return lstQueryFirstNode(&ll);
            }

            /*
             *@@ end:
             *      returns a list iterator to point
             *      behind the last list item.
             *
             *      This is special. STL list defines
             *      the "end" iterator to point _past_
             *      the last element... it does not
             *      return the last element itself.
             *
             *      A regular STL item loop looks like
             *      this:
             *
             +      list<X>::iterator i;
             +      for (i = list.begin();
             +           i != list.end();
             +           i++)
             +      {
             +          X x = *i;
             +      }
             *
             *      So for convenience, we define end()
             *      to return NULL, because
             *      list_iterator::operator++ will set its
             *      member to NULL also if there's no next
             *      item.
             */

            const list<P>::iterator inline end() const
            {
                return (const list<P>::iterator)NULL;
            }

            /*
             *@@ rbegin:
             *      for reverse iteration. Returns the last
             *      item.
             */

            list<P>::reverse_iterator inline rbegin()
            {
                return lstQueryLastNode(&ll);
            }

            list<P>::reverse_iterator inline rend()
            {
                return (list<P>::reverse_iterator)NULL;
            }

            P inline back()
            {
                PLISTNODE pNode = lstQueryLastNode(&ll);
                if (pNode)
                    return (P)pNode->pItemData;

                return (P)NULL;
            }

            void inline sort(PFNSORTLIST pfnCompare)
            {
                lstQuickSort(&ll,
                             pfnCompare,
                             NULL);
            }
    };

#endif


