
/*
 *@@sourcefile bs_base.cpp:
 *      implements the BSRoot class, which was added
 *      with V0.9.14.
 *
 *@@header "cppbase\bs_base.h"
 *@@header "cppbase\bs_list.h"
 *@@header "cppbase\bs_errors.h"
 *@@added V0.9.14 (2001-07-12) [umoeller]
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

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSSEMAPHORES
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "setup.h"

// base includes
#include "cppbase\bs_base.h"

#pragma hdrstop

/* ******************************************************************
 *
 *  Root classes
 *
 ********************************************************************/

// class ID of BSRoot, which is the ONLY class ID
// whose parent is NULL
BSClassID BSRoot::tBSRoot("BSRoot", NULL);

#ifdef __DEBUG__
    FILE*       G_DebugLogFile = NULL;

    /*
     *@@ OpenDebugLog:
     *
     */

    void BSRoot::OpenDebugLog(const char *pcszDir)
    {
        if (!G_DebugLogFile)
        {
            CHAR sz[256];
            strcpy(sz, pcszDir);
            strcat(sz, "\\");
            strcat(sz, "debug.log");
            G_DebugLogFile = fopen(sz, "a");
            fprintf(G_DebugLogFile, "\n\nDebug log opened\n\n");
        }
    }

    /*
     *@@ WriteToDebugLog:
     *
     */

    void BSRoot::WriteToDebugLog(const char *pcszFormat, ...)
    {
        if (G_DebugLogFile)
        {
            va_list     args;
            va_start(args, pcszFormat);
            vfprintf(G_DebugLogFile, pcszFormat, args);
            va_end(args);
        }
    }
#endif

/*
 *@@ BSRoot:
 *      default constructor. This is protected to
 *      make sure that derived classes won't
 *      forget to specify their class ID when
 *      constructing objects.
 */

BSRoot::BSRoot(BSClassID &Class)
    : _Class(Class)
{
    #ifdef __DEBUG__
        if (G_DebugLogFile)
        {
            fprintf(G_DebugLogFile,
                    "Created %s at 0x%lX\n",
                    _Class._pcszClassName,
                    this);
            fflush(G_DebugLogFile);
        }
    #endif
}

/*
 *@@ BSRoot:
 *      protected copy constructor.
 *
 *      This is only provided so that classes
 *      derived from BSRoot need not always
 *      specify a copy constructor themselves.
 */

BSRoot::BSRoot(const BSRoot &p)
    : _Class(p._Class)
{
}

/*
 *@@ operator=:
 *      protected assignment operator.
 *
 *      This is only provided so that classes
 *      derived from BSRoot need not always
 *      specify an assigment operator themselves.
 */

BSRoot& BSRoot::operator=(const BSRoot &p)
{
    _Class = p._Class;
    return *this;
}

/*
 *@@ ~BSRoot:
 *      public virtual destructor.
 */

BSRoot::~BSRoot()
{
    #ifdef __DEBUG__
        if (G_DebugLogFile)
        {
            fprintf(G_DebugLogFile,
                    "Deleted %s at 0x%lX\n",
                    _Class._pcszClassName,
                    this);
            fflush(G_DebugLogFile);
        }
    #endif
}

/*
 *@@ IsA:
 *      returns true if the object is an instance of
 *      the given class, or if the given class is a
 *      parent class of the object's class.
 *
 *      This is similar to the somIsA() method in SOM
 *      and can be very useful for testing whether an
 *      object supports a certain method (before doing
 *      an explicit typecast).
 *
 *      For example, assume the following hierarchy:
 *
 +              BSRoot
 +                  +-- MyGrandParentClass
 +                          +-- MyParentClass
 +                                  +-- MyClass
 *
 *      Assume that "obj" is an instance of MyParentClass.
 *      Invoking obj.isA() will return:
 *
 +              obj.isA(BSRoot::tBSRoot) == true
 +              obj.isA(MyGrandParentClass::tMyGrandParentClass) == true
 +              obj.isA(MyParentClass::tMyParentClass) == true
 +              obj.isA(MyClass::tMyClass) == false
 */

bool BSRoot::IsA(BSClassID &Class) const
{
    BSClassID *c = &_Class;
    while (c)
    {
        if (c == &Class)
            return true;

        c = c->_pParentClass;
    }

    return false;
}

/*
 *@@ QueryClassName:
 *      returns the name of the class that this object
 *      is an instance of.
 */

const char* BSRoot::QueryClassName() const
{
    return (_Class._pcszClassName);
}

/* ******************************************************************
 *
 *  Locks
 *
 ********************************************************************/

/*
 *@@ BSMutex:
 *
 *@@added V0.9.20 (2002-07-06) [umoeller]
 */

BSMutex::BSMutex()
{
    DosCreateMutexSem(NULL, &_hmtx, 0, FALSE);
}

/*
 *@@ ~BSMutex:
 *
 *@@added V0.9.20 (2002-07-06) [umoeller]
 */

BSMutex::~BSMutex()
{
    // DosCloseMutexSem(_hmtx);
    // do not close the sem, or we can't use strings
    // during termination
}

/*
 *@@ Request:
 *
 *@@added V0.9.20 (2002-07-06) [umoeller]
 */

int BSMutex::Request() const
{
    APIRET arc;
    if (arc = DosRequestMutexSem(_hmtx, SEM_INDEFINITE_WAIT))
    {
        DosBeep(1000, 100);
        throw arc;
    }

    return true;
}

/*
 *@@ Release:
 *
 *@@added V0.9.20 (2002-07-06) [umoeller]
 */

int BSMutex::Release() const
{
    DosReleaseMutexSem(_hmtx);
    return true;
}


