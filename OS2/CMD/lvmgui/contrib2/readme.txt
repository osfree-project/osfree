XWP Helpers 0.9.7 README
(W) Ulrich M”ller, October 26, 2000
Last updated Feb 03, 2002 Ulrich M”ller


0. CONTENTS OF THIS FILE
========================

    1. LICENSE, COPYRIGHT, DISCLAIMER
    2. INTRODUCTION
    3. EXECUTABLE TOOLS
    4. CREATING CODE DOCUMENTATION
    5. COMPILING
    6. INCLUDING HEADER FILES


1. LICENSE, COPYRIGHT, DISCLAIMER
=================================

    Copyright (C) 1997-2001 Ulrich M”ller,
                            Christian Langanke,
                            and others (see the individual source files).

    Most of this library is published under the GNU General Public Licence.
    You can redistribute it and/or modify those parts under the terms of the
    GNU General Public License as contained in the file COPYING in the
    main directory.

    Parts of this library are published under MIT licence. See the
    COPYING.MIT file in the main directory.

    See the individual source files for what licence applies.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


2. INTRODUCTION
===============

    Welcome to the XWorkplace Helpers.

    This CVS archive is intended to support OS/2 developers with any
    code they might need writing OS/2 programs.

    The XWPHelpers are presently used in XWorkplace and WarpIN. They
    started out from various code snippets I created for XFolder,
    the predecessor of XWorkplace. I then isolated the code which could
    be used independently and put that code into separate directories
    in the WarpIN CVS repository (also at Netlabs).

    At Warpstock Europe 2000 in Karlsruhe, I talked to a number of
    developers and then decided that this should become an independent
    Netlabs CVS archive so that other people can more easily contribute.

    Even though the helpers are called "XWorkplace helpers", they
    have nothing to do with WPS and SOM programming. They can help
    any OS/2 programmer.

    The XWPHelpers offer you frequently used code for writing all
    sorts of OS/2 programs, including:

    --  standard C code which is independent of the OS/2 platform
        (and should even work for Unix or Windows);

    --  OS/2-specific code which can be used in any OS/2 program
        (VIO or PM);

    --  PM-specific code which assists you in writing PM programs.

    The XWPHelpers can be compiled with EMX/GCC or IBM VisualAge
    C++ 3.08. They can be used with C or C++ programs. All typecasts
    should be C++-compatible.


    Getting Sources from Netlabs CVS
    --------------------------------

    First set the CVS enviroment:
        CVSROOT=:pserver:guest@www.netlabs.org:d:/netlabs.src/xwphelpers
        USER=guest

    Then, to check out the most current XWPHelpers sources, create
    a subdirectory in your CVS root dir called "xwphelpers".

    Do a "cvs login" with "readonly" as your password and do a
    "cvs checkout ." from the "xwphelpers" subdirectory. Don't forget
    the dot.

    Alternatively, use the Netlabs Open Source Archive Client (NOSAC).
    See http://www.netlabs.org/nosa for details.

    In any case, I strongly recommend to create a file in $(HOME)
    called ".cvsrc" and add "cvs -z9" in there to enable maximum
    compression during transfers. This greatly speeds up things.


3. EXECUTABLE TOOLS
===================

    The root directory of this repository contains a few executables
    in binary form that are used by XWorkplace and WarpIN.


    --  FastDep.exe was written by Knut Stange Osmundsen (taken from
        the Odin sources) and is used by the "nmake dep" facility
        of both WarpIN and XWorkplace. This scans C source files
        and writes dependencies for the makefiles automatically.

    --  h2i.exe was written by me (the sources are in the xworkplace
        repository, in the "tools" directory) to translate a bunch
        of HTML files to IPF.

    --  strrpl.exe was written by me (the sources are in the warpin
        repository, in the "tools" directory) to replace strings in
        text files.

    --  xdoc.exe was written by me (the sources are in the warpin
        repository, in the "tools" directory) to extract documentation
        from C source files directory. See "Creating code documentation"
        below.


