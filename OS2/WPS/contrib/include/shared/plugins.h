
/*
 *@@sourcefile plugins.h:
 *      public header file for the plugins.
 *
 *      This contains all declarations which are needed by
 *      all parts of the XCenter and to implement widget
 *      plugin DLLs.
 *
 *      If you are looking at this file from the "toolkit\shared"
 *      directory of a binary XWorkplace release, this is an
 *      exact copy of the file in "include\shared" from the
 *      XWorkplace sources.
 *
 *@@include #include "shared\plugins.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#ifndef PLUGINS_HEADER_INCLUDED
    #define PLUGINS_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Public definitions
     *
     ********************************************************************/

    /*
     *@@ PLUGINCLASS:
     *      describes one plugin class which can be used by
     *      clients. Such a class can either be internal or in
     *      a plugin DLL.
     */

    typedef struct _PLUGINCLASS
    {
       ULONG       ulUser1;
       ULONG       ulUser2;

       const char  *pcszClass;
                // internal plugin class name; this is used to identify
                // the class. This must be unique on the category and must
                // not contain special characters like commas, brackets,
                // etc. Besides, since this must work with all codepages,
                // use only ASCII characters <= 127, and use an English
                // name always.
                // A valid name would be "MySampleClass".
                // This is stored internally in the category data and is
                // used whenever the category is opened to create all the
                // instances from the respective plugin classes. In other
                // words, this is the class identifier and must never
                // change between several releases or different NLS versions
                // of your plugin DLL, or otherwise plugin creation will fail.
                // Choose this name carefully.

       const char  *pcszClassTitle;
                // explanatory plugin class title, which is e.g. shown to the
                // user in the "Add widget" popup menu. Example: "Sample plugin".
                // This is not used to identify the class internally and may
                // change between releases and NLS versions. So you can set
                // this to a language-specific string.
                // Note: If this has the highest bit set, it is assumed to
                // be a string resource ID in the XWorkplace NLS DLL instead
                // of a real string:
                #define PLUGIN_STRING_RESOURCE     0x80000000
                            // V0.9.19 (2002-05-07) [umoeller]

       ULONG       ulClassFlags;

       PVOID       pvSettings;

    } PLUGINCLASS, *PPLUGINCLASS;

    typedef const struct _PLUGINCLASS *PCPLUGINCLASS;

    typedef struct _PLUGINCATEGORY
    {
        PCSZ            pcszName;
        PCSZ            pcszLocalizedName;
        HMTX            hmtx;
        LINKLIST        llClasses;
        BOOL            fClassesLoaded;
        ULONG           ulClassesRefCount;
        LINKLIST        llModules;
        BOOL            fModulesInitialized;
        ULONG           ulExtra;
        ULONG           ulPredefinedClasses;
        PPLUGINCLASS    pPredefinedClasses;
    } PLUGINCATEGORY, *PPLUGINCATEGORY;

    /* ******************************************************************
     *
     *   Plugins category management
     *
     ********************************************************************/

    BOOL plgRegisterCategory(PPLUGINCATEGORY pplgCategory,
                             PCSZ pcszCategoryName,
                             PCSZ pcszLocalizedName,
                             ULONG cbCategoryData,
                             ULONG ulPredefined,
                             PVOID pPredefined);

    PLINKLIST plgQueryCategories(VOID);

    /* ******************************************************************
     *
     *   Plugins category management
     *
     ********************************************************************/

    BOOL plgLockClasses(PPLUGINCATEGORY pplgCategory);

    VOID plgUnlockClasses(PPLUGINCATEGORY pplgCategory);

    VOID plgLoadClasses(PPLUGINCATEGORY pplgCategory);

    #ifdef LINKLIST_HEADER_INCLUDED
        PLINKLIST plgQueryClasses(PPLUGINCATEGORY pplgCategory);
    #endif

    VOID plgFreeClasses(PPLUGINCATEGORY pplgCategory);

    APIRET plgFindClass(PPLUGINCATEGORY pplgCategory,
                        PCSZ pcszWidgetClass,
                        PCPLUGINCLASS *ppClass);

    APIRET plgQueryClassVersion(PPLUGINCLASS pClass,
                                PULONG pulMajor,
                                PULONG pulMinor,
                                PULONG pulRevision);

    APIRET plgQueryClassModuleName(PPLUGINCLASS pClass,
                                   ULONG cbName,
                                   PCHAR pch);

    BOOL plgIsClassBuiltIn(PPLUGINCLASS pClass);

    BOOL plgIsClassVisible(PPLUGINCLASS pClass);

    BOOL plgIsClassEnabled(PPLUGINCLASS pClass);

    /* ******************************************************************
     *
     *   Plugin DLL Exports
     *
     ********************************************************************/

    // init-module export (ordinal 1)
    // WARNING: THIS PROTOTYPE HAS CHANGED WITH V0.9.9
    // IF QUERY-VERSION IS EXPORTED (@3, see below) THE NEW
    // PROTOTYPE IS USED
    typedef ULONG EXPENTRY FNPLGINITMODULE_OLD(HAB hab,
                                               HMODULE hmodXFLDR,
                                               PPLUGINCLASS *ppaClasses,
                                               PSZ pszErrorMsg);
    typedef FNPLGINITMODULE_OLD *PFNPLGINITMODULE_OLD;

    typedef ULONG EXPENTRY FNPLGINITMODULE_099(HAB hab,
                                               HMODULE hmodPlugin,
                                               HMODULE hmodXFLDR,
                                               PPLUGINCLASS *ppaClasses,
                                               PSZ pszErrorMsg);
    typedef FNPLGINITMODULE_099 *PFNPLGINITMODULE_099;

    // un-init-module export (ordinal 2)
    typedef VOID EXPENTRY FNPLGUNINITMODULE(VOID);
    typedef FNPLGUNINITMODULE *PFNPLGUNINITMODULE;

    // query version (ordinal 3; added V0.9.9 (2001-02-01) [umoeller])
    // IF QUERY-VERSION IS EXPORTED,THE NEW PROTOTYPE FOR
    // INIT_MODULE (above) WILL BE USED
    typedef VOID EXPENTRY FNPLGQUERYVERSION(PULONG pulMajor,
                                            PULONG pulMinor,
                                            PULONG pulRevision);
    typedef FNPLGQUERYVERSION *PFNPLGQUERYVERSION;
#endif
