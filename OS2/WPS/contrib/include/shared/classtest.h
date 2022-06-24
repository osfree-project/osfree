
/*
 *@@sourcefile classtest.h:
 *      header file for classtest.c.
 *
 *@@include #include <wpobject.h>
 *@@include #include "setup\classtest.h"
 */

/*
 *      Copyright (C) 2002-2008 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef CLASSTEST_HEADER_INCLUDED
    #define CLASSTEST_HEADER_INCLUDED

    VOID ctsSetClassFlags(WPObject *somSelf,
                          PULONG pfl);

    BOOL ctsIsAbstract(WPObject *somSelf);

    BOOL ctsIsShadow(WPObject *somSelf);

    BOOL ctsIsTransient(WPObject *somSelf);

    BOOL ctsIsMinWin(WPObject *somSelf);

    BOOL ctsIsIcon(WPObject *somSelf);

    BOOL ctsIsPointer(WPObject *somSelf);

    BOOL ctsIsImageFile(WPObject *somSelf);

    BOOL ctsIsCommandFile(WPObject *somSelf);

    SOMClass* ctsResolveWPUrl(VOID);

    BOOL ctsIsRootFolder(WPObject *somSelf);

    BOOL ctsIsSharedDir(WPObject *somSelf);

    BOOL ctsIsServer(WPObject *somSelf);

    BOOL ctsIsNetgrp(WPObject *somSelf);

    BOOL ctsDescendedFromSharedDir(SOMClass *pClassObject);

    BOOL ctsIsDisk(WPObject *somSelf);

    BOOL ctsIsPrinter(WPObject *somSelf);

    BOOL ctsIsTrashable(WPObject *somSelf);

#endif