4. CREATING CODE DOCUMENTATION
==============================

    The XWPHelpers do not come with pre-made documentation. However,
    you can automatically have extensive, categorized documentation
    generated automatically from the C sources using my "xdoc"
    utility, which resides in the main directory of the helpers.
    (The source code for xdoc is in the WarpIN CVS repository
    because it shares some C++ code with WarpIN.)

    To have the code generated, simply call "createdoc.cmd" in the
    main directory. This will call xdoc in turn with the proper
    parameters and create a new "HTML" directory, from where you
    should start with the "index.html" file.


5. COMPILING
============

    You don't have to worry about compilation if you only want
    to compile XWorkplace or WarpIN. The makefiles of those two
    projects are set up properly to automatically compile the
    XWPHelpers as well. This section is only for people who
    want to compile the XWPHelpers separately for use in a
    different project.

    Compiling is a bit tricky because the code and the makefiles
    were designed to be independent of any single project. As a
    result, I had to used environment variables in order to pass
    parameters to the makefiles.

    --  The most important environment variable is PROJECT_BASE_DIR.
        This should point to the root directory of your own project.

    --  In the "make" subdirectory of that directory, the helpers
        makefiles expect a file called "setup.in" which sets up more
        environment variables. You can take the ones from XWorkplace
        or WarpIN as a template. Those setup.in files in turn expect
        a config.in in PROJECT_BASE_DIR itself, but that is no
        precondition required by the helpers makefiles themselves.

        Here's an example: Say you have a CVS root directory of
        "C:\cvs" on your system. The XWPHelpers reside in
        "C:\cvs\xwphelpers". Your own project resides in
        "C:\cvs\myproject" (plus subdirectories). So set
        PROJECT_BASE_DIR to "C:\cvs\myproject", create
        "C:\cvs\myproject\make", and put setup.in in there.

    --  OUTPUTDIR_HELPERS must point to the directory where the
        output .obj and .lib files should be created.

    --  Note that the XWPHelpers also expect a "setup.h" header
        file to be somewhere on your INCLUDE path. See remarks
        below.

    Of course, nothing stops you from writing your own makefile
    if you find all this too complicated. However, if you choose
    to use my makefile from within your own project, you can
    then simply change to the src\helpers directory and start a
    second nmake from your own makefile like this:

        @cd xxx\src\helpers
        nmake -nologo "PROJECT_BASE_DIR=C:\myproject" "OUTPUTDIR_HELPERS=C:\myproject\bin" "MAINMAKERUNNING=YES"
        @cd olddir


6. INCLUDING HEADER FILES
=========================

    The "include policy" of the helpers is that the "include"
    directory in the helpers source tree should be part of your
    include path. This way you can include helper headers in
    your own project code using

        #include "helpers\header.h"

    so that the helpers headers won't interfere with your own
    headers.

    Note that all the helpers C code includes their own include
    files this way. As a result, the XWPHelpers "include"
    directory must be in your include path, or this won't
    compile.

    I have a "flat" include policy, meaning that include files
    may not include other files. Instead, #include's may only
    occur in C files. This makes makefile dependencies easier
    to maintain.

    Besides, the helpers C code expects a file called "setup.h"
    in your include path somewhere. This is included by _all_
    the C files so you can (re)define certain macros there.
    XWorkplace and WarpIN both have such a header file in their
    respective "include" directories.

    With V0.9.7, many function prototypes have been changed in
    the helpers headers to allow exporting them in a DLL.
    (This was necessary for creating XWorkplace plugin DLLs.)
    As a result, you now MUST define XWPENTRY in your setup.h
    to contain the linkage for the helpers functions. This
    can look like this:

        #ifdef __EMX__
            // with EMX, do nothing; EMX always uses _System linkage
            #define XWPENTRY
        #elif defined (__IBMCPP__) || defined (__IBMC__)
            // with VAC, use _Optlink; that's faster than _System
            #define XWPENTRY _Optlink
            // or: #define XWPENTRY _System
        #endif


