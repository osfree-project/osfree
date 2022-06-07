
/*
 *@@sourcefile bs_refptr.h:
 *      template definition for REFPOINTER.b
 */


/*
 *@@ REFPOINTER:
 *      generic pointer class template, which allows us to
 *      to store pointers in STL containers and still have
 *      operators work right.
 *
 *      The problem with pointers and the STL is that the
 *      operators such as "<" are not invoked on the object
 *      directly, but on the pointer, so that sorting etc.
 *      occurs according to pointer values.
 *
 *      It doesn't really make that much sense to sort items
 *      according to storage addresses in memory... so
 *      instead of storing "real" pointers, you can store
 *      instances of this class template, which supports
 *      the "*", "->", "==", "<" operators.
 *
 *      Declare your list like this:
 +          list< REFPOINTER<Class> > ClassesList;
 *
 *      Mind the spaces in "> >", because otherwise the
 *      compiler considers this an ">>" operator. There
 *      are no asterices (*) anywhere.
 *
 *      Use the "*" or "->" operators on the REFPOINTER
 *      to get the actual class instance.
 */

template <class T>
class REFPOINTER
{
    private:
        T* ptr;

    public:
        // default constructor
        inline REFPOINTER()
        { ptr = 0; }

        // destructor
        inline ~REFPOINTER()
        { }

        // constructor from object
        inline REFPOINTER(T& t)
        { ptr = &t; }

        // copy constructor 1
        inline REFPOINTER(const REFPOINTER<T>& X)
        { ptr = X.ptr; }

        // copy constructor 2
        inline REFPOINTER(const void* p)
        { ptr = (T*)p; }

        // dereference
        inline T& operator*()
                  const
        { return *ptr; }

        inline T* operator->()
                  const
        { return ptr; }

        // conversion to pointer to class T
        inline operator T*()
        { return ptr; }

        // () operator
        inline operator bool()
                        const
        { return ptr != 0; }

        // ! operator
        inline bool operator!()
                    const
        { return ptr == 0; }

        // comparison
        inline friend bool operator == (const REFPOINTER<T>& left,
                                        const REFPOINTER<T>& right)
        {
            return *(left.ptr) == *(right.ptr);
        }

        inline friend bool operator < (const REFPOINTER<T>& left,
                                       const REFPOINTER<T>& right)
        {
            return *(left.ptr) < *(right.ptr);
        }
};


