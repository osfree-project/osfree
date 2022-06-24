
XWorkplace Source README
(W) Ulrich M”ller, July 2, 1999
Last updated July 2, 2002, Ulrich M”ller


0. CONTENTS OF THIS FILE
========================

    1. LICENSE, COPYRIGHT, DISCLAIMER
    2. INTRODUCTION
    3. COMPILATION/INSTALLATION
    4. CONTRIBUTORS


1. LICENSE, COPYRIGHT, DISCLAIMER
=================================

    Copyright (C) 1997-2003 Ulrich M”ller
                            and others (see the individual source files).

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as contained in
    the file COPYING in this distribution.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


2. INTRODUCTION
===============

    Welcome to the XWorkplace source code.

    Since XWorkplace is a fairly complex beast, I have tried to add
    plenty of documentation, most of which is contained in the
    PROGREF.INF file in the main directory of the source tree.

    In order to get to know XWorkplace's structure, you might first
    want to read the "XWorkplace Internals" section in the XWorkplace
    User Guide, which comes with the binary distribution already.

    The README that is used for the latest _binary_ release is in the
    "001\readme" file.


3. COMPILATION/INSTALLATION
===========================

    See PROGREF.INF for detailed instructions.

    NOTE: The build process has been changed with V0.9.12.
    PROGREF.INF has been updated to reflect these changes.

    Essentially, if you managed to build XWP before, you
    need to

    1)  take a look at the new "config.in" file and adjust it to
        match your system (this replaces the environment
        variables which were necessary previously);

    2)  run "nmake dep" once to create .depend files in the
        subdirectories;

    3)  run "nmake really_all" to build the whole system. As
        opposed to plain "nmake", this builds the NLS and a
        few extra programs too.


    Working With Netlabs CVS
    ------------------------

    Please see "cvs.txt" in this directory.


4. CONTRIBUTORS
===============

    Newer contributions (since about V0.9.1) are marked in xdoc
    style. The following author acronyms are used in the code:

        umoeller            Ulrich M”ller (djmutex@xworkplace.org)

        dk                  Dmitry Kubov <Dmitry@north.cs.msu.su>
        jsmall              John Small (jsmall@lynxus.com)
        lafaix              Martin Lafaix (lafaix@ibm.net)
        pr                  Paul Ratcliffe (paul@orac.clara.co.uk)
        kso                 Knut Stange Osmundsen (bird@anduin.net)

