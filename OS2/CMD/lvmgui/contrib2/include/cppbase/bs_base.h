
/*
 * bs_base.h:
 *
 *@@added V0.9.14 (2001-07-12) [umoeller]
 *@@include #include "base\bs_base.h"
 */

/*
 *      This file Copyright (C) 2001 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef BSBASE_HEADER_INCLUDED
    #define BSBASE_HEADER_INCLUDED

    /* ******************************************************************
     *
     *  Root classes
     *
     ********************************************************************/

    /*
     *@@ BSClassID:
     *      represents a class ID for a WarpIN class
     *      derived from BSRoot.
     *
     *      This class is used for WarpIN run-time
     *      type information (RTTI) since we do not
     *      use the C++ facilities for that (VAC
     *      doesn't support those).
     *
     *      If you're familiar with SOM, a BSClassID
     *      can be compared to a SOM "class object",
     *      which is an instance of a class's metaclass.
     *
     *      Each instance of BSRoot contains a reference
     *      to its BSClassID so we can check each
     *      object's class at runtime.
     *      No matter how many instances of a class
     *      exist, there will be only one BSClassID
     *      instance for all instances of the class.
     *
     *      The BSRoot constructor takes a reference
     *      to the class's BSClassID as its only (and
     *      required) parameter, which is then stored
     *      in the BSRoot instance data. This way, at
     *      any time, each object can query its own
     *      class and climb up the tree of parent
     *      classes, if desired.
     *
     *      Unless the BSClassID instance represents
     *      BSRoot itself, the pParentClass member
     *      will always point to the parent class,
     *      which may be the BSRoot id if the class
     *      is derived from BSRoot directly.
     *
     *      This concept doesn't support multiple
     *      inheritance at this point, but that's not
     *      used in WarpIN anyway.
     */

    class BSClassID
    {
        public:
            const char  *_pcszClassName;        // clear class name (e.g. "MyClass")
            BSClassID   *_pParentClass;       // pointer to parent class ID; if NULL,
                                                // this represents BSRoot.

            BSClassID(const char *pcszClassName,
                    BSClassID *pParentClass)
            {
                _pcszClassName = pcszClassName;
                _pParentClass = pParentClass;
            }
    };

    /*
     *@@ BSRoot:
     *      the root class of the WarpIN class hierarchy.
     *
     *      BSRoot implements the following:
     *
     *      --  run-time type identification; each object
     *          can inspect its _Class member and always
     *          know which class it is an instance of.
     *
     *      --  "derived from" queries: invoke IsA() on
     *          an object and find out whether the object
     *          is an instance of a certain class.
     *
     *      --  list<P> support; while this isn't really
     *          necessary, the debug list implementation
     *          invokes BSRoot methods for logging, so
     *          presently all objects that are supposed
     *          to be stored with a linked list (see
     *          bs_list.h) must be instances of BSRoot
     *          or a class derived from BSRoot.
     *
     *      BSRoot itself cannot be instantiated because
     *      the constructor is protected. A class that
     *      derives from BSRoot must do three things:
     *
     *      1)  Declare the class ID in the class's
     *          declaration, like this:
     *
     +          class MyClass : public BSRoot
     +          {
     +              public: DECLARE_CLASS(MyClass);
     +          }
     *
     *          DECLARE_CLASS(MyClass) expands to
     *
     +              static BSClassID tMyClass;
     *
     *          BSClassID represents a "class object" for
     *          each class. See BSClassID for details.
     *          You can later reference the class's
     *          ID by using MyClass::tMyClass from
     *          anywhere. The "t" prefix is arbitrary,
     *          but if you use the macros, you better
     *          stick with it.
     *
     *      2)  Define the class ID in your source file.
     *          Since the class ID is just a regular
     *          static class variable, it must be initialized
     *          statically, and once only (in the source
     *          file).
     *
     *          Another macro was defined for this, which
     *          expects the class's parent class as well:
     *
     +              DEFINE_CLASS(MyClass, BSRoot);      // MyClass is derived from BSRoot
     *
     *          The parent class better match the class
     *          declaration. This macro expands to:
     *
     +              BSClassID MyClass::tMyClass("MyClass",            // class name string
     +                                          &BSRoot::tBSRoot)     // parent class object
     *
     *          In other words, the DECLARE_CLASS and DEFINE_CLASS
     *          macros are just a quick way to declare and
     *          implement a static BSClassID class variable.
     *
     *      3)  Your class's constructors must always call
     *          the BSRoot constructor explicitly in the
     *          initializer list. Give your own class ID to
     *          BSRoot's constructor. This will properly set
     *          BSRoot's _Class member.
     *
     *          Since BSRoot::BSRoot is protected, the compiler
     *          will properly warn you if you forget this.
     *
     *          For example:
     *
     +              MyClass::MyClass(int iDummy)
     +                  : BSBase(tMyClass)
     +              {...}
     *
     *      After all this, BSRoot gives you the following nice
     *      features:
     *
     *      1)  BSRoot::QueryClassName() returns the
     *          name of the class that the object really
     *          is an instance of.
     *
     *      2)  BSRoot::IsA() returns true if the object
     *          is an instance of the given class.
     *
     *@@changed V0.9.14 (2001-08-03) [umoeller]: overrode new and delete ptrs
     */

    class BSRoot
    {
        // class methods
        public:
            static BSClassID tBSRoot;

        public:
            #ifdef __DEBUG__
                static void OpenDebugLog(const char *pcszDir);
                static void WriteToDebugLog(const char *pcszFormat, ...);
                static void CloseDebugLog();
            #endif

        // instance methods
        public:
            BSClassID     &_Class;

        protected:
            BSRoot(BSClassID &Class);

            BSRoot(const BSRoot &p);
            BSRoot& operator=(const BSRoot &p);

        public:
            virtual ~BSRoot();

            bool IsA(BSClassID &Class) const;

            const char* QueryClassName() const;

         /*
         public:
            void* operator new(size_t n) { return malloc(n); };
            void operator delete(void* p) { free(p); };
         */
    };

    /*
     *@@ DECLARE_CLASS:
     *      handy macro to declare class object for
     *      use with BSRoot and BSClassID.
     *
     *      This should appear in the class declaration.
     *
     */

    #define DECLARE_CLASS(cls) static BSClassID t##cls

    /*
     *@@ DEFINE_CLASS:
     *      handy macro to define a class object with
     *      its parent class object for use with BSRoot
     *      and BSClassID.
     *
     *      For each class that was declared with DECLARE_CLASS,
     *      a corresponding DEFINE_CLASS must appear in the
     *      source somewhere.
     */

    #define DEFINE_CLASS(cls, parent) BSClassID cls::t##cls(#cls, &parent::t##parent)

    /*
     *@@ DYNAMIC_CAST:
     *      freaky macro to simulate a dynamic cast.
     *
     *      Usage:
     *
     +      DYNAMIC_CAST(BSDerived, pobj, pSource);
     *
     *      expands to
     *
     +      BSDerived *pobj = (pSource->IsA(BSDerived::tBSDerived) ? ((BSDerived*)pSource) : 0;
     */

    #define DYNAMIC_CAST(cls, pT, p) cls *pT = ((p)->IsA(cls::t##cls)) ? (cls*)(p) : 0

    /* ******************************************************************
     *
     *  Locks
     *
     ********************************************************************/

    /*
     *@@ BSMutex:
     *      representation of an OS/2 mutex, to
     *      be used within BSLock.
     *
     *@@added V0.9.19 (2002-05-07) [umoeller]
     */

    class BSMutex
    {
        unsigned long _hmtx;

        public:
            BSMutex();
            ~BSMutex();
            bool Request() const;
            bool Release() const;
    };

    /*
     *@@ BSLock:
     *      exception-safe mutex implementation.
     *      Creating an instance of this requests
     *      the BSMutex given in the constructor,
     *      and the destructor releases it.
     *
     *@@added V0.9.19 (2002-05-07) [umoeller]
     */

    class BSLock
    {
        const BSMutex   &_mtx;
        int             _cRequests;

        public:
            BSLock(const BSMutex &mtx)
                : _mtx(mtx)
            {
                if (mtx.Request())
                    ++_cRequests;
            }

            ~BSLock()
            {
                if (    (_cRequests)
                     && (_mtx.Release())
                   )
                    --_cRequests;
            }
    };

#endif

