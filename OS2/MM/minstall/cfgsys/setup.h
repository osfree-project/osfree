
/*
 *@@setup.h:
 *      common include file for _all_ XWorkplace code
 *      files (src\main and src\helpers) which defines
 *      common flags for debugging etc.
 *
 *      If this file is changed, this will cause the
 *      makefiles to recompile _all_ XWorkplace sources,
 *      because this is included will all source files.
 *
 *      Include this _after_ os2.h and standard C includes
 *      (stdlib.h et al), but _before_ any project includes,
 *      because this modifies some standard definitions.
 */

#ifdef SETUP_HEADER_INCLUDED
    #error setup.h included twice.
#else
    #define SETUP_HEADER_INCLUDED

    /*************************************************************
     *
     *   Common declarations
     *
     *************************************************************/

    // STATIC is now used with V1.0.0 (2002-09-02) [umoeller]
    // instead of "static" for static functions. Unfortunately
    // I spent a lot of work on making functions static a while
    // ago to reduce the stress on the linker and avoid name
    // conflicts, but only now discovered that static functions
    // do not appear in the map files and therefore not in the
    // sym files shipped with XWP either, which makes my trap
    // logs a lot less useful.

    // To be able to switch back to the "real" static functions
    // later, I now define STATIC to do nothing, but this could
    // be switched back eventually.

    // #define STATIC static
    #define STATIC

    // XWPENTRY defines the standard linkage for the
    // XWorkplace helpers.

    // VAC:
    #if defined(__IBMC__) || defined(__IBMCPP__)
        #define XWPENTRY _Optlink
    #else
    // EMX or Watcom
        #define XWPENTRY _System
    #endif

    // All these have been added with V0.9.12 (2001-05-18) [umoeller].
    // If you run into trouble with these in one of your code files,
    // after including setup.h, #undef the respective flag.

    // enable wrappers in include\helpers\winh.h
    #define WINH_STANDARDWRAPPERS
    // and include\helpers\dosh.h
    #define DOSH_STANDARDWRAPPERS

    /*************************************************************
     *
     *   Page tuning
     *
     *************************************************************/

    // dosh.c
    #pragma alloc_text(FREQ_CODE1, doshSleep)
    #pragma alloc_text(FREQ_CODE1, doshRequestMutexSem)
    #pragma alloc_text(FREQ_CODE1, doshSetExceptionHandler)
    #pragma alloc_text(FREQ_CODE1, doshUnsetExceptionHandler)
    #pragma alloc_text(FREQ_CODE1, PerformMatch)
    #pragma alloc_text(FREQ_CODE1, doshMatchCaseNoPath)
    #pragma alloc_text(FREQ_CODE1, doshQuerySysUptime)
    #pragma alloc_text(FREQ_CODE1, doshFindEAValue)

    #pragma alloc_text(FREQ_CODE1, nlsDBCS)
    #pragma alloc_text(FREQ_CODE1, nlschr)
    #pragma alloc_text(FREQ_CODE1, nlsrchr)
    #pragma alloc_text(FREQ_CODE1, nlsUpper)

    #pragma alloc_text(FREQ_CODE1, strhdup)

    // winh.c
    #pragma alloc_text(FREQ_CODE1, winhRequestMutexSem)

    // common.c
    #pragma alloc_text(FREQ_CODE1, cmnQuerySetting)
    #pragma alloc_text(FREQ_CODE1, cmnGetString)
    #pragma alloc_text(FREQ_CODE1, nlsGetString)
    #pragma alloc_text(FREQ_CODE1, cmnLockObject)

    /*
     *  functions used with wpQueryIcon and
     *  extended associations
     *
     */

    // linklist.c
    #pragma alloc_text(FREQ_CODE1, lstInit)
    #pragma alloc_text(FREQ_CODE1, lstCreate)
    #pragma alloc_text(FREQ_CODE1, lstQueryFirstNode)
    #pragma alloc_text(FREQ_CODE1, lstCountItems)
    #pragma alloc_text(FREQ_CODE1, lstNodeFromIndex)
    #pragma alloc_text(FREQ_CODE1, lstNodeFromItem)
    #pragma alloc_text(FREQ_CODE1, lstAppendItem)
    #pragma alloc_text(FREQ_CODE1, lstClear)
    #pragma alloc_text(FREQ_CODE1, lstFree)

    // tree.c
    #pragma alloc_text(FREQ_CODE1, treeFind)
    #pragma alloc_text(FREQ_CODE1, treeCompareKeys)
    #pragma alloc_text(FREQ_CODE1, treeCompareStrings)

    // object.c
    #pragma alloc_text(FREQ_CODE1, objResolveIfShadow)
    #pragma alloc_text(FREQ_CODE1, objIsAFolder)
    #pragma alloc_text(FREQ_CODE1, LockHandlesCache)
    #pragma alloc_text(FREQ_CODE1, UnlockHandlesCache)
    #pragma alloc_text(FREQ_CODE1, objFindObjFromHandle)

    // xfdataf.c
    #pragma alloc_text(FREQ_CODE1, xdf_wpQueryIcon)
    #pragma alloc_text(FREQ_CODE1, xdf_wpQueryAssociatedProgram)
    #pragma alloc_text(FREQ_CODE1, xdf_wpQueryAssociatedFileIcon)

    // filetype.c
    #pragma alloc_text(FREQ_CODE1, prfhQueryKeysForApp)
    #pragma alloc_text(FREQ_CODE1, prfhQueryProfileData)

    #pragma alloc_text(FREQ_CODE1, ListAssocsForType)
    #pragma alloc_text(FREQ_CODE1, ftypForEachAutoType)
    #pragma alloc_text(FREQ_CODE1, fncbBuildAssocsList)
    #pragma alloc_text(FREQ_CODE1, BuildAssocsList)
    #pragma alloc_text(FREQ_CODE1, FreeAssocsList)
    #pragma alloc_text(FREQ_CODE1, ftypQueryAssociatedProgram)

    /*
     *  functions used with exe icons
     *
     */

    #pragma alloc_text(FREQ_CODE2, xpgf_wpSetProgIcon)
    #pragma alloc_text(FREQ_CODE2, doshGetExtension)

    #pragma alloc_text(FREQ_CODE2, krnLock)
    #pragma alloc_text(FREQ_CODE2, krnUnlock)

    #pragma alloc_text(FREQ_CODE2, icoLoadICOFile)
    #pragma alloc_text(FREQ_CODE2, doshOpen )
    #pragma alloc_text(FREQ_CODE2, doshReadAt)
    #pragma alloc_text(FREQ_CODE2, doshQueryPathSize )
    #pragma alloc_text(FREQ_CODE2, doshQueryFileSize )
    #pragma alloc_text(FREQ_CODE2, icoBuildPtrHandle )
    #pragma alloc_text(FREQ_CODE2, doshLockFile)
    #pragma alloc_text(FREQ_CODE2, doshUnlockFile)
    #pragma alloc_text(FREQ_CODE2, doshClose )

    #pragma alloc_text(FREQ_CODE2, exehOpen)
    #pragma alloc_text(FREQ_CODE2, exehClose)

    #pragma alloc_text(FREQ_CODE2, doshAllocArray)

    #pragma alloc_text(FREQ_CODE2, exehLoadLXMaps )
    #pragma alloc_text(FREQ_CODE2, exehFreeLXMaps)

    #pragma alloc_text(FREQ_CODE2, GetOfsFromPageTableIndex)
    #pragma alloc_text(FREQ_CODE2, ExpandIterdata1)
    #pragma alloc_text(FREQ_CODE2, ExpandIterdata2)
    #pragma alloc_text(FREQ_CODE2, memcpyb)
    #pragma alloc_text(FREQ_CODE2, memcpyw)
    #pragma alloc_text(FREQ_CODE2, exehReadLXPage )

    #pragma alloc_text(FREQ_CODE2, exehLoadLXResource )

    #pragma alloc_text(FREQ_CODE2, ConvertWinIcon)
    #pragma alloc_text(FREQ_CODE2, LoadWinNEResource )

    // skipping OS2 NE (too rare)
    // todo: LoadWinPEResource

    #pragma alloc_text(FREQ_CODE2, xpgf_xwpQueryProgType)
    #pragma alloc_text(FREQ_CODE2, progQueryProgType)

    #pragma alloc_text(FREQ_CODE2, progFindIcon)

    #pragma alloc_text(FREQ_CODE2, icoLoadExeIcon)

    #pragma alloc_text(FREQ_CODE2, LockIcons)
    #pragma alloc_text(FREQ_CODE2, UnlockIcons)
    #pragma alloc_text(FREQ_CODE2, cmnGetStandardIcon)

    /*************************************************************
     *
     *   Feature selections
     *
     *************************************************************/

    // This section describes what features can be disabled
    // selectively. The source code reacts to those #defines.

    #ifdef __XWPLITE__
        #include "_features_lite.h"
    #endif

    /*************************************************************
     *
     *   Debug info setup
     *
     *************************************************************/

    /*
     *  The following #define's determine whether additional debugging
     *  info will be compiled into xfldr.dll.
     *
     *  1)  The general DONTDEBUGATALL flag will disable all other
     *      debugging flags, if defined. DONTDEBUGATALL gets set
     *      automatically if __DEBUG__ is not defined (that is,
     *      if DEBUG is disabled in setup.in).
     *
     *      Alternatively, set DONTDEBUGATALL explicitly here to
     *      disable the debugging flags completely. Of course, that
     *      does not affect compiler options.
     *
     *  2)  If DONTDEBUGATALL is not set (i.e. if we're in debug mode),
     *      the various flags below are taken into account. Note that
     *      many of these are from very old XFolder versions, and
     *      I cannot guarantee that these will compile any more.
     *
     *      Anyway, this setup allows us to finally enable certain
     *      groups of Pmprintf output. In debug mode, there's a
     *      new page in XWPSetup which allows you to set which output
     *      should be enabled.
     *
     *      To add a new Pmprintf group, do the following:
     *
     *      1)  Add one block to the list below to define both
     *          a new member in the DEBUGGINGFLAGS enumeration
     *          and a corresponding PMPF_* macro.
     *
     *      2)  Add the new enum value to the G_aDebugDescrs
     *          array in shared\xsetup.c.
     */

    // disable debugging if debug code is off
    #ifndef __DEBUG__
        #define DONTDEBUGATALL
    #endif

    // or set it here explicitly even though debugging is on:
        // #define DONTDEBUGATALL

    #ifndef DONTDEBUGATALL

        // If the following is commented out, no PMPRINTF will be
        // used at all. XWorkplace uses Dennis Bareis' PMPRINTF
        // package to do this.

        // **** IMPORTANT NOTE: if you use this flag, you _must_
        // have the PMPRINTF DLLs somewhere on your LIBPATH, or
        // otherwise XFLDR.DLL cannot be loaded, because the imports
        // will fail. That is, XWorkplace classes can neither be registered
        // nor loaded at Desktop startup. This has cost me a lot of thought
        // once, and you'll get no error message, so be warned.
            //#define _PMPRINTF_
            //#define _WPSDEBUG_

        typedef enum _DEBUGGINGFLAGS
        {
    #endif



/* general debugging */

        // the following will printf language code queries and
        // NLS DLL evaluation
    #ifndef DONTDEBUGATALL
            DBGSET_LANGCODES,
                #define PMPF_LANGCODES(b) { if (G_aDebugs[DBGSET_LANGCODES]) PmpfF(b); }
    #else
                #define PMPF_LANGCODES(b)
    #endif

    #ifndef DONTDEBUGATALL
        // debug notebook.c callbacks
            DBGSET_NOTEBOOKS,
                #define PMPF_NOTEBOOKS(b) { if (G_aDebugs[DBGSET_NOTEBOOKS]) PmpfF(b); }
    #else
                #define PMPF_NOTEBOOKS(b)
    #endif

    #ifndef DONTDEBUGATALL

/* object handling */

        // debug wpRestoreData and such
        // WARNING: this produces LOTS of output
            DBGSET_RESTOREDATA,
                #define PMPF_RESTOREDATA(b) { if (G_aDebugs[DBGSET_RESTOREDATA]) PmpfF(b); }
    #else
                #define PMPF_RESTOREDATA(b)
    #endif

    #ifndef DONTDEBUGATALL
        // debug icon replacements
            DBGSET_ICONREPLACEMENTS,
                #define PMPF_ICONREPLACEMENTS(b) { if (G_aDebugs[DBGSET_ICONREPLACEMENTS]) PmpfF(b); }
    #else
                #define PMPF_ICONREPLACEMENTS(b)
    #endif

    #ifndef DONTDEBUGATALL

/* startup, shutdown */

        // debug startup (folder, archives) processing
            DBGSET_STARTUP,
                #define PMPF_STARTUP(b) { if (G_aDebugs[DBGSET_STARTUP]) PmpfF(b); }
    #else
                #define PMPF_STARTUP(b)
    #endif

    #ifndef DONTDEBUGATALL
        // the following allows debug mode for XShutdown, which
        // will be enabled by holding down SHIFT while selecting
        // "Shutdown..." from the desktop context menu. In addition,
        // you'll get some PMPRINTF info and beeps
            DBGSET_SHUTDOWN,
                #define PMPF_SHUTDOWN(b) { if (G_aDebugs[DBGSET_SHUTDOWN]) PmpfF(b); }
    #else
                #define PMPF_SHUTDOWN(b)
    #endif

    #ifndef DONTDEBUGATALL

/* folder debugging */

        // the following gives information on ordered folder content
        // (sorting by .ICONPOS etc.)
            DBGSET_ORDEREDLIST,
                #define PMPF_ORDEREDLIST(b) { if (G_aDebugs[DBGSET_ORDEREDLIST]) PmpfF(b); }
    #else
                #define PMPF_ORDEREDLIST(b)
    #endif

    #ifndef DONTDEBUGATALL
        // the following will printf wpAddToContent
            DBGSET_CNRCONTENT,
                #define PMPF_CNRCONTENT(b) { if (G_aDebugs[DBGSET_CNRCONTENT]) PmpfF(b); }
    #else
                #define PMPF_CNRCONTENT(b)
    #endif

    #ifndef DONTDEBUGATALL
        // the following displays internal status bar data
            DBGSET_STATUSBARS,
                #define PMPF_STATUSBARS(b) { if (G_aDebugs[DBGSET_STATUSBARS]) PmpfF(b); }
    #else
                #define PMPF_STATUSBARS(b)
    #endif

    #ifndef DONTDEBUGATALL
        // the following will printf lots of sort info
            DBGSET_SORT,
                #define PMPF_SORT(b) { if (G_aDebugs[DBGSET_SORT]) PmpfF(b); }
    #else
                #define PMPF_SORT(b)
    #endif

    #ifndef DONTDEBUGATALL
        // the following will printf folder/global hotkey info
            DBGSET_KEYS,
                #define PMPF_KEYS(b) { if (G_aDebugs[DBGSET_KEYS]) PmpfF(b); }
    #else
                #define PMPF_KEYS(b)
    #endif

    #ifndef DONTDEBUGATALL
        // the following displays a lot of infos about menu
        // processing (msgs), esp. for folder content menus
            DBGSET_MENUS,
                #define PMPF_MENUS(b) { if (G_aDebugs[DBGSET_MENUS]) PmpfF(b); }
    #else
                #define PMPF_MENUS(b)
    #endif

    #ifndef DONTDEBUGATALL
        // the following debugs turbo folders and fast
        // content trees
            DBGSET_TURBOFOLDERS,
                #define PMPF_TURBOFOLDERS(b) { if (G_aDebugs[DBGSET_TURBOFOLDERS]) PmpfF(b); }
    #else
                #define PMPF_TURBOFOLDERS(b)
    #endif

    #ifndef DONTDEBUGATALL
        // the following debugs painting cnr backgrounds
        // (split view only presently)
            DBGSET_CNRBITMAPS,
                #define PMPF_CNRBITMAPS(b) { if (G_aDebugs[DBGSET_CNRBITMAPS]) PmpfF(b); }
    #else
                #define PMPF_CNRBITMAPS(b)
    #endif

    #ifndef DONTDEBUGATALL
        // this debugs the split view populate thread
            DBGSET_SPLITVIEW,
                #define PMPF_SPLITVIEW(b) { if (G_aDebugs[DBGSET_SPLITVIEW]) PmpfF(b); }
    #else
                #define PMPF_SPLITVIEW(b)
    #endif

    #ifndef DONTDEBUGATALL
        // this debugs rootfolders/disks
            DBGSET_DISK,
                #define PMPF_DISK(b) { if (G_aDebugs[DBGSET_DISK]) PmpfF(b); }
    #else
                #define PMPF_DISK(b)
    #endif

    #ifndef DONTDEBUGATALL

/* file ops debugging */

        // debug title clash dialog
            DBGSET_TITLECLASH,
                #define PMPF_TITLECLASH(b) { if (G_aDebugs[DBGSET_TITLECLASH]) PmpfF(b); }
    #else
                #define PMPF_TITLECLASH(b)
    #endif

    #ifndef DONTDEBUGATALL
        // debug data/program file associations/icons
            DBGSET_ASSOCS,
                #define PMPF_ASSOCS(b) { if (G_aDebugs[DBGSET_ASSOCS]) PmpfF(b); }
    #else
                #define PMPF_ASSOCS(b)
    #endif

    #ifndef DONTDEBUGATALL
        // debug file operations engine
            DBGSET_FOPS,
                #define PMPF_FOPS(b) { if (G_aDebugs[DBGSET_FOPS]) PmpfF(b); }
    #else
                #define PMPF_FOPS(b)
    #endif

    #ifndef DONTDEBUGATALL
        // debug trashcan
            DBGSET_TRASHCAN,
                #define PMPF_TRASHCAN(b) { if (G_aDebugs[DBGSET_TRASHCAN]) PmpfF(b); }
    #else
                #define PMPF_TRASHCAN(b)
    #endif

    #ifndef DONTDEBUGATALL

/* program objects */
        // debug program startup data
            DBGSET_PROGRAMSTART,
                #define PMPF_PROGRAMSTART(b) { if (G_aDebugs[DBGSET_PROGRAMSTART]) PmpfF(b); }
    #else
                #define PMPF_PROGRAMSTART(b)
    #endif

    #ifndef DONTDEBUGATALL

/* misc */

        // debug new system sounds
            DBGSET_SOUNDS,
                #define PMPF_SOUNDS(b) { if (G_aDebugs[DBGSET_SOUNDS]) PmpfF(b); }
    #else
                #define PMPF_SOUNDS(b)
    #endif

    #ifndef DONTDEBUGATALL

/* misc */

        // debug new XWPObjList class V1.0.1 (2002-12-11) [umoeller]
            DBGSET_OBJLISTS,
                #define PMPF_OBJLISTS(b) { if (G_aDebugs[DBGSET_OBJLISTS]) PmpfF(b); }
    #else
                #define PMPF_OBJLISTS(b)
    #endif

    #ifndef DONTDEBUGATALL      // V1.0.5 (2006-06-04) [pr]: Added SOM debug flag

/* SOM */

        // debug SOM methods
            DBGSET_SOMMETHODS,
                #define PMPF_SOMMETHODS(b) { if (G_aDebugs[DBGSET_SOMMETHODS]) PmpfF(b); }
    #else
                #define PMPF_SOMMETHODS(b)
    #endif

    #ifndef DONTDEBUGATALL

        // debug SOM weirdos
            DBGSET_SOMFREAK,
                #define PMPF_SOMFREAK(b) { if (G_aDebugs[DBGSET_SOMFREAK]) PmpfF(b); }
    #else
                #define PMPF_SOMFREAK(b)
    #endif

    #ifndef DONTDEBUGATALL      // V1.0.5 (2006-06-04) [pr]: Added CLASSLIST debug flag

/* Class List */

        // debug Class List
            DBGSET_CLASSLIST,
                #define PMPF_CLASSLIST(b) { if (G_aDebugs[DBGSET_CLASSLIST]) PmpfF(b); }
    #else
                #define PMPF_CLASSLIST(b)
    #endif

    #ifndef DONTDEBUGATALL

            __LAST_DBGSET

        } DEBUGGINGFLAGS;

        extern char G_aDebugs[__LAST_DBGSET];

    #endif

    /********************************************************************
     *
     *   Global #include's
     *
     ********************************************************************/

    #ifdef OS2_INCLUDED
        // the following reacts to the _PMPRINTF_ macro def'd above;
        // if that's #define'd, _Pmpf(()) commands will produce output,
        // if not, no code will be produced.
        #ifdef _PMPRINTF_
          #include "pmprintf.h"
          #include "wpsdebug.h"
        #endif

        // SOMMethodDebug is the macro defined for all those
        // xxxMethodDebug thingies created by the SOM compiler.
        // If you have uncommended DEBUG_SOMMETHODS above, this
        // will make sure that _Pmpf is used for that. In order
        // for this to work, you'll also need _PMPRINTF_.
        #ifdef SOMMethodDebug
            #undef  SOMMethodDebug
        #endif

        // V1.0.5 (2006-06-04) [pr]
        #define  SOMMethodDebug(c,m) PMPF_SOMMETHODS(("%s::%s", c,m))
    #endif

    #define _min(a,b) ( ((a) > (b)) ? b : a )
    #define _max(a,b) ( ((a) > (b)) ? a : b )

    // all this added V0.9.2 (2000-03-10) [umoeller]
    #if ( defined (  __IBMCPP__ ) && (  __IBMCPP__ < 400 ) )
        typedef int bool;
        #define true 1
        #define false 0
        #define _BooleanConst    // needed for some VAC headers, which define bool also
    #endif

    #ifndef __stdlib_h          // <stdlib.h>
        #include <stdlib.h>
    #endif
    #ifndef __string_h          // <string.h>
        #include <string.h>
    #endif

    /*
    #ifdef __XWPLITE__
        #ifndef __DEBUG__
            // no trap logs with eWorkplace
            #define __NO_LOUD_EXCEPTION_HANDLERS__
        #endif
    #endif
    */

    #ifdef __DEBUG__
        // enable memory debugging; comment out this line
        // if you don't want it
        // #define __XWPMEMDEBUG__

        #include "memdebug.h"

        // allow _interrupt(3) only on my private machine
        // or we'll trap every other developer machine too
        #ifdef __INT3__
            #define INT3() _interrupt(3)
        #else
            #define INT3()
        #endif
    #else
        #define INT3()
    #endif

    /*************************************************************
     *
     *   DBCS support
     *
     *************************************************************/

    // XWP uses strchr and the like in many places, which are
    // not DBCS-aware. So globally replace these funcs with the
    // NLS-aware replacements in the helpers.
    // V0.9.20 (2002-07-03) [umoeller]

    #ifndef DONT_REPLACE_FOR_DBCS
        #include "nls.h"

        #ifdef strchr
            #undef strchr
        #endif
        #define strchr(p, c) nlschr(p, c)

        #ifdef strrchr
            #undef strrchr
        #endif
        #define strrchr(p, c) nlsrchr(p, c)
    #endif

#endif

